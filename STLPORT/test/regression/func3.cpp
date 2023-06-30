// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#ifdef MAIN 
#define func3_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int func3_test(int, char**)
{
  cout<<"Results of func3_test:"<<endl;
  vector<int>v;
  v.push_back(4);
  v.push_back(1);
  v.push_back(5);
  sort(v.begin(), v.end(), greater<int>());
  std::vector<int>::iterator i;
  for(i = v.begin(); i != v.end(); i++)
    cout << *i << endl;
  return 0;
}
