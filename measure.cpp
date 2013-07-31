#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <gsl/gsl_sort_int.h>
#include <gsl/gsl_statistics_int.h>

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
int const min_threshold = 100;
int const sensor_center = 309;

double cameraMatrixData[3][3]={314.51901676957806, 0.0, 319.5, 0.0, 314.51901676957806, 239.5, 0.0, 0.0, 1.0};
double distCoeffsData[5]={0.32667523460674525, -0.53290547319982662, 0.0, 0.0, 0.21020317981028694};
Mat cameraMatrix(3,3, CV_64FC1, cameraMatrixData);// Scalar(314.51901676957806, 0.0, 319.5, 0.0, 314.51901676957806, 239.5, 0.0, 0.0, 1.0);
Mat distCoeffs(5,1,CV_64FC1, distCoeffsData);

double total[num_of_zones], 
  maxv[num_of_zones], 
  cnt[num_of_zones],
  laser[num_of_zones];

int readings[num_of_zones][total_height];
int readings_sorted[num_of_zones][total_height];

Mat src, src_gray, dst, rdisplay;
const char* window_name = "Laser Range Finder";
const char* range_window_name = "Range";

const char* trackbar_type = "Type: \n 0: Binary \n 1: Binary Inverted \n 2: Truncate \n 3: To Zero \n 4: To Zero Inverted";
const char* trackbar_value = "Value";

/// Function headers
void Threshold_Demo( int, void* );
void findLaser(Mat I);
void onMouse( int event, int x, int y, int, void*);
Point mapToRange(int x,int y);

/**
 * @function main
 */
int main( int argc, char** argv )
{

  rdisplay.create(200,200,CV_8UC1);

  //VideoCapture cap(1); // open the default camera
  // if(!cap.isOpened())  // check if we succeeded
  //   return -1;
  /// Create a window to display results
  namedWindow( window_name, CV_WINDOW_AUTOSIZE );
  namedWindow( range_window_name, CV_WINDOW_AUTOSIZE );

  /// Create Trackbar to choose type of Threshold
  createTrackbar( trackbar_type,
                  window_name, &threshold_type,
                  max_type, Threshold_Demo );

  createTrackbar( trackbar_value,
                  window_name, &threshold_value,
                  max_value, Threshold_Demo );

  setMouseCallback(window_name, onMouse, 0);

  for(;;){
    Mat src, undistorted;
    rdisplay.setTo(Scalar(30));
    // cap >> src; // get a new frame from camera
    src = imread( argv[1], 1 );
    undistorted = src.clone();
    undistort(src, undistorted, cameraMatrix, distCoeffs);

    Mat crop(undistorted, Rect(0, top_start, total_width, total_height));

    /// Convert the image to Gray
    cvtColor( crop, src_gray, CV_RGB2GRAY );

    findLaser(src_gray);

    /// Call the function to initialize
    Threshold_Demo( 0, 0 );

    /// Wait until user finishes program
    int c;
    c = waitKey(20);
    if( (char)c == 27 ) return 0;
  }

}

void findLaser(Mat I){
  // Initialization
  for(int n = 0; n < num_of_zones; n++){
    cnt[n] = 0;
    total[n] = 0;
    maxv[n] = 0;
  }

  // Accumulation
  for(int y = 0; y < total_height; y++){
    double sx = -158.61 + 0.61682 * y;
    double dx = (1.2243 * (476 - y) + 301) / num_of_zones;
    for(int n = 0; n < num_of_zones; n++){
      int x = sx + dx * (n + 0.5);
      if(x < 0 || x > total_width) continue;
      int value = (src_gray.ptr<uchar>(y))[x];
      cnt[n]++;
      total[n] += value;
      if(maxv[n] < value) maxv[n] = value;
      readings[n][total_height - y] = value;
      readings_sorted[n][total_height - y] = value;
    }
  }

  // Calculation
  for(int n = 0; n < num_of_zones; n++){
    gsl_sort_int(readings_sorted[n], 1, cnt[n]);
    // double mean = total[n] / cnt[n];
    int t = gsl_stats_int_quantile_from_sorted_data(readings_sorted[n], 1, cnt[n], 0.99);
    // cout << cnt[n] << ", \t" << n << ", \t" << t;
    int local_sum = 0;
    int local_cnt = 0;
    int last_sum = 0;
    int last_cnt = 0;
    int xcnt = 0;
    if(t == maxv[n]) t = maxv[n] - 1;
    if(t < min_threshold) t = min_threshold;
    if(t >= 0){
      for(int y = 0; y < cnt[n]; y++){
        if(readings[n][y] > t){
          local_sum += readings[n][y] * y;
          local_cnt += readings[n][y];
          xcnt ++;
        }else {
          if(local_sum > 0){
            last_sum = local_sum;
            last_cnt = local_cnt;
          }
          local_sum = 0;
          local_cnt = 0;
        }
      }
      // cout << ", \t" << last_sum << ", \t" << last_cnt << ", \t" << xcnt << endl;
      if(last_cnt > 0){
        laser[n] = last_sum / last_cnt;
      }else {
        laser[n] = -1;
      }
    }else {
      laser[n] = -1;
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

  int miny = 1000, maxy = 0, cy = 0, ay = 0, zy = 0;
  for(int n = 0; n < num_of_zones; n++){
    double y = total_height - laser[n];
    double sx = -158.61 + 0.61682 * y;
    double dx = (1.2243 * (476 - y) + 301) / num_of_zones;
    int x = sx + dx * n;
    if(x < 0 || x > total_width) continue;
    circle(dst, Point(x, y), 5, 80, 2);
    Point mapped = mapToRange(x, y);
    circle(rdisplay, Point(mapped.x + 100, 200-mapped.y), 3, 255, 1);
    if(y > 50 && y < 90){
      if(ay == 0) ay = y;
      zy = y;
      if(n == 50) cy = y;
      if(y > maxy) maxy = y;
      if(y < miny) miny = y;
    }
  }
  // cout << miny << '\t' << cy << '\t' << maxy << '\t' << ay << '\t' << zy << endl;
  // for(int i = 0; i < src_gray.cols; ++i){
  //   (dst.ptr<uchar>((int)laser[i]))[i] = 125;
  // }

  imshow( window_name, dst );
  imshow( range_window_name, rdisplay);
}

void onMouse( int event, int x, int y, int, void*){
  if(event != 4) return;
  cout << event << ", " << x << ", " << y << endl;
}

Point mapToRange(int x, int y){
  x = x - sensor_center;
  y = total_height - y;

  int rx = (int)(x / (15590.7/ y - 48.0679));
  int ry = (int)(4.52556087963698 * y/ (317.3293561387 - y));

  return Point(rx, ry);
}