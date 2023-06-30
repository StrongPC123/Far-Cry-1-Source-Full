// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <algorithm>
#include <iostream>


#ifdef MAIN 
#define alg2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int alg2_test(int, char**)
{
  cout<<"Results of alg2_test:"<<endl;
int n = 0;
int i [] = { 1, 4, 2, 8, 2, 2 };

  count(i, i + 6, 2, n);
  cout << "Count of 2s = " << n << endl;
  return 0;
}
