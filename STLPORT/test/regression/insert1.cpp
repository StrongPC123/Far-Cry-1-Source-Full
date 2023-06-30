// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <iostream>
#include <deque>
#include <algorithm>

#ifdef MAIN 
#define insert1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

int insert1_test(int, char**)
{
  cout<<"Results of insert1_test:"<<endl;
  char* array1 [] = { "laurie", "jennifer", "leisa" };
  char* array2 [] = { "amanda", "saskia", "carrie" };

  deque<char*> names(array1, array1 + 3);
  std::deque<char*>::iterator i = names.begin() + 2;
  copy(array2, array2 + 3, insert_iterator<deque <char*> >(names, i));
  std::deque<char*>::iterator j;
  for(j = names.begin(); j != names.end(); j++)
    cout << *j << endl;
  return 0;
}
