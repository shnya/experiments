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
  NNGraph<vector<string>, LevenshteinSimilarity> g(lines);
  for(size_t i = 0; i < lines.size(); i++){
    std::vector<std::pair<int,float> > nn = g.get_nn(i);
    cout << lines[i] << "\t";
    for(size_t j = 0; j < nn.size(); j++){
      cout << lines[nn[j].first] << "," << nn[j].second << " ";
    }
    cout << endl;
  }


  return 0;
}
