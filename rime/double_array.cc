#ifndef _DOUBLE_ARRAY_H_
#define _DOUBLE_ARRAY_H_
#include <vector>
#include <iostream>
#include <stack>
#include <string>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <cassert>

using namespace std;
template<class T> std::ostream &operator<<(std::ostream &o,const std::vector<T> &v)
{ o << "["; for(std::size_t i=0;i<v.size();i++) o << v[i] << (i < v.size()-1 ? ",":""); return o << "]"; }
template<class T,class U> std::ostream &operator<<(std::ostream &o,const std::pair<T,U> &v)
{ return o << v.first << ":" << v.second; }


class DoubleArray {
  typedef unsigned char uchar;
  struct node {
    int base;
    int check;
    char child;
    char sibling;
  };

  void set_check(int pos, int base){
    if(array[pos].check > 0){
      array[pos].check = base;
    }else if(pos == empty_head){
      empty_head = -array[pos].check;
      array[pos].check = base;
    }else{
      int i;
      i = empty_head;
      while(1){
        if(pos == -array[i].check) break;
        i = -array[i].check;
      }
      array[i].check = array[pos].check;
      array[pos].check = base;
      //cout << "set_check " << array[pos].check << " " << array[i].check << endl;
    }
  }

  void delete_check(int pos){
    if(pos < empty_head){
      array[pos].check = -empty_head;
      empty_head = pos;
    }else{
      int i;
      i = empty_head;
      while(1){
        if(i < pos && pos < -array[i].check) break;
        i = -array[i].check;
      }
      array[pos].check = array[i].check;
      array[i].check = -pos;
      //cout << "delete_check " << array[pos].check << " " << array[i].check << endl;
    }
  }

  void expand(size_t pos){
    while(pos >= array.size()){
      int i = array.size();
      array.resize(array.size() * 2);
      int n = array.size();
      for(; i < n; i++){
        array[i].check = -(i+1);
      }
    }
  }

  std::pair<int,const char *> fetch(const char *str){
    const char *p = str;
    int state = 1;
    while(*p != '\0'){
      uchar c = static_cast<uchar>(*p);
      int t = array[state].base + c;
      if(t < static_cast<int>(array.size()) && array[t].check == state){
        state = t;
        ++p;
      }else{
        return std::make_pair(-state,p);
      }
    }
    return std::make_pair(-array[state].base,p);
  }

public:

  void commonPrefixSearch(const char *str,
                          std::vector<int> &res_len,
                          std::vector<int> &res_id) const {
    const char *p = str;
    int state = 1;
    while(1){
      int t = array[state].base;
      if(state != 0 && t < static_cast<int>(array.size()) &&
         array[t].check == state){
        res_len.push_back(std::distance(str,p));
        res_id.push_back(-array[t].base);
      }
      uchar c = static_cast<uchar>(*p);
      cout << t + c << " " << *p  << " " << array[t+c].check << endl;
      if(t+c < static_cast<int>(array.size()) && array[t+c].check == state){
        if(c == 0){
          return;
        }
        state = t+c;
        ++p;
      }else{
        return;
      }
    }
  }

  std::vector<uchar> get_labels(int index){
    size_t base = array[index].base;
    size_t maxlen = min(array.size(),base + 256);
    std::vector<uchar> res;
    for(size_t i = base; i < maxlen; i++){
      if(array[i].check == index){
        res.push_back(i - base);
      }
    }
    return res;
  }

  int find_base(const std::vector<uchar> &codes, uchar c){
    int base_cand;
    int empty_index = empty_head;
    while(1){
      //cout << empty_index << endl;
      if(empty_index < 0)
        print_array();
      expand(empty_index);
      base_cand = empty_index - c;
      if(base_cand <= 0){
        empty_index = -array[empty_index].check;
        continue;
      }
      bool found = true;
      for(size_t i = 0; i < codes.size(); i++){
        expand(base_cand + codes[i]);
        if(array[base_cand + codes[i]].check >= 0){
          found = false;
          break;
        }
      }
      if(found) break;
      empty_index = -array[empty_index].check;
    }
    return base_cand;
  }

  void _insert(const char *str, int base){
    int pos = array[base].base + static_cast<uchar>(*str);
    if(array[base].base == 0 || array[pos].check >= 0){ //conflict
      int oldbase = array[base].base;
      std::vector<uchar> codes;
      if(oldbase > 0) codes = get_labels(base);
      int base_cand = find_base(codes,*str);
      array[base].base = base_cand;
      for(size_t i = 0; i < codes.size(); i++){
        int t = base_cand + codes[i];
        int old_t = oldbase + codes[i];
        set_check(t, base);
        array[t].base = array[old_t].base;
        if(array[old_t].base > 0){
          std::vector<uchar> trans = get_labels(old_t);
          for(size_t j = 0; j < trans.size(); j++){
            set_check(array[old_t].base + trans[j], t);
          }
          delete_check(old_t);
          array[old_t].base = 0;
        }
      }
      pos = base_cand + static_cast<uchar>(*str);
    }
    set_check(pos,base);
    if(*str != '\0')
      _insert(str+1,pos);
  }

  int insert(const char *str){
    std::pair<int,const char*> state = fetch(str);
    if(state.first > 0){
      return -1;
    }
    _insert(state.second, -state.first);
    return 0;
  }


  DoubleArray() : array(2) {
    array[0].check = 0;
    empty_head = 1;
    array[1].check = -2;
  }

  bool save(std::ostream &os) const {
    int i;
    int n = array.size();
    for(i = n - 1; i >= 0; i--){
      if(array[i].check >= 0) break;
    }
    std::vector<int> v((i+1) * 2 + 1);
    for(int j = 0; j <= i; j++){
      v[j*2+1] = array[j].base;
      v[j*2+2] = array[j].check;
    }
    v[0] = i;
    os.write(reinterpret_cast<const char *>(&v[0]), v.size()*sizeof(int));
    if(os.bad()) return false;
    return true;
  }

  bool load(std::istream &is){
    int i = 0;
    is.read((char *)&i, sizeof(int));
    if(is.bad()) return false;
    array.resize(i);
    std::vector<int> v((i+1) * 2);
    is.read(reinterpret_cast<char *>(&v[0]), v.size()*sizeof(int));
    if(is.bad()) return false;
    for(int j = 0; j <= i; j++){
      array[j].base = v[j*2];
      array[j].check = v[j*2+1];
    }
    return true;
  }

  void print_array(){
    cout << "[";
    for(size_t i = 0; i < array.size(); i++){
      cout << i << ":" << array[i].base << ":" << array[i].check << ",";
    }
    cout << "]" << endl;
  }

private:
  std::vector<node> array;
  int empty_head;
};

#include <fstream>
using namespace std;
int
main(int argc, char *argv[]){
  string line;
  vector<pair<string,int> > lines;
  int n = 1;
  DoubleArray da;
  while(getline(cin,line)){
    lines.push_back(make_pair(line,n++));
    da.insert(line.c_str());
    //cout << n << " " << line << endl;
  }
  //da.print_array();
  vector<int> v1;
  vector<int> v2;
  if(argc < 1 || argv[1] == NULL){
    da.commonPrefixSearch("bisons",v1,v2);
    for(size_t i = 0; i < v1.size(); i++)
      cout << v1[i] << " " << v2[i] << endl;
  }else{
    da.commonPrefixSearch(argv[1],v1,v2);
  }
  return 0;
}
#endif /* _DOUBLE_ARRAY_H_ */
