// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <numeric>
#include <iostream>

#ifdef MAIN 
#define iota1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int iota1_test(int, char**)
{
  cout<<"Results of iota1_test:"<<endl;
  int numbers[10];
  iota(numbers, numbers + 10, 42);
  for(int i = 0; i < 10; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
