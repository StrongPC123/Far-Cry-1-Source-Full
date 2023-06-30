// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <list>

#ifdef MAIN 
#define revbit2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int revbit2_test(int, char**)
{
  cout<<"Results of revbit2_test:"<<endl;
int array [] = { 1, 5, 2, 3 };

  list<int> v(array, array + 4);
  std::list<int>::reverse_iterator r;
  for(r = v.rbegin(); !(r == v.rend()); r++)
    cout << *r << endl;
  return 0;
}
