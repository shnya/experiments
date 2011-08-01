#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>

#include "double_array.hpp"

template<class Cont, class T>
bool dict_read(std::vector<Cont> &res, const char *filename){
  std::ifstream ifs(filename);
  int n;
  ifs.read(reinterpret_cast<char*>(&n),sizeof(int));
  if(ifs.bad()){ ifs.close(); return false; }
  std::vector<T> v(n);
  ifs.read(reinterpret_cast<char*>(&v[0]),sizeof(T)*n);
  if(ifs.bad()){ ifs.close(); return false; }
  for(size_t i = 0; i < v.size(); i++){
    size_t k = static_cast<size_t>(v[i++]);
    res.push_back(Cont(&(v[i]),&(v[i+k])));
    i += k-1;
  }
  ifs.close();
  if(ifs.bad()) return false;
  return true;
}

bool katakana_read(DoubleArray &da){
  std::ifstream ifs("system.idx");
  if(ifs.bad()) return false;
  return da.load(ifs);
}

bool kanakanmap_read(std::vector<std::vector<int> > &kanakan_map){
  return dict_read<std::vector<int>, int>(kanakan_map, "map.idx");
}

bool kanjimap_read(std::vector<std::string> &kanji_map){
  return dict_read<std::string, char>(kanji_map, "kanji.idx");
}

bool bigram_read(std::map<std::pair<int,int>,int> &res){
  std::ifstream ifs("bigram.idx");
  int n;
  ifs.read(reinterpret_cast<char*>(&n),sizeof(int));
  if(ifs.bad()){ ifs.close(); return false; }
  std::vector<int> v(n);
  ifs.read(reinterpret_cast<char*>(&v[0]),sizeof(int)*n);
  if(ifs.bad()){ ifs.close(); return false; }
  for(size_t i = 0; i < v.size(); i+=3){
    res.insert(std::make_pair(std::make_pair(v[i],v[i+1]),v[i+2]));
  }
  ifs.close();
  if(ifs.bad()) return false;
  return true;
}


#endif /* _DICTIONARY_H_ */
