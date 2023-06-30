// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define find1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int find1_test(int, char**)
{
  cout<<"Results of find1_test:"<<endl;
int years[] = { 1942, 1952, 1962, 1972, 1982, 1992 };

  const unsigned yearCount = sizeof(years) / sizeof(years[0]);
  int* location = find((int*)years, (int*)years + yearCount, 1972);
  cout << "Found 1972 at offset " <<(location - years) << endl;
  return 0;
}
