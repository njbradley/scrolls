#include "common.h"

#ifdef _WIN32
#include <profileapi.h>

using TimeTicks = unsigned LONGLONG;

TimeTicks get_ticks_per_sec() {
  LARGE_INTEGER val;
  QueryPerformanceFrequency(&val);
  return val.QuadPart;
}

TimeTicks get_raw_ticks() {
  LARGE_INTEGER val;
  QueryPerformanceCounter(&val);
  return val.QuadPart;
}

#else

using TimeTicks = unsigned long long int;

TimeTicks get_ticks_per_sec() {
  return 1000000000;
}

TimeTicks get_raw_ticks() {
  timespec vals;
  clock_gettime(CLOCK_MONOTONIC, &vals);
  return (TimeTicks)vals.tv_sec * get_ticks_per_sec() + vals.tv_nsec;
}

#endif

double getTime() {
  static double ticks_per_sec = get_ticks_per_sec();
  static TimeTicks start_ticks = get_raw_ticks();
  return (double) (get_raw_ticks() - start_ticks) / ticks_per_sec;
}
