// Copyright (C) 2010 Masahiko Higashiyama
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef UTF8_UTILITY_HPP
#define UTF8_UTILITY_HPP

#include <cstring>
#include <exception>
#include <vector>

class UTF8Exception : public std::exception {
public:
  UTF8Exception() {}
  virtual ~UTF8Exception() throw() {}
};

int utf8charlen(const unsigned char c)
{
  if(c == 0x00) return 0;
  if(c < 0x80) return 1;
  if(c < 0xC2) throw UTF8Exception();
  if(c < 0xE0) return 2;
  if(c < 0xF0) return 3;
  if(c < 0xF8) return 4;
  if(c < 0xFC) return 5;
  if(c < 0xFE) return 6;
  return 1;
}

char *utf8substr(const char *s, int len){
  int n = 0, size = 0;
  const char *p = s;
  int l;
  while((l = utf8charlen(*p)) && n != len){
    p += l;
    size += l;
    n++;
  }
  if(l == 0) size++;
  char *str = new char[size + 1];
  strncpy(str, s, size);
  str[size] = '\0';

  return str;
}

int utf8len(const char *s, int len){
  const char *p = s;
  int n = 0, nc = 0;
  while(*p != '\0' && n < len){
    int k = utf8charlen(*p);
    n += k;
    nc++;
    p += k;
  }
  if(n > len) return -1;
  return nc;
}

const char *utf8nextchar(const char *s){
  return s + utf8charlen(*s);
}

std::vector<const char *> utf8index(const char *s){
  std::vector<const char *> v;
  const char *p = s;
  while(*p != '\0'){
    v.push_back(p);
    p = utf8nextchar(p);
  }
  return v;
}

const char *utf8advance(const char *s, unsigned int len){
  size_t l = 0;
  const char *p = s;
  while(*p != '\0' && l < len ){
    p = utf8nextchar(p);
    l++;
  }
  return p;
}

// // bi-gram extraction example
// #include <iostream>
// using namespace std;
//
// int main(int argc, char *argv[])
// {
//   const char *p = "大きなノッポの古時計";
//   while(*p != '\0'){
//     char *sub = utf8substr(p, 2);
//     cout << sub << " ";
//     delete[] sub;
//     p = utf8nextchar(p);
//   }
//   cout << endl;
//   return 0;
// }
#endif /* UTF8_UTILITY_HPP */
