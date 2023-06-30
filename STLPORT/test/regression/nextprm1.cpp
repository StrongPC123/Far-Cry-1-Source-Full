// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <iostream>

#ifdef MAIN 
#define nextprm1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int nextprm1_test(int, char**)
{
  cout<<"Results of nextprm1_test:"<<endl;
  vector <int> v1(3);
  iota(v1.begin(), v1.end(), 0);
  ostream_iterator<int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  for(int i = 0; i < 9; i++)
  {
    next_permutation(v1.begin(), v1.end());
    copy(v1.begin(), v1.end(), iter);
    cout << endl;
  }
  return 0;
}
