// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <string>
#include <iostream>

#ifdef MAIN 
#define string1_test main
#endif

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
int string1_test(int, char**)
{
  cout<<"Results of string1_test:"<<endl;
  char* array = "Hello, World!";
  std::string v(array);
  int i;
  cout << v << endl;
  v.erase(v.begin() + 1, v.end() - 1); // Erase all but first and last.
  for(i = 0; i < v.size(); i++)
    cout << "v[" << i << "] = " << v[i] << endl;
  cout << endl;
  v.insert(1, (char*)array);
  v.erase(v.begin()); // Erase first element.
  v.erase(v.end() - 1); // Erase last element.
  cout << v << endl;
  v.clear(); // Erase all.
  return 0;
}
