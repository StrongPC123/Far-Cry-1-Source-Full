// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define adjfind2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
static int equal_length(const char* v1_, const char* v2_)
{
  return ::strlen(v1_) == ::strlen(v2_);
}
int adjfind2_test(int, char**)
{
  cout<<"Results of adjfind2_test:"<<endl;
typedef vector <char*> CStrVector;

char* names[] = { "Brett", "Graham", "Jack", "Mike", "Todd" };

  const int nameCount = sizeof(names)/sizeof(names[0]);
  CStrVector v(nameCount);
  for(int i = 0; i < nameCount; i++)
    v[i] = names[i];
  CStrVector::iterator location;
  location = adjacent_find(v.begin(), v.end(), equal_length);
  if(location != v.end())
    cout
      << "Found two adjacent strings of equal length: "
      << *location
      << " -and- "
      << *(location + 1)
      << endl;
  else
    cout << "Didn't find two adjacent strings of equal length.";
  return 0;
}
