#include "mmap_vector.hpp"
#include "nng_builder.hpp"
#include "sampling.hpp"
#include "similarity.hpp"
#include "line_reader.hpp"


using namespace std;


int main(int argc, char *argv[])
{
  if(argc < 3) return 0;
  MMapMatrix<pair<int,float> > mat(argv[1]);
  LineReader reader(argv[2], (string(argv[2]) + ".idx").c_str());
  const char *str = argv[3];

  int k = mat.col();
  int n = mat.row();
  GenSample::init();
  vector<int> cur;
  int R = 5;
  GenSample::sampling_overwrapped(n,R,cur);
  vector<pair<int,float> > out;
  LevenshteinSimilarity simfunc;
  double minn = -1.0E+09;
  int T = 10;
  for(size_t i = 0; i < cur.size(); i++){
    for(int j = 0; j < T; j++){
      int mini = -1;
      for(int i = 0; i < k; i++){
        int id = cur[i];
        float sim = simfunc(str,reader[cur[i]]);
        out.push_back(make_pair(id,sim));
        if(sim > minn + 0.000001){
          mini = id;
          minn = sim;
        }
      }
      if(mini == -1) break;
      vector<int> vec(k);
      const pair<int,float> *k_bests = mat.row(mini);
      for(int i = 0; i < k; i++){
        vec.push_back(k_bests[i].first);
      }
    }
  }

  sort(out.begin(),out.end(),value_comp());
  for(size_t i = 0; i < out.size(); i++){
    cout << reader[out[i].first] << "," << out[i].second << " ";
  }
  cout << endl;

  return 0;
}

