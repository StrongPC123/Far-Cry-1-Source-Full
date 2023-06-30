// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <numeric>
#include <iostream>

#ifdef MAIN 
#define partsum0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int partsum0_test(int, char**)
{
  cout<<"Results of partsum0_test:"<<endl;
int numbers[6] = { 1, 2, 3, 4, 5, 6 };

  int result[6];
  partial_sum((int*)numbers, (int*)numbers + 6, (int*)result);
  for(int i = 0; i < 6; i ++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
