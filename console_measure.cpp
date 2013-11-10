#include <iostream>
#include <boost/thread.hpp>
#include <string>
#include "time.h"
#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "laser.h"
#include "server.h"
#include "base64.h"
#include "sha1.h"
#include "boost/date_time/posix_time/posix_time.hpp"

typedef boost::posix_time::ptime Time;

#define BUFFER_SIZE 1024
#define WEB_SOCKET_MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define WEB_SOCKET_ACCEPT "Sec-WebSocket-Accept"
#define WEB_SOCKET_KEY "Sec-WebSocket-Key"

using namespace cv;
using namespace std;
using namespace boost;

/// Global variables

int threshold_value = 217;
int threshold_type = 3;
int const video_device = 0;
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
condition_variable newDataCond;
mutex mut;
bool newDataReady;
long remapTime = 0;

void processRange(int sock, string request);
void processRequest(int sock);
void processHTTP(int sock, string request);

/**
 * @function main
 */
int main( int argc, char** argv )
{
  Mat src, undistorted;
  VideoCapture cap(video_device);
  if(!cap.isOpened()){
    cout << "Failed to open video device " << video_device << ". Please check the camera is properly attached" << endl;
    return -1;
  }

  thread* server = startServer(3090, processRequest);
  int frameCounter = 0;
  time_t start = time(NULL);

  while(src.empty()) cap >> src; // Get a first frame to generate the undistort map
  Size size = src.size();
  Mat map1, map2;
  initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(),
      getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, size, 1, size, 0),
      size, CV_16SC2, map1, map2);

  for(;;){
    frameCounter++;

    cap >> src;
    if(src.empty()) continue;

    Time us=boost::posix_time::microsec_clock::local_time();
    remap(src, undistorted, map1, map2, INTER_LINEAR);
    Time ue=boost::posix_time::microsec_clock::local_time();
    remapTime += (ue - us).total_microseconds();

    //imwrite("output.png", undistorted);
    Mat crop(undistorted, Rect(0, top_start, total_width, total_height));
    /// Convert the image to Gray
    cvtColor( crop, src_gray, COLOR_RGB2GRAY );

    findLaser(src_gray, num_of_zones, laser);
    {
        lock_guard<mutex> lock(mut);
        newDataReady=true;
    }
    newDataCond.notify_one();

    time_t now = time(NULL);
    if(now - start >= 1) {
      start = now;
      cout << "\r" << frameCounter << "fps \tremap time: " << (remapTime / frameCounter) << "         " << flush;
      frameCounter = 0;
      remapTime = 0;
    }
  }
  cout << "Done." << endl << flush;
  server->join();
}

string prepareJSON(){
  ostringstream buf;
  buf << "[";
  int c = 0;
  for(int n = 0; n < num_of_zones; n++){
    if(laser[n] < 0) continue;
    if(c > 0) buf << ",\n";
    Point p = laserToRange(n, laser[n], num_of_zones, total_height);
    buf << "[" << p.x << ", " << p.y << "]";
    c++;
  }
  buf << "]";
  return buf.str();
}

/**
 * Process the request as a HTTP request
 */
void processHTTP(int sock, string request){
  ostringstream response;
  size_t webSocketIndex = request.find(WEB_SOCKET_KEY);
  if(webSocketIndex != string::npos){
    // Socket IO request
    // Get the value of Sec-WebSocket-Accept
    size_t start = webSocketIndex + strlen(WEB_SOCKET_KEY);
    while(request.at(start) == ' ' || request.at(start) == ':') start++;
    size_t end = request.find("\r\n", start);
    if(end == string::npos) return;
    string value = request.substr(start, end-start);

    // Calculate the hash
    string str = value + WEB_SOCKET_MAGIC_STRING;
    unsigned char shabin[20];

    sha1::calc(str.c_str(), str.length(), shabin);
    int hashlen = 0;

    char* shabase64 = base64(shabin, 20, &hashlen);

    cout<<shabase64;

    // Response
    response << "HTTP/1.1 101 Switching Protocols\r\n"
        << "Connection: Upgrade\r\n"
        << "Upgrade: websocket\r\n"
        << "Sec-WebSocket-Accept: " << shabase64 << "\r\n\r\n";

    string rstr = response.str();

    // Content
    write(sock, rstr.c_str(), rstr.length());

    // Start transfering
    unique_lock<mutex> lock(mut);
    while(true){
      while(!newDataReady){
        newDataCond.wait(lock);
      }
      newDataReady = false;

      string json = prepareJSON() + "\r\n\r\n";
      int len = json.length();

      // Message header, text message
      unsigned char b = 129;
      write(sock, &b, 1);

      if(len < 126){
        b = len;
        write(sock, &b, 1);   
      }else {
        // 2 byte length, no mask
        b = 126;
        write(sock, &b, 1); 
        b = (len >> 8) & 0xFF;
        write(sock, &b, 1);
        b = len & 0xFF;
        write(sock, &b, 1);
      }

      int r = write(sock, json.c_str(), json.length());
      if(r == -1) {
        cout << "Write to client failed" << endl << flush;
        return;
      }
    }

  }else {
    // Normal http
    response << "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nAccess-Control-Allow-Origin: http://localhost\r\n\r\n";
    response << prepareJSON();
    string rstr = response.str();
    write(sock, rstr.c_str(), rstr.length());
  }
}

void processRange(int sock, string request){
  unique_lock<mutex> lock(mut);
  int buf[2];
  while(true){
    while(!newDataReady){
      newDataCond.wait(lock);
    }
    newDataReady = false;
    for(int n = 0; n < num_of_zones; n++){
      if(laser[n] < 0) continue;
      Point p = laserToRange(n, laser[n], num_of_zones, total_height);
      buf[0] = p.x;
      buf[1] = p.y;
      int r = write(sock, buf, 2);
      if(r == -1) {
        cout << "Write to client failed" << endl << flush;
        return;
      }
    }
  }
}

/**
 * Process request from conecting socket
 */
void processRequest(int sock){
  string request;

  do{
    bzero(buffer,BUFFER_SIZE);
    read(sock,buffer,BUFFER_SIZE);
    request += buffer;
  }while (request.find("\r\n\r\n") == string::npos);

  //cout << request << endl;

  if(request.find("GET ") == 0){
    // Process HTTP request
    processHTTP(sock, request);
  }else if(request.find("RANGE") == 0){
    // Send raw range data
    processRange(sock, request);
  }
  close(sock);
}
