// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define swap1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int swap1_test(int, char**)
{
  cout<<"Results of swap1_test:"<<endl;
  int a = 42;
  int b = 19;
  cout << "a = " << a << " b = " << b << endl;
  swap(a, b);
  cout << "a = " << a << " b = " << b << endl;
  return 0;
}
