#ifndef _DATASET_H_
#define _DATASET_H_

#include <vector>

struct null_value;

template<class KeyT>
class DataSetBase {
protected:
  std::vector<KeyT> keys;
  DataSetBase(){} //prohibit base class construction.

public:
  size_t size() const {
    return keys.size();
  }

  const KeyT& key_at(size_t i) const {
    return keys[i];
  }

  KeyT& key_at(size_t i){
    return keys[i];
  }

  const KeyT operator[](size_t i) const {
    return keys[i];
  }
};


template<class KeyT, class ValueT = null_value>
class DataSet : public DataSetBase<KeyT> {
  std::vector<ValueT> values;
public:
  const ValueT& value_at(size_t i) const {
    return values[i];
  }

  ValueT& value_at(size_t i){
    return values[i];
  }

  void push_back(const KeyT &key, const ValueT &value){
    DataSetBase<KeyT>::keys.push_back(key);
    values.push_back(key);
  }
};

template<class KeyT>
class DataSet<KeyT,null_value> : public DataSetBase<KeyT> {
public:
  void push_back(const KeyT &key){
    DataSetBase<KeyT>::keys.push_back(key);
  }
};

#endif /* _DATASET_H_ */
