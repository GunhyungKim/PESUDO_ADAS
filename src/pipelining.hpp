#include <opencv2/core.hpp>
#include <string>
#include <iostream>
#include <pthread.h>
#include "detect.hpp"
#include "Location/location.hpp"

#pragma once

namespace pipelineing{
    extern bool use_stereo;
    extern bool (*get_img_mono)(cv::Mat&);
    extern bool (*get_img_stereo)(cv::Mat&, cv::Mat&);
    extern bool (*get_imu_data)(location::IMUData& imu);
    extern bool (*get_gps_data)(location::GPSData& gps);
    extern bool RECORD;
    extern bool ShowCenterDistance;
    extern bool EnableDetection;
    void run();
}