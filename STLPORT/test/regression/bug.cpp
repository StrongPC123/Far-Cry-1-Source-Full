# include <iostream>
# include <string>

# if 0
namespace N1 {
     template <class T>
     struct X {};
}

namespace N2 {
  using N1::X;
  //  using namespace N1;
  
  class AA 
  {
    
    X<int> xint;
  }
  ;
  typedef X<int> xint;
  
  
  template <class T>
     struct X1 : N1::X<T> {}; // fatal error C1001:Internal compiler error
}

namespace N3 {
  using N1::X;
  using N2::X;
  using N2::xint;
  
  N2::xint x;
  N2::X<int> y;
}


using N2::X;
N2::X<int> y;

N2::X1<int> y;

main()
{
  std::basic_ostream<char, std::char_traits<char> > os;
  
  return 0;
}

# endif

# include <iostream>

using namespace std;

# if 0
class mystream : public std::istream 
{
public:
    typedef std::istream _Base;
    
    mystream(const std::istream& s) :  _Base(s)
        {
        }

    mystream& operator >>(char& c) 
        {
            using std::istream;
            ((std::istream&)*this)>> ( c ) ;
//            istream::operator>>(c);
            
            return ( *this ) ;
        }
    
    
}
;

mystream& aaa ( mystream& m, char& c)
{
    m>>c;
    return m;
}
# endif


main()
{
    string sss = "Hello World";
    string ss2 = "aaa";
    
    if (ss2 >= 'c' + sss)
        cout<<(sss+ss2);
    
//    mystream mm(std::cin);
//    char cc;
//    aaa(mm, cc);
    
}
