// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>

#ifdef MAIN 
#define nextprm0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int nextprm0_test(int, char**)
{
  cout<<"Results of nextprm0_test:"<<endl;
int v1[3] = { 0, 1, 2 };

  next_permutation(v1, v1 + 3);
  for(int i = 0; i < 3; i++)
    cout << v1[i] << ' ';
  cout << endl;
  return 0;
}
