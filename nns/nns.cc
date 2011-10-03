#include <map>
#include "mmap_vector.hpp"
#include "nng_builder.hpp"
#include "sampling.hpp"
#include "similarity.hpp"
#include "line_reader.hpp"


using namespace std;


int main(int argc, char *argv[])
{
  if(argc < 3){
    cerr << "Usage: ./nns [filename] [query]" << endl;
    return -1;
  }
  int R = 50;
  int T = 10;
  int O = 10;


  try{
    MMapMatrix<pair<int,float> > mat((string(argv[1]) + ".mat").c_str());
    LineReader reader(argv[1], (string(argv[1]) + ".idx").c_str());
    const char *str = argv[2];

    int k = mat.col();
    int n = mat.row();
    vector<int> cur;
    GenSample::init();
    GenSample::sampling_overwrapped(n,R,cur);
    map<int,float> out;
    LevenshteinSimilarity simfunc;
    for(size_t i = 0; i < cur.size(); i++){
      double minn = -1.0E+09;
      int mini = cur[i];
      for(int j = 0; j < T; j++){
        int id = mini;
        for(int l = 0; l < k; l++){
          const pair<int,float> &near_k = mat.at(id,l);
          //cout << near_k.first << endl;
          float sim = simfunc(str,reader[near_k.first]);
          if(sim > minn + 0.000001){
            mini = near_k.first;
            minn = sim;
          }
        }
        out.insert(make_pair(mini,minn));
      }
    }

    vector<pair<float,int> > out2;
    for(map<int,float>::const_iterator itr = out.begin();
        itr != out.end(); ++itr){
      out2.push_back(make_pair(itr->second,itr->first));
      //cout << itr->first << "," << itr->second << " ";
    }
    cout << endl;
    sort(out2.begin(),out2.end(),greater<pair<float,int> >());
    for(int i = 0; i < O; i++){
      cout << reader[out2[i].second] << "," << out2[i].first << " ";
    }
    cout << endl;
  }catch(MMapException &e){
    cerr << e.what() << endl;
    return -1;
  }

  return 0;
}
