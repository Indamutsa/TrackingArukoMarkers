#include "opencv2/highgui.hpp"
#include "opencv2/aruco.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

#include <sstream>
#include <iostream>
#include <fstream>

using namespace std;
using namespace cv;

const float calibrationSquareDimension = 0.01905f; // meters
const float arucoSquareDimension = 0.1016f;
const Size chessboardDimensions = Size(6,9);
bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoeffients);
void createArucoMarkers();
void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners);
void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResult );
void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients );


void createArucoMarkers()
{
    Mat outputMarker;
    Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

    for(int i = 0; i < 50; i++)
    {
        aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
        ostringstream convert;

        string imageName = "4x4Marker_";
        convert << imageName << i << ".jpg";

        imwrite(convert.str(), outputMarker);
    }
}

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners)
{
    for(int i = 0; i < boardSize.height; i++)
    {
        for(int j = 0 ; j < boardSize.width; j++)
        {
            corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));
        }
    }
}

void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = false )
{
    for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++)
    {
        vector<Point2f> pointBuf;
        bool found = findChessboardCorners(*iter, Size(9,6), pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK);

        if(found){
            allFoundCorners.push_back(pointBuf);
        }

        if(showResults)
        {
            drawChessboardCorners(*iter, Size(9,6), pointBuf, found);
            imshow("Looking for corners", *iter);
            waitKey(0);
        }
    }

}

bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoeffients)
{
    ofstream outStream(name);
    if(outStream)
    {
        uint16_t rows = cameraMatrix.rows;
        uint16_t columns = cameraMatrix.cols;

        for(int r = 0; r < rows; r++)
        {
            for(int c = 0; c< columns; c++)
            {
                double value = cameraMatrix.at<double>(r,c);
                outStream << value << endl;
            }
        }

        rows = distanceCoeffients.rows;
        columns = distanceCoeffients.cols;

        for(int r = 0; r < rows; r++)
        {
            for(int c = 0; c< columns; c++)
            {
                double value = distanceCoeffients.at<double>(r,c);
                outStream << value << endl;
            }
        }

        outStream.close();
        return true;
    }

    return false;
}

void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distanceCoefficients )
{
    vector<vector<Point2f>> checkerboardSpacePoints;

    //False bcz we dont want to see the output
    getChessboardCorners(calibrationImages, checkerboardSpacePoints, false);

    vector<vector<Point3f>> worldSpaceCornerPoints(1);

    createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);
    worldSpaceCornerPoints.resize(checkerboardSpacePoints.size(), worldSpaceCornerPoints[0]);

    vector<Mat> rVectors, tVectors;

    distanceCoefficients = Mat::zeros(8,1,CV_64F);

    //THE MAGIC HAPPENS FROM HERE
    calibrateCamera(worldSpaceCornerPoints, checkerboardSpacePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
}



int main(int argv, char **argc)
{
    Mat frame;
    Mat drawToFrame;
    Mat distanceCoeffients;
    int framePerSecond = 20;

    Mat cameraMatrix = Mat::eye(3,3,CV_64F);
    vector<Mat> savedImages;

    vector<vector<Point2f>> markerCorners, rejectedCandidates;

    VideoCapture vid(0);

    if(!vid.isOpened())
    {
        return 0;
    }

    namedWindow("Webcam", 1000);

    while(true)
    {
       cout <<  vid.read(frame) ;
       if(!vid.read(frame))
            break;

        vector<Vec2f> foundPoints;
        bool found = false;

        found = findChessboardCorners(frame, chessboardDimensions, foundPoints, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK );
        frame.copyTo(drawToFrame);

        drawChessboardCorners(drawToFrame, chessboardDimensions, foundPoints, found);

        if(found)
            imshow("Webcam", drawToFrame);
        else
            imshow("Webcam", frame);

        char character = waitKey(1000 / framePerSecond);


        switch(character)
        {
            case' ':
            //saving the image
            if(found)
            {
                Mat temp;
                frame.copyTo(temp);
                savedImages.push_back(temp);
            }
                break;
            case 13:
                if(savedImages.size() > 15)
                {
                    cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix, distanceCoeffients);
                    saveCameraCalibration("Ourcameracalibration", cameraMatrix, distanceCoeffients);
                }
            //start calibration
                break;

            case 27:
                //exit
                return 0;
                break;
        }
        
    }
    return 0;
}

