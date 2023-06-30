// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <functional>

#ifdef MAIN 
#define negate_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int negate_test(int, char**)
{
  cout<<"Results of negate_test:"<<endl;
int input [3] = { 1, 2, 3 };

  int output[3];
  transform((int*)input, (int*)input + 3, (int*)output, negate<int>());
  for(int i = 0; i < 3; i++)
    cout << output[i] << endl;
  return 0;
}
