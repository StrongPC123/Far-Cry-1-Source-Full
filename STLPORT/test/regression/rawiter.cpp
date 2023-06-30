// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <iterator>
#include <memory>

#ifdef MAIN 
#define rawiter_test main
#endif

#include "rawiter.hpp"

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

int rawiter_test(int, char**)
{
  cout<<"Results of rawiter_test:"<<endl;

  allocator<X> a;
  typedef X* x_pointer;
  x_pointer save_p, p;
  p = a.allocate(5); 
  save_p=p;
  raw_storage_iterator<X*, X> r(p);
  int i;
  for(i = 0; i < 5; i++)
    *r++ = X(i);
  for(i = 0; i < 5; i++)
    cout << *p++ << endl;
# ifdef __STLPORT_VERSION
  a.deallocate(save_p,5);
# else
  a.deallocate(save_p);
# endif
  return 0;
}
