// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <vector>
#include <iostream>
#include <iterator>

#ifdef MAIN 
#define pheap1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int pheap1_test(int, char**)
{
  cout<<"Results of pheap1_test:"<<endl;
  vector<int> v;

  v.push_back(1);
  v.push_back(20);
  v.push_back(4);
  make_heap(v.begin(), v.end());

  v.push_back(7);
  push_heap(v.begin(), v.end());

  sort_heap(v.begin(), v.end());
  ostream_iterator<int> iter(cout, " ");
  copy(v.begin(), v.end(), iter);
  cout << endl;

  return 0;
}
