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

#ifndef _STLP_INTERNAL_DEQUE_H
#define _STLP_INTERNAL_DEQUE_H

# ifndef _STLP_INTERNAL_ALGOBASE_H
#  include <stl/_algobase.h>
# endif

# ifndef _STLP_INTERNAL_ALLOC_H
#  include <stl/_alloc.h>
# endif

# ifndef _STLP_INTERNAL_ITERATOR_H
#  include <stl/_iterator.h>
# endif

# ifndef _STLP_INTERNAL_UNINITIALIZED_H
#  include <stl/_uninitialized.h>
# endif

# ifndef _STLP_RANGE_ERRORS_H
#  include <stl/_range_errors.h>
# endif

/* Class invariants:
 *  For any nonsingular iterator i:
 *    i.node is the address of an element in the map array.  The
 *      contents of i.node is a pointer to the beginning of a node.
 *    i.first == *(i.node) 
 *    i.last  == i.first + node_size
 *    i.cur is a pointer in the range [i.first, i.last).  NOTE:
 *      the implication of this is that i.cur is always a dereferenceable
 *      pointer, even if i is a past-the-end iterator.
 *  Start and Finish are always nonsingular iterators.  NOTE: this means
 *    that an empty deque must have one node, and that a deque
 *    with N elements, where N is the buffer size, must have two nodes.
 *  For every node other than start.node and finish.node, every element
 *    in the node is an initialized object.  If start.node == finish.node,
 *    then [start.cur, finish.cur) are initialized objects, and
 *    the elements outside that range are uninitialized storage.  Otherwise,
 *    [start.cur, start.last) and [finish.first, finish.cur) are initialized
 *    objects, and [start.first, start.cur) and [finish.cur, finish.last)
 *    are uninitialized storage.
 *  [map, map + map_size) is a valid, non-empty range.  
 *  [start.node, finish.node] is a valid range contained within 
 *    [map, map + map_size).  
 *  A pointer in the range [map, map + map_size) points to an allocated node
 *    if and only if the pointer is in the range [start.node, finish.node].
 */

# undef deque
# define deque __WORKAROUND_DBG_RENAME(deque)

_STLP_BEGIN_NAMESPACE

template <class _Tp>
struct _Deque_iterator_base {

  enum _Constants { 
    _blocksize = _MAX_BYTES, 
    __buffer_size = (sizeof(_Tp) < (size_t)_blocksize ?
   		    ( (size_t)_blocksize / sizeof(_Tp)) : size_t(1))
  };

  typedef random_access_iterator_tag iterator_category;

  typedef _Tp value_type;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

  typedef value_type** _Map_pointer;

  typedef _Deque_iterator_base< _Tp > _Self;

  value_type* _M_cur;
  value_type* _M_first;
  value_type* _M_last;
  _Map_pointer _M_node;

  _Deque_iterator_base(value_type* __x, _Map_pointer __y) 
    : _M_cur(__x), _M_first(*__y),
      _M_last(*__y + __buffer_size), _M_node(__y) {}
  _Deque_iterator_base() : _M_cur(0), _M_first(0), _M_last(0), _M_node(0) {}

  difference_type _M_subtract(const _Self& __x) const {
    return difference_type(__buffer_size) * (_M_node - __x._M_node - 1) +
      (_M_cur - _M_first) + (__x._M_last - __x._M_cur);
  }

  void _M_increment() {
    if (++_M_cur == _M_last) {
      _M_set_node(_M_node + 1);
      _M_cur = _M_first;
    }
  }

  void _M_decrement() {
    if (_M_cur == _M_first) {
      _M_set_node(_M_node - 1);
      _M_cur = _M_last;
    }
    --_M_cur;
  }

  void _M_advance(difference_type __n)
  {
    difference_type __offset = __n + (_M_cur - _M_first);
    if (__offset >= 0 && __offset < difference_type(__buffer_size))
      _M_cur += __n;
    else {
      difference_type __node_offset =
        __offset > 0 ? __offset / __buffer_size
                   : -difference_type((-__offset - 1) / __buffer_size) - 1;
      _M_set_node(_M_node + __node_offset);
      _M_cur = _M_first + 
        (__offset - __node_offset * difference_type(__buffer_size));
    }
  }

  void _M_set_node(_Map_pointer __new_node) {
    _M_last = (_M_first = *(_M_node = __new_node)) + difference_type(__buffer_size);
  }
};



template <class _Tp, class _Traits>
struct _Deque_iterator : public _Deque_iterator_base< _Tp> {

  typedef random_access_iterator_tag iterator_category;
  typedef _Tp value_type;
  typedef typename _Traits::reference  reference;
  typedef typename _Traits::pointer    pointer;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef value_type** _Map_pointer;

  typedef _Deque_iterator_base< _Tp > _Base;
  typedef _Deque_iterator<_Tp, _Traits> _Self;
  typedef _Deque_iterator<_Tp, _Nonconst_traits<_Tp> > _Nonconst_self;
  typedef _Deque_iterator<_Tp, _Const_traits<_Tp> > _Const_self;

  _Deque_iterator(value_type* __x, _Map_pointer __y) :
    _Deque_iterator_base<value_type>(__x,__y) {}

  _Deque_iterator() {}
  _Deque_iterator(const _Nonconst_self& __x) : 
    _Deque_iterator_base<value_type>(__x) {}

  reference operator*() const { 
      return *this->_M_cur; 
  }

  _STLP_DEFINE_ARROW_OPERATOR

  difference_type operator-(const _Self& __x) const { return this->_M_subtract(__x); }

  _Self& operator++() { this->_M_increment(); return *this; }
  _Self operator++(int)  {
    _Self __tmp = *this;
    ++*this;
    return __tmp;
  }

  _Self& operator--() { this->_M_decrement(); return *this; }
  _Self operator--(int) {
    _Self __tmp = *this;
    --*this;
    return __tmp;
  }

  _Self& operator+=(difference_type __n) { this->_M_advance(__n); return *this; }
  _Self operator+(difference_type __n) const
  {
    _Self __tmp = *this;
    return __tmp += __n;
  }

  _Self& operator-=(difference_type __n) { return *this += -__n; }
  _Self operator-(difference_type __n) const {
    _Self __tmp = *this;
    return __tmp -= __n;
  }

  reference operator[](difference_type __n) const { return *(*this + __n); }
};

template <class _Tp, class _Traits>
inline _Deque_iterator<_Tp, _Traits> _STLP_CALL
operator+(ptrdiff_t __n, const _Deque_iterator<_Tp, _Traits>& __x)
{
   return __x + __n;
}


#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

template <class _Tp>
inline bool _STLP_CALL 
operator==(const _Deque_iterator_base<_Tp >& __x,
	   const _Deque_iterator_base<_Tp >& __y) { 
    return __x._M_cur == __y._M_cur; 
}

template <class _Tp>
inline bool _STLP_CALL 
operator < (const _Deque_iterator_base<_Tp >& __x,
	    const _Deque_iterator_base<_Tp >& __y) { 
  return (__x._M_node == __y._M_node) ? 
    (__x._M_cur < __y._M_cur) : (__x._M_node < __y._M_node);
}

template <class _Tp>
inline bool _STLP_CALL 
operator!=(const _Deque_iterator_base<_Tp >& __x,
	   const _Deque_iterator_base<_Tp >& __y) { 
    return __x._M_cur != __y._M_cur; 
}
template <class _Tp>
inline bool _STLP_CALL 
operator>(const _Deque_iterator_base<_Tp >& __x,
	  const _Deque_iterator_base<_Tp >& __y) { 
    return __y < __x;
}
template <class _Tp>
inline bool  _STLP_CALL operator>=(const _Deque_iterator_base<_Tp >& __x,
                                   const _Deque_iterator_base<_Tp >& __y) { 
    return !(__x < __y);
}
template <class _Tp>
inline bool  _STLP_CALL operator<=(const _Deque_iterator_base<_Tp >& __x,
                                   const _Deque_iterator_base<_Tp >& __y) { 
    return !(__y < __x);
}

# else

template <class _Tp, class _Traits1, class _Traits2>
inline bool  _STLP_CALL
operator==(const _Deque_iterator<_Tp, _Traits1 >& __x,
	   const _Deque_iterator<_Tp, _Traits2 >& __y) { 
    return __x._M_cur == __y._M_cur; 
}

template <class _Tp, class _Traits1, class _Traits2>
inline bool _STLP_CALL 
operator < (const _Deque_iterator<_Tp, _Traits1 >& __x,
	    const _Deque_iterator<_Tp, _Traits2 >& __y) { 
  return (__x._M_node == __y._M_node) ? 
    (__x._M_cur < __y._M_cur) : (__x._M_node < __y._M_node);
}

template <class _Tp>
inline bool _STLP_CALL 
operator!=(const _Deque_iterator<_Tp, _Nonconst_traits<_Tp> >& __x,
	   const _Deque_iterator<_Tp, _Const_traits<_Tp> >& __y) { 
    return __x._M_cur != __y._M_cur; 
}
template <class _Tp>
inline bool _STLP_CALL 
operator>(const _Deque_iterator<_Tp, _Nonconst_traits<_Tp> >& __x,
	  const _Deque_iterator<_Tp, _Const_traits<_Tp> >& __y) { 
    return __y < __x;
}
template <class _Tp>
inline bool  _STLP_CALL
operator>=(const _Deque_iterator<_Tp, _Nonconst_traits<_Tp> >& __x,
           const _Deque_iterator<_Tp, _Const_traits<_Tp> >& __y) { 
    return !(__x < __y);
}
template <class _Tp>
inline bool _STLP_CALL
operator<=(const _Deque_iterator<_Tp, _Nonconst_traits<_Tp> >& __x,
           const _Deque_iterator<_Tp, _Const_traits<_Tp> >& __y) { 
    return !(__y < __x);
}
# endif

# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
template <class _Tp, class _Traits> inline _Tp*  _STLP_CALL value_type(const _Deque_iterator<_Tp, _Traits  >&) { return (_Tp*)0; }
template <class _Tp, class _Traits> inline random_access_iterator_tag _STLP_CALL 
iterator_category(const _Deque_iterator<_Tp, _Traits  >&) { return random_access_iterator_tag(); }
template <class _Tp, class _Traits> inline ptrdiff_t* _STLP_CALL 
distance_type(const _Deque_iterator<_Tp, _Traits  >&) { return 0; }
#endif

// Deque base class.  It has two purposes.  First, its constructor
//  and destructor allocate (but don't initialize) storage.  This makes
//  exception safety easier.  Second, the base class encapsulates all of
//  the differences between SGI-style allocators and standard-conforming
//  allocators.

template <class _Tp, class _Alloc>
class _Deque_base {
public:
  typedef _Tp value_type;
  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef typename _Alloc_traits<_Tp,_Alloc>::allocator_type  allocator_type;
  typedef typename _Alloc_traits<_Tp*, _Alloc>::allocator_type _Map_alloc_type;

  typedef _Deque_iterator<_Tp, _Nonconst_traits<_Tp> > iterator;
  typedef _Deque_iterator<_Tp, _Const_traits<_Tp> >   const_iterator;

  static size_t  _STLP_CALL buffer_size() { return (size_t)_Deque_iterator_base<_Tp>::__buffer_size; } 

  _Deque_base(const allocator_type& __a, size_t __num_elements)
    : _M_start(), _M_finish(), _M_map(_STLP_CONVERT_ALLOCATOR(__a, _Tp*), 0),
      _M_map_size(__a, (size_t)0) {
	_M_initialize_map(__num_elements);
  }
  _Deque_base(const allocator_type& __a)
    : _M_start(), _M_finish(), _M_map(_STLP_CONVERT_ALLOCATOR(__a, _Tp*), 0), 
      _M_map_size(__a, (size_t)0) {
  }
  ~_Deque_base();    

protected:
  void _M_initialize_map(size_t);
  void _M_create_nodes(_Tp** __nstart, _Tp** __nfinish);
  void _M_destroy_nodes(_Tp** __nstart, _Tp** __nfinish);
  enum { _S_initial_map_size = 8 };

protected:
  iterator _M_start;
  iterator _M_finish;
  _STLP_alloc_proxy<value_type**, value_type*, _Map_alloc_type>  _M_map;
  _STLP_alloc_proxy<size_t, value_type,  allocator_type>   _M_map_size;  
};


template <class _Tp, _STLP_DEFAULT_ALLOCATOR_SELECT(_Tp) >
class deque : protected _Deque_base<_Tp, _Alloc> {
  typedef _Deque_base<_Tp, _Alloc> _Base;
  typedef deque<_Tp, _Alloc> _Self;
public:                         // Basic types
  typedef _Tp value_type;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef random_access_iterator_tag _Iterator_category;
  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef typename _Base::allocator_type allocator_type;

public:                         // Iterators
  typedef typename _Base::iterator       iterator;
  typedef typename _Base::const_iterator const_iterator;

  _STLP_DECLARE_RANDOM_ACCESS_REVERSE_ITERATORS;

protected:                      // Internal typedefs
  typedef pointer* _Map_pointer;
  typedef typename  __type_traits<_Tp>::has_trivial_assignment_operator _TrivialAss;
  typedef typename  __type_traits<_Tp>::has_trivial_assignment_operator _IsPODType;

public:                         // Basic accessors
  iterator begin() { return this->_M_start; }
  iterator end() { return this->_M_finish; }
  const_iterator begin() const { return const_iterator(this->_M_start); }
  const_iterator end() const { return const_iterator(this->_M_finish); }

  reverse_iterator rbegin() { return reverse_iterator(this->_M_finish); }
  reverse_iterator rend() { return reverse_iterator(this->_M_start); }
  const_reverse_iterator rbegin() const 
    { return const_reverse_iterator(this->_M_finish); }
  const_reverse_iterator rend() const 
    { return const_reverse_iterator(this->_M_start); }

  reference operator[](size_type __n)
    { return this->_M_start[difference_type(__n)]; }
  const_reference operator[](size_type __n) const 
    { return this->_M_start[difference_type(__n)]; }

  void _M_range_check(size_type __n) const {
    if (__n >= this->size())
      __stl_throw_out_of_range("deque");
  }
  reference at(size_type __n)
    { _M_range_check(__n); return (*this)[__n]; }
  const_reference at(size_type __n) const
    { _M_range_check(__n); return (*this)[__n]; }

  reference front() { return *this->_M_start; }
  reference back() {
    iterator __tmp = this->_M_finish;
    --__tmp;
    return *__tmp;
  }
  const_reference front() const { return *this->_M_start; }
  const_reference back() const {
    const_iterator __tmp = this->_M_finish;
    --__tmp;
    return *__tmp;
  }

  size_type size() const { return this->_M_finish - this->_M_start; }
  size_type max_size() const { return size_type(-1); }
  bool empty() const { return this->_M_finish == this->_M_start; }
  allocator_type get_allocator() const { return this->_M_map_size; }

public:                         // Constructor, destructor.
  explicit deque(const allocator_type& __a = allocator_type()) 
    : _Deque_base<_Tp, _Alloc>(__a, 0) {}

  deque(const _Self& __x) : 
    _Deque_base<_Tp, _Alloc>(__x.get_allocator(), __x.size()) { 
      __uninitialized_copy(__x.begin(), __x.end(), this->_M_start, _IsPODType()); 
  }

  deque(size_type __n, const value_type& __val,
        const allocator_type& __a = allocator_type()) : 
    _Deque_base<_Tp, _Alloc>(__a, __n)
    { _M_fill_initialize(__val); }
  // int,long variants may be needed 
  explicit deque(size_type __n) : _Deque_base<_Tp, _Alloc>(allocator_type(), __n)
    { _M_fill_initialize(value_type()); }

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _Integer>
  void _M_initialize_dispatch(_Integer __n, _Integer __x, const __true_type&) {
    this->_M_initialize_map(__n);
    _M_fill_initialize(__x);
  }

  template <class _InputIter>
  void _M_initialize_dispatch(_InputIter __first, _InputIter __last,
                              const __false_type&) {
    _M_range_initialize(__first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIter));
  }

# ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  // VC++ needs this
  template <class _InputIterator>
  deque(_InputIterator __first, _InputIterator __last) : 
    _Deque_base<_Tp, _Alloc>(allocator_type()) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_dispatch(__first, __last, _Integral());
  }
# endif

  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  deque(_InputIterator __first, _InputIterator __last,
        const allocator_type& __a _STLP_ALLOCATOR_TYPE_DFL) : 
    _Deque_base<_Tp, _Alloc>(__a) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_dispatch(__first, __last, _Integral());
  }

# else
  deque(const value_type* __first, const value_type* __last,
        const allocator_type& __a = allocator_type() ) 
    : _Deque_base<_Tp, _Alloc>(__a, __last - __first) { 
    __uninitialized_copy(__first, __last, this->_M_start, _IsPODType()); 
  }

  deque(const_iterator __first, const_iterator __last,
        const allocator_type& __a = allocator_type() ) 
    : _Deque_base<_Tp, _Alloc>(__a, __last - __first) { 
    __uninitialized_copy(__first, __last, this->_M_start, _IsPODType()); 
  }
#endif /* _STLP_MEMBER_TEMPLATES */

  ~deque() { 
    _STLP_STD::_Destroy(this->_M_start, this->_M_finish); 
  }

  _Self& operator= (const _Self& __x);

  void swap(_Self& __x) {
    _STLP_STD::swap(this->_M_start, __x._M_start);
    _STLP_STD::swap(this->_M_finish, __x._M_finish);
    _STLP_STD::swap(this->_M_map, __x._M_map);
    _STLP_STD::swap(this->_M_map_size, __x._M_map_size);
  }

public: 
  // assign(), a generalized assignment member function.  Two
  // versions: one that takes a count, and one that takes a range.
  // The range version is a member template, so we dispatch on whether
  // or not the type is an integer.

  void _M_fill_assign(size_type __n, const _Tp& __val) {
    if (__n > size()) {
      _STLP_STD::fill(begin(), end(), __val);
      insert(end(), __n - size(), __val);
    }
    else {
      erase(begin() + __n, end());
      _STLP_STD::fill(begin(), end(), __val);
    }
  }

  void assign(size_type __n, const _Tp& __val) {
    _M_fill_assign(__n, __val);
  }

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIterator>
  void assign(_InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_assign_dispatch(__first, __last, _Integral());
  }

private:                        // helper functions for assign() 

  template <class _Integer>
  void _M_assign_dispatch(_Integer __n, _Integer __val, const __true_type&)
    { _M_fill_assign((size_type) __n, (_Tp) __val); }

  template <class _InputIterator>
  void _M_assign_dispatch(_InputIterator __first, _InputIterator __last,
                          const __false_type&) {
    _M_assign_aux(__first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIterator));
  }

  template <class _InputIter>
  void _M_assign_aux(_InputIter __first, _InputIter __last, const input_iterator_tag &) {
    iterator __cur = begin();
    for ( ; __first != __last && __cur != end(); ++__cur, ++__first)
      *__cur = *__first;
    if (__first == __last)
      erase(__cur, end());
    else
      insert(end(), __first, __last);
  }

  template <class _ForwardIterator>
  void _M_assign_aux(_ForwardIterator __first, _ForwardIterator __last,
                     const forward_iterator_tag &) {
    size_type __len = _STLP_STD::distance(__first, __last);
    if (__len > size()) {
      _ForwardIterator __mid = __first;
      _STLP_STD::advance(__mid, size());
      _STLP_STD::copy(__first, __mid, begin());
      insert(end(), __mid, __last);
    }
    else
      erase(copy(__first, __last, begin()), end());
  }

#endif /* _STLP_MEMBER_TEMPLATES */

public:                         // push_* and pop_*
  
  void push_back(const value_type& __t) {
    if (this->_M_finish._M_cur != this->_M_finish._M_last - 1) {
      _STLP_STD::_Construct(this->_M_finish._M_cur, __t);
      ++this->_M_finish._M_cur;
    }
    else
      _M_push_back_aux_v(__t);
  }
  void push_front(const value_type& __t) {
    if (this->_M_start._M_cur != this->_M_start._M_first) {
      _STLP_STD::_Construct(this->_M_start._M_cur - 1, __t);
      --this->_M_start._M_cur;
    }
    else
      _M_push_front_aux_v(__t);
  }

# ifndef _STLP_NO_ANACHRONISMS
  void push_back() {
    if (this->_M_finish._M_cur != this->_M_finish._M_last - 1) {
      _STLP_STD::_Construct(this->_M_finish._M_cur);
      ++this->_M_finish._M_cur;
    }
    else
      _M_push_back_aux();
  }
  void push_front() {
    if (this->_M_start._M_cur != this->_M_start._M_first) {
      _STLP_STD::_Construct(this->_M_start._M_cur - 1);
      --this->_M_start._M_cur;
    }
    else
      _M_push_front_aux();
  }
# endif

  void pop_back() {
    if (this->_M_finish._M_cur != this->_M_finish._M_first) {
      --this->_M_finish._M_cur;
      _STLP_STD::_Destroy(this->_M_finish._M_cur);
    }
    else
      _M_pop_back_aux();
  }

  void pop_front() {
    if (this->_M_start._M_cur != this->_M_start._M_last - 1) {
      _STLP_STD::_Destroy(this->_M_start._M_cur);
      ++this->_M_start._M_cur;
    }
    else 
      _M_pop_front_aux();
  }

public:                         // Insert

  iterator insert(iterator __position, const value_type& __x) {
    if (__position._M_cur == this->_M_start._M_cur) {
      push_front(__x);
      return this->_M_start;
    }
    else if (__position._M_cur == this->_M_finish._M_cur) {
      push_back(__x);
      iterator __tmp = this->_M_finish;
      --__tmp;
      return __tmp;
    }
    else {
      return _M_insert_aux(__position, __x);
    }
  }

  iterator insert(iterator __position)
    { return insert(__position, value_type()); }

  void insert(iterator __pos, size_type __n, const value_type& __x) {
    _M_fill_insert(__pos, __n, __x);
  }

  void _M_fill_insert(iterator __pos, size_type __n, const value_type& __x);

#ifdef _STLP_MEMBER_TEMPLATES  

  // Check whether it's an integral type.  If so, it's not an iterator.
  template <class _InputIterator>
  void insert(iterator __pos, _InputIterator __first, _InputIterator __last) {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_insert_dispatch(__pos, __first, __last, _Integral());
  }

  template <class _Integer>
  void _M_insert_dispatch(iterator __pos, _Integer __n, _Integer __x,
                          const __true_type&) {
    _M_fill_insert(__pos, (size_type) __n, (value_type) __x);
  }

  template <class _InputIterator>
  void _M_insert_dispatch(iterator __pos,
                          _InputIterator __first, _InputIterator __last,
                          const __false_type&) {
    insert(__pos, __first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIterator));
  }

#else /* _STLP_MEMBER_TEMPLATES */

  void insert(iterator __pos,
              const value_type* __first, const value_type* __last);
  void insert(iterator __pos,
              const_iterator __first, const_iterator __last);

#endif /* _STLP_MEMBER_TEMPLATES */

  void resize(size_type __new_size, value_type __x) {
    const size_type __len = size();
    if (__new_size < __len) 
      erase(this->_M_start + __new_size, this->_M_finish);
    else
      insert(this->_M_finish, __new_size - __len, __x);
  }

  void resize(size_type new_size) { resize(new_size, value_type()); }

public:                         // Erase
  iterator erase(iterator __pos) {
    iterator __next = __pos;
    ++__next;
    difference_type __index = __pos - this->_M_start;
    if (size_type(__index) < this->size() >> 1) {
      _STLP_STD::copy_backward(this->_M_start, __pos, __next);
      pop_front();
    }
    else {
      _STLP_STD::copy(__next, this->_M_finish, __pos);
      pop_back();
    }
    return this->_M_start + __index;
  }

  iterator erase(iterator __first, iterator __last);
  void clear(); 

protected:                        // Internal construction/destruction

  void _M_fill_initialize(const value_type& __val);

#ifdef _STLP_MEMBER_TEMPLATES 

  template <class _InputIterator>
  void _M_range_initialize(_InputIterator __first,
			   _InputIterator __last,
			   const input_iterator_tag &) {
    this->_M_initialize_map(0);
    _STLP_TRY {
      for ( ; __first != __last; ++__first)
        push_back(*__first);
    }
    _STLP_UNWIND(clear());
  }
 template <class _ForwardIterator>
 void  _M_range_initialize(_ForwardIterator __first,
                           _ForwardIterator __last,
                           const forward_iterator_tag &)  {
   size_type __n = _STLP_STD::distance(__first, __last);
   this->_M_initialize_map(__n);
   _Map_pointer __cur_node;
   _STLP_TRY {
    for (__cur_node = this->_M_start._M_node; 
         __cur_node < this->_M_finish._M_node; 
	 ++__cur_node) {
      _ForwardIterator __mid = __first;
      advance(__mid, this->buffer_size());
      _STLP_STD::uninitialized_copy(__first, __mid, *__cur_node);
      __first = __mid;
    }
    _STLP_STD::uninitialized_copy(__first, __last, this->_M_finish._M_first);
   }
  _STLP_UNWIND(_STLP_STD::_Destroy(this->_M_start, iterator(*__cur_node, __cur_node)));
 }
#endif /* _STLP_MEMBER_TEMPLATES */

protected:                        // Internal push_* and pop_*

  void _M_push_back_aux_v(const value_type&);
  void _M_push_front_aux_v(const value_type&);
# ifndef _STLP_NO_ANACHRONISMS
  void _M_push_back_aux();
  void _M_push_front_aux();
# endif
  void _M_pop_back_aux();
  void _M_pop_front_aux();

protected:                        // Internal insert functions

#ifdef _STLP_MEMBER_TEMPLATES

template <class _InputIterator>
void 
insert(iterator __pos,
       _InputIterator __first,
       _InputIterator __last,
       const input_iterator_tag &)
{
  _STLP_STD::copy(__first, __last, inserter(*this, __pos));
}

template <class _ForwardIterator>
void  insert(iterator __pos,
	     _ForwardIterator __first,
	     _ForwardIterator __last,
	     const forward_iterator_tag &)
 {
  size_type __n = _STLP_STD::distance(__first, __last);
  if (__pos._M_cur == this->_M_start._M_cur) {
    iterator __new_start = _M_reserve_elements_at_front(__n);
    _STLP_TRY {
      _STLP_STD::uninitialized_copy(__first, __last, __new_start);
      this->_M_start = __new_start;
    }
    _STLP_UNWIND(this->_M_destroy_nodes(__new_start._M_node, this->_M_start._M_node));
  }
  else if (__pos._M_cur == this->_M_finish._M_cur) {
    iterator __new_finish = _M_reserve_elements_at_back(__n);
    _STLP_TRY {
      _STLP_STD::uninitialized_copy(__first, __last, this->_M_finish);
      this->_M_finish = __new_finish;
    }
    _STLP_UNWIND(this->_M_destroy_nodes(this->_M_finish._M_node + 1, __new_finish._M_node + 1));
  }
  else
    _M_insert_aux(__pos, __first, __last, __n);
}
#endif /* _STLP_MEMBER_TEMPLATES */

  iterator _M_insert_aux(iterator __pos, const value_type& __x);
  iterator _M_insert_aux(iterator __pos);
  iterator _M_insert_aux_prepare(iterator __pos);

  void _M_insert_aux(iterator __pos, size_type __n, const value_type& __x);

#ifdef _STLP_MEMBER_TEMPLATES  
  template <class _ForwardIterator>
  void _M_insert_aux(iterator __pos,
                     _ForwardIterator __first,
                     _ForwardIterator __last,
                     size_type __n) {
    
    const difference_type __elemsbefore = __pos - this->_M_start;
    size_type __length = size();
    if (__elemsbefore < difference_type(__length / 2)) {
      iterator __new_start = _M_reserve_elements_at_front(__n);
      iterator __old_start = this->_M_start;
      __pos = this->_M_start + __elemsbefore;
      _STLP_TRY {
	if (__elemsbefore >= difference_type(__n)) {
	  iterator __start_n = this->_M_start + difference_type(__n); 
	  _STLP_STD::uninitialized_copy(this->_M_start, __start_n, __new_start);
	  this->_M_start = __new_start;
	  _STLP_STD::copy(__start_n, __pos, __old_start);
	  _STLP_STD::copy(__first, __last, __pos - difference_type(__n));
	}
	else {
	  _ForwardIterator __mid = __first;
	  _STLP_STD::advance(__mid, difference_type(__n) - __elemsbefore);
	  __uninitialized_copy_copy(this->_M_start, __pos, __first, __mid,
				    __new_start, _IsPODType());
	  this->_M_start = __new_start;
	  _STLP_STD::copy(__mid, __last, __old_start);
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
        _STLP_STD::uninitialized_copy(__finish_n, this->_M_finish, this->_M_finish);
        this->_M_finish = __new_finish;
        _STLP_STD::copy_backward(__pos, __finish_n, __old_finish);
       _STLP_STD::copy(__first, __last, __pos);
      }
      else {
        _ForwardIterator __mid = __first;
        _STLP_STD::advance(__mid, __elemsafter);
        __uninitialized_copy_copy(__mid, __last, __pos, this->_M_finish, this->_M_finish, _IsPODType());
        this->_M_finish = __new_finish;
        _STLP_STD::copy(__first, __mid, __pos);
      }
      }
      _STLP_UNWIND(this->_M_destroy_nodes(this->_M_finish._M_node + 1, __new_finish._M_node + 1));
    }
  }
#else /* _STLP_MEMBER_TEMPLATES */
  
  void _M_insert_aux(iterator __pos,
                     const value_type* __first, const value_type* __last,
                     size_type __n);

  void _M_insert_aux(iterator __pos, 
                     const_iterator __first, const_iterator __last,
                     size_type __n);
 
#endif /* _STLP_MEMBER_TEMPLATES */

  iterator _M_reserve_elements_at_front(size_type __n) {
    size_type __vacancies = this->_M_start._M_cur - this->_M_start._M_first;
    if (__n > __vacancies) 
      _M_new_elements_at_front(__n - __vacancies);
    return this->_M_start - difference_type(__n);
  }

  iterator _M_reserve_elements_at_back(size_type __n) {
    size_type __vacancies = (this->_M_finish._M_last - this->_M_finish._M_cur) - 1;
    if (__n > __vacancies)
      _M_new_elements_at_back(__n - __vacancies);
    return this->_M_finish + difference_type(__n);
  }

  void _M_new_elements_at_front(size_type __new_elements);
  void _M_new_elements_at_back(size_type __new_elements);

protected:                      // Allocation of _M_map and nodes

  // Makes sure the _M_map has space for new nodes.  Does not actually
  //  add the nodes.  Can invalidate _M_map pointers.  (And consequently, 
  //  deque iterators.)

  void _M_reserve_map_at_back (size_type __nodes_to_add = 1) {
    if (__nodes_to_add + 1 > this->_M_map_size._M_data - (this->_M_finish._M_node - this->_M_map._M_data))
      _M_reallocate_map(__nodes_to_add, false);
  }

  void _M_reserve_map_at_front (size_type __nodes_to_add = 1) {
    if (__nodes_to_add > size_type(this->_M_start._M_node - this->_M_map._M_data))
      _M_reallocate_map(__nodes_to_add, true);
  }

  void _M_reallocate_map(size_type __nodes_to_add, bool __add_at_front);
 
};

# define _STLP_TEMPLATE_CONTAINER deque<_Tp, _Alloc>
# define _STLP_TEMPLATE_HEADER    template <class _Tp, class _Alloc>
# include <stl/_relops_cont.h>
# undef _STLP_TEMPLATE_CONTAINER
# undef _STLP_TEMPLATE_HEADER

_STLP_END_NAMESPACE 

// do a cleanup
# undef deque
# undef __deque__
# define __deque__ __WORKAROUND_DBG_RENAME(deque)

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_deque.c>
# endif

#if defined (_STLP_DEBUG)
# include <stl/debug/_deque.h>
#endif

# if defined (_STLP_USE_WRAPPER_FOR_ALLOC_PARAM)
#  include <stl/wrappers/_deque.h>
# endif
  
#endif /* _STLP_INTERNAL_DEQUE_H */

// Local Variables:
// mode:C++
// End:

