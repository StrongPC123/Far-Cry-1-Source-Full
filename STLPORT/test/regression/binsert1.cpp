// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define binsert1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int binsert1_test(int, char**)
{
  cout<<"Results of binsert1_test:"<<endl;
char* array [] = { "laurie", "jennifer", "leisa" };

  vector<char*> names;
  copy(array, array + 3, back_insert_iterator<vector <char*> >(names));
  std::vector<char*>::iterator i;
  for(i = names.begin(); i != names.end(); i++)
    cout << *i << endl;
  return 0;
}
