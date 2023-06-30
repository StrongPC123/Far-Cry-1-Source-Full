// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define uniqcpy1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int uniqcpy1_test(int, char**)
{
  cout<<"Results of uniqcpy1_test:"<<endl;
int numbers[8] = { 0, 1, 1, 2, 2, 2, 3, 4 };
int result[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  unique_copy((int*)numbers, (int*)numbers + 8, (int*)result);
  for(int i = 0; i < 8; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
