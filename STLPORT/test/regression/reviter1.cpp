// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define reviter1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int reviter1_test(int, char**)
{
  cout<<"Results of reviter1_test:"<<endl;
int array [] = { 1, 5, 2, 3 };

  vector<int> v(array, array + 4);
  typedef std::vector<int>::reverse_iterator reviter;
  reviter r(v.end());
  while(!(r ==v.rbegin()))
    cout << *r++ << endl;
  return 0;
}
