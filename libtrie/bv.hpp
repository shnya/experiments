#ifndef BV_HPP
#define BV_HPP

#include <cstring>
#include <cassert>
#include <iostream>
#include "bv_util.hpp"

template<class T = uint32_t>
class BitVector {
  T *array_;
  size_t siz_;
  size_t capacity_;
  static const size_t t_size_ = sizeof(T) * 8;
  static const size_t multi_ = BV_UTIL::lg2_static<t_size_>::value;
  static const T mask_ = (1 << (multi_)) - 1;

public:
  BitVector(){
    capacity_ = t_size_;
    siz_ = 0;
    array_ = new T[1];
    array_[0] = 0;
  }

  BitVector(T bytes[], size_t len){
    siz_ = capacity_ = t_size_ * len;
    array_ = new T[sizeof(T) * len];
    memcpy(array_, bytes, sizeof(T) * len);
  }

  BitVector(const BitVector<T>& bv){
    size_t new_block_size = bv.siz_ / bv.t_size_;
    if(bv.siz_ % bv.t_size_ != 0)
      new_block_size += 1;

    assert(bv.capacity_ / t_size_ >= new_block_size);
    capacity_ = new_block_size * t_size_;
    siz_ = bv.siz_;
    array_ = new T[new_block_size];
    memcpy(array_, bv.array_, new_block_size * sizeof(T));
  }

  BitVector<T>& operator=(const BitVector<T>& bv){
    if(this == &bv) return *this;
    size_t new_block_size = bv.siz_ / bv.t_size_;
    if(bv.siz_ % bv.t_size_ != 0)
      new_block_size += 1;

    assert(bv.capacity_ / t_size_ >= new_block_size);
    capacity_ = new_block_size * t_size_;
    siz_ = bv.siz_;
    T* new_ptr = new T[new_block_size];
    memcpy(new_ptr, bv.array_, new_block_size * sizeof(T));
    delete[] array_;
    array_ = new_ptr;
    return *this;
  }

  ~BitVector(){
    delete[] array_;
  }

  size_t size() const {
    return siz_;
  }

  void clear(){
    siz_ = 0;
  }

  size_t capacity() const {
    return capacity_;
  }

  void reserve(size_t newsize){
    if(newsize <= capacity_)
      return;

    size_t new_block_size = newsize / t_size_ + 1;
    size_t capacity_block_size = capacity_ / t_size_;
    T* new_ptr = new T[new_block_size];

    assert(capacity_ % t_size_ == 0);
    assert(new_block_size > capacity_block_size);

    memcpy(new_ptr, array_, capacity_block_size * sizeof(T));
    memset(new_ptr + capacity_block_size, 0,
           (new_block_size - capacity_block_size) * sizeof(T));
    delete[] array_;
    array_ = new_ptr;
    capacity_ = new_block_size * t_size_;
  }

  void truncate(){
    size_t new_block_size = siz_ / t_size_;
    if(siz_ % t_size_ != 0)
      new_block_size += 1;

    T* new_ptr = new T[new_block_size];
    memcpy(new_ptr, array_, new_block_size * sizeof(T));
    delete[] array_;
    array_ = new_ptr;
    capacity_ = new_block_size * t_size_;
  }

  void push_back(bool bit){
    if(siz_ + 1 > capacity_)
      reserve(capacity_ * 2);

    assert(siz_ < capacity_);
    set(siz_, bit);
    siz_++;
  }

  size_t rank(bool bit, size_t index) const {
    if(siz_ <= index)
      return static_cast<size_t>(-1);
    size_t num = 0;
    for(size_t i = 0; i <= index; i++){
      if(test(i) == bit) num++;
    }
    return num;
  }

  size_t rank1(size_t index) const {
    return rank(1,index);
  }

  size_t rank0(size_t index) const {
    return rank(0,index);
  }

  size_t select(bool bit, size_t num) const {
    size_t count = 0;
    for(size_t i = 0; i < siz_; i++){
      if(test(i) == bit) count++;
      if(count == num) return i;
    }
    return siz_;
  }

  size_t select1(size_t num) const {
    return select(1,num);
  }

  size_t select0(size_t num) const {
    return select(0,num);
  }

  void set(size_t idx, bool bit){
    assert(siz_ / t_size_ + 1 > (idx >> multi_));
    assert(idx < capacity_);
    array_[idx >> multi_] |= (bit << (idx & mask_));
  }

  void clr(size_t idx){
    assert(siz_ / t_size_ + 1 > (idx >> multi_));
    assert(idx < capacity_);
    array_[idx >> multi_] &= ~(1 << (idx & mask_));
  }

  bool test(size_t idx) const {
    assert(siz_ / t_size_ + 1 > (idx >> multi_));
    assert(idx < capacity_);
    return array_[idx >> multi_] & (1<<(idx & mask_));
  }

  T get_byte(size_t idx) const {
    //std::cout << "get_byte " << idx << " " << (idx >> multi_) << " " << array_[idx >> multi_] << std::endl;
    return array_[idx >> multi_];
  }

  bool save(std::ostream &os) const {
    uint32_t n = (siz_ + t_size_ - 1) / t_size_;
    uint32_t bitnum = sizeof(T);
    os.write(reinterpret_cast<const char *>(&bitnum), sizeof(uint32_t));
    if(os.fail()) return false;
    os.write(reinterpret_cast<const char *>(&siz_), sizeof(uint32_t));
    if(os.fail()) return false;
    os.write(reinterpret_cast<const char *>(array_), sizeof(T) * n);
    if(os.fail()) return false;
    //std::cout << "write bit vector " << sizeof(T) * n << "bytes" << std::endl;

    return true;
  }

  bool load(std::istream &is) {
    uint32_t bitnum;
    is.read(reinterpret_cast<char *>(&bitnum), sizeof(uint32_t));
    if(is.fail()) return false;
    is.read(reinterpret_cast<char *>(&siz_), sizeof(uint32_t));
    if(is.fail()) return false;
    uint32_t n = (siz_ + t_size_ - 1) / t_size_;
    T *new_ptr = new T[n];
    is.read(reinterpret_cast<char *>(new_ptr), sizeof(T) * n);
    if(is.fail() && !is.eof()) return false;
    capacity_ = n * t_size_;
    delete[] array_;
    array_ = new_ptr;
    return true;
  }

  void print() const {
    for(size_t i = 0; i < siz_; i++){
      if(test(i))
        std::cout << 1;
      else
        std::cout << 0;
    }
  }
};

#endif /* BV_HPP */
