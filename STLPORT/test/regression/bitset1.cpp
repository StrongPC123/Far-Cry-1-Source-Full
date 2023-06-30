// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.


#include <stdexcept>
#include <iostream>
#include <bitset>
#include <algorithm>

#if defined (_STLP_MSVC) && (_MSC_VER < 1300)
# define _STLP_NON_TYPE_TMPL_PARAM_BUG
# define _STLP_NO_EXPLICIT_FUNCTION_TMPL_ARGS
#endif

# ifdef _STLP_NO_EXPLICIT_FUNCTION_TMPL_ARGS
# define CHART
# else
# define CHART <char, char_traits<char>, allocator<char> >
# endif

#ifdef MAIN 
#define bitset1_test main
#endif

#if !defined (STLPORT) || defined(_STLP_USE_NAMESPACES)
using namespace std;
#endif

int bitset1_test(int, char**)
{
  cout<<"Results of bitset1_test:"<<endl;

# if ! (defined (_STLP_MSVC) && (_MSC_VER < 1300)) && ! (defined (__SUNPRO_CC) && (__SUNPRO_CC < 0x500))
  bitset<13U> b1(0xFFFF);
  bitset<13U> b2(0x1111);
  // Boris : MSVC just cannot take it right
  cout << "b1 size = " << b1.size() << endl;
# if !defined (_STLP_NON_TYPE_TMPL_PARAM_BUG)
  cout << "b1 = "<<b1<<endl;
# else
  cout << "b1 = "<<b1.to_string CHART ()<<endl;
# endif
  cout << "b2 size = " << b2.size() << endl;
# if defined (_STLP_NON_TYPE_TMPL_PARAM_BUG)
  cout << "b2 = "<<b2.to_string CHART ()<<endl;
# else
  cout << "b2 = "<<b2<<endl;
# endif
  b1 = b1^(b2<<2);
# ifdef _STLP_NON_TYPE_TMPL_PARAM_BUG
  cout << "b1 = "<<b1.to_string CHART ()<<endl;
# else
  cout << "b1 = "<<b1<<endl;
# endif
# endif /* MSVC */
  return 0;
}

