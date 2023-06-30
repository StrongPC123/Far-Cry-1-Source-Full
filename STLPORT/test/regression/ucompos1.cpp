// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include "unary.h"

#ifdef MAIN 
#define ucompos1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

int ucompos1_test(int, char**)
{
  cout<<"Results of ucompos1_test:"<<endl;

int input [3] = { -1, -4, -16 };

  int output [3];
  transform((int*)input, (int*)input + 3, (int*)output, 
	    //	    compose1(square_root(), negate<int>()));
  	    unary_compose<square_root, negate<int> >(square_root(), negate<int>()));
  for(int i = 0; i < 3; i++)
    cout << output[i] << endl;
  return 0;
}
