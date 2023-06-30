// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <vector>

#ifdef MAIN 
#define vec1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int vec1_test(int, char**)
{
  //   cout<<"Sizeof(vector) is "<<sizeof(vector<int>)<<endl;
  //  cout<<"Sizeof(fw) is "<<sizeof(forward_iterator_tag)<<endl;
  //  cout<<"Sizeof(ra) is "<<sizeof(random_access_iterator_tag)<<endl;
  cout<<"Results of vec1_test:"<<endl;
  vector<int> v1; // Empty vector of integers.
  cout << "empty = " << v1.empty() << endl;
  cout << "size = " << v1.size() << endl;
  cout << "max_size = " << v1.max_size() << endl;
  v1.push_back(42); // Add an integer to the vector.
  cout << "size = " << v1.size() << endl;
  cout << "v1[0] = " << v1[0] << endl;
  return 0;
}
