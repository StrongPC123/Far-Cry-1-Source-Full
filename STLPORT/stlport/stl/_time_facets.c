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
#ifndef _STLP_TIME_FACETS_C
#define _STLP_TIME_FACETS_C

#ifndef _STLP_INTERNAL_TIME_FACETS_H
# include <stl/_time_facets.h>
#endif

#if defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION)

#ifndef _STLP_INTERNAL_NUM_PUT_H
# include <stl/_num_put.h>
#endif

#ifndef _STLP_INTERNAL_NUM_GET_H
# include <stl/_num_get.h>
#endif

_STLP_BEGIN_NAMESPACE

//----------------------------------------------------------------------
// Declarations of static template members.
# if ( _STLP_STATIC_TEMPLATE_DATA > 0 )

template <class _CharT, class _InputIterator>
locale::id time_get<_CharT, _InputIterator>::id;

template <class _CharT, class _OutputIterator>
locale::id time_put<_CharT, _OutputIterator>::id;

# else /* ( _STLP_STATIC_TEMPLATE_DATA > 0 ) */

typedef time_get<char, const char*> time_get_char;
typedef time_get<char, char*> time_get_char_2;
typedef time_get<char, istreambuf_iterator<char, char_traits<char> > > time_get_char_3;
typedef time_put<char, const char*> time_put_char;
typedef time_put<char, char*> time_put_char_2;
typedef time_put<char, ostreambuf_iterator<char, char_traits<char> > > time_put_char_3;

__DECLARE_INSTANCE(locale::id, time_get_char::id, );
__DECLARE_INSTANCE(locale::id, time_get_char_2::id, );
__DECLARE_INSTANCE(locale::id, time_get_char_3::id, );
__DECLARE_INSTANCE(locale::id, time_put_char::id, );
__DECLARE_INSTANCE(locale::id, time_put_char_2::id, );
__DECLARE_INSTANCE(locale::id, time_put_char_3::id, );

# ifndef _STLP_NO_WCHAR_T

typedef time_get<wchar_t, const wchar_t*> time_get_wchar_t;
typedef time_get<wchar_t, wchar_t*> time_get_wchar_t_2;
typedef time_get<wchar_t, istreambuf_iterator<wchar_t, char_traits<wchar_t> > > time_get_wchar_t_3;
typedef time_put<wchar_t, const wchar_t*> time_put_wchar_t;
typedef time_put<wchar_t, wchar_t*> time_put_wchar_t_2;
typedef time_put<wchar_t, ostreambuf_iterator<wchar_t, char_traits<wchar_t> > > time_put_wchar_t_3;

__DECLARE_INSTANCE(locale::id, time_get_wchar_t::id, );
__DECLARE_INSTANCE(locale::id, time_get_wchar_t_2::id, );
__DECLARE_INSTANCE(locale::id, time_get_wchar_t_3::id, );
__DECLARE_INSTANCE(locale::id, time_put_wchar_t::id, );
__DECLARE_INSTANCE(locale::id, time_put_wchar_t_2::id, );
__DECLARE_INSTANCE(locale::id, time_put_wchar_t_3::id, );

# endif

# endif /* ( _STLP_STATIC_TEMPLATE_DATA > 0 ) */

template <class _InIt, class _RAIt, class _DiffType>
_RAIt _STLP_CALL
__match(_InIt& __first, _InIt& __last, _RAIt __name, _RAIt __name_end, _DiffType*) {
  typedef _DiffType difference_type;
  difference_type __n = __name_end - __name;
  size_t __max_pos = 0;
  difference_type __i;
  difference_type __pos = 0;
  difference_type __check_count = __n;
  bool __do_check[_MAXNAMES];
  _RAIt __matching_name[_MAX_NAME_LENGTH];

  for (__i = 0; __i < __n; ++__i)
    __max_pos = (max)(__max_pos,  __name[__i].size());

  for (__i = 0; __i < _MAXNAMES; ++__i)
    __do_check[__i] = true;



  for (__i = 0; __i < _MAX_NAME_LENGTH; ++__i)
    __matching_name[__i] = __name_end;

  while (__first != __last) {
    for (__i = 0; __i < __n; ++__i)
      if (__do_check[__i])
        if (*__first == __name[__i][__pos]) {
          if (__pos == _DiffType(__name[__i].size()) - 1) {
            __do_check[__i] = 0;
            __matching_name[__pos+1] = __name + __i;
            --__check_count;
            if (__check_count == 0) {
              ++__first; 
	      return __name + __i;
	    }
          }
        }
        else {
          __do_check[__i] = 0;
          --__check_count;
          if (__check_count == 0) 
            return __matching_name[__pos];
        }

    ++__first; ++__pos;
  }

  return __matching_name[__pos];
}

template <class _InIt, class _RAIt>
_RAIt _STLP_CALL
__match(_InIt& __first, _InIt& __last, _RAIt __name, _RAIt __name_end) {
  return __match((_InIt&)__first, (_InIt&)__last, __name, __name_end, _STLP_DISTANCE_TYPE(__name, _InIt));
}

// __get_formatted_time reads input that is assumed to be formatted
// according to the rules for the C strftime function (C standard,
// 7.12.3.5).  This function is used to implement the do_get_time
// and do_get_date virtual functions, which depend on the locale
// specifications for the time and day formats respectively.
// Note the catchall default case, intended mainly for the '%Z'
// format designator, which does not make sense here since the
// representation of timezones is not part of the locale.
//
// The case branches are implemented either by doing a match using
// the appopriate name table or by doing a __get_integer_nogroup.
//
// 'y' format is assumed to mean that the input represents years
// since 1900.  That is, 2002 should be represented as 102.  There
// is no century-guessing.
//
// The match is successful if and only if the second component of the
// return value is format_end.

// Note that the antepenultimate parameter is being used only to determine
// the correct overloading for the calls to __get_integer_nogroup.

template <class _InIt1, class _InIt2 /* , class _Ch */ >
_InIt2 _STLP_CALL
__get_formatted_time _STLP_WEAK (_InIt1 __first,  _InIt1 __last,
                     _InIt2 __format, _InIt2 __format_end,
				 /* _Ch, */ const _Time_Info& __table,
		     ios_base::iostate& __err,
                     tm*         __t) {
  while(__first != __last && __format != __format_end) {
    if (*__format == '%') {
      ++__format;
      char __c = *__format;
      switch (__c) {
        case 'a': {
          const string* __pr =
            __match(__first, __last,
		    (string*)__table._M_dayname + 0 , (string*)__table._M_dayname + 7);
            if (__pr == (string*)__table._M_dayname + 7)
              return __format;
            __t->tm_wday = (int)(__pr - (string*)__table._M_dayname);
            break;
        }

        case 'A': {
          const string* __pr =
            __match(__first, __last,
		    (string*)__table._M_dayname + 7, (string*)__table._M_dayname + 14);
            if (__pr == (string*)__table._M_dayname + 14)
              return __format;
            __t->tm_wday = (int)(__pr - (string*)__table._M_dayname - 7);
            break;
        }

        case 'b': {
          const string* __pr =
            __match(__first, __last,
		    (string*)__table._M_monthname + 0, (string*)__table._M_monthname + 12);
            if (__pr == (string*)__table._M_monthname + 12)
              return __format;
            __t->tm_mon = (int)(__pr - (string*)__table._M_monthname);
            break;
        }

        case 'B': {
          const string* __pr =
            __match(__first, __last,
		    (string*)__table._M_monthname + 12, (string*)__table._M_monthname + 24);
            if (__pr == (string*)__table._M_monthname + 24)
              return __format;
            __t->tm_mon = (int)(__pr - (string*)__table._M_monthname - 12);
            break;
        }

        case 'd': {
          bool __pr =
            __get_decimal_integer(__first, __last, __t->tm_mday);
          if (!__pr || __t->tm_mday < 1 || __t->tm_mday > 31) {
	    __err |= ios_base::failbit;
            return __format;
	  }
          break;
        }
        
        case 'H': case 'I': {
          bool __pr =
            __get_decimal_integer(__first, __last, __t->tm_hour);
            if (!__pr)
              return __format;
            break;
        }

        case 'j': {
          bool __pr =
            __get_decimal_integer(__first, __last, __t->tm_yday);
          if (!__pr)
            return __format;
          break;
        }

        case 'm': {
          bool __pr =
            __get_decimal_integer(__first, __last, __t->tm_mon);
	    --__t->tm_mon;
          if (!__pr || __t->tm_mon < 0 || __t->tm_mon > 11) {
	    __err |= ios_base::failbit;
            return __format;
	  }
          break;
        }

        case 'M': {
          bool __pr =
            __get_decimal_integer(__first, __last, __t->tm_min);
          if (!__pr)
            return __format;
          break;
        }

        case 'p': {
          const string* __pr =
            __match(__first, __last, (string*)__table._M_am_pm + 0, (string*)__table._M_am_pm + 2);
          if (__pr == (string*)__table._M_am_pm + 2)
            return __format;
          if (__pr == (string*)__table._M_am_pm + 1)
            __t->tm_hour += 12;
          break;
        }

        case 'S': {
          bool __pr =
            __get_decimal_integer(__first, __last, __t->tm_sec);
          if (!__pr)
            return __format;
          break;
        }

	case 'y': {
	  bool __pr =
	    __get_decimal_integer(__first, __last, __t->tm_year);
	  if (!__pr)
	    return __format;
	  break;
        }

        case 'Y': {
	  bool __pr =
            __get_decimal_integer(__first, __last, __t->tm_year);
          __t->tm_year -= 1900;
          if (!__pr)
            return __format;
          break;
        }

        default:
          break;
      }

    }
    else {
      if (*__first++ != *__format) break;
    }
    
    ++__format;
  }

  return __format;
}

template <class _InIt>
bool _STLP_CALL
__get_short_or_long_dayname(_InIt& __first, _InIt& __last,
                            const _Time_Info& __table, tm* __t) {
  const string* __pr =
    __match(__first, __last, __table._M_dayname + 0, __table._M_dayname + 14);
  __t->tm_wday = (int)(__pr - __table._M_dayname) % 7;
  return __pr != __table._M_dayname + 14;
}

template <class _InIt>
bool _STLP_CALL
__get_short_or_long_monthname(_InIt& __first, _InIt& __last,
                              const _Time_Info& __table, tm* __t) {
  const string* __pr =
    __match(__first, __last, (string*)__table._M_monthname + 0, (string*)__table._M_monthname + 24);
  __t->tm_mon = (int)(__pr - __table._M_monthname) % 12;
  return __pr != __table._M_monthname + 24;
}

# ifndef _STLP_NO_WCHAR_T
template <class _OuIt>
_OuIt _STLP_CALL
__put_time(char * __first, char * __last, _OuIt __out,
           const ios_base& __s, wchar_t) {
    const ctype<wchar_t>& __ct = *(ctype<wchar_t>*)__s._M_ctype_facet();
    wchar_t __wbuf[64];
    __ct.widen(__first, __last, __wbuf);
    ptrdiff_t __len = __last - __first;
    wchar_t * __eend = __wbuf + __len;
    return copy((wchar_t*)__wbuf, __eend, __out);
}
# endif

template <class _Ch, class _InIt>
_InIt
time_get<_Ch, _InIt>::do_get_date(_InIt __s, _InIt  __end,
				  ios_base& /* __str */, ios_base::iostate&  __err,
				  tm* __t) const 
{
  typedef string::const_iterator string_iterator;

  string_iterator __format
    = _M_timeinfo._M_date_format.begin();
  string_iterator __format_end
    = _M_timeinfo._M_date_format.end();
  
  string_iterator __result
    = __get_formatted_time(__s, __end, __format, __format_end,
                           /* _Ch() ,*/  _M_timeinfo, __err, __t);
  if (__result == __format_end)
    __err = ios_base::goodbit;
  else {
    __err = ios_base::failbit;
    if (__s == __end)
      __err |= ios_base::eofbit;
  }
  return __s;
}

template <class _Ch, class _InIt>
_InIt
time_get<_Ch, _InIt>::do_get_time(_InIt __s, _InIt  __end,
				  ios_base& /* __str */, ios_base::iostate&  __err,
				  tm* __t) const 
{
  typedef string::const_iterator string_iterator;
  string_iterator __format
    = _M_timeinfo._M_time_format.begin();
  string_iterator __format_end
    = _M_timeinfo._M_time_format.end();
  
  string_iterator __result
    = __get_formatted_time(__s, __end, __format, __format_end,
			   /* _Ch() , */ _M_timeinfo, __err, __t);
  __err = __result == __format_end ? ios_base::goodbit 
    : ios_base::failbit;
  if (__s == __end)
    __err |= ios_base::eofbit;
  return __s;
}

template <class _Ch, class _InIt>
_InIt
time_get<_Ch, _InIt>::do_get_year(_InIt __s, _InIt  __end,
				  ios_base&, 
				  ios_base::iostate&  __err,
				  tm* __t) const
{
  
  if (__s == __end) {
    __err = ios_base::failbit | ios_base::eofbit;
    return __s;
  }
  
  bool __pr =  __get_decimal_integer(__s, __end, __t->tm_year);
  __t->tm_year -= 1900;
  __err = __pr ? ios_base::goodbit : ios_base::failbit;
  if (__s == __end)
    __err |= ios_base::eofbit;
  
  return __s;
}

template <class _Ch, class _InIt>
_InIt
time_get<_Ch, _InIt>::do_get_weekday(_InIt __s, _InIt  __end,
				     ios_base& /* __str */, 
				     ios_base::iostate&  __err,
				     tm* __t) const 
{
    bool __result =
      __get_short_or_long_dayname(__s, __end, _M_timeinfo, __t);
    if (__result)
      __err = ios_base::goodbit;
    else {
      __err = ios_base::failbit;
      if (__s == __end)
        __err |= ios_base::eofbit;
    }
    return __s;
}

template <class _Ch, class _InIt>
_InIt
time_get<_Ch, _InIt>::do_get_monthname(_InIt __s, _InIt  __end,
				       ios_base& /* __str */, 
				       ios_base::iostate&  __err,
				       tm* __t) const 
{
  bool __result =
    __get_short_or_long_monthname(__s, __end, _M_timeinfo, __t);
  if (__result)
    __err = ios_base::goodbit;
  else {
    __err = ios_base::failbit;
    if (__s == __end)
      __err |= ios_base::eofbit;
  }
  return __s;
}

template<class _Ch, class _OutputIter>
_OutputIter
time_put<_Ch,_OutputIter>::put(_OutputIter __s, ios_base& __f, _Ch __fill,
			       const tm* __tmb,
			       const _Ch* __pat, const _Ch* __pat_end) const 
{
  //  locale __loc = __f.getloc();
  //  const ctype<_Ch>& _Ct = use_facet<ctype<_Ch> >(__loc); 
  const ctype<_Ch>& _Ct = *(ctype<_Ch>*)__f._M_ctype_facet(); 
  while (__pat != __pat_end) {
    char __c = _Ct.narrow(*__pat, 0);
    if (__c == '%') {
      char __mod = 0;
      ++__pat;
      __c = _Ct.narrow(*__pat++, 0);
      if(__c == '#') { // MS extension
        __mod = __c;
        __c = _Ct.narrow(*__pat++, 0);
      }
      __s = do_put(__s, __f, __fill, __tmb, __c, __mod);
    }
    else
      *__s++ = *__pat++;
  }
  return __s;
}

template<class _Ch, class _OutputIter>
_OutputIter
time_put<_Ch,_OutputIter>::do_put(_OutputIter __s, ios_base& __f, _Ch     /* __fill */ ,
				  const tm* __tmb,
				  char __format, char __modifier ) const 
{
  char __buf[64];
  char * __iend = __write_formatted_time(__buf, __format, __modifier,
					 _M_timeinfo, __tmb);
  //  locale __loc = __f.getloc();
  return __put_time(__buf, __iend, __s, __f, _Ch());
}

_STLP_END_NAMESPACE

# endif /* defined (_STLP_EXPOSE_STREAM_IMPLEMENTATION) */
#endif /* _STLP_TIME_FACETS_C */

// Local Variables:
// mode:C++
// End:
