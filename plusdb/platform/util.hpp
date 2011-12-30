#include <unistd.h>

class Env {
public:
  static int GetPageSize()
  {
#ifdef BSD
    return getpagesize();
#else
    return sysconf(_SC_PAGE_SIZE);
#endif
  }

};
