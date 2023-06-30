// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <vector>
#include <iostream>

#ifdef MAIN 
#define replif1_test main
#endif
static bool odd(int a_)
{
  return a_ % 2;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int replif1_test(int, char**)
{
  cout<<"Results of replif1_test:"<<endl;

  vector <int> v1(10);
  int i;
  for(i = 0; i < v1.size(); i++)
  {
    v1[i] = i % 5;
    cout << v1[i] << ' ';
  }
  cout << endl;
  replace_if(v1.begin(), v1.end(), odd, 42);
  for(i = 0; i < v1.size(); i++)
    cout << v1[i] << ' ';
  cout << endl;
  return 0;
}
