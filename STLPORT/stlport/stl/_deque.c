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
#ifndef _STLP_DEQUE_C
# define _STLP_DEQUE_C

# ifndef _STLP_INTERNAL_DEQUE_H
#  include <stl/_deque.h>
# endif

_STLP_BEGIN_NAMESPACE

// Non-inline member functions from _Deque_base.

template <class _Tp, class _Alloc >
_Deque_base<_Tp,_Alloc >::~_Deque_base() {
  if (_M_map._M_data) {
    _M_destroy_nodes(_M_start._M_node, this->_M_finish._M_node + 1);
    _M_map.deallocate(_M_map._M_data, _M_map_size._M_data);
  }
}

template <class _Tp, class _Alloc >
void
_Deque_base<_Tp,_Alloc>::_M_initialize_map(size_t __num_elements)
{
  size_t __num_nodes = 
    __num_elements / this->buffer_size() + 1 ;

  _M_map_size._M_data = (max)((size_t) _S_initial_map_size, __num_nodes + 2);
  _M_map._M_data = _M_map.allocate(_M_map_size._M_data);

  _Tp** __nstart = _M_map._M_data + (_M_map_size._M_data - __num_nodes) / 2;
  _Tp** __nfinish = __nstart + __num_nodes;
    
  _STLP_TRY {
    _M_create_nodes(__nstart, __nfinish);
  }
  _STLP_UNWIND((_M_map.deallocate(_M_map._M_data, _M_map_size._M_data), 
                _M_map._M_data = 0, _M_map_size._M_data = 0));
  _M_start._M_set_node(__nstart);
  this->_M_finish._M_set_node(__nfinish - 1);
  _M_start._M_cur = _M_start._M_first;
  this->_M_finish._M_cur = this->_M_finish._M_first +
               __num_elements % this->buffer_size();
}

template <class _Tp, class _Alloc >
void
_Deque_base<_Tp,_Alloc>::_M_create_nodes(_Tp** __nstart,
                                                  _Tp** __nfinish)
{
  _Tp** __cur;
  _STLP_TRY {
    for (__cur = __nstart; __cur < __nfinish; ++__cur)
      *__cur = _M_map_size.allocate(this->buffer_size());
  }
  _STLP_UNWIND(_M_destroy_nodes(__nstart, __cur));
}

template <class _Tp, class _Alloc >
void 
_Deque_base<_Tp,_Alloc>::_M_destroy_nodes(_Tp** __nstart,
                                                   _Tp** __nfinish)
{
  for (_Tp** __n = __nstart; __n < __nfinish; ++__n)
    _M_map_size.deallocate(*__n, this->buffer_size());
}



// Non-inline member functions

# if defined ( _STLP_NESTED_TYPE_PARAM_BUG )
// qualified references 
#   define __iterator__           _Deque_iterator<_Tp, _Nonconst_traits<_Tp> >
#   define const_iterator         _Deque_iterator<_Tp, _Const_traits<_Tp>  > 
#   define iterator               __iterator__
#   define size_type              size_t
#   define value_type             _Tp
# else
#  define __iterator__           _STLP_TYPENAME_ON_RETURN_TYPE __deque__<_Tp, _Alloc>::iterator
# endif

template <class _Tp, class _Alloc >
__deque__<_Tp, _Alloc >&  
__deque__<_Tp, _Alloc >::operator= (const __deque__<_Tp, _Alloc >& __x) {
  const size_type __len = size();
  if (&__x != this) {
    if (__len >= __x.size())
      erase(_STLP_STD::copy(__x.begin(), __x.end(), this->_M_start), this->_M_finish);
    else {
      const_iterator __mid = __x.begin() + difference_type(__len);
      _STLP_STD::copy(__x.begin(), __mid, this->_M_start);
      insert(this->_M_finish, __mid, __x.end());
    }
  }
  return *this;
}        

template <class _Tp, class _Alloc >
void 
__deque__<_Tp, _Alloc >::_M_fill_insert(iterator __pos,
					     size_type __n, const value_type& __x)
{
  if (__pos._M_cur == this->_M_start._M_cur) {
    iterator __new_start = _M_reserve_elements_at_front(__n);
    _STLP_TRY {
      uninitialized_fill(__new_start, this->_M_start, __x);
    }
    _STLP_UNWIND(this->_M_destroy_nodes(__new_start._M_node, this->_M_start._M_node));
    this->_M_start = __new_start;
  }
  else if (__pos._M_cur == this->_M_finish._M_cur) {
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    _STLP_TRY {
      uninitialized_fill(this->_M_finish, __new_finish, __x);
    }
    _STLP_UNWIND(this->_M_destroy_nodes(this->_M_finish._M_node+1, __new_finish._M_node+1));
    this->_M_finish = __new_finish;
  }
  else 
    _M_insert_aux(__pos, __n, __x);
}

#ifndef _STLP_MEMBER_TEMPLATES  

template <class _Tp, class _Alloc >
void __deque__<_Tp, _Alloc>::insert(iterator __pos,
                                           const value_type* __first,
                                           const value_type* __last) {
  size_type __n = __last - __first;
  if (__pos._M_cur == this->_M_start._M_cur) {
    iterator __new_start = _M_reserve_elements_at_front(__n);
    _STLP_TRY {
      uninitialized_copy(__first, __last, __new_start);
    }
    _STLP_UNWIND(this->_M_destroy_nodes(__new_start._M_node, this->_M_start._M_node));
    this->_M_start = __new_start;
  }
  else if (__pos._M_cur == this->_M_finish._M_cur) {
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    _STLP_TRY {
      uninitialized_copy(__first, __last, this->_M_finish);
    }
    _STLP_UNWIND(this->_M_destroy_nodes(this->_M_finish._M_node + 1, 
                                  __new_finish._M_node + 1));
    this->_M_finish = __new_finish;
  }
  else
    _M_insert_aux(__pos, __first, __last, __n);
}

template <class _Tp, class _Alloc >
void __deque__<_Tp,_Alloc>::insert(iterator __pos,
                                         const_iterator __first,
                                         const_iterator __last)
{
  size_type __n = __last - __first;
  if (__pos._M_cur == this->_M_start._M_cur) {
    iterator __new_start = _M_reserve_elements_at_front(__n);
    _STLP_TRY {
      uninitialized_copy(__first, __last, __new_start);
    }
    _STLP_UNWIND(this->_M_destroy_nodes(__new_start._M_node, this->_M_start._M_node));
    this->_M_start = __new_start;
  }
  else if (__pos._M_cur == this->_M_finish._M_cur) {
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    _STLP_TRY {
      uninitialized_copy(__first, __last, this->_M_finish);
    }
    _STLP_UNWIND(this->_M_destroy_nodes(this->_M_finish._M_node + 1,__new_finish._M_node + 1));
    this->_M_finish = __new_finish;
  }
  else
    _M_insert_aux(__pos, __first, __last, __n);
}

#endif /* _STLP_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc >
__iterator__ 
__deque__<_Tp,_Alloc>::erase(iterator __first, iterator __last)
{
  if (__first == this->_M_start && __last == this->_M_finish) {
    clear();
    return this->_M_finish;
  }
  else {
    difference_type __n = __last - __first;
    difference_type __elems_before = __first - this->_M_start;
    if (__elems_before < difference_type(this->size() - __n) / 2) {
      _STLP_STD::copy_backward(this->_M_start, __first, __last);
      iterator __new_start = this->_M_start + __n;
      _STLP_STD::_Destroy(this->_M_start, __new_start);
      this->_M_destroy_nodes(this->_M_start._M_node, __new_start._M_node);
      this->_M_start = __new_start;
    }
    else {
      _STLP_STD::copy(__last, this->_M_finish, __first);
      iterator __new_finish = this->_M_finish - __n;
      _STLP_STD::_Destroy(__new_finish, this->_M_finish);
      this->_M_destroy_nodes(__new_finish._M_node + 1, this->_M_finish._M_node + 1);
      this->_M_finish = __new_finish;
    }
    return this->_M_start + __elems_before;
  }
}

template <class _Tp, class _Alloc >
void __deque__<_Tp,_Alloc>::clear()
{
  for (_Map_pointer __node = this->_M_start._M_node + 1;
       __node < this->_M_finish._M_node;
       ++__node) {
    _STLP_STD::_Destroy(*__node, *__node + this->buffer_size());
    this->_M_map_size.deallocate(*__node, this->buffer_size());
  }

  if (this->_M_start._M_node != this->_M_finish._M_node) {
    _STLP_STD::_Destroy(this->_M_start._M_cur, this->_M_start._M_last);
    _STLP_STD::_Destroy(this->_M_finish._M_first, this->_M_finish._M_cur);
    this->_M_map_size.deallocate(this->_M_finish._M_first, this->buffer_size());
  }
  else
    _STLP_STD::_Destroy(this->_M_start._M_cur, this->_M_finish._M_cur);

  this->_M_finish = this->_M_start;
}

// Precondition: this->_M_start and this->_M_finish have already been initialized,
// but none of the deque's elements have yet been constructed.
template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_fill_initialize(const value_type& __val) {
  _Map_pointer __cur;
  _STLP_TRY {
    for (__cur = this->_M_start._M_node; __cur < this->_M_finish._M_node; ++__cur)
      uninitialized_fill(*__cur, *__cur + this->buffer_size(), __val);
    uninitialized_fill(this->_M_finish._M_first, this->_M_finish._M_cur, __val);
  }
  _STLP_UNWIND(_STLP_STD::_Destroy(this->_M_start, iterator(*__cur, __cur)));
}


// Called only if this->_M_finish._M_cur == this->_M_finish._M_last - 1.
template <class _Tp, class _Alloc >
void
__deque__<_Tp,_Alloc>::_M_push_back_aux_v(const value_type& __t)
{
  value_type __t_copy = __t;
  _M_reserve_map_at_back();
  *(this->_M_finish._M_node + 1) = this->_M_map_size.allocate(this->buffer_size());
  _STLP_TRY {
    _STLP_STD::_Construct(this->_M_finish._M_cur, __t_copy);
    this->_M_finish._M_set_node(this->_M_finish._M_node + 1);
    this->_M_finish._M_cur = this->_M_finish._M_first;
  }
  _STLP_UNWIND(this->_M_map_size.deallocate(*(this->_M_finish._M_node + 1), 
				      this->buffer_size()));
}

# ifndef _STLP_NO_ANACHRONISMS
// Called only if this->_M_finish._M_cur == this->_M_finish._M_last - 1.
template <class _Tp, class _Alloc >
void
__deque__<_Tp,_Alloc>::_M_push_back_aux()
{
  _M_reserve_map_at_back();
  *(this->_M_finish._M_node + 1) = this->_M_map_size.allocate(this->buffer_size());
  _STLP_TRY {
    _STLP_STD::_Construct(this->_M_finish._M_cur);
    this->_M_finish._M_set_node(this->_M_finish._M_node + 1);
    this->_M_finish._M_cur = this->_M_finish._M_first;
  }
  _STLP_UNWIND(this->_M_map_size.deallocate(*(this->_M_finish._M_node + 1), 
				      this->buffer_size()));
}
# endif

// Called only if this->_M_start._M_cur == this->_M_start._M_first.
template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_push_front_aux_v(const value_type& __t)
{
  value_type __t_copy = __t;
  _M_reserve_map_at_front();
  *(this->_M_start._M_node - 1) = this->_M_map_size.allocate(this->buffer_size());
  _STLP_TRY {
    this->_M_start._M_set_node(this->_M_start._M_node - 1);
    this->_M_start._M_cur = this->_M_start._M_last - 1;
    _STLP_STD::_Construct(this->_M_start._M_cur, __t_copy);
  }
  _STLP_UNWIND((++this->_M_start, 
		this->_M_map_size.deallocate(*(this->_M_start._M_node - 1), this->buffer_size())));
} 


# ifndef _STLP_NO_ANACHRONISMS
// Called only if this->_M_start._M_cur == this->_M_start._M_first.
template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_push_front_aux()
{
  _M_reserve_map_at_front();
  *(this->_M_start._M_node - 1) = this->_M_map_size.allocate(this->buffer_size());
  _STLP_TRY {
    this->_M_start._M_set_node(this->_M_start._M_node - 1);
    this->_M_start._M_cur = this->_M_start._M_last - 1;
    _STLP_STD::_Construct(this->_M_start._M_cur);
  }
  _STLP_UNWIND((++this->_M_start, this->_M_map_size.deallocate(*(this->_M_start._M_node - 1), 
						   this->buffer_size() )));
} 
# endif

// Called only if this->_M_finish._M_cur == this->_M_finish._M_first.
template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_pop_back_aux()
{
  this->_M_map_size.deallocate(this->_M_finish._M_first, this->buffer_size());
  this->_M_finish._M_set_node(this->_M_finish._M_node - 1);
  this->_M_finish._M_cur = this->_M_finish._M_last - 1;
  _STLP_STD::_Destroy(this->_M_finish._M_cur);
}

// Called only if this->_M_start._M_cur == this->_M_start._M_last - 1.  Note that 
// if the deque has at least one element (a precondition for this member 
// function), and if this->_M_start._M_cur == this->_M_start._M_last, then the deque 
// must have at least two nodes.
template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_pop_front_aux()
{
  _STLP_STD::_Destroy(this->_M_start._M_cur);
  this->_M_map_size.deallocate(this->_M_start._M_first, this->buffer_size());
  this->_M_start._M_set_node(this->_M_start._M_node + 1);
  this->_M_start._M_cur = this->_M_start._M_first;
}      



template <class _Tp, class _Alloc >
__iterator__
__deque__<_Tp,_Alloc>::_M_insert_aux_prepare(iterator __pos) {
  difference_type __index = __pos - this->_M_start;
  if (__index < difference_type(size() / 2)) {
    push_front(front());
    iterator __front1 = this->_M_start;
    ++__front1;
    iterator __front2 = __front1;
    ++__front2;
    __pos = this->_M_start + __index;
    iterator __pos1 = __pos;
    ++__pos1;
    copy(__front2, __pos1, __front1);
  }
  else {
    push_back(back());
    iterator __back1 = this->_M_finish;
    --__back1;
    iterator __back2 = __back1;
    --__back2;
    __pos = this->_M_start + __index;
    copy_backward(__pos, __back2, __back1);
  }
  return __pos;
}

template <class _Tp, class _Alloc >
__iterator__
__deque__<_Tp,_Alloc>::_M_insert_aux(iterator __pos,
				     const value_type& __x) {
  value_type __x_copy = __x;
  _STLP_MPWFIX_TRY		//*TY 06/01/2000 - mpw forget to call dtor on __x_copy without this try block
  __pos = _M_insert_aux_prepare(__pos);
  *__pos = __x_copy;
  return __pos;
  _STLP_MPWFIX_CATCH		//*TY 06/01/2000 - 
}

template <class _Tp, class _Alloc >
__iterator__
__deque__<_Tp,_Alloc>::_M_insert_aux(iterator __pos)
{
  __pos = _M_insert_aux_prepare(__pos);
  *__pos = value_type();
  return __pos;
}

template <class _Tp, class _Alloc >
void
__deque__<_Tp,_Alloc>::_M_insert_aux(iterator __pos,
                                           size_type __n,
                                           const value_type& __x)
{
  const difference_type __elems_before = __pos - this->_M_start;
  size_type __length = this->size();
  value_type __x_copy = __x;
  if (__elems_before < difference_type(__length / 2)) {
    iterator __new_start = _M_reserve_elements_at_front(__n);
    iterator __old_start = this->_M_start;
    __pos = this->_M_start + __elems_before;
    _STLP_TRY {
      if (__elems_before >= difference_type(__n)) {
        iterator __start_n = this->_M_start + difference_type(__n);
        uninitialized_copy(this->_M_start, __start_n, __new_start);
        this->_M_start = __new_start;
        copy(__start_n, __pos, __old_start);
        fill(__pos - difference_type(__n), __pos, __x_copy);
      }
      else {
        __uninitialized_copy_fill(this->_M_start, __pos, __new_start, 
	                          this->_M_start, __x_copy);
        this->_M_start = __new_start;
        fill(__old_start, __pos, __x_copy);
      }
    }
    _STLP_UNWIND(this->_M_destroy_nodes(__new_start._M_node, this->_M_start._M_node));
  }
  else {
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    iterator __old_finish = this->_M_finish;
    const difference_type __elems_after = 
      difference_type(__length) - __elems_before;
    __pos = this->_M_finish - __elems_after;
    _STLP_TRY {
      if (__elems_after > difference_type(__n)) {
        iterator __finish_n = this->_M_finish - difference_type(__n);
        uninitialized_copy(__finish_n, this->_M_finish, this->_M_finish);
        this->_M_finish = __new_finish;
        copy_backward(__pos, __finish_n, __old_finish);
        fill(__pos, __pos + difference_type(__n), __x_copy);
      }
      else {
        __uninitialized_fill_copy(this->_M_finish, __pos + difference_type(__n),
                                  __x_copy, __pos, this->_M_finish);
        this->_M_finish = __new_finish;
        fill(__pos, __old_finish, __x_copy);
      }
    }
    _STLP_UNWIND(this->_M_destroy_nodes(this->_M_finish._M_node + 1, __new_finish._M_node + 1));
  }
}

#ifndef _STLP_MEMBER_TEMPLATES 
template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_insert_aux(iterator __pos,
                                           const value_type* __first,
                                           const value_type* __last,
                                           size_type __n)
{

  const difference_type __elemsbefore = __pos - this->_M_start;
  size_type __length = size();
  if (__elemsbefore < difference_type(__length / 2)) {
    iterator __new_start = _M_reserve_elements_at_front(__n);
    iterator __old_start = this->_M_start;
    __pos = this->_M_start + __elemsbefore;
    _STLP_TRY {
      if (__elemsbefore >= difference_type(__n)) {
        iterator __start_n = this->_M_start + difference_type(__n);
        uninitialized_copy(this->_M_start, __start_n, __new_start);
        this->_M_start = __new_start;
        copy(__start_n, __pos, __old_start);
        copy(__first, __last, __pos - difference_type(__n));
      }
      else {
        const value_type* __mid = 
	  __first + (difference_type(__n) - __elemsbefore);
        __uninitialized_copy_copy(this->_M_start, __pos, __first, __mid,
                                  __new_start, _IsPODType());
        this->_M_start = __new_start;
        copy(__mid, __last, __old_start);
      }
    }
    _STLP_UNWIND(this->_M_destroy_nodes(__new_start._M_node, this->_M_start._M_node));
  }
  else {
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    iterator __old_finish = this->_M_finish;
    const difference_type __elemsafter = 
      difference_type(__length) - __elemsbefore;
    __pos = this->_M_finish - __elemsafter;
    _STLP_TRY {
      if (__elemsafter > difference_type(__n)) {
        iterator __finish_n = this->_M_finish - difference_type(__n);
        uninitialized_copy(__finish_n, this->_M_finish, this->_M_finish);
        this->_M_finish = __new_finish;
        copy_backward(__pos, __finish_n, __old_finish);
        copy(__first, __last, __pos);
      }
      else {
        const value_type* __mid = __first + __elemsafter;
        __uninitialized_copy_copy(__mid, __last, __pos, this->_M_finish, this->_M_finish, _IsPODType());
        this->_M_finish = __new_finish;
        copy(__first, __mid, __pos);
      }
    }
    _STLP_UNWIND(this->_M_destroy_nodes(this->_M_finish._M_node + 1, __new_finish._M_node + 1));
  }
}

template <class _Tp, class _Alloc >
void
__deque__<_Tp,_Alloc>::_M_insert_aux(iterator __pos,
                                           const_iterator __first,
                                           const_iterator __last,
                                           size_type __n)
{
  const difference_type __elemsbefore = __pos - this->_M_start;
  size_type __length = size();
  if (__elemsbefore < difference_type(__length / 2)) {
    iterator __new_start = _M_reserve_elements_at_front(__n);
    iterator __old_start = this->_M_start;
    __pos = this->_M_start + __elemsbefore;
    _STLP_TRY {
      if (__elemsbefore >= difference_type(__n)) {
        iterator __start_n = this->_M_start + __n;
        uninitialized_copy(this->_M_start, __start_n, __new_start);
        this->_M_start = __new_start;
        copy(__start_n, __pos, __old_start);
        copy(__first, __last, __pos - difference_type(__n));
      }
      else {
        const_iterator __mid = __first + (__n - __elemsbefore);
        __uninitialized_copy_copy(this->_M_start, __pos, __first, __mid,
                                  __new_start, _IsPODType());
        this->_M_start = __new_start;
        copy(__mid, __last, __old_start);
      }
    }
    _STLP_UNWIND(this->_M_destroy_nodes(__new_start._M_node, this->_M_start._M_node));
  }
  else {
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    iterator __old_finish = this->_M_finish;
    const difference_type __elemsafter = __length - __elemsbefore;
    __pos = this->_M_finish - __elemsafter;
    _STLP_TRY {
      if (__elemsafter > difference_type(__n)) {
        iterator __finish_n = this->_M_finish - difference_type(__n);
        uninitialized_copy(__finish_n, this->_M_finish, this->_M_finish);
        this->_M_finish = __new_finish;
        copy_backward(__pos, __finish_n, __old_finish);
        copy(__first, __last, __pos);
      }
      else {
        const_iterator __mid = __first + __elemsafter;
        __uninitialized_copy_copy(__mid, __last, __pos, this->_M_finish, this->_M_finish, _IsPODType());
        this->_M_finish = __new_finish;
        copy(__first, __mid, __pos);
      }
    }
    _STLP_UNWIND(this->_M_destroy_nodes(this->_M_finish._M_node + 1, __new_finish._M_node + 1));
  }
}

#endif /* _STLP_MEMBER_TEMPLATES */

template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_new_elements_at_front(size_type __new_elems)
{
  size_type __new_nodes
      = (__new_elems + this->buffer_size() - 1) / this->buffer_size();
  _M_reserve_map_at_front(__new_nodes);
  size_type __i =1;
  _STLP_TRY {
    for (; __i <= __new_nodes; ++__i)
      *(this->_M_start._M_node - __i) = this->_M_map_size.allocate(this->buffer_size());
  }
#       ifdef _STLP_USE_EXCEPTIONS
  catch(...) {
    for (size_type __j = 1; __j < __i; ++__j)
      this->_M_map_size.deallocate(*(this->_M_start._M_node - __j), this->buffer_size());
    throw;
  }
#       endif /* _STLP_USE_EXCEPTIONS */
}

template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_new_elements_at_back(size_type __new_elems)
{
  size_type __new_nodes
      = (__new_elems + this->buffer_size() - 1) / this->buffer_size();
  _M_reserve_map_at_back(__new_nodes);
  size_type __i = 1;
  _STLP_TRY {
    for (; __i <= __new_nodes; ++__i)
      *(this->_M_finish._M_node + __i) = this->_M_map_size.allocate(this->buffer_size());
  }
#       ifdef _STLP_USE_EXCEPTIONS
  catch(...) {
    for (size_type __j = 1; __j < __i; ++__j)
      this->_M_map_size.deallocate(*(this->_M_finish._M_node + __j), this->buffer_size());
    throw;
  }
#       endif /* _STLP_USE_EXCEPTIONS */
}

template <class _Tp, class _Alloc >
void 
__deque__<_Tp,_Alloc>::_M_reallocate_map(size_type __nodes_to_add,
                                              bool __add_at_front)
{
  size_type __old_num_nodes = this->_M_finish._M_node - this->_M_start._M_node + 1;
  size_type __new_num_nodes = __old_num_nodes + __nodes_to_add;

  _Map_pointer __new_nstart;
  if (this->_M_map_size._M_data > 2 * __new_num_nodes) {
    __new_nstart = this->_M_map._M_data + (this->_M_map_size._M_data - __new_num_nodes) / 2 
                     + (__add_at_front ? __nodes_to_add : 0);
    if (__new_nstart < this->_M_start._M_node)
      _STLP_STD::copy(this->_M_start._M_node, this->_M_finish._M_node + 1, __new_nstart);
    else
      _STLP_STD::copy_backward(this->_M_start._M_node, this->_M_finish._M_node + 1, 
                    __new_nstart + __old_num_nodes);
  }
  else {
    size_type __new_map_size = 
      this->_M_map_size._M_data + (max)((size_t)this->_M_map_size._M_data, __nodes_to_add) + 2;

    _Map_pointer __new_map = this->_M_map.allocate(__new_map_size);
    __new_nstart = __new_map + (__new_map_size - __new_num_nodes) / 2
                         + (__add_at_front ? __nodes_to_add : 0);
    _STLP_STD::copy(this->_M_start._M_node, this->_M_finish._M_node + 1, __new_nstart);
    this->_M_map.deallocate(this->_M_map._M_data, this->_M_map_size._M_data);

    this->_M_map._M_data = __new_map;
    this->_M_map_size._M_data = __new_map_size;
  }

  this->_M_start._M_set_node(__new_nstart);
  this->_M_finish._M_set_node(__new_nstart + __old_num_nodes - 1);
}

_STLP_END_NAMESPACE

# undef __iterator__
# undef iterator
# undef const_iterator
# undef size_type
# undef value_type

#endif /*  _STLP_DEQUE_C */

// Local Variables:
// mode:C++
// End:
