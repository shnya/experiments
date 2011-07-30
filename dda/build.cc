#include "double_array.hpp"
#include <fstream>
using namespace std;



int main(int argc, char *argv[])
{
  DoubleArray da;
  {
    string line;
    vector<string> lines;
    while(getline(cin,line)){
      lines.push_back(line);
    }
    stable_sort(lines.begin(),lines.end());

    for(size_t i = 0; i < lines.size(); i++){
      da.insert(lines[i].c_str());
    }
  }

  ofstream ofs_da("da.dat");
  ofstream ofs_dda("dda.dat");
  da.save(ofs_da);
  da.save_dag(ofs_dda);

  return 0;
}
