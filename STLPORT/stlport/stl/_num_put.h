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
// WARNING: This is an internal header file, included by other C++
// standard library headers.  You should not attempt to use this header
// file directly.


#ifndef _STLP_INTERNAL_NUM_PUT_H
#define _STLP_INTERNAL_NUM_PUT_H

#ifndef _STLP_INTERNAL_NUMPUNCT_H
# include <stl/_numpunct.h>
#endif
#ifndef _STLP_INTERNAL_CTYPE_H
# include <stl/_ctype.h>
#endif
#ifndef _STLP_INTERNAL_OSTREAMBUF_ITERATOR_H
# include <stl/_ostreambuf_iterator.h>
#endif

_STLP_BEGIN_NAMESPACE

//----------------------------------------------------------------------
// num_put facet

# ifdef _STLP_LIMITED_DEFAULT_TEMPLATES
template <class _CharT, class _OutputIter>  
# else
template <class _CharT, class _OutputIter = ostreambuf_iterator<_CharT, char_traits<_CharT> > >  
# endif
class num_put: public locale::facet
{
  friend class _Locale;
public:
  typedef _CharT      char_type;
  typedef _OutputIter iter_type;

  explicit num_put(size_t __refs = 0) : _BaseFacet(__refs) {}

# ifndef _STLP_NO_BOOL
  iter_type put(iter_type __s, ios_base& __f, char_type __fill,
                bool __val) const {
    return do_put(__s, __f, __fill, __val);
  }
# endif
  iter_type put(iter_type __s, ios_base& __f, char_type __fill,
               long __val) const {
    return do_put(__s, __f, __fill, __val);
  }

  iter_type put(iter_type __s, ios_base& __f, char_type __fill,
                unsigned long __val) const {
    return do_put(__s, __f, __fill, __val);
  }

#ifdef _STLP_LONG_LONG
  iter_type put(iter_type __s, ios_base& __f, char_type __fill,
                _STLP_LONG_LONG __val) const {
    return do_put(__s, __f, __fill, __val);
  }

  iter_type put(iter_type __s, ios_base& __f, char_type __fill,
                unsigned _STLP_LONG_LONG __val) const {
    return do_put(__s, __f, __fill, __val);
  }
#endif

  iter_type put(iter_type __s, ios_base& __f, char_type __fill,
                double __val) const {
    return do_put(__s, __f, __fill, (double)__val);
  }

#ifndef _STLP_NO_LONG_DOUBLE
  iter_type put(iter_type __s, ios_base& __f, char_type __fill,
                long double __val) const {
    return do_put(__s, __f, __fill, __val);
  }
# endif

  iter_type put(iter_type __s, ios_base& __f, char_type __fill,
                const void * __val) const {
    return do_put(__s, __f, __fill, __val);
  }

  _STLP_STATIC_MEMBER_DECLSPEC static locale::id id;

protected:
  ~num_put() {}   
# ifndef _STLP_NO_BOOL
  virtual _OutputIter do_put(_OutputIter __s, ios_base& __f, _CharT __fill, bool __val) const;
# endif
  virtual _OutputIter do_put(_OutputIter __s, ios_base& __f, _CharT __fill, long __val) const;
  virtual _OutputIter do_put(_OutputIter __s, ios_base& __f, _CharT __fill, unsigned long __val) const;
  virtual _OutputIter do_put(_OutputIter __s, ios_base& __f, _CharT __fill, double __val) const;
#ifndef _STLP_NO_LONG_DOUBLE
  virtual _OutputIter do_put(_OutputIter __s, ios_base& __f, _CharT __fill, long double __val) const;
#endif

#ifdef _STLP_LONG_LONG
  virtual _OutputIter do_put(_OutputIter __s, ios_base& __f, _CharT __fill, _STLP_LONG_LONG __val) const;
  virtual _OutputIter do_put(_OutputIter __s, ios_base& __f, _CharT __fill, 
                           unsigned _STLP_LONG_LONG __val) const ;
#endif /* _STLP_LONG_LONG  */
  virtual _OutputIter do_put(_OutputIter __s, ios_base& __f, _CharT __fill, const void* __val) const;
};

# ifdef _STLP_USE_TEMPLATE_EXPORT
_STLP_EXPORT_TEMPLATE_CLASS num_put<char, ostreambuf_iterator<char, char_traits<char> > >;
// _STLP_EXPORT_TEMPLATE_CLASS num_put<char, char*>;
#  ifndef _STLP_NO_WCHAR_T
_STLP_EXPORT_TEMPLATE_CLASS num_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > >;
// _STLP_EXPORT_TEMPLATE_CLASS num_put<wchar_t, wchar_t*>;
#  endif /* _STLP_NO_WCHAR_T */
# endif

# if defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION)

template <class _Integer>
char* _STLP_CALL
__write_integer_backward(char* __buf, ios_base::fmtflags __flags, _Integer __x);

void  _STLP_CALL __string_to_float(const string&, float&);
void  _STLP_CALL __string_to_float(const string&, double&);
extern void _STLP_CALL __write_float(string&, ios_base::fmtflags, int, double);
# ifndef _STLP_NO_LONG_DOUBLE
void  _STLP_CALL __string_to_float(const string&, long double&);
extern void _STLP_CALL __write_float(string&, ios_base::fmtflags, int, long double);
# endif

#ifndef _STLP_NO_WCHAR_T
extern wchar_t* _STLP_CALL __convert_float_buffer(const char*, const char*, wchar_t*, const ctype<wchar_t>&, wchar_t);
#endif
extern void _STLP_CALL __adjust_float_buffer(char*, char*, char);

extern char* _STLP_CALL
__write_integer(char* buf, ios_base::fmtflags flags, long x);

extern ptrdiff_t _STLP_CALL __insert_grouping(char* first, char* last, const string&, char, char, char, int);
#  ifndef _STLP_NO_WCHAR_T
extern ptrdiff_t _STLP_CALL __insert_grouping(wchar_t*, wchar_t*, const string&, wchar_t, wchar_t, wchar_t, int);
#  endif

# endif

# if defined (__BORLANDC__) && defined (_RTLDLL)
inline void _Stl_loc_init_num_put() {
  
  num_put<char, ostreambuf_iterator<char, char_traits<char> > >::id._M_index = 14;
  num_put<char, char*>::id._M_index = 15;
  
# ifndef _STLP_NO_WCHAR_T
  num_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > > ::id._M_index = 33;
  num_put<wchar_t, wchar_t*>::id._M_index = 34;
# endif
  
}
 
# endif

_STLP_END_NAMESPACE

#  if defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION) && ! defined (_STLP_LINK_TIME_INSTANTIATION)
#   include <stl/_num_put.c>
#  endif

#endif /* _STLP_INTERNAL_NUMERIC_FACETS_H */

// Local Variables:
// mode:C++
// End:

