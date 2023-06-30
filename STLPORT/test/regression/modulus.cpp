// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <functional>

#ifdef MAIN 
#define modulus_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int modulus_test(int, char**)
{
  cout<<"Results of modulus_test:"<<endl;
int input1 [4] = { 6, 8, 10, 2 };
int input2 [4] = { 4, 2, 11, 3 };

  int output [4];
  transform((int*)input1, (int*)input1 + 4, (int*)input2, (int*)output, modulus<int>());
  for(int i = 0; i < 4; i++)
    cout << output[i] << endl;
  return 0;
}
