// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <iterator>

#include <vector>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <iterator>

#ifdef MAIN 
#define partsum1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int partsum1_test(int, char**)
{
  cout<<"Results of partsum1_test:"<<endl;
  vector <int> v1(10);
  iota(v1.begin(), v1.end(), 0);
  vector <int> v2(v1.size());
  partial_sum(v1.begin(), v1.end(), v2.begin());
  ostream_iterator <int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  copy(v2.begin(), v2.end(), iter);
  cout << endl;
  return 0;
}
