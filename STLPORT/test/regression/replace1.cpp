// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include <functional>
#include <iterator>

#ifdef MAIN 
#define replace1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int replace1_test(int, char**)
{
  cout<<"Results of replace1_test:"<<endl;
  vector <int> v1(10);
  for(int i = 0; i < v1.size(); i++)
    v1[i] = i % 5;
  ostream_iterator <int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  replace(v1.begin(), v1.end(), 2, 42);
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  return 0;
}
