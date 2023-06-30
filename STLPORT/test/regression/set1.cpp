// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <set>

#ifdef MAIN 
#define set1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int set1_test(int, char**)
{
  cout<<"Results of set1_test:"<<endl;
  set<int, less<int> > s;
  cout << "count(42) = " << s.count(42) << endl;
  s.insert(42);
  cout << "count(42) = " << s.count(42) << endl;
  s.insert(42);
  cout << "count(42) = " << s.count(42) << endl;
  int count = s.erase(42);
  cout << count << " elements erased" << endl;
  return 0;
}
