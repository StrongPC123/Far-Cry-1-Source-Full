// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <utility>
#include <iostream>

#ifdef MAIN 
#define pair1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int pair1_test(int, char**)
{
  cout<<"Results of pair1_test:"<<endl;
  pair<int, int> p = make_pair(1, 10);
  cout << "p.first = " << p.first << ", p.second = " << p.second << endl;
  return 0;
}
