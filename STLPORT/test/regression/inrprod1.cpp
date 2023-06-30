// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <numeric>
#include <iostream>
#include <iterator>
#include <string>

#ifdef MAIN 
#define inrprod1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int inrprod1_test(int, char**)
{
  cout<<"Results of inrprod1_test:"<<endl;
  vector <int> v1(3);
  vector <int> v2(v1.size());
  for(int i = 0; i < v1.size(); i++)
  {
    v1[i] = i + 1;
    v2[i] = v1.size() - i;
  }
  ostream_iterator<int> iter(cout, " ");
  cout << "Inner product(sum of products) of:\n\t";
  copy(v1.begin(), v1.end(), iter);
  cout << "\n\t";
  copy(v2.begin(), v2.end(), iter);
  int result = inner_product(v1.begin(), v1.end(), v2.begin(), 0);
  cout << "\nis: " << result << endl;
  return 0;
}
