#ifndef _NNG_CONSTRUCTION_H_
#define _NNG_CONSTRUCTION_H_
#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>

#include "profile.hpp"
#include "sampling.hpp"
#include "string_piece.hpp"


struct NN {
  int id;
  float sim;
};

struct sim_comp {
  bool operator()(const NN &a, const NN &b){
    return a.sim > b.sim;
  }
};

struct value_comp {
  bool operator()(const std::pair<int,float> &a,
                  const std::pair<int,float> &b){
    return a.second > b.second;
  }
};

template <class DataSet, class SimFunc>
class NNGraph {

  std::vector<std::vector<NN> > graph_;
  std::vector<bool> newflgs_;
  const DataSet &data_;
  SimFunc sfunc_;
  int size_, k_;
  double sample_ratio_;
  double converge_ratio_;
  double t1,t2;

  bool _updateNN(int i,int j, double sim){
    std::vector<NN> &node = graph_[i];
    if(sim > node[0].sim){
      for(int l = 0; l < k_; l++)
        if(node[l].id == j) return false;
      pop_heap(node.begin(), node.end(), sim_comp());
      node[k_-1].id = j;
      node[k_-1].sim = sim;
      newflgs_[i*k_+k_-1] = true;
      push_heap(node.begin(), node.end(), sim_comp());
      return true;
    }else{
      return false;
    }
  }

  void _sampling(){
    //Profile<> p;
    std::vector<int> random_indices;
    random_indices.resize(size_);
    for(int i = 0; i < size_; i++){
      random_indices[i] = i;
    }
    for(int i = 0; i < size_; i++){
      std::vector<NN> &node = graph_[i];
      std::vector<int> out;
      GenSample::sampling_exclude_me(random_indices,
                                     k_, i,
                                     out);
      for(int j = 0; j < k_; j++){
        assert(out[j] != i);
        node[j].id = out[j];
        node[j].sim = -1.0E+09;
        newflgs_[i*k_+j] = true;
      }
      make_heap(node.begin(),node.end(),sim_comp());
    }
  }

  void _reverse(const std::vector<std::vector<int> > &g,
                std::vector<std::vector<int> > &rev){
    for(int i = 0; i < size_; i++){
      const std::vector<int> &nodes = g[i];
      int n = nodes.size();
      for(int j = 0; j < n; j++){
        rev[nodes[j]].push_back(i);
      }
    }
  }

  void _sample_union(std::vector<int> &nodes,
                     std::vector<int> &nodes_rev,
                     std::vector<int> &out){

    std::vector<int> nodes_rev_sample;
    GenSample::sampling(nodes_rev, k_ * sample_ratio_, nodes_rev_sample);
    std::sort(nodes_rev_sample.begin(),nodes_rev_sample.end());
    std::sort(nodes.begin(),nodes.end());
    std::set_union(nodes_rev_sample.begin(),nodes_rev_sample.end(),
                   nodes.begin(),nodes.end(),std::back_inserter(out));
    //cout << "nodes " << nodes << endl;
    //cout << "nodes_rev " << nodes_rev << endl;
    //cout << "out " << out << endl;
  }

  void _build_new_old_graph(std::vector<std::vector<int> > &new_graph,
                            std::vector<std::vector<int> > &old_graph){
    //Profile<> p;
    for(int i = 0; i < size_; i++){
      std::vector<NN> &nodes = graph_[i];
      std::vector<int> new_nodes;
      for(int j = 0; j < k_; j++){
        if(newflgs_[i*k_+j]){
          new_nodes.push_back(nodes[j].id);
        }else{
          old_graph[i].push_back(nodes[j].id);
        }
      }
      std::vector<int> new_res;
      GenSample::sampling(new_nodes, k_ * sample_ratio_, new_res);
      for(size_t l = 0; l < new_res.size(); l++){
        for(int j = 0; j < k_; j++){
          if(nodes[j].id == new_res[l]){
            newflgs_[i*k_+j] = false;
          }
        }
        new_graph[i].push_back(new_res[l]);
      }
    }
  }

  void _build(){
    _sampling();
    int gcount = 0;
    int rate_item = size_ / 20;

    while(gcount < 100000){
      //print_NN();
      gcount++;
      std::vector<std::vector<int> > new_graph(size_), old_graph(size_);
      _build_new_old_graph(new_graph,old_graph);

      std::vector<std::vector<int> > new_rev(size_), old_rev(size_);
      //Profile<> p;
      _reverse(new_graph,new_rev);
      _reverse(old_graph,old_rev);
      //p.end();
      int c = 0;
      //Profile<> pp;
      for(int i = 0; i < size_; i++){
        if(i % rate_item == 0) std::cerr << '.' << std::flush;
        std::vector<int> new_nodes, old_nodes;
        _sample_union(new_graph[i],new_rev[i],new_nodes);
        _sample_union(old_graph[i],old_rev[i],old_nodes);
        for(size_t l = 0; l < new_nodes.size(); l++){
          int u1 = new_nodes[l];
          for(size_t m = l+1; m < new_nodes.size(); m++){
            int u2 = new_nodes[m];
            double sim = sfunc_(data_[u1],data_[u2]);
            c += _updateNN(u1,u2,sim);
            c += _updateNN(u2,u1,sim);
          }
          for(size_t m = 0; m < old_nodes.size(); m++){
            int u2 = old_nodes[m];
            if(u1 == u2) continue;
            double sim = sfunc_(data_[u1],data_[u2]);
            c += _updateNN(u1,u2,sim);
            c += _updateNN(u2,u1,sim);
          }
        }
      }
      std::cerr << " iteration " << gcount << ": update " << c << std::endl;
      if(c <= converge_ratio_ * size_ * k_) break;
    }
  }

public:
  NNGraph(DataSet &d, int k = 10, double sample_ratio = 0.5,
          double converge_ratio = 0.0001,
          SimFunc sim = SimFunc())
    : data_(d), sfunc_(sim), size_(d.size()), k_(k),
      sample_ratio_(sample_ratio), converge_ratio_(converge_ratio) {
    graph_.resize(size_);
    newflgs_.resize(size_*k_);
    for(int i = 0; i < size_; i++){
      graph_[i].resize(k_);
    }
    GenSample::init();
    _build();
  }

  std::vector<std::pair<int,float> > get_nn(size_t i){
    std::vector<NN> &nodes = graph_[i];
    std::vector<std::pair<int,float> > res;
    for(int i = 0; i < k_; i++){
      res.push_back(std::make_pair(nodes[i].id,nodes[i].sim));
    }
    std::sort(res.begin(),res.end(),value_comp());
    return res;
  }

  bool save(std::ostream &os){
    int siz = size_, k = k_;
    os.write(reinterpret_cast<const char *>(&siz), sizeof(int));
    if(!os) return false;
    os.write(reinterpret_cast<const char *>(&k), sizeof(int));
    if(!os) return false;
    size_t i = 0;
    bool res = true;
    while(res && i < siz){
      res = os.write(reinterpret_cast<const char *>(&graph_[i][0]), sizeof(NN) * k_);
      i++;
    }
    return os;
  }

  void print_NN() const {
    for(int i = 0; i < size_; i++){
      std::cout << data_[i] << "\t";
      std::vector<NN> out(graph_[i]);
      sort(out.begin(),out.end(),sim_comp());
      for(int j = 0; j < k_; j++){
        std::cout << data_[out[j].id] << "," << out[j].sim << " ";
      }
      std::cout << std::endl;
    }
  }

};

#endif /* _NNG_CONSTRUCTION_H_ */
