// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define count0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int count0_test(int, char**)
{
  cout<<"Results of count0_test:"<<endl;
int numbers[10] = { 1, 2, 4, 1, 2, 4, 1, 2, 4, 1 };

  int result = 0;
  count(numbers, numbers + 10, 1, result);
  cout << "Found " << result << " 1's." << endl;
  return 0;
}
