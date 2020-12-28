#include "utils.hpp"

double what_time_is_it_now()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void proc_time::begin()
{
        time_begin = what_time_is_it_now();
};
void proc_time::end()
{
        time_end = what_time_is_it_now();
}
double proc_time::diff()
{
    return time_end-time_begin;
}
double proc_time::fps()
{
        return 1/diff();
}
void proc_time::mark_image(cv::Mat& im,int x,int y,std::string name,double font_scale=1)
{
    putTextWithBG(im,name+" "+fps_string(),x,y,0,5,font_scale);
}
std::string proc_time::fps_string()
{
    char buff[100];
    sprintf(buff,"FPS: %.0f",fps());
    return std::string(buff);
}

std::string time_stamp()
{
    char buffer[80];
    time_t rawtime;
    time(&rawtime);
    tm *timieinfo = localtime(&rawtime);
    strftime(buffer,sizeof(buffer),"%d-%m-%Y %H:%M:%S",timieinfo);
    std::string str(buffer);
    return str;
}