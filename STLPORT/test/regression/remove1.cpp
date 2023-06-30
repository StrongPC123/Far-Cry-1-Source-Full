// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define remove1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int remove1_test(int, char**)
{
  cout<<"Results of remove1_test:"<<endl;
int numbers[6] = { 1, 2, 3, 1, 2, 3 };

  remove((int*)numbers, (int*)numbers + 6, 1);
  for(int i = 0; i < 6; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
