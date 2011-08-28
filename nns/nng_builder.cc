#include <iostream>

#include "nng_builder.hpp"
#include "similarity.hpp"
#include "line_reader.hpp"

using namespace std;


int main(int argc, char *argv[])
{
  if(argc < 2) return -1;
  //cout << "begin index" << endl;
  LineReader::make_index(argv[1], (string(argv[1]) + ".idx").c_str());
  //LineReader reader(argv[1], (string(argv[1]) + ".idx").c_str());
  std::ifstream ifs(argv[1]);
  vector<string> lines;
  string line;
  while(getline(ifs,line)){
    lines.push_back(line);
  }
  //cout << "made index" << endl;
  NNGraph<vector<string>, LevenshteinSimilarity> g(lines,25);
  g.print_NN();


  return 0;
}
