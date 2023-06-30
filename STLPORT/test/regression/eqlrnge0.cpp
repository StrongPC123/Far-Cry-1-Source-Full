// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define eqlrnge0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int eqlrnge0_test(int, char**)
{
  cout<<"Results of eqlrnge0_test:"<<endl;
int numbers[10] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3 };

  pair <int*, int*> range = equal_range((int*)numbers, (int*)numbers + 10, 2);
  cout
    << "2 can be inserted from before index "
    <<(range.first - numbers)
    << " to before index "
    <<(range.second - numbers)
    << endl;
  return 0;
}
