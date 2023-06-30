// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define inplmrg1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int inplmrg1_test(int, char**)
{
  cout<<"Results of inplmrg1_test:"<<endl;
int numbers[6] = { 1, 10, 42, 3, 16, 32 };

  int i;
  for(i = 0; i < 6; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  inplace_merge(numbers, numbers + 3, numbers + 6);
  for(i = 0; i < 6; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
