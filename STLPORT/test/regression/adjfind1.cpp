// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define adjfind1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int adjfind1_test(int, char**)
{
  cout<<"Results of adjfind1_test:"<<endl;
  typedef vector<int> IntVector;
  IntVector v(10);
  for(int i = 0; i < v.size(); i++)
    v[i] = i;
  IntVector::iterator location;
  location = adjacent_find(v.begin(), v.end());
  if(location != v.end())
    cout << "Found adjacent pair of: " << *location << endl;
  else
    cout << "No adjacent pairs" << endl;
  v[6] = 7;
  location = adjacent_find(v.begin(), v.end());
  if(location != v.end())
    cout << "Found adjacent pair of: " << *location << endl;
  else
    cout << "No adjacent pairs" << endl;
  return 0;
}
