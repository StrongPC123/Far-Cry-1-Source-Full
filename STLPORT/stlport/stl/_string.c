/*
 *
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Copyright (c) 1997
 * Moscow Center for SPARC Technology
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
#ifndef _STLP_STRING_C
#define _STLP_STRING_C

#ifndef _STLP_STRING_H
# include <stl/_string.h>
#endif

# ifdef _STLP_DEBUG
#  define basic_string _Nondebug_string
# endif

# if defined (_STLP_USE_OWN_NAMESPACE) || !defined (_STLP_USE_NATIVE_STRING)

# if defined (_STLP_NESTED_TYPE_PARAM_BUG)
#  define __size_type__ size_t
#  define size_type size_t
#  define iterator   _CharT*
# else
#  define __size_type__ _STLP_TYPENAME_ON_RETURN_TYPE basic_string<_CharT,_Traits,_Alloc>::size_type
# endif

_STLP_BEGIN_NAMESPACE

// ------------------------------------------------------------
// Non-inline declarations.


// Change the string's capacity so that it is large enough to hold
//  at least __res_arg elements, plus the terminating _CharT().  Note that,
//  if __res_arg < capacity(), this member function may actually decrease
//  the string's capacity.
template <class _CharT, class _Traits, class _Alloc> void basic_string<_CharT,_Traits,_Alloc>::reserve(size_type __res_arg) {

  if (__res_arg >= capacity())
    {      
      if (__res_arg > max_size())
	this->_M_throw_length_error();

      size_type __n = __res_arg + 1;
      pointer __new_start = this->_M_end_of_storage.allocate(__n);
      pointer __new_finish = __new_start;
      
      _STLP_TRY {
	__new_finish = uninitialized_copy(this->_M_start, this->_M_finish, __new_start);
	_M_construct_null(__new_finish);
      }
      _STLP_UNWIND((_STLP_STD::_Destroy(__new_start, __new_finish), 
		    this->_M_end_of_storage.deallocate(__new_start, __n)));
      
      _STLP_STD::_Destroy(this->_M_start, this->_M_finish + 1);
      this->_M_deallocate_block();
      this->_M_start = __new_start;
      this->_M_finish = __new_finish;
      this->_M_end_of_storage._M_data = __new_start + __n;
    }
}

template <class _CharT, class _Traits, class _Alloc> basic_string<_CharT,_Traits,_Alloc>& basic_string<_CharT,_Traits,_Alloc>::append(size_type __n, _CharT __c) {
  if (__n > max_size() || size() > max_size() - __n)
    this->_M_throw_length_error();
  if (size() + __n > capacity())
    reserve(size() + (max)(size(), __n));
  if (__n > 0) {
    uninitialized_fill_n(this->_M_finish + 1, __n - 1, __c);
    _STLP_TRY {
      _M_construct_null(this->_M_finish + __n);
    }
    _STLP_UNWIND(_STLP_STD::_Destroy(this->_M_finish + 1, this->_M_finish + __n));
    _Traits::assign(*end(), __c);
    this->_M_finish += __n;
  }
  return *this;
}

#ifndef _STLP_MEMBER_TEMPLATES

template <class _CharT, class _Traits, class _Alloc> basic_string<_CharT, _Traits, _Alloc>& basic_string<_CharT, _Traits, _Alloc>::append(const _CharT* __first,
					      const _CharT* __last)
{
  if (__first != __last) {
    const size_type __old_size = size();
    ptrdiff_t __n = __last - __first;
    if ((size_type)__n > max_size() || __old_size > max_size() - __n)
      this->_M_throw_length_error();
    if (__old_size + __n > capacity()) {
      const size_type __len = __old_size + (max)(__old_size, (size_t) __n) + 1;
      pointer __new_start = this->_M_end_of_storage.allocate(__len);
      pointer __new_finish = __new_start;
      _STLP_TRY {
        __new_finish = uninitialized_copy(this->_M_start, this->_M_finish, __new_start);
        __new_finish = uninitialized_copy(__first, __last, __new_finish);
        _M_construct_null(__new_finish);
      }
      _STLP_UNWIND((_STLP_STD::_Destroy(__new_start,__new_finish),
                    this->_M_end_of_storage.deallocate(__new_start,__len)));
      _STLP_STD::_Destroy(this->_M_start, this->_M_finish + 1);
      this->_M_deallocate_block();
      this->_M_start = __new_start;
      this->_M_finish = __new_finish;
      this->_M_end_of_storage._M_data = __new_start + __len; 
    }
    else {
      const _CharT* __f1 = __first;
      ++__f1;
      uninitialized_copy(__f1, __last, this->_M_finish + 1);
      _STLP_TRY {
        _M_construct_null(this->_M_finish + __n);
      }
      _STLP_UNWIND(_STLP_STD::_Destroy(this->_M_finish + 1, this->_M_finish + __n));
      _Traits::assign(*end(), *__first);
      this->_M_finish += __n;
    }
  }
  return *this;  
}

#endif /* _STLP_MEMBER_TEMPLATES */

template <class _CharT, class _Traits, class _Alloc> 
basic_string<_CharT,_Traits,_Alloc>& 
basic_string<_CharT,_Traits,_Alloc>::assign(size_type __n, _CharT __c) {
  if (__n <= size()) {
    _Traits::assign(this->_M_start, __n, __c);
    erase(begin() + __n, end());
  }
  else {
    _Traits::assign(this->_M_start, size(), __c);
    append(__n - size(), __c);
  }
  return *this;
}

template <class _CharT, class _Traits, class _Alloc> _CharT* 
basic_string<_CharT,_Traits,_Alloc> ::_M_insert_aux(_CharT* __p,
                  _CharT __c)
{
  pointer __new_pos = __p;
  if (this->_M_finish + 1 < this->_M_end_of_storage._M_data) {
    _M_construct_null(this->_M_finish + 1);
    _Traits::move(__p + 1, __p, this->_M_finish - __p);
    _Traits::assign(*__p, __c);
    ++this->_M_finish;
  }
  else {
    const size_type __old_len = size();
    const size_type __len = __old_len +
                            (max)(__old_len, __STATIC_CAST(size_type,1)) + 1;
    pointer __new_start = this->_M_end_of_storage.allocate(__len);
    pointer __new_finish = __new_start;
    _STLP_TRY {
      __new_pos = uninitialized_copy(this->_M_start, __p, __new_start);
      _Construct(__new_pos, __c);
      __new_finish = __new_pos + 1;
      __new_finish = uninitialized_copy(__p, this->_M_finish, __new_finish);
      _M_construct_null(__new_finish);
    }
    _STLP_UNWIND((_STLP_STD::_Destroy(__new_start,__new_finish), 
                  this->_M_end_of_storage.deallocate(__new_start,__len)));
    _STLP_STD::_Destroy(this->_M_start, this->_M_finish + 1);
    this->_M_deallocate_block();
    this->_M_start = __new_start;
    this->_M_finish = __new_finish;
    this->_M_end_of_storage._M_data = __new_start + __len;
  }
  return __new_pos;
}

template <class _CharT, class _Traits, class _Alloc> void basic_string<_CharT,_Traits,_Alloc>::insert(iterator __position,
           size_t __n, _CharT __c)
{
  if (__n != 0) {
    if (size_type(this->_M_end_of_storage._M_data - this->_M_finish) >= __n + 1) {
      const size_type __elems_after = this->_M_finish - __position;
      pointer __old_finish = this->_M_finish;
      if (__elems_after >= __n) {
        uninitialized_copy((this->_M_finish - __n) + 1, this->_M_finish + 1,
                           this->_M_finish + 1);
        this->_M_finish += __n;
        _Traits::move(__position + __n,
                      __position, (__elems_after - __n) + 1);
        _Traits::assign(__position, __n, __c);
      }
      else {
        uninitialized_fill_n(this->_M_finish + 1, __n - __elems_after - 1, __c);
        this->_M_finish += __n - __elems_after;
        _STLP_TRY {
          uninitialized_copy(__position, __old_finish + 1, this->_M_finish);
          this->_M_finish += __elems_after;
        }
        _STLP_UNWIND((_STLP_STD::_Destroy(__old_finish + 1, this->_M_finish), 
                      this->_M_finish = __old_finish));
        _Traits::assign(__position, __elems_after + 1, __c);
      }
    }
    else {
      const size_type __old_size = size();        
      const size_type __len = __old_size + (max)(__old_size, __n) + 1;
      pointer __new_start = this->_M_end_of_storage.allocate(__len);
      pointer __new_finish = __new_start;
      _STLP_TRY {
        __new_finish = uninitialized_copy(this->_M_start, __position, __new_start);
        __new_finish = uninitialized_fill_n(__new_finish, __n, __c);
        __new_finish = uninitialized_copy(__position, this->_M_finish,
                                          __new_finish);
        _M_construct_null(__new_finish);
      }
      _STLP_UNWIND((_STLP_STD::_Destroy(__new_start,__new_finish),
                    this->_M_end_of_storage.deallocate(__new_start,__len)));
      _STLP_STD::_Destroy(this->_M_start, this->_M_finish + 1);
      this->_M_deallocate_block();
      this->_M_start = __new_start;
      this->_M_finish = __new_finish;
      this->_M_end_of_storage._M_data = __new_start + __len;    
    }
  }
}

#ifndef _STLP_MEMBER_TEMPLATES

template <class _CharT, class _Traits, class _Alloc> void 
basic_string<_CharT,_Traits,_Alloc>::insert(iterator __position,
                                            const _CharT* __first, 
                                            const _CharT* __last)
{
  if (__first != __last) {
    const ptrdiff_t __n = __last - __first;
    if (this->_M_end_of_storage._M_data - this->_M_finish >= __n + 1) {
      const ptrdiff_t __elems_after = this->_M_finish - __position;
      pointer __old_finish = this->_M_finish;
      if (__elems_after >= __n) {
        uninitialized_copy((this->_M_finish - __n) + 1, this->_M_finish + 1,
                           this->_M_finish + 1);
        this->_M_finish += __n;
        _Traits::move(__position + __n,
                      __position, (__elems_after - __n) + 1);
        _M_copy(__first, __last, __position);
      }
      else {
        const _CharT* __mid = __first;
        advance(__mid, __elems_after + 1);
        uninitialized_copy(__mid, __last, this->_M_finish + 1);
        this->_M_finish += __n - __elems_after;
        _STLP_TRY {
          uninitialized_copy(__position, __old_finish + 1, this->_M_finish);
          this->_M_finish += __elems_after;
        }
        _STLP_UNWIND((_STLP_STD::_Destroy(__old_finish + 1, this->_M_finish), 
                      this->_M_finish = __old_finish));
        _M_copy(__first, __mid, __position);
      }
    }
    else {
      size_type __old_size = size();        
      size_type __len
        = __old_size + (max)(__old_size, __STATIC_CAST(const size_type,__n)) + 1;
      pointer __new_start = this->_M_end_of_storage.allocate(__len);
      pointer __new_finish = __new_start;
      _STLP_TRY {
        __new_finish = uninitialized_copy(this->_M_start, __position, __new_start);
        __new_finish = uninitialized_copy(__first, __last, __new_finish);
        __new_finish
          = uninitialized_copy(__position, this->_M_finish, __new_finish);
        _M_construct_null(__new_finish);
      }
      _STLP_UNWIND((_STLP_STD::_Destroy(__new_start,__new_finish),
                    this->_M_end_of_storage.deallocate(__new_start,__len)));
      _STLP_STD::_Destroy(this->_M_start, this->_M_finish + 1);
      this->_M_deallocate_block();
      this->_M_start = __new_start;
      this->_M_finish = __new_finish;
      this->_M_end_of_storage._M_data = __new_start + __len; 
    }
  }
}

#endif /* _STLP_MEMBER_TEMPLATES */

template <class _CharT, class _Traits, class _Alloc> basic_string<_CharT,_Traits,_Alloc>& basic_string<_CharT,_Traits,_Alloc> ::replace(iterator __first, iterator __last, size_type __n, _CharT __c)
{
  size_type __len = (size_type)(__last - __first);
  
  if (__len >= __n) {
    _Traits::assign(__first, __n, __c);
    erase(__first + __n, __last);
  }
  else {
    _Traits::assign(__first, __len, __c);
    insert(__last, __n - __len, __c);
  }
  return *this;
}

#ifndef _STLP_MEMBER_TEMPLATES


template <class _CharT, class _Traits, class _Alloc> basic_string<_CharT,_Traits,_Alloc>& basic_string<_CharT,_Traits,_Alloc> ::replace(iterator __first, iterator __last,
            const _CharT* __f, const _CharT* __l)
{
  const ptrdiff_t         __n = __l - __f;
  const difference_type __len = __last - __first;
  if (__len >= __n) {
    _M_copy(__f, __l, __first);
    erase(__first + __n, __last);
  }
  else {
    const _CharT* __m = __f + __len;
    _M_copy(__f, __m, __first);
    insert(__last, __m, __l);
  }
  return *this;
}

#endif /* _STLP_MEMBER_TEMPLATES */

template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::find(const _CharT* __s, size_type __pos, size_type __n) const 
{
  if (__pos + __n > size())
    return npos;
  else {
    const const_pointer __result =
      _STLP_STD::search((const _CharT*)this->_M_start + __pos, (const _CharT*)this->_M_finish, 
			__s, __s + __n, _Eq_traits<_Traits>());
    return __result != this->_M_finish ? __result - this->_M_start : npos;
  }
}

template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::find(_CharT __c, size_type __pos) const 
{
  if (__pos >= size())
    return npos;
  else {
    const const_pointer __result =
      _STLP_STD::find_if((const _CharT*)this->_M_start + __pos, (const _CharT*)this->_M_finish,
			 _Eq_char_bound<_Traits>(__c));
    return __result != this->_M_finish ? __result - this->_M_start : npos;
  }
}    

template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::rfind(const _CharT* __s, size_type __pos, size_type __n) const 
{
  const size_t __len = size();

  if (__n > __len)
    return npos;
  else if (__n == 0)
    return (min) (__len, __pos);
  else {
    const_pointer __last = this->_M_start + (min) (__len - __n, __pos) + __n;
    const_pointer __result = _STLP_STD::find_end((const_pointer)this->_M_start, __last,
						 __s, __s + __n,
						 _Eq_traits<_Traits>());
    return __result != __last ? __result - this->_M_start : npos;
  }
}

template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::rfind(_CharT __c, size_type __pos) const 
{
  const size_type __len = size();

  if (__len < 1)
    return npos;
  else {
    const const_iterator __last = begin() + (min) (__len - 1, __pos) + 1;
    const_reverse_iterator __rresult =
      _STLP_STD::find_if(const_reverse_iterator(__last), rend(),
              _Eq_char_bound<_Traits>(__c));
    return __rresult != rend() ? (__rresult.base() - 1) - begin() : npos;
  }
}

template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::find_first_of(const _CharT* __s, size_type __pos, size_type __n) const
{
  if (__pos >= size())
    return npos;
  else {
    const_iterator __result = __find_first_of(begin() + __pos, end(),
                                              __s, __s + __n,
                                              _Eq_traits<_Traits>());
    return __result != end() ? __result - begin() : npos;
  }
}


template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::find_last_of(const _CharT* __s, size_type __pos, size_type __n) const
{
  const size_type __len = size();

  if (__len < 1)
    return npos;
  else {
    const const_iterator __last = begin() + (min) (__len - 1, __pos) + 1;
    const const_reverse_iterator __rresult =
      __find_first_of(const_reverse_iterator(__last), rend(),
                      __s, __s + __n,
                      _Eq_traits<_Traits>());
    return __rresult != rend() ? (__rresult.base() - 1) - begin() : npos;
  }
}


template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::find_first_not_of(const _CharT* __s, size_type __pos, size_type __n) const
{
  typedef typename _Traits::char_type _CharType;
  if (__pos > size())
    return npos;
  else {
    const_pointer __result = _STLP_STD::find_if((const _CharT*)this->_M_start + __pos, 
				      (const _CharT*)this->_M_finish,
                                _Not_within_traits<_Traits>((const _CharType*)__s, 
							    (const _CharType*)__s + __n));
    return __result != this->_M_finish ? __result - this->_M_start : npos;
  }
}

template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::find_first_not_of(_CharT __c, size_type __pos) const
{
  if (__pos > size())
    return npos;
  else {
    const_pointer __result = _STLP_STD::find_if((const _CharT*)this->_M_start + __pos, (const _CharT*)this->_M_finish,
						_Neq_char_bound<_Traits>(__c));
    return __result != this->_M_finish ? __result - this->_M_start : npos;
  }
}    

template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT,_Traits,_Alloc> ::find_last_not_of(const _CharT* __s, size_type __pos, size_type __n) const 
{
  typedef typename _Traits::char_type _CharType;
  const size_type __len = size();

  if (__len < 1)
    return npos;
  else {
    const_iterator __last = begin() + (min) (__len - 1, __pos) + 1;
    const_reverse_iterator __rlast = const_reverse_iterator(__last);
    const_reverse_iterator __rresult =
      _STLP_STD::find_if(__rlast, rend(),
			 _Not_within_traits<_Traits>((const _CharType*)__s, 
						     (const _CharType*)__s + __n));
    return __rresult != rend() ? (__rresult.base() - 1) - begin() : npos;
  }
}

template <class _CharT, class _Traits, class _Alloc> __size_type__
basic_string<_CharT, _Traits, _Alloc> ::find_last_not_of(_CharT __c, size_type __pos) const 
{
  const size_type __len = size();

  if (__len < 1)
    return npos;
  else {
    const_iterator __last = begin() + (min) (__len - 1, __pos) + 1;
    const_reverse_iterator __rlast = const_reverse_iterator(__last);
    const_reverse_iterator __rresult =
      _STLP_STD::find_if(__rlast, rend(),
			 _Neq_char_bound<_Traits>(__c));
    return __rresult != rend() ? (__rresult.base() - 1) - begin() : npos;
  }
}

template <class _CharT, class _Traits, class _Alloc> void _STLP_CALL _S_string_copy(const basic_string<_CharT,_Traits,_Alloc>& __s,
                    _CharT* __buf,
                    size_t __n)
{
  if (__n > 0) {
    __n = (min) (__n - 1, __s.size());
    _STLP_STD::copy(__s.begin(), __s.begin() + __n, __buf);
    __buf[__n] = _CharT();
  }
}
_STLP_END_NAMESPACE

// _string_fwd has to see clean basic_string
# undef basic_string

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_string_fwd.c> 
# endif

# ifdef _STLP_DEBUG
#  define basic_string _Nondebug_string
# endif

# include <stl/_range_errors.h>  
_STLP_BEGIN_NAMESPACE

// _String_base methods
template <class _Tp, class _Alloc> void _String_base<_Tp,_Alloc>::_M_throw_length_error() const {
    __stl_throw_length_error("basic_string");
}

template <class _Tp, class _Alloc> void _String_base<_Tp, _Alloc>::_M_throw_out_of_range() const {
    __stl_throw_out_of_range("basic_string");
}

template <class _Tp, class _Alloc> void _String_base<_Tp, _Alloc>::_M_allocate_block(size_t __n) {  
  if ((__n <= (max_size()+1)) && (__n>0)){ 
    _M_start  = _M_end_of_storage.allocate(__n); 
    _M_finish = _M_start; 
    _M_end_of_storage._M_data = _M_start + __n; 
  } 
    else 
      _M_throw_length_error(); 
} 
 
template <class _CharT, class _Traits, class _Alloc> basic_string<_CharT, _Traits, _Alloc>::basic_string()
  : _String_base<_CharT,_Alloc>(allocator_type()) {  
  this->_M_start = this->_M_end_of_storage.allocate(8); 
  this->_M_finish = this->_M_start; 
  this->_M_end_of_storage._M_data = this->_M_start + 8; 
  _M_terminate_string();  
} 


template <class _CharT, class _Traits, class _Alloc> basic_string<_CharT, _Traits, _Alloc>::basic_string(const _CharT* __s, 
						    const allocator_type& __a) 
  : _String_base<_CharT,_Alloc>(__a)  
{ 
  _STLP_FIX_LITERAL_BUG(__s) 
    _M_range_initialize(__s, __s + traits_type::length(__s));  
} 


template <class _CharT, class _Traits, class _Alloc> basic_string<_CharT, _Traits, _Alloc>::basic_string(const basic_string<_CharT, _Traits, _Alloc> & __s)  
  : _String_base<_CharT,_Alloc>(__s.get_allocator())  
{  
  _M_range_initialize(__s._M_start, __s._M_finish);  
} 
  
# if defined ( __SUNPRO_CC) && ! defined(_STLP_STATIC_CONST_INIT_BUG)
template <class _CharT, class _Traits, class _Alloc> const size_t basic_string<_CharT, _Traits, _Alloc>::npos;
# endif

_STLP_END_NAMESPACE

# undef basic_string
# undef __size_type__
# undef size_type
# undef iterator
# endif /* NATIVE */

#endif /*  _STLP_STRING_C */

// Local Variables:
// mode:C++
// End:
