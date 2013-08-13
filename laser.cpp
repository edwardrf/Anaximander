#include "laser.h"

void findLaser(Mat I, int num_of_zones, double* laser){
  int total_height = I.rows;
  int total_width = I.cols;

  double total[num_of_zones],
    maxv[num_of_zones],
    cnt[num_of_zones],
    maxv_sorted[num_of_zones];

  int readings[num_of_zones][total_height];
  int readings_sorted[num_of_zones][total_height];

  // Initialization
  for(int n = 0; n < num_of_zones; n++){
    cnt[n] = 0;
    total[n] = 0;
    maxv[n] = 0;
    maxv_sorted[n] = 0;
  }

  // Accumulation
  for(int y = 0; y < total_height; y++){
    double sx = -158.61 + 0.61682 * y;
    double dx = (1.2243 * (476 - y) + 301) / num_of_zones;
    for(int n = 0; n < num_of_zones; n++){
      int x = sx + dx * (n + 0.5);
      if(x < 0 || x > total_width) continue;
      int value = (I.ptr<uchar>(y))[x];
      cnt[n]++;
      total[n] += value;
      if(maxv[n] < value){
        maxv[n] = value;
        maxv_sorted[n] = value;
      }
      readings[n][total_height - y] = value;
      readings_sorted[n][total_height - y] = value;
    }
  }

  gsl_sort(maxv_sorted, 1, num_of_zones);
  int mostv = gsl_stats_quantile_from_sorted_data(maxv_sorted, 1, num_of_zones, 0.7);

  // Calculation
  for(int n = 0; n < num_of_zones; n++){

    int t = maxv[n] - 5;
    if (t < 0) t = 0;

    int local_sum = 0;
    int local_cnt = 0;
    int last_sum = 0;
    int last_cnt = 0;
    int xcnt = 0;
    int status = 0;
    int start_val = 0;

    for(int y = 1; y < cnt[n]; y++){
      if(status == 0){
        int g = readings[n][y] - readings[n][y-1];
        if(g > 10){
          status = 1;
          start_val = readings[n][y - 1];
          xcnt ++;
        }
      }else if(status == 1) {
        if(readings[n][y] > start_val) {
          local_sum += readings[n][y] * y;
          local_cnt += readings[n][y];
        }else {
          last_sum = local_sum;
          last_cnt = local_cnt;
          local_sum = 0;
          local_cnt = 0;
          status = 0;
        }
      }
    }

    // Continue here, new method: try to find local max
/*
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
*/
    if(last_cnt > 0){
      laser[n] = (double)last_sum / last_cnt;
    }else {
      laser[n] = -1;
    }

    // Remove readings either not significant or have too many local max
    if((double)maxv[n] / mostv < 0.8 || xcnt > 5) laser[n] = -1;

  }
}


Point mapToRange(double x, double y, int height){
  x = x - sensor_center;
  y = height - y;

  int rx = (int)(x / (15590.7/ y - 48.0679));
  int ry = (int)(4.52556087963698 * y/ (317.3293561387 - y));

  return Point(rx, ry);
}

Point laserToRange(int n, double y, int num_of_zones, int height){
  double sx = -158.61 + 0.61682 * y;
  double dx = (1.2243 * (476 - y) + 301) / num_of_zones;
  double x = sx + dx * (n + 0.5);
  return mapToRange(x, y, height);
}
