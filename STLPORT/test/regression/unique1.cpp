// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define unique1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int unique1_test(int, char**)
{
  cout<<"Results of unique1_test:"<<endl;
int numbers[8] = { 0, 1, 1, 2, 2, 2, 3, 4 };

  unique((int*)numbers, (int*)numbers + 8);
  for(int i = 0; i < 8; i ++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
