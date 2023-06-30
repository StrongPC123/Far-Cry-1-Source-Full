// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <iostream>
#include <hash_set>

#ifdef MAIN 
#define hmset1_test main
#endif

// struct hash<string> {
//      size_t operator()(const string& s) const { return __stl_hash_string(s.c_str()); }
//};

#if defined (__MVS__)
  #define star   92 
#else
  #define star   42

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif
#endif

typedef hash_multiset<char, hash<char>, equal_to<char> > hmset;
// __STL_TYPE_TRAITS_POD_SPECIALIZE(_Hashtable_node<char>*);


int hmset1_test(int, char**)
{
  cout<<"Results of hmset1_test:"<<endl;
  hmset s;
  cout << "count(" << star << ") = " << s.count(star) << endl;
  s.insert(star);
  cout << "count(" << star << ") = " << s.count(star) << endl;
  s.insert(star);
  cout << "count(" << star << ") = " << s.count(star) << endl;
  hmset::iterator i = s.find(40);
  if(i == s.end())
    cout << "40 Not found" << endl;
  else
    cout << "Found " << *i << endl;
  i = s.find(star);
  if(i == s.end())
    cout << "Not found" << endl;
  else
    cout << "Found " << *i << endl;
  int count = s.erase(star);
  cout << "Erased " << count << " instances" << endl;
  return 0;
}
