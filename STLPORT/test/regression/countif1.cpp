// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <vector>
#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define countif1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
static int odd(int a_)
{
  return a_ % 2;
}
int countif1_test(int, char**)
{
  cout<<"Results of countif1_test:"<<endl;

  vector <int> numbers(100);
  for(int i = 0; i < 100; i++)
    numbers[i] = i % 3;
  int elements = 0;
  count_if(numbers.begin(), numbers.end(), odd, elements);
  cout << "Found " << elements << " odd elements." << endl;
  return 0;
}
