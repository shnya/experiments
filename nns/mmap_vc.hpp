#ifndef _MMAP_VC_H_
#define _MMAP_VC_H_
#include "stdint.h"
#include "mmap.hpp"
#include "vc.hpp"

class MMapVC {
  MMapFile<MMapMode::READ,MAP_PRIVATE> mmap_;
  VC_OP op_;

public:
  MMapVC(const char *path, size_t offset = 0, size_t size = 0)
    : mmap_(path,offset,size) {
    const uint32_t *ptr = mmap_.data<uint32_t>();
    uint32_t bit1_num = ptr[0];
    uint32_t max_num = ptr[1];
    uint32_t bitset_size = ptr[2];
    uint32_t base_size = ptr[3];
    const uint32_t *bitset = &ptr[5];
    const uint32_t *base = bitset + bitset_size;
    const uint32_t *accm = base + base_size;
    op_.init(bitset,base,accm,bit1_num,max_num);
  }

  int select(size_t num) const {
    return op_.select(num);
  }

  int rank(size_t idx) const {
    return op_.rank(idx);
  }

  int psum(size_t idx) const {
    return op_.select(idx);
  }

  size_t size() const {
    return op_.size();
  }

  void release() {
    mmap_.munmap();
  }

};

#endif /* _MMAP_VC_H_ */
