#ifndef _MMAP_VECTOR_H_
#define _MMAP_VECTOR_H_
#include "mmap.hpp"

template<class T>
class MMapVector {
  MMapFile<MMapMode::READ,MAP_PRIVATE> mmap;
  const T *ptr_;
  size_t siz_;

public:
  MMapVector(const char *path, size_t offset = 0, size_t size = 0)
    : ptr_(NULL), siz_(0) {
    mmap.open(path,offset,size);
    ptr_ = mmap.data<const T>();
    siz_ = mmap.size() / sizeof(T);
  }

  const T& operator[](size_t i) const {
    return ptr_[i];
  }

  const T* at_ptr(size_t i) const {
    return &ptr_[i];
  }

  size_t size() const {
    return siz_;
  }

  void release() throw(MMapException) {
    mmap.munmap();
  }
};

template <class T>
class MMapMatrix {
  MMapFile<MMapMode::READ,MAP_PRIVATE> mmap;
  const T *ptr_;
  size_t row_;
  size_t col_;

public:
  MMapMatrix(const char *path, size_t offset = 0, size_t size = 0)
    : ptr_(NULL), row_(0),col_(0) {
      mmap.open(path,offset,size);
    int *data = mmap.data<int>();
    row_ = *data;
    col_ = *(data + 1);
    void *ptr = static_cast<void *>(data + 2);
    ptr_ = reinterpret_cast<T*>(ptr);
  }

  const T* row(size_t i) const {
    return &ptr_[i*col_];
  }

  const T& at(size_t i, size_t j) const {
    return ptr_[i*col+j];
  }

  size_t row() const {
    return row_;
  }

  size_t col() const {
    return col_;
  }

  void release() throw(MMapException) {
    mmap.munmap();
  }
};

#endif /* _MMAP_VECTOR_H_ */
