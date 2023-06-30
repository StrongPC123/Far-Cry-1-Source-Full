// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define min1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int min1_test(int, char**)
{
  cout<<"Results of min1_test:"<<endl;
  cout << min(42, 100) << endl;
  return 0;
}
