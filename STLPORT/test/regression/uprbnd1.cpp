// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define uprbnd1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int uprbnd1_test(int, char**)
{
  cout<<"Results of uprbnd1_test:"<<endl;
  int array[20];
  for(int i = 0; i < 20; i++)
  {
    array[i] = i/4;
    cout << array[i] << ' ';
  }
  cout
    << "\n3 can be inserted at index: "
    <<(upper_bound((int*)array, (int*)array + 20, 3) - array)
    << endl;
  return 0;
}
