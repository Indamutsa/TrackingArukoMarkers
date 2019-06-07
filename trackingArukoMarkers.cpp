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
void cameraCalibrationProcess(Mat& cameraMatrix, Mat& distanceCoefficients);
bool loadCameraCalibration(string name, Mat& cameraMatrix, Mat& distanceCoeffients);
int startWebCameraMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions);

// This function will print 50 aruco markers
void createArucoMarkers()
{
    // We initialize the output image and the markers ( 50 different markers)
    Mat outputMarker;
    Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

    for(int i = 0; i < 50; i++)
    {
        // We will these markers on the image
        /*The first parameter is the Dictionary object previously created.
          The second parameter is the marker id, in this case the marker 23 of the dictionary DICT_4X4_. Note that each dictionary is composed by a different number of markers. 
          In this case, the valid ids go from 0 to 49. 
          The third parameter is the size of output image
          The forth parameter is the output image.
          Finally, the last parameter is an optional parameter to specify the width of the marker black border.
         */
        aruco::drawMarker(markerDictionary, i, 500, outputMarker, 1);
        ostringstream convert;

        string imageName = "4x4Marker_";
        convert << imageName << i << ".jpg";
        
        // We write it to the image
        imwrite(convert.str(), outputMarker);
    }
}

//This function  create known board position in 3D
void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners)
{
    for(int i = 0; i < boardSize.height; i++)
    {
        for(int j = 0 ; j < boardSize.width; j++)
        {
            // The corners detected are pushed into corners
            corners.push_back(Point3f(j * squareEdgeLength, i * squareEdgeLength, 0.0f));
        }
    }
}

//We retrieve the corners from the chessboard
void getChessboardCorners(vector<Mat> images, vector<vector<Point2f>>& allFoundCorners, bool showResults = false )
{
    for (vector<Mat>::iterator iter = images.begin(); iter != images.end(); iter++)
    {
        vector<Point2f> pointBuf;
        bool found = findChessboardCorners(*iter, Size(9,6), pointBuf, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK);

        if(found){
            //We save the corner in allfoundconers data structure
            allFoundCorners.push_back(pointBuf);
        }

        if(showResults)
        {
            // We can draw the corners and show them
            drawChessboardCorners(*iter, Size(9,6), pointBuf, found);
            imshow("Looking for corners", *iter);
            waitKey(0);
        }
    }

}

// This function will populate cameraMatrix which carries our camera model as well as distance coefficients
bool saveCameraCalibration(string name, Mat cameraMatrix, Mat distanceCoeffients)
{
    ofstream outStream(name);
    if(outStream)
    {
        uint16_t rows = cameraMatrix.rows;
        uint16_t columns = cameraMatrix.cols;

        outStream << rows << endl;
        outStream << columns << endl;

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

        outStream << rows << endl;
        outStream << columns << endl;

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
    vector<vector<Point2f>> checkerboardImageSpacePoints;

    //false bcz we dont want to see the output
    getChessboardCorners(calibrationImages, checkerboardImageSpacePoints, false);

    // 3D coordinates
    vector<vector<Point3f>> worldSpaceCornerPoints(1);

    // Extract the 3D coordinates relative to the board size
    createKnownBoardPosition(boardSize, squareEdgeLength, worldSpaceCornerPoints[0]);

    // We resize the world space corners
    worldSpaceCornerPoints.resize(checkerboardImageSpacePoints.size(), worldSpaceCornerPoints[0]);

    vector<Mat> rVectors, tVectors;

    distanceCoefficients = Mat::zeros(8,1,CV_64F);

    //We calibrate the camera
    calibrateCamera(worldSpaceCornerPoints, checkerboardImageSpacePoints, boardSize, cameraMatrix, distanceCoefficients, rVectors, tVectors);
}

// Track aruco markers
int startWebCameraMonitoring(const Mat& cameraMatrix, const Mat& distanceCoefficients, float arucoSquareDimensions)
{
    Mat frame;

    vector<int> markerIds;
    vector<vector<Point2f>> markerCorners, rejectedCandidates;


    //aruco::DetectorParameters parameters;
    cv::Ptr<cv::aruco::DetectorParameters> parameters;
    Ptr<aruco::Dictionary> markerDictionary = aruco::getPredefinedDictionary(aruco::PREDEFINED_DICTIONARY_NAME::DICT_4X4_50);

    // We capture the video
    VideoCapture vid(0);

    if(!vid.isOpened())
    {
        return -1;
    }

    namedWindow("Webcam", 1000);

    vector<Vec3d> rotationVectors, translationVectors;

    while (true)
    {
        if(!vid.read(frame))
            break;
        
        // Detect the markers and estimate the pose of given marker
        aruco::detectMarkers(frame, markerDictionary, markerCorners, markerIds );
        aruco::estimatePoseSingleMarkers(markerCorners, arucoSquareDimensions, cameraMatrix, distanceCoefficients, rotationVectors, translationVectors);

        for(int i = 0; i < markerIds.size(); i++)
        {
            aruco::drawAxis(frame, cameraMatrix, distanceCoefficients, rotationVectors[i], translationVectors[i], 0.1f);
        }

        imshow("Webcam", frame);

        if(waitKey(30) >= 0) break;
    }

    return 1;
}

bool loadCameraCalibration(string name, Mat& cameraMatrix, Mat& distanceCoeffients)
{
    ifstream inStream(name);

    if(inStream)
    {
        uint16_t rows;
        uint16_t columns;

        inStream >> rows;
        inStream >> columns;

        cameraMatrix = Mat(Size(columns, rows), CV_64F);

        for(int r = 0; r < rows; r++)
        {
            for(int c = 0; c < columns; c++)
            {
                double  read = 0.0f;

                inStream >> read;
                cameraMatrix.at<double>(r,c) = read;
                cout << cameraMatrix.at<double>(r, c) << "\n";
            }
        }

        // Distance coefficients
        inStream >> rows;
        inStream >> columns;

        distanceCoeffients = Mat::zeros(rows, columns, CV_64F);

        for(int r = 0; r < rows; r++)
        {
            for(int c = 0; c < columns; c++)
            {
                double read = 0.0f;
                inStream >> read;
                distanceCoeffients.at<double>(r,c) = read;
                cout << distanceCoeffients.at<double>(r, c) << "\n";
            }
        }

        inStream.close();
        return true;
    }

    return false;
}

void cameraCalibrationProcess(Mat& cameraMatrix, Mat& distanceCoeffients)
{
    Mat frame;
    Mat drawToFrame;
    
    int framePerSecond = 20;

    vector<Mat> savedImages;

    vector<vector<Point2f>> markerCorners, rejectedCandidates;

    VideoCapture vid(0);

    if(!vid.isOpened())
    {
        return;
    }

    namedWindow("Webcam", 1000);

    while(true)
    {
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
        
        // This character will help us save images for callibration
        char character = waitKey(1000 / framePerSecond);

        /*
         * Press s to save the images. First do a camera calibration, change posistion of the chessboard and press s for as many times as more than specified
         * Press f to finish saving the images
         * b break out to get the camera model
         */

        switch(character)
        {
            case 's':
            //saving the image
            if(found)
            {
                Mat temp;
                frame.copyTo(temp);
                savedImages.push_back(temp);
            }
                break;
            case 'f':
                if(savedImages.size() > 3)
                {
                    cameraCalibration(savedImages, chessboardDimensions, calibrationSquareDimension, cameraMatrix, distanceCoeffients);
                    saveCameraCalibration("CameraCalibrationFile.txt", cameraMatrix, distanceCoeffients);
                }
            //start calibration
                break;

            case 'b':
                //exit
                return;
                break;
        }
    }
}

int main(int argv, char **argc)
{
    Mat cameraMatrix = Mat::eye(3,3, CV_64F);
    Mat distanceCoefficients;
    
    // Uncomment this line and comment the two lines below
    //cameraCalibrationProcess(cameraMatrix, distanceCoefficients);
    loadCameraCalibration("CameraCalibrationFile.txt", cameraMatrix, distanceCoefficients);
    startWebCameraMonitoring(cameraMatrix, distanceCoefficients, 0.099f);

    return 0;
}

