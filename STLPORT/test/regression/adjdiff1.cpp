// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <numeric>
#include <iterator>
#include <iostream>

#ifdef MAIN 
#define adjdiff1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int adjdiff1_test(int, char**)
{
  cout<<"Results of adjdiff1_test:"<<endl;
  vector <int> v(10);
  for(int i = 0; i < v.size(); i++)
    v[i] = i * i;
  vector <int> result(v.size());
  adjacent_difference(v.begin(), v.end(), result.begin());
  ostream_iterator<int> iter(cout, " ");
  // vector<int>::iterator iter;
  copy(v.begin(), v.end(), iter);
  cout << endl;
  copy(result.begin(), result.end(), iter);
  cout << endl;
  return 0;
}
