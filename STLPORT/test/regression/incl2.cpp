// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>
#include <cstring>

#ifdef MAIN 
#define incl2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
static bool compare_strings(const char* s1_, const char* s2_)
{
  return strcmp(s1_, s2_) < 0 ? 1 : 0;
}
int incl2_test(int, char**)
{
  cout<<"Results of incl2_test:"<<endl;

char* names[] = {  "Todd", "Mike", "Graham", "Jack", "Brett"};

  const unsigned nameSize = sizeof(names)/sizeof(names[0]);
  vector <char*> v1(nameSize);
  for(int i = 0; i < v1.size(); i++)
  {
    v1[i] = names[i];
  }
  vector <char*> v2(2);

  v2[0] = "foo";
  v2[1] = "bar";
  sort(v1.begin(), v1.end(), compare_strings);
//  cout << "v1 sorted;\n";
  sort(v2.begin(), v2.end(), compare_strings);
//  cout << "v1 sorted;\n";

  bool inc = includes(v1.begin(), v1.end(),
                       v2.begin(), v2.end(),
                       compare_strings);
  if(inc)
    cout << "v1 includes v2" << endl;
  else
    cout << "v1 does not include v2" << endl;
  v2[0] = "Brett";
  v2[1] = "Todd";
  inc = includes(v1.begin(), v1.end(),
                  v2.begin(), v2.end(),
                  compare_strings);
  if(inc)
    cout << "v1 includes v2" << endl;
  else
    cout << "v1 does not include v2" << endl;
  return 0;
}
