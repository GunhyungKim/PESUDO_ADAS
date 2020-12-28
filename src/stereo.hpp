#include <opencv2/opencv.hpp>


using namespace cv;

extern bool RECALIBRATE;
int calitest();
struct stereo_match
{
    Mat imgU1,imgU2;
    Mat imL,imR;
    Mat XYZ;
    Mat nomalized_disp;
    float distance(int x,int y);
    float distance_relative(double x,double y);
    void calc_disp(Mat& imL,Mat& imR);
    stereo_match(Size size);
    stereo_match();
};


