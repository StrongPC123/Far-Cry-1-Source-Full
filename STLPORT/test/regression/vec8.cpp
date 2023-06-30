// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define vec8_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int vec8_test(int, char**)
{
  cout<<"Results of vec8_test:"<<endl;
  vector<int> v;
  cout << "capacity = " << v.capacity() << endl;
  v.push_back(42);
  cout << "capacity = " << v.capacity() << endl;
  v.reserve(5000);
  cout << "capacity = " << v.capacity() << endl;
  return 0;
}
