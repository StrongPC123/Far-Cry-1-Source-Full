// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <iostream>
#include <map>

#ifdef MAIN 
#define mmap2_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

int mmap2_test(int, char**)
{
  cout<<"Results of mmap2_test:"<<endl;
static int hook=0;
int bb=0;


typedef pair<const int, char> pair_type;

pair_type p1(3, 'c');
pair_type p2(6, 'f');
pair_type p3(1, 'a');
pair_type p4(2, 'b');
pair_type p5(3, 'x');
pair_type p6(6, 'f');

  typedef multimap<int, char, less<int> > mmap;

pair_type array [] =
  {
    p1,
    p2,
    p3,
    p4,
    p5,
    p6
  };

  mmap m(array+0, array + 6);
  mmap::iterator i;
  i = m.lower_bound(3);
  cout << "lower bound:" << endl;
  cout <<(*i).first << " -> " <<(*i).second << endl;
  i = m.upper_bound(3);
  cout << "upper bound:" << endl;
  cout <<(*i).first << " -> " <<(*i).second << endl;
  return 0;
}
