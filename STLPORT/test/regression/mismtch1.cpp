// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

#ifdef MAIN 
#define mismtch1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int mismtch1_test(int, char**)
{
  cout<<"Results of mismtch1_test:"<<endl;
  typedef vector<int> IntVec;
  IntVec v1(10);
  IntVec v2(v1.size());
  iota(v1.begin(), v1.end(), 0);
  iota(v2.begin(), v2.end(), 0);
  pair <IntVec::iterator, IntVec::iterator> result =
      mismatch(v1.begin(), v1.end(), v2.begin());
  if(result.first == v1.end() && result.second == v2.end())
    cout << "v1 and v2 are the same" << endl;
  else
    cout << "mismatch at index: " <<(result.first - v1.begin()) << endl;
  v2[v2.size()/2] = 42;
  result = mismatch(v1.begin(), v1.end(), v2.begin());
  if(result.first == v1.end() && result.second == v2.end())
    cout << "v1 and v2 are the same" << endl;
  else
    cout << "mismatch at index: " <<(result.first - v1.begin()) << endl;
  return 0;
}
