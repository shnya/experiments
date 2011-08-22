#ifndef _MMAP_H_
#define _MMAP_H_

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>


struct MMapMode {
  enum {
    WRITE,
    READ,
    READ_WRITE,
    UNKNOWN
  };
};

class MMapException : public std::exception {
  std::string message_;

public:
  MMapException(){};
  MMapException(std::string err, std::string filename = "")
  { message_ = err + " : filename = " + filename; }
  virtual ~MMapException() throw() {};
  virtual const char *what() throw()
  { return message_.c_str(); }
};

template <int Mode = MMapMode::READ, int Flag = MAP_SHARED>
class MMapFile {

  static const size_t DEFAULT_FILE_SIZE = 1024;
  int fd_;
  const char *path_;
  size_t size_, offset_, mapped_size_;
  void *ptr_;

  int GetPageSize()
  {
#ifdef BSD
    return getpagesize();
#else
    return sysconf(_SC_PAGE_SIZE);
#endif
  }

  int GetOpenMode(){
    return Mode == MMapMode::WRITE ? O_WRONLY :
      Mode == MMapMode::READ_WRITE ? O_RDWR : O_RDONLY;
  }

  int GetMMapProt(){
    return Mode == MMapMode::WRITE ? PROT_WRITE :
      Mode == MMapMode::READ_WRITE ? PROT_READ|PROT_WRITE : PROT_READ;
  }

  int GetMMapFlag(){
    return Flag == MAP_SHARED ? Flag : MAP_PRIVATE;
  }

  int GetFileSize()
  {
    struct stat st_buf;
    if(stat(path_, &st_buf) < 0){
      if(errno == ENOENT) return -1;
      throw MMapException("stat", path_);
    }

    return st_buf.st_size;
  }

  void ExpandFile() throw(MMapException)
  {
    if(lseek(fd_,offset_+size_-1,SEEK_SET) < 0){
      throw MMapException("lseek", path_);
    }
    unsigned char c;
    if(read(fd_,&c,sizeof(unsigned char)) == -1){
      c = '\0';
    }
    if(write(fd_,&c,sizeof(unsigned char)) == -1){
      throw MMapException("write", path_);
    }
    if(lseek(fd_,0,SEEK_SET) < 0){
      throw MMapException("lseek", path_);
    }
  }


  void MapFile() throw(MMapException)
  {
    int pagesize = GetPageSize();
    int flag = GetMMapFlag();
    int prot = GetMMapProt();
    mapped_size_ = ((size_  + pagesize - 1) / pagesize) * pagesize;

    ptr_ = mmap(0, mapped_size_, prot, flag, fd_, offset_);
    if(ptr_ == MAP_FAILED){
      close(fd_);
      throw MMapException("mmap", path_);
    }
  }


  void Open() throw(MMapException) {
    int mode = GetOpenMode();
    int siz = GetFileSize();
    if(siz == -1){
      if(Mode == MMapMode::READ)
        throw MMapException("can't find file", path_);
      else
        mode |= O_CREAT;
      siz = 0;
    }
    size_ = size_ <= 0 ? siz : size_;

    if((fd_=::open(path_, mode, 0644))== -1){
      throw MMapException("open", path_);
    }

    if(static_cast<int>(size_ + offset_) > siz){
      if(Mode == MMapMode::READ){
        size_ = std::max(siz - offset_ ,0UL);
      }else{
        ExpandFile();
      }
    }
  }

public:
  MMapFile(const char* path, size_t offset = 0, size_t size = 0)
    throw(MMapException)
    : path_(path), size_(size), offset_(offset)
  {
    Open();
    MapFile();
  }

  MMapFile() : path_(NULL), size_(0), offset_(0) {}

  void open(const char *path, size_t offset = 0, size_t size = 0)
    throw(MMapException) {
    path_ = path;
    size = size_;
    offset_ = offset;
    Open();
    MapFile();
  }

  ~MMapFile() throw()
  {
    //ignore exception
    try{
      munmap();
    }catch(MMapException &e){}
  }

  template <class T>
  T* data(){
    return reinterpret_cast<T*>(ptr_);
  }

  template <class T>
  T* data() const {
    return reinterpret_cast<T*>(ptr_);
  }

  size_t size(){
    return size_;
  }

  void msync() throw(MMapException)
  {
    if(::msync(ptr_, size_, MS_SYNC) == -1){
      close(fd_);
      throw MMapException("msync", path_);
    }
  }

  void munmap() throw(MMapException)
  {
    if(ptr_ == NULL) return;
    if(::munmap(ptr_, mapped_size_) == -1){
      close(fd_);
      throw MMapException("munmap", path_);
    }
    ptr_ = NULL;
    close(fd_);
  }
};

#endif /* _MMAP_H_ */


/*
  int main(int argc, char *argv[])
  {

  MMap<MMapMode::READ_WRITE,MAP_SHARED> m("a.txt", 1024);
  char *a = m.data<char *>();
  cout << m.size() << endl;
  for(size_t i = 0; i < m.size(); i++){
  *a++ = 'a';
  }
  m.msync();
  m.munmap();

  return 0;
  }
*/
