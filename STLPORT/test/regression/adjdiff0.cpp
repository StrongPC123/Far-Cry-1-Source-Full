// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <numeric>
#include <iostream>


#ifdef MAIN 
#define adjdiff0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

int adjdiff0_test(int, char**)
{
  cout<<"Results of adjdiff0_test:"<<endl;
int numbers[5] = { 1, 2, 4, 8, 16 };

  int difference[5];
  adjacent_difference(numbers, numbers + 5, (int*)difference);
  int i;
  for(i = 0; i < 5; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  for(i = 0; i < 5; i++)
    cout << difference[i] << ' ';
  cout << endl;
  return 0;
}
