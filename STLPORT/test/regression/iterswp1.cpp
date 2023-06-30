// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iterator>

#ifdef MAIN 
#define iterswp1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int iterswp1_test(int, char**)
{
  cout<<"Results of iterswp1_test:"<<endl;
  vector <int> v1(6);
  iota(v1.begin(), v1.end(), 0);
  iter_swap(v1.begin(), v1.begin() + 3);
  ostream_iterator <int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  return 0;
}
