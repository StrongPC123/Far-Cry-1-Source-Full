// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <vector>
#include <iostream>

#ifdef MAIN 
#define count1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int count1_test(int, char**)
{
  cout<<"Results of count1_test:"<<endl;
  vector <int> numbers(100);
  for(int i = 0; i < 100; i++)
    numbers[i] = i % 3;
  int elements = 0;
  count(numbers.begin(), numbers.end(), 2, elements);
  cout << "Found " << elements << " 2's." << endl;
  return 0;
}
