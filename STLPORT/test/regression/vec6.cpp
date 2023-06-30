// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>

#ifdef MAIN 
#define vec6_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int vec6_test(int, char**)
{
  cout<<"Results of vec6_test:"<<endl;
int array [] = { 1, 4, 9, 16, 25, 36 };

  vector<int> v(array, array + 6);
  int i;
  for(i = 0; i < v.size(); i++)
    cout << "v[" << i << "] = " << v[i] << endl;
  cout << endl;
  v.erase(v.begin()); // Erase first element.
  for(i = 0; i < v.size(); i++)
    cout << "v[" << i << "] = " << v[i] << endl;
  cout << endl;
  v.erase(v.end() - 1); // Erase last element.
  for(i = 0; i < v.size(); i++)
    cout << "v[" << i << "] = " << v[i] << endl;
  cout << endl;
  v.erase(v.begin() + 1, v.end() - 1); // Erase all but first and last.
  for(i = 0; i < v.size(); i++)
    cout << "v[" << i << "] = " << v[i] << endl;
  cout << endl;
  /*
  v.erase(v.begin(), v.end()); // Erase all.
  */
  return 0;
}
