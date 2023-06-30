// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstdlib>

#include "fadapter.h"

#ifdef MAIN 
#define gener1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

static  int cxxrand() { return rand();}

int gener1_test(int, char**)
{
  cout<<"Results of gener1_test:"<<endl;
  int numbers[10];
#if defined(__MVS__)
  generate(numbers, numbers + 10, ptr_gen(cxxrand));
#else
  generate(numbers, numbers + 10, cxxrand);
#endif

  for(int i = 0; i < 10; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
