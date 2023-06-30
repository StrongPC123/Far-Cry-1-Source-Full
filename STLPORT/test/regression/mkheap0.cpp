// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define mkheap0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int mkheap0_test(int, char**)
{
  cout<<"Results of mkheap0_test:"<<endl;
int numbers[6] = { 5, 10, 4, 13, 11, 19 };

  make_heap(numbers, numbers + 6);
  for(int i = 6; i >= 1; i--)
  {
    cout << numbers[0] << endl;
    pop_heap(numbers, numbers + i);
  }
  return 0;
}
