// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>

#include "fadapter.h"

#ifdef MAIN 
#define foreach1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
static void print_sqr(int a_)
{
  cout << a_ * a_ << " ";
}
int foreach1_test(int, char**)
{
  cout<<"Results of foreach1_test:"<<endl;

  vector <int> v1(10);
  for(int i = 0; i < v1.size(); i++)
    v1[i] = i;
  for_each(v1.begin(), v1.end(), ptr_proc(print_sqr) );
  cout << endl;
  return 0;
}
