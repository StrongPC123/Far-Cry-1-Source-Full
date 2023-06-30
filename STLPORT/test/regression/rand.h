#ifndef _rand_h
#define _rand_h
#include <cstdlib>

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

class MyRandomGenerator
{
  public:
    unsigned long operator()(unsigned long n_)
      {
      return rand() % n_;
      }
};

#endif // _rand_h
