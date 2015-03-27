#include <tcutil.h>
#include <tchdb.h>
#include <stdexcept>
#include <cstring>
#include <utility>
#include <tr1/unordered_map>



class AveragedPerceptron {
  typedef std::tr1::unordered_map<std::string, double> FeatureVec;
  TCHDB *hdb;
  size_t numOfExample;

  TCHDB *OpenDB(const char *path){
    if(path == NULL || (strcmp(path, "") == 0))
      throw std::logic_error("a hdb path is not specified");

    hdb = tchdbnew();
    if(hdb == NULL)
      throw std::logic_error("tchdbnew failed");

    if(!tchdbsetmutex(hdb)){
      tchdbdel(hdb);
      throw std::logic_error("tchdbsetmutex failed");
    }

    if(!tchdbopen(hdb, path, HDBOWRITER | HDBOCREAT)){
      tchdbdel(hdb);
      throw std::logic_error("tchdbopen failed");
    }
  }

  std::pair<double,double> GetFeatureValue(const std::string &key){
    double value_0 = 0.0f;
    double value_a = 0.0f;
    char *res = tchdbget2(hdb, key.c_str());
    if(res == NULL || strlen(res) * sizeof(char) != sizeof(double) * 2)
      return std::make_pair(value_0, value_a);
    value_0 = *(reinterpret_cast<double*>(res));
    value_a = *(reinterpret_cast<double*>(res + sizeof(double)));
    free(res);
    return std::make_pair(value_0, value_a);
  }

  bool SetFeatureValue(const std::string &key, std::pair<double,double> value){
    char *value_str = new char[sizeof(double) * 2 + 1];
    strncpy(value_str,
            reinterpret_cast<char*>(&(value.first)), sizeof(double));
    strncpy(value_str + sizeof(double),
            reinterpret_cast<char*>(&(value.second)), sizeof(double) + 1);
    return tchdbput2(hdb, key.c_str(), value_str);
  }

  bool IsNeedUpdate(const FeatureVec example, double y){
    double score = 0.0f;
    for(FeatureVec::const_iterator itr = example.begin();
        itr != example.end(); ++itr){
      std::pair<double,double> val = GetFeatureValue(itr->first);
      score += itr->second * val.first;
    }
    return (y * score) <= 0.0f;
  }

  void _Update(const FeatureVec example, double y){
    for(FeatureVec::const_iterator itr = example.begin();
        itr != example.end(); ++itr){
      std::pair<double,double> value = GetFeatureValue(itr->first);
      value.first += y * itr->second;
      value.second += numOfExample * y * itr->second;
      SetFeatureValue(itr->first, value);
    }
    numOfExample++;
  }

  double _Classify(const FeatureVec example){
    double score = 0.0f;
    for(FeatureVec::const_iterator itr = example.begin();
        itr != example.end(); ++itr){
      std::pair<double,double> value = GetFeatureValue(itr->first);
      score += value.first - value.second / numOfExample;
    }
    return score;
  }

  size_t GetNumOfExample(){
    size_t value = 1;
    char *res = tchdbget2(hdb, "Num Of Examples");
    if(res == NULL || strlen(res) * sizeof(char) != sizeof(size_t))
      return value;
    value = *(reinterpret_cast<size_t*>(res));
    free(res);
    return value;
  }

  void SetNumOfExample(size_t num){
    char *value_str = new char[sizeof(size_t) + 1];
    strncpy(value_str, reinterpret_cast<char*>(&num), sizeof(size_t));
    if(!tchdbput2(hdb, "Num Of Examples", value_str))
      throw std::logic_error("Can't set NumOfExample");
  }


public:
  AveragedPerceptron(const char *path){
    OpenDB(path);
  }

  ~AveragedPerceptron(){
    tchdbdel(hdb);
  }

};
