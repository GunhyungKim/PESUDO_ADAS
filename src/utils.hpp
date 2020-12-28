#include<ctime>
#include <sys/time.h>
#include<iostream>
#include<string>
#include "drawing.hpp"
#pragma once
/*#define CHECK_FPS_BEGIN(x)  clock_t fps_start,fps_end; fps_start = clock(); std::string fps_name =x;
#define CHECK_FPS_END    fps_end = clock(); \
        double fps_dif = (fps_end-fps_start)/(double)CLOCKS_PER_SEC; \
        std::string fps_str = fps_name+" FPS: "+std::to_string(1/fps_dif) +"\n"; \
        std::cout<<fps_str;

#define CHECK_FPS_IMG_END(img,x,y) fps_end = clock(); \
        double fps_dif = (fps_end-fps_start)/(double)CLOCKS_PER_SEC; \
        std::string fps_str = fps_name+" FPS: "+std::to_string(1/fps_dif); \
        putTextWithBG(img,fps_str,x,y);
*/
double what_time_is_it_now();
#define CHECK_FPS_BEGIN(x)  double fps_start,fps_end,fps_dif; std::string fps_str;\
        fps_start = what_time_is_it_now(); std::string fps_name =x;

#define CHECK_FPS_END    fps_end =  what_time_is_it_now(); \
        fps_dif = (fps_end-fps_start); \
        fps_str = fps_name+" FPS: "+std::to_string(1/fps_dif) +"\n"; \
        std::cout<<fps_str;

#define CHECK_FPS_IMG_END(img,x,y) fps_end =  what_time_is_it_now(); \
        fps_dif = (fps_end-fps_start); \
        fps_str = fps_name+" FPS: "+std::to_string(1/fps_dif); \
        putTextWithBG(img,fps_str,x,y);

struct proc_time
{
        double time_begin;
        double time_end;
        void begin();
        void end();
        double fps();
        double diff();
        std::string fps_string();
        void mark_image(cv::Mat& im,int x,int y,std::string name,double font_scale);
};
std::string time_stamp();