/*
 * Copyright (c) 1997-1999
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

#ifndef _STLP_STRING_IO_H
#define _STLP_STRING_IO_H

// this is for link-time instantiation
#if !defined  ( _STLP_STRING )
# include <string>
# endif

//#ifndef _STLP_LOCALE
//# include <locale>
//#endif

# ifdef _STLP_DEBUG
#  define basic_string _Nondebug_string
# endif

// I/O.  
_STLP_BEGIN_NAMESPACE

#if defined (_STLP_USE_NEW_IOSTREAMS)

template <class _CharT, class _Traits, class _Alloc>
basic_ostream<_CharT, _Traits>& _STLP_CALL
operator<<(basic_ostream<_CharT, _Traits>& __os, 
           const basic_string<_CharT,_Traits,_Alloc>& __s);

template <class _CharT, class _Traits, class _Alloc>
basic_istream<_CharT, _Traits>&  _STLP_CALL
operator>>(basic_istream<_CharT, _Traits>& __is,
           basic_string<_CharT,_Traits,_Alloc>& __s);

template <class _CharT, class _Traits, class _Alloc>    
basic_istream<_CharT, _Traits>& _STLP_CALL 
getline(basic_istream<_CharT, _Traits>& __is,
        basic_string<_CharT,_Traits,_Alloc>& __s,
        _CharT __delim);

# if !(defined (__BORLANDC__) && ! defined (_STLP_USE_OWN_NAMESPACE))

template <class _CharT, class _Traits, class _Alloc>    
inline basic_istream<_CharT, _Traits>& _STLP_CALL 
getline(basic_istream<_CharT, _Traits>& __is,
        basic_string<_CharT,_Traits,_Alloc>& __s)
{
  return getline(__is, __s, __is.widen('\n'));
}
# endif

template <class _CharT, class _Traits>
bool _STLP_CALL
__stlp_string_fill(basic_ostream<_CharT, _Traits>& __os,
                  basic_streambuf<_CharT, _Traits>* __buf,
                  size_t __n);
#elif ! defined ( _STLP_USE_NO_IOSTREAMS )

// (reg) For Watcom IO, this tells if ostream class is in .exe or in .dll
# ifdef _WPRTLINK
typedef _WPRTLINK ostream _OSTREAM_DLL;
typedef _WPRTLINK istream _ISTREAM_DLL;
#else
typedef           ostream _OSTREAM_DLL;
typedef           istream _ISTREAM_DLL;
#endif

template <class _CharT, class _Traits, class _Alloc>
_OSTREAM_DLL& _STLP_CALL operator<<(_OSTREAM_DLL& __os, 
                    const basic_string<_CharT,_Traits,_Alloc>& __s);

template <class _CharT, class _Traits, class _Alloc>
_ISTREAM_DLL& _STLP_CALL operator>>(_ISTREAM_DLL& __is, basic_string<_CharT,_Traits,_Alloc>& __s);

template <class _CharT, class _Traits, class _Alloc>    
_ISTREAM_DLL& _STLP_CALL getline(_ISTREAM_DLL& __is,
                 basic_string<_CharT,_Traits,_Alloc>& __s,
                 _CharT __delim);


template <class _CharT, class _Traits, class _Alloc>    
inline _ISTREAM_DLL& _STLP_CALL 
getline(_ISTREAM_DLL& __is, basic_string<_CharT,_Traits,_Alloc>& __s)
{
  return getline(__is, __s, '\n');
}

inline void  _STLP_CALL
__stlp_string_fill(_OSTREAM_DLL& __os, streambuf* __buf, size_t __n)
{
  char __f = __os.fill();
  size_t __i;

  for (__i = 0; __i < __n; ++__i) __buf->sputc(__f);
}

#endif /* _STLP_USE_NEW_IOSTREAMS */

_STLP_END_NAMESPACE

# undef basic_string

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_string_io.c>
# endif

#endif /* _STLP_STRING_IO_H */
