#include<iostream>
#include<opencv2/opencv.hpp>
#include<ctime>
#include<string>
#include<vector>
#include<memory>
extern "C"{
#include"darknet.h"
}
 
#pragma once


extern double detection_threshold;
struct detection_object
{
    int x;
    int y;
    int width;
    int height;
    int img_width;
    int img_height;
    double distance;
    std::string class_name;
    double probability;
    void print();
    cv::Rect get_bbox();
    int left();
    int top();
    double relative_x();
    double relative_y();
};

std::vector<detection_object> detect_objects(cv::Mat& input,cv::Mat& resized);
std::vector<detection_object> detect_objects(image& input);
int init_network();

void mat_into_image(cv::Mat* src,image& im);

float* network_predict_image(image& im);