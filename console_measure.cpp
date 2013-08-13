#include <iostream>
#include <boost/thread.hpp>
#include <string>
#include "time.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "laser.h"
#include "capture.h"
#include "server.h"

#define BUFFER_SIZE 1024

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

Mat src, src_gray, dst;

char buffer[BUFFER_SIZE];

void processHTTP(int sock, string request){
  ostringstream response;
  response << "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n";
  if(request.find("X-Requested-With") != string::npos){
    // AJAX request
  }else {
    // Normal http
  }

  // response << "[";
  for(int n = 0; n < num_of_zones; n++){
    Point p = laserToRange(n, laser[n], num_of_zones, total_height);
    // response << "{x:" << p.x << ", " << "y:" << p.y << "}";
    response << p.x << ", " << p.y;
    if(n + 1 < num_of_zones) response << ",\n";
  }
  // response << "]";
  string rstr = response.str();
  write(sock, rstr.c_str(), rstr.length());
}

void processRange(int sock, string request){
  cout << "RANGE" << request << endl;
}

void serveValue(int sock){
  string request;

  do{
    bzero(buffer,BUFFER_SIZE);
    read(sock,buffer,BUFFER_SIZE);
    request += buffer;
  }while (request.find("\r\n\r\n") == string::npos);

  cout << request << endl;

  if(request.find("GET ") == 0){
    // Process HTTP request
    processHTTP(sock, request);
  }else if(request.find("RANGE") == 0){
    // Send raw range data
    processRange(sock, request);
  }
  close(sock);
}


/**
 * @function main
 */
int main( int argc, char** argv )
{

  initCapture();
  Mat src(480, 640, CV_8UC3), undistorted;
  boost::thread* server = startServer(3090, serveValue);
  int frameCounter = 0;
  time_t start = time(NULL);
  for(;;){
    captureOneFrame(src.ptr(), CAPTURE_TYPE_BGR); // get a new frame from camera
    frameCounter++;
    undistort(src, undistorted, cameraMatrix, distCoeffs);

    //imwrite("output.png", undistorted);
    Mat crop(undistorted, Rect(0, top_start, total_width, total_height));

    /// Convert the image to Gray
    cvtColor( crop, src_gray, COLOR_RGB2GRAY );

    findLaser(crop, num_of_zones, laser);
    time_t now = time(NULL);
    if(now - start >= 1) {
      start = now;
      cout << "\r" << frameCounter << "fps   " << flush;
      frameCounter = 0;
    }
  }

  server->join();
  finishCapture();
}
