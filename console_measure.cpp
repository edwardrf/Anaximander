#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include "laser.h"
#include "capture.h"
#include "server.h"

using namespace cv;
using namespace std;

/// Global variables

int threshold_value = 217;
int threshold_type = 3;;
int const max_value = 255;
int const max_type = 4;
int const max_BINARY_value = 255;
int const top_start = 120;
int const total_height = 360;
int const total_width = 640;
int const num_of_zones = 200;

double laser[num_of_zones];
double cameraMatrixData[3][3]={{314.51901676957806, 0.0, 319.5}, {0.0, 314.51901676957806, 239.5}, {0.0, 0.0, 1.0}};
double distCoeffsData[5]={0.32667523460674525, -0.53290547319982662, 0.0, 0.0, 0.21020317981028694};
Mat cameraMatrix(3,3, CV_64FC1, cameraMatrixData);// Scalar(314.51901676957806, 0.0, 319.5, 0.0, 314.51901676957806, 239.5, 0.0, 0.0, 1.0);
Mat distCoeffs(5,1,CV_64FC1, distCoeffsData);

Mat src, src_gray, dst, rdisplay;
const char* window_name = "Laser Range Finder";
const char* range_window_name = "Range";

const char* trackbar_type = "Type: \n 0: Binary \n 1: Binary Inverted \n 2: Truncate \n 3: To Zero \n 4: To Zero Inverted";
const char* trackbar_value = "Value";

/// Function headers
void Threshold_Demo( int, void* );

void serveValue(int sock){
    int n;
    char buffer[256];
    bzero(buffer,256);
    n = read(sock,buffer,255);
    if (n < 0) puts("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);
    n = write(sock,"I got your message",18);
    if (n < 0) puts("ERROR writing to socket");
}

/**
 * @function main
 */
int main( int argc, char** argv )
{

  initCapture();
  Mat src(480, 640, CV_8UC3), undistorted;
  captureOneFrame(src.ptr(), CAPTURE_TYPE_BGR); // get a new frame from camera
  finishCapture();
  // src = imread( argv[1], 1 );
  //undistorted = src.clone();
  undistort(src, undistorted, cameraMatrix, distCoeffs);

  imwrite("output.png", undistorted);
  Mat crop(undistorted, Rect(0, top_start, total_width, total_height));

  /// Convert the image to Gray
  cvtColor( crop, src_gray, CV_RGB2GRAY );

  findLaser(crop, num_of_zones, laser);

  Threshold_Demo(0,0);
  startServer(3090, serveValue);
  puts("hi");
  wait();
}

/**
 * @function Threshold_Demo
 */
void Threshold_Demo( int, void*)
{

  for(int n = 0; n < num_of_zones; n++){
    cout << laser[n] << "\t";
  }

}
