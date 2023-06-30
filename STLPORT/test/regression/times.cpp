// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <numeric>

#ifdef MAIN 
#define times_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int times_test(int, char**)
{
  cout<<"Results of times_test:"<<endl;
int input [4] = { 1, 5, 7, 2 };

  int total = accumulate(input, input + 4, 1, multiplies<int>());
  cout << "total = " << total << endl;
  return 0;
}
