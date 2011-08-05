#include <iostream>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include "bv.hpp"
#include "profile.hpp"


using namespace std;


class RankDict {
  struct Bucket {
    uint32_t accm_;
    uint64_t rel_;
    Bucket(uint32_t accm, uint64_t rel)
      : accm_(accm),rel_(rel) {}
  };

  void build(){
    uint64_t count1 = bv_->test(0);
    int count1_all = count1;
    int count1_before = 0;
    int count0_all = (count1 ? 0 : 1);
    uint64_t rel = 0;
    select1_dic_.push_back(0);
    select0_dic_.push_back(0);

    for(size_t i = 1; i < bv_->size(); i++){
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
      if(bv_->test(i)){
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

  void validate(){

    for(size_t i = 0; i < bv_->size(); i++){
      std::cout << i << " " << bv_->rank1(i) << " " << rank1(i) << std::endl;
      if(bv_->rank1(i) != rank1(i))     throw std::runtime_error("error rank1");
      if(bv_->rank0(i) != rank0(i))     throw std::runtime_error("error rank1");
      size_t select_1 = bv_->select1(i);
      size_t select_0 = bv_->select0(i);
      if(select_1 != bv_->size())
        if(select_1 != select1(i)){
          std::cout << select_1 << " " << select1(i) << " " << bv_->size() << std::endl;
          throw std::runtime_error("error select1");
        }
      if(select_0 != bv_->size())
        if(select_0 != select0(i)){
          std::cout << select_0 << " " << select0(i) << " " << bv_->size() << std::endl;
          throw std::runtime_error("error select0");
        }
    }
  }

public:

  void build(const BitVector<> *bv){
    bv_ = bv;
    build();
  }

  RankDict(const BitVector<> *bv) : bv_(bv){
    build();
  }
  RankDict() {}

  ~RankDict(){
    PrintTime(0,t_total);
  }

  uint32_t rank1(uint32_t pos){
    //double t1,t2;
    uint32_t rel_idx = (pos & first_dir_mask) >> second_dir_bitnum;
    uint32_t mod = pos & second_dir_mask;
    Bucket &b = buckets_[pos >> first_dir_bitnum];
    uint32_t res = b.accm_;
    if(rel_idx > 0){
      //std::cout << "mask " << (rel_idx-1) * 9 << " " << ((rel_idx * 9) - 1) << std::endl;
      //BV_UTIL::print_bit(BV_UTIL::make_mask<uint32_t>(0,8));
      res += BV_UTIL::get_bits(b.rel_, (rel_idx-1) * 9, rel_idx * 9 - 1);
    }
    //std::cout << "res1 " << res << endl;
    //std::cout << bv_->test(pos) << endl;
    //t1 = GetusageSec();
    res += BV_UTIL::pop_count(bv_->get_byte(pos - mod)
                              & BV_UTIL::make_mask_lower<uint32_t>(std::min(31U,mod)));
    //std::cout << "res2 " << res << endl;
    if(mod > 31)
      res += BV_UTIL::pop_count(bv_->get_byte(pos - mod + 32)
                                & BV_UTIL::make_mask_lower<uint32_t>(mod-32U));
    //t2 = GetusageSec();
    //t_total += t2 - t1;
    return res;
  }

  uint32_t rank0(uint32_t pos){
    return pos - rank1(pos) + 1;
  }

  uint32_t rank(bool bit, uint32_t pos){
    if(bit) return rank1(pos);
    return rank0(pos);
  }

  uint32_t select(bool bit, uint32_t pos, const vector<uint32_t> &dic){
    //if((pos & (select_block_num - 1)) == 0) return dic[pos];
    size_t dicidx = std::distance(dic.begin(),
                                  lower_bound(dic.begin(),dic.end(),pos));
    int low = 0, high = bv_->size() - 1;
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

  uint32_t select1(uint32_t val){
    return select(1,val,select1_dic_);
  }

  uint32_t select0(uint32_t val){
    return select(0,val,select0_dic_);
  }

private:
  double t_total;
  const BitVector<> *bv_;
  std::vector<Bucket> buckets_;
  std::vector<uint32_t> select0_dic_,select1_dic_;
  static const uint32_t select_block_num = 512;
  static const uint32_t first_dir_mask = 512 - 1;
  static const uint32_t first_dir_bitnum = 9;
  static const uint32_t second_dir_mask = 64 - 1;
  static const uint32_t second_dir_bitnum = 6;
};

/*
  using namespace std;
  using namespace BV_UTIL;


  int main(int argc, char *argv[])
  {
  uint32_t a = 0, b = 7;

  //cout << get_bits(a, 0, 2) << endl;

  vector<uint32_t> v;
  for(int i = 0; i < 1000; i++){
  v.push_back(i);
  }
  BitVector<> bv(&v[0],v.size());
  RankDict rd(&bv);

  for(size_t i = 0; i < 32000; i++){
  cout << i << endl;
  if(bv.select(1,i) == static_cast<size_t>(-1)) break;
  if(bv.select1(i) != rd.select1(i)){
  cout << "wrong! " << bv.select1(i) << " " << rd.select1(i) << endl;
  }
  }
  return 0;
  }
*/
