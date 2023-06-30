// STLport regression testsuite component.
// To compile as a separate example, please #define MAIN.

#include <algorithm>
#include <iostream>
#include <cstring>
#include <iterator>
#include <functional>

#if !defined (STLPORT) || defined(__STL_USE_NAMESPACES)
using namespace std;
#endif

#ifdef MAIN 
#define search2_test main
#endif
static bool str_equal(const char* a_, const char* b_)
{
  return strcmp(a_, b_) == 0 ? 1 : 0;
}

int search2_test(int, char**)
{
  cout<<"Results of search2_test:"<<endl;

char* grades[] = { "A", "B", "C", "D", "F" };
char* letters[] = { "Q", "E", "D" };

  const unsigned gradeCount = sizeof(grades) / sizeof(grades[0]);
  const unsigned letterCount = sizeof(letters) / sizeof(letters[0]);
  ostream_iterator <char*> iter(cout, " ");
  cout << "grades: ";
  copy(grades, grades + gradeCount, iter);
  cout << "\nletters:";
  copy(letters, letters + letterCount, iter);
  cout << endl;

  char** location =
    search((char**)grades, (char**)grades + gradeCount,
            (char**)letters, (char**)letters + letterCount,
            str_equal);

  if(location == grades + gradeCount)
    cout << "letters not found in grades" << endl;
  else
    cout << "letters found in grades at offset: " << location - grades << endl;

  copy((char**)grades + 1, (char**)grades + 1 + letterCount, (char**)letters);

  cout << "grades: ";
  copy(grades, grades + gradeCount, iter);
  cout << "\nletters:";
  copy(letters, letters + letterCount, iter);
  cout << endl;

  location = search((char**)grades, (char**)grades + gradeCount,
                     (char**)letters, (char**)letters + letterCount,
                     str_equal);

  if(location == grades + gradeCount)
    cout << "letters not found in grades" << endl;
  else
    cout
      << "letters found in grades at offset: " << location - grades << endl;
  return 0;
}
