// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <deque>
#include <algorithm>

#ifdef MAIN 
#define finsert2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int finsert2_test(int, char**)
{
  cout<<"Results of finsert2_test:"<<endl;
char* array [] = { "laurie", "jennifer", "leisa" };

  deque<char*> names;
  copy(array, array + 3, front_inserter(names));
  deque<char*>::iterator i;
  for(i = names.begin(); i != names.end(); i++)
    cout << *i << endl;
  return 0;
}
