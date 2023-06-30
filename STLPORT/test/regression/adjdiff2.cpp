// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <vector>
#include <numeric>
#include <iterator>
#include <iostream>


#ifdef MAIN 
#define adjdiff2_test main
#endif
static int mult(int a_, int b_)
{
  return a_ * b_;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int adjdiff2_test(int, char**)
{
  cout<<"Results of adjdiff2_test:"<<endl;

  vector <int> v(10);
  for(int i = 0; i < v.size(); i++)
    v[i] = i + 1;
  vector <int> rslt(v.size());
  adjacent_difference(v.begin(), v.end(), rslt.begin(), mult);
  ostream_iterator<int> iter(cout, " ");
  copy(v.begin(), v.end(), iter);
  cout << endl;
  copy(rslt.begin(), rslt.end(), iter);
  cout << endl;
  return 0;
}
