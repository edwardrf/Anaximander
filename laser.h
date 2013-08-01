#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <gsl/gsl_sort_int.h>
#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics_int.h>
#include <gsl/gsl_statistics.h>

using namespace cv;
using namespace std;

int const sensor_center = 309;

void findLaser(Mat I, int, double*);
Point mapToRange(double x, double y, Mat I);