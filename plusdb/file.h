#ifndef _FILE_H_
#define _FILE_H_

#include <cstdio>

class File {
  int fd_;
  const char *fname_;
  off_t size_;

  off_t GetFileSize();
  void ExpandFile(off_t len);
  void Open();

public:
  File(const char* path);
  ~File() throw();

  void close();
  void expand(off_t len);

  void write(const char *data, off_t len);
  void read(char *data, off_t len);
  void seek(off_t len);

  off_t size() const {
    return size_;
  }
  int fd() const {
    return fd_;
  }
  const char *filename() const {
    return fname_;
  }
};

#endif /* _FILE_H_ */
