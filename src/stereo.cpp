#include "stereo.hpp"
#include <string>
#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <boost/filesystem.hpp>
//#define XIMGPROC
#ifdef XIMGPROC
#include <opencv2/ximgproc.hpp>
#endif

using namespace std;


stereo_match::stereo_match(){}

string cali_file_name = "cali.yaml";

void get_img_stereo_blocking(cv::Mat& imL,cv::Mat& imR);
int calitest()
{
    int numBoards = 10;
    int numCornersHor =9;
    int numCornersVer =6;
    int numSquares = numCornersHor * numCornersVer;
    Size board_sz = Size(numCornersHor, numCornersVer);

    vector<vector<Point3f>> object_points;
    vector<vector<Point2f>> image_points;

    vector<Point2f> corners;
    int successes=0;

    Mat image,imageR;
    Mat gray_image;
    get_img_stereo_blocking(image,imageR);
    vector<Point3f> obj;
    for(int j=0;j<numSquares;j++)
        obj.push_back(Point3f(j/numCornersHor, j%numCornersHor, 0.0f));

      while(successes<numBoards)
    {
        cvtColor(image, gray_image, CV_BGR2GRAY);
         bool found = findChessboardCorners(image, board_sz, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);

        if(found)
        {
            cornerSubPix(gray_image, corners, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
            drawChessboardCorners(gray_image, board_sz, corners, found);
        }
         imshow("win1", image);
        imshow("win2", gray_image);

        get_img_stereo_blocking(image,imageR);
        int key = waitKey(1);
        if(key>0)
            cout<<key<<endl;
          if(key==27)

            return 0;

        if(key==' ' && found!=0)
        {
            image_points.push_back(corners);
            object_points.push_back(obj);

            printf("Snap stored!");

            successes++;

            if(successes>=numBoards)
                break;
        }
    }
    Mat intrinsic = Mat(3, 3, CV_32FC1);
    Mat distCoeffs;
    vector<Mat> rvecs;
    vector<Mat> tvecs;
    
    intrinsic.ptr<float>(0)[0] = 1;
    intrinsic.ptr<float>(1)[1] = 1;
    calibrateCamera(object_points, image_points, image.size(), intrinsic, distCoeffs, rvecs, tvecs);
     Mat imageUndistorted;
    while(1)
    {
        get_img_stereo_blocking(image,imageR);
        undistort(image, imageUndistorted, intrinsic, distCoeffs);

        imshow("win1", image);
        imshow("win2", imageUndistorted);
        waitKey(1);
    }
        
    return 0;
}

void cali_save()
{
    vector<vector<Point2f>> imgpointsR,imgpointsL;
    vector<Point3f> objPts;
    vector<vector<Point3f>> arrobjPts;
    double box_size = 24/1000.;
    
    
    for(int y=0;y<6;y++)
    for(int x=0;x<9;x++)
        objPts.push_back(Point3f(x*box_size,y*box_size,0));

    TermCriteria criteria,criteriaStereo;
    criteria.type = TermCriteria::EPS + TermCriteria::MAX_ITER;
    criteria.epsilon = 0.1;
    criteria.maxCount = 30;
    criteriaStereo.type =  TermCriteria::EPS + TermCriteria::MAX_ITER;
    criteriaStereo.epsilon=0.1;
    criteriaStereo.maxCount=30;
    Mat chessImgR ,chessImgL;
    for(int i=0;i<20;i++)
    {
        string n = to_string(i);
        
        Mat R,L;
        string fileR = "../darknet/python/Stereo/imgs/chessboard-R"+n+".png";
        string fileL = "../darknet/python/Stereo/imgs/chessboard-L"+n+".png";
        if(!boost::filesystem::exists(fileL)||!boost::filesystem::exists(fileR))
            continue;

        R = imread(fileR,0);
        L = imread(fileL,0);
        resize(R,chessImgR,Size(640,480));
        resize(L,chessImgL,Size(640,480));
        Mat cornersR,cornersL;
        bool retR,retL;
        retR = findChessboardCorners(chessImgR,Size(9,6),cornersR);
        retL = findChessboardCorners(chessImgL,Size(9,6),cornersL);
        if(retR&&retL)
        {

            cornerSubPix(chessImgR,cornersR,Size(11,11),Size(-1,-1),criteria);
            cornerSubPix(chessImgL,cornersL,Size(11,11),Size(-1,-1),criteria);
            imgpointsR.push_back(cornersR);
            imgpointsL.push_back(cornersL);
            arrobjPts.push_back(objPts);
        }
        cout<<"calibration..."<<i<<endl;

    }
    Size imgSize = chessImgL.size();
    Mat mtxR, distR, mtxL,distL;
    vector<Mat> rvecsR,tvecsR, rvecsL,tvecsL;
    double rms_r = calibrateCamera(arrobjPts,imgpointsR,imgSize,mtxR,distR,rvecsR,tvecsR);
    double rms_l = calibrateCamera(arrobjPts,imgpointsL,imgSize,mtxL,distL,rvecsL,tvecsL);
    Mat optR = getOptimalNewCameraMatrix(mtxR,distR,imgSize,1,imgSize);
    Mat optL = getOptimalNewCameraMatrix(mtxL,distL,imgSize,1,imgSize);
    //mtxR = optR;
    //mtxL = optL;
    
    Vec3d T;
    Mat R,E,F;
    int flasgs =0;
    flasgs|= CALIB_FIX_INTRINSIC;
    stereoCalibrate(arrobjPts,imgpointsL,imgpointsR,mtxL,distL,mtxR,distR,imgSize,R,T,E,F,flasgs,criteriaStereo);

    Mat R1,R2,P1,P2,Q;
    stereoRectify(mtxL,distL,mtxR,distR,imgSize,R,T,R1,R2,P1,P2,Q);

    FileStorage fs1(cali_file_name, FileStorage::WRITE);
    fs1<<"mtxR"<<mtxR;
    fs1<<"mtxL"<<mtxL;
    fs1<<"distR"<<distR;
    fs1<<"distL"<<distL;
    fs1<<"R"<<R;
    fs1<<"T"<<T;
    fs1<<"E"<<E;
    fs1<<"F"<<F;
    fs1<<"optR"<<optR;
    fs1<<"optL"<<optL;


    cout<<"cal saved"<<endl;

    fs1<<"R1"<<R1;
    fs1<<"R2"<<R2;
    fs1<<"P1"<<P1;
    fs1<<"P2"<<P2;
    fs1<<"Q"<<Q;

    cout<<"rectification saved"<<endl;
}


void reprojectImage(Mat& disp,Mat& _3d,Mat& Q)
{
    Mat_<float> vec_tmp(4,1);
    for(int y=0;y<disp.rows;y++)
        for(int x=0;x<disp.cols;x++)
        {
            vec_tmp(0)=x;vec_tmp(1)=y;vec_tmp(2)=disp.at<float>(y,x);vec_tmp(3)=1;
            vec_tmp = Q*vec_tmp;
            vec_tmp/=vec_tmp(3);
            _3d.at<Point3d>(y,x) = Point3d(vec_tmp(0),vec_tmp(1),vec_tmp(3));
        }
}
static Ptr<StereoBM> left_matcher;
static Ptr<StereoMatcher> right_matcher;
#ifdef XIMGPROC
static Ptr<ximgproc::DisparityWLSFilter> wls_filter;
#endif
static Ptr<StereoSGBM> sgbm;
static Mat mtxR, distR, mtxL,distL,R,optR,optL;
static Vec3d T;
static Mat R1,R2,P1,P2,Q;
static Mat lmapx,lmapy,rmapx,rmapy;
Size imgSize;
bool initialized = false;
bool RECALIBRATE = false;
void init()
{
    initialized = true;
    cout<<cv::getBuildInformation()<<endl;
    if(RECALIBRATE)
    {
       cali_save();
    }
    FileStorage fs1(cali_file_name,FileStorage::READ);
    fs1["mtxL"]>>mtxL;
    fs1["mtxR"]>>mtxR;
    fs1["distL"]>>distL;
    fs1["distR"]>>distR;
    fs1["optR"]>>optR;
    fs1["optL"]>>optL;
    fs1["R"]>>R;
    fs1["T"]>>T;
    fs1["R1"]>>R1;
    fs1["R2"]>>R2;
    fs1["P1"]>>P1;
    fs1["P2"]>>P2;
    fs1["Q"]>>Q;
    cout<<"stereo param loaded"<<endl;
    
    initUndistortRectifyMap(mtxL,distL,R1,P1,imgSize,CV_32F,lmapx,lmapy);
    initUndistortRectifyMap(mtxR,distR,R2,P2,imgSize,CV_32F,rmapx,rmapy);

    left_matcher = cuda::StereoBM::create();
    left_matcher->setMinDisparity(0);
    left_matcher->setNumDisparities(64);
    left_matcher->setBlockSize(15);
    left_matcher->setUniquenessRatio(15);
    left_matcher->setSpeckleWindowSize(30);
    left_matcher->setSpeckleRange(41);
    left_matcher->setDisp12MaxDiff(20);
    left_matcher->setPreFilterSize(21);
    left_matcher->setPreFilterCap(21);
    left_matcher->setDisp12MaxDiff(20);
    
    #ifdef XIMGPROC
    right_matcher = ximgproc::createRightMatcher(left_matcher);
    wls_filter = ximgproc::createDisparityWLSFilter(left_matcher);
    wls_filter->setLambda(8000);
    wls_filter->setSigmaColor(1.8);
    #endif

    sgbm = StereoSGBM::create();
    sgbm->setMinDisparity(0);
    sgbm->setNumDisparities(64);
    sgbm->setBlockSize(21);
    sgbm->setUniquenessRatio(5);
    sgbm->setSpeckleWindowSize(20);
    sgbm->setSpeckleRange(7);
    sgbm->setDisp12MaxDiff(20);
    sgbm->setP1(8*3*5*5);
    sgbm->setP2(32*3*5*5);
    sgbm->setMode(StereoSGBM::MODE_SGBM);
}

void onClick(int event,int x,int y, int flags,void* userdata)
{
    stereo_match* sm =(stereo_match*) userdata;
    if(event==EVENT_LBUTTONDOWN)
    {
        sm->distance(x,y);
    }
}


float stereo_match::distance(int x,int y)
{
    int width = XYZ.size().width;
    int height = XYZ.size().height;
    int cnt = 0;
    double sum = 0;
    for(int i=x-3;i<x+3;i++)
        for(int j=y-3;j<y+3;j++)
    {
        if(i<0||j<0||i>=width||j>=height)
            continue;

        Vec3f pt = XYZ.at<Vec3f>(y,x);    
        if(pt(2)>0&&pt(2)<200)
        {
            cnt++;
            sum+=pt(2);
        }
    }
    //Vec3f p = XYZ.at<Vec3f>(y,x);    
    //printf("x:%f y:%f z: %f\n",p(0),p(1),p(2));
    if(cnt>0)
        sum /=cnt;
    return sum;
}
float stereo_match::distance_relative(double x,double y)
{
    int xpos,ypos;
    xpos = x*this->XYZ.size().width;
    ypos = y*this->XYZ.size().height;
    return this->distance(xpos,ypos);
}
void stereo_match::calc_disp(Mat& imL,Mat& imR)
{
    this->imL = imL;
    this->imR = imR;
    Mat gray1,gray2;
    remap(imL,imgU1,lmapx,lmapy,INTER_LINEAR);
    remap(imR,imgU2,rmapx,rmapy,INTER_LINEAR);
    cvtColor(imgU1,gray1,COLOR_BGR2GRAY);
    cvtColor(imgU2,gray2,COLOR_BGR2GRAY);
    /*imshow("u1",imgU1);
    imshow("u2",imgU2); //멀티스레딩시 오류 유발
    waitKey(1);*/
    Mat dispL,dispR,dispSBMscale,filtered_disp;
    left_matcher->compute(gray1,gray2,dispL);


    #ifdef XIMGPROC
    right_matcher->compute(gray2,gray1,dispR);
    wls_filter->filter(dispL,gray1,filtered_disp,dispR);
    filtered_disp.convertTo(dispSBMscale,CV_32F,1/16.,-sgbm->getMinDisparity());
    normalize(filtered_disp,this->nomalized_disp ,0,255,NORM_MINMAX,CV_8U);
    
    #else
    dispL.convertTo(dispSBMscale,CV_32F,1/16.,-sgbm->getMinDisparity());
    normalize(dispL,this->nomalized_disp ,0,255,NORM_MINMAX,CV_8U);
    #endif
    
    reprojectImageTo3D(dispSBMscale,XYZ,Q);

    /*
    namedWindow("disp",1);
    setMouseCallback("disp",onClick,this);
    imshow("disp",ndisp);
    waitKey(5);*/
}
stereo_match::stereo_match(Size size)
{
    imgSize = size;
    if(!initialized)
        init();
}

int test1()
{
    
    Mat imL,imR;
    imL = imread("../darknet/python/Stereo/imgs/chessboard-L5.png",1);
    imR = imread("../darknet/python/Stereo/imgs/chessboard-R5.png",1);
    stereo_match sm(imL.size());
    sm.calc_disp(imL,imR);
    return 0;
}
