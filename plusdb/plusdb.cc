#include "page.h"



int main(int argc, char *argv[])
{
  File pf("a.txt");
  Page m(pf, 4096);
  char *a = m.data<char>();
  for(off_t i = 0; i < m.size(); i++){
    *a++ = 'b';
  }
  m.msync();
  m.munmap();

  return 0;
}

