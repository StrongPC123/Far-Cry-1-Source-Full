// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

#ifdef MAIN 
#define eqlrnge1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int eqlrnge1_test(int, char**)
{
  cout<<"Results of eqlrnge1_test:"<<endl;
  typedef vector <int> IntVec;
  IntVec v(10);
  for(int i = 0; i < v.size(); i++)
    v[i] = i / 3;
  ostream_iterator<int> iter(cout, " ");
  cout << "Within the collection:\n\t";
  copy(v.begin(), v.end(), iter);
  pair <IntVec::iterator, IntVec::iterator> range =
	equal_range(v.begin(), v.end(), 2);
  cout
    << "\n2 can be inserted from before index "
    <<(range.first - v.begin())
    << " to before index "
    <<(range.second - v.begin())
    << endl;
  return 0;
}
