#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

#include <string>
#include <exception>
#include <cstring>
#include <errno.h>

class PlusDBException : public std::exception {
protected:
  std::string message_;
  PlusDBException(){};
public:
  PlusDBException(std::string err)
  { message_ = err; }
  virtual ~PlusDBException() throw() {};
  virtual const char *what() throw()
  { return message_.c_str(); }
};


class FileException : public PlusDBException {
public:
  FileException(std::string err, std::string filename = "")
  { message_ = err + " : filename = " + filename + " " + strerror(errno); }
};

class MMapException : public PlusDBException {
public:
  MMapException(std::string err, std::string filename = "")
  { message_ = err + " : filename = " + filename + " " + strerror(errno); }
};


#endif /* _EXCEPTION_H_ */
