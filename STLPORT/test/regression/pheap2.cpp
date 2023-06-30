// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <functional>
#include <algorithm>
#include <vector>
#include <iterator>
#include <iostream>

#ifdef MAIN 
#define pheap2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int pheap2_test(int, char**)
{
  cout<<"Results of pheap2_test:"<<endl;
  vector<int> v;

  v.push_back(1);
  v.push_back(20);
  v.push_back(4);
  make_heap(v.begin(), v.end(), greater<int>());

  v.push_back(7);
  push_heap(v.begin(), v.end(), greater<int>());

  sort_heap(v.begin(), v.end(), greater<int>());
  ostream_iterator<int> iter(cout, " ");
  copy(v.begin(), v.end(), iter);
  cout << endl;

  return 0;
}
