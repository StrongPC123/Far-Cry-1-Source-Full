// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstring>

#ifdef MAIN 
#define swprnge1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int swprnge1_test(int, char**)
{
  cout<<"Results of swprnge1_test:"<<endl;
  char word1[] = "World";
  char word2[] = "Hello";
  cout << word1 << " " << word2 << endl;
  swap_ranges((char*)word1, (char*)word1 + ::strlen(word1), (char*)word2);
  cout << word1 << " " << word2 << endl;
  return 0;
}
