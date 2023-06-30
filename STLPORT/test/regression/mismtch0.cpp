// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>

#ifdef MAIN 
#define mismtch0_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int mismtch0_test(int, char**)
{
  cout<<"Results of mismtch0_test:"<<endl;
int n1[5] = { 1, 2, 3, 4, 5 };
int n2[5] = { 1, 2, 3, 4, 5 };
int n3[5] = { 1, 2, 3, 2, 1 };

  pair <int*, int*> result = mismatch((int*)n1, (int*)n1 + 5, (int*)n2);
  if(result.first ==(n1 + 5) && result.second ==(n2 + 5))
    cout << "n1 and n2 are the same" << endl;
  else
    cout << "Mismatch at offset: " <<(result.first - n1) << endl;
  result = mismatch((int*)n1, (int*)n1 + 5, (int*)n3);
  if(result.first ==(n1 + 5) && result.second ==(n3 + 5))
    cout << "n1 and n3 are the same" << endl;
  else
    cout << "Mismatch at offset: " <<(result.first - n1) << endl;
  return 0;
}
