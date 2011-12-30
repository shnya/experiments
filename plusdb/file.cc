#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>

#include "exception.hpp"
#include "file.h"

off_t File::GetFileSize(){
  struct stat st_buf;
  while(fstat(fd_, &st_buf) == -1){
    if(errno == EINTR) continue;
    else if(errno == ENOENT) return -1;
    throw FileException("stat", fname_);
  }

  return st_buf.st_size;
}

void File::ExpandFile(off_t len){
  if(len <= size_) return;
  while(ftruncate(fd_,len) == -1){
    if(errno == EINTR) continue;
    throw FileException("ftruncate", fname_);
  }
  size_ = len;
}

void File::Open(){
  while((fd_=::open(fname_, O_RDWR | O_CREAT, 0644)) == -1){
    if(errno == EINTR) continue;
    throw FileException("open", fname_);
  }
  size_ = GetFileSize();
}

File::File(const char* path) : fname_(path) {
  Open();
}

File::~File() throw(){
  //ignore exception
  try{
    close();
  }catch(FileException &e){}
}

void File::write(const char *data, off_t len){
  off_t numbytes = 0;
  while((numbytes = ::write(fd_,data,len)) != len){
    if(numbytes < 0){
      if(errno == EINTR) continue;
      throw FileException("write", fname_);
    }else{
      data += numbytes;
      len -= numbytes;
    }
  }
}

void File::read(char *data, off_t len){
  off_t numbytes = 0;
  while((numbytes = ::read(fd_,data,len)) != len){
    if(numbytes < 0){
      if(errno == EINTR) continue;
      throw FileException("write", fname_);
    }else{
      data += numbytes;
      len -= numbytes;
    }
  }
}

void File::seek(off_t len){
  while(::lseek(fd_,len,SEEK_SET) == -1){
    if(errno == EINTR) continue;
    throw FileException("write", fname_);
  }
}


void File::close(){
  while(::close(fd_) == -1){
    if(errno == EINTR) continue;
    throw FileException("", fname_);
  }
}

void File::expand(off_t len){
  ExpandFile(len);
}


