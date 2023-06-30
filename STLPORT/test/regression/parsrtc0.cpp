// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define parsrtc0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int parsrtc0_test(int, char**)
{
  cout<<"Results of parsrtc0_test:"<<endl;
int numbers[6] = { 5, 2, 4, 3, 1, 6 };

  int result[3];
  partial_sort_copy((int*)numbers, (int*)numbers + 6, (int*)result, (int*)result + 3);
  for(int i = 0; i < 3; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
