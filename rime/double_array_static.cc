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


class DoubleArray {
  typedef unsigned char uchar;
  struct node {
    int base;
    int check;
  };

  struct StackNode {
    size_t depth;
    size_t low;
    size_t up;
    int base;
    StackNode(int d, int l, int u, int b) :
      depth(d), low(l), up(u), base(b) {}
  };

  void set_check(int pos, int base){
    if(pos == empty_head){
      empty_head = -array[pos].check;
      array[pos].check = base;
    }else{
      size_t i;
      for(i = empty_head; i < array.size(); i = -array[i].check){
	if(pos == -array[i].check){
	  break;
	}
      }
      array[i].check = array[pos].check;
      array[pos].check = base;
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

  void build(std::vector<std::pair<std::string,int> > &words){
    std::sort(words.rbegin(), words.rend());
    std::stack<StackNode> st;
    st.push(StackNode(0,0,words.size(),0));
    array.resize(512);
    array[0].check=0;
    for(size_t i = 1; i < array.size(); i++)
      array[i].check=-(i+1);
    empty_head = 1;

    while(!st.empty()){
      StackNode stn = st.top(); st.pop();

      if(stn.up - stn.low == 1 && words[stn.low].first.size() < stn.depth){
	array[stn.base].base = -words[stn.low].second;
	continue;
      }

      // extract traverses
      std::vector<uchar> codes;
      std::vector<std::pair<size_t, size_t> > ranges;
      uchar c_before =  0;
      if(words[stn.low].first.size() > stn.depth)
	c_before = static_cast<uchar>((words[stn.low].first)[stn.depth]);

      size_t low_before = stn.low;
      for(size_t i = stn.low+1; i < stn.up; i++){
	uchar c = 0;
	if(stn.depth < words[i].first.size())
	  c = static_cast<uchar>((words[i].first)[stn.depth]);

	if(c != c_before){
	  codes.push_back(c_before);
	  ranges.push_back(std::make_pair(low_before,i));
	  c_before = c;
	  low_before = i;
	}
      }
      codes.push_back(c_before);
      ranges.push_back(std::make_pair(low_before, stn.up));

      // check a invalid input
      if(codes.size() == 1 && codes[0] == 0 && stn.up - stn.low > 1)
	throw std::logic_error("overlapped");
      assert(codes.size() != 0);

      // find a base cand
      int fcode = *(codes.begin());
      int base_cand = empty_head - fcode;
      int empty_index = empty_head;
      while(1){
	expand(empty_index);
	if(empty_index - fcode < 0){
	  empty_index = -array[empty_index].check;
	  continue;
	}
	bool found = true;
	base_cand = empty_index - fcode;
	for(size_t i = 1; i < codes.size(); i++){
	  expand(base_cand + codes[i]);
	  if(array[base_cand + codes[i]].check >= 0){
	    found = false;
	    break;
	  }
	}
	if(found) break;
	empty_index = -array[empty_index].check;
      }

      // set a base and checks
      array[stn.base].base = base_cand;
      for(size_t i = 0; i < codes.size(); i++){
	size_t pos = base_cand + codes[i];
	set_check(pos,stn.base);
	st.push(StackNode(stn.depth+1,ranges[i].first,
	      ranges[i].second,pos));
      }
    }
  }
  public:

  void commonPrefixSearch(const char *str,
      std::vector<int> &res_len,
      std::vector<int> &res_id) const {
    const char *p = str;
    int state = 0;
    while(1){
      int t = array[state].base;
      if(state != 0 && t < static_cast<int>(array.size()) &&
	  array[t].check == state){
	res_len.push_back(std::distance(str,p));
	res_id.push_back(-array[t].base);
      }
      uchar c = static_cast<uchar>(*p);
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

  DoubleArray(std::vector<std::pair<std::string,int> > &v) {
    build(v);
  }

  DoubleArray() : array(1) {}

  bool save(std::ostream &os){
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
    if(os.fail()) return false;
    return true;
  }

  bool load(std::istream &is){
    int i = 0;
    is.read((char *)&i, sizeof(int));
    if(is.fail()) return false;
    array.resize(i);
    std::vector<int> v((i+1) * 2);
    is.read(reinterpret_cast<char *>(&v[0]), v.size()*sizeof(int));
    if(is.fail() && !is.eof()) return false;
    for(int j = 0; j <= i; j++){
      array[j].base = v[j*2];
      array[j].check = v[j*2+1];
    }
    return true;
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
  while(getline(cin,line)){
    lines.push_back(make_pair(line,n++));
  }
  ofstream ofs("test.dat");
  DoubleArray da(lines);
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
