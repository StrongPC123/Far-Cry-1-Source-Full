/*
 * Copyright (c) 1999
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1999 
 * Boris Fomitchev
 *
 * This material is provided "as is", with absolutely no warranty expressed
 * or implied. Any use is at your own risk.
 *
 * Permission to use or copy this software for any purpose is hereby granted 
 * without fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 *
 */ 
# include "stlport_prefix.h"
#include <stl/_num_get.h>
#include <stl/_istream.h>
#include <stl/_algo.h>

_STLP_BEGIN_NAMESPACE

//----------------------------------------------------------------------
// num_get


static char narrow_digits[11]  = "0123456789";
static char narrow_xdigits[13] = "aAbBcCdDeEfF";

# ifndef _STLP_NO_WCHAR_T 

void  _STLP_CALL
_Initialize_get_digit(wchar_t* digits, wchar_t* xdigits,
                       const ctype<wchar_t>& ct)
{
  ct.widen(narrow_digits + 0,  narrow_digits + 10,  digits);
  ct.widen(narrow_xdigits + 0, narrow_xdigits + 10, xdigits);
}

// Return either the digit corresponding to c, or a negative number if
// if c isn't a digit.  We return -1 if c is the separator character, and
// -2 if it's some other non-digit.
int _STLP_CALL __get_digit(wchar_t c,
                           const wchar_t* digits, const wchar_t* xdigits,
                           wchar_t separator)
{
  // Test if it's the separator.
  if (c == separator)
    return -1;

  const wchar_t* p;

  // Test if it's a decimal digit.
  p = find(digits, digits + 10, c);
  if (p != digits + 10)
    return (int)(p - digits);

  // Test if it's a hex digit.
  p = find(xdigits, xdigits + 12, c);
  if (p != xdigits + 12)
    return (int)(10 + (xdigits - p) / 2);
  else
    return -2;                  // It's not a digit and not the separator.
}

# endif /* _STLP_NO_WCHAR_T */

// __valid_grouping compares two strings, one representing the
// group sizes encountered when reading an integer, and the other
// representing the valid group sizes as returned by the numpunct
// grouping() member function.  Both are interpreted right-to-left.
// The grouping string is treated as if it were extended indefinitely
// with its last value.  For a grouping to be valid, each term in
// the first string must be equal to the corresponding term in the
// second, except for the last, which must be less than or equal.

// boris : this takes reversed first string !
bool  _STLP_CALL
__valid_grouping(const char * first1, const char * last1, 
                 const char * first2, const char * last2)
{
  if (first1 == last1 || first2 == last2) return true;

  --last1; --last2;

  while (first1 != last1) {
    if (*last1 != *first2)
      return false;
    --last1;
    if (first2 != last2) ++first2;
  }

  return *last1 <= *first2;
}

// this needed for some compilers to make sure sumbols are extern
extern const unsigned char __digit_val_table[];
extern const char __narrow_atoms[];


const unsigned char __digit_val_table[128] = 
{
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
   0, 1, 2, 3, 4, 5, 6, 7, 8, 9,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,10,11,12,13,14,15,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,10,11,12,13,14,15,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};     

const char __narrow_atoms[5] = {'+', '-', '0', 'x', 'X'};

// index is actually a char

# ifndef _STLP_NO_WCHAR_T

// Similar, except return the character itself instead of the numeric
// value.  Used for floating-point input.
bool  _STLP_CALL __get_fdigit(wchar_t& c, const wchar_t* digits)
{
  const wchar_t* p = find(digits, digits + 10, c);
  if (p != digits + 10) {
    c = (char)('0' + (p - digits));
    return true;
  }
  else
    return false;
}

bool  _STLP_CALL __get_fdigit_or_sep(wchar_t& c, wchar_t sep,
                                     const wchar_t * digits)
{
  if (c == sep) {
    c = (char)',';
    return true;
  }
  else
    return __get_fdigit(c, digits);
}

//----------------------------------------------------------------------
// Force instantiation of of num_get<>

#if !defined(_STLP_NO_FORCE_INSTANTIATE)
template class _STLP_CLASS_DECLSPEC  istreambuf_iterator<wchar_t, char_traits<wchar_t> >;
template class num_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > >;
// template class num_get<wchar_t, const wchar_t*>;
#endif

# endif /* _STLP_NO_WCHAR_T */

//----------------------------------------------------------------------
// Force instantiation of of num_get<>

#if !defined(_STLP_NO_FORCE_INSTANTIATE)
template class _STLP_CLASS_DECLSPEC istreambuf_iterator<char, char_traits<char> >;
// template class num_get<char, const char*>;
template class num_get<char, istreambuf_iterator<char, char_traits<char> > >;
#endif
 
// basic_streambuf<char, char_traits<char> >* _STLP_CALL _M_get_istreambuf(basic_istream<char, char_traits<char> >& ) ;

_STLP_END_NAMESPACE

// Local Variables:
// mode:C++
// End:


