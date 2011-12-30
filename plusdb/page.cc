#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

#include "exception.hpp"
#include "platform/util.hpp"
#include "page.h"

off_t Page::pagesize_ = 0;
off_t Page::GetPageSize(){
  if(pagesize_ != 0) return pagesize_;
  pagesize_ = Env::GetPageSize();
  return pagesize_;
}

void Page::MapFile(int fd, bool is_shared){
  if(file.size() < offset_ + size_){
    file.expand(offset_ + size_);
  }
  int flag = is_shared ? MAP_SHARED : MAP_PRIVATE;
  while(1){
    ptr_ = mmap(0, size_, PROT_READ|PROT_WRITE, flag, fd, offset_);
    if(ptr_ == MAP_FAILED){
      if(errno == EINTR) continue;
      throw MMapException("mmap", file.filename());
    }else{
      break;
    }
  }
}

Page::Page(File &pf, off_t offset, off_t scale, bool is_shared)
  : file(pf), offset_(offset) {
  size_ = scale * GetPageSize();
  MapFile(file.fd(), is_shared);
}

Page::~Page() throw()
{
  //ignore exception
  try{
    munmap();
  }catch(MMapException &e){}
}

void Page::msync(){
  while(::msync(ptr_, size_, MS_SYNC) == -1){
    if(errno == EINTR) continue;
    throw MMapException("msync", file.filename());
  }
}

void Page::munmap(){
  if(ptr_ == NULL) return;
  while(::munmap(ptr_, size_) == -1){
    if(errno == EINTR) continue;
    throw MMapException("munmap", file.filename());
  }
  ptr_ = NULL;
}
