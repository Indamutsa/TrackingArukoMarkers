#include "opencv2/opencv.hpp"
#include <stdint.h>


using namespace cv;
using namespace std;

int main(int argv, char** argc)
{
    Mat original = imread("Me.jpg", CV_LOAD_IMAGE_COLOR);
    Mat modified = imread("Me.jpg", CV_LOAD_IMAGE_COLOR);
    
    for (int r =  0; r< modified.rows; r++)
    {
        for (int c = 0; c < modified.cols; c++ )
        {
            // Removed the blue
            //modified.at<cv::Vec3b>(r,c)[0] = modified.at<Vec3b>(r,c)[0] * 0 ;

            // Removed the green
            //modified.at<cv::Vec3b>(r,c)[1] = modified.at<Vec3b>(r,c)[1] * 0 ;
            
            // Removed the red
            modified.at<cv::Vec3b>(r,c)[2] = modified.at<Vec3b>(r,c)[2] * 0 ;
        }
    }

    imshow("Original", original);
    imshow("Modified", modified);
    waitKey();
}
