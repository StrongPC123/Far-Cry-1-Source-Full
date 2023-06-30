// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <hash_set>

#ifdef MAIN 
#define hset2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

// __STL_TYPE_TRAITS_POD_SPECIALIZE(_Hashtable_node<int>*);

int hset2_test(int, char**)
{
  cout<<"Results of hset2_test:"<<endl;
  hash_set<int, hash<int>, equal_to<int> > s;
  pair<std::hash_set<int, hash<int>, equal_to<int> >::const_iterator, bool> p = s.insert(42);
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
