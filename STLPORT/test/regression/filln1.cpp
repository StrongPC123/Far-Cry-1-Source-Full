// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define filln1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int filln1_test(int, char**)
{
  cout<<"Results of filln1_test:"<<endl;
  vector <int> v(10);
  fill_n(v.begin(), v.size(), 42);
  for(int i = 0; i < 10; i++)
    cout << v[i] << ' ';
  cout << endl;
  return 0;
}
