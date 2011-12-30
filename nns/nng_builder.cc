#include <iostream>

#include "nng_builder.hpp"
#include "similarity.hpp"
#include "line_reader.hpp"

using namespace std;


int main(int argc, char *argv[])
{
  if(argc < 2){
    cerr << "Usage: ./nng_builder [filename]" << endl;
    return -1;
  }
  //cout << "begin index" << endl;
  bool res = LineReader::make_index(argv[1], 
                                    (string(argv[1]) + ".idx").c_str());
  if(!res){
    cerr << "make index failed" << endl;
    return -1;
  }
  //LineReader reader(argv[1], (string(argv[1]) + ".idx").c_str());
  ifstream ifs(argv[1]);
  vector<string> lines;
  string line;
  while(getline(ifs,line)){
    lines.push_back(line);
  }
  if(!ifs.eof() && !ifs){
    ifs.close();
    cerr << "file read failed" << endl;
    return -1;
  }
  //cout << "made index" << endl;
  NNGraph<vector<string>, LevenshteinSimilarity> g(lines,30);
  ofstream ofs((string(argv[1]) + ".mat").c_str());
  if(!ofs || !g.save(ofs)){
    ofs.close();
    cerr << "file write failed" << endl;
    return -1;
  }
  ofs.close();
  if(ofs.fail()){
    cerr << "file close failed" << endl;
    return -1;
  }
  g.print_NN();


  return 0;
}
