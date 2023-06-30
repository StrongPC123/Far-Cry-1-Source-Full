// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <iterator>

#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>
#include <numeric>
//#include <functional>

#ifdef MAIN 
#define rndshuf1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int rndshuf1_test(int, char**)
{
  cout<<"Results of rndshuf1_test:"<<endl;
  vector <int> v1(10);
  iota(v1.begin(), v1.end(), 0);
  ostream_iterator <int> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  for(int i = 0; i < 3; i++)
  {
    random_shuffle(v1.begin(), v1.end());
    copy(v1.begin(), v1.end(), iter);
    cout << endl;
  }
  return 0;
}
