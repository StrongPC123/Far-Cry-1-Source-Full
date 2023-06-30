// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <iterator>
#include <functional>

#ifdef MAIN 
#define ptrbinf2_test main
#endif
static int sum(int x_, int y_)
{
  return x_ + y_;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int ptrbinf2_test(int, char**)
{
  cout<<"Results of ptrbinf2_test:"<<endl;

int input1 [4] = { 7, 2, 3, 5 };
int input2 [4] = { 1, 5, 5, 8 };

  int output [4];
  transform((int*)input1, (int*)input1 + 4, (int*)input2, (int*)output, ptr_fun(sum));
  for(int i = 0; i < 4; i++)
    cout << output[i] << endl;
  return 0;
}
