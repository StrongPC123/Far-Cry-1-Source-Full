// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <functional>

#ifdef MAIN 
#define bind1st2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int bind1st2_test(int, char**)
{
  cout<<"Results of bind1st2_test:"<<endl;
int array [3] = { 1, 2, 3 };

  int* p = remove_if((int*)array, (int*)array + 3, bind1st(less<int>(), 2));
  for(int* i = array; i != p; i++)
    cout << *i << endl;
  return 0;
}
