#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <sys/resource.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <memory>
#include <iostream>
#include <iomanip>

#ifdef DEBUG
#else

#endif /* DEBUG */



struct ProfilePrinter {
  void operator()(double utime, double systime){
    char buf[50];
    snprintf(buf, sizeof(buf), "%.10f", utime);
    buf[49] = '\0';
    std::cerr << "user time: " << buf << "\t";
    snprintf(buf, sizeof(buf), "%.10f", systime);
    buf[49] = '\0';
    std::cerr << "sys time: " << buf << std::endl;
  }
};

template<class Callback = ProfilePrinter>
class Profile {
  std::auto_ptr<Callback> callback_;
  double utime_,systime_;

  std::pair<double,double> GetCurrentTime() const {
    rusage t;
    timeval tuv,tsv;
    getrusage(RUSAGE_SELF, &t);
    tuv = t.ru_utime;
    tsv = t.ru_stime;
    double utime = tuv.tv_sec + (double)tuv.tv_usec*1e-6;
    double systime = tsv.tv_sec + (double)tsv.tv_usec*1e-6;
    return std::make_pair(utime,systime);
  }

  Profile &operator=(const Profile &);
  Profile(const Profile &);

public:
  Profile() :
    callback_(new Callback())
  {
    std::pair<double,double> time_pair = GetCurrentTime();
    utime_ = time_pair.first;
    systime_ = time_pair.second;
  }

  std::pair<double,double> time() const {
    std::pair<double,double> time_pair = GetCurrentTime();
    return std::make_pair(time_pair.first - utime_,
                     time_pair.second - systime_);
  }

  void measure() const {
    std::pair<double,double> time_pair = GetCurrentTime();
    (*callback_)(time_pair.first - utime_,
                 time_pair.second - systime_);
  }

  void end(){
    if(callback_.get() != NULL){
      measure();
      callback_.reset();
    }
  }

  void invalidate(){
    if(callback_.get() != NULL){
      callback_.reset();
    }
  }

  ~Profile(){
    if(callback_.get() != NULL)
      measure();
  }
};

template<class Callback = ProfilePrinter>
class CulmativeProfile {
  std::auto_ptr<Callback> callback_;
  double utime_,systime_;

  CulmativeProfile &operator=(const CulmativeProfile &);
  CulmativeProfile(const CulmativeProfile &);


public:
  CulmativeProfile() : 
    callback_(new Callback()), utime_(0.0), systime_(0.0) {}


  void measure() const {
    (*callback_)(utime_, systime_);
  }

  template<class T>
  void add(Profile<T> &prof){
    std::pair<double,double> time_pair = prof.time();
    prof.invalidate();
    utime_ += time_pair.first;
    systime_ += time_pair.second;
  }

  void invalidate(){
    callback_.reset();
  }

  ~CulmativeProfile(){
    if(callback_.get() != NULL)
      measure();
  }

};

#endif /* _PROFILE_H_ */
