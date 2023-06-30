// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define minelem1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int minelem1_test(int, char**)
{
  cout<<"Results of minelem1_test:"<<endl;
int numbers[6] = { -10, 15, -100, 36, -242, 42 };

  cout << *min_element((int*)numbers, (int*)numbers + 6) << endl;
  return 0;
}
