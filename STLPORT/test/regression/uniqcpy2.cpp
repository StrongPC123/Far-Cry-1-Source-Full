// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

#ifdef MAIN 
#define uniqcpy2_test main
#endif
static bool str_equal(const char* a_, const char* b_)
{
  return strcmp(a_, b_) == 0 ? 1 : 0;
}

int uniqcpy2_test(int, char**)
{
  cout<<"Results of uniqcpy2_test:"<<endl;

  char* labels[] = { "Q","Q","W","W","E","E","R","T","T","Y","Y" };

  const unsigned count = sizeof(labels) / sizeof(labels[0]);
  ostream_iterator <char*> iter(cout);
  copy((char**)labels, (char**)labels + count, iter);
  cout << endl;
  char* uCopy[count];
  fill((char**)uCopy, (char**)uCopy + count, (char*)"");
  unique_copy((char**)labels, (char**)labels + count, (char**)uCopy, str_equal);
  copy((char**)labels, (char**)labels + count, iter);
  cout << endl;
  copy((char**)uCopy, (char**)uCopy + count, iter);
  cout << endl;
  return 0;
}
