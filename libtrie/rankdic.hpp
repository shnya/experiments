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
#ifndef _RANKDIC_HPP_
#define _RANKDIC_HPP_

#include <iostream>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include "bv.hpp"
#include "profile.hpp"


class RankDict {
  struct Bucket {
    uint64_t rel_;
    uint32_t accm_;
    Bucket(uint32_t accm, uint64_t rel)
      : rel_(rel),accm_(accm) {}
    Bucket() {}
  };

  void build(){
    uint64_t count1 = bv_.test(0);
    int count1_all = count1;
    int count1_before = 0;
    int count0_all = (count1 ? 0 : 1);
    uint64_t rel = 0;
    select1_dic_.push_back(0);
    select0_dic_.push_back(0);

    for(size_t i = 1; i < bv_.size(); i++){
      if(i % 512 == 0){
        //cout << "count_all " << count_all << " " << rel << endl;
        buckets_.push_back(Bucket(count1_before,rel));
        count1_before = count1_all;
        rel = count1 = 0;
      }else if(i % 64 == 0){
        //cout << "cout " << i << " " << count << " " << (i%512)/64-1 << endl;
        //cout << rel << endl;
        BV_UTIL::set_bits(rel,count1,((i%512)/64-1)*9);
        //cout << rel << endl;
      }
      if((i & (select_block_num - 1)) == 0){
        select1_dic_.push_back(count1_all);
        select0_dic_.push_back(count0_all);
      }
      if(bv_.test(i)){
        count1++;
        count1_all++;
      }else{
        count0_all++;
      }
    }
    //cout << select1_dic_.size() << " " << select0_dic_.size() << endl;
    buckets_.push_back(Bucket(count1_before,rel));
    //validate();
  }

  void validate() const {
    for(size_t i = 0; i < bv_.size(); i++){
      std::cout << i << " " << bv_.rank1(i) << " " << rank1(i) << std::endl;
      if(bv_.rank1(i) != rank1(i))     throw std::runtime_error("error rank1");
      if(bv_.rank0(i) != rank0(i))     throw std::runtime_error("error rank0");
      size_t select_1 = bv_.select1(i);
      size_t select_0 = bv_.select0(i);
      if(select_1 != bv_.size())
        if(select_1 != select1(i)){
          std::cout << select_1 << " " << select1(i) << " " << bv_.size() << std::endl;
          throw std::runtime_error("error select1");
        }
      if(select_0 != bv_.size())
        if(select_0 != select0(i)){
          std::cout << select_0 << " " << select0(i) << " " << bv_.size() << std::endl;
          throw std::runtime_error("error select0");
        }
    }
  }

public:

  RankDict(const BitVector<uint32_t> &bv) : bv_(bv){
    build();
  }

  RankDict() {}

  void build(const BitVector<uint32_t> &bv){
    bv_ = bv;
    build();
  }

  ~RankDict(){
    //PrintTime(0,t_total);
  }

  uint32_t rank1(uint32_t pos) const {
    //double t1,t2;
    uint32_t rel_idx = (pos & first_dir_mask) >> second_dir_bitnum;
    uint32_t mod = pos & second_dir_mask;
    const Bucket &b = buckets_[pos >> first_dir_bitnum];
    uint32_t res = b.accm_;
    if(rel_idx > 0){
      //std::cout << "mask " << (rel_idx-1) * 9 << " " << ((rel_idx * 9) - 1) << std::endl;
      //BV_UTIL::print_bit(BV_UTIL::make_mask<uint32_t>(0,8));
      res += BV_UTIL::get_bits(b.rel_, (rel_idx-1) * 9, rel_idx * 9 - 1);
    }
    //std::cout << "res1 " << res << endl;
    //std::cout << bv_.test(pos) << endl;
    //t1 = GetusageSec();
    res += BV_UTIL::pop_count(bv_.get_byte(pos - mod)
                              & BV_UTIL::make_mask_lower<uint32_t>(std::min(31U,mod)));
    //std::cout << "res2 " << res << endl;
    if(mod > 31)
      res += BV_UTIL::pop_count(bv_.get_byte(pos - mod + 32)
                                & BV_UTIL::make_mask_lower<uint32_t>(mod-32U));
    //t2 = GetusageSec();
    //t_total += t2 - t1;
    return res;
  }

  uint32_t rank0(uint32_t pos) const {
    return pos - rank1(pos) + 1;
  }

  uint32_t rank(bool bit, uint32_t pos) const {
    if(bit) return rank1(pos);
    return rank0(pos);
  }

  uint32_t select(bool bit, uint32_t pos, const std::vector<uint32_t> &dic) const {
    //if((pos & (select_block_num - 1)) == 0) return dic[pos];
    size_t dicidx = std::distance(dic.begin(),
                                  lower_bound(dic.begin(),dic.end(),pos));
    int low = 0, high = bv_.size() - 1;
    if(dicidx < dic.size()) high = dicidx * select_block_num + 1;
    if(dicidx > 0) low = (dicidx - 1) * select_block_num;
    //cout << "low " << low << " high " << high << endl;
    int idx = 0;
    while(low < high){
      idx = (high + low) / 2;
      if(rank(bit,idx) < pos){
        low = idx + 1;
      }else{
        high = idx;
      }
    }
    return low;
  }

  uint32_t select1(uint32_t val) const{
    return select(1,val,select1_dic_);
  }

  uint32_t select0(uint32_t val) const {
    return select(0,val,select0_dic_);
  }

  uint32_t test(uint32_t idx) const {
    return bv_.test(idx);
  }


  bool save(std::ostream &os) const {
    uint32_t n = buckets_.size();
    os.write(reinterpret_cast<const char *>(&n), sizeof(uint32_t));
    if(os.fail()) return false;
    os.write(reinterpret_cast<const char *>(&buckets_[0]), sizeof(Bucket) * n);
    if(os.fail()) return false;
    //std::cout << "write rank dic " << sizeof(Bucket) * n << "bytes" << std::endl;

    n = select0_dic_.size();
    os.write(reinterpret_cast<const char *>(&n), sizeof(uint32_t));
    if(os.fail()) return false;
    os.write(reinterpret_cast<const char *>(&select0_dic_[0]),
             sizeof(uint32_t) * n);
    if(os.fail()) return false;
    //std::cout << "write select0 dic " << sizeof(uint32_t) * n << "bytes" << std::endl;

    n = select1_dic_.size();
    os.write(reinterpret_cast<const char *>(&n), sizeof(uint32_t));
    if(os.fail()) return false;
    os.write(reinterpret_cast<const char *>(&select1_dic_[0]),
             sizeof(uint32_t) * n);
    if(os.fail()) return false;
    //std::cout << "write select1 dic " << sizeof(uint32_t) * n << "bytes" << std::endl;
    return bv_.save(os);
  }

  bool load(std::istream &is) {
    uint32_t n;

    is.read(reinterpret_cast<char *>(&n), sizeof(uint32_t));
    if(is.fail()) return false;
    buckets_.resize(n);
    is.read(reinterpret_cast<char *>(&buckets_[0]), sizeof(Bucket) * n);
    if(is.fail()) return false;

    is.read(reinterpret_cast<char *>(&n), sizeof(uint32_t));
    if(is.fail()) return false;
    select0_dic_.resize(n);
    is.read(reinterpret_cast<char *>(&select0_dic_[0]), sizeof(uint32_t) * n);
    if(is.fail()) return false;

    is.read(reinterpret_cast<char *>(&n), sizeof(uint32_t));
    if(is.fail()) return false;
    select1_dic_.resize(n);
    is.read(reinterpret_cast<char *>(&select1_dic_[0]), sizeof(uint32_t) * n);
    if(is.fail()) return false;
    return bv_.load(is);
  }


private:
  //double t_total;
  BitVector<uint32_t> bv_;
  std::vector<Bucket> buckets_;
  std::vector<uint32_t> select0_dic_, select1_dic_;
  static const uint32_t select_block_num = 512;
  static const uint32_t first_dir_mask = 512 - 1;
  static const uint32_t first_dir_bitnum = 9;
  static const uint32_t second_dir_mask = 64 - 1;
  static const uint32_t second_dir_bitnum = 6;
};


#endif /* _RANKDIC_HPP_ */
