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
#ifndef _STLP_OSTREAM_C
#define _STLP_OSTREAM_C


#ifndef _STLP_INTERNAL_OSTREAM_H
# include <stl/_ostream.h>
#endif

#if defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION)

#if !defined (_STLP_INTERNAL_NUM_PUT_H)
# include <stl/_num_put.h>            // For basic_streambuf and iterators
#endif

_STLP_BEGIN_NAMESPACE

// Helper functions for istream<>::sentry constructor.
template <class _CharT, class _Traits>
bool
_M_init(basic_ostream<_CharT, _Traits>& __str) {
  if (__str.good()) {
    // boris : check if this is needed !
    if (!__str.rdbuf())
      __str.setstate(ios_base::badbit);
    if (__str.tie())
      __str.tie()->flush();
    return __str.good();
  } else
    return false;
}

//----------------------------------------------------------------------
// Definitions of non-inline member functions.

// Constructor, destructor

template <class _CharT, class _Traits>
basic_ostream<_CharT, _Traits>
  ::basic_ostream(basic_streambuf<_CharT, _Traits>* __buf)
    : basic_ios<_CharT, _Traits>() 
{
  this->init(__buf);
}

template <class _CharT, class _Traits>
basic_ostream<_CharT, _Traits>::~basic_ostream()
{}

// Output directly from a streambuf.
template <class _CharT, class _Traits>
basic_ostream<_CharT, _Traits>& 
basic_ostream<_CharT, _Traits>::operator<<(basic_streambuf<_CharT, _Traits>* __from)
{
  sentry __sentry(*this);
  if (__sentry) {
    if (__from) {
      bool __any_inserted = __from->gptr() != __from->egptr()
        ? this->_M_copy_buffered(__from, this->rdbuf())
        : this->_M_copy_unbuffered(__from, this->rdbuf());
      if (!__any_inserted)
        this->setstate(ios_base::failbit);
    }
    else
      this->setstate(ios_base::badbit);
  }

  return *this;
}

// Helper functions for the streambuf version of operator<<.  The
// exception-handling code is complicated because exceptions thrown
// while extracting characters are treated differently than exceptions
// thrown while inserting characters.

template <class _CharT, class _Traits>
bool basic_ostream<_CharT, _Traits>
  ::_M_copy_buffered(basic_streambuf<_CharT, _Traits>* __from,
                     basic_streambuf<_CharT, _Traits>* __to)
{
  bool __any_inserted = false;

  while (__from->egptr() != __from->gptr()) {
    const ptrdiff_t __avail = __from->egptr() - __from->gptr();

    streamsize __nwritten;
    _STLP_TRY {
      __nwritten = __to->sputn(__from->gptr(), __avail);
      __from->gbump((int)__nwritten);
    }
    _STLP_CATCH_ALL {
      this->_M_handle_exception(ios_base::badbit);
      return __any_inserted;
    }

    if (__nwritten == __avail) {
      _STLP_TRY {
        if (this->_S_eof(__from->sgetc()))
          return true;
        else
          __any_inserted = true;
      }
      _STLP_CATCH_ALL {
        this->_M_handle_exception(ios_base::failbit);
        return false;
      }
    }

    else if (__nwritten != 0)
      return true;

    else
      return __any_inserted;
  }

  // No characters are in the buffer, but we aren't at EOF.  Switch to
  // unbuffered mode.
  return __any_inserted || this->_M_copy_unbuffered(__from, __to);
}

template <class _CharT, class _Traits>
bool basic_ostream<_CharT, _Traits>
  ::_M_copy_unbuffered(basic_streambuf<_CharT, _Traits>* __from,
                       basic_streambuf<_CharT, _Traits>* __to)
{
  bool __any_inserted = false;

  while (true) {
    int_type __c;
    _STLP_TRY {
      __c = __from->sbumpc();
    }
    _STLP_CATCH_ALL {
      this->_M_handle_exception(ios_base::failbit);
      return __any_inserted;
    }

    if (this->_S_eof(__c))
      return __any_inserted;

    else {
      int_type __tmp;
      _STLP_TRY {
        __tmp = __to->sputc(__c);
      }
      _STLP_CATCH_ALL {
        this->_M_handle_exception(ios_base::badbit);
        return __any_inserted;
      }

      if (this->_S_eof(__tmp)) {
        _STLP_TRY {
          /* __tmp = */ __from->sputbackc(__c);
        }
        _STLP_CATCH_ALL {
          this->_M_handle_exception(ios_base::badbit);
          return __any_inserted;
        }
      }
      else
        __any_inserted = true;
    }
  }
}

// Helper function for numeric output.

template <class _CharT, class _Traits, class _Number>
basic_ostream<_CharT, _Traits>&  _STLP_CALL
_M_put_num(basic_ostream<_CharT, _Traits>& __os, _Number __x)
{
  typedef typename basic_ostream<_CharT, _Traits>::sentry _Sentry;
  _Sentry __sentry(__os);
  bool __failed = true;

  if (__sentry) {
    _STLP_TRY {
      typedef num_put<_CharT, ostreambuf_iterator<_CharT, _Traits> > _NumPut;      
      __failed = (use_facet<_NumPut>(__os.getloc())).put(
                                                         ostreambuf_iterator<_CharT, _Traits>(__os.rdbuf()), 
                                                         __os, __os.fill(),
                                                         __x).failed();
    }
    _STLP_CATCH_ALL {
      __os._M_handle_exception(ios_base::badbit);
    }
  }
  if (__failed)
    __os.setstate(ios_base::badbit); 
  return __os;
}

# if defined (_STLP_USE_TEMPLATE_EXPORT)  && defined (__BUILDING_STLPORT)
_STLP_EXPORT_TEMPLATE _STLP_DECLSPEC basic_ostream<char, char_traits<char> >& _STLP_CALL
_M_put_num(basic_ostream<char, char_traits<char> >&, unsigned long);
_STLP_EXPORT_TEMPLATE _STLP_DECLSPEC basic_ostream<char, char_traits<char> >&  _STLP_CALL
_M_put_num(basic_ostream<char, char_traits<char> >&, long);
#  if defined (_STLP_LONG_LONG)
_STLP_EXPORT_TEMPLATE _STLP_DECLSPEC basic_ostream<char, char_traits<char> >&  _STLP_CALL
_M_put_num(basic_ostream<char, char_traits<char> >&, unsigned _STLP_LONG_LONG);
_STLP_EXPORT_TEMPLATE _STLP_DECLSPEC basic_ostream<char, char_traits<char> >&  _STLP_CALL
_M_put_num(basic_ostream<char, char_traits<char> >&, _STLP_LONG_LONG );
#  endif
# endif

template <class _CharT, class _Traits>
void basic_ostream<_CharT, _Traits>::_M_put_char(_CharT __c)
{
  sentry __sentry(*this);
  if (__sentry) {
    bool __failed = true;
    _STLP_TRY {
      streamsize __npad = this->width() > 0 ? this->width() - 1 : 0;
      //      if (__npad <= 1)
      if (__npad == 0)
        __failed = this->_S_eof(this->rdbuf()->sputc(__c));
      else if ((this->flags() & ios_base::adjustfield) == ios_base::left) {
        __failed = this->_S_eof(this->rdbuf()->sputc(__c));
        __failed = __failed || 
                   this->rdbuf()->_M_sputnc(this->fill(), __npad) != __npad;
      }
      else {
        __failed = this->rdbuf()->_M_sputnc(this->fill(), __npad) != __npad;
        __failed = __failed || this->_S_eof(this->rdbuf()->sputc(__c));
      }

      this->width(0);
    }
    _STLP_CATCH_ALL {
      this->_M_handle_exception(ios_base::badbit);
    }

    if (__failed)
      this->setstate(ios_base::badbit);
  }
}

template <class _CharT, class _Traits>
void basic_ostream<_CharT, _Traits>::_M_put_nowiden(const _CharT* __s)
{
  sentry __sentry(*this);
  if (__sentry) {
    bool __failed = true;
    streamsize __n = _Traits::length(__s);
    streamsize __npad = this->width() > __n ? this->width() - __n : 0;

    _STLP_TRY {
      if (__npad == 0)
        __failed = this->rdbuf()->sputn(__s, __n) != __n;
      else if ((this->flags() & ios_base::adjustfield) == ios_base::left) {
        __failed = this->rdbuf()->sputn(__s, __n) != __n;
        __failed = __failed || 
                   this->rdbuf()->_M_sputnc(this->fill(), __npad) != __npad;
      }
      else {
        __failed = this->rdbuf()->_M_sputnc(this->fill(), __npad) != __npad;
        __failed = __failed || this->rdbuf()->sputn(__s, __n) != __n;
      }

      this->width(0);
    }
    _STLP_CATCH_ALL {
      this->_M_handle_exception(ios_base::badbit);
    }

    if (__failed)
      this->setstate(ios_base::failbit);
  }
}

template <class _CharT, class _Traits>
void basic_ostream<_CharT, _Traits>::_M_put_widen(const char* __s)
{
  sentry __sentry(*this);
  if (__sentry) {
    bool __failed = true;
    streamsize __n = char_traits<char>::length(__s);
    streamsize __npad = this->width() > __n ? this->width() - __n : 0;

    _STLP_TRY {
      if (__npad == 0)
        __failed = !this->_M_put_widen_aux(__s, __n);
      else if ((this->flags() & ios_base::adjustfield) == ios_base::left) {
        __failed = !this->_M_put_widen_aux(__s, __n);
        __failed = __failed || 
                   this->rdbuf()->_M_sputnc(this->fill(), __npad) != __npad;
      }
      else {
        __failed = this->rdbuf()->_M_sputnc(this->fill(), __npad) != __npad;
        __failed = __failed || !this->_M_put_widen_aux(__s, __n);
      }

      this->width(0);
    }
    _STLP_CATCH_ALL {
      this->_M_handle_exception(ios_base::badbit);
    }

    if (__failed)
      this->setstate(ios_base::failbit);
  }
}

template <class _CharT, class _Traits>
bool basic_ostream<_CharT, _Traits>::_M_put_widen_aux(const char* __s,
                                                      streamsize __n)
{
  basic_streambuf<_CharT, _Traits>* __buf = this->rdbuf();

  for ( ; __n > 0 ; --__n)
    if (this->_S_eof(__buf->sputc(this->widen(*__s++))))
      return false;
  return true;
}

// Unformatted output of a single character.
template <class _CharT, class _Traits>
basic_ostream<_CharT, _Traits>&
basic_ostream<_CharT, _Traits>::put(char_type __c)
{
  sentry __sentry(*this);
  bool __failed = true;

  if (__sentry) {
    _STLP_TRY {
      __failed = this->_S_eof(this->rdbuf()->sputc(__c));
    }
    _STLP_CATCH_ALL {
      this->_M_handle_exception(ios_base::badbit);
    }
  }

  if (__failed)
    this->setstate(ios_base::badbit);

  return *this;
}

// Unformatted output of a single character.
template <class _CharT, class _Traits>
basic_ostream<_CharT, _Traits>&
basic_ostream<_CharT, _Traits>::write(const char_type* __s, streamsize __n)
{
  sentry __sentry(*this);
  bool __failed = true;

  if (__sentry) {
    _STLP_TRY {
      __failed = this->rdbuf()->sputn(__s, __n) != __n;
    }
    _STLP_CATCH_ALL {
      this->_M_handle_exception(ios_base::badbit);
    }
  }

  if (__failed)
    this->setstate(ios_base::badbit);

  return *this;
}

_STLP_END_NAMESPACE

#endif /* defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION) */

#endif /* _STLP_OSTREAM_C */
