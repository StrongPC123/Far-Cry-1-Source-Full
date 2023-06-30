// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <functional>

#ifdef MAIN 
#define less_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int less_test(int, char**)
{
  cout<<"Results of less_test:"<<endl;
int array [4] = { 3, 1, 4, 2 };

  sort(array, array + 4, less<int>());
  for(int i = 0; i < 4; i++)
    cout << array[i] << endl;
  return 0;
}
