/*
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

/* NOTE: This is an internal header file, included by other STL headers.
 *   You should not attempt to use it directly.
 */

#ifndef _STLP_INTERNAL_DBG_LIST_H
#define _STLP_INTERNAL_DBG_LIST_H

#include <stl/debug/_iterator.h>

# ifndef _STLP_USE_WRAPPER_FOR_ALLOC_PARAM
#  undef  _DBG_list
#  define _DBG_list list
# endif

#  define _STLP_DBG_LIST_BASE \
__WORKAROUND_DBG_RENAME(list) <_Tp, _Alloc>

_STLP_BEGIN_NAMESPACE

# ifdef _STLP_DEBUG_USE_DISTINCT_VALUE_TYPE_HELPERS
template <class _Tp, class _Alloc>
inline _Tp*
value_type(const  _DBG_iter_base< _STLP_DBG_LIST_BASE >&) {
  return (_Tp*)0;
}
template <class _Tp, class _Alloc>
inline bidirectional_iterator_tag
iterator_category(const _DBG_iter_base< _STLP_DBG_LIST_BASE >&) {
  return bidirectional_iterator_tag();
}
# endif

template <class _Tp, _STLP_DEFAULT_ALLOCATOR_SELECT(_Tp) >
class _DBG_list : public _STLP_DBG_LIST_BASE {

  typedef _STLP_DBG_LIST_BASE _Base;
  typedef _DBG_list<_Tp, _Alloc> _Self;

public:      
  __IMPORT_CONTAINER_TYPEDEFS(_Base)

public:
  typedef _DBG_iter<_Base, _Nonconst_traits<value_type> >     iterator;
  typedef _DBG_iter<_Base, _Const_traits<value_type> >    const_iterator;

  _STLP_DECLARE_BIDIRECTIONAL_REVERSE_ITERATORS;

protected:
  mutable __owned_list _M_iter_list;
  void _Invalidate_all() { _M_iter_list._Invalidate_all();}

public:
  const _Base* _Get_base() const { return (const _Base*)this; }
  _Base* _Get_base() { return (_Base*)this; }
  explicit _DBG_list(const allocator_type& __a = allocator_type()) :
    _STLP_DBG_LIST_BASE(__a), _M_iter_list(_Get_base()) {}
  _DBG_list(size_type __n, const _Tp& __value,
	    const allocator_type& __a = allocator_type())
    : _STLP_DBG_LIST_BASE(__n, __value, __a), _M_iter_list(_Get_base()) {}
  explicit _DBG_list(size_type __n)
    : _STLP_DBG_LIST_BASE(__n), _M_iter_list(_Get_base()) {}
  
#ifdef _STLP_MEMBER_TEMPLATES

  // We don't need any dispatching tricks here, because insert does all of
  // that anyway.  
#  ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  template <class _InputIterator>
  _DBG_list(_InputIterator __first, _InputIterator __last)
    : _STLP_DBG_LIST_BASE(__first, __last, allocator_type()), _M_iter_list(_Get_base()) {}
#  endif
  template <class _InputIterator>
  _DBG_list(_InputIterator __first, _InputIterator __last,
       const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL)
    : _STLP_DBG_LIST_BASE(__first, __last, __a), _M_iter_list(_Get_base()) {}
#else /* _STLP_MEMBER_TEMPLATES */

  _DBG_list(const _Tp* __first, const _Tp* __last,
       const allocator_type& __a = allocator_type())
    : _STLP_DBG_LIST_BASE(__first, __last, __a), _M_iter_list(_Get_base()) {}
  _DBG_list(const_iterator __first, const_iterator __last,
       const allocator_type& __a = allocator_type()):
    _STLP_DBG_LIST_BASE(__first._M_iterator, __last._M_iterator, __a),
    _M_iter_list(_Get_base()) {}

#endif /* _STLP_MEMBER_TEMPLATES */

  _DBG_list(const _Self& __x) : 
    _STLP_DBG_LIST_BASE(__x) , _M_iter_list(_Get_base()) {}

  _Self& operator=(const _Self& __x) {
    _Invalidate_all();
    _Base::operator=((const _Base&)__x);
    return *this;
  }

public:
  iterator begin()             { return iterator(&_M_iter_list, _Base::begin()); }
  const_iterator begin() const { return const_iterator(&_M_iter_list, _Base::begin()); }

  iterator end()               { return iterator(&_M_iter_list, _Base::end()); }
  const_iterator end() const   { return const_iterator(&_M_iter_list, _Base::end()); }

  reverse_iterator rbegin()    { return reverse_iterator(end()); }
  reverse_iterator rend()      { return reverse_iterator(begin()); }

  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

  // those are here to enforce checking
  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }
  reference back() { return *(--end()); }
  const_reference back() const { return *(--end()); }

  void swap(_Self& __x) {
    _M_iter_list._Swap_owners(__x._M_iter_list);
    _Base::swap(__x); 
  }

  iterator insert(iterator __position, const _Tp& __x) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list,__position))
      return iterator(&_M_iter_list,_Base::insert(__position._M_iterator, __x) );
  }

# ifndef _STLP_NO_ANACHRONISMS
  iterator insert(iterator __position) { return insert(__position, _Tp()); }
# endif

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIterator>
  void insert(iterator __position, _InputIterator __first, _InputIterator __last) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list,__position))
    _Base::insert(__position._M_iterator, __first, __last);
  }

#else

  void insert(iterator __position, const _Tp* __first, const _Tp* __last) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list,__position))
    _Base::insert(__position._M_iterator, __first, __last);
  }

  void insert(iterator __position,
	      const_iterator __first, const_iterator __last) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list,__position))
    _Base::insert(__position._M_iterator, __first._M_iterator, __last._M_iterator);
  }
  
#endif /* _STLP_MEMBER_TEMPLATES */

  void insert(iterator __position, size_type __n, const _Tp& __x) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list,__position))
    _Base::insert(__position._M_iterator, __n, __x);    
  }

  void pop_back() {
    _STLP_VERBOSE_ASSERT(!this->empty(), _StlMsg_EMPTY_CONTAINER)
    __invalidate_iterator(&_M_iter_list,end());
    _Base::pop_back();
  }

  void pop_front() {
    _STLP_VERBOSE_ASSERT(!this->empty(), _StlMsg_EMPTY_CONTAINER)
    __invalidate_iterator(&_M_iter_list,begin());
    _Base::pop_front();
  }

  iterator erase(iterator __position) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list,__position))
    _STLP_VERBOSE_ASSERT(__position._M_iterator._M_node!=this->_M_node._M_data, 
			 _StlMsg_ERASE_PAST_THE_END)
    // fbp : CHECK !!!
    __invalidate_iterator(&_M_iter_list, __position);
    return iterator(&_M_iter_list,_Base::erase(__position._M_iterator));
  }
  iterator erase(iterator __first, iterator __last) {
    while (__first != __last)
      erase(__first++);
    return __last;
  }

  void resize(size_type __new_size, const _Tp& __x) {
    typename _Base::iterator __i = _Base::begin();
    size_type __len = 0;
    for ( ; __i != _Base::end() && __len < __new_size; ++__i, ++__len);
    
    if (__len == __new_size)
      erase(iterator(&_M_iter_list,__i), end());
    else                          // __i == end()
      _Base::insert(_Base::end(), __new_size - __len, __x);
  }

  void resize(size_type __new_size) { this->resize(__new_size, _Tp()); }

  void remove(const _Tp& __value) {
    typename _Base::iterator __first = _Base::begin();
    typename _Base::iterator __last = _Base::end();
    while (__first != __last) {
      typename _Base::iterator __next = __first;
      ++__next;
      if (__value == *__first) erase(iterator(&_M_iter_list,__first));
      __first = __next;
    }
  }

  void clear() {   
    _Invalidate_all();
    _Base::clear(); 
  }

public:
  void splice(iterator __position, _Self& __x) {
    _STLP_VERBOSE_ASSERT(&__x!=this, _StlMsg_INVALID_ARGUMENT)
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list,__position))
    _Base::splice(__position._M_iterator, __x);
    __x._Invalidate_all();
  }

  void splice(iterator __position, _Self& __x, iterator __i) {
      _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list,__position) &&
                        __check_if_owner(&__x._M_iter_list ,__i))
    _STLP_DEBUG_CHECK(_Dereferenceable(__i))
    // fbp : CHECK !!!
	// __invalidate_iterator(&__x._M_iter_list, __i);
    _Base::splice(__position._M_iterator, __x, __i._M_iterator);
  }

  void splice(iterator __position, _Self& __x, iterator __first, iterator __last) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __position))
    _STLP_VERBOSE_ASSERT(__first._Owner()==&__x._M_iter_list && __last._Owner()==&__x._M_iter_list, 
			 _StlMsg_NOT_OWNER)
    _Base::splice(__position._M_iterator, __x, __first._M_iterator, __last._M_iterator);
  }

  void merge(_Self& __x) {   
    __x._Invalidate_all();
    _Base::merge(__x); 
  }
  void reverse() {
    _Invalidate_all();
    _Base::reverse();
  }
  void unique() {
    _Invalidate_all();
    _Base::unique();
  }
  void sort() {
    _Invalidate_all();
    _Base::sort();
  }

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _Predicate> void remove_if(_Predicate __pred)  {
    _Base::remove_if(__pred);
  }
  template <class _BinaryPredicate>
    void unique(_BinaryPredicate __binary_pred) {
    _Base::unique(__binary_pred);
  }
  template <class _StrictWeakOrdering>
    void merge(_Self& __x,
	       _StrictWeakOrdering __comp) {
    __x._Invalidate_all();
    _Base::merge(__x, __comp);
  }

  template <class _StrictWeakOrdering>
  void sort(_StrictWeakOrdering __comp) {
      _Invalidate_all();
      _Base::sort(__comp);
  }
#endif /* _STLP_MEMBER_TEMPLATES */

};

#define _STLP_TEMPLATE_HEADER template <class _Tp, class _Alloc>
#define _STLP_TEMPLATE_CONTAINER _DBG_list<_Tp,_Alloc>
#define _STLP_TEMPLATE_CONTAINER_BASE _STLP_DBG_LIST_BASE
#include <stl/debug/_relops_cont.h>
#undef _STLP_TEMPLATE_CONTAINER_BASE
#undef _STLP_TEMPLATE_CONTAINER
#undef _STLP_TEMPLATE_HEADER

_STLP_END_NAMESPACE 

#endif /* _STLP_INTERNAL_LIST_H */

// Local Variables:
// mode:C++
// End:
