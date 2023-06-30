#include <string>
#include <deque>
#include <iterator>
#include <iostream>
typedef std::string Str;
typedef std::deque<Str> Dq;
int insert1_test()
{
  Str array1 [] = { "laurie", "jennifer", "leisa" };
  Dq nam(array1, array1 + 3);
  Dq::iterator i = nam.begin() + 2;
  return 0;
}
