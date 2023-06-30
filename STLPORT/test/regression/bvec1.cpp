// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <vector>

#ifdef MAIN 
#define bvec1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

int bvec1_test(int, char**)
{
  bool ii[3]= {1,0,1};
  cout<<"Results of bvec1_test:"<<endl;
  bit_vector b(3);
  int i;
  for(i = 0; i < b.size(); i++)
    cout << b[i];
  cout << endl;
  b[0] = b[2] = 1;
  for(i = 0; i < b.size(); i++)
    cout << b[i];
  cout << endl;
  b.insert(b.begin(),(bool*)ii, ii+2);
  for(i = 0; i < b.size(); i++)
    cout << b[i];
  cout << endl;
  bit_vector bb = b;
  if (bb != b)
    exit(1);

  b[0] |= 0;
  b[1] |= 0;
  b[2] |= 1;
  b[3] |= 1;
  if ((b[0] != 1) || (b[1] != 0) || (b[2] != 1) || (b[3] != 1))
    exit(1);
  
  bb[0] &= 0;
  bb[1] &= 0;
  bb[2] &= 1;
  bb[3] &= 1;
  if ((bb[0] != 0) || (bb[1] != 0) || (bb[2] != 1) || (bb[3] != 0))
    exit(1);
  
  return 0;
}
