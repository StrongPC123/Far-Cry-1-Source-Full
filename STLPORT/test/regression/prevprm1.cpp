// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <iterator>

#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
// #include <functional>
#include <numeric>

#ifdef MAIN 
#define prevprm1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int prevprm1_test(int, char**)
{
  cout<<"Results of prevprm1_test:"<<endl;
  vector <int> v1(3);
  iota(v1.begin(), v1.end(), 0);
  ostream_iterator<int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  for(int i = 0; i < 9; i++)
  {
    prev_permutation(v1.begin(), v1.end());
    copy(v1.begin(), v1.end(), iter);
    cout << endl;
  }
  return 0;
}
