// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <deque>
#include <queue>

#ifdef MAIN 
#define pqueue1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int pqueue1_test(int, char**)
{
  cout<<"Results of pqueue1_test:"<<endl;
  priority_queue<int, deque<int>, less<int> > q;
  q.push(42);
  q.push(101);
  q.push(69);
  while(!q.empty())
  {
    cout << q.top() << endl;
    q.pop();
  }
  return 0;
}
