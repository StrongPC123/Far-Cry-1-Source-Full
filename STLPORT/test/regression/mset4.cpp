// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <set>

#ifdef MAIN 
#define mset4_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int mset4_test(int, char**)
{
  cout<<"Results of mset4_test:"<<endl;
int array [] = { 3, 6, 1, 2, 3, 2, 6, 7, 9 };

  typedef multiset<int, less<int> > mset;
  mset s(array, array + 9);
  pair<mset::const_iterator, mset::const_iterator> p = s.equal_range(3);
  cout << "lower bound = " << *(p.first) << endl;
  cout << "upper bound = " << *(p.second) << endl;
  return 0;
}
