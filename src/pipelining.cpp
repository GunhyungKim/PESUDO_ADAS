#include "pipelining.hpp"
#include "drawing.hpp"
#include "Location/web_socket_server.hpp"
#include "utils.hpp"
#include "semaphore.h"
#include "stereo.hpp"
#include "Location/udp_socket.hpp"
#include <stdlib.h>


namespace pipelineing
{
    bool RECORD = false;
    bool ShowCenterDistance = false;
    bool EnableDetection=true;
    using namespace cv;
    using namespace std;
    bool (*get_img_mono)(cv::Mat&);
    bool (*get_img_stereo)(cv::Mat&, cv::Mat&);
    bool (*get_imu_data)(location::IMUData& imu);
    bool (*get_gps_data)(location::GPSData& gps);
    #define BUFF_SIZE 5

    struct ppline_data
    {
        image darknet_img;
        Mat imL,imR,main_img;
        vector<detection_object> objs;
        stereo_match stereo;
        location::GPSData gps;
        location::IMUData imu;
        proc_time elapsed_time[4];
        void write_all_fps(Mat& im)
        {
            
            elapsed_time[0].mark_image(im,0,470-20,"Matching",0.8);
            elapsed_time[2].mark_image(im,0,470,"Detection",0.8);
        }
        void print_sensor_data(Mat& im)
        {
            putTextWithBG(im,imu.tostring(),5,15,0,5,0.5);
            putTextWithBG(im,gps.tostring(),5,30,0,5,0.5);
        }
    };

    bool use_stereo = false;
    static ppline_data buff[BUFF_SIZE];
    static unsigned int buff_index=0;
    static bool fetch_result=false;
    #define GET_BUFF(offset) buff[(buff_index+offset)%BUFF_SIZE]


    //semaphore - thread syncronization
    sem_t thread_sem[5];
    sem_t& sem_join = thread_sem[4];
    sem_t& sem_fetch_start = thread_sem[0];
    sem_t& sem_dnet_fetch_start = thread_sem[1];
    sem_t& sem_detect_start = thread_sem[2];
    sem_t& sem_show_start = thread_sem[3];
    

    void* img_fetch_mono(void *arg)
    {
        while(1)
        {
            sem_wait(&sem_fetch_start);
            ppline_data& d = GET_BUFF(4);
            d.elapsed_time[0].begin();
            Mat& img = d.main_img;
            fetch_result = get_img_mono(img);


            if(get_gps_data)//check whether get_gps_data is defined
            {
                while(!get_gps_data(d.gps)){}
            }
            if(get_imu_data)//check whether get_imu_data is defined
            {
                while(!get_imu_data(d.imu)){}
            }
            d.elapsed_time[0].end();
            sem_post(&sem_join);
        }
        
    }

    void* img_fetch_stereo(void *arg)
    {
        fetch_result = true;
        while(1)
        {
            sem_wait(&sem_fetch_start);
            //CHECK_FPS_BEGIN("a");
            ppline_data& d = GET_BUFF(4);
            d.elapsed_time[0].begin();
            while(!get_img_stereo(d.imL,d.imR))//request frame repeatedly
            {}
            
            d.stereo = stereo_match(d.imL.size());
            d.stereo.calc_disp(d.imL,d.imR);
            d.main_img = d.stereo.imgU1;
            
            if(get_gps_data)
            {
                while(!get_gps_data(d.gps)){}
            }
            if(get_imu_data)
            {
                while(!get_imu_data(d.imu)){}
            }
            //CHECK_FPS_END;
            d.elapsed_time[0].end();
            sem_post(&sem_join);
        }
        
    }

    

    void* dnet_img_fetch(void *arg)
    {
        while(1){
            sem_wait(&sem_dnet_fetch_start);
            //CHECK_FPS_BEGIN("11");
            ppline_data& d = GET_BUFF(3);
            d.elapsed_time[1].begin();
            Mat& img = d.main_img;
            Mat resized;
            if(img.empty())
            {
                sem_post(&sem_join);
                continue;
            }
            resize(img,resized,Size(0,0),1,1,1);
            mat_into_image(&resized,d.darknet_img);
            rgbgr_image(d.darknet_img);
            d.elapsed_time[1].end();
            sem_post(&sem_join);
            //CHECK_FPS_END;
        }
    }

    
    void* detect(void *arg)
    {
        while(1){

            sem_wait(&sem_detect_start);
            ppline_data& d = GET_BUFF(2);
            d.elapsed_time[2].begin();
            Mat& marked_img = d.main_img;
            vector<detection_object> r;
            image& im =d.darknet_img;
            if(im.data==0)
            {
                sem_post(&sem_join);
                continue;
            }
                
            //CHECK_FPS_BEGIN("11");
            if(EnableDetection)
            {
                r = detect_objects(im);
            }
            else
            {
            free_image(im);
            }
            d.objs = r;
            d.elapsed_time[2].end();
            //CHECK_FPS_END;
            sem_post(&sem_join);
        }
    }

    void onClick(int event,int x,int y, int flags,void* userdata)
    {
        stereo_match* sm =(stereo_match*) userdata;
        if(sm&&event==EVENT_LBUTTONDOWN)
        {
            double dist = sm->distance(x,y);
            cout<<"dist = "<<dist<<endl;
        }

        if(!sm&&event==EVENT_LBUTTONDBLCLK)
        {
            exit(0);
        }
    }
    
    

    VideoWriter* v_writer_disp;
    VideoWriter* v_writer_detection;
    void* show(void *arg)
    {
        
        
        while(1)
        {
            sem_wait(&sem_show_start);
            ppline_data& d = GET_BUFF(1);
            d.elapsed_time[3].begin();
            Mat& im = d.main_img;

            vector<detection_object>& r = d.objs;
            vector<location::ObjectLocation> loc_arr;
            location::ObjectLocation camLoc;
            camLoc.lat = d.gps.latitude;
            camLoc.lng = d.gps.longitude;
            camLoc.azimuth = d.imu.yaw;
            camLoc.name = "normal";
            loc_arr.push_back(camLoc);
            double min_dist = 987654321;
            for(int i=0;i<r.size();i++)/////////calculate location
            {
                if(use_stereo)
                {
                    r[i].distance = d.stereo.distance_relative(r[i].relative_x(),r[i].relative_y());
                    min_dist = std::min(min_dist,r[i].distance);
                }
                else
                    r[i].distance = 20; //모노사용시 고정거리값 사용

                location::PixelData cam(r[i].img_width,r[i].img_height,r[i].x,r[i].y,r[i].distance,r[i].class_name);
                location::ObjectLocation loc = location::calculateLocation(d.imu,d.gps,cam);
                loc_arr.push_back(loc);
            }/////////////////////////////////
            ///set car state//////
            /////////////////////////
            if(min_dist<=1)
                loc_arr[0].name = "crashed";
            else if(min_dist<=2)
                loc_arr[0].name = "warning";
            else
                loc_arr[0].name = "normal";

            ///send message
            string loc_json = toJson(loc_arr);
            socket_server::set_message(loc_json);
            udp_socket::send(loc_json);


            /////display&recording
            if(!im.empty())
            {
                
                draw_detections(r,im);
                d.write_all_fps(im);
                d.print_sensor_data(im);
                if(ShowCenterDistance)
                {
                    char buff[30];
                    sprintf(buff,"%.2fm",d.stereo.distance_relative(.5,.5));
                    putTextWithBG(im,buff,320,240);
                    putMarker(im,320,240);
                    
                }
                namedWindow("view",WINDOW_NORMAL);
                setWindowProperty("view",CV_WND_PROP_FULLSCREEN,CV_WINDOW_FULLSCREEN);
                setMouseCallback("view",onClick,NULL);
                imshow("view",im);
                if(RECORD){
                        if(v_writer_detection==nullptr)
                        {
                            v_writer_detection = new VideoWriter();
                            v_writer_detection->open("detection"+time_stamp()+".avi",CV_FOURCC('X','2','6','4'),40,im.size(),true);
                        }
                        v_writer_detection->write(im);
                }
                if(use_stereo)
                {
                    namedWindow("disp");
                    setMouseCallback("disp",onClick,&d.stereo);
                    imshow("disp",d.stereo.nomalized_disp);
                    //imshow("LU",d.stereo.imgU1);
                    //imshow("RU",d.stereo.imgU2);
                    //imshow("L",d.imL);
                    //imshow("R",d.imR);
                    
                        if(RECORD)
                        {
                            if(v_writer_disp==nullptr)
                            {
                                v_writer_disp = new VideoWriter();
                                v_writer_disp->open("disp"+time_stamp()+".avi",CV_FOURCC('X','2','6','4'),40,d.stereo.nomalized_disp.size(),false);
                                
                            }
                            v_writer_disp->write(d.stereo.nomalized_disp);
                        }
                }  

                int key =  cvWaitKey(10);
                if(key>=0)
                    cout<<"key = "<<key<<endl;
                if(key==27||key=='q')
                {
                    if(v_writer_detection)
                    {
                        v_writer_detection->release();
                        v_writer_detection = NULL;
                    }
                        
                    exit(0);    
                }
            }
            d.elapsed_time[3].end();
            sem_post(&sem_join);
        }
        
    }
    
    void run()
    {
        for(int i=0;i<5;i++)
        {
            if(sem_init(&thread_sem[i],0,0))perror("Semaphore creation error");
        }    
        
        void* (*fetch_func)(void*) = use_stereo?img_fetch_stereo:img_fetch_mono;

        pthread_t thread_fetch,thread_dnet_fetch,thread_detect,thread_show;
        if(pthread_create(&thread_fetch,NULL,fetch_func,NULL))perror("Thread creation error");
        if(pthread_create(&thread_show,NULL,show,NULL))perror("Thread creation error");
        if(pthread_create(&thread_fetch,NULL,dnet_img_fetch,NULL))perror("Thread creation error");
        if(pthread_create(&thread_detect,NULL,detect,NULL))perror("Thread creation error");
        
        do
        {
            proc_time t;
            t.begin();

            for(int i=0;i<4;i++)
            {
                sem_post(&thread_sem[i]);
            }

            for(int i=0;i<4;i++)
            {
                sem_wait(&sem_join);
            }

            t.end();
            //Mat& marked_img = GET_BUFF(2).main_img;
            //t.mark_image(marked_img,10,30,"",1);

            buff_index++;

        }while(fetch_result);
        cout<<"fetch finished."<<endl;   

    }
}