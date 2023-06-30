// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstring>

#ifdef MAIN 
#define copy1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int copy1_test(int, char**)
{
  cout<<"Results of copy1_test:"<<endl;
char string[23] = "A string to be copied.";

  char result[23];
  copy(string, string + 23, result);
  cout << " Src: " << string << "\nDest: " << result << endl;
  return 0;
}
