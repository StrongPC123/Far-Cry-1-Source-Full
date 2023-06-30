// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <numeric>
#include <iostream>
#include <iterator>
#include <string>

#ifdef MAIN 
#define inrprod2_test main
#endif
static int add(int a_, int b_)
{
  return a_ + b_;
}

static int mult(int a_, int b_)
{
  return a_ * b_;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int inrprod2_test(int, char**)
{
  cout<<"Results of inrprod2_test:"<<endl;

  vector <int> v1(3);
  vector <int> v2(v1.size());
  for(int i = 0; i < v1.size(); i++)
  {
    v1[i] = i + 1;
    v2[i] = v1.size() - i;
  }
  ostream_iterator<int> iter(cout, " ");
  cout << "Inner product(product of sums):\n\t";
  copy(v1.begin(), v1.end(), iter);
  cout << "\n\t";
  copy(v2.begin(), v2.end(), iter);
  int result =
    inner_product(v1.begin(), v1.end(),
                   v2.begin(),
                   1,
                   mult, add);
  cout << "\nis: " << result << endl;
  return 0;
}
