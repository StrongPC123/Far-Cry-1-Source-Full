// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define iterswp0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int iterswp0_test(int, char**)
{
  cout<<"Results of iterswp0_test:"<<endl;
int numbers[6] = { 0, 1, 2, 3, 4, 5 };

  iter_swap(numbers, numbers + 3);
  for(int i = 0; i < 6; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
