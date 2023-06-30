// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include "unary.h"

#ifdef MAIN 
#define unegate1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int unegate1_test(int, char**)
{
  cout<<"Results of unegate1_test:"<<endl;

int array [3] = { 1, 2, 3 };

  int* p = find_if((int*)array, (int*)array + 3, unary_negate<odd>(odd()));
  if(p != array + 3)
    cout << *p << endl;
  return 0;
}
