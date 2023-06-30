// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define revcopy1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int revcopy1_test(int, char**)
{
  cout<<"Results of revcopy1_test:"<<endl;
int numbers[6] = { 0, 1, 2, 3, 4, 5 };

  int result[6];
  reverse_copy((int*)numbers, (int*)numbers + 6, (int*)result);
  int i;
  for(i = 0; i < 6; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  for(i = 0; i < 6; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
