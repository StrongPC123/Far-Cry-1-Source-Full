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
#ifndef _STLP_LIST_C
#define _STLP_LIST_C

#ifndef _STLP_INTERNAL_LIST_H
# include <stl/_list.h>
#endif

#if defined (__WATCOMC__)
#include <vector>
#endif

# undef list
# define  list  __WORKAROUND_DBG_RENAME(list)

_STLP_BEGIN_NAMESPACE

# if defined (_STLP_EXPOSE_GLOBALS_IMPLEMENTATION)

template <class _Dummy>
void _STLP_CALL
_List_global<_Dummy>::_Transfer(_List_node_base* __position, 
				_List_node_base* __first, _List_node_base* __last) {
  if (__position != __last) {
    // Remove [first, last) from its old position.
    ((_Node*) (__last->_M_prev))->_M_next = __position;
    ((_Node*) (__first->_M_prev))->_M_next    = __last;
    ((_Node*) (__position->_M_prev))->_M_next = __first; 
    
    // Splice [first, last) into its new position.
    _Node* __tmp = (_Node*) (__position->_M_prev);
    __position->_M_prev = __last->_M_prev;
    __last->_M_prev      = __first->_M_prev; 
    __first->_M_prev    = __tmp;
  }
}

#endif /* defined (__BUILDING_STLPORT) || ! defined (_STLP_OWN_IOSTREAMS) */


template <class _Tp, class _Alloc>
void 
_List_base<_Tp,_Alloc>::clear() 
{
  _List_node<_Tp>* __cur = (_List_node<_Tp>*) this->_M_node._M_data->_M_next;
  while (__cur != this->_M_node._M_data) {
    _List_node<_Tp>* __tmp = __cur;
    __cur = (_List_node<_Tp>*) __cur->_M_next;
    _STLP_STD::_Destroy(&__tmp->_M_data);
    this->_M_node.deallocate(__tmp, 1);
  }
  this->_M_node._M_data->_M_next = this->_M_node._M_data;
  this->_M_node._M_data->_M_prev = this->_M_node._M_data;
}

# if defined (_STLP_NESTED_TYPE_PARAM_BUG) 
#  define size_type      size_t
# endif

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::resize(size_type __new_size, _Tp __x)
{
  iterator __i = begin();
  size_type __len = 0;
  for ( ; __i != end() && __len < __new_size; ++__i, ++__len);

  if (__len == __new_size)
    erase(__i, end());
  else                          // __i == end()
    insert(end(), __new_size - __len, __x);
}

template <class _Tp, class _Alloc>
list<_Tp, _Alloc>& list<_Tp, _Alloc>::operator=(const list<_Tp, _Alloc>& __x)
{
  if (this != &__x) {
    iterator __first1 = begin();
    iterator __last1 = end();
    const_iterator __first2 = __x.begin();
    const_iterator __last2 = __x.end();
    while (__first1 != __last1 && __first2 != __last2) 
      *__first1++ = *__first2++;
    if (__first2 == __last2)
      erase(__first1, __last1);
    else
      insert(__last1, __first2, __last2);
  }
  return *this;
}

template <class _Tp, class _Alloc>
void list<_Tp, _Alloc>::_M_fill_assign(size_type __n, const _Tp& __val) {
  iterator __i = begin();
  for ( ; __i != end() && __n > 0; ++__i, --__n)
    *__i = __val;
  if (__n > 0)
    insert(end(), __n, __val);
  else
    erase(__i, end());
}

template <class _Tp, class _Alloc, class _Predicate> 
void _S_remove_if(list<_Tp, _Alloc>& __that, _Predicate __pred)  {
  typename list<_Tp, _Alloc>::iterator __first = __that.begin();
  typename list<_Tp, _Alloc>::iterator __last = __that.end();
  while (__first != __last) {
    typename list<_Tp, _Alloc>::iterator __next = __first;
    ++__next;
    if (__pred(*__first)) __that.erase(__first);
    __first = __next;
  }
}

template <class _Tp, class _Alloc, class _BinaryPredicate>
void _S_unique(list<_Tp, _Alloc>& __that, _BinaryPredicate __binary_pred) {
  typename list<_Tp, _Alloc>::iterator __first = __that.begin();
  typename list<_Tp, _Alloc>::iterator __last = __that.end();
  if (__first == __last) return;
  typename list<_Tp, _Alloc>::iterator __next = __first;
  while (++__next != __last) {
    if (__binary_pred(*__first, *__next))
      __that.erase(__next);
    else
      __first = __next;
    __next = __first;
  }
}

template <class _Tp, class _Alloc, class _StrictWeakOrdering>
void _S_merge(list<_Tp, _Alloc>& __that, list<_Tp, _Alloc>& __x,
	      _StrictWeakOrdering __comp) {
  typedef typename list<_Tp, _Alloc>::iterator _Literator;
  _Literator __first1 = __that.begin();
  _Literator __last1 = __that.end();
  _Literator __first2 = __x.begin();
  _Literator __last2 = __x.end();
  while (__first1 != __last1 && __first2 != __last2)
    if (__comp(*__first2, *__first1)) {
      _Literator __next = __first2;
      _List_global_inst::_Transfer(__first1._M_node, __first2._M_node, (++__next)._M_node);
      __first2 = __next;
    }
    else
      ++__first1;
  if (__first2 != __last2) _List_global_inst::_Transfer(__last1._M_node, __first2._M_node, __last2._M_node);
}

template <class _Tp, class _Alloc, class _StrictWeakOrdering>
void _S_sort(list<_Tp, _Alloc>& __that, _StrictWeakOrdering __comp) {
  // Do nothing if the list has length 0 or 1.
  if (__that._M_node._M_data->_M_next != __that._M_node._M_data &&
      (__that._M_node._M_data->_M_next)->_M_next != __that._M_node._M_data) {
    list<_Tp, _Alloc> __carry;
#if !defined (__WATCOMC__)
    list<_Tp, _Alloc> __counter[64];
#else
    __vector__<list<_Tp, _Alloc>, _Alloc> __counter(64);		
#endif		//*TY 05/25/2000 - 
    int __fill = 0;
    while (!__that.empty()) {
      __carry.splice(__carry.begin(), __that, __that.begin());
      int __i = 0;
      while(__i < __fill && !__counter[__i].empty()) {
	_S_merge(__counter[__i], __carry, __comp);
	__carry.swap(__counter[__i++]);
      }
      __carry.swap(__counter[__i]);         
      if (__i == __fill) ++__fill;
    } 
    
    for (int __i = 1; __i < __fill; ++__i) 
      _S_merge(__counter[__i], __counter[__i-1], __comp);
    __that.swap(__counter[__fill-1]);
  }
}

# undef  list
# undef  size_type

_STLP_END_NAMESPACE

#endif /*  _STLP_LIST_C */

// Local Variables:
// mode:C++
// End:
