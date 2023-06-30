// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <deque>
#include <algorithm>

#ifdef MAIN 
#define alg5_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int alg5_test(int, char**)
{
  cout<<"Results of alg5_test:"<<endl;
  deque<int> years;
  years.push_back(1962);
  years.push_back(1992);
  years.push_back(2001);
  years.push_back(1999);
  deque<int>::iterator i;
  for(i = years.begin(); i != years.end(); i++)
    cout << *i << endl;
  return 0;
}
