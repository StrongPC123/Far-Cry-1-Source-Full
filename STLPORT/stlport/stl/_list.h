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

#ifndef _STLP_INTERNAL_LIST_H
#define _STLP_INTERNAL_LIST_H

# ifndef _STLP_INTERNAL_ALGOBASE_H
#  include <stl/_algobase.h>
# endif

# ifndef _STLP_INTERNAL_ALLOC_H
#  include <stl/_alloc.h>
# endif

# ifndef _STLP_INTERNAL_ITERATOR_H
#  include <stl/_iterator.h>
# endif

# ifndef _STLP_INTERNAL_CONSTRUCT_H
#  include <stl/_construct.h>
# endif

# ifndef _STLP_INTERNAL_FUNCTION_BASE_H
#  include <stl/_function_base.h>
# endif

_STLP_BEGIN_NAMESPACE

# undef list
# define  list  __WORKAROUND_DBG_RENAME(list)

struct _List_node_base {
  _List_node_base* _M_next;
  _List_node_base* _M_prev;
};

template <class _Dummy>
class _List_global {
public:
  typedef _List_node_base _Node;
  static void  _STLP_CALL _Transfer(_List_node_base* __position, 
                                    _List_node_base* __first, _List_node_base* __last);
};

# if defined (_STLP_USE_TEMPLATE_EXPORT) 
_STLP_EXPORT_TEMPLATE_CLASS _List_global<bool>;
# endif
typedef _List_global<bool> _List_global_inst;

template <class _Tp>
struct _List_node : public _List_node_base {
  _Tp _M_data;
  __TRIVIAL_STUFF(_List_node)

#ifdef __DMC__
  // for some reason, Digital Mars C++ needs a constructor...
 private:
  _List_node();
#endif
};

struct _List_iterator_base {
  typedef size_t                     size_type;
  typedef ptrdiff_t                  difference_type;
  typedef bidirectional_iterator_tag iterator_category;

  _List_node_base* _M_node;

  _List_iterator_base(_List_node_base* __x) : _M_node(__x) {}
  _List_iterator_base() {}

  void _M_incr() { _M_node = _M_node->_M_next; }
  void _M_decr() { _M_node = _M_node->_M_prev; }
  bool operator==(const _List_iterator_base& __y ) const { 
    return _M_node == __y._M_node; 
  }
  bool operator!=(const _List_iterator_base& __y ) const { 
    return _M_node != __y._M_node; 
  }
};  




template<class _Tp, class _Traits>
struct _List_iterator : public _List_iterator_base {
  typedef _Tp value_type;
  typedef typename _Traits::pointer    pointer;
  typedef typename _Traits::reference  reference;

  typedef _List_iterator<_Tp, _Nonconst_traits<_Tp> > iterator;
  typedef _List_iterator<_Tp, _Const_traits<_Tp> >    const_iterator;
  typedef _List_iterator<_Tp, _Traits>                       _Self;

  typedef bidirectional_iterator_tag iterator_category;
  typedef _List_node<_Tp> _Node;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  _List_iterator(_Node* __x) : _List_iterator_base(__x) {}
  _List_iterator() {}
  _List_iterator(const iterator& __x) :  _List_iterator_base(__x._M_node) {}

  reference operator*() const { return ((_Node*)_M_node)->_M_data; }

  _STLP_DEFINE_ARROW_OPERATOR

  _Self& operator++() { 
    this->_M_incr();
    return *this;
  }
  _Self operator++(int) { 
    _Self __tmp = *this;
    this->_M_incr();
    return __tmp;
  }
  _Self& operator--() { 
    this->_M_decr();
    return *this;
  }
  _Self operator--(int) { 
    _Self __tmp = *this;
    this->_M_decr();
    return __tmp;
  }
};


#ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
template <class _Tp, class _Traits>
inline _Tp* value_type(const _List_iterator<_Tp, _Traits>&) { return 0; }
inline bidirectional_iterator_tag iterator_category(const _List_iterator_base&) { return bidirectional_iterator_tag();}
inline ptrdiff_t* distance_type(const _List_iterator_base&) { return 0; }
#endif


// Base class that encapsulates details of allocators and helps 
// to simplify EH

template <class _Tp, class _Alloc>
class _List_base 
{
protected:
  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef _List_node<_Tp> _Node;
  typedef typename _Alloc_traits<_Node, _Alloc>::allocator_type
           _Node_allocator_type;
public:
  typedef typename _Alloc_traits<_Tp, _Alloc>::allocator_type
          allocator_type;

  allocator_type get_allocator() const { 
    return _STLP_CONVERT_ALLOCATOR((const _Node_allocator_type&)_M_node, _Tp);
  }

  _List_base(const allocator_type& __a) : _M_node(_STLP_CONVERT_ALLOCATOR(__a, _Node), (_Node*)0) {
    _Node* __n = _M_node.allocate(1);
    __n->_M_next = __n;
    __n->_M_prev = __n;
    _M_node._M_data = __n;
  }
  ~_List_base() {
    clear();
    _M_node.deallocate(_M_node._M_data, 1);
  }

  void clear();

public:
  _STLP_alloc_proxy<_Node*, _Node, _Node_allocator_type>  _M_node;
};

template <class _Tp, _STLP_DEFAULT_ALLOCATOR_SELECT(_Tp) >
class list;

// helper functions to reduce code duplication
template <class _Tp, class _Alloc, class _Predicate> 
void _S_remove_if(list<_Tp, _Alloc>& __that, _Predicate __pred);

template <class _Tp, class _Alloc, class _BinaryPredicate>
void _S_unique(list<_Tp, _Alloc>& __that, _BinaryPredicate __binary_pred);

template <class _Tp, class _Alloc, class _StrictWeakOrdering>
void _S_merge(list<_Tp, _Alloc>& __that, list<_Tp, _Alloc>& __x,
	      _StrictWeakOrdering __comp);

template <class _Tp, class _Alloc, class _StrictWeakOrdering>
void _S_sort(list<_Tp, _Alloc>& __that, _StrictWeakOrdering __comp);

template <class _Tp, class _Alloc>
class list : public _List_base<_Tp, _Alloc> {
  typedef _List_base<_Tp, _Alloc> _Base;
  typedef list<_Tp, _Alloc> _Self;
public:      
  typedef _Tp value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef _List_node<_Tp> _Node;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef typename _Base::allocator_type allocator_type;
  typedef bidirectional_iterator_tag _Iterator_category;

public:
  typedef _List_iterator<_Tp, _Nonconst_traits<_Tp> > iterator;
  typedef _List_iterator<_Tp, _Const_traits<_Tp> >    const_iterator;
  _STLP_DECLARE_BIDIRECTIONAL_REVERSE_ITERATORS;

protected:
  _Node* _M_create_node(const _Tp& __x)
  {
    _Node* __p = this->_M_node.allocate(1);
    _STLP_TRY {
      _STLP_STD::_Construct(&__p->_M_data, __x);
    }
    _STLP_UNWIND(this->_M_node.deallocate(__p, 1));
    return __p;
  }

  _Node* _M_create_node()
  {
    _Node* __p = this->_M_node.allocate(1);
    _STLP_TRY {
      _STLP_STD::_Construct(&__p->_M_data);
    }
    _STLP_UNWIND(this->_M_node.deallocate(__p, 1));
    return __p;
  }

public:
# if !(defined(__MRC__)||(defined(__SC__) && !defined(__DMC__)))
  explicit
# endif
  list(const allocator_type& __a = allocator_type()) :
    _List_base<_Tp, _Alloc>(__a) {}

  iterator begin()             { return iterator((_Node*)(this->_M_node._M_data->_M_next)); }
  const_iterator begin() const { return const_iterator((_Node*)(this->_M_node._M_data->_M_next)); }

  iterator end()             { return this->_M_node._M_data; }
  const_iterator end() const { return this->_M_node._M_data; }

  reverse_iterator rbegin() 
    { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const 
    { return const_reverse_iterator(end()); }

  reverse_iterator rend()
    { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const
    { return const_reverse_iterator(begin()); }

  bool empty() const { return this->_M_node._M_data->_M_next == this->_M_node._M_data; }
  size_type size() const {
    size_type __result = distance(begin(), end());
    return __result;
  }
  size_type max_size() const { return size_type(-1); }

  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }
  reference back() { return *(--end()); }
  const_reference back() const { return *(--end()); }

  void swap(list<_Tp, _Alloc>& __x) {
    _STLP_STD::swap(this->_M_node, __x._M_node); 
  }

  iterator insert(iterator __position, const _Tp& __x) {

    _Node* __tmp = _M_create_node(__x);
    _List_node_base* __n = __position._M_node;
    _List_node_base* __p = __n->_M_prev;
    __tmp->_M_next = __n;
    __tmp->_M_prev = __p;
    __p->_M_next = __tmp;
    __n->_M_prev = __tmp;
    return __tmp;
  }

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  void insert(iterator __pos, _InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_insert_dispatch(__pos, __first, __last, _Integral());
  }
  // Check whether it's an integral type.  If so, it's not an iterator.
  template<class _Integer>
  void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __x,
                          const __true_type&) {
    _M_fill_insert(__pos, (size_type) __n, (_Tp) __x);
  }
  template <class _InputIter>
  void 
  _M_insert_dispatch(iterator __position,
		     _InputIter __first, _InputIter __last,
		     const __false_type&) 
#else /* _STLP_MEMBER_TEMPLATES */
  void insert(iterator __position, const _Tp* __first, const _Tp* __last) {
    for ( ; __first != __last; ++__first)
      insert(__position, *__first);
  }
  void insert(iterator __position, const_iterator __first, const_iterator __last)
#endif /* _STLP_MEMBER_TEMPLATES */
  {
    for ( ; __first != __last; ++__first)
      insert(__position, *__first);
  }
  void insert(iterator __pos, size_type __n, const _Tp& __x) { _M_fill_insert(__pos, __n, __x); }
  
  void _M_fill_insert(iterator __pos, size_type __n, const _Tp& __x) {
    for ( ; __n > 0; --__n)
      insert(__pos, __x);
  } 
  void push_front(const _Tp& __x) { insert(begin(), __x); }
  void push_back(const _Tp& __x) { insert(end(), __x); }

# ifndef _STLP_NO_ANACHRONISMS
  iterator insert(iterator __position) { return insert(__position, _Tp()); }
  void push_front() {insert(begin());}
  void push_back() {insert(end());}
# endif

  iterator erase(iterator __position) {
    _List_node_base* __next_node = __position._M_node->_M_next;
    _List_node_base* __prev_node = __position._M_node->_M_prev;
    _Node* __n = (_Node*) __position._M_node;
    __prev_node->_M_next = __next_node;
    __next_node->_M_prev = __prev_node;
    _STLP_STD::_Destroy(&__n->_M_data);
    this->_M_node.deallocate(__n, 1);
    return iterator((_Node*)__next_node);
    }
  
  iterator erase(iterator __first, iterator __last) {
    while (__first != __last)
      erase(__first++);
    return __last;
  }

  void resize(size_type __new_size, _Tp __x);
  void resize(size_type __new_size) { this->resize(__new_size, _Tp()); }

  void pop_front() { erase(begin()); }
  void pop_back() { 
    iterator __tmp = end();
    erase(--__tmp);
  }
  list(size_type __n, const _Tp& __val,
       const allocator_type& __a = allocator_type())
    : _List_base<_Tp, _Alloc>(__a)
    { this->insert(begin(), __n, __val); }
  explicit list(size_type __n)
    : _List_base<_Tp, _Alloc>(allocator_type())
    { this->insert(begin(), __n, _Tp()); }

#ifdef _STLP_MEMBER_TEMPLATES
  // We don't need any dispatching tricks here, because insert does all of
  // that anyway.
# ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  template <class _InputIterator>
  list(_InputIterator __first, _InputIterator __last)
    : _List_base<_Tp, _Alloc>(allocator_type())
  { insert(begin(), __first, __last); }
# endif  
  template <class _InputIterator>
  list(_InputIterator __first, _InputIterator __last,
       const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL)
    : _List_base<_Tp, _Alloc>(__a)
  { insert(begin(), __first, __last); }
  
#else /* _STLP_MEMBER_TEMPLATES */

  list(const _Tp* __first, const _Tp* __last,
       const allocator_type& __a = allocator_type())
    : _List_base<_Tp, _Alloc>(__a)
    { insert(begin(), __first, __last); }
  list(const_iterator __first, const_iterator __last,
       const allocator_type& __a = allocator_type())
    : _List_base<_Tp, _Alloc>(__a)
    { insert(begin(), __first, __last); }

#endif /* _STLP_MEMBER_TEMPLATES */
  list(const list<_Tp, _Alloc>& __x) : _List_base<_Tp, _Alloc>(__x.get_allocator())
    { insert(begin(), __x.begin(), __x.end()); }

  ~list() { }

  list<_Tp, _Alloc>& operator=(const list<_Tp, _Alloc>& __x);

public:
  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.

  void assign(size_type __n, const _Tp& __val) { _M_fill_assign(__n, __val); }

  void _M_fill_assign(size_type __n, const _Tp& __val);

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }

  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val, const __true_type&)
    { assign((size_type) __n, (_Tp) __val); }

  template <class _InputIterator>
  void _M_assign_dispatch(_InputIterator __first2, _InputIterator __last2,
                          const __false_type&) {
    iterator __first1 = begin();
    iterator __last1 = end();
    for ( ; __first1 != __last1 && __first2 != __last2; ++__first1, ++__first2)
      *__first1 = *__first2;
    if (__first2 == __last2)
      erase(__first1, __last1);
    else
      insert(__last1, __first2, __last2);
  }

#endif /* _STLP_MEMBER_TEMPLATES */

public:
  void splice(iterator __position, _Self& __x) {
    if (!__x.empty()) 
      _List_global_inst::_Transfer(__position._M_node, __x.begin()._M_node, __x.end()._M_node);
  }
  void splice(iterator __position, _Self&, iterator __i) {
    iterator __j = __i;
    ++__j;
    if (__position == __i || __position == __j) return;
    _List_global_inst::_Transfer(__position._M_node, __i._M_node, __j._M_node);
  }
  void splice(iterator __position, _Self&, iterator __first, iterator __last) {
    if (__first != __last) 
      _List_global_inst::_Transfer(__position._M_node, __first._M_node, __last._M_node);
  }

  void remove(const _Tp& __val) {
    iterator __first = begin();
    iterator __last = end();
    while (__first != __last) {
      iterator __next = __first;
      ++__next;
      if (__val == *__first) erase(__first);
      __first = __next;
    }
  }
  
  void unique() {
    _S_unique(*this, equal_to<_Tp>());
  }
  
  void merge(_Self& __x) {
    _S_merge(*this, __x, less<_Tp>());
  }

  void reverse() {
    _List_node_base* __p = this->_M_node._M_data;
    _List_node_base* __tmp = __p;
    do {
      _STLP_STD::swap(__tmp->_M_next, __tmp->_M_prev);
      __tmp = __tmp->_M_prev;     // Old next node is now prev.
    } while (__tmp != __p);
  }    
  
  void sort() {
    _S_sort(*this, less<_Tp>());
  }

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _Predicate> void remove_if(_Predicate __pred)  {
    _S_remove_if(*this, __pred);
  }
  template <class _BinaryPredicate>
    void unique(_BinaryPredicate __binary_pred) {
    _S_unique(*this, __binary_pred);
  }

  template <class _StrictWeakOrdering>
    void merge(list<_Tp, _Alloc>& __x,
	  _StrictWeakOrdering __comp) {
    _S_merge(*this, __x, __comp);
  }

  template <class _StrictWeakOrdering>
    void sort(_StrictWeakOrdering __comp) {
    _S_sort(*this, __comp);
  }
#endif /* _STLP_MEMBER_TEMPLATES */

};

template <class _Tp, class _Alloc>
_STLP_INLINE_LOOP bool  _STLP_CALL
operator==(const list<_Tp,_Alloc>& __x, const list<_Tp,_Alloc>& __y)
{
  typedef typename list<_Tp,_Alloc>::const_iterator const_iterator;
  const_iterator __end1 = __x.end();
  const_iterator __end2 = __y.end();

  const_iterator __i1 = __x.begin();
  const_iterator __i2 = __y.begin();
  while (__i1 != __end1 && __i2 != __end2 && *__i1 == *__i2) {
    ++__i1;
    ++__i2;
  }
  return __i1 == __end1 && __i2 == __end2;
}

# define _STLP_EQUAL_OPERATOR_SPECIALIZED
# define _STLP_TEMPLATE_HEADER    template <class _Tp, class _Alloc>
# define _STLP_TEMPLATE_CONTAINER list<_Tp, _Alloc>
# include <stl/_relops_cont.h>
# undef _STLP_TEMPLATE_CONTAINER
# undef _STLP_TEMPLATE_HEADER
# undef _STLP_EQUAL_OPERATOR_SPECIALIZED

_STLP_END_NAMESPACE 

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_list.c>
# endif

// do a cleanup
# undef list
# define __list__ __FULL_NAME(list)

#if defined (_STLP_DEBUG)
# include <stl/debug/_list.h>
#endif

#if defined (_STLP_USE_WRAPPER_FOR_ALLOC_PARAM)
# include <stl/wrappers/_list.h>
#endif

#endif /* _STLP_INTERNAL_LIST_H */

// Local Variables:
// mode:C++
// End:
