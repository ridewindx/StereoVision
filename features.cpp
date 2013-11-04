#include "features.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"

using namespace cv;
using namespace std;

namespace features {

void testHarrisCorner()
{
    string rootDirectory = "E:/build-StereoVision-Desktop_Qt_5_1_0_MSVC2010_32bit_OpenGL-Debug/";
    string filename = rootDirectory + "Harris_Detector_Original_Image.jpg";


    int thresh = 190;
    //int max_thresh = 255;

    /// Load source image and convert it to gray
    Mat src, src_gray;
    src = imread(filename, IMREAD_COLOR);
    cvtColor(src, src_gray, CV_BGR2GRAY); // BGR --> GRAY
    filename = rootDirectory + "Harris_Detector_Original_Image_grayed.jpg";
    imwrite(filename, src_gray);

    /// Create a window and a trackbar
    //char* source_window = "Source image";
    //namedWindow(source_window, CV_WINDOW_AUTOSIZE );
    //createTrackbar("Threshold: ", source_window, &thresh, max_thresh, cornerHarris_demo);
    //imshow(source_window, src);


    Mat dst, dst_norm, dst_norm_scaled;
    dst = Mat::zeros(src.size(), CV_32FC1);

    /// Detector parameters
    int blockSize = 3;
    int apertureSize = 3;
    double k = 0.04;

    /// Detecting corners
    cornerHarris(src_gray, dst, blockSize, apertureSize, k, BORDER_DEFAULT);

    filename = rootDirectory + "Harris_Detector_Original_Image_harris.jpg";
    imwrite(filename, dst);

    /// Normalizing
    normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
    filename = rootDirectory + "Harris_Detector_Original_Image_normal.jpg";
    imwrite(filename, dst_norm);
    //convertScaleAbs(dst_norm, dst_norm_scaled );
    //filename = rootDirectory + "Harris_Detector_Original_Image_abs.jpg";
    //imwrite(filename, dst_norm_scaled);

    /// Drawing a circle around corners
    for(int j = 0; j < dst_norm.rows; j++) {
        for(int i = 0; i < dst_norm.cols; i++) {
            if((int) dst_norm.at<float>(j,i) > thresh) {
                circle(dst_norm, Point( i, j), 5,  Scalar(0), 2, 8, 0);
            }
        }
    }
    /// Showing the result
    //char* corners_window = "Corners detected";
    //namedWindow(corners_window, CV_WINDOW_AUTOSIZE );
    //imshow(corners_window, dst_norm_scaled );
    filename = rootDirectory + "Harris_Detector_Original_Image_detected.jpg";
    imwrite(filename, dst_norm);
}


}
