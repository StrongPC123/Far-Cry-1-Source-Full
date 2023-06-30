// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <functional>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>

#ifdef MAIN 
#define nthelem2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int nthelem2_test(int, char**)
{
  cout<<"Results of nthelem2_test:"<<endl;
  vector <int> v1(10);
  for(int i = 0; i < v1.size(); i++)
    v1[i] = rand() % 10;
  ostream_iterator<int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  nth_element(v1.begin(),
               v1.begin() + v1.size() / 2,
               v1.end(),
               greater<int>());
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  return 0;
}
