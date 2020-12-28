#include <cmath>
#include <string>
#include <cblas.h>
#include <vector>
#include <iomanip>
#include <sstream>
#include "../myahrs_plus.hpp"
#pragma once




namespace location{

    struct IMUData
    {
        double roll,pitch,yaw,r_yaw,r_pitch,r_roll;
        IMUData(double roll=0,double pitch=0,double yaw=0);
        IMUData(WithrobotIMU::EulerAngle& euler);
        void print();
        std::string tostring();
        static int YAW_BIAS;
    };

    struct GPSData
    {
        double latitude,longitude,altitude;
        double length_per_latitude;
        double length_per_longitude;
        std::string status;
        GPSData(double latitude=35.888034,double longitude=128.612606,double altitude=0);
        void calculateLoc(double dist,double angle, double& outlat, double& outlng) const;
        void print();
        std::string tostring();
    };

    struct PixelData
    {
        double field_of_view_horizental = 92.8;
        double field_of_view_vertical = 50.0;
        int horizental_resolution;
        int vertical_resolution;
        int center_x;
        int center_y;
        double angle_per_x_pixel;
        double angle_per_y_pixel;
        std::string name;
        int x_pixel,y_pixel;
        double distance;
        PixelData(int img_w,int img_h,int x_pixel,int y_pixel,double distance, std::string name);
        void CalculatePixelAngle(double& x_angle,double& y_angle) const;


    };

    struct ObjectLocation
    {
        std::string name;
        double lat;
        double lng;
        double azimuth;
        std::string toJson();
    };

    ObjectLocation calculateLocation(const IMUData& imu_data,const GPSData& gps_data,const PixelData& cam);
    std::string toJson(std::vector<ObjectLocation> loc_arr);

}
