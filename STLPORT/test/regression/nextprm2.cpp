// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <iterator>

#include <vector>
#include <algorithm>
#include <numeric>
#include <iterator>
#include <iostream>

#ifdef MAIN 
#define nextprm2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int nextprm2_test(int, char**)
{
  cout<<"Results of nextprm2_test:"<<endl;
  vector <char> v1(3);
  iota(v1.begin(), v1.end(), 'A');
  ostream_iterator<char> iter(cout);
  copy(v1.begin(), v1.end(), iter);
  cout << endl;
  for(int i = 0; i < 9; i++)
  {
    next_permutation(v1.begin(), v1.end(), less<char>());
    copy(v1.begin(), v1.end(), iter);
    cout << endl;
  }
  return 0;
}
