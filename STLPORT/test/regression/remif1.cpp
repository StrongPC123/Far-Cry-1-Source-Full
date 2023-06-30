// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define remif1_test main
#endif
static bool odd(int a_)
{
  return a_ % 2;
}

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int remif1_test(int, char**)
{
  cout<<"Results of remif1_test:"<<endl;

int numbers[6] = { 0, 0, 1, 1, 2, 2 };

  remove_if((int*)numbers, (int*)numbers + 6, odd);
  for(int i = 0; i < 6; i++)
    cout << numbers[i] << ' ';
  cout << endl;
  return 0;
}
