// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <iterator>
#include <functional>

#ifdef MAIN 
#define sort2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int sort2_test(int, char**)
{
  cout<<"Results of sort2_test:"<<endl;
int array[] = { 1, 50, -10, 11, 42, 19 };

  int count = sizeof(array) / sizeof(array[0]);
  ostream_iterator <int> iter(cout, " ");
  cout << "before: ";
  copy(array, array + count, iter);
  cout << "\nafter: ";
  sort(array, array + count, greater<int>());
  copy(array, array + count, iter);
  cout << endl;
  return 0;
}
