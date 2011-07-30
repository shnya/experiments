#ifndef _DOUBLE_ARRAY_H_
#define _DOUBLE_ARRAY_H_
#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <iostream>

#include "dag_vector.hpp"

//#define DEBUG

#ifdef DEBUG
#include <cstdio>
#include <cstdlib>
#define dprint(format, args...) printf(format, args)
#else
#define dprint(format, args...)
#endif /* DEBUG */

class DoubleArray {

  int& empty_head() {
    return check_[0];
  }

  int& entry_num() {
    return base_[0];
  }

  void set_check(int pos, int base, bool overwrite = false){
    if(overwrite && check_[pos] > 0){
      check_[pos] = base;
    }else if(pos == empty_head()){
      empty_head() = -check_[pos];
      check_[pos] = base;
    }else{
      int i = empty_head();
      int n = base_.size();
      while(i < n){
        if(pos == -check_[i]) break;
        i = -check_[i];
      }
      if(i >= n) throw std::runtime_error("failed set check");
      check_[i] = check_[pos];
      check_[pos] = base;
    }
  }

  void delete_check(int pos){
    if(pos < empty_head()){
      check_[pos] = -empty_head();
      empty_head() = pos;
    }else{
      int i = empty_head();
      int n = base_.size();
      while(i < n){
        if(i < pos && pos < -check_[i]) break;
        i = -check_[i];
      }
      if(i >= n) throw std::runtime_error("failed delete check");
      check_[pos] = check_[i];
      check_[i] = -pos;
    }
  }

  void expand(size_t pos){
    if(pos < base_.size()) return;
    size_t i = base_.size();
    size_t n = base_.capacity();
    while(pos > n) n <<= 1;
    base_.reserve(n);
    base_.resize(pos+1);
    check_.reserve(n);
    check_.resize(pos+1);
    for(; i <= pos; i++){
      check_[i] = -(i+1);
    }
  }

  std::pair<int,const char *> fetch(const char *str) const {
    const char *p = str;
    int state = 1;
    int n = base_.size();
    while(1){
      unsigned char c = *p;
      int t = base_[state] + c;
      dprint("%d %d %d %c %d\n", state, t, base_[state], c, check_[t]);
      if(t < n && check_[t] == state){
        if(c == 0) return std::make_pair(state,p);
        state = t;
        ++p;
      }else{
        return std::make_pair(-state,p);
      }
    }
  }

  std::vector<unsigned char> get_labels(int index, int base) const {
    size_t maxlen = std::min(static_cast<int>(base_.size()), base + 256);
    std::vector<unsigned char> res;
    for(size_t i = base; i < maxlen; i++){
      if(check_[i] == index){
        res.push_back(i - base);
      }
    }
    return res;
  }

  int find_base(const std::vector<unsigned char> &codes, unsigned char c){
    int base_cand;
    int empty_index = empty_head();
    while(1){
      expand(empty_index);
      base_cand = empty_index - c;
      if(base_cand <= 1){
        empty_index = -check_[empty_index];
        continue;
      }
      bool found = true;
      for(size_t i = 0; i < codes.size(); i++){
        expand(base_cand + codes[i]);
        if(check_[base_cand + codes[i]] > 0){
          found = false;
          break;
        }
      }
      if(found) break;
      empty_index = -check_[empty_index];
    }
    return base_cand;
  }

  void move_to(int from, int from_base, int to){
    dprint("copy base[%d] = from %d\n", to, from_base);
    base_[to] = from_base;
    if(from_base > 0){
      std::vector<unsigned char> trans = get_labels(from,from_base);
      for(size_t j = 0; j < trans.size(); j++){
        dprint("move from check[%d](%c) = %d\n",
               from_base + trans[j], trans[j], to);
        set_check(from_base + trans[j], to, true);
      }
    }

    dprint("init from address %d %d %d\n", from, base_[from], from_base);
    base_[from] = 0;
    delete_check(from);
  }

  void _insert(const char *str, int base){
    int pos = base_[base] + static_cast<unsigned char>(*str);
    expand(std::max(base,pos));
    if(base_[base] == 0 || check_[pos] >= 0){ //conflict
      int oldbase = base_[base];
      std::vector<unsigned char> codes;
      if(oldbase > 0) codes = get_labels(base,oldbase);
      int base_cand = find_base(codes,*str);

      dprint("set base base[%d] = %d\n", base, base_cand);
      base_[base] = base_cand;

      std::vector<int> from,from_base;
      for(size_t i = 0; i < codes.size(); i++){
        int old_t = oldbase + codes[i];
        from.push_back(old_t);
        from_base.push_back(base_[old_t]);

        dprint("move check check[%d](%c) = %d\n",
               base_cand + codes[i], codes[i], base);
        set_check(base_cand + codes[i], base);
      }
      for(size_t i = 0; i < from.size(); i++){
        move_to(from[i], from_base[i], base_cand + codes[i]);
      }
      pos = base_cand + static_cast<unsigned char>(*str);
    }

    dprint("set check check[%d](%c) = %d\n", pos, *str, base);
    set_check(pos,base);

    if(*str != '\0'){
      _insert(str+1,pos);
    }else{
      //      base_[pos] = -(entry_num() + 1);
      //      entry_num() += 1;
    }
  }

  void _erase(const char *str, int index, const char *p){
    int newbase = check_[index];
    delete_check(base_[index]+*p);
    std::vector<unsigned char> labels = get_labels(index,base_[index]);
    if(labels.size() == 0 && str != p)
      _erase(str, newbase, --p);
  }

#ifdef DEBUG
  /* For DEBUG */
  void print_array() const {
    std::cout << "[";
    for(size_t i = 0; i < base_.size(); i++){
      std::cout << "element " << i << ":" << base_[i] << ":" << check_[i] << std::endl;
    }
    std::cout << "]" << std::endl;
  }

  void validate_array() const {
    std::cout << "start validation" << std::endl;
    for(int i = 1; i < static_cast<int>(base_.size()); i++){
      for(int j = i; j < static_cast<int>(base_.size()); j++){
        if(check_[j] == i && j > base_[i]+255){
          printf("Impossible Transitions from %d to %d (base[%d]=%d)\n", i, j, i, base_[i]);
          abort();
        }
      }
    }
  }
#endif /* DEBUG */

public:

  int exactMatch(const char *str) const {
    std::pair<int,const char*> state = fetch(str);
    if(state.first > 0){
      int t = base_[state.first] + static_cast<unsigned char>(*state.second);
      return -base_[t];
    }
    return -1;
  }

  void commonPrefixSearch(const char *str,
                          std::vector<int> &res_len,
                          std::vector<int> &res_id) const {
    const char *p = str;
    int state = 1;
    int n = base_.size();
    while(1){
      int t = base_[state];
      if(state != 0 && t < static_cast<int>(base_.size()) &&
         check_[t] == state){
        res_len.push_back(std::distance(str,p));
        res_id.push_back(-base_[t]);
      }
      unsigned char c = *p;
      if(t+c < n && check_[t+c] == state){
        if(c == 0) return;
        state = t+c;
        ++p;
      }else{
        res_id.clear();
        res_len.clear();
        return;
      }
    }
  }

  bool insert(const char *str){
    std::pair<int,const char*> state = fetch(str);
    if(state.first > 0){
      dprint("insert failed %s, Found.", str);
      return false;
    }
    dprint("insert %s", str);
    _insert(state.second, -state.first);
    return true;
  }


  bool erase(const char *str){
    std::pair<int,const char*> state = fetch(str);
    if(state.first < 0){
      dprint("erase failed %s, Not Found.", str);
      return false;
    }
    dprint("erase %s", str);
    _erase(str, state.first, state.second);

    return true;
  }

  DoubleArray() : base_(2), check_(2) {
    entry_num() = 0;
    empty_head() = 1;
    check_[1] = -2;
    expand(8192);
  }

  bool save(std::ostream &os) const {
    os.write(reinterpret_cast<const char *>(&base_[0]),
             base_.size()*sizeof(int));
    if(os.bad()) return false;
    os.write(reinterpret_cast<const char *>(&check_[0]),
             check_.size()*sizeof(int));
    if(os.bad()) return false;
    return true;
  }

  void save_dag(std::ostream &os){
    base_[0] = 0;
    check_[0] = 0;
    {
      dag::dag_vector dv_base;
      for(size_t i = 0; i < base_.size(); i++)
        dv_base.push_back(base_[i]);
      dv_base.save(os);
    }
    {
      dag::dag_vector dv_check;
      for(size_t i = 0; i < check_.size(); i++)
        dv_check.push_back(std::max(0,check_[i]));
      dv_check.save(os);
    }
  }

  bool load(std::istream &is){
    size_t siz_ = is.seekg(0,std::ios::end).tellg();
    if(is.bad()) return false;
    base_.resize(siz_ / (2 * sizeof(int)));
    check_.resize(base_.size());
    is.seekg(0, std::ios::beg);
    if(is.bad()) return false;
    is.read(reinterpret_cast<char *>(&base_[0]), base_.size()*sizeof(int));
    if(is.bad()) return false;
    is.read(reinterpret_cast<char *>(&check_[0]), check_.size()*sizeof(int));
    if(is.bad()) return false;
    return true;
  }

private:
  std::vector<int> base_;
  std::vector<int> check_;
};

class DDASearcher {
public:

  DDASearcher(std::istream &is){
    dv_base_.load(is);
    dv_check_.load(is);
  }

  int exact_match(const char *str) const {
    const char *p = str;
    size_t state = 1;
    size_t n = dv_base_.size();
    while(1){
      size_t t = dv_base_[state];
      unsigned char c = *p;
      if(t+c < n && dv_check_[t+c] == state){
        if(c == 0) return dv_base_[t];
        state = t+c;
        ++p;
      }else{
        return -1;
      }
    }
  }

private:
  dag::dag_vector dv_base_, dv_check_;
};


#endif /* _DOUBLE_ARRAY_H_ */
