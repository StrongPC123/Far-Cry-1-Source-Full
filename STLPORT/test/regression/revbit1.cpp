// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <list>

#ifdef MAIN 
#define revbit1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int revbit1_test(int, char**)
{
  cout<<"Results of revbit1_test:"<<endl;
int array [] = { 1, 5, 2, 3 };

  list<int> v(array, array + 4);
  std::list<int>::reverse_iterator r(v.rbegin());
  while(!(r == v.rend()))
    cout << *r++ << endl;
  return 0;
}
