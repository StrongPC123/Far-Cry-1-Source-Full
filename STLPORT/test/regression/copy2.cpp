// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>
#include <iterator>

#ifdef MAIN 
#define copy2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int copy2_test(int, char**)
{
  cout<<"Results of copy2_test:"<<endl;
  vector <int> v(10);
  for(int i = 0; i < v.size(); i++)
    v[i] = i;
  ostream_iterator<int> iter(cout, " ");
  copy(v.begin(), v.end(), iter);
  cout << endl;
  return 0;
}
