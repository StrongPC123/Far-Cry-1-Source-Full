// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define equal2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
bool values_squared(int a_, int b_)
{
  return(a_ * a_ == b_);
}
int equal2_test(int, char**)
{
  cout<<"Results of equal2_test:"<<endl;

  vector <int> v1(10);
  vector <int> v2(10);
  for(int i = 0; i < v1.size(); i++)
  {
    v1[i] = i;
    v2[i] = i * i;
  }
  if(equal(v1.begin(), v1.end(), v2.begin(), values_squared))
    cout << "v2[i] == v1[i] * v1[i]" << endl;
  else
    cout << "v2[i] != v1[i] * v1[i]" << endl;
  return 0;
}
