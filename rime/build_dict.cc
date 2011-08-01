#include "double_array.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

using namespace std;

template <class Cont, class T>
void dict_write(const vector<Cont> &map, const char *filename){
  vector<T> v;
  for(size_t i = 0; i < map.size(); i++){
    v.push_back(map[i].size());
    for(size_t j = 0; j < map[i].size(); j++){
      v.push_back(map[i][j]);
    }
  }
  int n = v.size();
  ofstream ofs(filename);
  if(ofs.bad()) throw logic_error("open error");
  ofs.write(reinterpret_cast<const char*>(&n),sizeof(int));
  if(ofs.bad()) throw logic_error("write error");
  ofs.write(reinterpret_cast<const char*>(&v[0]),sizeof(T)*v.size());
  if(ofs.bad()) throw logic_error("write error");
  ofs.close();
  if(ofs.bad()) throw logic_error("write error");
}

void map_write(const vector<vector<int> > &keymap){
  dict_write<vector<int>,int>(keymap, "map.idx");
}

void kanji_write(const vector<string> &kanjimap){
  dict_write<string,char>(kanjimap, "kanji.idx");
}

void bigram_write(const vector<pair<pair<int,int>,float> > &bigram){
  vector<int> v;
  for(size_t i = 0; i < bigram.size(); i++){
    v.push_back(bigram[i].first.first);
    v.push_back(bigram[i].first.second);
    v.push_back(log(bigram[i].second));
  }
  ofstream ofs("bigram.idx");
  if(ofs.bad()) throw logic_error("open error");
  int n = v.size();
  ofs.write(reinterpret_cast<const char*>(&n),sizeof(int));
  if(ofs.bad()) throw logic_error("write error");
  ofs.write(reinterpret_cast<const char*>(&v[0]),sizeof(int)*v.size());
  if(ofs.bad()) throw logic_error("write error");
  ofs.close();
  if(ofs.bad()) throw logic_error("write error");
}


int main(int argc, char *argv[])
{
  ios_base::sync_with_stdio(false);

  int n;
  string line;

  //make katakana dic.
  cin >> n;
  getline(cin,line);
  vector<pair<string,int> > da_key;
  for(int i = 0; i < n; i++){
    getline(cin,line);
    istringstream iss(line);
    int id; string key;
    iss >> id >> key;
    da_key.push_back(make_pair(key,id));
  }
  DoubleArray da(da_key);
  ofstream ofs("system.idx");
  if(ofs.bad()) throw logic_error("open error");
  bool res = da.save(ofs);
  if(!res) throw logic_error("write error");
  ofs.close();

  cin >> n;
  getline(cin,line);
  vector<vector<int> > keymap;
  keymap.push_back(vector<int>(1,n));
  for(int i = 0; i < n; i++){
    getline(cin,line);
    istringstream iss(line);
    int id; vector<int> ids;
    iss >> id;
    while(iss){
      int kanji_id;
      iss >> kanji_id;
      ids.push_back(kanji_id);
    }
    keymap.push_back(ids);
  }
  map_write(keymap);

  cin >> n;
  getline(cin,line);
  vector<string> kanji_map;
  kanji_map.push_back("");
  for(int i = 0; i < n; i++){
    getline(cin,line);
    istringstream iss(line);
    int id; string key;
    iss >> id >> key;
    kanji_map.push_back(key);
  }
  kanji_write(kanji_map);

  vector<pair<pair<int,int>,float> > bigram;
  while(getline(cin,line)){
    istringstream iss(line);
    int left, right;
    float freq;
    iss >> freq >> left >> right;
    bigram.push_back(make_pair(make_pair(left,right),freq));
  }
  bigram_write(bigram);

  return 0;
}
