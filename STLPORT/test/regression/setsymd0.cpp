// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define setsymd0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int setsymd0_test(int, char**)
{
  cout<<"Results of setsymd0_test:"<<endl;
int v1[3] = { 13, 18, 23 };
int v2[4] = { 10, 13, 17, 23 };
int result[4] = { 0, 0, 0, 0 };

  set_symmetric_difference((int*)v1, (int*)v1 + 3, (int*)v2, (int*)v2 + 4, (int*)result);
  for(int i = 0; i < 4; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
