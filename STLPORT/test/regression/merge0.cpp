// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define merge0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int merge0_test(int, char**)
{
  cout<<"Results of merge0_test:"<<endl;
int numbers1[5] = { 1, 6, 13, 25, 101 };
int numbers2[5] = {-5, 26, 36, 46, 99 };

  int result[10];
  merge((int*)numbers1, (int*)numbers1 + 5, (int*)numbers2, (int*)numbers2 + 5, (int*)result);
  for(int i = 0; i < 10; i++)
    cout << result[i] << ' ';
  cout << endl;
  return 0;
}
