#include <iostream>
using namespace std;


class A {


};

class B : public A {
public:
  void foo() const{
    std::cout << "foo" << std::endl;
  };
};

class C : public A {
public:
  void hoge();
};


void sim(const A &a){
  static_cast<const B&>(a).foo();
}

template <class T>
struct D{};

template <>
struct D<int>{
  int a;

  int foo(){
    cout << "ccc" << endl;
    return 2;
  }
};

template <>
struct D<unsigned>{
  int b;
  unsigned c;
  int foo(){
    cout << "ccc" << endl;
    return 3;
  }
};

template<class T>
class E{
  T a;
public:
  E(T _a) : a(_a) {}
};


int main(int argc, char *argv[])
{
  E<int> e(3);
  
  return 0;
}

