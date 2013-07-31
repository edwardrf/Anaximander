#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>

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
int const num_of_zones = 100;
int const total_width = 640;

Mat src, src_gray, dst;
const char* window_name = "Threshold Demo";

const char* trackbar_type = "Type: \n 0: Binary \n 1: Binary Inverted \n 2: Truncate \n 3: To Zero \n 4: To Zero Inverted";
const char* trackbar_value = "Value";

/// Function headers
void Threshold_Demo( int, void* );
void findLaser(Mat I);

/**
 * @function main
 */
int main( int argc, char** argv )
{
  VideoCapture cap(2); // open the default camera
  if(!cap.isOpened())  // check if we succeeded
    return -1;
  /// Create a window to display results
  namedWindow( window_name, CV_WINDOW_AUTOSIZE );

  /// Create Trackbar to choose type of Threshold
  createTrackbar( trackbar_type,
                  window_name, &threshold_type,
                  max_type, Threshold_Demo );

  createTrackbar( trackbar_value,
                  window_name, &threshold_value,
                  max_value, Threshold_Demo );

  for(;;){
    /// Load an image
    Mat src;
    cap >> src; // get a new frame from camera
    if(src.empty()) {
        cout << "Skipping empty frame" << endl;
        cout.flush();
        continue;
    }
    Mat crop(src, Rect(0, top_start, total_width, total_height));

    /// Convert the image to Gray
    cvtColor( crop, src_gray, CV_RGB2GRAY );

    //findLaser(src_gray);

    /// Call the function to initialize
    Threshold_Demo( 0, 0 );

    /// Wait until user finishes program
    int c;
    c = waitKey(20);
    if( (char)c == 27 ) return 0;
  }

}

void findLaser(Mat I){
  for(int n = 0; n < num_of_zones; n++){
    for(int y = 0; y < total_height; y++){
      double sx = -158.61 + 0.61682 * y;
      double dx = (1.2243 * (476 - y) + 301) / num_of_zones;
      int x1 = sx + dx * n;
      int x2 = x1 + dx;
      if(x2 < 0 || x1 > total_width) continue;
      if(x1 < 0) x1 = 0;
      if(x2 > total_width) x2 = total_width;
      for(int x = x1; x < x2; x++){
        
      }
    }
  }
}

/**
 * @function Threshold_Demo
 */
void Threshold_Demo( int, void* )
{
  /* 0: Binary
     1: Binary Inverted
     2: Threshold Truncated
     3: Threshold to Zero
     4: Threshold to Zero Inverted
   */

  threshold( src_gray, dst, threshold_value, max_BINARY_value,threshold_type );

  for(int y = 0; y < total_height; y++){
    double sx = -158.61 + 0.61682 * y;
    double dx = (1.2243 * (476 - y) + 301) / num_of_zones;
    for(int n = 0; n < num_of_zones; n++){
      int x = sx + dx * n;
      if(x < 0 || x > total_width) continue;
      (dst.ptr<uchar>(y))[x] = 125;
    }
  }

  // for(int i = 0; i < src_gray.cols; ++i){
  //   (dst.ptr<uchar>((int)laser[i]))[i] = 125;
  // }

  imshow( window_name, dst );
}