// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstring>
#include <iterator>
#include <functional>

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

#ifdef MAIN 
#define unique2_test main
#endif
static bool str_equal(const char* a_, const char* b_)
{
  return strcmp(a_, b_) == 0 ? 1 : 0;
}

int unique2_test(int, char**)
{
  cout<<"Results of unique2_test:"<<endl;

char* labels[] = { "Q","Q","W","W","E","E","R","T","T","Y","Y" };

  const unsigned count = sizeof(labels) / sizeof(labels[0]);
  ostream_iterator <char*> iter(cout);
  copy((char**)labels, (char**)labels + count, iter);
  cout << endl;
  unique((char**)labels, (char**)labels + count, str_equal);
  copy((char**)labels, (char**)labels + count, iter);
  cout << endl;
  return 0;
}
