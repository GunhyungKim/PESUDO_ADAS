#include "location.hpp"

using namespace std;
using namespace location;
double radians(double degree)
{
    return degree * M_PI/180;
}
double degrees(double radian)
{
    return radian / M_PI * 180;
}
int IMUData::YAW_BIAS = 0;

IMUData::IMUData(WithrobotIMU::EulerAngle& euler):IMUData::IMUData(-euler.roll,-euler.pitch, (YAW_BIAS - euler.yaw) + ((YAW_BIAS<euler.yaw)?360:0) )
{

}

IMUData::IMUData(double roll,double pitch,double yaw)
{


    this->yaw = yaw;
    this->pitch = pitch;
    this->roll = roll;
    
    r_yaw = radians(yaw);
    r_pitch = radians(pitch);
    r_roll = radians(roll);

}
void IMUData::print()
{
    printf("%s\n",tostring().c_str());
}
string IMUData::tostring()
{
    char buff[100];
    sprintf(buff,"IMU: yaw:%6.1f pitch:%6.1f roll:%6.1f",yaw,pitch,roll);
    string str(buff);
    return str;
}


void GPSData::print()
{
    printf("%s\n",tostring().c_str());
}
string GPSData::tostring()
{
    char buff[100];
    sprintf(buff,"GPS: lat:%3.8f lng:%3.8f",this->latitude,this->longitude);
    string str(buff);
    return str;
}

GPSData::GPSData(double latitude,double longitude,double altitude)
{
    this->latitude = latitude;
    this->longitude = longitude;  
    this->altitude = altitude;
    length_per_latitude = 110.569 * 1000;
    length_per_longitude = 111.322*1000*cos(radians(latitude));
}
void GPSData::calculateLoc(double dist,double angle, double& outlat, double& outlng) const
{
    angle = radians(angle);
    outlat = latitude + cos(angle) * dist/length_per_latitude;
    outlng = longitude + sin(angle) * dist /length_per_longitude; 
}




PixelData::PixelData(int img_width,int img_height,int x_pixel,int y_pixel,double distance, string name)
{
    this->distance = distance;
    this->x_pixel = x_pixel;
    this->y_pixel = y_pixel;
    this->name  = name;

    this->horizental_resolution = img_width;
    this->vertical_resolution = img_height;
    this->center_x = this->horizental_resolution/2;
    this->center_y = this->vertical_resolution/2;
    this->angle_per_x_pixel = this->field_of_view_horizental/this->horizental_resolution;
    this->angle_per_y_pixel = this->field_of_view_vertical/this->vertical_resolution;

}
void PixelData::CalculatePixelAngle(double& x_angle,double& y_angle) const
{
    x_angle = angle_per_x_pixel * (x_pixel - center_x);
    y_angle = angle_per_y_pixel *(y_pixel - center_y);
}


void calculatePixelAngle(int im_width,int im_height,int x_pixel,int y_pixel, double distance, string name,double& xangle,double& yangle)
{
    PixelData cmath(im_width,im_height,x_pixel,y_pixel,distance,name);
    cmath.CalculatePixelAngle(xangle,yangle);
}


ObjectLocation location::calculateLocation(const IMUData& imu_data,const GPSData& gps_data,const PixelData& cam)
{
    double angle_x,angle_y;
    cam.CalculatePixelAngle(angle_x,angle_y);
    double angle_matrix[2] = {angle_x,angle_y};
    double roll_matrix[4] =
    {cos(imu_data.r_roll),-sin(imu_data.r_roll),
    sin(imu_data.r_roll),cos(imu_data.r_roll)};

    double result[2];
    cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,2,1,2,1.0,roll_matrix,2,angle_matrix,1,0,result,1);
    result[0] += imu_data.yaw;
    result[1] += imu_data.pitch;
    double pi_angle = result[0];
    double theta_angle = result[1];

    double xy_distance = cos(radians(theta_angle))*cam.distance;

    //printf("xy_plane_distance: %fm\n",xy_distance);
    //printf("rotated pixel angle %f %f\n",pi_angle,theta_angle);

    double lat, lng;
    gps_data.calculateLoc(xy_distance,pi_angle,lat,lng);
    ObjectLocation loc;
    loc.lat = lat;
    loc.lng = lng;
    loc.name = cam.name;
    loc.azimuth = INFINITY;
    return loc;
} 
string ObjectLocation::toJson()
{
    stringstream ss;
    ss<<fixed<<setprecision(10);
    ss<<"{\n";
    ss<<"\"lat\":"<<lat<<",\n";
    ss<<"\"lng\":"<<lng<<",\n";
    ss<<"\"name\":"<<"\""<<name<<"\"";
    if(!isinf(azimuth)){
    ss<<",\n"<<"\"azimuth\":"<<azimuth;
    }
    ss<<"\n}";
    return ss.str();
}
string location::toJson(vector<ObjectLocation> locations)
{
    stringstream ss;
    ss<<"[";
    for(int i=0;i<locations.size();i++)
    {
        ObjectLocation &loc = locations[i];
        
        ss<<loc.toJson();
        if(i+1!=locations.size())
            ss<<",\n";
    }
    ss<<"]";
    return ss.str();
}


int test2()
{
    IMUData imu(0,0,0);
    GPSData gps(35.886906,128.60928,0);
    double dist = 100;
    int x=640;
    int y=480;
    PixelData cam(1280,960,x,y,dist,"test_name");
    for(int i=0;i<10;i++)
    {
        imu = IMUData(0,0,36*i);
        auto r = location::calculateLocation(imu,gps,cam);
        printf("lat:%f lng:%f\n",r.lat,r.lng);
    }

    return 0;
}



