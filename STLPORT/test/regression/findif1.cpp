// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define findif1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
static bool div_3(int a_)
{
  return a_ % 3 ? 0 : 1;
}
int findif1_test(int, char**)
{
  cout<<"Results of findif1_test:"<<endl;

  typedef vector <int> IntVec;
  IntVec v(10);
  for(int i = 0; i < v.size(); i++)
    v[i] =(i + 1) *(i + 1);
  IntVec::iterator iter;
  iter = find_if(v.begin(), v.end(), div_3);
  if(iter != v.end())
    cout
      << "Value "
      << *iter
      << " at offset "
      <<(iter - v.begin())
      << " is divisible by 3"
      << endl;
  return 0;
}
