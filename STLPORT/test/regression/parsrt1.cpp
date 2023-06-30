// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>

#ifdef MAIN 
#define parsrt1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int parsrt1_test(int, char**)
{
  cout<<"Results of parsrt1_test:"<<endl;
  vector <int> v1(10);
  for(int i = 0; i < v1.size(); i++)
    v1[i] = rand() % 10;
  ostream_iterator<int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  partial_sort(v1.begin(),
                v1.begin() + v1.size() / 2,
                v1.end());
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  return 0;
}
