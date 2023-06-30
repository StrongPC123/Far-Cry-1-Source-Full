// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <set>

#ifdef MAIN 
#define set2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int set2_test(int, char**)
{
  cout<<"Results of set2_test:"<<endl;
  set<int, less<int> > s;
  pair<std::set<int, less<int> >::const_iterator, bool> p = s.insert(42);
  if(p.second)
   cout << "Inserted new element " << *(p.first) << endl;
  else
   cout << "Existing element = " << *(p.first) << endl;
  p = s.insert(42);
  if(p.second)
   cout << "Inserted new element " << *(p.first) << endl;
  else
   cout << "Existing element = " << *(p.first) << endl;
  return 0;
}
