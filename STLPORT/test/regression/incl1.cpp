// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define incl1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int incl1_test(int, char**)
{
  cout<<"Results of incl1_test:"<<endl;
  vector<int> v1(10);
  vector<int> v2(3);
  int i;
  for(i = 0; i < v1.size(); i++)
  {
    v1[i] = i;
  }
  if(includes(v1.begin(), v1.end(), v2.begin(), v2.end()))
    cout << "v1 includes v2" << endl;
  else
    cout << "v1 does not include v2" << endl;
  for(i = 0; i < v2.size(); i++)
    v2[i] = i + 3;
  if(includes(v1.begin(), v1.end(), v2.begin(), v2.end()))
    cout << "v1 includes v2" << endl;
  else
    cout << "v1 does not include v2" << endl;
  return 0;
}
