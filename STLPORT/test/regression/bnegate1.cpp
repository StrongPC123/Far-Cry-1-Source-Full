// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <functional>

#ifdef MAIN 
#define bnegate1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int bnegate1_test(int, char**)
{
  cout<<"Results of bnegate1_test:"<<endl;
int array [4] = { 4, 9, 7, 1 };

  sort(array, array + 4, binary_negate<greater<int> >(greater<int>()));
  for(int i = 0; i < 4; i++)
    cout << array[i] << endl;
  return 0;
}
