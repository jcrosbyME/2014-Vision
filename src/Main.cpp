#include "PiCam.hpp"
#include <iostream>

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
    cv::cvtColor(frame, frame, CV_RGB2BGR);
    //cv::Mat blurred(frame.rows, frame.cols);
    //cv::GaussianBlur(frame, blurred, cv::Size(9, 9), 2, 2);
    //cv::Canny(frame, frame, 20, 60, 3);
/*
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(blurred, circles, CV_HOUGH_GRADIENT, 2, frame.rows/8, 30, 100, 5, 100);

    std::cout << circles.size() << " circles!\n";
    for(size_t i = 0; i < circles.size(); i++) {
        cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        cv::circle(frame, center, 3, cv::Scalar(0), -1, 8, 0);
        cv::circle(frame, center, radius, cv::Scalar(0), 3, 8, 0);
    }
*/

    cv::imshow("RPi Cam", frame);
}

int main(int argc, char** argv) {
    cv::namedWindow("RPi Cam", cv::WINDOW_AUTOSIZE);
    PiCam cam(320, 240, &process_frame);
    vcos_sleep(100000);
    //cv::waitKey(0);
    return 0;
}
