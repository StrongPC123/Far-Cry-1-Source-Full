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
// #include <iterator>
#include <complex>
#include <istream>

_STLP_BEGIN_NAMESPACE

// Force instantiation of complex I/O functions

#if !(defined (_STLP_NO_FORCE_INSTANTIATE) || defined (_STLP_NO_WCHAR_T))

_STLP_OPERATOR_SPEC basic_istream<wchar_t, char_traits<wchar_t> >&  _STLP_CALL
operator>>(basic_istream<wchar_t, char_traits<wchar_t> >&, complex<float>&);

_STLP_OPERATOR_SPEC basic_istream<wchar_t, char_traits<wchar_t> >&  _STLP_CALL
operator>>(basic_istream<wchar_t, char_traits<wchar_t> >&, complex<double>&);

#ifndef _STLP_NO_LONG_DOUBLE
_STLP_OPERATOR_SPEC basic_istream<wchar_t, char_traits<wchar_t> >&  _STLP_CALL
operator>>(basic_istream<wchar_t, char_traits<wchar_t> >&, complex<long double>&);

_STLP_OPERATOR_SPEC basic_ostream<wchar_t, char_traits<wchar_t> >&  _STLP_CALL
operator<<(basic_ostream<wchar_t, char_traits<wchar_t> >&, const complex<long double>&);
#endif

_STLP_OPERATOR_SPEC basic_ostream<wchar_t, char_traits<wchar_t> >&  _STLP_CALL
operator<<(basic_ostream<wchar_t, char_traits<wchar_t> >&, const complex<float>&);

_STLP_OPERATOR_SPEC basic_ostream<wchar_t, char_traits<wchar_t> >&  _STLP_CALL
operator<<(basic_ostream<wchar_t, char_traits<wchar_t> >&, const complex<double>&);

# endif /* _STLP_NO_WCHAR_T */

_STLP_END_NAMESPACE

// Local Variables:
// mode:C++
// End:

