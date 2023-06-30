// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <functional>

#ifdef MAIN 
#define stblptn1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int stblptn1_test(int, char**)
{
  cout<<"Results of stblptn1_test:"<<endl;
  vector <int> v1(10);
  for(int i = 0; i < v1.size(); i++)
    v1[i] = rand() % 20;
  ostream_iterator <int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  stable_partition(v1.begin(), v1.end(), bind2nd(less<int>(), 11));
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  return 0;
}
