#ifndef _STRING_PIECE_H_
#define _STRING_PIECE_H_
#include <string>
#include <cstring>
#include <iostream>


class StringPiece {
  const char *ptr_;
  size_t len_;

public:
  StringPiece(const char *str)
    : ptr_(str), len_(strlen(str)) {}

  StringPiece(const char *str, size_t len)
    : ptr_(str), len_(len) {}

  StringPiece(const std::string &str)
    : ptr_(str.c_str()), len_(str.size()){}

  size_t size() const {
    return len_;
  }

  char operator[](size_t i) const {
    return ptr_[i];
  }

  const char *data() const {
    return ptr_;
  }
};

std::ostream&
operator<<(std::ostream &os, const StringPiece &p){
  for(size_t i = 0; i < p.size(); i++){
    os << p[i];
  }
  return os;
}

#endif /* _STRING_PIECE_H_ */
