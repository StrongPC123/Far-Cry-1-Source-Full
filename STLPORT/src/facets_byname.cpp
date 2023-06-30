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

#include <hash_map>
#include "locale_impl.h"
#include "c_locale.h"

#include "locale_nonclassic.h"


#include <stl/_codecvt.h>
#include <stl/_collate.h>
#include <stl/_ctype.h>
#include <stl/_monetary.h>
#include <stl/_time_facets.h>
#include <stl/_messages_facets.h>
#include <stl/_istream.h>
#include <stl/_num_get.h>
#include <stl/_num_put.h>

#include <algorithm>
// #include <stl/_ctype.h>
#include <stl/_function.h>
#include "c_locale.h"

_STLP_BEGIN_NAMESPACE

_Locale_ctype* __acquire_ctype(const char* name); 
void __release_ctype(_Locale_ctype* cat);


//----------------------------------------------------------------------
// ctype_byname<char>

ctype_byname<char>::ctype_byname(const char* name, size_t refs)
  : ctype<char>(_M_byname_table+1, false, refs),
  _M_ctype(__acquire_ctype(name))
{
  
  if (!_M_ctype)
    locale::_M_throw_runtime_error();
  
  // We have to do this, instead of just pointer twiddling, because
  // ctype_base::mask isn't the same type as _Locale_mask_t.  

  const _Locale_mask_t* p = _Locale_ctype_table(_M_ctype);

   if (!p)
     locale::_M_throw_runtime_error(); 

  for (size_t i = 0; i < table_size + 1; ++i) {
    _Locale_mask_t __m = p[i];
    if (__m & (upper | lower))
      __m |= alpha;
    _M_byname_table[i] = ctype_base::mask(__m);
  }
}

ctype_byname<char>::~ctype_byname()
{
  __release_ctype(_M_ctype);
}

char ctype_byname<char>::do_toupper(char c) const 
{
  return _Locale_toupper(_M_ctype, c);
}

char ctype_byname<char>::do_tolower(char c) const 
{
  return _Locale_tolower(_M_ctype, c);
}

const char*
ctype_byname<char>::do_toupper(char* first, const char* last) const
{
  for ( ; first != last ; ++first) 
    *first = _Locale_toupper(_M_ctype, *first);
  return last;
}

const char*
ctype_byname<char>::do_tolower(char* first, const char* last) const
{
  for ( ; first != last ; ++first) 
    *first = _Locale_tolower(_M_ctype, *first);
  return last;
}


// Some helper functions used in ctype<>::scan_is and scan_is_not.

# ifndef _STLP_NO_WCHAR_T

// ctype_byname<wchar_t>

  struct _Ctype_byname_w_is_mask {
    typedef wchar_t argument_type;
    typedef bool    result_type;

    /* ctype_base::mask*/ int  M;
    _Locale_ctype* M_ctp;

    _Ctype_byname_w_is_mask(/* ctype_base::mask */ int m, _Locale_ctype* c) : M((int)m), M_ctp(c) {}
    bool operator()(wchar_t c) const
      { return (M & _Locale_wchar_ctype(M_ctp, c, M)) != 0; }
  };

ctype_byname<wchar_t>::ctype_byname(const char* name, size_t refs)
  : ctype<wchar_t>(refs),
    _M_ctype(__acquire_ctype(name))
{
  if (!_M_ctype)
    locale::_M_throw_runtime_error();    
}

ctype_byname<wchar_t>::~ctype_byname()
{
  __release_ctype(_M_ctype);
}

bool ctype_byname<wchar_t>::do_is(ctype_base::mask  m, wchar_t c) const
{
  return (m & _Locale_wchar_ctype(_M_ctype, c, m)) != 0;
}

const wchar_t*
ctype_byname<wchar_t>::do_is(const wchar_t* low, const wchar_t* high,
                             ctype_base::mask * m) const
{
  ctype_base::mask all_bits = ctype_base::mask(
    ctype_base::space |
    ctype_base::print |
    ctype_base::cntrl |
    ctype_base::upper |
    ctype_base::lower |
    ctype_base::alpha |
    ctype_base::digit |
    ctype_base::punct |
    ctype_base::xdigit);

  for ( ; low < high; ++low, ++m)
    *m = ctype_base::mask (_Locale_wchar_ctype(_M_ctype, *low, all_bits));
  return high;
}

    
const wchar_t*
ctype_byname<wchar_t>
  ::do_scan_is(ctype_base::mask  m, const wchar_t* low, const wchar_t* high) const
{
  return find_if(low, high, _Ctype_byname_w_is_mask(m, _M_ctype));
}

const wchar_t*
ctype_byname<wchar_t>
  ::do_scan_not(ctype_base::mask  m, const wchar_t* low, const wchar_t* high) const
{
  return find_if(low, high, not1(_Ctype_byname_w_is_mask(m, _M_ctype)));
}

wchar_t ctype_byname<wchar_t>::do_toupper(wchar_t c) const 
{
  return _Locale_wchar_toupper(_M_ctype, c);
}

const wchar_t* 
ctype_byname<wchar_t>::do_toupper(wchar_t* low, const wchar_t* high) const
{
  for ( ; low < high; ++low)
    *low = _Locale_wchar_toupper(_M_ctype, *low);
  return high;
}

wchar_t ctype_byname<wchar_t>::do_tolower(wchar_t c) const 
{
  return _Locale_wchar_tolower(_M_ctype, c);
}

const wchar_t* 
ctype_byname<wchar_t>::do_tolower(wchar_t* low, const wchar_t* high) const
{
  for ( ; low < high; ++low)
    *low = _Locale_wchar_tolower(_M_ctype, *low);
  return high;
}

# endif /* WCHAR_T */

_STLP_END_NAMESPACE


// # include "collate_byname.cpp"

#include "stl/_collate.h"
#include "c_locale.h"
#include <vector>

_STLP_BEGIN_NAMESPACE

// collate_byname<char>
_Locale_collate* __acquire_collate(const char* name);
void __release_collate(_Locale_collate* cat);

collate_byname<char>::collate_byname(const char* name, size_t refs)
  : collate<char>(refs),
    _M_collate(__acquire_collate(name))
{
  if (!_M_collate)
    locale::_M_throw_runtime_error();
}

collate_byname<char>::~collate_byname()
{
  __release_collate(_M_collate);
}

int collate_byname<char>::do_compare(const char* __low1,
                                     const char* __high1,
                                     const char* __low2,
                                     const char* __high2) const {
  return _Locale_strcmp(_M_collate,
                        __low1, __high1 - __low1, 
                        __low2, __high2 - __low2);
}

collate_byname<char>::string_type
collate_byname<char>::do_transform(const char* low, const char* high) const {
  size_t n = _Locale_strxfrm(_M_collate,
                             NULL, 0,
                             low, high - low);

  __vector__<char, allocator<char> > buf(n);
  _Locale_strxfrm(_M_collate, &buf.front(), n,
                              low, high - low);

   char& __c1 = *(buf.begin());
   char& __c2 = (n == (size_t)-1) ? *(buf.begin() + (high-low-1)) : *(buf.begin() + n);
   //   char& __c2 = *(buf.begin() + n);
   return string_type( &__c1, &__c2 );
}


# ifndef _STLP_NO_WCHAR_T

// collate_byname<wchar_t>

collate_byname<wchar_t>::collate_byname(const char* name, size_t refs)
  : collate<wchar_t>(refs),
    _M_collate(__acquire_collate(name))
{
  if (!_M_collate)
    locale::_M_throw_runtime_error();
}

collate_byname<wchar_t>::~collate_byname() 
{
  __release_collate(_M_collate);
}

int collate_byname<wchar_t>::do_compare(const wchar_t* low1,
                                        const wchar_t* high1,
                                        const wchar_t* low2,
                                        const wchar_t* high2) const
{
  return _Locale_strwcmp(_M_collate,
                         low1, high1 - low1, 
                         low2, high2 - low2);
}

collate_byname<wchar_t>::string_type
collate_byname<wchar_t>
  ::do_transform(const wchar_t* low, const wchar_t* high) const
{
  size_t n = _Locale_strwxfrm(_M_collate,
                              NULL, 0,
                              low, high - low);

  __vector__<wchar_t, allocator<wchar_t> > buf(high - low);
  _Locale_strwxfrm(_M_collate, &buf.front(), n,
                               low, high - low);
  wchar_t& __c1 = *(buf.begin());
  wchar_t& __c2 = (n == (size_t)-1) ? *(buf.begin() + (high-low-1)) : *(buf.begin() + n);
  // wchar_t& __c2 = *(buf.begin() + n);
  return string_type( &__c1, &__c2 );
}

# endif /*  _STLP_NO_WCHAR_T */

_STLP_END_NAMESPACE

# ifndef _STLP_NO_MBSTATE_T

#include <stl/_codecvt.h>
#include <stl/_algobase.h>
#include "c_locale.h"

_STLP_BEGIN_NAMESPACE


//----------------------------------------------------------------------
// codecvt_byname<char>

codecvt_byname<char, char, mbstate_t>
  ::codecvt_byname(const char* /* name */, size_t refs)
    : codecvt<char, char, mbstate_t>(refs)
{}

codecvt_byname<char, char, mbstate_t>::~codecvt_byname() {}


# ifndef _STLP_NO_WCHAR_T

//----------------------------------------------------------------------
// codecvt_byname<wchar_t>

_Locale_ctype* __acquire_ctype(const char* name);
void __release_ctype(_Locale_ctype* cat);

codecvt_byname<wchar_t, char, mbstate_t>
  ::codecvt_byname(const char* name, size_t refs)
    : codecvt<wchar_t, char, mbstate_t>(refs),
      _M_ctype(__acquire_ctype(name))
{
  if (!_M_ctype)
    locale::_M_throw_runtime_error();
}

codecvt_byname<wchar_t, char, mbstate_t>::~codecvt_byname()
{
  __release_ctype(_M_ctype);
}

codecvt<wchar_t, char, mbstate_t>::result
codecvt_byname<wchar_t, char, mbstate_t>
  ::do_out(state_type&     state,
           const wchar_t*  from,
           const wchar_t*  from_end,
           const wchar_t*& from_next,
           char*           to,
           char*           to_limit,
           char*&          to_next) const
{
  while (from != from_end) {
    size_t chars_stored = _Locale_wctomb(_M_ctype,
                                         to, to_limit - to, *from,
                                         &state);
    if (chars_stored == (size_t) -1) {
      from_next = from;
      to_next   = to;
      return error;
    }

    else if (chars_stored == (size_t) -2) {
      from_next = from;
      to_next   = to;
      return partial;
    }

    ++from;
    to += chars_stored;
  }

  from_next = from;
  to_next   = to;
  return ok;
}

codecvt<wchar_t, char, mbstate_t>::result
codecvt_byname<wchar_t, char, mbstate_t>
  ::do_in(state_type&         state,
          const extern_type*  from,
          const extern_type*  from_end,
          const extern_type*& from_next,
          intern_type*        to,
          intern_type*        ,
          intern_type*&       to_next) const
{
  while (from != from_end) {
    size_t chars_read = _Locale_mbtowc(_M_ctype,
                                       to, from, from_end - from,
                                       &state);
    if (chars_read == (size_t) -1) {
      from_next = from;
      to_next   = to;
      return error;
    }

    if (chars_read == (size_t) -2) {
      from_next = from;
      to_next   = to;
      return partial;
    }

    from += chars_read;
    to++;
  }

  from_next = from;
  to_next   = to;
  return ok;
}

codecvt<wchar_t, char, mbstate_t>::result
codecvt_byname<wchar_t, char, mbstate_t>
  ::do_unshift(state_type&   state,
               extern_type*  to,
               extern_type*  to_limit,
               extern_type*& to_next) const
{
  to_next = to;
  size_t result = _Locale_unshift(_M_ctype, &state,
                                  to, to_limit - to, &to_next);
  if (result == (size_t) -1)
    return error;
  else if (result == (size_t) -2)
    return partial;
  else
#ifdef __ISCPP__
    return /*to_next == to ? noconv :*/ ok;
#else
    return to_next == to ? noconv : ok;
#endif
}

int
codecvt_byname<wchar_t, char, mbstate_t>::do_encoding() const _STLP_NOTHROW
{
  if (_Locale_is_stateless(_M_ctype)) {
    int max_width = _Locale_mb_cur_max(_M_ctype);
    int min_width = _Locale_mb_cur_min(_M_ctype);
    return min_width == max_width ? min_width : 0;
  }
  else
    return -1;
}


bool codecvt_byname<wchar_t, char, mbstate_t>
  ::do_always_noconv() const _STLP_NOTHROW
{
  return false;
}

int 
codecvt_byname<wchar_t, char, mbstate_t>::do_length(
                                                    const state_type&,
                                                    const  extern_type* from, const  extern_type* end,
                                                    size_t mx) const 
{
  return (int)(min) ((size_t) (end - from), mx);
}

int
codecvt_byname<wchar_t, char, mbstate_t>::do_max_length() const _STLP_NOTHROW
{
  return _Locale_mb_cur_max(_M_ctype);
}
# endif /* WCHAR_T */

_STLP_END_NAMESPACE

# endif /* MBSTATE_T */

#include "locale_impl.h"
# include <stl/_numpunct.h>

_STLP_BEGIN_NAMESPACE

_Locale_numeric*  _STLP_CALL __acquire_numeric(const char* name);
void  _STLP_CALL __release_numeric(_Locale_numeric* cat);

// numpunct_byname<char>

numpunct_byname<char>::numpunct_byname(const char* name, size_t refs)
  : numpunct<char>(refs),
    _M_numeric(__acquire_numeric(name))
{
  if (!_M_numeric)
    locale::_M_throw_runtime_error();

  _M_truename  = _Locale_true(_M_numeric);
  _M_falsename = _Locale_false(_M_numeric);
}

numpunct_byname<char>::~numpunct_byname()
{
  __release_numeric(_M_numeric);
}

char numpunct_byname<char>::do_decimal_point() const {
  return _Locale_decimal_point(_M_numeric);
}

char numpunct_byname<char>::do_thousands_sep() const {
  return _Locale_thousands_sep(_M_numeric);
}

string numpunct_byname<char>::do_grouping() const {
  const char * __grouping = _Locale_grouping(_M_numeric);
  if (__grouping != NULL && __grouping[0] == CHAR_MAX)
    __grouping = "";
  return __grouping;
}

//----------------------------------------------------------------------
// numpunct<wchar_t>

# ifndef _STLP_NO_WCHAR_T

// numpunct_byname<wchar_t> 

numpunct_byname<wchar_t>::numpunct_byname(const char* name, size_t refs)
  : numpunct<wchar_t>(refs),
    _M_numeric(__acquire_numeric(name))
{
  if (!_M_numeric)
    locale::_M_throw_runtime_error();

  const char* truename  = _Locale_true(_M_numeric);
  const char* falsename = _Locale_false(_M_numeric);
  _M_truename.resize(strlen(truename));
  _M_falsename.resize(strlen(falsename));
  copy(truename,  truename  + strlen(truename), _M_truename.begin());
  copy(falsename, falsename + strlen(falsename), _M_falsename.begin());
}

numpunct_byname<wchar_t>::~numpunct_byname()
{
  __release_numeric(_M_numeric);
}

wchar_t numpunct_byname<wchar_t>::do_decimal_point() const {
  return (wchar_t) _Locale_decimal_point(_M_numeric);
}

wchar_t numpunct_byname<wchar_t>::do_thousands_sep() const {
  return (wchar_t) _Locale_thousands_sep(_M_numeric);
}

string numpunct_byname<wchar_t>::do_grouping() const {
  const char * __grouping = _Locale_grouping(_M_numeric);
  if (__grouping != NULL && __grouping[0] == CHAR_MAX)
    __grouping = "";
  return __grouping;
}

# endif

_STLP_END_NAMESPACE

#include <stl/_monetary.h>
// #include <stl/_ostream.h>
// #include <stl/_istream.h>
#include "c_locale.h"


_STLP_BEGIN_NAMESPACE

static void _Init_monetary_formats(money_base::pattern& pos_format,
                                    money_base::pattern& neg_format,
                                    _Locale_monetary * monetary) {
  switch (_Locale_p_sign_posn(monetary)) {
    case 0: case 1:
      pos_format.field[0] = (char) money_base::sign;
      if (_Locale_p_cs_precedes(monetary)) {
	pos_format.field[1] = (char) money_base::symbol;
	if (_Locale_p_sep_by_space(monetary)) {
	  pos_format.field[2] = (char) money_base::space;
	  pos_format.field[3] = (char) money_base::value;
	}
	else {
	  pos_format.field[2] = (char) money_base::value;
	  pos_format.field[3] = (char) money_base::none;
	}
      }
      else {
	pos_format.field[2] = (char) money_base::value;
	if (_Locale_p_sep_by_space(monetary)) {
	  pos_format.field[2] = (char) money_base::space;
	  pos_format.field[3] = (char) money_base::symbol;
	}
	else {
	  pos_format.field[2] = (char) money_base::symbol;
	  pos_format.field[3] = (char) money_base::none;
	}
      }       
      break;
    case 2:
      if (_Locale_p_cs_precedes(monetary)) {
	pos_format.field[0] = (char) money_base::symbol;
	if (_Locale_p_sep_by_space(monetary)) {
	  pos_format.field[1] = (char) money_base::space;
	  pos_format.field[2] = (char) money_base::value;
	  pos_format.field[3] = (char) money_base::sign;
	}
	else {
	  pos_format.field[1] = (char) money_base::value;
	  pos_format.field[2] = (char) money_base::sign;
	  pos_format.field[3] = (char) money_base::none;
	}
      }
      else {
	pos_format.field[1] = (char) money_base::value;
	if (_Locale_p_sep_by_space(monetary)) {
	  pos_format.field[1] = (char) money_base::space;
	  pos_format.field[2] = (char) money_base::symbol;
	  pos_format.field[3] = (char) money_base::sign;
	}
	else {
	  pos_format.field[1] = (char) money_base::symbol;
	  pos_format.field[2] = (char) money_base::sign;
	  pos_format.field[3] = (char) money_base::none;
	}
      }
      break;
    case 3:
      if (_Locale_p_cs_precedes(monetary)) {
	pos_format.field[0] = (char) money_base::sign;
	pos_format.field[1] = (char) money_base::symbol;
	if (_Locale_p_sep_by_space(monetary)) {
	  pos_format.field[2] = (char) money_base::space;
	  pos_format.field[3] = (char) money_base::value;
	}
	else {
	  pos_format.field[2] = (char) money_base::value;
	  pos_format.field[3] = (char) money_base::none;
	}
      }
      else {
	pos_format.field[0] = (char) money_base::value;
	pos_format.field[1] = (char) money_base::sign;
	pos_format.field[2] = (char) money_base::symbol;
	pos_format.field[3] = (char) money_base::none;
      }
      break;
    case 4: default:
      if (_Locale_p_cs_precedes(monetary)) {
	pos_format.field[0] = (char) money_base::symbol;
	pos_format.field[1] = (char) money_base::sign;
	pos_format.field[2] = (char) money_base::value;
	pos_format.field[3] = (char) money_base::none;
      }
      else {
	pos_format.field[0] = (char) money_base::value;
	if (_Locale_p_sep_by_space(monetary)) {
	  pos_format.field[1] = (char) money_base::space;
	  pos_format.field[2] = (char) money_base::symbol;
	  pos_format.field[3] = (char) money_base::sign;
	}
        else {
	  pos_format.field[1] = (char) money_base::symbol;
	  pos_format.field[2] = (char) money_base::sign;
	  pos_format.field[3] = (char) money_base::none;
	}
      }
    break;
  }

  switch (_Locale_n_sign_posn(monetary)) {
    case 0: case 1:
      neg_format.field[0] = (char) money_base::sign;
      if (_Locale_n_cs_precedes(monetary)) {
        neg_format.field[1] = (char) money_base::symbol;
	if (_Locale_n_sep_by_space(monetary)) {
	  neg_format.field[2] = (char) money_base::space;
	  neg_format.field[3] = (char) money_base::value;
	}
	else {
	  neg_format.field[2] = (char) money_base::value;
	  neg_format.field[3] = (char) money_base::none;
	}
      }
      else {
	neg_format.field[2] = (char) money_base::value;
	if (_Locale_n_sep_by_space(monetary)) {
	  neg_format.field[2] = (char) money_base::space;
	  neg_format.field[3] = (char) money_base::symbol;
	}
	else {
	  neg_format.field[2] = (char) money_base::symbol;
	  neg_format.field[3] = (char) money_base::none;
	}
      }       
      break;
    case 2:
      if (_Locale_n_cs_precedes(monetary)) {
	neg_format.field[0] = (char) money_base::symbol;
	if (_Locale_n_sep_by_space(monetary)) {
	  neg_format.field[1] = (char) money_base::space;
	  neg_format.field[2] = (char) money_base::value;
	  neg_format.field[3] = (char) money_base::sign;
	}
	else {
	  neg_format.field[1] = (char) money_base::value;
	  neg_format.field[2] = (char) money_base::sign;
	  neg_format.field[3] = (char) money_base::none;
	}
      }
      else {
	neg_format.field[1] = (char) money_base::value;
	if (_Locale_n_sep_by_space(monetary)) {
	  neg_format.field[1] = (char) money_base::space;
	  neg_format.field[2] = (char) money_base::symbol;
	  neg_format.field[3] = (char) money_base::sign;
	}
	else {
	  neg_format.field[1] = (char) money_base::symbol;
	  neg_format.field[2] = (char) money_base::sign;
	  neg_format.field[3] = (char) money_base::none;
	}
      }
      break;
    case 3:
      if (_Locale_n_cs_precedes(monetary)) {
	neg_format.field[0] = (char) money_base::sign;
	neg_format.field[1] = (char) money_base::symbol;
	if (_Locale_n_sep_by_space(monetary)) {
	  neg_format.field[2] = (char) money_base::space;
	  neg_format.field[3] = (char) money_base::value;
	}
	else {
	  neg_format.field[2] = (char) money_base::value;
	  neg_format.field[3] = (char) money_base::none;
	}
      }
      else {
	neg_format.field[0] = (char) money_base::value;
	neg_format.field[1] = (char) money_base::sign;
	neg_format.field[2] = (char) money_base::symbol;
	neg_format.field[3] = (char) money_base::none;
      }
      break;
    case 4: default:
      if (_Locale_n_cs_precedes(monetary)) {
	neg_format.field[0] = (char) money_base::symbol;
	neg_format.field[1] = (char) money_base::sign;
	neg_format.field[2] = (char) money_base::value;
	neg_format.field[3] = (char) money_base::none;
      }
      else {
	neg_format.field[0] = (char) money_base::value;
	if (_Locale_n_sep_by_space(monetary)) {
	  neg_format.field[1] = (char) money_base::space;
	  neg_format.field[2] = (char) money_base::symbol;
	  neg_format.field[3] = (char) money_base::sign;
	}
        else {
	  neg_format.field[1] = (char) money_base::symbol;
	  neg_format.field[2] = (char) money_base::sign;
	  neg_format.field[3] = (char) money_base::none;
	}
      }
      break;
  }
}


//
// moneypunct_byname<>
//

_Locale_monetary* __acquire_monetary(const char* name);
void __release_monetary(_Locale_monetary* mon);

moneypunct_byname<char, true>::moneypunct_byname(const char * name,
						 size_t refs):
  moneypunct<char, true>(refs),
  _M_monetary(__acquire_monetary(name))
{
  if (!_M_monetary)
    locale::_M_throw_runtime_error();
  _Init_monetary_formats(_M_pos_format, _M_neg_format, _M_monetary);
}

moneypunct_byname<char, true>::~moneypunct_byname()
{
  __release_monetary(_M_monetary);
}

char moneypunct_byname<char, true>::do_decimal_point() const 
  {return _Locale_mon_decimal_point(_M_monetary);}

char moneypunct_byname<char, true>::do_thousands_sep() const
  {return _Locale_mon_thousands_sep(_M_monetary);}

string moneypunct_byname<char, true>::do_grouping() const
  {return _Locale_mon_grouping(_M_monetary);}

string moneypunct_byname<char, true>::do_curr_symbol() const
  {return _Locale_int_curr_symbol(_M_monetary);}

string moneypunct_byname<char, true>::do_positive_sign() const
  {return _Locale_positive_sign(_M_monetary);}

string moneypunct_byname<char, true>::do_negative_sign() const
  {return _Locale_negative_sign(_M_monetary);}

int moneypunct_byname<char, true>::do_frac_digits() const 
  {return _Locale_int_frac_digits(_M_monetary);}

moneypunct_byname<char, false>::moneypunct_byname(const char * name,
						  size_t refs):
  moneypunct<char, false>(refs),
  _M_monetary(__acquire_monetary(name))
{
  if (!_M_monetary)
    locale::_M_throw_runtime_error();
  _Init_monetary_formats(_M_pos_format, _M_neg_format, _M_monetary);
}

moneypunct_byname<char, false>::~moneypunct_byname()
{
  __release_monetary(_M_monetary);
}

char moneypunct_byname<char, false>::do_decimal_point() const
  {return _Locale_mon_decimal_point(_M_monetary);}

char moneypunct_byname<char, false>::do_thousands_sep() const
  {return _Locale_mon_thousands_sep(_M_monetary);}

string moneypunct_byname<char, false>::do_grouping() const
  {return _Locale_mon_grouping(_M_monetary);}

string moneypunct_byname<char, false>::do_curr_symbol() const
  {return _Locale_currency_symbol(_M_monetary);}

string moneypunct_byname<char, false>::do_positive_sign() const
  {return _Locale_positive_sign(_M_monetary);}

string moneypunct_byname<char, false>::do_negative_sign() const
  {return _Locale_negative_sign(_M_monetary);}

int moneypunct_byname<char, false>::do_frac_digits() const 
  {return _Locale_frac_digits(_M_monetary);}

//
// moneypunct_byname<wchar_t>
//
# ifndef _STLP_NO_WCHAR_T

moneypunct_byname<wchar_t, true>::moneypunct_byname(const char * name,
						 size_t refs):
  moneypunct<wchar_t, true>(refs),
  _M_monetary(__acquire_monetary(name))
{
  if (!_M_monetary)
    locale::_M_throw_runtime_error();
  _Init_monetary_formats(_M_pos_format, _M_neg_format, _M_monetary);
}

moneypunct_byname<wchar_t, true>::~moneypunct_byname() 
{
  __release_monetary(_M_monetary);
}

wchar_t moneypunct_byname<wchar_t, true>::do_decimal_point() const
  {return _Locale_mon_decimal_point(_M_monetary);}

wchar_t moneypunct_byname<wchar_t, true>::do_thousands_sep() const
  {return _Locale_mon_thousands_sep(_M_monetary);}

string moneypunct_byname<wchar_t, true>::do_grouping() const
  {return _Locale_mon_grouping(_M_monetary);}

wstring moneypunct_byname<wchar_t, true>::do_curr_symbol() const
{
  string str = _Locale_int_curr_symbol(_M_monetary);
# if defined (_STLP_NO_MEMBER_TEMPLATES) || defined (_STLP_MSVC) || defined(__MRC__) || defined(__SC__)		//*ty 05/26/2001 - added workaround for mpw
  wstring result(wstring::_Reserve_t(), str.size());
  copy(str.begin(), str.end(), result.begin());
# else
  wstring result(str.begin(), str.end());
# endif
  return result;
}

wstring moneypunct_byname<wchar_t, true>::do_positive_sign() const
{
  string str = _Locale_positive_sign(_M_monetary);
# if defined (_STLP_NO_MEMBER_TEMPLATES) || defined (_STLP_MSVC) || defined(__MRC__) || defined(__SC__)		//*ty 05/26/2001 - added workaround for mpw
  wstring result(wstring::_Reserve_t(), str.size());
  copy(str.begin(), str.end(), result.begin());
# else
  wstring result(str.begin(), str.end());
# endif
  return result;
}


wstring moneypunct_byname<wchar_t, true>::do_negative_sign() const
{
  string str = _Locale_negative_sign(_M_monetary);
# if defined (_STLP_NO_MEMBER_TEMPLATES) || defined (_STLP_MSVC)  || defined(__MRC__) || defined(__SC__)		//*ty 05/26/2001 - added workaround for mpw
  wstring result(wstring::_Reserve_t(), str.size());
  copy(str.begin(), str.end(), result.begin());
# else
  wstring result(str.begin(), str.end());
# endif
  return result;
}

int moneypunct_byname<wchar_t, true>::do_frac_digits() const 
  {return _Locale_int_frac_digits(_M_monetary);}

moneypunct_byname<wchar_t, false>::moneypunct_byname(const char * name,
						 size_t refs):
  moneypunct<wchar_t, false>(refs),
  _M_monetary(__acquire_monetary(name))
{
  if (!_M_monetary)
    locale::_M_throw_runtime_error() ;
  _Init_monetary_formats(_M_pos_format, _M_neg_format, _M_monetary);
}

moneypunct_byname<wchar_t, false>::~moneypunct_byname()
{
  __release_monetary(_M_monetary);
}

wchar_t moneypunct_byname<wchar_t, false>::do_decimal_point() const
  {return _Locale_mon_decimal_point(_M_monetary);}

wchar_t moneypunct_byname<wchar_t, false>::do_thousands_sep() const
  {return _Locale_mon_thousands_sep(_M_monetary);}

string moneypunct_byname<wchar_t, false>::do_grouping() const
  {return _Locale_mon_grouping(_M_monetary);}

wstring moneypunct_byname<wchar_t, false>::do_curr_symbol() const
{
  string str =  _Locale_currency_symbol(_M_monetary);
# if defined (_STLP_NO_MEMBER_TEMPLATES) || defined (_STLP_MSVC) || defined(__MRC__) || defined(__SC__)		//*ty 05/26/2001 - added workaround for mpw
  wstring result(wstring::_Reserve_t(), str.size());
  copy(str.begin(), str.end(), result.begin());
# else
  wstring result(str.begin(), str.end());
# endif
  return result;
}

wstring moneypunct_byname<wchar_t, false>::do_positive_sign() const
{
  string str = _Locale_positive_sign(_M_monetary);
# if defined (_STLP_NO_MEMBER_TEMPLATES) || defined (_STLP_MSVC) || defined(__MRC__) || defined(__SC__)		//*ty 05/26/2001 - added workaround for mpw
  wstring result(wstring::_Reserve_t(), str.size());
  copy(str.begin(), str.end(), result.begin());
# else
  wstring result(str.begin(), str.end());
# endif
  return result;
}

wstring moneypunct_byname<wchar_t, false>::do_negative_sign() const
{
  string str = _Locale_negative_sign(_M_monetary);
# if defined (_STLP_NO_MEMBER_TEMPLATES) || defined (_STLP_MSVC) || defined(__MRC__) || defined(__SC__)		//*ty 05/26/2001 - added workaround for mpw
  wstring result(wstring::_Reserve_t(), str.size());
  copy(str.begin(), str.end(), result.begin());
# else
  wstring result(str.begin(), str.end());
# endif
  return result;
}

int moneypunct_byname<wchar_t, false>::do_frac_digits() const 
  {return _Locale_frac_digits(_M_monetary);}

# endif

_STLP_END_NAMESPACE  

#include <stl/_messages_facets.h>
#include "message_facets.h"
#include <typeinfo>

_STLP_BEGIN_NAMESPACE

void _Catalog_locale_map::insert(int key, const locale& L)
{
# ifdef _STLP_NO_WCHAR_T
  typedef char _Char;
# else
  typedef wchar_t _Char;
# endif
#if !defined(_STLP_NO_TYPEINFO)
  // Don't bother to do anything unless we're using a non-default ctype facet
  _STLP_TRY {
    typedef ctype<_Char> wctype;
    wctype& wct = (wctype &)use_facet<wctype>(L);
    wctype* zz;
    if (typeid(&wct) != typeid(zz)) {
      if (!M)
        M = new hash_map<int, locale, hash<int>, equal_to<int> >;

#if defined(__SC__)
      if (!M) delete M;
#endif
      if (M->find(key) == M->end())
        M->insert(pair<const int, locale>(key, L));
    }
  }
  _STLP_CATCH_ALL {}
# endif /* _STLP_NO_TYPEINFO */
}

void _Catalog_locale_map::erase(int key)
{
  if (M)
    M->erase(key);
}

locale _Catalog_locale_map::lookup(int key) const
{
  if (M) {
    hash_map<int, locale, hash<int>, equal_to<int> >::iterator i = M->find(key);
    return i != M->end() ? (*i).second : locale::classic();
  }
  else
    return locale::classic();
}


//----------------------------------------------------------------------
//
//

_Messages_impl::_Messages_impl(bool is_wide) : 
  _M_message_obj(0), _M_map(0)
{ 
  _M_delete = true;
  if (is_wide) 
    _M_map = new _Catalog_locale_map;
  _M_message_obj = __acquire_messages("C");
}

_Messages_impl::_Messages_impl(bool is_wide, _Locale_messages* msg_obj ) : 
  _M_message_obj(msg_obj), _M_map(0)
{ 
  _M_delete = true;
  if (is_wide) 
    _M_map = new _Catalog_locale_map;
}

_Messages_impl::~_Messages_impl()
{
  __release_messages(_M_message_obj);
  if (_M_map) delete _M_map;
}

int _Messages_impl::do_open(const string& filename, const locale& L) const
{  
  int result = _M_message_obj
    ? _Locale_catopen(_M_message_obj, filename.c_str())
    : -1;

  if (result >= 0 && _M_map != 0)
    _M_map->insert(result, L);

  return result;
}

string _Messages_impl::do_get(catalog cat,
                              int set, int p_id, const string& dfault) const
{
  return _M_message_obj != 0 && cat >= 0
    ? string(_Locale_catgets(_M_message_obj, cat, set, p_id, dfault.c_str()))
    : dfault;
}

# ifndef _STLP_NO_WCHAR_T

wstring
_Messages_impl::do_get(catalog thecat,
		       int set, int p_id, const wstring& dfault) const
{
  typedef ctype<wchar_t> wctype;
  const wctype& ct = use_facet<wctype>(_M_map->lookup(thecat));

  const char* str = _Locale_catgets(_M_message_obj, thecat, set, p_id, "");

  // Verify that the lookup failed; an empty string might represent success.
  if (!str)
    return dfault;
  else if (str[0] == '\0') {
    const char* str2 = _Locale_catgets(_M_message_obj, thecat, set, p_id, "*");
    if (!str2 || strcmp(str2, "*") == 0)
      return dfault;
  }

  // str is correct.  Now we must widen it to get a wstring.
  size_t n = strlen(str);

  // NOT PORTABLE.  What we're doing relies on internal details of the 
  // string implementation.  (Contiguity of string elements.)
  wstring result(n, wchar_t(0));
  ct.widen(str, str + n, &*result.begin());
  return result;
}

# endif

void _Messages_impl::do_close(catalog thecat) const
{
  if (_M_message_obj)
    _Locale_catclose(_M_message_obj, thecat);
  if (_M_map) _M_map->erase(thecat);
}


//----------------------------------------------------------------------
// messages<char>

messages<char>::messages(size_t refs)  : 
  _BaseFacet(refs), _M_impl(new _Messages_impl(false))
{}

messages<char>::messages(size_t refs, _Locale_messages* msg_obj) : _BaseFacet(refs), 
  _M_impl(new _Messages_impl(false, msg_obj))
{}


//----------------------------------------------------------------------
// messages_byname<char>

messages_byname<char>::messages_byname(const char* name, size_t refs)
  : messages<char>(refs, name ? __acquire_messages(name) : 0)
{}

messages_byname<char>::~messages_byname()
{}

# ifndef _STLP_NO_WCHAR_T

//----------------------------------------------------------------------
// messages<wchar_t>

messages<wchar_t>::messages(size_t refs)  : 
  _BaseFacet(refs), _M_impl(new _Messages_impl(true))
{}

messages<wchar_t>::messages(size_t refs, _Locale_messages* msg_obj)
  : _BaseFacet(refs),
    _M_impl(new _Messages_impl(true, msg_obj))
{}

//----------------------------------------------------------------------
// messages_byname<wchar_t>


messages_byname<wchar_t>::messages_byname(const char* name, size_t refs)
  : messages<wchar_t>(refs, name ? __acquire_messages(name) : 0)
{}

messages_byname<wchar_t>::~messages_byname()
{}

# endif

_STLP_END_NAMESPACE

