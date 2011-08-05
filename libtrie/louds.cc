#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <deque>
#include <algorithm>

#include "bv.hpp"
#include "rankdic.hpp"


using namespace std;

class LOUDS {
  RankDict rd;
  BitVector<> bv;
  vector<char> trans;

  struct Range {
    size_t begin;
    size_t end;
    size_t level;
    Range(size_t b, size_t e, size_t l)
      : begin(b), end(e), level(l) {}
  };

  void build(const vector<const char*> &words){
    bv.clear();
    bv.push_back(1);
    bv.push_back(0);
    trans.push_back('\0');
    trans.push_back('\0');
    //cout << "  ";
    //bv.print(); cout << endl;

    deque<Range> queue;
    queue.push_back(Range(0, words.size(), 0));
    while(!queue.empty()){
      //bv.print();
      //cout << endl;
      Range r = queue.front(); queue.pop_front();
      size_t begin = r.begin;
      size_t end = r.end;
      size_t level = r.level;
      int c = words[begin][level];
      //cout << (char)c << " " << begin << " " << end << endl;
      for(size_t i = begin + 1; i <= end; i++){
        //cout << (char)c << " " << i << endl;
        if(i == end || c != words[i][level]){
          if(c != '\0'){
            //for(size_t j = 0; j < level; j++)  cout << ' ';
            //cout << (char)c << " " << words[i-1][level-1] << " "
            //<< words[i-1] << endl;
            //cout << (char)c;
            queue.push_back(Range(begin, i, level+1));
          }else{
            queue.push_back(Range(begin, begin, level));
          }
          bv.push_back(true);
          //cout << "push 1 ";
          //bv.print();
          //cout << endl;
          trans.push_back(c);
          begin = i;
          if(i != end) c = words[i][level];
        }
      }
      //cout << " ";
      trans.push_back('\0');
      //cout << "push 0 ";
      bv.push_back(false);
      //bv.print();
      //cout << endl;
    }
    //cout << endl;
    /*
      for(size_t i = 0; i < trans.size(); i++) {
      if(trans[i] == '\0') cout << '0';
      else cout << trans[i];
      }
      //cout << bv.size() << " " << trans.size() << endl;
      cout << endl;
    */
    //bv.print();
    //cout << endl;
    rd.build(&bv);
  }

  bool isleaf(size_t x){
    if(bv.test(x) == 0) return true;
    return false;
  }

  int parent(size_t x){
    return rd.select1(rd.rank0(x));
  }

  int first_child(size_t x){
    int y = rd.select0(rd.rank1(x)) + 1;
    if(bv.test(y) == 0) return -1;
    return y;
  }

  int last_child(size_t x){
    int y = rd.select0(rd.rank1(x) + 1) -1;
    if(bv.test(y) == 0) return -1;
    return y;
  }

  int next_sibling(size_t x){
    if(bv.test(x+1) == 0) return -1;
    return x+1;
  }

public:
  LOUDS(const vector<const char*> &words){
    build(words);
  }

  LOUDS(const vector<string> &words){
    vector<const char *> wordsp;
    for(size_t i = 0; i < words.size(); i++)
      wordsp.push_back(words[i].c_str());
    build(wordsp);
  }

  void common_prefix_search(const string &str, vector<string> &vec){
    int index = 0;
    vec.clear();
    const char *p = str.c_str();

    for(size_t i = 0; i < str.size() + 1; i++,p++){
      int c = first_child(index);
      //cout << "first " << c << endl;
      int c2 = -1, term = -1;
      while(c != -1){
        char label = trans[c];
        if(label == *p){
          c2 = c;
        }else if(label == '\0'){
          term = c;
        }
        c = next_sibling(c);
      }
      if(term != -1 && first_child(term) == -1){
        vec.push_back(str.substr(0, i));
      }
      if(c2 == -1) return;
      index = c2;
    }
    //cout << "end " << index << endl;
    if(index != -1 && first_child(index) == -1){
      vec.push_back(str);
    }
  }

  bool exact_match(const string &str){
    int index = 0;
    const char *p = str.c_str();

    for(size_t i = 0; i < str.size() + 1; i++,p++){
      int c = first_child(index);
      int c2 = -1;
      while(c != -1){
        char label = trans[c];
        if(label == *p){
          c2 = c;
        }
        c = next_sibling(c);
      }
      if(c2 == -1) return false;
      index = c2;
    }
    if(index != -1 && first_child(index) == -1){
      return true;
    }
    return false;
  }
};

#include "profile.hpp"



int main(int argc, char *argv[])
{
  string line;
  double t1,t2;
  vector<string> lines;
  while(getline(cin, line)) {
    lines.push_back(line);
  }
  t1 = GetusageSec();
  std::stable_sort(lines.begin(), lines.end());
  LOUDS dic(lines);
  t2 = GetusageSec();
  PrintTime(t1,t2);

  vector<string> vec;

  t1 = GetusageSec();
  int n = 0;
  for(vector<string>::iterator itr = lines.begin();  itr != lines.end(); ++itr){
    //dic.common_prefix_search(itr->c_str(), vec);
    if(!dic.exact_match(*itr)){
      //if(id == 0){
      cout << "error" << endl;
    }
    if(n++ % 100000 == 0){
      cout << n << endl;
    }
    /*
      cout << *itr << "\t";
      if(vec.size() > 0){
      cout << "Found: num=" << vec.size() << endl;
      }else{
      cout << "Not Found" << endl;
      }
      for(vector<string>::iterator itr = vec.begin();
      itr != vec.end(); ++itr) {
      cout << *itr << endl;
      }
    */
  }
  t2 = GetusageSec();
  PrintTime(t1,t2);

  return 0;
}
