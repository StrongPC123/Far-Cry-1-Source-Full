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
#ifndef _STLP_MONETARY_C
#define _STLP_MONETARY_C

# ifndef _STLP_INTERNAL_MONETARY_H
#  include <stl/_monetary.h>
# endif

# if defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION)

#ifndef _STLP_INTERNAL_IOS_H
# include <stl/_ios.h>
#endif

#ifndef _STLP_INTERNAL_NUM_PUT_H
# include <stl/_num_put.h>
#endif

#ifndef _STLP_INTERNAL_NUM_GET_H
# include <stl/_num_get.h>
#endif

_STLP_BEGIN_NAMESPACE

# if ( _STLP_STATIC_TEMPLATE_DATA > 0 )

template <class _CharT, class _InputIterator>
locale::id money_get<_CharT, _InputIterator>::id;

template <class _CharT, class _OutputIterator>
locale::id money_put<_CharT, _OutputIterator>::id;

# else /* ( _STLP_STATIC_TEMPLATE_DATA > 0 ) */

typedef money_get<char, const char*> money_get_char;
typedef money_put<char, char*> money_put_char;
typedef money_get<char, istreambuf_iterator<char, char_traits<char> > > money_get_char_2;
typedef money_put<char, ostreambuf_iterator<char, char_traits<char> > > money_put_char_2;

__DECLARE_INSTANCE(locale::id, money_get_char::id, );
__DECLARE_INSTANCE(locale::id, money_put_char::id, );
__DECLARE_INSTANCE(locale::id, money_get_char_2::id, );
__DECLARE_INSTANCE(locale::id, money_put_char_2::id, );

# ifndef _STLP_NO_WCHAR_T

typedef money_get<wchar_t, const wchar_t*> money_get_wchar_t;
typedef money_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > > money_get_wchar_t_2;
typedef money_put<wchar_t, wchar_t*> money_put_wchar_t;
typedef money_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > > money_put_wchar_t_2;

__DECLARE_INSTANCE(locale::id, money_get_wchar_t::id, );
__DECLARE_INSTANCE(locale::id, money_put_wchar_t::id, );
__DECLARE_INSTANCE(locale::id, money_get_wchar_t_2::id, );
__DECLARE_INSTANCE(locale::id, money_put_wchar_t_2::id, );

# endif
# endif /* ( _STLP_STATIC_TEMPLATE_DATA > 0 ) */

// money_get facets


// helper functions for do_get
template <class _InIt1, class _InIt2>
pair<_InIt1, bool> __get_string(_InIt1 __first,     _InIt1 __last,
                               _InIt2 __str_first, _InIt2 __str_last) {
  pair<_InIt1, _InIt2> __pr = mismatch(__first, __last, __str_first);
  return make_pair(__pr.first, __pr.second == __str_last);
}

template <class _InIt, class _OuIt, class _CharT>
bool
__get_monetary_value(_InIt& __first, _InIt __last, _OuIt __out,
                     const ctype<_CharT>& _c_type,
                     _CharT   __point,
                     int      __frac_digits,
                     _CharT __sep,
                     const string& __grouping,
                     bool&         __syntax_ok)
{
  if (__first == __last || !_c_type.is(ctype_base::digit, *__first))
    return false;

  char __group_sizes[128];
  char* __group_sizes_end = __grouping.size() == 0 ? 0 : __group_sizes;
  char   __current_group_size = 0;

  while (__first != __last) {
    if (_c_type.is(ctype_base::digit, *__first)) {
      ++__current_group_size;
      *__out++ = *__first++;
    }
    else if (__group_sizes_end) {
      if (*__first == __sep) {
	*__group_sizes_end++ = __current_group_size;
	__current_group_size = 0;
	++__first;
      }
      else break;
    }
    else
      break;
  }

  if (__grouping.size() == 0)
    __syntax_ok = true;
  else {
    if (__group_sizes_end != __group_sizes)
      *__group_sizes_end++ = __current_group_size;
    
    __syntax_ok = __valid_grouping(__group_sizes, __group_sizes_end,
                                   __grouping.data(), __grouping.data()+ __grouping.size());  
    
    if (__first == __last || *__first != __point) {
      for (int __digits = 0; __digits != __frac_digits; ++__digits)
        *__out++ = _CharT('0');
      return true; // OK not to have decimal point
    }
  }

  ++__first;

  size_t __digits = 0;

  while (__first != __last && _c_type.is(ctype_base::digit, *__first)) {
      *__out++ = *__first++;
     ++__digits;
  }

  __syntax_ok = __syntax_ok && (__digits == __frac_digits);

  return true;
}

# ifndef _STLP_NO_LONG_DOUBLE

//===== methods ======
template <class _CharT, class _InputIter>
_InputIter 
money_get<_CharT, _InputIter>::do_get(_InputIter __s, _InputIter  __end, bool  __intl,
				      ios_base&  __str, ios_base::iostate& __err,
				      long double& __units) const {
  string_type __buf;
  __s = do_get(__s, __end, __intl, __str, __err, __buf);

  if (__err == ios_base::goodbit || __err == ios_base::eofbit) {
    __buf.push_back(0);
    typename string_type::iterator __b = __buf.begin(), __e = __buf.end();
    // Can't use atold, since it might be wchar_t. Don't get confused by name below :
    // it's perfectly capable of reading long double.
    __get_decimal_integer(__b, __e, __units);
  }
  if (__s == __end)
    __err |= ios_base::eofbit;
  return __s;
}
# endif

template <class _CharT, class _InputIter>
_InputIter 
money_get<_CharT, _InputIter>::do_get(iter_type __s, 
				      iter_type  __end, bool  __intl,
				      ios_base&  __str, ios_base::iostate&  __err,
				      string_type& __digits) const {
  if (__s == __end) {
    __err |= ios_base::eofbit;
    return __s;
  }

  typedef moneypunct<_CharT, false> _Punct;
  typedef moneypunct<_CharT, true>  _Punct_intl;
  typedef ctype<_CharT>             _Ctype;

  locale __loc = __str.getloc();
  const _Punct&      __punct      = use_facet<_Punct>(__loc) ;
  const _Punct_intl& __punct_intl = use_facet<_Punct_intl>(__loc) ;
  const _Ctype&      __c_type      = use_facet<_Ctype>(__loc) ;
                   
  money_base::pattern __format = __intl ? __punct_intl.neg_format()
                                        : __punct.neg_format();
  string_type __ns = __intl ? __punct_intl.negative_sign()
                            : __punct.negative_sign();
  string_type __ps = __intl ? __punct_intl.positive_sign()
                            : __punct.positive_sign();
  int __i;
  bool __is_positive = true;
  bool __symbol_required = (__str.flags() & ios_base::showbase) !=0;
  string_type __buf;
  back_insert_iterator<string_type> __out(__buf);
//  pair<iter_type, bool> __result;

  for (__i = 0; __i < 4; ++__i) {
    switch (__format.field[__i]) {
    case (char) money_base::none:
      if (__i == 3) {
        if (__c_type.is(ctype_base::space, *__s)) {
          __err = ios_base::failbit;
          return __s;
        }
        break;
      }
      while (__s != __end && __c_type.is(ctype_base::space, *__s))
        ++__s;
      break;
    case (char) money_base::space:
      if (!__c_type.is(ctype_base::space, *__s)) {
        __err = ios_base::failbit;
        return __s;
      }
      ++__s;
      while (__s != __end && __c_type.is(ctype_base::space, *__s))
        ++__s;
      break;
    case money_base::symbol: {
      string_type __curs = __intl ? __punct_intl.curr_symbol()
                                : __punct.curr_symbol();
      pair<iter_type, bool>
      __result  = __get_string(__s, __end, __curs.begin(), __curs.end());
      if (!__result.second && __symbol_required)
        __err = ios_base::failbit;
      __s = __result.first;
      break;
    }
    case money_base::sign: {
      if (__s == __end) {
        if (__ps.size() == 0)
          break;
        if (__ns.size() == 0) {
          __is_positive = false;
          break;
        }
        __err = ios_base::failbit;
        return __s;
      }
      else {
        if (__ps.size() == 0) {
          if (__ns.size() == 0)
            break;
          if (*__s == ++__ns[0]) {
            ++__s;
            __is_positive = false;
            break;
          }
          __err = ios_base::failbit;
	  //          return __s;
        } 
        else {
          if (*__s == __ps[0]) {
            ++__s;
            break;
          }
          if (__ns.size() == 0)
            break;
          if (*__s == __ns[0]) {
            ++__s;
            __is_positive = false;
            break;
          }
          __err = ios_base::failbit;
	  //          return __s;
        }
      }
      return __s;
      //      break;
    }
    case money_base::value: {
      _CharT __point = __intl ? __punct_intl.decimal_point()
                              : __punct.decimal_point();
      int __frac_digits = __intl ? __punct_intl.frac_digits()
                                 : __punct.frac_digits();
      string __grouping = __intl ? __punct_intl.grouping()
                                 : __punct.grouping();
      bool __syntax_ok = true;

      bool __result;

      _CharT __sep = __grouping.size() == 0 ? _CharT() : 
	__intl ? __punct_intl.thousands_sep() : __punct.thousands_sep();

      __result = __get_monetary_value(__s, __end, __out, __c_type,
                                      __point, __frac_digits,
                                      __sep,
                                      __grouping, __syntax_ok);      

      if (!__syntax_ok)
        __err |= ios_base::failbit;
      if (!__result) {
        __err = ios_base::failbit;
        return __s;
      }
      break;
      
    }                           // Close money_base::value case


    }                           // Close switch statement
  }                             // Close for loop

  if (__is_positive) {
    if (__ps.size() > 1) {
      pair<_InputIter, bool>
        __result = __get_string(__s, __end, __ps.begin() + 1, __ps.end());
      __s = __result.first;
      if (!__result.second)
	__err |= ios::failbit;
    }
    if (!(__err & ios_base::failbit))
      __digits = __buf;
  }
  else {
    if (__ns.size() > 1) {
      pair<_InputIter, bool>
        __result = __get_string(__s, __end, __ns.begin() + 1, __ns.end());
      __s = __result.first;
      if (!__result.second)
	__err |= ios::failbit;
    }
    if (!(__err & ios::failbit)) {
      __buf.insert(__buf.begin(),__c_type.widen('-'));
      __digits = __buf;
    }
  }
  if (__s == __end)
    __err |= ios::eofbit;

  return __s;
}

// money_put facets

template <class _CharT, class _OutputIter>
_OutputIter
money_put<_CharT, _OutputIter>
 ::do_put(_OutputIter __s, bool __intl, ios_base& __str,
          char_type __fill,
          const string_type& __digits) const { 
  typedef ctype<_CharT>             _Ctype;
  typedef moneypunct<_CharT, false> _Punct;
  typedef moneypunct<_CharT, true>  _Punct_intl;

  locale __loc = __str.getloc();
  const _Ctype&      __c_type      = use_facet<_Ctype>(__loc) ;
  const _Punct&      __punct      = use_facet<_Punct>(__loc) ;
  const _Punct_intl& __punct_intl = use_facet<_Punct_intl>(__loc) ;

  // some special characters

  char_type __minus = __c_type.widen('-');
  char_type __plus  = __c_type.widen('+');
  char_type __space = __c_type.widen(' ');
  char_type __zero  = __c_type.widen('0');
  char_type __point = __intl ? __c_type.widen(__punct_intl.decimal_point())
			     : __c_type.widen(__punct.decimal_point());

  char_type __sep = __intl ? __punct_intl.thousands_sep()
			   : __punct     .thousands_sep();

  string __grouping = __intl ? __punct_intl.grouping()
		             : __punct     .grouping();
				
  int __frac_digits      = __intl ? __punct_intl.frac_digits() 
                                  : __punct.frac_digits();

  string_type __curr_sym = __intl ? __punct_intl.curr_symbol() 
                                  : __punct.curr_symbol();

    // if there are no digits we are going to return __s.  If there
    // are digits, but not enough to fill the frac_digits, we are
    // going to add zeros.  I don't know whether this is right or
    // not.

  if (__digits.size() == 0) 
    return __s;

  typename string_type::const_iterator __digits_first = __digits.begin();
  typename string_type::const_iterator __digits_last  = __digits.end();

  bool __is_negative = *__digits_first == __minus;
  if (__is_negative)
    ++__digits_first;

  string_type __sign = __intl ?
			 __is_negative ? __punct_intl.negative_sign()
				       : __punct_intl.positive_sign()
			      :
			 __is_negative ? __punct.negative_sign()
				       : __punct.positive_sign();
  typename string_type::const_iterator __cp = __digits_first;
  while (__cp != __digits_last && __c_type.is(ctype_base::digit, *__cp))
    ++__cp;
  if (__cp == __digits_first)
    return __s;
  __digits_last = __cp;

  // If grouping is required, we make a copy of __digits and
  // insert the grouping.

  // To handle the fractional digits, we augment the first group
  // by frac_digits.  If there is only one group, we need first
  // to duplicate it.

  string_type __new_digits(__digits_first, __digits_last);

  if (__grouping.size() != 0) {
    if (__grouping.size() == 1)
      __grouping.push_back(__grouping[0]);
    __grouping[0] += __frac_digits;
    _CharT* __data_ptr = __CONST_CAST(_CharT*,__new_digits.data());
    _CharT* __data_end = __data_ptr + __new_digits.size();
    
    ptrdiff_t __value_length = __insert_grouping(__data_ptr,
	  				         __data_end,
					         __grouping,
					         __sep,
					         __plus, __minus, 0);
    __digits_first = __new_digits.begin();
    __digits_last  = __digits_first + __value_length;
  }

  // Determine the amount of padding required, if any.  
    
  size_t __width        = __str.width();

#if defined(_STLP_DEBUG) && (defined(__HP_aCC) || (__HP_aCC <= 1))
  size_t __value_length = operator -(__digits_last, __digits_first);
#else
  size_t __value_length = __digits_last - __digits_first;
#endif

  size_t __length       = __value_length;
      
  __length += __sign.size();
  if (__frac_digits != 0)
    ++__length;

  bool __generate_curr = (__str.flags() & ios_base::showbase) !=0;
  if (__generate_curr)
    __length += __curr_sym.size();
  money_base::pattern __format =
    __intl ? (__is_negative ? __punct_intl.neg_format() 
                            : __punct_intl.pos_format())
           : (__is_negative ? __punct.neg_format() 
                            : __punct.pos_format());
  {
    for (int __i = 0; __i < 4; ++__i)
      if (__format.field[__i] == (char) money_base::space)
        ++__length;
  }

  size_t __fill_amt = __length < __width ? __width - __length : 0;

  ios_base::fmtflags __fill_pos = __str.flags() & ios_base::adjustfield;

  if (__fill_amt != 0 &&
      !(__fill_pos & (ios_base::left | ios_base::internal)))
    __s = fill_n(__s, __fill_amt, __fill);
    
  for (int __i = 0; __i < 4; ++__i) {
    char __ffield = __format.field[__i];
    if (__ffield == money_base::none) {
      if (__fill_amt != 0 && __fill_pos == ios_base::internal)
        __s = fill_n(__s, __fill_amt, __fill);
    }
    else if (__ffield == money_base::space) {
      *__s++ = __space;
      if (__fill_amt != 0 && __fill_pos == ios_base::internal)
        __s = fill_n(__s, __fill_amt, __fill);
    }
    else if (__ffield == money_base::symbol) {
      if (__generate_curr)
        __s = copy(__curr_sym.begin(), __curr_sym.end(), __s);
    }
    else if (__ffield == money_base::sign) {
      if (__sign.size() != 0)
        *__s++ = __sign[0];
    }
    else if (__ffield == money_base::value) {
      if (__frac_digits == 0)
        __s = copy(__digits_first, __digits_last, __s);
      else {
        if ((int)__value_length <= __frac_digits) {
          *__s++ = __point;
          __s = copy(__digits_first, __digits_last, __s);
          __s =  fill_n(__s, __frac_digits - __value_length, __zero);
        }
        else {
          __s = copy(__digits_first, __digits_last - __frac_digits, __s);
          if (__frac_digits != 0) {
            *__s++ = __point;
            __s = copy(__digits_last - __frac_digits, __digits_last, __s);
          }
        }
      }
    }
  } // Close for loop

  // Ouput rest of sign if necessary.

  if (__sign.size() > 1)
    __s = copy(__sign.begin() + 1, __sign.end(), __s);
  if (!(__fill_pos & (ios_base::right | ios_base::internal)))
    __s = fill_n(__s, __fill_amt, __fill);
  
  return __s;
}

_STLP_END_NAMESPACE

# endif /* EXPOSE */

#endif /* _STLP_MONETARY_C */
