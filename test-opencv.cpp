#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;

int main(int argv, char* argc)
{
    Mat A;
    A = Mat::zeros(100, 100, CV_8U);
    namedWindow("x", WINDOW_AUTOSIZE);
    imshow("x", A);
    waitKey(0);
    return 0;
}
