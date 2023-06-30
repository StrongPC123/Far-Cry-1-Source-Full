// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define func1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
static bool bigger(int i_)
{
  return i_ > 3;
}
int func1_test(int, char**)
{
  cout<<"Results of func1_test:"<<endl;

  vector<int>v;
  v.push_back(4);
  v.push_back(1);
  v.push_back(5);
  int n = 0;
  count_if(v.begin(), v.end(), bigger, n);
  cout << "Number greater than 3 = " << n << endl;
  return 0;
}
