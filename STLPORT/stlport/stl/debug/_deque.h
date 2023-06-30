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

#ifndef _STLP_INTERNAL_DBG_DEQUE_H
#define _STLP_INTERNAL_DBG_DEQUE_H

#include <stl/debug/_iterator.h>

# if !defined (_STLP_USE_WRAPPER_FOR_ALLOC_PARAM) && !defined (_STLP_NO_DEFAULT_NON_TYPE_PARAM)
#  undef  _DBG_deque
#  define _DBG_deque deque
# endif

# define _DEQUE_WRAPPER _DBG_deque<_Tp,_Alloc>

# define _STLP_DEQUE_SUPER   __WORKAROUND_DBG_RENAME(deque) <_Tp,_Alloc>

_STLP_BEGIN_NAMESPACE

# ifdef _STLP_DEBUG_USE_DISTINCT_VALUE_TYPE_HELPERS
template <class _Tp, class _Alloc>
inline _Tp* value_type(const  _DBG_iter_base< _STLP_DEQUE_SUPER >&) {
  return (_Tp*)0;
}
template <class _Tp, class _Alloc>
inline random_access_iterator_tag iterator_category(const  _DBG_iter_base< _STLP_DEQUE_SUPER >&) {
  return random_access_iterator_tag();
}
# endif

template <class _Tp, _STLP_DBG_ALLOCATOR_SELECT(_Tp) >
class _DBG_deque : public _STLP_DEQUE_SUPER {

  typedef _DBG_deque<_Tp,_Alloc> _Self;
  typedef _STLP_DEQUE_SUPER _Base;

public:                         // Basic types

  __IMPORT_CONTAINER_TYPEDEFS(_Base)

public:                         // Iterators
  typedef _DBG_iter< _STLP_DEQUE_SUPER, _Nonconst_traits<value_type> >    iterator;
  typedef _DBG_iter< _STLP_DEQUE_SUPER, _Const_traits<value_type> >   const_iterator;

  _STLP_DECLARE_RANDOM_ACCESS_REVERSE_ITERATORS;

protected:
  __owned_list _M_iter_list;
  void _Invalidate_iterator(const iterator& __it) { 
    __invalidate_iterator(&_M_iter_list,__it);
  }
  void _Invalidate_all() {
      _M_iter_list._Invalidate_all();
  }

public:                         // Basic accessors
  iterator begin() { return iterator(&_M_iter_list, this->_M_start); }
  iterator end() { return iterator(&_M_iter_list, this->_M_finish); }
  const_iterator begin() const { 
    return const_iterator(&_M_iter_list, this->_M_start); 
  }
  const_iterator end() const { 
    return const_iterator(&_M_iter_list,  this->_M_finish); 
  }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rbegin() const 
    { return const_reverse_iterator(end()); }
  const_reverse_iterator rend() const 
    { return const_reverse_iterator(begin()); }

  reference operator[](size_type __n)
    { return begin()[difference_type(__n)]; }
  const_reference operator[](size_type __n) const 
    { return begin()[difference_type(__n)]; }

  reference front() { return *begin(); }
  reference back() {
    iterator __tmp = end();
    --__tmp;
    return *__tmp;
  }
  const_reference front() const { return *begin(); }
  const_reference back() const {
    const_iterator __tmp = end();
    --__tmp;
    return *__tmp;
  }

public:                         // Constructor, destructor.

  const _Base* _Get_base() const { return (const _Base*)this; }

  explicit _DBG_deque(const allocator_type& __a = allocator_type()) :
    _STLP_DEQUE_SUPER(__a), _M_iter_list(_Get_base()) {}
  _DBG_deque(const _Self& __x) : _STLP_DEQUE_SUPER(__x), _M_iter_list(_Get_base()) {}
  _DBG_deque(size_type __n, const value_type& __value,
        const allocator_type& __a = allocator_type()) : 
    _STLP_DEQUE_SUPER(__n, __value, __a), _M_iter_list(_Get_base()) {}
  explicit _DBG_deque(size_type __n) : _STLP_DEQUE_SUPER(__n), _M_iter_list(_Get_base()) {}

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  _DBG_deque(_InputIterator __first, _InputIterator __last,
        const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL) : 
    _STLP_DEQUE_SUPER(__first, __last, __a) , _M_iter_list(_Get_base()) {}
# ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  template <class _InputIterator>
  _DBG_deque(_InputIterator __first, _InputIterator __last):
    _STLP_DEQUE_SUPER(__first, __last, allocator_type()) , _M_iter_list(_Get_base()) {}
# endif
#else /* _STLP_MEMBER_TEMPLATES */
  _DBG_deque(const value_type* __first, const value_type* __last,
        const allocator_type& __a = allocator_type()) 
    : _STLP_DEQUE_SUPER(__first, __last, __a), _M_iter_list(_Get_base()) {}

  _DBG_deque(const_iterator __first, const_iterator __last,
        const allocator_type& __a = allocator_type()) 
    : _STLP_DEQUE_SUPER(__first._M_iterator, __last._M_iterator, __a), 
      _M_iter_list(_Get_base()) {}
#endif /* _STLP_MEMBER_TEMPLATES */

  _Self& operator= (const _Self& __x) {
    _Invalidate_all();
    (_Base&)*this = (const _Base&)__x; 
    return *this;
  }

  void swap(_Self& __x) {
    _M_iter_list._Swap_owners(__x._M_iter_list);
    _Base::swap(__x);
  }

public: 
  void assign(size_type __n, const _Tp& __val) {
    _Base::assign(__n, __val);
  }

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last) {
    _Base::assign(__first, __last);
  }
#endif /* _STLP_MEMBER_TEMPLATES */

public:                         // push_* and pop_*
  
  void push_back(const value_type& __t) {
    _Invalidate_all();
    _Base::push_back(__t);
  }

  void push_back() {
    _Invalidate_all();
    _Base::push_back();
  }

  void push_front(const value_type& __t) {
    _Invalidate_all();
    _Base::push_front(__t);
  }

  void push_front() {
    _Base::push_front();
    _Invalidate_all();
  }


  void pop_back() {
    _Invalidate_iterator(end());
    _Base::pop_back();
  }

  void pop_front() {
    _Invalidate_iterator(begin());        
    _Base::pop_front();
  }

public:                         // Insert

  iterator insert(iterator __position, const value_type& __x) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __position))
    // fbp : invalidation !
    return iterator(&_M_iter_list, _Base::insert(__position._M_iterator, __x));
  }

  iterator insert(iterator __position) { 
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __position))
    // fbp : invalidation !
    return iterator(&_M_iter_list, _Base::insert(__position._M_iterator));
  }

  void insert(iterator __position, size_type __n, const value_type& __x) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __position))
    // fbp : invalidation !
    _Base::insert(__position._M_iterator, __n, __x);
  }

#ifdef _STLP_MEMBER_TEMPLATES  
  template <class _InputIterator>
  void insert(iterator __position, _InputIterator __first, _InputIterator __last) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __position))
    // fbp : invalidation !
    _Base::insert(__position._M_iterator, __first, __last);
  }
#else /* _STLP_MEMBER_TEMPLATES */
  void insert(iterator __position,
              const value_type* __first, const value_type* __last) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __position))
    _Base::insert(__position._M_iterator, __first, __last);
  }
  void insert(iterator __position,
              const_iterator __first, const_iterator __last) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __position))
    _Base::insert(__position._M_iterator, __first._M_iterator, __last._M_iterator);
  }
#endif /* _STLP_MEMBER_TEMPLATES */

public:                         // Erase
  iterator erase(iterator __pos) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __pos) && (__pos != end()))
    return iterator (&_M_iter_list, _Base::erase(__pos._M_iterator));
  }

  iterator erase(iterator __first, iterator __last) {
    _STLP_DEBUG_CHECK(__first >= begin() && __last <= end())
    return iterator (&_M_iter_list, _Base::erase(__first._M_iterator, __last._M_iterator));
  }
  
  void clear() {
    _Invalidate_all();
    _Base::clear();
  }
};

#define _STLP_TEMPLATE_HEADER template <class _Tp, class _Alloc>
#define _STLP_TEMPLATE_CONTAINER _DBG_deque<_Tp, _Alloc>
#define _STLP_TEMPLATE_CONTAINER_BASE _STLP_DEQUE_SUPER
#include <stl/debug/_relops_cont.h>
#undef _STLP_TEMPLATE_CONTAINER_BASE
#undef _STLP_TEMPLATE_CONTAINER
#undef _STLP_TEMPLATE_HEADER

_STLP_END_NAMESPACE 

# undef _DBG_deque
# undef _STLP_DEQUE_SUPER
  
#endif /* _STLP_INTERNAL_DEQUE_H */

// Local Variables:
// mode:C++
// End:
