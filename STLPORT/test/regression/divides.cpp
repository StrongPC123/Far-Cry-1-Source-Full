// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <numeric>

#ifdef MAIN 
#define divides_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int divides_test(int, char**)
{
  cout<<"Results of divides_test:"<<endl;
int input [3] = { 2, 3, 4 };

  int result = accumulate(input, input + 3, 48, divides<int>());
  cout << "result = " << result << endl;
  return 0;
}
