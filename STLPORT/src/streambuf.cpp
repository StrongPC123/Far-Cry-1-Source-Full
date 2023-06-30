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


#include <stl/_streambuf.h>
#include <stl/_algobase.h>

// Implementation of non-inline member functions of class
// basic_streambuf<char, char_traits<char> >

# if defined (__hpux)
#  define FILE_CAST(x) (__REINTERPRET_CAST(FILE*, x))
# else
#  define FILE_CAST(x) x
# endif

_STLP_BEGIN_NAMESPACE

#if !defined(_STLP_WINCE)

basic_streambuf<char, char_traits<char> >::~basic_streambuf() {}

// This constructor is an extension.  It is for streambuf subclasses that
// are synchronized with C stdio files.
basic_streambuf<char, char_traits<char> >
  ::basic_streambuf(FILE* __get, FILE* __put)
    : _M_get(__get ? __get : FILE_CAST(&_M_default_get)),
      _M_put(__put ? __put : FILE_CAST(&_M_default_put)),
      _M_locale()
{
  _M_lock._M_initialize();

  if (_M_get == FILE_CAST(&_M_default_get))
    _FILE_I_set(_M_get, 0, 0, 0);
  if (_M_put == FILE_CAST(&_M_default_put))
    _FILE_O_set(_M_put, 0, 0, 0);      
}

// virtual functions

void basic_streambuf<char, char_traits<char> >::imbue(const locale&)
{}

basic_streambuf<char, char_traits<char> >*
basic_streambuf<char, char_traits<char> >::setbuf(char*, streamsize)
{
  return this;
}

basic_streambuf<char, char_traits<char> >::pos_type
basic_streambuf<char, char_traits<char> >
  ::seekoff(off_type, ios_base::seekdir, ios_base::openmode)
{
  return pos_type(-1);
}

basic_streambuf<char, char_traits<char> >::pos_type
basic_streambuf<char, char_traits<char> >
  ::seekpos(pos_type, ios_base::openmode)
{
  return pos_type(-1);
}

int basic_streambuf<char, char_traits<char> >::sync()
{
  return 0;
}

streamsize basic_streambuf<char, char_traits<char> >::showmanyc()
{
  return 0;
}

streamsize basic_streambuf<char, char_traits<char> >
  ::xsgetn(char* s, streamsize n)
{
  streamsize result = 0;
  const int_type eof = traits_type::eof();

  while (result < n) {
    if (_FILE_I_avail(_M_get) > 0) {
      size_t chunk = (min) (__STATIC_CAST(size_t,_FILE_I_avail(_M_get)),
                         __STATIC_CAST(size_t,n - result));
      traits_type::copy(s, _FILE_I_next(_M_get), chunk);
      result += chunk;
      s += chunk;
      _FILE_I_bump(_M_get, chunk);
    }
    else {
      int_type c = sbumpc();
      if (c != eof) {
        *s = c;
        ++result;
	++s;
      }
      else
        break; 
    }
  }
  
  return result;
}

basic_streambuf<char, char_traits<char> >::int_type
basic_streambuf<char, char_traits<char> >::underflow()
{
  return traits_type::eof();
}

basic_streambuf<char, char_traits<char> >::int_type
basic_streambuf<char, char_traits<char> >::uflow()
{
  const int_type eof = traits_type::eof();
  return this->underflow() == eof 
    ? eof
    : traits_type::to_int_type(_FILE_I_postincr(_M_get));
}

basic_streambuf<char, char_traits<char> >::int_type
basic_streambuf<char, char_traits<char> >::pbackfail(int_type /* __c */)
{
  return traits_type::eof();
}


streamsize basic_streambuf<char, char_traits<char> >
  ::xsputn(const char* s, streamsize n)
{
  streamsize result = 0;
  const int_type eof = traits_type::eof();

  while (result < n) {
    if (_FILE_O_avail(_M_put) > 0) {
      size_t chunk = (min) (__STATIC_CAST(size_t,_FILE_O_avail(_M_put)),
                         __STATIC_CAST(size_t,n - result));
      traits_type::copy(_FILE_O_next(_M_put), s, chunk);
      result += chunk;
      s += chunk;
      _FILE_O_bump(_M_put, (int)chunk);
    }

    else if (this->overflow(traits_type::to_int_type(*s)) != eof) {
      ++result;
      ++s;
    }
    else
      break;
  }
  return result;
}

streamsize basic_streambuf<char, char_traits<char> >
  ::_M_xsputnc(char c, streamsize n)
{
  streamsize result = 0;
  const int_type eof = traits_type::eof();

  while (result < n) {
    if (_FILE_O_avail(_M_put) > 0) {
      size_t chunk = (min) (__STATIC_CAST(size_t,_FILE_O_avail(_M_put)),
                         __STATIC_CAST(size_t,n - result));
      traits_type::assign(_FILE_O_next(_M_put), chunk, c);
      result += chunk;
      _FILE_O_bump(_M_put, (int)chunk);
    }

    else if (this->overflow(traits_type::to_int_type(c)) != eof)
      ++result;
    else
      break;
  }
  return result;
}

basic_streambuf<char, char_traits<char> >::int_type
basic_streambuf<char, char_traits<char> >::overflow(int_type/*  c */)
{
  return traits_type::eof();
}

basic_streambuf<char, char_traits<char> >::int_type
basic_streambuf<char, char_traits<char> >::_M_snextc_aux()
{
  int_type eof = traits_type::eof();
  if (_FILE_I_avail(_M_get) == 0)
    return this->uflow() == eof ? eof : this->sgetc();
  else {
    _FILE_I_set(_M_get,
              _FILE_I_begin(_M_get), _FILE_I_end(_M_get), _FILE_I_end(_M_get));
    return this->underflow();
  }
}


locale basic_streambuf<char, char_traits<char> >::pubimbue(const locale& loc)
{
  this->imbue(loc);
  locale tmp = _M_locale;
  _M_locale = loc;
  return tmp;
}

#else

#if !defined(_STLP_NO_FORCE_INSTANTIATE)
template class basic_streambuf<char, char_traits<char> >;
#endif

#endif /* _STLP_WINCE */

//----------------------------------------------------------------------
// Force instantiation of basic_streambuf

// not basic_streambuf<char>, because it's specialized.

#if !defined(_STLP_NO_FORCE_INSTANTIATE)
#if !defined (_STLP_NO_WCHAR_T)
template class basic_streambuf<wchar_t, char_traits<wchar_t> >;
#endif /* INSTANTIATE_WIDE_STREAMS */
#endif

_STLP_END_NAMESPACE

// Local Variables:
// mode:C++
// End:
