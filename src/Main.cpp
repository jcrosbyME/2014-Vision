#include "PiCam.hpp"
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <opencv2/imgproc/imgproc.hpp>

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

cv::MatND hist;
cv::MatND back_hist;
bool done_hist = false;
//cv::MatND hist;

/**
 * Process a single frame. 
 *
 * Currently this converts the RGB image from the camera
 * to the BGR format expected by OpenCV, then displays
 * the image using OpenCV. In the future, image processing
 * will happen here.
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

        cv::circle(disp, cv::Point((int)centroid_x, (int)centroid_y), 5, cv::Scalar(255, 0, 0), -1);

        float r = sqrt(centroid_x*centroid_x + centroid_y*centroid_y);
    }
    /*
    cv::Mat hue(frame.size(), CV_8U);
    const int fromto[] = {0, 0};
    cv::mixChannels(&frame, 1, &hue, 1, fromto, 1);
    cv::Mat binary(frame.size(), CV_8U);
    cv::inRange(frame, 0, 50, binary);
    */

    

    //cv::Mat_<unsigned char> binary = cv::Mat_<unsigned char>::zeros(frame.rows, frame.cols);
    /*
    for(int j = 0; j < frame.rows; ++j) {
        for(int i = 0; i < frame.cols; ++i) {
            int r = frame.at<cv::Vec3b>(j, i)[0];
            int b = frame.at<cv::Vec3b>(j, i)[1];
            int g = frame.at<cv::Vec3b>(j, i)[2];
            binary(j, i) = r - r*(b + g)/(2*255);
            float r = frame.at<cv::Vec3b>(j, i)[0]/255.0f;
            float b = frame.at<cv::Vec3b>(j, i)[1]/255.0f;
            float g = frame.at<cv::Vec3b>(j, i)[2]/255.0f;
            float h = 0;

            if(r>=g) {
                if(g>=b) {
                    h = 60*(2-(g-b)/(r-b));
                } else {
                    60*(4-(g-r)/(b-r));
                }
            } else {
                
            }

            if(r>=g && g>=b)        h = 60*(2-(g-b)/(r-b));
            else if(g>r && r>=b)    h = 60*(2-(r-b)/(g-b));
            else if(g>=b && b>r)    h = 60*(2+(b-r)/(g-r));
            else if(b>g && g>r)     h = 60*(4-(g-r)/(b-r));
            else if(b>r && r>=g)    h = 60*(4+(r-g)/(b-g));
            else if(r>=b && b>g)    h = 60*(6-(b-g)/(r-g));

            h *= 255.0f/360.0f;

            //binary(j, i) = target - (unsigned char)h;
            if( (target -(unsigned char)h) < 10 ||  ((unsigned char)h - target) < 10) {
                binary(j, i) = 255;
            } else {
                binary(j, i) = 0;
            }
        }
    }
    */

    /*
    cv::Mat blurred(binary.size(), CV_8U);
    cv::GaussianBlur(binary, blurred, cv::Size(9, 9), 2, 2);
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(blurred, circles, CV_HOUGH_GRADIENT, 2, frame.rows, 30, 100, 5, 100);

    for(size_t i = 0; i < circles.size(); ++i) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        cv::circle(disp, center, 5, cv::Scalar(0, 255, 0), -1, 8, 0);
        //cv::circle(frame, center, radius, cv::Scalar(0), 3, 8, 0);
    }


    */
    cv::circle(disp, cv::Point(frame.size().width/2,frame.size().height/2), 25, cv::Scalar(0, 255, 0));

    
    VC_RECT_T dst_rect;
    vc_dispmanx_rect_set( &dst_rect, 0, 0, 160, 120);
    int ret = vc_dispmanx_resource_write_data(  display_resource,
                                            VC_IMAGE_RGB888,
                                            160*3,
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

void doHistogram(int,void*) {
    done_hist = true;
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
}

void doNothing(int,void*) {}

int main(int argc, char** argv) {
    cam = new PiCam(160, 120, &process_frame);
    std::cout << "Initialized camera" << std::endl;

    int ret;
    display = vc_dispmanx_display_open(0); // "window"
    std::cout << "Opened display" << std::endl;

    unsigned int vc_image_ptr;

    display_resource = vc_dispmanx_resource_create(VC_IMAGE_RGB888, 160 | (160*3) << 16, 120, &vc_image_ptr);
    std::cout << "Created resource" << std::endl;

    VC_DISPMANX_ALPHA_T alpha = { /*DISPMANX_FLAGS_ALPHA_FROM_SOURCE |*/ DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS, 
                             255, /*alpha 0->255*/
                             0 };
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    DISPMANX_MODEINFO_T info;
    ret = vc_dispmanx_display_get_info(display, &info);
    assert(ret==0);
    std::cout << "Got display info" << std::endl;

    //vc_dispmanx_rect_set( &dst_rect, 0, 0, 160, 120);
    vc_dispmanx_rect_set( &src_rect, 0, 0, 160<<16, 120<<16);

    vc_dispmanx_rect_set( &dst_rect, ( info.width - 160 ) / 2,
                                     ( info.height - 120 ) / 2,
                                     160,
                                     120 );

    DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(10);
    assert(update != DISPMANX_NO_HANDLE);
    std::cout << "Started update" << std::endl;

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
    std::cout << "Added element" << std::endl;
    ret = vc_dispmanx_update_submit_sync(update);
    assert(ret == 0);
    std::cout << "Finished update" << std::endl;



    //cv::namedWindow("RPi Cam Raw", cv::WINDOW_AUTOSIZE);
    //cv::namedWindow("RPi Cam Proc", cv::WINDOW_AUTOSIZE);

    //hist = cv::Mat(240, 320, CV_8UC3, 0);

    /*
    cv::createTrackbar("Hue", "RPi Cam", &hue, 255, &doNothing);
    cv::createTrackbar("Range", "RPi Cam", &range, 128, &doNothing);
    cv::createTrackbar("Min Sat", "RPi Cam", &s_min, 255, &doNothing);
    cv::createTrackbar("Max Sat", "RPi Cam", &s_max, 255, &doNothing);
    cv::createTrackbar("Min Value", "RPi Cam", &v_min, 255, &doNothing);
    cv::createTrackbar("Max Value", "RPi Cam", &v_max, 255, &doNothing);
    */
    //cv::createTrackbar("Histogram", "RPi Cam Raw", &_hist_, 1, &doHistogram);
    //cv::createTrackbar("Threshold", "RPi Cam Raw", &threshold, 1000, &doNothing);
    std::cout << "Setup dispmanx" << std::endl;

    cam->start();
    //vcos_sleep(100000);
    //std::cout << (n / 100.0) << " FPS\n";
    //cv::waitKey(0);
    return 0;
}
