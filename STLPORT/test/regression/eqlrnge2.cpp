// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <functional>

#ifdef MAIN 
#define eqlrnge2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int eqlrnge2_test(int, char**)
{
  cout<<"Results of eqlrnge2_test:"<<endl;
char chars[] = "aabbccddggghhklllmqqqqssyyzz";

  const unsigned count = sizeof(chars) - 1;
  ostream_iterator<char> iter(cout);
  cout << "Within the collection:\n\t";
  copy(chars, chars + count, iter);
  pair <char*, char*> range =
	 equal_range((char*)chars, (char*)chars + count, 'q', less<char>());
  cout
    << "\nq can be inserted from before index "
    <<(range.first - chars)
    << " to before index "
    <<(range.second - chars)
    << endl;
  return 0;
}
