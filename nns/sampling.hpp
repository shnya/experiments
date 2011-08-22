#ifndef _SAMPLE_H_
#define _SAMPLE_H_

#include <vector>
#include <cstdlib>
#include <time.h>

class GenSample {
public:
  static void init(){
    srand(static_cast<unsigned>(time(NULL)));
  }

  static void sampling(std::vector<int> &v, int k, std::vector<int> &out){
    int n = v.size();
    k = std::min(n,k);
    std::vector<int> vec(k);
    for(int i = 0; i < k; i++,n--){
      int r = rand() % n;
      vec[i] = v[r];
      std::swap(v[r],v[n-1]);
    }
    vec.swap(out);
  }

  static void sampling_overwrapped(int n, int k, std::vector<int> &out){
    std::vector<int> vec(k);
    for(int i = 0; i < k; i++){
      int r = rand() % n;
      vec[i] = r;
    }
    vec.swap(out);
  }

  static void sampling_exclude_me(std::vector<int> &v, int k, int me,
                                  std::vector<int> &out){
    int n = v.size() - 1;
    k = std::min(n,k);
    std::vector<int> vec(k);
    for(int i = 0; i < k; i++,n--){
      int r = rand() % n;
      if(me == v[r]){
        vec[i] = v[v.size()-1];
      }else{
        vec[i] = v[r];
      }
      std::swap(v[r],v[n-1]);
    }
    vec.swap(out);
  }

  static void sampling_all(std::vector<int> &v,std::vector<int> &out){
    sampling(v,v.size(),out);
  }
};

#endif /* _SAMPLE_H_ */

