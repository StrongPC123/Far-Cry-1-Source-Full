// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <list>

#ifdef MAIN 
#define list2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int list2_test(int, char**)
{
  cout<<"Results of list2_test:"<<endl;
int array1 [] = { 1, 16 };
int array2 [] = { 4, 9 };

  list<int> l1(array1, array1 + 2);
  list<int> l2(array2, array2 + 2);
  std::list<int>::iterator i = l1.begin();
  i++;
  l1.splice(i, l2, l2.begin(), l2.end());
  i = l1.begin();
  while(i != l1.end())
    cout << *i++ << endl;
  return 0;
}
