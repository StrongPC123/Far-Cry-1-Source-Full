// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define remcopy1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int remcopy1_test(int, char**)
{
  cout<<"Results of remcopy1_test:"<<endl;
int numbers[6] = { 1, 2, 3, 1, 2, 3 };
int result[6] = { 0, 0, 0, 0, 0, 0 };

  remove_copy((int*)numbers, (int*)numbers + 6, (int*)result, 2);
  for(int i = 0; i < 6; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
