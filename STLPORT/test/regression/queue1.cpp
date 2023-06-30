// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <algorithm>
#include <queue>
#include <list>

#ifdef MAIN 
#define queue1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int queue1_test(int, char**)
{
  cout<<"Results of queue1_test:"<<endl;
  queue<int, list<int> > q;
  q.push(42);
  q.push(101);
  q.push(69);
  while(!q.empty())
  {
    cout << q.front() << endl;
    q.pop();
  }
  return 0;
}
