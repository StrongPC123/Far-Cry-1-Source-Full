// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define reviter2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int reviter2_test(int, char**)
{
  cout<<"Results of reviter2_test:"<<endl;
int array [] = { 1, 5, 2, 3 };

  std::vector<int> v(array, array + 4);
  std::vector<int>::reverse_iterator r;
  for(r = v.rbegin(); r != v.rend(); r++)
    cout << *r << endl;
  return 0;
}
