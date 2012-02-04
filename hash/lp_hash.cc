#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <iostream>


class HashTable {
  // rotate hash From http://burtleburtle.net/bob/hash/doobs.html
  static uint32_t hash_func(const void *keyp, uint32_t len, uint32_t mask)
  {
    const uint8_t *key = static_cast<const uint8_t*>(keyp);
    uint32_t hash, i;
    for(hash = len, i = 0; i < len; ++i)
      hash = (hash << 4)^(hash >> 28)^key[i];
    return (hash ^ (hash >> 10) ^ (hash >> 20)) & mask;
  }

  struct block {
    char *key;
    char *value;
  };

  void resize(){
    uint32_t new_mask = ((mask + 1) << 1) - 1;
    block *new_blocks = new block[new_mask + 1];
    //std::cout << new_mask + 1 << std::endl;
    memset(new_blocks, 0, sizeof(block) * (new_mask + 1));
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

  static bool _insert(const char *key, const char *value,
                      block* _blocks, uint32_t _mask){
    uint32_t hash = hash_func(key, strlen(key), _mask);
    for(uint32_t i = 0; i < (_mask + 1); i++){
      if(_blocks[(i + hash) & _mask].key == NULL){
        uint32_t block_idx = (i + hash) & _mask;
        block &blk = _blocks[block_idx];
        blk.key = strdup(key);
        blk.value = strdup(value);
        if(blk.key == NULL || blk.value == NULL){
          if(blk.key != NULL) free(blk.key);
          blk.key = NULL;
          if(blk.value != NULL) free(blk.value);
          blk.value = NULL;
          return false;
        }
        return true;
      }
    }
    return false;
  }

  static bool _update(const char *key, const char *value,
                      block* _blocks, uint32_t _mask){
    uint32_t hash = hash_func(key, strlen(key), _mask);
    for(uint32_t i = 0; i < (_mask + 1); i++){
      uint32_t block_idx = (i + hash) & _mask;
      if(strcmp(_blocks[block_idx].key, key) == 0){
        uint32_t block_idx = (i + hash) & _mask;
        block &blk = _blocks[block_idx];
        blk.value = strdup(value);
        if(blk.value == NULL){
          if(blk.value != NULL) free(blk.value);
          blk.value = NULL;
          return false;
        }
        return true;
      }
    }
    return false;
  }

  int32_t _find(const char *key){
    uint32_t hash = hash_func(key, strlen(key), mask);
    for(uint32_t i = 0; i < (mask + 1); i++){
      uint32_t block_idx = (i + hash) & mask;
      if(blocks[block_idx].key == NULL) return -1;
      if(strcmp(blocks[block_idx].key, key) == 0){
        return block_idx;
      }
    }
    return -1;
  }


public:
  HashTable(uint32_t len = 1024){
    blocks = new block[len];
    mask = len - 1;
    cnt = 0;
    memset(blocks, 0, sizeof(block) * (mask + 1));
  }

  ~HashTable(){
    for(uint32_t i = 0; i < (mask + 1); i++){
      if(blocks[i].key != NULL){
        free(blocks[i].key);
        free(blocks[i].value);
      }
    }

    delete[] blocks;
  }

  bool insert(const char *key, const char *value){
    if(find(key) != NULL)
      return false;
    bool result =false;
    if((double)cnt / (mask+1) > 0.5) resize();
    do {
      result = _insert(key, value, blocks, mask);
      if(!result) resize();
    }while(!result);
    cnt++;
    return true;
  }

  bool update(const char *key, const char *value){
    bool result = false;
    do {
      if(find(key) == NULL){
        result = _insert(key, value, blocks, mask);
      }else{
        result = _update(key, value, blocks, mask);
      }
      if(!result) resize();
    }while(!result);
    return true;
  }

  const char *find(const char *key){
    uint32_t hash = hash_func(key, strlen(key), mask);
    for(uint32_t i = 0; i < (mask + 1); i++){
      uint32_t block_idx = (i + hash) & mask;
      if(blocks[block_idx].key == NULL) return NULL;
      if(strcmp(blocks[block_idx].key, key) == 0){
        return blocks[block_idx].value;
      }
    }
    return NULL;
  }

private:
  block* blocks;
  uint32_t mask;
  uint32_t cnt;
};

#include <iostream>
#include <vector>
#include <string>
#include "profile.hpp"


using namespace std;

int main(int argc, char *argv[])
{
  HashTable htable(16777216<<1);
  vector<string> v;
  while(cin){
    string line;
    std::getline(cin,line);
    v.push_back(line);
  }
  {
    Profile<> p;
    for(size_t i = 0; i < v.size(); i++){
      htable.insert(v[i].c_str(),v[i].c_str());
    }
  }
  {
    Profile<> p;
    for(size_t i = 0; i < v.size(); i++){
      if(strcmp(v[i].c_str(),htable.find(v[i].c_str())) != 0){
        cout << v[i] << endl;
      }
    }
  }

  return 0;
}
