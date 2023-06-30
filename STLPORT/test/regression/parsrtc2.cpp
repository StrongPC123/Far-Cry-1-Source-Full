// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iterator>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>

#ifdef MAIN 
#define parsrtc2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

static bool str_compare(const char* a_, const char* b_)
{
  return strcmp(a_, b_) < 0 ? 1 : 0;
}

int parsrtc2_test(int, char**)
{
  cout<<"Results of parsrtc2_test:"<<endl;

char* names[] = { "aa", "ff", "dd", "ee", "cc", "bb" };

  const unsigned nameSize = sizeof(names) / sizeof(names[0]);
  vector <char*> v1(nameSize);
  for(int i = 0; i < v1.size(); i++)
    v1[i] = names[i];
  ostream_iterator<char*> iter(cout, " ");
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  vector <char*> result(5);
  partial_sort_copy(v1.begin(),
                     v1.end(),
                     result.begin(),
                     result.end(),
                     str_compare);
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  return 0;
}
