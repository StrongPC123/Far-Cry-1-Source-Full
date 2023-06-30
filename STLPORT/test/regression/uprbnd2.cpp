// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstring>

#ifdef MAIN 
#define uprbnd2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

static bool char_str_less(const char* a_, const char* b_)
{
  return strcmp(a_, b_) < 0 ? 1 : 0;
}

int uprbnd2_test(int, char**)
{
  cout<<"Results of uprbnd2_test:"<<endl;

char* str [] = { "a", "a", "b", "b", "q", "w", "z" };

  const unsigned strCt = sizeof(str)/sizeof(str[0]);
  // DEC C++ generates incorrect template instatiation code 
  // for "d" so must cast 
  cout
    << "d can be inserted at index: "
    << (upper_bound((char**)str,  (char**)str + strCt, (const char *)"d", char_str_less) - str)
    << endl;
  return 0;
}
