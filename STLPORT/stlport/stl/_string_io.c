#ifndef _STLP_STRING_IO_C
#define _STLP_STRING_IO_C

#ifndef _STLP_STRING_IO_H
# include <stl/_string_io.h>
#endif

#ifndef _STLP_INTERNAL_CTYPE_H
# include <stl/_ctype.h>
#endif

# ifdef _STLP_DEBUG
#  define basic_string _Nondebug_string
# endif

_STLP_BEGIN_NAMESPACE

# if defined (_STLP_OWN_IOSTREAMS)
#  define _STLP_USING_IO
# else
#  define _STLP_USING_IO _STLP_USING_VENDOR_STD
# endif

#if defined (_STLP_USE_NEW_IOSTREAMS)

template <class _CharT, class _Traits>
bool _STLP_CALL
__stlp_string_fill(basic_ostream<_CharT, _Traits>& __os,
                  basic_streambuf<_CharT, _Traits>* __buf,
                  size_t __n)
{
  _CharT __f = __os.fill();
  size_t __i;
  bool __ok = true;

  for (__i = 0; __i < __n; ++__i)
    __ok = __ok && !_Traits::eq_int_type(__buf->sputc(__f), _Traits::eof());
  return __ok;
}

template <class _CharT, class _Traits, class _Alloc>
basic_ostream<_CharT, _Traits>& _STLP_CALL
operator<<(basic_ostream<_CharT, _Traits>& __os, 
           const basic_string<_CharT,_Traits,_Alloc>& __s)
{

  _STLP_USING_IO
  typedef basic_ostream<_CharT, _Traits> __ostream;
  typename __ostream::sentry __sentry(__os);
  bool __ok = false;

  if (__sentry) {
    __ok = true;
    size_t __n = __s.size();
    size_t __pad_len = 0;
    const bool __left = (__os.flags() & __ostream::left) != 0;
    const size_t __w = __os.width(0);
    basic_streambuf<_CharT, _Traits>* __buf = __os.rdbuf();

    if (__n < __w) {
      __pad_len = __w - __n;
    }
    
    if (!__left)
      __ok = __stlp_string_fill(__os, __buf, __pad_len);    

    __ok = __ok && (__buf->sputn(__s.data(), streamsize(__n)) == streamsize(__n));

    if (__left)
      __ok = __ok && __stlp_string_fill(__os, __buf, __pad_len);
  }

  if (!__ok)
    __os.setstate(__ostream::failbit);

  return __os;
}
 
template <class _CharT, class _Traits, class _Alloc>
basic_istream<_CharT, _Traits>& _STLP_CALL 
operator>>(basic_istream<_CharT, _Traits>& __is,
           basic_string<_CharT,_Traits, _Alloc>& __s)
{
  _STLP_USING_IO
  typedef basic_istream<_CharT, _Traits> __istream;
  typename __istream::sentry __sentry(__is);

  if (__sentry) {
    basic_streambuf<_CharT, _Traits>* __buf = __is.rdbuf();
    typedef ctype<_CharT> _C_type;

#ifdef _STLP_OWN_IOSTREAMS
    //    const _C_type& _Ctype = use_facet<_C_type>(__loc);
    const _C_type& _Ctype = *(const _C_type*)__is._M_ctype_facet();
#else
# if defined (_STLP_MSVC) && (_STLP_MSVC <= 1200 ) || defined (__ICL)
    const locale& __loc = __is.getloc();
    const _C_type& _Ctype = use_facet(__loc , ( _C_type * ) 0, true);
# elif defined (__SUNPRO_CC)
    const locale& __loc = __is.getloc();
    const _C_type& _Ctype = use_facet(__loc , ( _C_type * ) 0);
# else
    const locale& __loc = __is.getloc();
    const _C_type& _Ctype = use_facet<_C_type>(__loc);
# endif
#endif
    __s.clear();
    size_t __n = __is.width(0);
    if (__n == 0)
      __n = __STATIC_CAST(size_t,-1);
    else
      __s.reserve(__n);
    

    while (__n-- > 0) {
      typename _Traits::int_type __c1 = __buf->sbumpc();
      if (_Traits::eq_int_type(__c1, _Traits::eof())) {
        __is.setstate(__istream::eofbit);
        break;
      }
      else {
        _CharT __c = _Traits::to_char_type(__c1);

        if (_Ctype.is(_C_type::space, __c)) {
          if (_Traits::eq_int_type(__buf->sputbackc(__c), _Traits::eof()))
            __is.setstate(__istream::failbit);
          break;
        }
        else
          __s.push_back(__c);
      }
    }
    
    // If we have read no characters, then set failbit.
    if (__s.size() == 0)
      __is.setstate(__istream::failbit);
  }
  else
    __is.setstate(__istream::failbit);

  return __is;
}

template <class _CharT, class _Traits, class _Alloc>    
basic_istream<_CharT, _Traits>& _STLP_CALL 
getline(basic_istream<_CharT, _Traits>& __is,
        basic_string<_CharT,_Traits,_Alloc>& __s,
        _CharT __delim)
{
  _STLP_USING_IO
  typedef basic_istream<_CharT, _Traits> __istream;
  size_t __nread = 0;
  typename basic_istream<_CharT, _Traits>::sentry __sentry(__is, true);
  if (__sentry) {
    basic_streambuf<_CharT, _Traits>* __buf = __is.rdbuf();
    __s.clear();

    while (__nread < __s.max_size()) {
      int __c1 = __buf->sbumpc();
      if (_Traits::eq_int_type(__c1, _Traits::eof())) {
        __is.setstate(__istream::eofbit);
        break;
      }
      else {
        ++__nread;
        _CharT __c = _Traits::to_char_type(__c1);
        if (!_Traits::eq(__c, __delim)) 
          __s.push_back(__c);
        else
          break;              // Character is extracted but not appended.
      }
    }
  }
  if (__nread == 0 || __nread >= __s.max_size())
    __is.setstate(__istream::failbit);

  return __is;
}

#elif ! defined ( _STLP_USE_NO_IOSTREAMS )

// (reg) For Watcom IO, _OSTREAM_DLL tells if ostream class is in .exe or in .dll

template <class _CharT, class _Traits, class _Alloc>
_OSTREAM_DLL&  _STLP_CALL operator<<(_OSTREAM_DLL& __os, 
                    const basic_string<_CharT,_Traits,_Alloc>& __s)
{
  _STLP_USING_IO
  streambuf* __buf = __os.rdbuf();
  if (__buf) {
    size_t __n = __s.size();
    size_t __pad_len = 0;
    const bool __left = (__os.flags() & ios::left) !=0;
    const size_t __w = __os.width();

    if (__n < __w) { 
      __pad_len = __w - __n; 
    } 
    
    if (!__left)
      __stlp_string_fill(__os, __buf, __pad_len);
  
    const size_t __nwritten = __buf->sputn(__s.data(), __n);

    if (__left)
      __stlp_string_fill(__os, __buf, __pad_len);

    if (__nwritten != __n)
      __os.clear(__os.rdstate() | ios::failbit);

    __os.width(0);
  }
  else
    __os.clear(__os.rdstate() | ios::badbit);

  return __os;
}

template <class _CharT, class _Traits, class _Alloc>
_ISTREAM_DLL& _STLP_CALL operator>>(_ISTREAM_DLL& __is, basic_string<_CharT,_Traits,_Alloc>& __s)
{
  _STLP_USING_IO
  if (!__is)
    return __is;

  streambuf* __buf = __is.rdbuf();
  if (__buf) {

    if (__is.flags() & ios::skipws) {
      //      _CharT __c;
      int __c;
      do {
        __c = __buf->sbumpc();
      }
      while (__c != EOF && isspace((unsigned char)__c));

      if (__c == EOF) {
        __is.clear(__is.rdstate() | ios::eofbit | ios::failbit);
      }
      else {
	if (__buf->sputbackc(__c) == EOF)
	  __is.clear(__is.rdstate() | ios::failbit);
      }
    }

    // If we arrive at end of file (or fail for some other reason) while
    // still discarding whitespace, then we don't try to read the string.
    if (__is) {
      __s.clear();

      size_t __n = __is.width();
      if (__n == 0)
        __n = __STATIC_CAST(size_t,-1);
      else
        __s.reserve(__n);

      while (__n-- > 0) {
        int __c1 = __buf->sbumpc();
        if (__c1 == EOF) {
          __is.clear(__is.rdstate() | ios::eofbit);
          break;
        }
        else {
          _CharT __c = _Traits::to_char_type(__c1);

          if (isspace((unsigned char) __c)) {
            if (__buf->sputbackc(__c) == EOF)
              __is.clear(__is.rdstate() | ios::failbit);
            break;
          }
          else
            __s.push_back(__c);
        }
      }
    
      // If we have read no characters, then set failbit.
      if (__s.size() == 0)
        __is.clear(__is.rdstate() | ios::failbit);
    }

    __is.width(0);
  }
  else                          // We have no streambuf.
    __is.clear(__is.rdstate() | ios::badbit);

  return __is;
}

template <class _CharT, class _Traits, class _Alloc>    
_ISTREAM_DLL& _STLP_CALL getline(_ISTREAM_DLL& __is,
                 basic_string<_CharT,_Traits,_Alloc>& __s,
                 _CharT __delim)
{
  _STLP_USING_IO
  streambuf* __buf = __is.rdbuf();
  if (__buf) {
    size_t __nread = 0;
    if (__is) {
      __s.clear();

      while (__nread < __s.max_size()) {
        int __c1 = __buf->sbumpc();
        if (__c1 == EOF) {
          __is.clear(__is.rdstate() | ios::eofbit);
          break;
        }
        else {
          ++__nread;
          _CharT __c = _Traits::to_char_type(__c1);
          if (!_Traits::eq(__c, __delim)) 
            __s.push_back(__c);
          else
            break;              // Character is extracted but not appended.
        }
      }
    }

    if (__nread == 0 || __nread >= __s.max_size())
      __is.clear(__is.rdstate() | ios::failbit);
  }
  else
    __is.clear(__is.rdstate() | ios::badbit);

  return __is;
}

# endif /* _STLP_NEW_IOSTREAMS */

_STLP_END_NAMESPACE

// # undef _STLP_USING_IO
# undef basic_string

#endif
