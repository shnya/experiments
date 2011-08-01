#include <map>
#include <numeric>
#include <algorithm>
#include <cmath>
#include "double_array.hpp"
#include "dictionary.hpp"
#include "utf8.hpp"

using namespace std;

class Convert {

  struct node {
    int to;
    int id;
    double score_min;
    int from_pos;
    int from_nth;
    node(int t,int i)
      : to(t), id(i), score_min(1E+10), from_pos(-1), from_nth(-1){}
  };

  double get_score(int id_from, int id_to){
    int n = bigram[make_pair(id_from,id_to)];
    return n != 0 ? exp(n) : 0.5;
  }

public:
  string convert(const char *orig){
    vector<const char *> letters = utf8index(orig);
    size_t n = letters.size();

    //build node
    vector<vector<node> > nodes(n+2);
    nodes[0].push_back(node(1,-1)); //BOS
    nodes[0][0].score_min = 0;
    nodes[n+1].push_back(node(-1,-2)); //EOS
    for(size_t i = 0; i < n; i++){
      vector<int> v_len,v_id;
      da.commonPrefixSearch(letters[i],v_len,v_id);
      for(size_t j = 0; j < v_len.size(); j++){
        int k = utf8len(letters[i],v_len[j]);
        if(k == -1 || i+1+k > n+2) continue;
        int l = i + k + 1;
        vector<int> &v = kanakan_map[v_id[j]];
        for(size_t m = 0; m < v.size(); m++){
          nodes[i+1].push_back(node(l,v[m]));
        }
      }
    }

    // viterbi
    for(size_t i = 0; i < nodes.size()-1; i++){
      for(size_t j = 0; j < nodes[i].size(); j++){
        node &from = nodes[i][j];
        for(size_t k = 0; k < nodes[from.to].size(); k++){
          node &to = nodes[from.to][k];
          double score = get_score(from.id,to.id);
          if(to.score_min > from.score_min + score){
            to.score_min = from.score_min + score;
            to.from_pos = i;
            to.from_nth = j;
          }
        }
      }
    }

    vector<int> path_pos,path_nth;
    int path_last_pos = nodes[n+1][0].from_pos;
    int path_last_nth = nodes[n+1][0].from_nth;
    vector<int> ids;
    while(path_last_pos != 0){
      if(path_last_pos < static_cast<int>(nodes.size())
         && path_last_pos >= 0
         && path_last_nth < static_cast<int>(nodes[path_last_pos].size())
         && path_last_nth >= 0){
        node &from = nodes[path_last_pos][path_last_nth];
        ids.push_back(from.id);
        path_last_pos = from.from_pos;
        path_last_nth = from.from_nth;
      }else{
        cout << "invalid input" << endl;
        break;
      }
    }
    string result;
    for(int i = ids.size()-1; i >= 0; i--){
      result += kanji_map[ids[i]];
    }
    return result;
  }

  Convert() {
    bool res = katakana_read(da);
    if(!res) throw logic_error("Can't read katakana dict");
    res = kanakanmap_read(kanakan_map);
    if(!res) throw logic_error("Can't read kanakan dict");
    res = kanjimap_read(kanji_map);
    if(!res) throw logic_error("Can't read kanji dict");
    res = bigram_read(bigram);
    if(!res) throw logic_error("Can't read bigram dict");
  }
private:
  vector<vector<int> > kanakan_map;
  vector<string> kanji_map;
  map<pair<int,int>,int> bigram;
  DoubleArray da;
};


int main(int argc, char *argv[])
{
  Convert conv;
  if(argc < 2){
    cout << conv.convert("わたしのなまえはなかのです") << endl;
    return 0;
  }
  string line;
  while(getline(cin,line)){
    cout << conv.convert(line.c_str()) << endl;
  }
  return 0;
}
