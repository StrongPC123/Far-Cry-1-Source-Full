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
#define rotcopy1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int rotcopy1_test(int, char**)
{
  cout<<"Results of rotcopy1_test:"<<endl;
  vector <int> v1(10);
  iota(v1.begin(), v1.end(), 0);
  ostream_iterator <int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  vector <int> v2(v1.size());
  for(int i = 0; i < v1.size(); i++)
  {
    rotate_copy(v1.begin(),
                 v1.begin() + i,
                 v1.end(),
                 v2.begin());
    ostream_iterator <int> iter(cout, " ");
    copy(v2.begin(), v2.end(), iter);
    cout << endl;
  }
  cout << endl;
  return 0;
}
