// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <iostream>

#ifdef MAIN 
#define merge2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int merge2_test(int, char**)
{
  cout<<"Results of merge2_test:"<<endl;
  vector <int> v1(5);
  vector <int> v2(v1.size());
  for(int i = 0; i < v1.size(); i++)
  {
    v1[i] = 10 - i;
    v2[i] =  7 - i;
  }
  vector <int> result(v1.size() + v2.size());
  merge(v1.begin(), v1.end(),
         v2.begin(), v2.end(),
         result.begin(),
         greater<int>() );
  ostream_iterator <int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  copy(v2.begin(), v2.end(), iter);
  cout << endl;
  copy(result.begin(), result.end(), iter);
  cout << endl;
  return 0;
}
