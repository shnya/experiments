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
#ifndef _LOUDS_HPP_
#define _LOUDS_HPP_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <deque>
#include <algorithm>

#include "rankdic.hpp"


class LOUDS {
  RankDict rd;
  std::vector<char> trans;

  struct Range {
    size_t begin;
    size_t end;
    size_t level;
    Range(size_t b, size_t e, size_t l)
      : begin(b), end(e), level(l) {}
  };

  void build(const std::vector<const char*> &words){
    BitVector<uint32_t> bv;
    bv.push_back(1);
    bv.push_back(0);
    trans.push_back('\0');
    //trans.push_back('\0');

    std::deque<Range> queue;
    queue.push_back(Range(0, words.size(), 0));
    while(!queue.empty()){
      Range r = queue.front(); queue.pop_front();
      size_t begin = r.begin;
      size_t end = r.end;
      size_t level = r.level;
      int c = words[begin][level];
      for(size_t i = begin + 1; i <= end; i++){
        if(i == end || c != words[i][level]){
          if(c != '\0'){
            queue.push_back(Range(begin, i, level+1));
          }else{
            queue.push_back(Range(begin, begin, level));
          }
          bv.push_back(true);
          trans.push_back(c);
          begin = i;
          if(i != end) c = words[i][level];
        }
      }
      //trans.push_back('\0');
      bv.push_back(false);
    }

    rd.build(bv);
  }

  bool isleaf(size_t x){
    if(rd.test(x) == 0) return true;
    return false;
  }

  int parent(size_t x){
    return rd.select1(rd.rank0(x));
  }

  int first_child(size_t x){
    int y = rd.select0(rd.rank1(x)) + 1;
    if(rd.test(y) == 0) return -1;
    return y;
  }

  int last_child(size_t x){
    int y = rd.select0(rd.rank1(x) + 1) -1;
    if(rd.test(y) == 0) return -1;
    return y;
  }

  int next_sibling(size_t x){
    if(rd.test(x+1) == 0) return -1;
    return x+1;
  }

public:
  LOUDS(const std::vector<const char*> &words){
    build(words);
  }

  LOUDS(const std::vector<std::string> &words){
    std::vector<const char *> wordsp;
    for(size_t i = 0; i < words.size(); i++)
      wordsp.push_back(words[i].c_str());
    build(wordsp);
  }

  LOUDS(){}

  void common_prefix_search(const std::string &str, 
                            std::vector<std::string> &vec){
    int index = 1;
    vec.clear();
    const char *p = str.c_str();

    for(size_t i = 0; i < str.size() + 1; i++,p++){
      int c = rd.select0(index) + 1;
      int label_id = rd.rank1(c) - 1;
      //cout << "first " << c << endl;
      int c2 = -1, term = -1;
      while(c != -1){
        char label = trans[label_id];
        if(label == *p){
          c2 = label_id;
        }else if(label == '\0'){
          term = label_id;
        }
        label_id++;
        c = next_sibling(c);
      }
      if(term != -1 && !rd.test(rd.select0(term + 1) + 1)){
        vec.push_back(str.substr(0, i));
      }
      if(c2 == -1) return;
      index = c2 + 1;
    }
    //cout << "end " << index << endl;
    if(!rd.test(rd.select0(index) + 1)){
      vec.push_back(str);
    }
  }

  bool exact_match(const std::string &str){
    int index = 1;
    const char *p = str.c_str();

    for(size_t i = 0; i < str.size() + 1; i++,p++){
      int c = rd.select0(index) + 1;
      int label_id = rd.rank1(c) - 1;
      while(c != -1){
        char label = trans[label_id];
        if(label == *p){
          break;
        }
        c = next_sibling(c);
        label_id++;
      }
      //std::cout << *p << " " << trans[label_id] << std::endl;
      if(c == -1) return false;
      index = label_id + 1;
    }
    if(!rd.test(rd.select0(index) + 1)){
      return true;
    }
    return false;
  }

  bool save(std::ostream &os) const {
    uint32_t n = trans.size();
    os.write(reinterpret_cast<const char *>(&n), sizeof(uint32_t));
    if(os.fail()) return false;
    os.write(reinterpret_cast<const char *>(&trans[0]), sizeof(char)*n);
    if(os.fail()) return false;
    //std::cout << "write label " << sizeof(char) * n << "bytes" << std::endl;

    return rd.save(os);
  }

  bool load(std::istream &is) {
    uint32_t n;
    is.read(reinterpret_cast<char *>(&n), sizeof(uint32_t));
    if(is.fail()) return false;
    trans.resize(n);
    is.read(reinterpret_cast<char *>(&trans[0]), sizeof(char) * n);
    if(is.fail()) return false;
    return rd.load(is);
  }
};

#endif /* _LOUDS_HPP_ */
