#include "drawing.hpp"
#include <sstream>
using namespace std;
using namespace cv;


static CvScalar textColor(239,101,9);
static CvScalar boxColor(255,0,0);
static CvScalar boxWarning(0,0,255);

void putMarker(Mat& im,int x,int y)
{
    drawMarker(im,Point(x,y), CV_RGB(255,0,0));
}

void putTextWithBG(Mat &im, string label ,int x,int y, int baseline ,int fontface,double fontscale,int thickness)
{
    Size t_size = cv::getTextSize(label,fontface,fontscale,thickness, &baseline);            
    y-= t_size.height/2;
    cv::rectangle(im,Point(x,y+baseline),Point(x+t_size.width,y-t_size.height),CV_RGB(255,255,255),CV_FILLED);
    putText(im,label,cvPoint(x,y),fontface,fontscale,textColor,thickness,CV_AA);
}
void draw_detections(vector<detection_object>& obj_arr, Mat& im)
{
    for(int i=0;i<obj_arr.size();i++)
    {
        detection_object& obj = obj_arr[i]; 
        
        if(obj.distance<=2)
        {
            rectangle(im,obj.get_bbox(),boxWarning,2);
        }else{
            rectangle(im,obj.get_bbox(),boxColor,2);
        }
            stringstream ss;
            ss<<fixed<<setprecision(1);
            ss<<obj.class_name<<" "<<to_string((int)(obj.probability*100))<<"% ";
            ss<<obj.distance<<"m";
            string label = ss.str();

            putTextWithBG(im,label,obj.left(),obj.top());

    }
}