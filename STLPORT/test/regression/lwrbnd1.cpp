// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <vector>
#include <iostream>

#ifdef MAIN 
#define lwrbnd1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int lwrbnd1_test(int, char**)
{
  cout<<"Results of lwrbnd1_test:"<<endl;
  std::vector <int> v1(20);
  for(int i = 0; i < v1.size(); i++)
  {
    v1[i] = i/4;
    cout << v1[i] << ' ';
  }
  std::vector <int>::iterator location =  lower_bound(v1.begin(), v1.end(), 3);
  cout
    << "\n3 can be inserted at index: "
    <<(location - v1.begin())
    << endl;
  return 0;
}
