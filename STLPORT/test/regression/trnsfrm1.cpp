// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define trnsfrm1_test main
#endif
static int negate_int(int a_)
{
  return -a_;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int trnsfrm1_test(int, char**)
{
  cout<<"Results of trnsfrm1_test:"<<endl;

int numbers[6] = { -5, -1, 0, 1, 6, 11 };

  int result[6];
  transform((int*)numbers, (int*)numbers + 6, (int*)result, negate_int);
  for(int i = 0; i < 6; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
