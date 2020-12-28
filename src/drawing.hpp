#include<opencv2/opencv.hpp>
#include<string>
#include "detect.hpp"
#pragma once

void putTextWithBG(cv::Mat &im, std::string label ,int x,int y, int baseline=0 ,int fontface = cv::FONT_HERSHEY_COMPLEX_SMALL,double fontscale=1.2,int thickness=1);
void draw_detections(std::vector<detection_object>& obj_arr, cv::Mat& im);
void putMarker(cv::Mat& im,int x,int y);