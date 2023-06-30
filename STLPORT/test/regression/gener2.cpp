// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <cstdlib>
#include <iterator>

#include "fib.h"


#ifdef MAIN 
#define gener2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

int gener2_test(int, char**)
{
  cout<<"Results of gener2_test:"<<endl;
  vector <int> v1(10);
  Fibonacci generator;
  generate(v1.begin(), v1.end(), generator);
  ostream_iterator<int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  return 0;
}
