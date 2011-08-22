/*
 * WRITTEN BY Masahiko Higashiyama in 2010.
 *
 * THIS CODE IS IN PUBLIC DOMAIN.
 * THIS SOFTWARE IS COMPLETELY FREE TO COPY, MODIFY AND/OR RE-DISTRIBUTE.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _DOUBLE_ARRAY_H_
#define _DOUBLE_ARRAY_H_
#include <vector>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <iostream>

class DoubleArray {
  struct node {
    int base;
    int check;
  };

  int& empty_head() {
    return array_[0].check;
  }

  int& entry_num() {
    return array_[0].base;
  }

  void set_check(int pos, int base, bool overwrite = false){
    if(overwrite && array_[pos].check > 0){
      array_[pos].check = base;
    }else if(pos == empty_head()){
      empty_head() = -array_[pos].check;
      array_[pos].check = base;
    }else{
      int i = empty_head();
      int n = array_.size();
      while(i < n){
        if(pos == -array_[i].check) break;
        i = -array_[i].check;
      }
      if(i >= n) throw std::runtime_error("failed set check");
      array_[i].check = array_[pos].check;
      array_[pos].check = base;
    }
  }

  void delete_check(int pos){
    if(pos < empty_head()){
      array_[pos].check = -empty_head();
      empty_head() = pos;
    }else{
      int i = empty_head();
      int n = array_.size();
      while(i < n){
        if(i < pos && pos < -array_[i].check) break;
        i = -array_[i].check;
      }
      if(i >= n) throw std::runtime_error("failed delete check");
      array_[pos].check = array_[i].check;
      array_[i].check = -pos;
    }
  }

  void expand(size_t pos){
    if(pos < array_.size()) return;
    size_t i = array_.size();
    size_t n = array_.capacity();
    while(pos > n) n <<= 1;
    array_.reserve(n);
    array_.resize(pos+1);
    for(; i <= pos; i++){
      array_[i].check = -(i+1);
    }
  }

  std::pair<int,const char *> fetch(const char *str) const {
    const char *p = str;
    int state = 1;
    while(1){
      unsigned char c = *p;
      int t = array_[state].base + c;
      //printf("%d %d %d %c %d\n",
      //state, t, array_[state].base, c, array_[t].check);
      if(t < static_cast<int>(array_.size()) && array_[t].check == state){
        if(c == 0) return std::make_pair(state,p);
        state = t;
        ++p;
      }else{
        return std::make_pair(-state,p);
      }
    }
  }

  std::vector<unsigned char> get_labels(int index, int base) const {
    size_t maxlen = std::min(static_cast<int>(array_.size()), base + 256);
    std::vector<unsigned char> res;
    for(size_t i = base; i < maxlen; i++){
      if(array_[i].check == index){
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
        empty_index = -array_[empty_index].check;
        continue;
      }
      bool found = true;
      for(size_t i = 0; i < codes.size(); i++){
        expand(base_cand + codes[i]);
        if(array_[base_cand + codes[i]].check > 0){
          found = false;
          break;
        }
      }
      if(found) break;
      empty_index = -array_[empty_index].check;
    }
    return base_cand;
  }

  void move_to(int from, int from_base, int to){
    //printf("copy base[%d] = from %d\n", to, from_base);
    array_[to].base = from_base;
    if(from_base > 0){
      std::vector<unsigned char> trans = get_labels(from,from_base);
      for(size_t j = 0; j < trans.size(); j++){
        //printf("move from check[%d](%c) = %d\n",
        //from_base + trans[j], trans[j], to);
        set_check(from_base + trans[j], to, true);
      }
    }
    //printf("init from address %d %d %d\n",
    //from, array_[from].base, from_base);
    array_[from].base = 0;
    delete_check(from);
  }

  void _insert(const char *str, int base){
    int pos = array_[base].base + static_cast<unsigned char>(*str);
    expand(std::max(base,pos));
    if(array_[base].base == 0 || array_[pos].check >= 0){ //conflict
      int oldbase = array_[base].base;
      std::vector<unsigned char> codes;
      if(oldbase > 0) codes = get_labels(base,oldbase);
      int base_cand = find_base(codes,*str);
      //printf("set base base[%d] = %d\n", base, base_cand);
      array_[base].base = base_cand;
      std::vector<int> from,from_base;
      for(size_t i = 0; i < codes.size(); i++){
        int old_t = oldbase + codes[i];
        from.push_back(old_t);
        from_base.push_back(array_[old_t].base);
        //printf("move check check[%d](%c) = %d\n",
        //base_cand + codes[i], codes[i], base);
        set_check(base_cand + codes[i], base);
      }
      for(size_t i = 0; i < from.size(); i++){
        move_to(from[i], from_base[i], base_cand + codes[i]);
      }
      pos = base_cand + static_cast<unsigned char>(*str);
    }
    //printf("set check check[%d](%c) = %d\n", pos, *str, base);
    set_check(pos,base);
    if(*str != '\0'){
      _insert(str+1,pos);
    }else{
      array_[pos].base = -(entry_num() + 1);
      entry_num() += 1;
    }
  }

  void _erase(const char *str, int index, const char *p){
    int newbase = array_[index].check;
    delete_check(array_[index].base+*p);
    std::vector<unsigned char> labels = get_labels(index,array_[index].base);
    if(labels.size() == 0 && str != p)
      _erase(str, newbase, --p);
  }

  /* For DEBUG */
  void print_array() const {
    std::cout << "[";
    for(size_t i = 0; i < array_.size(); i++){
      std::cout << "element " << i << ":" << array_[i].base << ":" << array_[i].check << std::endl;
    }
    std::cout << "]" << std::endl;
  }

public:
  int exact_match(const char *str) const {
    std::pair<int,const char*> state = fetch(str);
    if(state.first > 0){
      int t = array_[state.first].base + static_cast<unsigned char>(*state.second);
      return -array_[t].base;
    }
    return -1;
  }

  void common_prefix_search(const char *str,
                          std::vector<int> &res_len,
                          std::vector<int> &res_id) const {
    const char *p = str;
    int state = 1;
    while(1){
      int t = array_[state].base;
      if(state != 0 && t < static_cast<int>(array_.size()) &&
         array_[t].check == state){
        res_len.push_back(std::distance(str,p));
        res_id.push_back(-array_[t].base);
      }
      unsigned char c = *p;
      if(t+c < static_cast<int>(array_.size()) && array_[t+c].check == state){
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
      //printf("insert failed %s, Found.", str);
      return false;
    }
    //printf("insert %s", str);
    _insert(state.second, -state.first);
    return true;
  }


  bool erase(const char *str){
    std::pair<int,const char*> state = fetch(str);
    if(state.first < 0){
      //printf("erase failed %s, Not Found.", str);
      return false;
    }
    //printf("erase %s", str);
    _erase(str, state.first, state.second);

    return true;
  }

  DoubleArray() : array_(2) {
    entry_num() = 0;
    empty_head() = 1;
    array_[1].check = -2;
    expand(8192);
  }

  bool save(std::ostream &os) const {
    os.write(reinterpret_cast<const char *>(&array_[0]),
             array_.size()*sizeof(node));
    if(os.fail()) return false;
    return true;
  }

  bool load(std::istream &is){
    size_t siz_ = is.seekg(0,std::ios::end).tellg();
    if(is.fail()) return false;
    array_.resize(siz_ / sizeof(node));
    is.seekg(0, std::ios::beg);
    if(is.fail()) return false;
    is.read(reinterpret_cast<char *>(&array_[0]), array_.size()*sizeof(node));
    if(is.fail() && !is.eof()) return false;
    return true;
  }

private:
  std::vector<node> array_;
};

#endif /* _DOUBLE_ARRAY_H_ */
