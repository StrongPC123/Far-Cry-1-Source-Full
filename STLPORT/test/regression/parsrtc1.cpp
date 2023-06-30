// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>

#ifdef MAIN 
#define parsrtc1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int parsrtc1_test(int, char**)
{
  cout<<"Results of parsrtc1_test:"<<endl;
  vector <int> v1(10);
  for(int i = 0; i < v1.size(); i++)
    v1[i] = rand() % 10;
  vector <int> result(5);
  ostream_iterator<int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  partial_sort_copy(v1.begin(),
                     v1.end(),
                     result.begin(),
                     result.end());
  copy(result.begin(), result.end(), iter);
  cout << endl;
  return 0;
}
