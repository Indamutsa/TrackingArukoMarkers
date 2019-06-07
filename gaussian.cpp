void createGaussian(Size& size, Mat& output, int uX, int uY, float sigmaX, float sigmaY, float amplitude - 1.0f)
{
     Mat temp = Mat(size, CV_32F);
 
     for(_int r = 0; r < size.height; c++ )
     {
         for(int c =0; c< size.width; c++)
         {
             float x = i((c - uX) * ((float)c - uX)) / (2.0f * sigmaX * sigmaX);
             float y =  ((r - uY) * ((float)r - uY)) / (2.0f * sigmaY * sigmaY);
          
             float value = amplitude * exp(-(x + y));
             temp.at<float>(r, c) = value;
         }
     }
  
    normalize(temp, temp, 0.0f, 1.0f, NORM_MINIMAX);
    output = temp ;
}

int main(int argv, char** argc)
{
    Mat output;
    createCaussian(Size(256,256), output, 256 / 2, 256/ 2, 10, 10);
    imshow("Gaussian", output);
    waitKey();
}
