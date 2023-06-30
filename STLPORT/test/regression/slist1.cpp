// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <slist>
#include <iterator>

#ifdef MAIN 
#define slist1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

int slist1_test(int, char**)
{
  cout<<"Results of slist1_test:"<<endl;
char array [] = { 'x', 'l', 'x', 't', 's', 's' };
  ostream_iterator<char> o(cout,"");
  slist<char> str(array+0, array + 6);
  std::slist<char>::iterator i;
  cout << "original: ";
  copy(str.begin(), str.end(),o);
  cout << endl;
  cout << "reversed: ";
  str.reverse();
  for(i = str.begin(); i != str.end(); i++)
    cout << *i;
  cout << endl;
  cout << "removed: ";
  str.remove('x');
  for(i = str.begin(); i != str.end(); i++)
    cout << *i;
  cout << endl;
  cout << "uniqued: ";
  str.unique();
  for(i = str.begin(); i != str.end(); i++)
    cout << *i;
  cout << endl;
  cout << "sorted: ";
  str.sort();
  for(i = str.begin(); i != str.end(); i++)
    cout << *i;
  cout << endl;
  return 0;
}
