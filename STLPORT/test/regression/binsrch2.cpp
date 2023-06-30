// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstring>

#ifdef MAIN 
#define binsrch2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
static bool str_compare(const char* a_, const char* b_)
{
  return strcmp(a_, b_) < 0 ? 1 : 0;
}
int binsrch2_test(int, char**)
{
  cout<<"Results of binsrch2_test:"<<endl;

char* labels[] = { "aa", "dd", "ff", "jj", "ss", "zz" };

  const unsigned count = sizeof(labels) / sizeof(labels[0]);
  // DEC C++ generates incorrect template instatiation code 
  // for "ff" so must cast 
  if(binary_search(labels, labels + count, (const char *)"ff", str_compare))
    cout << "ff is in labels." << endl;
  else
    cout << "ff is not in labels." << endl;
  return 0;
}
