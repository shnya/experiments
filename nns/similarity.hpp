#ifndef _SIMILARITY_H_
#define _SIMILARITY_H_

#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "string_piece.hpp"



struct LevenshteinSimilarity {
  double operator()(const StringPiece &x, const StringPiece &y){
    int xn = x.size()+1, yn = y.size()+1;
    std::vector<int> dp(xn*yn);
    for(int i = 0; i < xn; i++) dp[i*yn] = i;
    for(int j = 0; j < yn; j++) dp[j] = j;
    for(int i = 1; i < xn; i++){
      for(int j = 1; j < yn; j++){
        if(x[i-1] == y[j-1]){
          dp[i*yn+j] = dp[(i-1)*yn+j-1];
        }else{
          dp[i*yn+j] =
            std::min(dp[i*yn+j-1] + 1,
                     std::min(dp[(i-1)*yn+j] + 1,
                              dp[(i-1)*yn+j-1] +1));
        }
      }
    }
    //std::cout << x << " " << y << " " << xn << " " << yn << " " << dp[xn*yn-1] << std::endl;
    return 100000 - dp[xn*yn-1];
  }
};

struct NgramJaccardSimilarity {
  double operator()(const StringPiece &x, const StringPiece &y){
    int xn = x.size()+1, yn = y.size()+1;
    const char *xp = x.data(), *yp = y.data();
    std::vector<std::string> v1,v2;
    for(size_t i = 1; i <= 3; i++){
      for(size_t j = 0; j < xn - i; j++){
        v1.push_back(std::string(xp+j,i));
      }
      for(size_t j = 0; j < yn - i; j++){
        v2.push_back(std::string(yp+j,i));
      }
    }
    std::sort(v1.begin(),v1.end());
    std::sort(v2.begin(),v2.end());
    std::vector<std::string> set_inter,set_uni;
    set_intersection(v1.begin(),v1.end(),v2.begin(),v2.end(),
                     back_inserter(set_inter));
    set_union(v1.begin(),v1.end(),v2.begin(),v2.end(),
              back_inserter(set_uni));
    return static_cast<double>(set_inter.size()) / set_uni.size();
  }
};


template<class T>
struct CosineSimilarity {
  double operator()(const std::vector<T> &x, const std::vector<T> &y){
    int xsiz = x.size(), ysiz = y.size();
    double res = 0.0;
    int i=0,j=0;
    for(; i < xsiz; i++){
      int xi = x[i];
      if(xi < y[i]) continue;
      for(; j < ysiz; j++)
        if(xi <= y[j])
          break;
      if(j == ysiz){
        break;
      }else if(xi == y[j]){
        res += 1.0;
      }
    }
    return res / (sqrt(xsiz) * sqrt(ysiz));
  }
};

#endif /* _SIMILARITY_H_ */
