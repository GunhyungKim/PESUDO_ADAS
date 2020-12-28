
#include "stereo_cam.hpp"
#include <opencv2/imgproc.hpp>
#include <pthread.h>
#include <unistd.h>
StereoCamera::StereoCamera(int resolution, double frame_rate): camera(NULL) {

    enum_dev_list();

    camera = new Withrobot::Camera(devPath_.c_str());

    if (resolution == 0) { width_ = 1280; height_ = 960;}
    if (resolution == 1) { width_ = 1280; height_ = 720;}
    if (resolution == 2) { width_ = 640; height_  = 480;}
    if (resolution == 3) { width_ = 640; height_  = 360;}

    camera->set_format(width_, height_, Withrobot::fourcc_to_pixformat('Y','U','Y','V'), 1, (unsigned int)frame_rate);

    /*
        * get current camera format (image size and frame rate)
        */
    camera->get_current_format(camFormat);

    camFormat.dbg_print();

    /* Withrobot camera start */
    camera->start();
}
StereoCamera::~StereoCamera() 
{
    camera->stop();
    delete camera;
}
void StereoCamera::enum_dev_list()
{
    /* enumerate device(UVC compatible devices) list */
    std::vector<Withrobot::usb_device_info> dev_list;
    int dev_num = Withrobot::get_usb_device_info_list(dev_list);

    if (dev_num < 1) {
        dev_list.clear();

        return;
    }

    for (unsigned int i=0; i < dev_list.size(); i++) {
        if (dev_list[i].product == "oCamS-1CGN-U")
        {
            devPath_ = dev_list[i].dev_node;
            return;
        }
    }
}
void StereoCamera::uvc_control(int exposure, int gain, int blue, int red, bool ae)
{
    /* Exposure Setting */
    camera->set_control("Exposure (Absolute)", exposure);

    /* Gain Setting */
    camera->set_control("Gain", gain);
    
    /* White Balance Setting */
    camera->set_control("White Balance Blue Component", blue);
    camera->set_control("White Balance Red Component", red);

    /* Auto Exposure Setting */
    if (ae)
        camera->set_control("Exposure, Auto", 0x3);
    else
        camera->set_control("Exposure, Auto", 0x1);
}
bool StereoCamera::getImages(cv::Mat &left_image, cv::Mat &right_image, uint32_t &time_stamp) {

    cv::Mat srcImg(cv::Size(camFormat.width, camFormat.height), CV_8UC2);
    cv::Mat dstImg[2];

    uint32_t ts;

    if (camera->get_frame(srcImg.data, camFormat.image_size, 1) != -1)
    {
        // time stamp
        memcpy(&ts, srcImg.data, sizeof(ts));

        cv::split(srcImg, dstImg);

        time_stamp = ts;
        cv::cvtColor(dstImg[0],right_image,cv::COLOR_BayerGB2BGR);
        cv::cvtColor(dstImg[1],left_image,cv::COLOR_BayerGB2BGR);
        return true;
    } else {
        return false;
    }
}

void MyAhrsDriver::OnSensorData(int sensor_id, SensorData data)
{
    LockGuard _l(lock_);
    sensor_data_ = data;
}
void MyAhrsDriver::OnAttributeChange(int sensor_id, std::string attribute_name, std::string value)
{
    printf("OnAttributeChange(id %d, %s, %s)\n", sensor_id, attribute_name.c_str(), value.c_str());
}
MyAhrsDriver::MyAhrsDriver(std::string port, int baud_rate)
      : iMyAhrsPlus(port, baud_rate)
  {

  }
MyAhrsDriver::~MyAhrsDriver(){}

bool MyAhrsDriver::initialize(std::string mode)
{
    bool ok = false;

    do
    {
        if(start() == false) break;

        /* IMU mode */
    if(cmd_data_format(mode.c_str()) == false) break;
    printf("IMU initialized: %s\r\n", mode.c_str());
        ok = true;
    } while(0);

    return ok;
}

inline void MyAhrsDriver::get_data(SensorData& data)
{
    LockGuard _l(lock_);
    data = sensor_data_;
}
inline SensorData MyAhrsDriver::get_data()
{
    LockGuard _l(lock_);
    return sensor_data_;
}




void StereoCamera::getImagesBlocking(cv::Mat& imL,cv::Mat& imR)
{
    while(!getImages(imL,imR)){} //카메라 이미지 획득
}


bool StereoCamera::getImages(cv::Mat& imL,cv::Mat& imR)//스테레오 이미지 획득 함수
{
    uint32_t time;
    return getImages(imL,imR,time); //카메라 이미지 획득
}

void StereoCamera::fetchLoop()
{
    cv::Mat tmpL,tmpR;
    while(1)
    {
        getImagesBlocking(tmpL,tmpR);
        buffer_mutex.lock();
        bufL = tmpL;
        bufR = tmpR;
        buffer_mutex.unlock();
    }
}

void* StereoCamera::fetchLoopStatic(void *arg)
{
    StereoCamera* cam = (StereoCamera*)arg;
    cam->fetchLoop();
    return NULL;
}

void StereoCamera::runFetchThread()
{
    if(buffer_thread)
    {
        std::cout<<"thread already running"<<std::endl;
        return;
    }
    pthread_create(&buffer_thread,0,StereoCamera::fetchLoopStatic,this);
}
bool StereoCamera::getThreadedImages(cv::Mat &left_image, cv::Mat &right_image)
{
    buffer_mutex.lock();
    if(!bufL.empty()&&!bufR.empty())
    {
        left_image = bufL;
        right_image = bufR;
    }
    buffer_mutex.unlock();
    if(left_image.empty()||right_image.empty())
        return false;
    else
        return true;
}
using namespace std;
 
int test3()
{
   
    StereoCamera scam(1,30);
    cv::Mat l,r;
    uint32_t time;
    scam.getImages(l,r,time);

    MyAhrsDriver ahrs;
    ahrs.initialize();
    SensorData sdata;
    ahrs.get_data(sdata);
    

    
    return 0;
}