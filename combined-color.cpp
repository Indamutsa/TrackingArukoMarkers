#include "opencv2/opencv.hpp"

using namespace cv;

int main(int argv, char ** argc)
{
    Mat original = imread("Me.jpg", CV_LOAD_IMAGE_COLOR);
    Mat splitChannels[3];
    Mat output;

    // Split the image into BGR
    split(original, splitChannels);
    
    imshow("B", splitChannels[0]);
    imshow("G", splitChannels[1]);
    imshow("R", splitChannels[2]);

    // Edit one column
    splitChannels[2] = Mat::zeros(splitChannels[2].size(), CV_8UC1);

    // Merge it  together
    merge(splitChannels, 3, output);

    imshow("Merged", output);
    waitKey();
}

