// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define replcpy1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int replcpy1_test(int, char**)
{
  cout<<"Results of replcpy1_test:"<<endl;
int numbers[6] = { 0, 1, 2, 0, 1, 2 };
int result[6] = { 0, 0, 0, 0, 0, 0 };

  replace_copy((int*)numbers, (int*)numbers + 6, (int*)result, 2, 42);
  for(int i = 0; i < 6; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
