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
#ifndef _STLP_NUM_PUT_C
#define _STLP_NUM_PUT_C

#ifndef _STLP_INTERNAL_NUM_PUT_H
# include <stl/_num_put.h>
#endif

# if defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION)

#ifndef _STLP_LIMITS_H
# include <stl/_limits.h>
#endif

_STLP_BEGIN_NAMESPACE

// _M_do_put_float and its helper functions.  Strategy: write the output
// to a buffer of char, transform the buffer to _CharT, and then copy
// it to the output.

template <class _CharT, class _OutputIter,class _Float>
_OutputIter _STLP_CALL
_M_do_put_float(_OutputIter __s, ios_base& __f, _CharT __fill,_Float    __x);


//----------------------------------------------------------------------
// num_put facet

template <class _CharT, class _OutputIter>
_OutputIter  _STLP_CALL
__copy_float_and_fill(const _CharT* __first, const _CharT* __last,
                      _OutputIter __out,
                      ios_base::fmtflags __flags,
                      streamsize __width, _CharT __fill,
                      _CharT __xplus, _CharT __xminus) {
  if (__width <= __last - __first)
    return copy(__first, __last, __out);
  else {
    streamsize __pad = __width - (__last - __first);
    ios_base::fmtflags __dir = __flags & ios_base::adjustfield;

    if (__dir == ios_base::left) {
      __out = copy(__first, __last, __out);
      return fill_n(__out, __pad, __fill);
    }
    else if (__dir == ios_base::internal && __first != __last &&
             (*__first == __xplus || *__first == __xminus)) {
      *__out++ = *__first++;
      __out = fill_n(__out, __pad, __fill);
      return copy(__first, __last, __out);
    }
    else {
      __out = fill_n(__out, __pad, __fill);
      return copy(__first, __last, __out);
    }
  }
}

#ifndef _STLP_NO_WCHAR_T
// Helper routine for wchar_t
template <class _OutputIter>
_OutputIter  _STLP_CALL
__put_float(char* __ibuf, char* __iend, _OutputIter __out,
            ios_base& __f, wchar_t __fill,
            wchar_t __decimal_point,
            wchar_t __sep, const string& __grouping)
{
  const ctype<wchar_t>& __ct = *(ctype<wchar_t>*)__f._M_ctype_facet() ;

  wchar_t __wbuf[128];
  wchar_t* __eend = __convert_float_buffer(__ibuf, __iend, __wbuf,
                                           __ct, __decimal_point);
  if (!__grouping.empty()) {
    // In order to do separator-insertion only to the left of the
    // decimal point, we adjust the size of the first (right-most)
    // group.  We need to be careful if there is only one entry in
    // grouping:  in this case we need to duplicate the first entry.

    string __new_grouping = __grouping;
    wchar_t* __decimal_pos = find(__wbuf, __eend, __decimal_point);
    if (__grouping.size() == 1)
      __new_grouping.push_back(__grouping[0]);

	// dwa 1/24/00 - try as I might, there doesn't seem to be a way
    // to suppress the warning
    __new_grouping[0] += __STATIC_CAST(char, __eend - __decimal_pos);
    ptrdiff_t __len = __insert_grouping(__wbuf, __eend, __new_grouping,
					__sep,
					__ct.widen('+'), __ct.widen('-'),
					0);
    __eend = __wbuf + __len;
  }

  return __copy_float_and_fill(__wbuf, __eend, __out,
                               __f.flags(), __f.width(0), __fill,
                               __ct.widen('+'), __ct.widen('-')); 
}
# endif /* WCHAR_T */

// Helper routine for char
template <class _OutputIter>
_OutputIter  _STLP_CALL
__put_float(char* __ibuf, char* __iend, _OutputIter __out,
            ios_base& __f, char __fill,
            char __decimal_point,
            char __sep, const string& __grouping)
{
  __adjust_float_buffer(__ibuf, __iend, __decimal_point);
  if (!__grouping.empty()) {
    string __new_grouping = __grouping;
    const char * __decimal_pos = find(__ibuf, __iend, __decimal_point);
    if (__grouping.size() == 1)
      __new_grouping.push_back(__grouping[0]);
    __new_grouping[0] += __STATIC_CAST(char, (__iend - __decimal_pos));
    ptrdiff_t __len = __insert_grouping(__ibuf, __iend, __new_grouping,
					__sep, '+', '-', 0);
    __iend = __ibuf + __len;
  }

  return __copy_float_and_fill(__ibuf, __iend, __out,
                               __f.flags(), __f.width(0), __fill, '+', '-');
}

template <class _CharT, class _OutputIter, class _Float>
_OutputIter _STLP_CALL
_M_do_put_float(_OutputIter __s, ios_base& __f,
                _CharT __fill, _Float __x)
{
  string __buf;
  __buf.reserve(128);
  __write_float(__buf, __f.flags(), (int)__f.precision(), __x);

  const numpunct<_CharT>& __np = *(const numpunct<_CharT>*)__f._M_numpunct_facet();
  
  return __put_float(__CONST_CAST(char*, __buf.c_str()), 
                     __CONST_CAST(char*, __buf.c_str()) + __buf.size(),
                     __s, __f, __fill,
                     __np.decimal_point(),
		                 __np.thousands_sep(), __f._M_grouping());
}

// _M_do_put_integer and its helper functions.

template <class _CharT, class _OutputIter>
_OutputIter _STLP_CALL
__copy_integer_and_fill(const _CharT* __buf, ptrdiff_t __len,
                        _OutputIter __out,
                        ios_base::fmtflags __flg, streamsize __wid, _CharT __fill,
                        _CharT __xplus, _CharT __xminus)
{
  if (__len >= __wid)
    return copy(__buf, __buf + __len, __out);
  else {
    ptrdiff_t __pad = __wid - __len;
    ios_base::fmtflags __dir = __flg & ios_base::adjustfield;

    if (__dir == ios_base::left) {
      __out = copy(__buf, __buf + __len, __out);
      return fill_n(__out, __pad, __fill);
    }
    else if (__dir == ios_base::internal && __len != 0 &&
             (__buf[0] == __xplus || __buf[0] == __xminus)) {
      *__out++ = __buf[0];
      __out = fill_n(__out, __pad, __fill);
      return copy(__buf + 1, __buf + __len, __out);
    }
    else if (__dir == ios_base::internal && __len >= 2 &&
             (__flg & ios_base::showbase) &&
             (__flg & ios_base::basefield) == ios_base::hex) {
      *__out++ = __buf[0];
      *__out++ = __buf[1];
      __out = fill_n(__out, __pad, __fill);
      return copy(__buf + 2, __buf + __len, __out);
    }
    else {
      __out = fill_n(__out, __pad, __fill);
      return copy(__buf, __buf + __len, __out);
    }
  }
}

#ifndef _STLP_NO_WCHAR_T
// Helper function for wchar_t
template <class _OutputIter>
_OutputIter _STLP_CALL
__put_integer(char* __buf, char* __iend, _OutputIter __s,
              ios_base& __f,
              ios_base::fmtflags __flags, wchar_t __fill)
{
  locale __loc = __f.getloc();
  //  const ctype<wchar_t>& __ct = use_facet<ctype<wchar_t> >(__loc);
  const ctype<wchar_t>& __ct = *(const ctype<wchar_t>*)__f._M_ctype_facet();

  wchar_t __xplus  = __ct.widen('+');
  wchar_t __xminus = __ct.widen('-');

  wchar_t __wbuf[64];
  __ct.widen(__buf, __iend, __wbuf);
  ptrdiff_t __len = __iend - __buf;
  wchar_t* __eend = __wbuf + __len;

  //  const numpunct<wchar_t>& __np = use_facet<numpunct<wchar_t> >(__loc);
  //  const string& __grouping = __np.grouping();

  const numpunct<wchar_t>& __np = *(const numpunct<wchar_t>*)__f._M_numpunct_facet();
  const string& __grouping = __f._M_grouping();

  if (!__grouping.empty()) {
    int __basechars;
    if (__flags & ios_base::showbase)
      switch (__flags & ios_base::basefield) {
	case ios_base::hex: __basechars = 2; break;
	case ios_base::oct: __basechars = 1; break;
	default: __basechars = 0;
      }
    else
      __basechars = 0;

    __len = __insert_grouping(__wbuf, __eend, __grouping, __np.thousands_sep(),
			      __xplus, __xminus, __basechars);
  }

  return __copy_integer_and_fill((wchar_t*)__wbuf, __len, __s,
                                 __flags, __f.width(0), __fill, __xplus, __xminus);
}
#endif

// Helper function for char
template <class _OutputIter>
_OutputIter _STLP_CALL
__put_integer(char* __buf, char* __iend, _OutputIter __s,
              ios_base& __f, ios_base::fmtflags __flags, char __fill)
{
  ptrdiff_t __len = __iend - __buf;
  char __grpbuf[64];

  //  const numpunct<char>& __np = use_facet<numpunct<char> >(__f.getloc());
  //  const string& __grouping = __np.grouping();

  const numpunct<char>& __np = *(const numpunct<char>*)__f._M_numpunct_facet();
  const string& __grouping = __f._M_grouping();

  if (!__grouping.empty()) {
    int __basechars;
    if (__flags & ios_base::showbase)
      switch (__flags & ios_base::basefield) {
	case ios_base::hex: __basechars = 2; break;
	case ios_base::oct: __basechars = 1; break;
	default: __basechars = 0;
      }
    else
      __basechars = 0;
 
     // make sure there is room at the end of the buffer
     // we pass to __insert_grouping

    copy(__buf, __iend, (char *) __grpbuf);
    __buf = __grpbuf;
    __iend = __grpbuf + __len; 
    __len = __insert_grouping(__buf, __iend, __grouping, __np.thousands_sep(), 
                              '+', '-', __basechars);
  }
  
  return __copy_integer_and_fill(__buf, __len, __s, __flags, __f.width(0), __fill, '+', '-');
}

#ifdef _STLP_LONG_LONG
typedef _STLP_LONG_LONG __max_int_t;
typedef unsigned _STLP_LONG_LONG __umax_int_t;
#else
typedef long __max_int_t;
typedef unsigned long __umax_int_t;
#endif

extern const char __hex_char_table_lo[];
extern const char __hex_char_table_hi[];

template <class _Integer>
inline char* _STLP_CALL
__write_decimal_backward(char* __ptr, _Integer __x, ios_base::fmtflags __flags, const __true_type& /* is_signed */)
{
  const bool __negative = __x < 0 ;
  __max_int_t __temp = __x;
  __umax_int_t __utemp = __negative?-__temp:__temp;

  for (; __utemp != 0; __utemp /= 10)
    *--__ptr = (int)(__utemp % 10) + '0';	  
  // put sign if needed or requested
  if (__negative)
    *--__ptr = '-';
  else if (__flags & ios_base::showpos)
    *--__ptr = '+';
  return __ptr;
}

template <class _Integer>
inline char* _STLP_CALL
__write_decimal_backward(char* __ptr, _Integer __x, ios_base::fmtflags __flags, const __false_type& /* is_signed */)
{
  for (; __x != 0; __x /= 10)
    *--__ptr = (int)(__x % 10) + '0';
  // put sign if requested
  if (__flags & ios_base::showpos)
    *--__ptr = '+';
  return __ptr;
}

template <class _Integer>
char* _STLP_CALL
__write_integer_backward(char* __buf, ios_base::fmtflags __flags, _Integer __x)
{
  char* __ptr = __buf;
  __umax_int_t __temp;

  if (__x == 0) {
    *--__ptr = '0';
    if ((__flags & ios_base::showpos) && ( (__flags & (ios_base::hex | ios_base::oct)) == 0 ))
      *--__ptr = '+';
  }
  else {
    
    switch (__flags & ios_base::basefield) {
    case ios_base::oct:
      __temp = __x;
      // if the size of integer is less than 8, clear upper part
      if ( sizeof(__x) < 8  && sizeof(__umax_int_t) >= 8 )
        __temp &= 0xFFFFFFFF;

      for (; __temp != 0; __temp >>=3)
        *--__ptr = (((unsigned)__temp)& 0x7) + '0';
      
      // put leading '0' is showbase is set
      if (__flags & ios_base::showbase)
        *--__ptr = '0';
      break;
    case ios_base::hex: 
      {
        const char* __table_ptr = (__flags & ios_base::uppercase) ? 
          __hex_char_table_hi : __hex_char_table_lo;
      __temp = __x;
      // if the size of integer is less than 8, clear upper part
      if ( sizeof(__x) < 8  && sizeof(__umax_int_t) >= 8 )
        __temp &= 0xFFFFFFFF;

        for (; __temp != 0; __temp >>=4)
          *--__ptr = __table_ptr[((unsigned)__temp & 0xF)];
        
        if (__flags & ios_base::showbase) {
          *--__ptr = __table_ptr[16];
          *--__ptr = '0';
        }
      }
      break;
    default: 
      {
#if defined(__HP_aCC) && (__HP_aCC == 1)
        bool _IsSigned = !((_Integer)-1 > 0);
	if (_IsSigned)
	  __ptr = __write_decimal_backward(__ptr, __x, __flags, __true_type() );
        else
	  __ptr = __write_decimal_backward(__ptr, __x, __flags, __false_type() );
#else
	typedef typename __bool2type<numeric_limits<_Integer>::is_signed>::_Ret _IsSigned;
	__ptr = __write_decimal_backward(__ptr, __x, __flags, _IsSigned());
# endif
      }
      break;
    }  
  }
  // return pointer to beginning of the string
  return __ptr;
}

//
// num_put<>
//

# if ( _STLP_STATIC_TEMPLATE_DATA > 0 )

template <class _CharT, class _OutputIterator>
locale::id num_put<_CharT, _OutputIterator>::id;
# else /* ( _STLP_STATIC_TEMPLATE_DATA > 0 ) */

typedef num_put<char, const char*> num_put_char;
typedef num_put<char, char*> num_put_char_2;
typedef num_put<char, ostreambuf_iterator<char, char_traits<char> > > num_put_char_3;

__DECLARE_INSTANCE(locale::id, num_put_char::id, );
__DECLARE_INSTANCE(locale::id, num_put_char_2::id, );
__DECLARE_INSTANCE(locale::id, num_put_char_3::id, );

# ifndef _STLP_NO_WCHAR_T

typedef num_put<wchar_t, const wchar_t*> num_put_wchar_t;
typedef num_put<wchar_t, wchar_t*> num_put_wchar_t_2;
typedef num_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > > num_put_wchar_t_3;

__DECLARE_INSTANCE(locale::id, num_put_wchar_t::id, );
__DECLARE_INSTANCE(locale::id, num_put_wchar_t_2::id, );
__DECLARE_INSTANCE(locale::id, num_put_wchar_t_3::id, );

# endif

# endif /* ( _STLP_STATIC_TEMPLATE_DATA > 0 ) */

// issue 118

# ifndef _STLP_NO_BOOL

template <class _CharT, class _OutputIter>  
_OutputIter 
num_put<_CharT, _OutputIter>::do_put(_OutputIter __s, ios_base& __f, 
                                     char_type __fill,  bool __val) const {
  if (!(__f.flags() & ios_base::boolalpha))
    return this->do_put(__s, __f, __fill, __STATIC_CAST(long,__val));

  locale __loc = __f.getloc();
  //  typedef numpunct<_CharT> _Punct;
  //  const _Punct& __np = use_facet<_Punct>(__loc);

  const numpunct<_CharT>& __np = *(const numpunct<_CharT>*)__f._M_numpunct_facet();

  basic_string<_CharT> __str = __val ? __np.truename() : __np.falsename();

  // Reuse __copy_integer_and_fill.  Since internal padding makes no
  // sense for bool, though, make sure we use something else instead.
  // The last two argument to __copy_integer_and_fill are dummies.
  ios_base::fmtflags __flags = __f.flags();
  if ((__flags & ios_base::adjustfield) == ios_base::internal)
    __flags = (__flags & ~ios_base::adjustfield) | ios_base::right;

  return __copy_integer_and_fill(__str.c_str(), __str.size(), __s,
                                 __flags, __f.width(0), __fill,
                                 (_CharT) 0, (_CharT) 0);
}

# endif

template <class _CharT, class _OutputIter>
_OutputIter 
num_put<_CharT, _OutputIter>::do_put(_OutputIter __s, ios_base& __f, _CharT __fill,
                                     long __val) const {

  char __buf[64];               // Large enough for a base 8 64-bit integer,
                                // plus any necessary grouping.  
  ios_base::fmtflags __flags = __f.flags();
  char* __ibeg = __write_integer_backward((char*)__buf+64, __flags, __val);  
  return __put_integer(__ibeg, (char*)__buf+64, __s, __f, __flags, __fill);
}


template <class _CharT, class _OutputIter>  
_OutputIter 
num_put<_CharT, _OutputIter>::do_put(_OutputIter __s, ios_base& __f, _CharT __fill,
				     unsigned long __val) const {
  char __buf[64];               // Large enough for a base 8 64-bit integer,
                                // plus any necessary grouping.
  
  ios_base::fmtflags __flags = __f.flags();
  char* __ibeg = __write_integer_backward((char*)__buf+64, __flags, __val);
  return __put_integer(__ibeg, (char*)__buf+64, __s, __f, __flags, __fill);
}

template <class _CharT, class _OutputIter>  
_OutputIter 
num_put<_CharT, _OutputIter>::do_put(_OutputIter __s, ios_base& __f, _CharT __fill,
                                     double __val) const {
  return _M_do_put_float(__s, __f, __fill, __val);
}

#ifndef _STLP_NO_LONG_DOUBLE
template <class _CharT, class _OutputIter>  
_OutputIter 
num_put<_CharT, _OutputIter>::do_put(_OutputIter __s, ios_base& __f, _CharT __fill,
                                     long double __val) const {
  return _M_do_put_float(__s, __f, __fill, __val);
}
#endif

#ifdef _STLP_LONG_LONG
template <class _CharT, class _OutputIter>  
_OutputIter 
num_put<_CharT, _OutputIter>::do_put(_OutputIter __s, ios_base& __f, _CharT __fill,
                                     _STLP_LONG_LONG __val) const {
  char __buf[64];               // Large enough for a base 8 64-bit integer,
                                // plus any necessary grouping.
  
  ios_base::fmtflags __flags = __f.flags();
  char* __ibeg = __write_integer_backward((char*)__buf+64, __flags, __val);
  return __put_integer(__ibeg, (char*)__buf+64, __s, __f, __flags, __fill);
}

template <class _CharT, class _OutputIter>  
_OutputIter 
num_put<_CharT, _OutputIter>::do_put(_OutputIter __s, ios_base& __f, _CharT __fill,
                                     unsigned _STLP_LONG_LONG __val) const {
  char __buf[64];               // Large enough for a base 8 64-bit integer,
                                // plus any necessary grouping.
  
  ios_base::fmtflags __flags = __f.flags();
  char* __ibeg = __write_integer_backward((char*)__buf+64, __flags, __val);  
  return __put_integer(__ibeg, (char*)__buf+64, __s, __f, __flags, __fill);
}

#endif /* _STLP_LONG_LONG */


// lib.facet.num.put.virtuals "12 For conversion from void* the specifier is %p."
template <class _CharT, class _OutputIter>
_OutputIter
num_put<_CharT, _OutputIter>::do_put(_OutputIter __s, ios_base& __f, _CharT /*__fill*/,
				     const void* __val) const {
  const ctype<_CharT>& __c_type = *(const ctype<_CharT>*)__f._M_ctype_facet();
  ios_base::fmtflags __save_flags = __f.flags();

  __f.setf(ios_base::hex, ios_base::basefield);
  __f.setf(ios_base::showbase);
  __f.setf(ios_base::internal, ios_base::adjustfield);
  __f.width((sizeof(void*) * 2) + 2); // digits in pointer type plus '0x' prefix
# if defined(_STLP_LONG_LONG) && !defined(__MRC__) //*ty 11/24/2001 - MrCpp can not cast from void* to long long
  _OutputIter result = this->do_put(__s, __f, __c_type.widen('0'), __REINTERPRET_CAST(unsigned _STLP_LONG_LONG,__val));
# else
  _OutputIter result = this->do_put(__s, __f, __c_type.widen('0'), __REINTERPRET_CAST(unsigned long,__val));
# endif
  __f.flags(__save_flags);
  return result;
}

_STLP_END_NAMESPACE

# endif /* _STLP_EXPOSE_STREAM_IMPLEMENTATION */

#endif /* _STLP_NUM_PUT_C */

// Local Variables:
// mode:C++
// End:
