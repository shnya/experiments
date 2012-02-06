#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <string>

#include <tr1/functional_hash.h>

class HashTable {
  static uint32_t hash_func(const std::string &key, uint32_t mask)
  {
    /*
    // rotate hash From http://burtleburtle.net/bob/hash/doobs.html
    const uint8_t *key = static_cast<const uint8_t*>(keyp);
    uint32_t hash, i;
    for(hash = len, i = 0; i < len; ++i)
    hash = (hash << 4)^(hash >> 28)^key[i];
    return (hash ^ (hash >> 10) ^ (hash >> 20)) & mask;
    */

    return hash_fun(key) & mask;
  }

  struct block {
    char *key;
    char *value;
    uint32_t hop_info;
  };

  void resize(){
    uint32_t new_mask = ((mask + 1) << 1) - 1;
    block *new_blocks = new block[new_mask + 1];
    memset(new_blocks, 0, sizeof(block) * (new_mask + 1));
    //std::cout << new_mask + 1 << std::endl;
    for(uint32_t i = 0; i < mask + 1; i++){
      if(blocks[i].key != NULL){
        _insert(blocks[i].key, blocks[i].value,
                new_blocks, new_mask);
        free(blocks[i].key);
        free(blocks[i].value);
      }
    }
    delete[] blocks;
    blocks = new_blocks;
    mask = new_mask;
  }

  static int32_t move_free(int32_t free_pos, block* _blocks, uint32_t _mask){
    for(int i = HOP_SIZE - 1; i >= 0; i--){
      int32_t start = free_pos - i;
      if(start < 0) continue;

      int cand = -1;
      uint32_t &info = _blocks[start].hop_info;
      for(int j = 0; j < i; j++){
        if(info & (1<<j)){
          cand = start + j;
          break;
        }
      }
      if(cand == -1) continue;

      block &target = _blocks[free_pos];
      block &from = _blocks[cand];

      target.key = from.key;
      target.value = from.value;
      from.key = NULL;
      from.value = NULL;
      info |= (1 << i);
      info &= ~(1 << (cand - start));
      return cand;
    }
    return -1;
  }

  static bool _insert(const std::string &key, const std::string &value,
                      block* _blocks, uint32_t _mask){
    int32_t hash = hash_func(key, _mask);
    int32_t free_pos = -1;
    for(uint32_t i = hash ; i < (_mask + 1); i++){
      if(_blocks[i].key == NULL){
        free_pos = i;
        break;
      }
    }

    while(free_pos != -1){
      if(free_pos - hash < HOP_SIZE){
        block& blk = _blocks[free_pos];
        blk.key = strdup(key.c_str());
        blk.value = strdup(value.c_str());
        if(blk.key == NULL || blk.value == NULL){
          if(blk.key != NULL) free(blk.key);
          blk.key = NULL;
          if(blk.value != NULL) free(blk.value);
          blk.value = NULL;
          continue;
        }
        _blocks[hash].hop_info |= 1 << (free_pos - hash);
        return true;
      }
      free_pos = move_free(free_pos, _blocks, _mask);
    }
    return false;
  }

  int32_t _find(const std::string &key){
    uint32_t hash = hash_func(key, mask);
    uint32_t info = blocks[hash].hop_info;
    const char *keyp = key.c_str();
    for(int i = 0; i < HOP_SIZE; i++){
      if(info & (1 << i)){
        if(strcmp(blocks[hash+i].key, keyp) == 0){
          return hash + i;
        }
      }
    }
    return -1;
  }


public:
  HashTable(uint32_t _len = 1024){
    uint32_t len = 1;
    while(len >= _len)
      len <<= 1;
    blocks = new block[len];
    mask = len - 1;
    memset(blocks, 0, sizeof(block) * (mask + 1));
  }

  ~HashTable(){
    for(int32_t i = 0; i < (mask + 1); i++){
      if(blocks[i].key != NULL){
        free(blocks[i].key);
        free(blocks[i].value);
      }
    }
    delete[] blocks;
  }

  bool insert(const std::string &key, const std::string &value){
    if(find(key) != NULL)
      return false;
    bool result =false;
    do {
      result = _insert(key, value, blocks, mask);
      if(!result) resize();
    }while(!result);
    return true;
  }

  const char *find(const std::string &key){
    int32_t idx = _find(key);
    if(idx == -1)
      return NULL;

    return blocks[idx].value;
  }

  size_t size(){
    return mask + 1;
  }

  static std::tr1::hash<std::string> hash_fun;

private:
  static const int HOP_SIZE = 32;
  block* blocks;
  uint32_t mask;
};

std::tr1::hash<std::string> HashTable::hash_fun = std::tr1::hash<std::string>();


#include <iostream>
#include <sstream>
#include <vector>

#include <tr1/unordered_map>
#include "profile.hpp"


using namespace std;
using namespace std::tr1;


int main(int argc, char *argv[])
{
  vector<string> v;
  /*
  for(int i = 0; i < 1000000; i++){
    ostringstream oss;
    oss << "abc" << i;
    v.push_back(oss.str());
  }
  */
  while(cin){
    string line;
    std::getline(cin,line);
    v.push_back(line);
  }
  cout << "Ver. Hash Table" << endl;
  HashTable htable;
  {
    Profile<> p;
    for(size_t i = 0; i < v.size(); i++){
      htable.insert(v[i],v[i]);
    }
  }
  {
    Profile<> p;
    for(size_t i = 0; i < v.size(); i++){
      if(strcmp(v[i].c_str(), htable.find(v[i])) != 0)
        cout << v[i] << "\t" << htable.find(v[i]) << endl;
    }
  }
  cout << "Ver. Unordered Map" << endl;
  unordered_map<string,string> htable2;
  {
    Profile<> p;
    for(size_t i = 0; i < v.size(); i++){
      htable2.insert(make_pair(v[i],v[i]));
    }
  }
  {
    Profile<> p;
    for(size_t i = 0; i < v.size(); i++){
      if(v[i] != htable2[v[i]]){
        cout << v[i] << " " << htable2[v[i]] << endl;
      }
    }
  }
  return 0;
}
