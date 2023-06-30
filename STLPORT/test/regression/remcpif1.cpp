// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define remcpif1_test main
#endif
static bool odd(int a_)
{
  return a_ % 2;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int remcpif1_test(int, char**)
{
  cout<<"Results of remcpif1_test:"<<endl;

int numbers[6] = { 1, 2, 3, 1, 2, 3 };
int result[6] = { 0, 0, 0, 0, 0, 0 };

  remove_copy_if((int*)numbers, (int*)numbers + 6, (int*)result, odd);
  for(int i = 0; i < 6; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
