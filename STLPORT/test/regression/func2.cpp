// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define func2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

static bool bigger_than(int x_, int y_)
{
  return x_ > y_;
}
int func2_test(int, char**)
{
  cout<<"Results of func2_test:"<<endl;

  std::vector<int>v;
  v.push_back(4);
  v.push_back(1);
  v.push_back(5);
  sort(v.begin(), v.end(), bigger_than);
  std::vector<int>::iterator i;
  for(i = v.begin(); i != v.end(); i++)
    cout << *i << endl;
  return 0;
}
