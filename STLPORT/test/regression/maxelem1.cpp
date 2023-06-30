// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define maxelem1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int maxelem1_test(int, char**)
{
  cout<<"Results of maxelem1_test:"<<endl;
int numbers[6] = { 4, 10, 56, 11, -42, 19 };

  cout << *max_element((int*)numbers, (int*)numbers + 6) << endl;
  return 0;
}
