// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <iterator>

#include <algorithm>
#include <vector>
#include <iterator>
// #include <functional>
#include <numeric>
#include <iostream>

#ifdef MAIN 
#define prevprm2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int prevprm2_test(int, char**)
{
  cout<<"Results of prevprm2_test:"<<endl;
  vector <int> v1(3);
  iota(v1.begin(), v1.end(), 0);
  ostream_iterator<int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  for(int i = 0; i < 9; i++)
  {
    prev_permutation(v1.begin(), v1.end(), greater<int>());
    copy(v1.begin(), v1.end(), iter);
    cout << endl;
  }
  return 0;
}
