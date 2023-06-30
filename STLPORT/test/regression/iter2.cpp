// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define iter2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
static void print(const vector<const char*>& v_)
{
  std::vector<const char*>::const_iterator i;
  for(i = v_.begin(); i != v_.end(); i++)
    cout << *i << endl;
}
int iter2_test(int, char**)
{
  cout<<"Results of iter2_test:"<<endl;
  vector<const char*> v;
  v.push_back((char*) "zippy");
  v.push_back((char*) "motorboy");
  print(v);
  return 0;
}
