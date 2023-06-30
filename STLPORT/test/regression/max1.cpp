// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define max1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int max1_test(int, char**)
{
  cout<<"Results of max1_test:"<<endl;
  cout << max(42, 100) << endl;
  return 0;
}
