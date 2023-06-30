// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define rotcopy0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int rotcopy0_test(int, char**)
{
  cout<<"Results of rotcopy0_test:"<<endl;
int numbers[6] = { 0, 1, 2, 3, 4, 5 };

  int result[6];
  rotate_copy((int*)numbers, (int*)numbers + 3, (int*)numbers + 6, (int*)result);
  for(int i = 0; i < 6; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
