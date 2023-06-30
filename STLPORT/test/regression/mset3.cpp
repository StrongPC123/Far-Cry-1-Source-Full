// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <set>

#ifdef MAIN 
#define mset3_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int mset3_test(int, char**)
{
  cout<<"Results of mset3_test:"<<endl;
int array [] = { 3, 6, 1, 2, 3, 2, 6, 7, 9 };

  multiset<int, less<int> > s(array, array + 9);
  std::multiset<int, less<int> >::iterator i;
  i = s.lower_bound(3);
  cout << "lower bound = " << *i << endl;
  i = s.upper_bound(3);
  cout << "upper bound = " << *i << endl;
  return 0;
}
