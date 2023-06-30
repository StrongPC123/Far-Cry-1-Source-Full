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
#ifndef _STLP_NUM_GET_C
#define _STLP_NUM_GET_C

#ifndef _STLP_INTERNAL_NUM_GET_H
# include <stl/_num_get.h>
#endif

# if defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION)

#ifndef _STLP_LIMITS_H
# include <stl/_limits.h>
#endif

_STLP_BEGIN_NAMESPACE

extern const unsigned char __digit_val_table[];

template < class _InputIter, class _Integer, class _CharT>
_InputIter _STLP_CALL
_M_do_get_integer(_InputIter&, _InputIter&, ios_base&, ios_base::iostate&, _Integer&, _CharT*);

// _M_do_get_integer and its helper functions.  

inline bool _STLP_CALL __get_fdigit(char& __c, const char*)
  { return __c >= '0' && __c <= '9'; }

inline bool _STLP_CALL __get_fdigit_or_sep(char& __c, char __sep, const char *)
{ 
  if (__c == __sep) { 
    __c = ',' ; 
    return true ;
  } else
    return  ( __c >= '0' && __c <= '9');
}

inline int _STLP_CALL
__get_digit_from_table(unsigned __index)
{
  return (__index > 127 ? 0xFF : __digit_val_table[__index]);
}

extern const char __narrow_atoms[];

template <class _InputIter, class _CharT>
int 
_M_get_base_or_zero(_InputIter& __in, _InputIter& __end, ios_base& __str, _CharT*)
{
  _CharT __atoms[5];
  const ctype<_CharT>& __c_type = use_facet< ctype<_CharT> >(__str.getloc());
  // const ctype<_CharT>& __c_type = *(const ctype<_CharT>*)__str._M_ctype_facet();

  __c_type.widen(__narrow_atoms, __narrow_atoms + 5, __atoms);

  bool __negative = false;
  _CharT __c = *__in;

  if (__c == __atoms[1] /* __xminus_char */ ) {
    __negative = true;
    ++__in;
  }
  else if (__c == __atoms[0] /* __xplus_char */ ) 
    ++__in;


  int __base;
  int __valid_zero = 0;
  
  ios_base::fmtflags __basefield = __str.flags() & ios_base::basefield; 

  switch (__basefield) {
  case ios_base::oct:
    __base = 8;
    break;
  case ios_base::dec:
    __base = 10;
    break;
  case ios_base::hex:
    __base = 16;
    if (__in != __end && *__in == __atoms[2] /* __zero_char */ ) {
      ++__in;
      if (__in != __end &&
          (*__in == __atoms[3] /* __x_char */ || *__in == __atoms[4] /* __X_char */ ))
        ++__in;
      else
        __valid_zero = 1; // That zero is valid by itself.
    }
    break;
  default:
    if (__in != __end && *__in == __atoms[2] /* __zero_char */ ) {
      ++__in;
      if (__in != __end &&
          (*__in == __atoms[3] /* __x_char */ || *__in == __atoms[4] /* __X_char */ )) {
        ++__in;
        __base = 16;
      }
      else
        {
          __base = 8;
          __valid_zero = 1; // That zero is still valid by itself.
        }
    }
    else
      __base = 10;
    break;
  }
  return (__base << 2) | ((int)__negative << 1) | __valid_zero;
}


template <class _InputIter, class _Integer>
bool _STLP_CALL
__get_integer(_InputIter& __first, _InputIter& __last,
	      int __base, _Integer& __val, 
	      int __got, bool __is_negative, char __separator, const string& __grouping, const __true_type&) 
{
  bool __ovflow = false;
  _Integer __result = 0;
  bool __is_group = !__grouping.empty();
  char __group_sizes[64];
  int __current_group_size = 0;
  char* __group_sizes_end = __group_sizes;
  
  _Integer __over_base = (numeric_limits<_Integer>::min)() / __STATIC_CAST(_Integer, __base);

   for ( ; __first != __last ; ++__first) {
  
     const char __c = *__first;
     
     if (__is_group && __c == __separator) {
       *__group_sizes_end++ = __current_group_size;
       __current_group_size = 0;
       continue;
     }
     
     int __n = __get_digit_from_table(__c);
     
     if (__n >= __base)
       break;
     
     ++__got;
     ++__current_group_size;
     
     if (__result < __over_base)
       __ovflow = true;  // don't need to keep accumulating
     else {
       _Integer __next = __STATIC_CAST(_Integer, __base * __result - __n);
       if (__result != 0)
	 __ovflow = __ovflow || __next >= __result;
       __result = __next;
     }
   }

   if (__is_group && __group_sizes_end != __group_sizes) {
     *__group_sizes_end++ = __current_group_size;
   }

   // fbp : added to not modify value if nothing was read
   if (__got > 0) {
       __val = __ovflow
	 ? __is_negative ? (numeric_limits<_Integer>::min)()
	 : (numeric_limits<_Integer>::max)()
	 : (__is_negative ? __result : __STATIC_CAST(_Integer, -__result));
   }
  // overflow is being treated as failure
  return ((__got > 0) && !__ovflow) && (__is_group == 0 || __valid_grouping(__group_sizes, __group_sizes_end,
									    __grouping.data(), __grouping.data()+ __grouping.size())) ;
}

template <class _InputIter, class _Integer>
bool _STLP_CALL
__get_integer(_InputIter& __first, _InputIter& __last,
	      int __base, _Integer& __val, 
	      int __got, bool __is_negative, char __separator, const string& __grouping, const __false_type&) 
{
  bool __ovflow = false;
  _Integer __result = 0;
  bool __is_group = !__grouping.empty();
  char __group_sizes[64];
  int __current_group_size = 0;
  char* __group_sizes_end = __group_sizes;

  _Integer  __over_base = (numeric_limits<_Integer>::max)() / __STATIC_CAST(_Integer, __base);

  for ( ; __first != __last ; ++__first) {

    const char __c = *__first;
    
    if (__is_group && __c == __separator) {
      *__group_sizes_end++ = __current_group_size;
      __current_group_size = 0;
      continue;
    }
    
    int __n = __get_digit_from_table(__c);
    
    if (__n >= __base)
      break;
    
    ++__got;
    ++__current_group_size;

    if (__result > __over_base)
      __ovflow = true;  //don't need to keep accumulating
    else {
      _Integer __next = __STATIC_CAST(_Integer, __base * __result + __n);
	if (__result != 0)
	  __ovflow = __ovflow || __next <= __result;
	__result = __next;
      }      
  }

  if (__is_group && __group_sizes_end != __group_sizes) {
      *__group_sizes_end++ = __current_group_size;
  }

  // fbp : added to not modify value if nothing was read
  if (__got > 0) {
      __val = __ovflow
	? (numeric_limits<_Integer>::max)()
	: (__is_negative ? __STATIC_CAST(_Integer, -__result) : __result);      
  }
  // overflow is being treated as failure
  return ((__got > 0) && !__ovflow) && 
    (__is_group == 0 || __valid_grouping(__group_sizes, __group_sizes_end,
					 __grouping.data(), __grouping.data()+ __grouping.size())) ;
}


template <class _InputIter, class _Integer>
bool _STLP_CALL
__get_decimal_integer(_InputIter& __first, _InputIter& __last, _Integer& __val)
{
  string __grp;
  return __get_integer(__first, __last, 10, __val, 0, false, ' ', __grp, __false_type());
}

template <class _InputIter, class _Integer, class _CharT>
_InputIter _STLP_CALL
_M_do_get_integer(_InputIter& __in, _InputIter& __end, ios_base& __str,
                  ios_base::iostate& __err, _Integer& __val, _CharT* __pc) 
{

#if defined(__HP_aCC) && (__HP_aCC == 1)
  bool _IsSigned = !((_Integer)(-1) > 0);
#else
  typedef typename __bool2type<numeric_limits<_Integer>::is_signed>::_Ret _IsSigned;
#endif

  const numpunct<_CharT>& __numpunct = *(const numpunct<_CharT>*)__str._M_numpunct_facet();
  const string& __grouping = __str._M_grouping(); // cached copy

  const int __base_or_zero = _M_get_base_or_zero(__in, __end, __str, __pc);
  int  __got = __base_or_zero & 1;

  bool __result;

  if (__in == __end) {      // We may have already read a 0.  If so,

    if (__got > 0) {       // the result is 0 even if we're at eof.
      __val = 0;
      __result = true;
    }
    else
      __result = false;    
  } else {

    const bool __negative = __base_or_zero & 2;
    const int __base = __base_or_zero >> 2;

#if defined(__HP_aCC) && (__HP_aCC == 1)
     if (_IsSigned)
       __result = __get_integer(__in, __end, __base,  __val, __got, __negative, __numpunct.thousands_sep(), __grouping, __true_type() );
     else
      __result = __get_integer(__in, __end, __base,  __val, __got, __negative, __numpunct.thousands_sep(), __grouping, __false_type() );
#else
    __result = __get_integer(__in, __end, __base,  __val, __got, __negative, __numpunct.thousands_sep(), __grouping, _IsSigned());
# endif
  }

  __err = __STATIC_CAST(ios_base::iostate, __result ? ios_base::goodbit : ios_base::failbit);

  if (__in == __end)
    __err |= ios_base::eofbit;
  return __in;
}

// _M_read_float and its helper functions.
template <class _InputIter, class _CharT>
_InputIter  _STLP_CALL
__copy_sign(_InputIter __first, _InputIter __last, string& __v,
            _CharT __xplus, _CharT __xminus) {
    if (__first != __last) {
    _CharT __c = *__first;
    if (__c == __xplus)
      ++__first;
    else if (__c == __xminus) {
      __v.push_back('-');
      ++__first;
    }
  }
  return __first;
}


template <class _InputIter, class _CharT>
bool _STLP_CALL
__copy_digits(_InputIter& __first, _InputIter& __last,
              string& __v, const _CharT* __digits)
{
  bool __ok = false;

  for ( ; __first != __last; ++__first) {
    _CharT __c = *__first;
    if (__get_fdigit(__c, __digits)) {
      __v.push_back((char)__c);
      __ok = true;
    }
    else
      break;
  }
  return __ok;
}

template <class _InputIter, class _CharT>
bool _STLP_CALL
__copy_grouped_digits(_InputIter& __first, _InputIter& __last,
		      string& __v, const _CharT * __digits,
		      _CharT __sep, const string& __grouping,
		      bool& __grouping_ok)
{
  bool __ok = false;
  char __group_sizes[64];
  char*__group_sizes_end = __group_sizes;
  char __current_group_size = 0;

  for ( ; __first != __last; ++__first) {
    _CharT __c = *__first;
    bool __tmp = __get_fdigit_or_sep(__c, __sep, __digits);
    if (__tmp) {
      if (__c == ',') {
        *__group_sizes_end++ = __current_group_size;
        __current_group_size = 0;
      }
      else {
        __ok = true;
        __v.push_back((char)__c);
        ++__current_group_size;
      }
    }
    else
      break;
  }
  
  if (__group_sizes_end != __group_sizes)
    *__group_sizes_end++ = __current_group_size;
  __grouping_ok = __valid_grouping(__group_sizes, __group_sizes_end, __grouping.data(), __grouping.data() + __grouping.size());
  return __ok;	
}


template <class _InputIter, class _CharT>
bool _STLP_CALL
_M_read_float(string& __buf, _InputIter& __in, _InputIter& __end, ios_base& __s, _CharT*)
{
  // Create a string, copying characters of the form 
  // [+-]? [0-9]* .? [0-9]* ([eE] [+-]? [0-9]+)?

  bool __digits_before_dot /* = false */;
  bool __digits_after_dot = false;
  bool __ok;

  bool   __grouping_ok = true;

  const ctype<_CharT>& __ct = use_facet< ctype<_CharT> >(__s.getloc());
  // const ctype<_CharT>& __ct = *(const ctype<_CharT>*)__s._M_ctype_facet();
  const numpunct<_CharT>& __numpunct = *(const numpunct<_CharT>*)__s._M_numpunct_facet();
  const string& __grouping = __s._M_grouping(); // cached copy

  _CharT __dot = __numpunct.decimal_point();
  _CharT __sep = __numpunct.thousands_sep();

  _CharT __digits[10];
  _CharT __xplus;
  _CharT __xminus;

  _CharT __pow_e;
  _CharT __pow_E;

  _Initialize_get_float(__ct, __xplus, __xminus, __pow_e, __pow_E, __digits);

  // Get an optional sign
  __in = __copy_sign(__in, __end, __buf, __xplus, __xminus);

  // Get an optional string of digits.
  if (__grouping.size() != 0)
    __digits_before_dot = __copy_grouped_digits(__in, __end, __buf, __digits,
						__sep, __grouping, __grouping_ok);
  else
    __digits_before_dot = __copy_digits(__in, __end, __buf, __digits);

  // Get an optional decimal point, and an optional string of digits.
  if (__in != __end && *__in == __dot) {
    __buf.push_back('.');
    ++__in;
    __digits_after_dot = __copy_digits(__in, __end, __buf, __digits);
  }

  // There have to be some digits, somewhere.
  __ok = __digits_before_dot || __digits_after_dot;
  
  // Get an optional exponent.
  if (__ok && __in != __end && (*__in == __pow_e || *__in == __pow_E)) {
    __buf.push_back('e');
    ++__in;
    __in = __copy_sign(__in, __end, __buf, __xplus, __xminus);
    __ok = __copy_digits(__in, __end, __buf, __digits);
    // If we have an exponent then the sign 
    // is optional but the digits aren't.
  }
  
  return __ok;
}

//
// num_get<>, num_put<>
//

# if ( _STLP_STATIC_TEMPLATE_DATA > 0 ) 
template <class _CharT, class _InputIterator>
locale::id num_get<_CharT, _InputIterator>::id;
# else

typedef num_get<char, const char*> num_get_char;
typedef num_get<char, istreambuf_iterator<char, char_traits<char> > > num_get_char_2;

__DECLARE_INSTANCE(locale::id, num_get_char::id, );
__DECLARE_INSTANCE(locale::id, num_get_char_2::id, );

# ifndef _STLP_NO_WCHAR_T

typedef num_get<wchar_t, const wchar_t*> num_get_wchar_t;
typedef num_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > > num_get_wchar_t_2;

__DECLARE_INSTANCE(locale::id, num_get_wchar_t::id, );
__DECLARE_INSTANCE(locale::id, num_get_wchar_t_2::id, );

# endif

# endif /* ( _STLP_STATIC_TEMPLATE_DATA > 0 ) */

# ifndef _STLP_NO_BOOL
template <class _CharT, class _InputIter>
_InputIter
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end,
                                    ios_base& __s,
                                    ios_base::iostate& __err, bool& __x) const
{
  if (__s.flags() & ios_base::boolalpha) {
    locale __loc = __s.getloc();
    const _Numpunct& __np = *(const _Numpunct*)__s._M_numpunct_facet();
    //    const numpunct<_CharT>& __np = use_facet<numpunct<_CharT> >(__loc) ;
//    const ctype<_CharT>& __ct =    use_facet<ctype<_CharT> >(__loc) ;

    const basic_string<_CharT> __truename  = __np.truename();
    const basic_string<_CharT> __falsename = __np.falsename();
    bool __true_ok  = true;
    bool __false_ok = true;

    size_t __n = 0;
    for ( ; __in != __end; ++__in) {
      _CharT __c = *__in;
      __true_ok  = __true_ok  && (__c == __truename[__n]);
      __false_ok = __false_ok && (__c == __falsename[__n]);
      ++__n;

      if ((!__true_ok && !__false_ok) ||
          (__true_ok  && __n >= __truename.size()) ||
          (__false_ok && __n >= __falsename.size())) {
	++__in;
        break;
      }
    }
    if (__true_ok  && __n < __truename.size())  __true_ok  = false;
    if (__false_ok && __n < __falsename.size()) __false_ok = false;
    
    if (__true_ok || __false_ok) {
      __err = ios_base::goodbit;
      __x = __true_ok;
    }
    else
      __err = ios_base::failbit;

    if (__in == __end)
      __err |= ios_base::eofbit;

    return __in;
  }

  else {
    long __lx;
    _InputIter __tmp = this->do_get(__in, __end, __s, __err, __lx);
    if (!(__err & ios_base::failbit)) {
      if (__lx == 0)
        __x = false;
      else if (__lx == 1)
        __x = true;
      else
        __err |= ios_base::failbit;
    }
    return __tmp;
  }
}

# endif /* _STLP_NO_BOOL */

# ifdef _STLP_FIX_LIBRARY_ISSUES
template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err, short& __val) const {
  return _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
}

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err, int& __val) const {
  return _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
}

# endif

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err, long& __val) const {
  return _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
}

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err,
                                    unsigned short& __val) const {
  return _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
}

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err, 
                                    unsigned int& __val) const {
  return _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
}

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err,
                                    unsigned long& __val) const {
  return _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
}


template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err,
                                    float& __val) const {
  string __buf ;
  bool __ok = _M_read_float(__buf, __in, __end, __str, (_CharT*)0 );
  __string_to_float(__buf, __val);  
  __err = __STATIC_CAST(ios_base::iostate, __ok ? ios_base::goodbit : ios_base::failbit);
  if (__in == __end)
    __err |= ios_base::eofbit;
  return __in;
}

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err, 
                                    double& __val) const {
  string __buf ;
  bool __ok = _M_read_float(__buf, __in, __end, __str, (_CharT*)0 );
  __string_to_float(__buf, __val);
  __err = __STATIC_CAST(ios_base::iostate, __ok ? ios_base::goodbit : ios_base::failbit);
  if (__in == __end)
    __err |= ios_base::eofbit;
  return __in;
}

#ifndef _STLP_NO_LONG_DOUBLE
template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
				    ios_base::iostate& __err,
                                    long double& __val) const {
  string __buf ;
  bool __ok = _M_read_float(__buf, __in, __end, __str, (_CharT*)0 );
  __string_to_float(__buf, __val);
  __err = __STATIC_CAST(ios_base::iostate, __ok ? ios_base::goodbit : ios_base::failbit);
  if (__in == __end)
    __err |= ios_base::eofbit;
  return __in;
}
#endif /* _STLP_NO_LONG_DOUBLE */

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                           ios_base::iostate& __err,
                           void*& __p) const {
# if defined(_STLP_LONG_LONG)&&!defined(__MRC__)		//*ty 12/07/2001 - MrCpp can not cast from long long to void*
  unsigned _STLP_LONG_LONG __val;
# else
  unsigned long __val;
# endif
    iter_type __tmp = _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
    if (!(__err & ios_base::failbit))
      __p = __REINTERPRET_CAST(void*,__val);
    return __tmp;
  }


#ifdef _STLP_LONG_LONG

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err,
                                    _STLP_LONG_LONG& __val) const {
  return _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
}

template <class _CharT, class _InputIter>  
_InputIter 
num_get<_CharT, _InputIter>::do_get(_InputIter __in, _InputIter __end, ios_base& __str,
                                    ios_base::iostate& __err,
                                    unsigned _STLP_LONG_LONG& __val) const {
  return _M_do_get_integer(__in, __end, __str, __err, __val, (_CharT*)0 );
}

#endif /* _STLP_LONG_LONG */

_STLP_END_NAMESPACE

# endif /* _STLP_EXPOSE_STREAM_IMPLEMENTATION */

#endif /* _STLP_NUMERIC_FACETS_C */

// Local Variables:
// mode:C++
// End:
