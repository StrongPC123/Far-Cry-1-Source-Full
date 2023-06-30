// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define vec5_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int vec5_test(int, char**)
{
  cout<<"Results of vec5_test:"<<endl;
int array [] = { 1, 4, 9, 16 };

  vector<int> v(array, array + 4);
  for(int i = 0; i < v.size(); i++)
    cout << "v[" << i << "] = " << v[i] << endl;
  return 0;
}
