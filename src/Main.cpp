#include "PiCam.hpp"
#include <iostream>
#include <cstdlib>

int n = 0;
unsigned char target = 0;

int hue = 0;
int range = 0;

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
    cv::cvtColor(frame, hsv, CV_RGB2HSV);
    //int min[] = {hue-range, 0, 0};
    //int max[] = {hue+range, 255, 255};
    cv::inRange(hsv, cv::Scalar(hue-range, 0, 0), cv::Scalar(hue+range, 255, 255), binary);
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
    cv::Mat blurred(frame.rows, frame.cols, CV_8UC3);
    cv::GaussianBlur(frame, blurred, cv::Size(9, 9), 2, 2);
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(blurred, circles, CV_HOUGH_GRADIENT, 2, frame.rows, 30, 100, 5, 100);

    std::cout << circles.size() << " circles!\n";
    for(size_t i = 0; i < circles.size(); ++i) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        cv::circle(frame, center, 3, cv::Scalar(0), -1, 8, 0);
        cv::circle(frame, center, radius, cv::Scalar(0), 3, 8, 0);
    }
    */

    cv::imshow("RPi Cam", binary);
}

void doNothing(int,void*) {}

int main(int argc, char** argv) {
    cv::namedWindow("RPi Cam", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("Hue", "RPi Cam", &hue, 255, &doNothing);
    cv::createTrackbar("Range", "RPi Cam", &range, 128, &doNothing);
    PiCam cam(320, 240, &process_frame);
    vcos_sleep(100000);
    std::cout << (n / 100.0) << " FPS\n";
    //cv::waitKey(0);
    return 0;
}
