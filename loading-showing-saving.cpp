#include "opencv2/opencv.hpp"

using namespace cv;

int main(int argv, char ** argc)
{
    Mat testColor = imread("Me.jpg", CV_LOAD_IMAGE_COLOR);
    Mat testGray = imread("Me.jpg", CV_LOAD_IMAGE_GRAYSCALE);

    imwrite("outputGray.jpg", testGray);
    
    imshow("color", testColor);
    imshow("gray", testGray);
    waitKey();
}

