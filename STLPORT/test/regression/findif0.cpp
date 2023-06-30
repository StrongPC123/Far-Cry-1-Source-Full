// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define findif0_test main
#endif
static bool odd(int a_)
{
  return a_ % 2;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int findif0_test(int, char**)
{
  cout<<"Results of findif0_test:"<<endl;

int numbers[6] = { 2, 4, 8, 15, 32, 64 };

  int* location = find_if((int*)numbers, (int*)numbers + 6, odd);
  if(location != numbers + 6)
    cout
      << "Value "
      << *location
      << " at offset "
      <<(location - numbers)
      << " is odd"
      << endl;
  return 0;
}
