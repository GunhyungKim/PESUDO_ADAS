#include "detect.hpp"
#include "utils.hpp"
using namespace std;
using namespace cv;

static network *net;

double detection_object::relative_x()
{
    return x/(double)img_width;
}
double detection_object::relative_y()
{
    return y/(double)img_height;
}


void mat_into_image(Mat* src,image& im)
{
    
    unsigned char *data = (unsigned char *)src->data;
    int h = src->rows;
    int w = src->cols;
    int c = src->channels();
    im = make_image(w,h,3);
    int step = src->step;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
}

void img_into_mat(Mat* out,image& im)
{
    unsigned char *data = (unsigned char *)out->data;
    int h = out->rows;
    int w = out->cols;
    int c = out->channels();
    int step = out->step;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                data[i*step + j*c + k] = im.data[k*w*h + i*w + j]*255;
            }
        }
    }
}

static bool initialized =false;
static char* datacfg = "../dataset_test/cfg/kym_test.data";
static char* netcfg="../dataset_test/cfg/yolov3-tiny-kym-run.cfg";
static char* weights="../darknet/backup1/yolov3-tiny-kym-train.backup";
/*static char* datacfg = "../darknet/cfg/coco.data";
static char* netcfg="../darknet/cfg/yolov3-tiny.cfg";
static char* weights="../darknet/yolov3-tiny.weights";*/



static int classes;
static char *name_list;
static char **names;



vector<detection_object> to_obj(int img_width,int img_height, detection *dets, int num,char **names,int classes)
{
    vector<detection_object> arr;
    for(int i=0;i<num;i++)
    {
        char labelstr[4096]={0};
        int cls = -1;
        for(int j=0;j<classes;j++)
        {
            if(dets[i].prob[j]> detection_threshold){
                if(cls<0) {
                    strcat(labelstr,names[j]);
                    cls = j;
                }else{
                    strcat(labelstr,",");
                    strcat(labelstr,names[j]);
                    
                }
                //printf("%s: %.0f%%\n", names[j], dets[i].prob[j]*100);
            }
        }
        if(cls >= 0){
            int width = img_height * .006;

            box b = dets[i].bbox;
            //printf("%f %f %f %f\n", b.x, b.y, b.w, b.h);

            int left  = (b.x-b.w/2.)*img_width;
            int right = (b.x+b.w/2.)*img_width;
            int top   = (b.y-b.h/2.)*img_height;
            int bot   = (b.y+b.h/2.)*img_height;

            if(left < 0) left = 0;
            if(right > img_width-1) right = img_width-1;
            if(top < 0) top = 0;
            if(bot > img_height-1) bot = img_height-1;

            detection_object obj;
            obj.class_name = string(labelstr);
            obj.height = bot-top;
            obj.x =(left+right)/2;
            obj.width = right -left;
            obj.y = (top+bot)/2;
            obj.probability = dets[i].prob[cls];
            obj.img_width = img_width;
            obj.img_height = img_height;
            arr.push_back(obj);
        }
        
    }
    return arr;
}
 int detection_object::left()
{
    return x - width/2;
}
 int detection_object::top()
{
    return y - height/2;
}
Rect detection_object::get_bbox()
{
    Rect rect(left(),top(),width,height);
    return rect;
}


double detection_threshold = .5;
            


int init_network()
{
    initialized = true;
    net = load_network(netcfg,weights,0);
    set_batch_network(net,1);

    
    ::list *options = read_data_cfg(datacfg);
    classes = option_find_int(options, "classes", 20);
    name_list = option_find_str(options, "names", "data/names.list");
    names = get_labels(name_list);   

}
float* network_predict_image(image& im)
{
    return network_predict_image(net,im);
    
}
vector<detection_object> detect_objects(image& input)
{
    vector<detection_object> r_val;
    //CHECK_FPS_BEGIN("detect");
    network_predict_image(input);
    
    //CHECK_FPS_END;
    int nboxes;
    detection *dets = get_network_boxes(net,input.w,input.h,detection_threshold,.5,0,1,&nboxes);
    do_nms_obj(dets,nboxes,classes,.45f);
    
    r_val = to_obj(input.w,input.h,dets,nboxes,names,classes);
    free_detections(dets,nboxes);
    free_image(input);

    return r_val;
}
vector<detection_object> detect_objects(Mat& input,Mat& resized)
{
    if(!initialized)
        init_network();
    
    //CHECK_FPS_BEGIN("img prepare");
    
    
    resize(input,resized,Size(0,0),1,1,1);
    image img;
    mat_into_image(&resized,img); 
    
    rgbgr_image(img);
    //CHECK_FPS_IMG_END(resized,10,80);
    vector<detection_object> r = detect_objects(img);
    return r;
} 



void test_detection()
{
    if(!initialized)
        init_network();

    VideoCapture cap("../졸프영상파일/Test03.mp4");
    if(!cap.isOpened())
        return;
        
    namedWindow("view",1);
    Mat frame,resized;
    while(cap.read(frame))
    {
        vector<detection_object> r = detect_objects(frame,resized);
        for(int i=0;i<r.size();i++)
        {
            r[i].print();
        }
        imshow("view",resized);
    }        

}

void detection_object::print()
{
    printf("x:%4d y:%4d width:%4d height:%4d prob:%5f class:%s\n",x,y,width,height,probability, class_name.c_str());
}

/*
int main()
{
    test_detection();
    return 0;
}*/
