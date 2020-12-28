#include "detect.hpp"
#include "stereo_cam.hpp"
#include "stereo.hpp"
#include "Location/location.hpp"
#include "Location/udp_socket.hpp"
#include "Location/web_socket_server.hpp"
#include "Location/gps_reading.hpp"
#include "drawing.hpp"
#include "utils.hpp"
#include "pipelining.hpp"
#include "boost/program_options.hpp"
#include "iostream"
using namespace std;
using namespace cv;


/* oCam 설정값 관련 디폴트
<!-- ... -->
<launch>
  <arg name="pi/2" value="1.5707963267948966" />
  <arg name="optical_rotate" value="0 0.06 0 0 0 0 1" />

  <node pkg="ocams" type="ocams" name="ocams" output="screen">
    <param name="resolution"    value="2"/> <!-- 0: 1280x960, 1: 1280x720, 2: 640x480, 3: 640x360 -->
    <param name="frame_rate"    value="30"/>
    <param name="exposure"      value="100"/>
    <param name="gain"          value="100"/>
    <param name="wb_blue"       value="180"/>
    <param name="wb_red"        value="145"/>
    <param name="auto_exposure" value="false"/>
    <param name="show_image"    value="false"/>
    <param name="port"          value="/dev/ttyACM0" /> <!-- imu port-->

    <!-- imu mode-->
    <!-- AMGQUA : Accelerometer(+gravity) + Magnetometer + Gyroscope + Quaternion -->
    <!-- LMGQUA : Linear Acceleration + Magnetometer + Gyroscope + Quaternion -->
    <param name="imu_mode"      value="LMGQUA" />

    <param name="left_frame_id" value="left_frame"/>
    <param name="right_frame_id" value="right_frame"/>
  </node>

  <node pkg="tf2_ros" type="static_transform_publisher" name="ocams_base_link1"
    args="0 0 0 0 0 0 1 map base_link" />

  <node pkg="tf2_ros" type="static_transform_publisher" name="ocams_base_link2"
    args="0 0.06 0 -$(arg pi/2) 0 -$(arg pi/2) base_link left_frame" />

</launch>
*/
//IMU와 카메라를 쓸수있게 준비한다.
#define OCAM_MODE_1280_960 0
#define OCAM_MODE_640_480 2
int ocam_exposure = 400;
int ocam_gain = 200;
bool ocam_ae = true;
void init_ocam(StereoCamera** cam,MyAhrsDriver** ahrs)
{

    (*cam) =new StereoCamera(OCAM_MODE_640_480,30);
    (*cam)->uvc_control(ocam_exposure,ocam_gain,180,145,ocam_ae);

    (*ahrs) =new MyAhrsDriver("/dev/ttyACM0",115200);
    //ahrs.get_data
    //string imu_mode = "AMGQUA";
    string imu_mode = "AMGQUA";
    if((*ahrs)->initialize(imu_mode) == false)
    {
        perror("IMU initialize false!\r\n oCamS-1CGN-U sends IMU data through Virtual COM port.\r\n \
    So, user needs to write following rules into udev rule file like below.\r\n \
    -------------------------------------------------------------------------------\r\n \
    $ sudo vi /etc/udev/rules.d/99-ttyacms.rules\r\n \
    ATTRS{idVendor}==\"04b4\" ATTRS{idProduct}==\"00f9\", MODE=\"0666\", ENV{ID_MM_DEVICE_IGNORE}=\"1\"\r\n \
    ATTRS{idVendor}==\"04b4\" ATTRS{idProduct}==\"00f8\", MODE=\"0666\", ENV{ID_MM_DEVICE_IGNORE}=\"1\"\r\n \
    $ sudo udevadm control -R\r\n \
    -------------------------------------------------------------------------------\r\n");
    //exit(0);
    
    }
    (*cam) ->runFetchThread();
}

//ocam 사용시 호출할 함수(병렬처리)) 안해서 느림)
void main_stereo()
{

    init_network();
    socket_server::run_server();
    SensorData imu_raw;
    StereoCamera* cam;
    MyAhrsDriver* ahrs;
    init_ocam(&cam,&ahrs);

    while(1)
    {
        Mat imgL,imgR,imgLHalf,imgRHalf;
        uint32_t time;
        vector<location::ObjectLocation> loc_arr;
        
        location::GPSData gps;
        location::IMUData imu;
        CHECK_FPS_BEGIN("main");

            
        ahrs->get_data(imu_raw);//imu 데이터 획득
        imu_raw.euler_angle = imu_raw.quaternion.to_euler_angle();  
        imu = location::IMUData(imu_raw.euler_angle);
        imu.print();

        if(!cam->getImages(imgL,imgR,time)) //카메라 이미지 획득
            continue;
        resize(imgL,imgLHalf,Size(0,0),.5,.5);
        resize(imgR,imgRHalf,Size(0,0),.5,.5);
        stereo_match sm(imgLHalf.size()); //스테레오 맵핑객체 생성
        sm.calc_disp(imgLHalf,imgRHalf); //disparity 계산

        Mat rectified = sm.imgU2;//rectify된 이미지 획득
        Mat marked_im;
        
        vector<detection_object> r = detect_objects(rectified,marked_im);

        for(int i=0;i<r.size();i++)
        {
            r[i].distance = sm.distance_relative(r[i].relative_x(),r[i].relative_y());
            location::PixelData cam(r[i].img_width,r[i].img_height,r[i].x,r[i].y,2,r[i].class_name);
            
            location::ObjectLocation loc = location::calculateLocation(imu,gps,cam);
            loc_arr.push_back(loc);
        }
        

        draw_detections(r,marked_im);

        
        CHECK_FPS_IMG_END(marked_im,10,30);
        
        
        imshow("view",marked_im);
        
        int key = waitKeyEx(1);
        if(key>=0)
            cout<<key<<endl;

        string loc_json = toJson(loc_arr);
        socket_server::set_message(loc_json);
        udp_socket::send(loc_json);

    }
}
 
static cv::VideoCapture cap;
static StereoCamera* stereo_cam;
static MyAhrsDriver* imu_reader;


bool get_img_video(Mat& output)
{
    if(cap.isOpened())
    {
        return cap.read(output);
    }
    else
        return false;
}


bool get_img_stereo(cv::Mat& imL,cv::Mat& imR)//스테레오 이미지 획득 함수
{
    uint32_t time;
    return stereo_cam->getThreadedImages(imL,imR); //카메라 이미지 획득
}
void get_img_stereo_blocking(cv::Mat& imL,cv::Mat& imR)
{
    return stereo_cam->getImagesBlocking(imL,imR); //카메라 이미지 획득
}

bool get_imu_data(location::IMUData& imu)//IMU데이터 획득 함수
{
    SensorData s_data;
    imu_reader->get_data(s_data);
    s_data.euler_angle = s_data.quaternion.to_euler_angle();
    imu = location::IMUData(s_data.euler_angle);
    return true;
}
bool get_gps_data(location::GPSData& gps)
{
    //gps = location::GPSData();//아직 센서가 없음
    gps = location::getLocationData();
    //cout<<gps.tostring(); 
    return true;
}

void run_ppline_mono()//비디오 파일이나 모노 카메라를 위한 최적화된 메인 함수
{
    //init_ocam(&stereo_cam,&imu_reader);
    //pipelineing::get_imu_data = get_imu_data;

    socket_server::run_server();
    init_network();
    pipelineing::get_img_mono = &get_img_video;
    pipelineing::run();
}

void run_ppline_stereo()//ocam을 위한 최적화된 메인 함수
{
    init_ocam(&stereo_cam,&imu_reader);
    socket_server::run_server();
    init_network();
    pipelineing::get_imu_data = get_imu_data;
    pipelineing::get_gps_data = get_gps_data;
    pipelineing::get_img_stereo = get_img_stereo;
    pipelineing::use_stereo = true;
    pipelineing::run();
}

int main(int argc,char** argv)
{

    using namespace boost::program_options;
    options_description desc("Allowed options");
    desc.add_options()
    ("help,h","prodece a help screen")
    ("stereo,s","use ocam(stereo processing)")
    ("mono","mono processing(use default video file)")
    ("video",value<string>(),"mono processing using specified video file")
    ("cam","mono processing using webcam0")
    ("record,r","record output")
    ("recalibrate","recalibrate from image")
    ("cal","calibration test")
    ("thresh",value<double>(),"detection thresh hold")
    ("exposure",value<int>()->default_value(200),"camera exposure(ocam)")
    ("gain",value<int>()->default_value(100),"camera gain(ocam)")
    ("ae",value<bool>()->default_value(true), "auto exposure")
    ("centerdist",value<bool>()->default_value(false),"show senter distance")
    ("enable-detection",value<bool>()->default_value(true),"enable detection")
    ("gps-stat",value<bool>()->default_value(false),"show gps status")
    ("addr",value<string>(),"server ipv4 address")
    ("port",value<int>(),"server port")
    ("bias",value<int>(),"Yaw bias value");
    variables_map vm;

    try{
    store(parse_command_line(argc,argv,desc),vm);
    }
    catch(const boost::program_options::error &ex){
        cerr<<ex.what()<<endl;
        return 0;
    }
    cap = VideoCapture("../dataset_test/data/test_movie/test_video_0531.mp4");//기본 비디오


    if(vm.count("help"))
    {
        cout<<"Usage: ./main [options]\n";
        cout<<desc;
        return 0;
    }
    if(vm.count("bias"))
    {
        location::IMUData::YAW_BIAS = vm["bias"].as<int>();
    }
    if(vm.count("addr"))
    {
        udp_socket::server_addr_str = vm["addr"].as<string>();
    }
    if(vm.count("port"))
    {
        udp_socket::server_port = vm["port"].as<int>();
    }
    if(vm.count("gps-stat"))
    {
        location::print_gps_log = vm["gps-stat"].as<bool>();
    }
    if(vm.count("enable-detection"))
    {
        pipelineing::EnableDetection = vm["enable-detection"].as<bool>();
    }
    if(vm.count("ae"))
    {
        ocam_ae = vm["ae"].as<bool>();
    }
    if(vm.count("centerdist"))
    {
        pipelineing::ShowCenterDistance = vm["centerdist"].as<bool>();
    }
    if(vm.count("exopsure"))
    {
        ocam_exposure = vm["exposure"].as<int>();
    }
    if(vm.count("gain"))
    {
        ocam_gain = vm["gain"].as<int>();
    }
    if(vm.count("cal"))
    {
        init_ocam(&stereo_cam,&imu_reader);
        calitest();
        
        return 0;
    }

    if(vm.count("thresh"))
    {
        detection_threshold = vm["thresh"].as<double>();
    }
    if(vm.count("record"))
    {
        pipelineing::RECORD = 1;   
    }
    if(vm.count("recalibrate"))
    {
        RECALIBRATE = true;
    }
    if(vm.count("video"))
    {
        cap = VideoCapture(vm["video"].as<string>());
    }

    location::run_gps_thread();

    if(vm.count("stereo"))
    {
        run_ppline_stereo();
    }
    else if(vm.count("mono"))
    {
        run_ppline_mono();
    }

    else if(vm.count("cam"))
    {
        cap = VideoCapture(0);
        run_ppline_mono();
    }
    else{
        run_ppline_stereo();
    }
        
    
    return 0;
}


