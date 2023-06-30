// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define sort1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int sort1_test(int, char**)
{
  cout<<"Results of sort1_test:"<<endl;
int array[6] = { 1, 50, -10, 11, 42, 19 };

  sort(array, array + 6);
  for(int i = 0; i < 6; i++)
    cout << array[i] << ' ';
  cout << endl;
  return 0;
}
