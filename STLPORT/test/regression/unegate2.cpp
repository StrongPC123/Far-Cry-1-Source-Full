// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include "unary.h"

#ifdef MAIN 
#define unegate2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int unegate2_test(int, char**)
{
  cout<<"Results of unegate2_test:"<<endl;

int array [3] = { 1, 2, 3 };

  int* p = find_if((int*)array, (int*)array + 3, not1(odd()));
  if(p != array + 3)
    cout << *p << endl;
  return 0;
}
