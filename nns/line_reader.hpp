#ifndef _LINE_READER_H_
#define _LINE_READER_H_
#include <fstream>

#include "mmap_vc.hpp"
#include "mmap_vector.hpp"
#include "string_piece.hpp"


class LineReader {
  MMapVector<char> mv;
  MMapVC mvc;

  static bool isLineSeparator(char c){
    if(c == '\r' || c == '\n') return true;
    return false;
  }

public:
  static bool make_index(const char *infile, const char *outfile){
    MMapVector<char> mv(infile);
    size_t siz = mv.size();
    std::vector<int> offset;
    bool flg = false;
    for(size_t i = 0; i < siz; i++){
      if(isLineSeparator(mv[i])){
        flg = true;
      }else if(flg){
        offset.push_back(i-1);
        flg =false;
      }
    }
    offset.push_back(siz-1);
    mv.release();
    VerticalCode vc(offset);
    std::ofstream ofs(outfile);
    if(!ofs) return false;
    bool res = vc.save(ofs);
    ofs.close();
    return res;
  }

  LineReader(const char *infile, const char *idxfile)
    : mv(infile), mvc(idxfile) {}

  StringPiece getline(size_t i) const {
    return (*this)[i];
  }

  StringPiece operator[](size_t i) const {
    int mini = mvc.psum(i);
    int maxi = mvc.psum(i+1);
    int len = 0;
    if(mini == -1 || maxi == -1) return std::string();

    for(; maxi >= mini; maxi--){
      if(!isLineSeparator(mv[maxi]))
        break;
    }
    len = std::max(0,maxi-mini-1);
    return StringPiece(mv.at_ptr(mini),len);
  }

  size_t size() const {
    return mvc.size();
  }

  void release() throw(MMapException) {
    mv.release();
    mvc.release();
  }
};

#endif /* _LINE_READER_H_ */


/*
using namespace std;


int main(int argc, char *argv[])
{
  try{
    LineReader::make_index("linux.words","linux.idx");
    LineReader reader("linux.words","linux.idx");

    for(size_t i = 0; i < reader.size(); i++){
      cout << reader.getline(i) << endl;
    }
  }catch(MMapException &e){
    cout << e.what() << endl;
  }

  return 0;
}

*/

