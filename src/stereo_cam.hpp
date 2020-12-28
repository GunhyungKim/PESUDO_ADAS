#include "withrobot_camera.hpp"
#include "format_converter/ConvertColor.h"
#include <iostream>
#include <opencv2/core.hpp>
#include "myahrs_plus.hpp"
#include "mutex"



class StereoCamera
{
    Withrobot::Camera* camera;
    Withrobot::camera_format camFormat;

private:
    int width_;
    int height_;
    std::string devPath_;
    cv::Mat bufL;
    cv::Mat bufR;
    std::mutex buffer_mutex;
    pthread_t buffer_thread=0;
    void fetchLoop();
    static void* fetchLoopStatic(void *arg);
public:
	/**
	 * @brief      { stereo camera driver }
	 *
	 * @param[in]  resolution  The resolution
	 * @param[in]  frame_rate  The frame rate
	 */
    StereoCamera(int resolution, double frame_rate);

	~StereoCamera();

    void enum_dev_list();

    void uvc_control(int exposure, int gain, int blue, int red, bool ae);

	/**
	 * @brief      Gets the images.
	 *
	 * @param      left_image   The left image
	 * @param      right_image  The right image
	 *
	 * @return     The images.
	 */
    bool getImages(cv::Mat &left_image, cv::Mat &right_image, uint32_t &time_stamp);
    bool getImages(cv::Mat &left_image, cv::Mat &right_image);
    void getImagesBlocking(cv::Mat& imL,cv::Mat& imR);
    void runFetchThread();
    bool getThreadedImages(cv::Mat &left_image, cv::Mat &right_image);


};

using namespace WithrobotIMU;

class MyAhrsDriver : public iMyAhrsPlus
{
private:
  Platform::Mutex lock_;
  SensorData sensor_data_;

  std::string parent_frame_id_;
  std::string frame_id_;
  double linear_acceleration_stddev_;
  double angular_velocity_stddev_;
  double magnetic_field_stddev_;
  double orientation_stddev_;

  void OnSensorData(int sensor_id, SensorData data);

  void OnAttributeChange(int sensor_id, std::string attribute_name, std::string value);


public:
  MyAhrsDriver(std::string port="", int baud_rate=115200);

  ~MyAhrsDriver();
  bool initialize(std::string mode="");

  void get_data(SensorData& data);

  SensorData get_data(); 

};