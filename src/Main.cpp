#include "PiCam.hpp"
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <opencv2/imgproc/imgproc.hpp>
#include <unistd.h>
#include <termios.h>
#include <bcm2835.h>

int n = 0;
unsigned char target = 0;
float h_range[] = {0,256};
float s_range[] = {0,256};
float v_range[] = {0,256};
const float* ranges[] = {h_range, s_range};//, v_range};

int channels[] = {0,1};

int threshold = 410;

DISPMANX_DISPLAY_HANDLE_T display;
DISPMANX_ELEMENT_HANDLE_T element;
DISPMANX_RESOURCE_HANDLE_T display_resource;

/*
int hue = 0;
int range = 0;
int s_min = 0;
int s_max = 0;
int v_min = 0;
int v_max = 0;
*/

bool disp_rgb = true;

cv::MatND hist;
cv::MatND back_hist;
bool done_hist = false;
//cv::MatND hist;

/**
 * Process a single frame. 
 *
 * This does not have to be a global function; it could also
 * be put inline as a lambda function.
 *
 * @param frame the frame to be processed.
 */
void process_frame(cv::Mat frame) {
    ++n;
    cv::Mat_<cv::Vec3b> hsv(frame.size());
    cv::Mat_<unsigned char> binary(frame.size());
    cv::Mat_<unsigned char> back(frame.size());
    cv::Mat_<cv::Vec3b> disp(frame.size());
    cv::cvtColor(frame, hsv, CV_RGB2HSV_FULL);
    frame.copyTo(disp);
    //cv::cvtColor(frame, disp, CV_RGB2BGR);
    //int min[] = {hue-range, 0, 0};
    //int max[] = {hue+range, 255, 255};
    /*
    cv::inRange(hsv, cv::Scalar(hue-range, s_min, v_min), cv::Scalar(hue+range, s_max, v_max), binary);
    */
    int numP = 0;
    if(done_hist) {
        cv::calcBackProject(&hsv, 1, channels, hist, binary, ranges, 1);
        cv::calcBackProject(&hsv, 1, channels, back_hist, back, ranges, 1);
        //cv::inRange(binary, 150, 255, binary);
        //cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20, 20)));
        //cv::close(binary, binary, getStructuringElement(MORPH_ELLIPSE, cv::Size(2*10+1, 2*10+1), cv::Point(10, 10)));

        float centroid_x = 0.0f;
        float centroid_y = 0.0f;
        for(int j = 0; j < frame.rows; ++j) {
            for(int i = 0; i < frame.cols; ++i) {
                float weight = binary(j, i);
                float back_weight = back(j, i);
                if(weight > back_weight * (threshold / 500.0f)) {
                    binary(j, i) = 255;
                //if(weight > 200) {
                    numP += weight;
                    centroid_x += i * weight;
                    centroid_y += j * weight;
                } else {
                    binary(j, i) = 0;
                }
            }
        }
        centroid_x /= numP;
        centroid_y /= numP;

        if(!disp_rgb) {
            cv::cvtColor(binary, disp, CV_GRAY2RGB);
        }

        cv::circle(disp, cv::Point((int)centroid_x, (int)centroid_y), 5, cv::Scalar(255, 0, 0), -1);

        float r = sqrt(centroid_x*centroid_x + centroid_y*centroid_y);
    }

    cv::circle(disp, cv::Point(frame.size().width/2,frame.size().height/2), 25, cv::Scalar(0, 255, 0));

    
    VC_RECT_T dst_rect;
    vc_dispmanx_rect_set( &dst_rect, 0, 0, disp.cols, disp.rows);
    int ret = vc_dispmanx_resource_write_data(  display_resource,
                                            VC_IMAGE_RGB888,
                                            disp.cols*3,
                                            disp.data,
                                            &dst_rect);
    assert(ret==0);
    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start( 10 );
    vc_dispmanx_element_modified(update, element, &dst_rect);
    vc_dispmanx_update_submit_sync(update);


    //cv::imshow("RPi Cam Raw", disp);
    //cv::imshow("RPi Cam Proc", binary);
}

int _hist_;

PiCam* cam;

void doHistogram() {
    cv::Mat hsv(cam->frame.size(), CV_8UC3);
    cv::cvtColor(cam->frame, hsv, CV_RGB2HSV_FULL);
    int hbins = 256, sbins = 256;
    int histSize[] = {hbins,sbins};

    cv::Mat_<unsigned char> mask(cam->frame.size());

    for(int j = 0; j < mask.rows; ++j) {
        for(int i = 0; i < mask.cols; ++i) {
            float x = i - mask.cols/2;
            float y = mask.rows/2 - j;
            float d = sqrt(x*x + y*y);
            if(d > 25.0f) {
                mask(j,i) = 0;
            } else {
                mask(j,i) = 255;
            }
        }
    }

    calcHist(&hsv, 1, channels, mask, hist, 2, histSize, ranges, true, false);

    for(int j = 0; j < mask.rows; ++j) {
        for(int i = 0; i < mask.cols; ++i) {
            float x = i - mask.cols/2;
            float y = mask.rows/2 - j;
            float d = sqrt(x*x + y*y);
            if(d <= 30.0f) {
                mask(j,i) = 0;
            } else {
                mask(j,i) = 255;
            }
        }
    }

    calcHist(&hsv, 1, channels, mask, back_hist, 2, histSize, ranges, true, false);

    done_hist = true;
}

void doNothing(int,void*) {}

int main(int argc, char** argv) {
    bool keyboard = false;

    if(argc > 1) {
        for(int i = 1; i < argc; ++i) {
            if(strcmp(argv[i], "--keyboard") == 0) {
                keyboard = true;
                std::cout << "Changing terminal keyboard mode.\n";
            } else {
                std::cerr << "Invalid argument '" << argv[i] << "'\n";
                exit(1);
            }
        }
    }

    cam = new PiCam(160, 120, &process_frame);

    int ret;
    display = vc_dispmanx_display_open(0); // "window"

    unsigned int vc_image_ptr;

    display_resource = vc_dispmanx_resource_create(VC_IMAGE_RGB888, 160 | (160*3) << 16, 120, &vc_image_ptr);

    VC_DISPMANX_ALPHA_T alpha = { /*DISPMANX_FLAGS_ALPHA_FROM_SOURCE |*/ DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 
                             255, /*alpha 0->255*/
                             0 };
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    DISPMANX_MODEINFO_T info;
    ret = vc_dispmanx_display_get_info(display, &info);
    assert(ret==0);

    //vc_dispmanx_rect_set( &dst_rect, 0, 0, 160, 120);
    vc_dispmanx_rect_set( &src_rect, 0, 0, 160<<16, 120<<16);

    vc_dispmanx_rect_set( &dst_rect, 0,
                                     0,
                                     info.width,
                                     info.height );

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(10);
    assert(update != DISPMANX_NO_HANDLE);

    element = vc_dispmanx_element_add(  update,
                                        display,
                                        2000,               // layer
                                        &dst_rect,
                                        display_resource,
                                        &src_rect,
                                        DISPMANX_PROTECTION_NONE,
                                        &alpha,
                                        NULL,             // clamp
                                        //VC_IMAGE_ROT0 );
                                        DISPMANX_NO_ROTATE);
    ret = vc_dispmanx_update_submit_sync(update);
    assert(ret == 0);

    cam->start();

    struct termios term;
    struct termios init_term;

    if(keyboard) {
        tcgetattr(STDIN_FILENO, &term);
        init_term = term;
        term.c_lflag &= ~ICANON;
        term.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &term);
    }

    int key;

    bcm2835_init();
    bcm2835_gpio_fsel(RPI_V2_GPIO_P1_03, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_ren(RPI_V2_GPIO_P1_03);

    while(true) {
        // Get all input from stdin
        while((key=std::getchar()) != EOF) {
            switch(key) {
            case 'q':
            case 'Q':
                goto done;
            case 'x':
            case 'X':
                disp_rgb = !disp_rgb;
            case 'c':
            case 'C':
                doHistogram();
            }
        }

        if(bcm2835_gpio_eds(RPI_V2_GPIO_P1_03)) {
            std::cout << "Rising edge detected on GPIO pin P1-3\n";
            bcm2835_gpio_set_eds(RPI_V2_GPIO_P1_03);
        }

        usleep(50000);
    }

done:

    if(keyboard) {
        tcsetattr(STDIN_FILENO, TCSANOW, &init_term);
    }

    bcm2835_gpio_clr_ren(RPI_V2_GPIO_P1_03);
    bcm2835_close();

    return 0;
}
