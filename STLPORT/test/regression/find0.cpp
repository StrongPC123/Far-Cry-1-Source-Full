// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define find0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int find0_test(int, char**)
{
  cout<<"Results of find0_test:"<<endl;
int numbers[10] = { 0, 1, 4, 9, 16, 25, 36, 49, 64 };

  int* location;
  location = find((int*)numbers, (int*)numbers + 10, 25);
  cout << "Found 25 at offset " <<(location - numbers) << endl;
  return 0;
}
