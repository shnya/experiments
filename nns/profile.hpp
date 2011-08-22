#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <sys/resource.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <cstdio>

double GetusageSec()
{
  struct rusage t;
  struct timeval tv;
  getrusage(RUSAGE_SELF, &t);
  tv = t.ru_utime;
  return tv.tv_sec + (double)tv.tv_usec*1e-6;
}

void PrintTime(double before, double after)
{
  fprintf(stderr,"  time = %10.30f\n", after-before);
  fflush(stderr);
}

#endif /* _PROFILE_H_ */
