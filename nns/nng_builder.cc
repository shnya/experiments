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
  LineReader reader(argv[1], (string(argv[1]) + ".idx").c_str());
  //cout << "made index" << endl;
  NNGraph<LineReader, LevenshteinSimilarity> g(reader,15);
  for(size_t i = 0; i < reader.size(); i++){
    std::vector<std::pair<int,float> > nn = g.get_nn(i);
    cout << reader[i] << "\t";
    for(size_t j = 0; j < nn.size(); j++){
      cout << reader[nn[j].first] << "," << nn[j].second << " ";
    }
    cout << endl;
  }


  return 0;
}
