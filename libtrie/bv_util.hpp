#ifndef _BV_UTIL_H_
#define _BV_UTIL_H_



#include <cassert>
#include <stdint.h>

namespace BV_UTIL {
  template <bool> struct _static_assert;
  template <> struct _static_assert<true> { static const bool value = true; };

  struct true_type { static const bool value = true; };
  struct false_type { static const bool value = false; };

  template <class T> struct is_unsigned : public false_type{};
  template <> struct is_unsigned<unsigned char> : public true_type{};
  template <> struct is_unsigned<const unsigned char> : public true_type{};
  template <> struct is_unsigned<unsigned short> : public true_type{};
  template <> struct is_unsigned<const unsigned short> : public true_type{};
  template <> struct is_unsigned<unsigned int> : public true_type{};
  template <> struct is_unsigned<const unsigned int> : public true_type{};
  template <> struct is_unsigned<unsigned long> : public true_type{};
  template <> struct is_unsigned<const unsigned long> : public true_type{};

  template <unsigned int N> struct lg2_static
  { enum { value  = lg2_static<(N >> 1)>::value + 1 }; };
  template <> struct lg2_static<1> { enum { value = 0 }; };

  template<class T> unsigned int lg2(T x){
    unsigned int cnt = 0; while(x >>= 1) cnt++; return cnt;
  }

  template<class T> struct PopCount;
  template<class T> struct PopCount<const T> {
    T operator()(T x) {
      return PopCount<T>()(x);
    }
  };

  // From Hacker's Delight
  template<> struct PopCount<uint8_t> {
    uint8_t operator()(uint8_t x) {
      x = (x & 0x55) + ((x >> 1) & 0x55);
      x = (x & 0x33) + ((x >> 2) & 0x33);
      x = (x & 0x0F) + ((x >> 4) & 0x0F);
      return x;
    }
  };

  template<> struct PopCount<uint16_t> {
    uint16_t operator()(uint16_t x) {
      x = (x & 0x5555) + ((x >> 1) & 0x5555);
      x = (x & 0x3333) + ((x >> 2) & 0x3333);
      x = (x & 0x0F0F) + ((x >> 4) & 0x0F0F);
      x = (x & 0x00FF) + ((x >> 8) & 0x00FF);
      return x;
    }
  };

  /*
  template<> struct PopCount<uint32_t> {
    uint32_t operator()(uint32_t x) {
      x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
      x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
      x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
      x = (x & 0x00FF00FF) + ((x >> 8) & 0x00FF00FF);
      x = (x & 0x0000FFFF) + ((x >> 16) & 0x0000FFFF);
      return x;
    }
  };
  */
  template<> struct PopCount<uint32_t> {
    uint32_t operator()(uint32_t x) {
      x = x - ((x >> 1) & 0x55555555);
      x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
      x = (x + (x >> 4)) & 0x0F0F0F0F;
      x += (x >> 8);
      x += (x >> 16);
      return x & 0x0000003F;
    }
  };

  template<> struct PopCount<uint64_t> {
    uint64_t operator()(uint64_t x) {
      x = (x & 0x5555555555555555ULL) + ((x >> 1) & 0x5555555555555555ULL);
      x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
      x = (x & 0x0F0F0F0F0F0F0F0FULL) + ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL);
      x = (x & 0x00FF00FF00FF00FFULL) + ((x >> 8) & 0x00FF00FF00FF00FFULL);
      x = (x & 0x0000FFFF0000FFFFULL) + ((x >> 16) & 0x0000FFFF0000FFFFULL);
      x = (x & 0x00000000FFFFFFFFULL) + ((x >> 32) & 0x00000000FFFFFFFFULL);
      return x;
    }
  };

  template<class T> T pop_count(T x){
    return PopCount<T>()(x);
  }

  template <class T> struct BitConst {
    static const unsigned int size = sizeof(T) * 8;
    static const T one = 1;
    static const T max = (((BitConst::one<<(BitConst::size-1))-1)<<1)+1;
  };

  template <class T>
  void set_bit(T &bs, unsigned int i, T bit){
    assert(i < BitConst<T>::size);
    assert(bit == 0 || bit == 1);
    bs |= (bit << i);
  }

  template <class T>
  bool get_bit(T bs, unsigned int i){
    assert(i < BitConst<T>::size);
    return bs & (BitConst<T>::one << i);
  }

  template <class T, unsigned int Lower, unsigned int Higher>
  struct make_mask_static {
    static const T value = (BitConst<T>::max >> (BitConst<T>::size - Higher - 1))
      ^ ((BitConst<T>::one << Lower) - 1);
  };

  template <class T>
  T make_mask_lower(unsigned int higher){
    return BitConst<T>::max >> (BitConst<T>::size - higher - 1);
  }

  template <class T>
  T make_mask(unsigned int lower, unsigned int higher){
    return (BitConst<T>::max >> (BitConst<T>::size - higher - 1))
      ^ ((BitConst<T>::one << lower) - 1);
  }

  template <class T>
  T make_mask_higher(unsigned int lower){
    return make_mask<T>(BitConst<T>::size-lower, BitConst<T>::size-1);
  }

  template<class T>
  T get_bits(T bs, unsigned lower, unsigned higher){
    return (bs & make_mask<T>(lower,higher)) >> lower;
  }

  template<class T>
  void set_bits(T &bs, T &val, unsigned lower){
    bs |= (val << lower);
  }

  template <unsigned int Lower, unsigned int Higher, class T>
  T get_bits(T bs){
    return (bs & make_mask_static<T,Lower,Higher>::value) >> Lower;
  }

  template <class T>
  void print_bit(T bs){
    for(unsigned int i = 1; i <= BitConst<T>::size; i++){
      std::cout << get_bit(bs, BitConst<T>::size  - i);
      if(i % 8 == 0)
        std::cout << ' ';
    }
    std::cout << std::endl;
  }

}
#endif /* _BV_UTIL_H_ */
