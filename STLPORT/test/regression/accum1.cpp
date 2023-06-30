// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <numeric>
#include <iostream>

#ifdef MAIN 
#define accum1_test main
#endif

#if !defined (STLPORT) || defined(_STLP_USE_NAMESPACES)
using namespace std;
#endif


int accum1_test(int, char**)
{
  cout<<"Results of accum1_test:"<<endl;
  vector <int> v(5);
  for(int i = 0; i < v.size(); i++)
    v[i] = i + 1;
  int sum = accumulate(v.begin(), v.end(), 0);
  cout << "sum = " << sum << endl;
  return 0;
}
