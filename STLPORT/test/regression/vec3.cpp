// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>
#include <algorithm>
#ifdef MAIN 
#define vec3_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int vec3_test(int, char**)
{
  cout<<"Results of vec3_test:"<<endl;
# ifdef __STLPORT_VERSION
  typedef  __vector__<char, allocator<char> > vec_type;
# else
  typedef  vector<char> vec_type;
# endif
  vec_type v1; // Empty vector of characters.
  v1.push_back('h');
  v1.push_back('i');
  cout << "v1 = " << v1[0] << v1[1] << endl;
  vec_type v2(v1.begin(), v1.end());
  v2[1] = 'o'; // Replace second character.
  cout << "v2 = " << v2[0] << v2[1] << endl;
  cout << "(v1 == v2) = " <<(v1 == v2) << endl;
  cout << "(v1 < v2) = " <<(v1 < v2) << endl;
  return 0;
}
