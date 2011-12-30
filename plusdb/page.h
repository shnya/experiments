#ifndef _PAGE_H_
#define _PAGE_H_

#include <stdint.h>
#include "file.h"


class Page {
  char *ptr_;
  off_t offset_, size_;
  File &file;
  uint32_t pageinfo;
  void MapFile(int fd, bool is_shared);

public:
  static off_t pagesize_;
  static off_t GetPageSize();

  Page(File &pf, off_t offset, off_t scale = 1, bool is_shared = true);
  ~Page() throw();
  void msync();
  void munmap();

  template <class T>
  T* data(){
    return reinterpret_cast<T*>(ptr_);
  }

  bool is_last(){
    return (pageinfo & (1UL << 31)) == 0;
  }


  uint32_t content_len(){
    return (pageinfo & ((1UL << 31) - 1));
  }

  off_t size() const {
    return size_;
  }
};


#endif /* _PAGE_H_ */
