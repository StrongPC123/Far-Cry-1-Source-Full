// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define nthelem0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int nthelem0_test(int, char**)
{
  cout<<"Results of nthelem0_test:"<<endl;
int numbers[7] = { 5, 2, 4, 1, 0, 3 ,77};

  nth_element(numbers, numbers + 3, numbers + 6);
  for(int i = 0; i < 6; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
