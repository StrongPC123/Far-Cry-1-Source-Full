// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstring>

#ifdef MAIN 
#define min2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

static bool str_compare(const char* a_, const char* b_)
{
  return strcmp(a_, b_) < 0 ? 1 : 0;
}

int min2_test(int, char**)
{
  cout<<"Results of min2_test:"<<endl;

  cout << min((char *)"shoe",(char *)"shine", str_compare) << endl;
  return 0;
}
