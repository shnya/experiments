#ifndef _VC_H_
#define _VC_H_
#include <stdint.h>
#include <vector>
#include <bitset>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>

class VC_OP {
  const static size_t t_size_ = sizeof(uint32_t) * 8;
  const static uint32_t bit1_ = 1;
  const uint32_t *bitset_, *accm_, *base_;
  uint32_t bit1_num_;
  uint32_t max_num_;

  static uint32_t pop_count(uint32_t x){
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x += (x >> 8);
    x += (x >> 16);
    return x & 0x0000003F;
  }

public:
  void init(const uint32_t *bitset, const uint32_t *base, const uint32_t *accm,
            uint32_t &bit1_num, uint32_t &max_num){
    bitset_ = bitset;
    base_ = base;
    accm_ = accm;
    bit1_num_ = bit1_num;
    max_num_ = max_num;
  }

  int select(uint32_t num) const {
    if(num > bit1_num_)
      return -1;

    //std::cout << accm.size() << "\t" << num/t_size << std::endl;
    //assert(num/t_size < accm.size());
    //assert(num/t_size+1 < base.size());

    size_t i = num / t_size_;
    uint32_t mask = (bit1_ << (num & (t_size_ - 1))) - 1;
    uint32_t val = accm_[i];
    //std::cout << i << "\t" << val << std::endl;
    //std::cout << base[i] << "\t" << base[i+1] << "\t" << std::endl;
    size_t base_min = base_[i], base_max = base_[i+1];
    for(size_t j = base_min; j < base_max; j++){
      val += pop_count(bitset_[j] & mask) << (j - base_min);
      //std::cout << base[i] << "\t" << base[i+1] << "\t" << j << "\t" << val << "\t" << num % t_size << std::endl;
    }
    return static_cast<int>(val + num);
  }

  int rank(size_t idx) const {
    if(idx > max_num_){
      return -1;
    }
    size_t min=0, max=max_num_;
    while(min < max){
      size_t num = (min + max) / 2;
      size_t res = static_cast<size_t>(select(num));
      if(res < idx){
        min = num + 1;
      }else{
        max = num;
      }
    }
    return min;
  }

  size_t size() const {
    return bit1_num_;
  }
};

class VerticalCode {
  std::vector<uint32_t> bitset;
  std::vector<uint32_t> base;
  std::vector<uint32_t> accm;
  uint32_t bit1_num,max_num;
  const static size_t t_size = sizeof(uint32_t) * 8;
  const static uint32_t bit1 = 1;
  VC_OP op;


  static void setbit(uint32_t &bs, size_t i, uint32_t bit){
    assert(i < t_size);
    assert(bit == 0 || bit == 1);
    bs |= (bit << i);
  }

  static bool getbit(uint32_t bs, size_t i){
    assert(i < t_size);
    return bs & (bit1 << i);
  }

  // For Debug
  /*
    static void printbit(uint32_t bs) {
    for(size_t i = 1; i <= t_size; i++){
    std::cout << getbit(bs, t_size  - i);
    if(i % 8 == 0)
    std::cout << ' ';
    }
    std::cout << std::endl;
    }
  */

  size_t CalcAccm(size_t idx, const std::vector<uint32_t>& array)
  {
    unsigned int total = 0;
    unsigned int max = 0;
    for(size_t i = 0; i < t_size; i++){
      assert((idx + i) < array.size() + t_size);
      unsigned int num = array[idx + i];
      total += num;
      if(max < num)
        max = num;
    }
    accm.push_back(total + accm.back());

    size_t maxbit = 0;
    while(max != 0){
      maxbit++;
      max >>= 1;
    }
    return maxbit;
  }

  void ConvVertical(size_t idx, std::vector<uint32_t>& array, size_t maxbit)
  {
    uint32_t mod = 1;
    unsigned int M = 0;
    for(size_t j = 0; j < maxbit; j++){
      uint32_t set = 0;
      for(size_t i = 0; i < t_size; i++){
        //std::cout << static_cast<int32_t>(array[idx + i]) << std::endl;
        //std::cout << array[idx + i] << "\t" << (mod << 1) << std::endl;
        assert((idx + i) < array.size() + t_size);
        assert(static_cast<int32_t>(array[idx + i]) >= 0);
        bool m = (array[idx + i] % (mod << 1));
        setbit(set, i, m);
        array[idx + i] -= static_cast<uint32_t>(m) * mod;
      }
      bitset.push_back(set);
      M++;
      mod <<= 1;
      //std::cout << M << "\t";
      //print_bit(set);
    }
    base.push_back(M + base.back());
  }

  // build array for bit_0 num.
  size_t BuildDiffVector(const std::vector<bool>& bits,
                         std::vector<uint32_t>& vec)
  {
    //std::cout << bits.size() << std::endl;
    size_t num = bits.size();
    int back = -1;
    for(size_t i = 0; i < num; i++){
      if(bits[i]){
        assert((static_cast<int>(i) - back - 1) >= 0);
        //std::cout << i << "\t" << back << "\t" << (i - back - 1) << std::endl;
        vec.push_back(i - back - 1);
        back = i;
      }
    }
    size_t count = vec.size();
    vec.resize((vec.size() + t_size - 1) / t_size * t_size, 0);
    return count;
  }

  void Initialize(){
    bitset.clear();
    base.clear();
    accm.clear();
    bit1_num = 0;
    max_num = 0;
  }

  void _build(std::vector<uint32_t> &diff_vec){
    accm.push_back(0);
    base.push_back(0);

    for(size_t i = 0; i < diff_vec.size() / t_size; i++){
      size_t maxbit = CalcAccm(i*t_size, diff_vec);
      ConvVertical(i*t_size, diff_vec, maxbit);
    }
    op.init(&bitset[0],&base[0],&accm[0],bit1_num,max_num);
  }


public:
  int select(size_t num) const {
    return op.select(num);
  }

  int rank(size_t idx) const {
    return op.rank(idx);
  }

  void truncate() {
    // truncate extra space
    std::vector<uint32_t>(bitset).swap(bitset);
    std::vector<uint32_t>(base).swap(base);
    std::vector<uint32_t>(accm).swap(accm);
  }

  int psum(size_t idx) const {
    return op.select(idx);
  }

  VerticalCode(const std::vector<bool>& bits)
  {
    build(bits);
  }

  template<class U>
  VerticalCode(const std::vector<U>& partial_sum){
    build(partial_sum);
  }


  VerticalCode() : bit1_num(0), max_num(0) {}

  template<int N>
  void build(const std::bitset<N>& bits){
    //std::cout << bits << std::endl;
    std::vector<bool> bitarray;
    for(size_t i = 0; i < N; i++){
      //std::cout << bits[i];
      bitarray.push_back(bits[i]);
    }
    //std::cout << std::endl;
    build(bitarray);
  }

  template<class U>
  void build(const std::vector<U>& partial_sum){
    Initialize();
    std::vector<uint32_t> vec(partial_sum.size());
    vec[0] = partial_sum[0];
    for(size_t i = 1; i < partial_sum.size(); i++){
      vec[i] = partial_sum[i] - partial_sum[i-1] - 1;
    }
    bit1_num = vec.size();
    max_num = partial_sum.back()+1;
    vec.resize((vec.size() + t_size - 1) / t_size * t_size, 0);
    _build(vec);
  }

  void build(const std::vector<bool>& bits)
  {
    Initialize();
    std::vector<uint32_t> vec;
    bit1_num = BuildDiffVector(bits, vec);
    max_num = bits.size();
    _build(vec);
  }

  bool save(std::ostream &os){
    uint32_t buf[5];
    buf[0] = bit1_num;
    buf[1] = max_num;
    buf[2] = bitset.size();
    buf[3] = base.size();
    buf[4] = accm.size();
    return os.write(reinterpret_cast<char *>(buf), sizeof(uint32_t)*5)
      && os.write(reinterpret_cast<char *>(&bitset[0]), sizeof(uint32_t) * bitset.size())
      && os.write(reinterpret_cast<char *>(&base[0]), sizeof(uint32_t) * base.size())
      && os.write(reinterpret_cast<char *>(&accm[0]), sizeof(uint32_t) * accm.size());
  }
};

#endif /* _VC_H_ */
