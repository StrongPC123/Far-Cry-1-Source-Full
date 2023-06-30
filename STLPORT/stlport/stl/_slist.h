/*
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

#ifndef _STLP_INTERNAL_SLIST_H
#define _STLP_INTERNAL_SLIST_H


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

# ifndef _STLP_INTERNAL_SLIST_BASE_H
#  include <stl/_slist_base.h>
# endif

# undef slist
# define  slist  __WORKAROUND_DBG_RENAME(slist)

_STLP_BEGIN_NAMESPACE 

template <class _Tp>
struct _Slist_node : public _Slist_node_base
{
  _Tp _M_data;
  __TRIVIAL_STUFF(_Slist_node)
};

struct _Slist_iterator_base {

  typedef size_t               size_type;
  typedef ptrdiff_t            difference_type;
  typedef forward_iterator_tag iterator_category;

  _Slist_node_base* _M_node;

  _Slist_iterator_base(_Slist_node_base* __x) : _M_node(__x) {}

  void _M_incr() { 
//    _STLP_VERBOSE_ASSERT(_M_node != 0, _StlMsg_INVALID_ADVANCE)
    _M_node = _M_node->_M_next; 
  }
  bool operator==(const _Slist_iterator_base& __y ) const { 
    return _M_node == __y._M_node; 
  }
  bool operator!=(const _Slist_iterator_base& __y ) const { 
    return _M_node != __y._M_node; 
  }
};

# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
inline ptrdiff_t* _STLP_CALL distance_type(const _Slist_iterator_base&) { return 0; }
inline forward_iterator_tag _STLP_CALL iterator_category(const _Slist_iterator_base&) { return forward_iterator_tag(); }
#endif

template <class _Tp, class _Traits>
struct _Slist_iterator : public _Slist_iterator_base
{
  typedef _Tp value_type;
  typedef typename _Traits::pointer    pointer;
  typedef typename _Traits::reference  reference;
  typedef forward_iterator_tag iterator_category;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  
  typedef _Slist_iterator<_Tp, _Nonconst_traits<_Tp> > iterator;
  typedef _Slist_iterator<_Tp, _Const_traits<_Tp> >    const_iterator;
  typedef _Slist_iterator<_Tp, _Traits>                       _Self;

  typedef _Slist_node<value_type> _Node;

  _Slist_iterator(_Node* __x) : _Slist_iterator_base(__x) {}
  _Slist_iterator() : _Slist_iterator_base(0) {}
  _Slist_iterator(const iterator& __x) : _Slist_iterator_base(__x._M_node) {}

  reference operator*() const { return ((_Node*) _M_node)->_M_data; }

  _STLP_DEFINE_ARROW_OPERATOR

  _Self& operator++()
  {
    _M_incr();
    return *this;
  }
  _Self operator++(int)
  {
    _Self __tmp = *this;
    _M_incr();
    return __tmp;
  }
};

#ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
template <class _Tp, class _Traits>
inline _Tp* _STLP_CALL value_type(const _Slist_iterator<_Tp, _Traits>&) { return (_Tp*)0; }
#endif /* OLD_QUERIES */

// Base class that encapsulates details of allocators and simplifies EH

template <class _Tp, class _Alloc> 
struct _Slist_base {
  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef typename _Alloc_traits<_Tp,_Alloc>::allocator_type allocator_type;
  typedef _Slist_node<_Tp> _Node;

  _Slist_base(const allocator_type& __a) : 
    _M_head(_STLP_CONVERT_ALLOCATOR(__a, _Node), _Slist_node_base() ) { 
    _M_head._M_data._M_next = 0; 
  }
  ~_Slist_base() { _M_erase_after(&_M_head._M_data, 0); }

protected:
  typedef typename _Alloc_traits<_Node,_Alloc>::allocator_type _M_node_allocator_type;

  _Slist_node_base* _M_erase_after(_Slist_node_base* __pos)
  {
    _Node* __next = (_Node*) (__pos->_M_next);
    _Slist_node_base* __next_next = __next->_M_next;
    __pos->_M_next = __next_next;
    _STLP_STD::_Destroy(&__next->_M_data);
    _M_head.deallocate(__next,1);
    return __next_next;
  }
  _Slist_node_base* _M_erase_after(_Slist_node_base*, _Slist_node_base*);

public:
  allocator_type get_allocator() const { 
    return _STLP_CONVERT_ALLOCATOR((const _M_node_allocator_type&)_M_head, _Tp); 
  }
  _STLP_alloc_proxy<_Slist_node_base, _Node, _M_node_allocator_type> _M_head;
};  

template <class _Tp, _STLP_DEFAULT_ALLOCATOR_SELECT(_Tp) >
class slist : protected _Slist_base<_Tp,_Alloc>
{
private:
  typedef _Slist_base<_Tp,_Alloc> _Base;
  typedef slist<_Tp,_Alloc> _Self;
public:
  typedef _Tp                value_type;
  typedef value_type*       pointer;
  typedef const value_type* const_pointer;
  typedef value_type&       reference;
  typedef const value_type& const_reference;
  typedef size_t            size_type;
  typedef ptrdiff_t         difference_type;
  typedef forward_iterator_tag _Iterator_category;

  typedef _Slist_iterator<_Tp, _Nonconst_traits<_Tp> >  iterator;
  typedef _Slist_iterator<_Tp, _Const_traits<_Tp> >     const_iterator;

  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef typename _Base::allocator_type allocator_type;


private:
  typedef _Slist_node<_Tp>      _Node;
  typedef _Slist_node_base      _Node_base;
  typedef _Slist_iterator_base  _Iterator_base;

  _Node* _M_create_node(const value_type& __x) {
    _Node* __node = this->_M_head.allocate(1);
    _STLP_TRY {
      _Construct(&__node->_M_data, __x);
      __node->_M_next = 0;
    }
    _STLP_UNWIND(this->_M_head.deallocate(__node, 1));
    return __node;
  }
  
  _Node* _M_create_node() {
    _Node* __node = this->_M_head.allocate(1);
    _STLP_TRY {
      _Construct(&__node->_M_data);
      __node->_M_next = 0;
    }
    _STLP_UNWIND(this->_M_head.deallocate(__node, 1));
    return __node;
  }

public:
  allocator_type get_allocator() const { return _Base::get_allocator(); }

  explicit slist(const allocator_type& __a = allocator_type()) : _Slist_base<_Tp,_Alloc>(__a) {}

  slist(size_type __n, const value_type& __x,
        const allocator_type& __a =  allocator_type()) : _Slist_base<_Tp,_Alloc>(__a)
    { _M_insert_after_fill(&this->_M_head._M_data, __n, __x); }

  explicit slist(size_type __n) : _Slist_base<_Tp,_Alloc>(allocator_type())
    { _M_insert_after_fill(&this->_M_head._M_data, __n, value_type()); }

#ifdef _STLP_MEMBER_TEMPLATES
  // We don't need any dispatching tricks here, because _M_insert_after_range
  // already does them.
  template <class _InputIterator>
  slist(_InputIterator __first, _InputIterator __last,
        const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL) : 
    _Slist_base<_Tp,_Alloc>(__a)
  { _M_insert_after_range(&this->_M_head._M_data, __first, __last); }
# ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  // VC++ needs this crazyness
  template <class _InputIterator>
  slist(_InputIterator __first, _InputIterator __last) :
    _Slist_base<_Tp,_Alloc>(allocator_type())
  { _M_insert_after_range(&this->_M_head._M_data, __first, __last); }
# endif  
#else /* _STLP_MEMBER_TEMPLATES */
  slist(const_iterator __first, const_iterator __last,
        const allocator_type& __a =  allocator_type() ) :
    _Slist_base<_Tp,_Alloc>(__a)
    { _M_insert_after_range(&this->_M_head._M_data, __first, __last); }
  slist(const value_type* __first, const value_type* __last,
        const allocator_type& __a =  allocator_type()) : 
    _Slist_base<_Tp,_Alloc>(__a)
    { _M_insert_after_range(&this->_M_head._M_data, __first, __last); }
#endif /* _STLP_MEMBER_TEMPLATES */

  slist(const _Self& __x) : _Slist_base<_Tp,_Alloc>(__x.get_allocator())
    { _M_insert_after_range(&this->_M_head._M_data, __x.begin(), __x.end()); }

  _Self& operator= (const _Self& __x);

  ~slist() {}

public:
  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.

  void assign(size_type __n, const _Tp& __val)
    { _M_fill_assign(__n, __val); }

  void _M_fill_assign(size_type __n, const _Tp& __val);

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }

  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val, const __true_type&)
    { _M_fill_assign((size_type) __n, (_Tp) __val); }

  template <class _InputIter>
  void
  _M_assign_dispatch(_InputIter __first, _InputIter __last,
		     const __false_type&) {
    _Node_base* __prev = &this->_M_head._M_data;
    _Node* __node = (_Node*) this->_M_head._M_data._M_next;
    while (__node != 0 && __first != __last) {
      __node->_M_data = *__first;
      __prev = __node;
      __node = (_Node*) __node->_M_next;
      ++__first;
    }
    if (__first != __last)
      _M_insert_after_range(__prev, __first, __last);
    else
      this->_M_erase_after(__prev, 0);
  }
#endif /* _STLP_MEMBER_TEMPLATES */

public:

  // Experimental new feature: before_begin() returns a
  // non-dereferenceable iterator that, when incremented, yields
  // begin().  This iterator may be used as the argument to
  // insert_after, erase_after, etc.  Note that even for an empty 
  // slist, before_begin() is not the same iterator as end().  It 
  // is always necessary to increment before_begin() at least once to
  // obtain end().
  iterator before_begin() { return iterator((_Node*) &this->_M_head._M_data); }
  const_iterator before_begin() const
    { return const_iterator((_Node*) &this->_M_head._M_data); }

  iterator begin() { return iterator((_Node*)this->_M_head._M_data._M_next); }
  const_iterator begin() const 
    { return const_iterator((_Node*)this->_M_head._M_data._M_next);}

  iterator end() { return iterator(0); }
  const_iterator end() const { return const_iterator(0); }

  size_type size() const { return _Sl_global_inst::size(this->_M_head._M_data._M_next); }
  
  size_type max_size() const { return size_type(-1); }

  bool empty() const { return this->_M_head._M_data._M_next == 0; }

  void swap(_Self& __x) { 
    _STLP_STD::swap(this->_M_head, __x._M_head); 
  }

public:
  reference front() { return ((_Node*) this->_M_head._M_data._M_next)->_M_data; }
  const_reference front() const 
    { return ((_Node*) this->_M_head._M_data._M_next)->_M_data; }
  void push_front(const value_type& __x)   {
    __slist_make_link(&this->_M_head._M_data, _M_create_node(__x));
  }

# ifndef _STLP_NO_ANACHRONISMS
  void push_front() { __slist_make_link(&this->_M_head._M_data, _M_create_node());}
# endif

  void pop_front() {
    _Node* __node = (_Node*) this->_M_head._M_data._M_next;
    this->_M_head._M_data._M_next = __node->_M_next;
    _STLP_STD::_Destroy(&__node->_M_data);
    this->_M_head.deallocate(__node, 1);
  }

  iterator previous(const_iterator __pos) {
    return iterator((_Node*) _Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node));
  }
  const_iterator previous(const_iterator __pos) const {
    return const_iterator((_Node*) _Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node));
  }

private:
  _Node* _M_insert_after(_Node_base* __pos, const value_type& __x) {
    return (_Node*) (__slist_make_link(__pos, _M_create_node(__x)));
  }

  _Node* _M_insert_after(_Node_base* __pos) {
    return (_Node*) (__slist_make_link(__pos, _M_create_node()));
  }

  void _M_insert_after_fill(_Node_base* __pos,
                            size_type __n, const value_type& __x) {
    for (size_type __i = 0; __i < __n; ++__i)
      __pos = __slist_make_link(__pos, _M_create_node(__x));
  }

#ifdef _STLP_MEMBER_TEMPLATES

  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InIter>
  void _M_insert_after_range(_Node_base* __pos, 
                             _InIter __first, _InIter __last) {
    typedef typename _Is_integer<_InIter>::_Integral _Integral;
    _M_insert_after_range(__pos, __first, __last, _Integral());
  }

  template <class _Integer>
  void _M_insert_after_range(_Node_base* __pos, _Integer __n, _Integer __x,
                             const __true_type&) {
    _M_insert_after_fill(__pos, __n, __x);
  }

  template <class _InIter>
  void _M_insert_after_range(_Node_base* __pos,
                             _InIter __first, _InIter __last,
                             const __false_type&) {
    while (__first != __last) {
      __pos = __slist_make_link(__pos, _M_create_node(*__first));
      ++__first;
    }
  }

#else /* _STLP_MEMBER_TEMPLATES */

  void _M_insert_after_range(_Node_base* __pos,
                             const_iterator __first, const_iterator __last) {
    while (__first != __last) {
      __pos = __slist_make_link(__pos, _M_create_node(*__first));
      ++__first;
    }
  }
  void _M_insert_after_range(_Node_base* __pos,
                             const value_type* __first,
                             const value_type* __last) {
    while (__first != __last) {
      __pos = __slist_make_link(__pos, _M_create_node(*__first));
      ++__first;
    }
  }

#endif /* _STLP_MEMBER_TEMPLATES */

public:

  iterator insert_after(iterator __pos, const value_type& __x) {
    return iterator(_M_insert_after(__pos._M_node, __x));
  }

  iterator insert_after(iterator __pos) {
    return insert_after(__pos, value_type());
  }

  void insert_after(iterator __pos, size_type __n, const value_type& __x) {
    _M_insert_after_fill(__pos._M_node, __n, __x);
  }

#ifdef _STLP_MEMBER_TEMPLATES

  // We don't need any dispatching tricks here, because _M_insert_after_range
  // already does them.
  template <class _InIter>
  void insert_after(iterator __pos, _InIter __first, _InIter __last) {
    _M_insert_after_range(__pos._M_node, __first, __last);
  }

#else /* _STLP_MEMBER_TEMPLATES */

  void insert_after(iterator __pos,
                    const_iterator __first, const_iterator __last) {
    _M_insert_after_range(__pos._M_node, __first, __last);
  }
  void insert_after(iterator __pos,
                    const value_type* __first, const value_type* __last) {
    _M_insert_after_range(__pos._M_node, __first, __last);
  }

#endif /* _STLP_MEMBER_TEMPLATES */

  iterator insert(iterator __pos, const value_type& __x) {
    return iterator(_M_insert_after(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node),
                    __x));
  }

  iterator insert(iterator __pos) {
    return iterator(_M_insert_after(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node),
                                    value_type()));
  }

  void insert(iterator __pos, size_type __n, const value_type& __x) {
    _M_insert_after_fill(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node), __n, __x);
  } 
    
#ifdef _STLP_MEMBER_TEMPLATES

  // We don't need any dispatching tricks here, because _M_insert_after_range
  // already does them.
  template <class _InIter>
  void insert(iterator __pos, _InIter __first, _InIter __last) {
    _M_insert_after_range(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node), 
                          __first, __last);
  }

#else /* _STLP_MEMBER_TEMPLATES */

  void insert(iterator __pos, const_iterator __first, const_iterator __last) {
    _M_insert_after_range(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node), 
                          __first, __last);
  }
  void insert(iterator __pos, const value_type* __first, 
                              const value_type* __last) {
    _M_insert_after_range(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node), 
                          __first, __last);
  }

#endif /* _STLP_MEMBER_TEMPLATES */


public:
  iterator erase_after(iterator __pos) {
    return iterator((_Node*) this->_M_erase_after(__pos._M_node));
  }
  iterator erase_after(iterator __before_first, iterator __last) {
    return iterator((_Node*) this->_M_erase_after(__before_first._M_node, 
                                            __last._M_node));
  } 

  iterator erase(iterator __pos) {
    return iterator((_Node*) this->_M_erase_after(_Sl_global_inst::__previous(&this->_M_head._M_data, 
                                                    __pos._M_node)));
  }
  iterator erase(iterator __first, iterator __last) {
    return iterator((_Node*) this->_M_erase_after(
      _Sl_global_inst::__previous(&this->_M_head._M_data, __first._M_node), __last._M_node));
  }

  void resize(size_type new_size, const _Tp& __x);
  void resize(size_type new_size) { resize(new_size, _Tp()); }
  void clear() {
    this->_M_erase_after(&this->_M_head._M_data, 0); 
  }

public:
  // Moves the range [__before_first + 1, __before_last + 1) to *this,
  //  inserting it immediately after __pos.  This is constant time.
  void splice_after(iterator __pos, 
                    iterator __before_first, iterator __before_last)
  {
    if (__before_first != __before_last) {
      _Sl_global_inst::__splice_after(__pos._M_node, __before_first._M_node, 
                           __before_last._M_node);
    }
  }

  // Moves the element that follows __prev to *this, inserting it immediately
  //  after __pos.  This is constant time.
  void splice_after(iterator __pos, iterator __prev)
  {
    _Sl_global_inst::__splice_after(__pos._M_node,
                         __prev._M_node, __prev._M_node->_M_next);
  }

  // Removes all of the elements from the list __x to *this, inserting
  // them immediately after __pos.  __x must not be *this.  Complexity:
  // linear in __x.size().
  void splice_after(iterator __pos, _Self& __x)
  {
    _Sl_global_inst::__splice_after(__pos._M_node, &__x._M_head._M_data);
  }

  // Linear in distance(begin(), __pos), and linear in __x.size().
  void splice(iterator __pos, _Self& __x) {
    if (__x._M_head._M_data._M_next)
      _Sl_global_inst::__splice_after(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node),
                           &__x._M_head._M_data, _Sl_global_inst::__previous(&__x._M_head._M_data, 0));
  }

  // Linear in distance(begin(), __pos), and in distance(__x.begin(), __i).
  void splice(iterator __pos, _Self& __x, iterator __i) {
    _Sl_global_inst::__splice_after(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node),
                         _Sl_global_inst::__previous(&__x._M_head._M_data, __i._M_node),
                         __i._M_node);
  }

  // Linear in distance(begin(), __pos), in distance(__x.begin(), __first),
  // and in distance(__first, __last).
  void splice(iterator __pos, _Self& __x, iterator __first, iterator __last)
  {
    if (__first != __last)
      _Sl_global_inst::__splice_after(_Sl_global_inst::__previous(&this->_M_head._M_data, __pos._M_node),
                           _Sl_global_inst::__previous(&__x._M_head._M_data, __first._M_node),
                           _Sl_global_inst::__previous(__first._M_node, __last._M_node));
  }

public:
  void reverse() { 
    if (this->_M_head._M_data._M_next)
      this->_M_head._M_data._M_next = _Sl_global_inst::__reverse(this->_M_head._M_data._M_next);
  }

  void remove(const _Tp& __val); 
  void unique(); 
  void merge(_Self& __x);
  void sort();     

#ifdef _STLP_MEMBER_TEMPLATES
  template <class _Predicate>
  void remove_if(_Predicate __pred) {
    _Node_base* __cur = &this->_M_head._M_data;
    while (__cur->_M_next) {
      if (__pred(((_Node*) __cur->_M_next)->_M_data))
	this->_M_erase_after(__cur);
      else
	__cur = __cur->_M_next;
    }
  }

  template <class _BinaryPredicate> 
  void unique(_BinaryPredicate __pred) {
    _Node* __cur = (_Node*) this->_M_head._M_data._M_next;
    if (__cur) {
      while (__cur->_M_next) {
	if (__pred(((_Node*)__cur)->_M_data, 
		   ((_Node*)(__cur->_M_next))->_M_data))
	  this->_M_erase_after(__cur);
	else
	  __cur = (_Node*) __cur->_M_next;
      }
    }
  }

  template <class _StrictWeakOrdering>
  void merge(slist<_Tp,_Alloc>& __x,
	     _StrictWeakOrdering __comp) {
    _Node_base* __n1 = &this->_M_head._M_data;
    while (__n1->_M_next && __x._M_head._M_data._M_next) {
      if (__comp(((_Node*) __x._M_head._M_data._M_next)->_M_data,
		 ((_Node*)       __n1->_M_next)->_M_data))
	_Sl_global_inst::__splice_after(__n1, &__x._M_head._M_data, __x._M_head._M_data._M_next);
      __n1 = __n1->_M_next;
    }
    if (__x._M_head._M_data._M_next) {
      __n1->_M_next = __x._M_head._M_data._M_next;
      __x._M_head._M_data._M_next = 0;
    }
  }

  template <class _StrictWeakOrdering> 
  void sort(_StrictWeakOrdering __comp) {
    if (this->_M_head._M_data._M_next && this->_M_head._M_data._M_next->_M_next) {
      slist __carry;
      slist __counter[64];
      int __fill = 0;
      while (!empty()) {
	_Sl_global_inst::__splice_after(&__carry._M_head._M_data, &this->_M_head._M_data, this->_M_head._M_data._M_next);
	int __i = 0;
	while (__i < __fill && !__counter[__i].empty()) {
	  __counter[__i].merge(__carry, __comp);
	  __carry.swap(__counter[__i]);
	  ++__i;
	}
	__carry.swap(__counter[__i]);
	if (__i == __fill)
	  ++__fill;
      }
      
      for (int __i = 1; __i < __fill; ++__i)
	__counter[__i].merge(__counter[__i-1], __comp);
      this->swap(__counter[__fill-1]);
    }
  }
#endif /* _STLP_MEMBER_TEMPLATES */

};

template <class _Tp, class _Alloc>
inline bool  _STLP_CALL
operator==(const slist<_Tp,_Alloc>& _SL1, const slist<_Tp,_Alloc>& _SL2)
{
  typedef typename slist<_Tp,_Alloc>::const_iterator const_iterator;
  const_iterator __end1 = _SL1.end();
  const_iterator __end2 = _SL2.end();

  const_iterator __i1 = _SL1.begin();
  const_iterator __i2 = _SL2.begin();
  while (__i1 != __end1 && __i2 != __end2 && *__i1 == *__i2) {
    ++__i1;
    ++__i2;
   }
  return __i1 == __end1 && __i2 == __end2;
}

# define _STLP_EQUAL_OPERATOR_SPECIALIZED
# define _STLP_TEMPLATE_HEADER    template <class _Tp, class _Alloc>
# define _STLP_TEMPLATE_CONTAINER slist<_Tp, _Alloc>
# include <stl/_relops_cont.h>
# undef _STLP_TEMPLATE_CONTAINER
# undef _STLP_TEMPLATE_HEADER
# undef _STLP_EQUAL_OPERATOR_SPECIALIZED

_STLP_END_NAMESPACE

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_slist.c>
# endif

#  undef  slist
#  define __slist__ __FULL_NAME(slist)

#if defined (_STLP_DEBUG) && !defined (_STLP_INTERNAL_DBG_SLIST_H)
# include <stl/debug/_slist.h>
#endif

_STLP_BEGIN_NAMESPACE
// Specialization of insert_iterator so that insertions will be constant
// time rather than linear time.

#ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION

template <class _Tp, class _Alloc>
class insert_iterator<slist<_Tp, _Alloc> > {
protected:
  typedef slist<_Tp, _Alloc> _Container;
  _Container* container;
  typename _Container::iterator iter;
public:
  typedef _Container          container_type;
  typedef output_iterator_tag iterator_category;
  typedef void                value_type;
  typedef void                difference_type;
  typedef void                pointer;
  typedef void                reference;

  insert_iterator(_Container& __x, typename _Container::iterator __i) 
    : container(&__x) {
    if (__i == __x.begin())
      iter = __x.before_begin();
    else
      iter = __x.previous(__i);
  }

  insert_iterator<_Container>&
  operator=(const typename _Container::value_type& __val) { 
    iter = container->insert_after(iter, __val);
    return *this;
  }
  insert_iterator<_Container>& operator*() { return *this; }
  insert_iterator<_Container>& operator++() { return *this; }
  insert_iterator<_Container>& operator++(int) { return *this; }
};

#endif /* _STLP_CLASS_PARTIAL_SPECIALIZATION */

_STLP_END_NAMESPACE


# if defined ( _STLP_USE_WRAPPER_FOR_ALLOC_PARAM )
# include <stl/wrappers/_slist.h>
# endif

#endif /* _STLP_INTERNAL_SLIST_H */

// Local Variables:
// mode:C++
// End:

