#include "profile.hpp"

int main(int argc, char *argv[])
{
  Profile<> p;
  CulmativeProfile<> cul;
  for(int i = 0; i < 1000000; i++){
    Profile<> pp;
    int k = 3; k++;
    cul.add(pp);
  }
  p.end();
  for(int i = 0; i < 1000000; i++){
    int k = 3; k++;
  }

  return 0;
}

