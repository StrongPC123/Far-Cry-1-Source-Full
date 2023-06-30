// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <iterator>

#ifdef MAIN 
#define ostmit_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int ostmit_test(int, char**)
{
  cout<<"Results of ostmit_test:"<<endl;
int array [] = { 1, 5, 2, 4 };

  char* string = "hello";
  ostream_iterator<char> it1(cout);
  copy(string, string + 5, it1);
  cout << endl;
  ostream_iterator<int> it2(cout);
  copy(array, array + 4, it2);
  cout << endl;
  return 0;
}
