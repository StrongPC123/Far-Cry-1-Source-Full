// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstring>
#include <iterator>
#include <functional>

#ifdef MAIN 
#define setdiff2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int setdiff2_test(int, char**)
{
  cout<<"Results of setdiff2_test:"<<endl;
char* word1 = "ABCDEFGHIJKLMNO";
char* word2 = "LMNOPQRSTUVWXYZ";

  ostream_iterator <char> iter(cout, " ");
  cout << "word1: ";
  copy(word1, word1 + ::strlen(word1), iter);
  cout << "\nword2: ";
  copy(word2, word2 + ::strlen(word2), iter);
  cout << endl;
  set_difference(word1, word1 + ::strlen(word1),
                  word2, word2 + ::strlen(word2),
                  iter,
                  less<char>());
  cout << endl;
  return 0;
}
