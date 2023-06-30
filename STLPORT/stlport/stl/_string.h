/*
 * Copyright (c) 1997-1999
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

#ifndef _STLP_STRING_H
#define _STLP_STRING_H

#ifndef _STLP_MEMORY
# include <memory> 
#endif

# ifndef _STLP_CCTYPE
#  include <cctype> 
# endif

#ifndef _STLP_STRING_FWD_H
#  include <stl/_string_fwd.h> 
#endif

#ifndef _STLP_INTERNAL_FUNCTION_BASE_H
# include <stl/_function.h> 
#endif

# include <stl/_ctraits_fns.h>  
#ifndef _STLP_INTERNAL_ALGOBASE_H
# include <stl/_algobase.h> 
#endif

#ifndef _STLP_INTERNAL_ITERATOR_H
# include <stl/_iterator.h> 
#endif

#if defined( __MWERKS__ ) && ! defined (_STLP_USE_OWN_NAMESPACE)

// MSL implementation classes expect to see the definition of streampos
// when this header is included. We expect this to be fixed in later MSL
// implementations
# if !defined( __MSL_CPP__ ) || __MSL_CPP__ < 0x4105
#  include <stl/msl_string.h> 
# endif

#endif // __MWERKS__

// Standard C++ string class.  This class has performance
// characteristics very much like vector<>, meaning, for example, that
// it does not perform reference-count or copy-on-write, and that
// concatenation of two strings is an O(N) operation. 

// There are three reasons why basic_string is not identical to
// vector.  First, basic_string always stores a null character at the
// end; this makes it possible for c_str to be a fast operation.
// Second, the C++ standard requires basic_string to copy elements
// using char_traits<>::assign, char_traits<>::copy, and
// char_traits<>::move.  This means that all of vector<>'s low-level
// operations must be rewritten.  Third, basic_string<> has a lot of
// extra functions in its interface that are convenient but, strictly
// speaking, redundant.

// Additionally, the C++ standard imposes a major restriction: according
// to the standard, the character type _CharT must be a POD type.  This
// implementation weakens that restriction, and allows _CharT to be a
// a user-defined non-POD type.  However, _CharT must still have a
// default constructor.

_STLP_BEGIN_NAMESPACE

# ifdef _STLP_DEBUG
#  define basic_string _Nondebug_string
# endif

// A helper class to use a char_traits as a function object.

template <class _Traits> struct _Not_within_traits
  : public unary_function<typename _Traits::char_type, bool> {
  typedef typename _Traits::char_type _CharT;
  const _CharT* _M_first;
  const _CharT* _M_last;

  _Not_within_traits(const typename _Traits::char_type* __f, 
		     const typename _Traits::char_type* __l) 
    : _M_first(__f), _M_last(__l) {}

  bool operator()(const typename _Traits::char_type& __x) const {
    return _STLP_STD::find_if(_M_first, _M_last, 
                   _Eq_char_bound<_Traits>(__x)) == _M_last;
  }
};

// ------------------------------------------------------------
// Class _String_base.  

// _String_base is a helper class that makes it it easier to write an
// exception-safe version of basic_string.  The constructor allocates,
// but does not initialize, a block of memory.  The destructor
// deallocates, but does not destroy elements within, a block of
// memory.  The destructor assumes that _M_start either is null, or else
// points to a block of memory that was allocated using _String_base's 
// allocator and whose size is _M_end_of_storage._M_data - _M_start.

template <class _Tp, class _Alloc> class _String_base {
public:
  _STLP_FORCE_ALLOCATORS(_Tp, _Alloc)
  typedef typename _Alloc_traits<_Tp, _Alloc>::allocator_type allocator_type;
  _Tp*    _M_start;
  _Tp*    _M_finish;
  _STLP_alloc_proxy<_Tp*, _Tp, allocator_type> _M_end_of_storage;
                                // Precondition: 0 < __n <= max_size().
  void _M_allocate_block(size_t);
  void _M_deallocate_block() 
    { _M_end_of_storage.deallocate(_M_start, _M_end_of_storage._M_data - _M_start); }
  
  size_t max_size() const { return (size_t(-1) / sizeof(_Tp)) - 1; }

  _String_base(const allocator_type& __a)
    : _M_start(0), _M_finish(0), _M_end_of_storage(__a, (_Tp*)0) {}
  
  _String_base(const allocator_type& __a, size_t __n)
    : _M_start(0), _M_finish(0), _M_end_of_storage(__a, (_Tp*)0)
    { _M_allocate_block(__n); }

  ~_String_base() { _M_deallocate_block(); }

  void _M_throw_length_error() const;
  void _M_throw_out_of_range() const;
};

# if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS _String_base<char, allocator<char> >;
#  if defined (_STLP_HAS_WCHAR_T)
_STLP_EXPORT_TEMPLATE_CLASS _String_base<wchar_t, allocator<wchar_t> >;
#  endif
# endif /* _STLP_USE_TEMPLATE_EXPORT */

// ------------------------------------------------------------
// Class basic_string.  

// Class invariants:
// (1) [start, finish) is a valid range.
// (2) Each iterator in [start, finish) points to a valid object
//     of type value_type.
// (3) *finish is a valid object of type value_type; in particular,
//     it is value_type().
// (4) [finish + 1, end_of_storage) is a valid range.
// (5) Each iterator in [finish + 1, end_of_storage) points to 
//     unininitialized memory.

// Note one important consequence: a string of length n must manage
// a block of memory whose size is at least n + 1.  

struct _String_reserve_t {};

template <class _CharT, class _Traits, class _Alloc> class basic_string : protected _String_base<_CharT,_Alloc> {
private:                        // Protected members inherited from base.
  typedef _String_base<_CharT,_Alloc> _Base;
  typedef basic_string<_CharT, _Traits, _Alloc> _Self;
  // fbp : used to optimize char/wchar_t cases, and to simplify
  // _STLP_DEFAULT_CONSTRUCTOR_BUG problem workaround
  typedef typename _Is_integer<_CharT>::_Integral _Char_Is_Integral;
public:
  typedef _CharT value_type;
  typedef _Traits traits_type;

  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef random_access_iterator_tag _Iterator_category;

  typedef const value_type*                const_iterator;
  typedef value_type*                      iterator;

  _STLP_DECLARE_RANDOM_ACCESS_REVERSE_ITERATORS;

# if defined(_STLP_STATIC_CONST_INIT_BUG)
  enum { npos = -1 };
# elif __GNUC__ == 2 && __GNUC_MINOR__ == 96
  // inline initializer conflicts with 'extern template' 
  static const size_t npos ;
# else
  static const size_t npos = ~(size_t)0;
# endif

  typedef _String_reserve_t _Reserve_t;
# if defined (_STLP_USE_NATIVE_STRING) && ! defined (_STLP_DEBUG)
#  if (defined(__IBMCPP__) && (500 <= __IBMCPP__) && (__IBMCPP__ < 600) )
   // this typedef is being used for conversions
   typedef typename _STLP_VENDOR_STD::basic_string<_CharT,_Traits, 
    typename _STLP_VENDOR_STD::allocator<_CharT> > __std_string;
#  else
   // this typedef is being used for conversions
   typedef _STLP_VENDOR_STD::basic_string<_CharT,_Traits, 
    _STLP_VENDOR_STD::allocator<_CharT> > __std_string;
#  endif
# endif
  
public:                         // Constructor, destructor, assignment.
  typedef typename _String_base<_CharT,_Alloc>::allocator_type allocator_type;

  allocator_type get_allocator() const {
    return _STLP_CONVERT_ALLOCATOR((const allocator_type&)this->_M_end_of_storage, _CharT);
  }

  basic_string();

  explicit basic_string(const allocator_type& __a)
    : _String_base<_CharT,_Alloc>(__a, 8) { 
    _M_terminate_string(); 
  }

  basic_string(_Reserve_t, size_t __n,
               const allocator_type& __a = allocator_type())
    : _String_base<_CharT,_Alloc>(__a, __n + 1) { 
    _M_terminate_string(); 
  }

  basic_string(const basic_string<_CharT, _Traits, _Alloc>&);

  basic_string(const _Self& __s, size_type __pos, size_type __n = npos,
               const allocator_type& __a = allocator_type()) 
    : _String_base<_CharT,_Alloc>(__a) {
    if (__pos > __s.size())
      this->_M_throw_out_of_range();
    else
      _M_range_initialize(__s._M_start + __pos,
                          __s._M_start + __pos + (min) (__n, __s.size() - __pos));
  }

  basic_string(const _CharT* __s, size_type __n,
               const allocator_type& __a = allocator_type()) 
    : _String_base<_CharT,_Alloc>(__a) 
    { 
      _STLP_FIX_LITERAL_BUG(__s)
      _M_range_initialize(__s, __s + __n); 
    }

  basic_string(const _CharT* __s,
               const allocator_type& __a = allocator_type());

  basic_string(size_type __n, _CharT __c,
               const allocator_type& __a = allocator_type())
    : _String_base<_CharT,_Alloc>(__a, __n + 1)
  {
    this->_M_finish = uninitialized_fill_n(this->_M_start, __n, __c);
    _M_terminate_string();
  }

  // Check to see if _InputIterator is an integer type.  If so, then
  // it can't be an iterator.
#if defined (_STLP_MEMBER_TEMPLATES) && !(defined(__MRC__)||(defined(__SC__) && !defined(__DMC__)))		//*ty 04/30/2001 - mpw compilers choke on this ctor
# ifdef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS
  template <class _InputIterator> basic_string(_InputIterator __f, _InputIterator __l)
    : _String_base<_CharT,_Alloc>(allocator_type())
  {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_dispatch(__f, __l, _Integral());
  }
# endif
  template <class _InputIterator> basic_string(_InputIterator __f, _InputIterator __l,
               const allocator_type & __a _STLP_ALLOCATOR_TYPE_DFL)
    : _String_base<_CharT,_Alloc>(__a)
  {
    typedef typename _Is_integer<_InputIterator>::_Integral _Integral;
    _M_initialize_dispatch(__f, __l, _Integral());
  }
#else /* _STLP_MEMBER_TEMPLATES */

  basic_string(const _CharT* __f, const _CharT* __l,
               const allocator_type& __a = allocator_type())
    : _String_base<_CharT,_Alloc>(__a)
  {
    _STLP_FIX_LITERAL_BUG(__f)  _STLP_FIX_LITERAL_BUG(__l)
    _M_range_initialize(__f, __l);
  }

#endif

# if defined (_STLP_USE_NATIVE_STRING) && ! defined (_STLP_DEBUG)
  // these conversion operations still needed for
  // strstream, etc.
  basic_string (const __std_string& __x): _String_base<_CharT,_Alloc>(allocator_type())
    {
      const _CharT* __s = __x.data();
      _M_range_initialize(__s, __s + __x.size()); 
    }
  
  operator __std_string() const { return __std_string(this->data(), this->size()); }
# endif

  ~basic_string() { _STLP_STD::_Destroy(this->_M_start, this->_M_finish + 1); }
    
  _Self& operator=(const _Self& __s) {
    if (&__s != this) 
      assign(__s._M_start, __s._M_finish);
    return *this;
  }

  _Self& operator=(const _CharT* __s) { 
    _STLP_FIX_LITERAL_BUG(__s)
    return assign(__s, __s + traits_type::length(__s)); 
  }

  _Self& operator=(_CharT __c)
    { return assign(__STATIC_CAST(size_type,1), __c); }

  static _CharT _STLP_CALL _M_null() {
    return _STLP_DEFAULT_CONSTRUCTED(_CharT);
  }

private:                        // Helper functions used by constructors
                                // and elsewhere.
  // fbp : simplify integer types (char, wchar)
  void _M_construct_null_aux(_CharT* __p, const __false_type&) {
    _Construct(__p);
  }
  void _M_construct_null_aux(_CharT* __p, const __true_type&) {
    *__p = 0;
  }

  void _M_construct_null(_CharT* __p) {
    _M_construct_null_aux(__p, _Char_Is_Integral());
  }

private:                        
  // Helper functions used by constructors.  It is a severe error for
  // any of them to be called anywhere except from within constructors.

  void _M_terminate_string_aux(const __false_type&) {
    _STLP_TRY {
      _M_construct_null(this->_M_finish);
    }
    _STLP_UNWIND(_STLP_STD::_Destroy(this->_M_start, this->_M_finish));
  }

  void _M_terminate_string_aux(const __true_type&) {
    *(this->_M_finish)=0;
  }

  void _M_terminate_string() {
    _M_terminate_string_aux(_Char_Is_Integral());
  }

#ifndef _STLP_MEMBER_TEMPLATES
  bool _M_inside(const _CharT* __s ) const {
    return (__s >= this->_M_start) && (__s < this->_M_finish);
  }
#else
  template <class _InputIter>
  bool _M_inside(_InputIter __i) const {
    const _CharT* __s = __STATIC_CAST(const _CharT*, &(*__i));
    return (__s >= this->_M_start) && (__s < this->_M_finish);
  }
#endif /*_STLP_MEMBER_TEMPLATES*/

#ifdef _STLP_MEMBER_TEMPLATES
    
  template <class _InputIter> void _M_range_initialize(_InputIter __f, _InputIter __l,
                           const input_iterator_tag &) {
    this->_M_allocate_block(8);
    _M_construct_null(this->_M_finish);
    _STLP_TRY {
      append(__f, __l);
    }
    _STLP_UNWIND(_STLP_STD::_Destroy(this->_M_start, this->_M_finish + 1));
  }

  template <class _ForwardIter> void _M_range_initialize(_ForwardIter __f, _ForwardIter __l, 
                           const forward_iterator_tag &) {
    difference_type __n = _STLP_STD::distance(__f, __l);
    this->_M_allocate_block(__n + 1);
    this->_M_finish = _STLP_STD::uninitialized_copy(__f, __l, this->_M_start);
    _M_terminate_string();
  }

  template <class _InputIter> void _M_range_initialize(_InputIter __f, _InputIter __l) {
    _M_range_initialize(__f, __l, _STLP_ITERATOR_CATEGORY(__f, _InputIter));
  }

  template <class _Integer> void _M_initialize_dispatch(_Integer __n, _Integer __x, const __true_type&) {
    this->_M_allocate_block(__n + 1);
    this->_M_finish = uninitialized_fill_n(this->_M_start, __n, __x);
    _M_terminate_string();
  }

  template <class _InputIter> void _M_initialize_dispatch(_InputIter __f, _InputIter __l, const __false_type&) {
     _M_range_initialize(__f, __l);
  }
    
#else /* _STLP_MEMBER_TEMPLATES */

  void _M_range_initialize(const _CharT* __f, const _CharT* __l) {
    ptrdiff_t __n = __l - __f;
    this->_M_allocate_block(__n + 1);
    this->_M_finish = _STLP_STD::uninitialized_copy(__f, __l, this->_M_start);
    _M_terminate_string();
  }

#endif /* _STLP_MEMBER_TEMPLATES */

public:                         // Iterators.
  iterator begin()             { return this->_M_start; }
  iterator end()               { return this->_M_finish; }
  const_iterator begin() const { return this->_M_start; }
  const_iterator end()   const { return this->_M_finish; }  

  reverse_iterator rbegin()             
    { return reverse_iterator(this->_M_finish); }
  reverse_iterator rend()               
    { return reverse_iterator(this->_M_start); }
  const_reverse_iterator rbegin() const 
    { return const_reverse_iterator(this->_M_finish); }
  const_reverse_iterator rend()   const 
    { return const_reverse_iterator(this->_M_start); }

public:                         // Size, capacity, etc.
  size_type size() const { return this->_M_finish - this->_M_start; }
  size_type length() const { return size(); }

  size_t max_size() const { return _Base::max_size(); }


  void resize(size_type __n, _CharT __c) {
    if (__n <= size())
      erase(begin() + __n, end());
    else
      append(__n - size(), __c);
  }
  void resize(size_type __n) { resize(__n, _M_null()); }

  void reserve(size_type = 0);

  size_type capacity() const { return (this->_M_end_of_storage._M_data - this->_M_start) - 1; }

  void clear() {
    if (!empty()) {
      _Traits::assign(*(this->_M_start), _M_null());
      _STLP_STD::_Destroy(this->_M_start+1, this->_M_finish+1);
      this->_M_finish = this->_M_start;
    }
  } 

  bool empty() const { return this->_M_start == this->_M_finish; }    

public:                         // Element access.

  const_reference operator[](size_type __n) const
    { return *(this->_M_start + __n); }
  reference operator[](size_type __n)
    { return *(this->_M_start + __n); }

  const_reference at(size_type __n) const {
    if (__n >= size())
      this->_M_throw_out_of_range();
    return *(this->_M_start + __n);
  }

  reference at(size_type __n) {
    if (__n >= size())
      this->_M_throw_out_of_range();
    return *(this->_M_start + __n);
  }

public:                         // Append, operator+=, push_back.

  _Self& operator+=(const _Self& __s) { return append(__s); }
  _Self& operator+=(const _CharT* __s) { _STLP_FIX_LITERAL_BUG(__s) return append(__s); }
  _Self& operator+=(_CharT __c) { push_back(__c); return *this; }

  _Self& append(const _Self& __s) 
    { return append(__s._M_start, __s._M_finish); }

  _Self& append(const _Self& __s,
                       size_type __pos, size_type __n)
  {
    if (__pos > __s.size())
      this->_M_throw_out_of_range();
    return append(__s._M_start + __pos,
                  __s._M_start + __pos + (min) (__n, __s.size() - __pos));
  }

  _Self& append(const _CharT* __s, size_type __n) 
    { _STLP_FIX_LITERAL_BUG(__s) return append(__s, __s+__n); }
  _Self& append(const _CharT* __s) 
    { _STLP_FIX_LITERAL_BUG(__s) return append(__s, __s + traits_type::length(__s)); }
  _Self& append(size_type __n, _CharT __c);

#ifdef _STLP_MEMBER_TEMPLATES

  // Check to see if _InputIterator is an integer type.  If so, then
  // it can't be an iterator.
  template <class _InputIter> _Self& append(_InputIter __first, _InputIter __last) {
    typedef typename _Is_integer<_InputIter>::_Integral _Integral;
    return _M_append_dispatch(__first, __last, _Integral());
  }

#else /* _STLP_MEMBER_TEMPLATES */

  _Self& append(const _CharT* __first, const _CharT* __last);

#endif /* _STLP_MEMBER_TEMPLATES */

  void push_back(_CharT __c) {
    if (this->_M_finish + 1 == this->_M_end_of_storage._M_data)
      reserve(size() + (max)(size(), __STATIC_CAST(size_type,1)));
    _M_construct_null(this->_M_finish + 1);
    _Traits::assign(*(this->_M_finish), __c);
    ++this->_M_finish;
  }

  void pop_back() {
    _Traits::assign(*(this->_M_finish - 1), _M_null());
    _STLP_STD::_Destroy(this->_M_finish);
    --this->_M_finish;
  }

private:                        // Helper functions for append.

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIter> _Self& append(_InputIter __first, _InputIter __last, const input_iterator_tag &)
  {
	  for ( ; __first != __last ; ++__first)
	    push_back(*__first);
	  return *this;
	}

  template <class _ForwardIter> _Self& append(_ForwardIter __first, _ForwardIter __last, 
                       const forward_iterator_tag &)  {
    if (__first != __last) {
	    const size_type __old_size = size();
	    difference_type __n = _STLP_STD::distance(__first, __last);
	    if (__STATIC_CAST(size_type,__n) > max_size() || __old_size > max_size() - __STATIC_CAST(size_type,__n))
	      this->_M_throw_length_error();
	    if (__old_size + __n > capacity()) {
	      const size_type __len = __old_size +
	                            (max)(__old_size, __STATIC_CAST(size_type,__n)) + 1;
	      pointer __new_start = this->_M_end_of_storage.allocate(__len);
	      pointer __new_finish = __new_start;
	      _STLP_TRY {
	        __new_finish = _STLP_STD::uninitialized_copy(this->_M_start, this->_M_finish, __new_start);
	        __new_finish = _STLP_STD::uninitialized_copy(__first, __last, __new_finish);
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
	      _ForwardIter __f1 = __first;
	      ++__f1;
	      _STLP_STD::uninitialized_copy(__f1, __last, this->_M_finish + 1);
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

  template <class _Integer> _Self& _M_append_dispatch(_Integer __n, _Integer __x, const __true_type&) {
    return append((size_type) __n, (_CharT) __x);
  }

  template <class _InputIter> _Self& _M_append_dispatch(_InputIter __f, _InputIter __l,
                                   const __false_type&) {
    return append(__f, __l, _STLP_ITERATOR_CATEGORY(__f, _InputIter));
  }

#endif /* _STLP_MEMBER_TEMPLATES */

public:                         // Assign
  
  _Self& assign(const _Self& __s) 
    { return assign(__s._M_start, __s._M_finish); }

  _Self& assign(const _Self& __s, 
                       size_type __pos, size_type __n) {
    if (__pos > __s.size())
      this->_M_throw_out_of_range();
    return assign(__s._M_start + __pos, 
                  __s._M_start + __pos + (min) (__n, __s.size() - __pos));
  }

  _Self& assign(const _CharT* __s, size_type __n)
    { _STLP_FIX_LITERAL_BUG(__s) return assign(__s, __s + __n); }

  _Self& assign(const _CharT* __s)
    { _STLP_FIX_LITERAL_BUG(__s) return assign(__s, __s + _Traits::length(__s)); }

  _Self& assign(size_type __n, _CharT __c);

#ifdef _STLP_MEMBER_TEMPLATES

private:                        // Helper functions for assign.

  template <class _Integer> 
  _Self& _M_assign_dispatch(_Integer __n, _Integer __x, const __true_type&) {
    return assign((size_type) __n, (_CharT) __x);
  }

  template <class _InputIter> 
  _Self& _M_assign_dispatch(_InputIter __f, _InputIter __l,
			    const __false_type&)  {
    pointer __cur = this->_M_start;
    while (__f != __l && __cur != this->_M_finish) {
      _Traits::assign(*__cur, *__f);
      ++__f;
      ++__cur;
    }
    if (__f == __l)
      erase(__cur, end());
    else
      append(__f, __l);
    return *this;
  }
  
public:
  // Check to see if _InputIterator is an integer type.  If so, then
  // it can't be an iterator.
  template <class _InputIter> _Self& assign(_InputIter __first, _InputIter __last) {
    typedef typename _Is_integer<_InputIter>::_Integral _Integral;
    return _M_assign_dispatch(__first, __last, _Integral());
  }
#endif  /* _STLP_MEMBER_TEMPLATES */

  // if member templates are on, this works as specialization 
  _Self& assign(const _CharT* __f, const _CharT* __l)
  {
    ptrdiff_t __n = __l - __f;
    if (__STATIC_CAST(size_type,__n) <= size()) {
      _Traits::copy(this->_M_start, __f, __n);
      erase(begin() + __n, end());
    }
    else {
      _Traits::copy(this->_M_start, __f, size());
      append(__f + size(), __l);
    }
    return *this;
  }
  
public:                         // Insert

  _Self& insert(size_type __pos, const _Self& __s) {
    if (__pos > size())
      this->_M_throw_out_of_range();
    if (size() > max_size() - __s.size())
      this->_M_throw_length_error();
    insert(begin() + __pos, __s._M_start, __s._M_finish);
    return *this;
  }

  _Self& insert(size_type __pos, const _Self& __s,
                       size_type __beg, size_type __n) {
    if (__pos > size() || __beg > __s.size())
      this->_M_throw_out_of_range();
    size_type __len = (min) (__n, __s.size() - __beg);
    if (size() > max_size() - __len)
      this->_M_throw_length_error();
    insert(begin() + __pos,
           __s._M_start + __beg, __s._M_start + __beg + __len);
    return *this;
  }

  _Self& insert(size_type __pos, const _CharT* __s, size_type __n) {
    _STLP_FIX_LITERAL_BUG(__s)
    if (__pos > size())
      this->_M_throw_out_of_range();
    if (size() > max_size() - __n)
      this->_M_throw_length_error();
    insert(begin() + __pos, __s, __s + __n);
    return *this;
  }

  _Self& insert(size_type __pos, const _CharT* __s) {
    _STLP_FIX_LITERAL_BUG(__s)
    if (__pos > size())
      this->_M_throw_out_of_range();
    size_type __len = _Traits::length(__s);
    if (size() > max_size() - __len)
      this->_M_throw_length_error();
    insert(this->_M_start + __pos, __s, __s + __len);
    return *this;
  }
    
  _Self& insert(size_type __pos, size_type __n, _CharT __c) {
    if (__pos > size())
      this->_M_throw_out_of_range();
    if (size() > max_size() - __n)
      this->_M_throw_length_error();
    insert(begin() + __pos, __n, __c);
    return *this;
  }

  iterator insert(iterator __p, _CharT __c) {
    _STLP_FIX_LITERAL_BUG(__p)
    if (__p == end()) {
      push_back(__c);
      return this->_M_finish - 1;
    }
    else
      return _M_insert_aux(__p, __c);
  }

  void insert(iterator __p, size_t __n, _CharT __c);

#ifdef _STLP_MEMBER_TEMPLATES

  // Check to see if _InputIterator is an integer type.  If so, then
  // it can't be an iterator.
  template <class _InputIter> void insert(iterator __p, _InputIter __first, _InputIter __last) {
    typedef typename _Is_integer<_InputIter>::_Integral _Integral;
    _M_insert_dispatch(__p, __first, __last, _Integral());
  }

#else /* _STLP_MEMBER_TEMPLATES */

  void insert(iterator __p, const _CharT* __first, const _CharT* __last);

#endif /* _STLP_MEMBER_TEMPLATES */

private:                        // Helper functions for insert.

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _InputIter> void insert(iterator __p, _InputIter __first, _InputIter __last,
	      const input_iterator_tag &)
  {
	  for ( ; __first != __last; ++__first) {
	    __p = insert(__p, *__first);
	    ++__p;
	  }
	}

  template <class _ForwardIter> 
  void insert(iterator __position, _ForwardIter __first, _ForwardIter __last, 
	      const forward_iterator_tag &)  {
    if (__first != __last) {
      difference_type __n = _STLP_STD::distance(__first, __last);
      if (this->_M_end_of_storage._M_data - this->_M_finish >= __n + 1) {
	const difference_type __elems_after = this->_M_finish - __position;
	pointer __old_finish = this->_M_finish;
	if (__elems_after >= __n) {
	  _STLP_STD::uninitialized_copy((this->_M_finish - __n) + 1, this->_M_finish + 1,
			     this->_M_finish + 1);
	  this->_M_finish += __n;
	  _Traits::move(__position + __n,
			__position, (__elems_after - __n) + 1);
	  _M_move(__first, __last, __position);
	      }
	else {
	  _ForwardIter __mid = __first;
	  advance(__mid, __elems_after + 1);
	  _STLP_STD::uninitialized_copy(__mid, __last, this->_M_finish + 1);
	  this->_M_finish += __n - __elems_after;
	        _STLP_TRY {
	          _STLP_STD::uninitialized_copy(__position, __old_finish + 1, this->_M_finish);
	          this->_M_finish += __elems_after;
	        }
	        _STLP_UNWIND((_STLP_STD::_Destroy(__old_finish + 1, this->_M_finish), 
	                      this->_M_finish = __old_finish));
	        _M_move(__first, __mid, __position);
	}
      }
      else {
	const size_type __old_size = size();        
	const size_type __len
	  = __old_size + (max)(__old_size, __STATIC_CAST(size_type,__n)) + 1;
	      pointer __new_start = this->_M_end_of_storage.allocate(__len);
	      pointer __new_finish = __new_start;
	      _STLP_TRY {
	        __new_finish = _STLP_STD::uninitialized_copy(this->_M_start, __position, __new_start);
	        __new_finish = _STLP_STD::uninitialized_copy(__first, __last, __new_finish);
	        __new_finish
	          = _STLP_STD::uninitialized_copy(__position, this->_M_finish, __new_finish);
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

  template <class _Integer> void _M_insert_dispatch(iterator __p, _Integer __n, _Integer __x,
                          const __true_type&) {
    insert(__p, (size_type) __n, (_CharT) __x);
  }

  template <class _InputIter> void _M_insert_dispatch(iterator __p, _InputIter __first, _InputIter __last,
                          const __false_type&) {
    insert(__p, __first, __last, _STLP_ITERATOR_CATEGORY(__first, _InputIter));
  }

  template <class _InputIterator> void 
  _M_copy(_InputIterator __first, _InputIterator __last, pointer __result) {
    for ( ; __first != __last; ++__first, ++__result)
      _Traits::assign(*__result, *__first);
  }

  template <class _InputIterator>
  void _M_move(_InputIterator __first, _InputIterator __last, pointer __result) {
    //call _M_copy as being here means that __result is not within [__first, __last)
    for ( ; __first != __last; ++__first, ++__result)
      _Traits::assign(*__result, *__first);
  }

#endif /* _STLP_MEMBER_TEMPLATES */

  pointer _M_insert_aux(pointer, _CharT);

  void 
  _M_copy(const _CharT* __first, const _CharT* __last, _CharT* __result) {
    _Traits::copy(__result, __first, __last - __first);
  }
  void _M_move(const _CharT* __first, const _CharT* __last, _CharT* __result) {
    _Traits::move(__result, __first, __last - __first);
  }

public:                         // Erase.

  _Self& erase(size_type __pos = 0, size_type __n = npos) {
    if (__pos > size())
      this->_M_throw_out_of_range();
    erase(begin() + __pos, begin() + __pos + (min) (__n, size() - __pos));
    return *this;
  }  

  iterator erase(iterator __position) {
                                // The move includes the terminating _CharT().
    _Traits::move(__position, __position + 1, this->_M_finish - __position);
    _STLP_STD::_Destroy(this->_M_finish);
    --this->_M_finish;
    return __position;
  }

  iterator erase(iterator __first, iterator __last) {
    if (__first != __last) {
                                // The move includes the terminating _CharT().
      traits_type::move(__first, __last, (this->_M_finish - __last) + 1);
      pointer __new_finish = this->_M_finish - (__last - __first);
      _STLP_STD::_Destroy(__new_finish + 1, this->_M_finish + 1);
      this->_M_finish = __new_finish;
    }
    return __first;
  }

public:                         // Replace.  (Conceptually equivalent
                                // to erase followed by insert.)
  _Self& replace(size_type __pos, size_type __n, 
                        const _Self& __s) {
    if (__pos > size())
      this->_M_throw_out_of_range();
    const size_type __len = (min) (__n, size() - __pos);
    if (size() - __len >= max_size() - __s.size())
      this->_M_throw_length_error();
    return replace(begin() + __pos, begin() + __pos + __len, 
                   __s._M_start, __s._M_finish);
  }

  _Self& replace(size_type __pos1, size_type __n1,
                        const _Self& __s,
                        size_type __pos2, size_type __n2) {
    if (__pos1 > size() || __pos2 > __s.size())
      this->_M_throw_out_of_range();
    const size_type __len1 = (min) (__n1, size() - __pos1);
    const size_type __len2 = (min) (__n2, __s.size() - __pos2);
    if (size() - __len1 >= max_size() - __len2)
      this->_M_throw_length_error();
    return replace(begin() + __pos1, begin() + __pos1 + __len1,
                   __s._M_start + __pos2, __s._M_start + __pos2 + __len2);
  }

  _Self& replace(size_type __pos, size_type __n1,
                        const _CharT* __s, size_type __n2) {
    _STLP_FIX_LITERAL_BUG(__s)
    if (__pos > size())
      this->_M_throw_out_of_range();
    const size_type __len = (min) (__n1, size() - __pos);
    if (__n2 > max_size() || size() - __len >= max_size() - __n2)
      this->_M_throw_length_error();
    return replace(begin() + __pos, begin() + __pos + __len,
                   __s, __s + __n2);
  }

  _Self& replace(size_type __pos, size_type __n1,
                        const _CharT* __s) {
    _STLP_FIX_LITERAL_BUG(__s)
    if (__pos > size())
      this->_M_throw_out_of_range();
    const size_type __len = (min) (__n1, size() - __pos);
    const size_type __n2 = _Traits::length(__s);
    if (__n2 > max_size() || size() - __len >= max_size() - __n2)
      this->_M_throw_length_error();
    return replace(begin() + __pos, begin() + __pos + __len,
                   __s, __s + _Traits::length(__s));
  }

  _Self& replace(size_type __pos, size_type __n1,
                        size_type __n2, _CharT __c) {
    if (__pos > size())
      this->_M_throw_out_of_range();
    const size_type __len = (min) (__n1, size() - __pos);
    if (__n2 > max_size() || size() - __len >= max_size() - __n2)
      this->_M_throw_length_error();
    return replace(begin() + __pos, begin() + __pos + __len, __n2, __c);
  }

  _Self& replace(iterator __first, iterator __last, 
                        const _Self& __s) 
    { return replace(__first, __last, __s._M_start, __s._M_finish); }

  _Self& replace(iterator __first, iterator __last,
                        const _CharT* __s, size_type __n) 
    { _STLP_FIX_LITERAL_BUG(__s) return replace(__first, __last, __s, __s + __n); }

  _Self& replace(iterator __first, iterator __last,
                        const _CharT* __s) {
    _STLP_FIX_LITERAL_BUG(__s)
    return replace(__first, __last, __s, __s + _Traits::length(__s));
  }

  _Self& replace(iterator __first, iterator __last, 
                        size_type __n, _CharT __c);

  // Check to see if _InputIterator is an integer type.  If so, then
  // it can't be an iterator.
#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIter> _Self& replace(iterator __first, iterator __last,
                        _InputIter __f, _InputIter __l) {
    typedef typename _Is_integer<_InputIter>::_Integral _Integral;
    return _M_replace_dispatch(__first, __last, __f, __l,  _Integral());
  }
#else /* _STLP_MEMBER_TEMPLATES */
  _Self& replace(iterator __first, iterator __last,
		 const _CharT* __f, const _CharT* __l);
#endif /* _STLP_MEMBER_TEMPLATES */

private:                        // Helper functions for replace.

#ifdef _STLP_MEMBER_TEMPLATES

  template <class _Integer> _Self& _M_replace_dispatch(iterator __first, iterator __last,
                                    _Integer __n, _Integer __x,
                                    const __true_type&) {
    return replace(__first, __last, (size_type) __n, (_CharT) __x);
  }

  template <class _InputIter> _Self& _M_replace_dispatch(iterator __first, iterator __last,
                                    _InputIter __f, _InputIter __l,
                                    const __false_type&) {
    return replace(__first, __last, __f, __l, _STLP_ITERATOR_CATEGORY(__f, _InputIter));
  }

  template <class _InputIter> _Self& replace(iterator __first, iterator __last,
                        _InputIter __f, _InputIter __l, const input_iterator_tag &)  {
	  for ( ; __first != __last && __f != __l; ++__first, ++__f)
	    _Traits::assign(*__first, *__f);

	  if (__f == __l)
	    erase(__first, __last);
	  else
	    insert(__last, __f, __l);
	  return *this;
	}

  template <class _InputIter>
  _Self& replace(iterator __first, iterator __last,
                 _InputIter __f, _InputIter __l, const random_access_iterator_tag &) {
    //might be overlapping
    if (_M_inside(__f)) {
      difference_type __n = __l - __f;
      const difference_type __len = __last - __first;
      if (__len >= __n) {
        _M_move(__f, __l, __first);
        erase(__first + __n, __last);
      }
      else {
        _InputIter __m = __f + __len;
        if ((__l <= __first) || (__f >= __last)) {
				  //no overlap:
          _M_copy(__f, __m, __first);
          insert(__last, __m, __l);
        }
        else {
				  //we have to take care of reallocation:
				  const difference_type __off_dest = __first - this->begin();
				  const difference_type __off_src = __f - this->begin();
				  insert(__last, __m, __l);
				  _Traits::move(begin() + __off_dest, begin() + __off_src, __n);
        }
      }
      return *this;
    }
	  else {
		  return replace(__first, __last, __f, __l, forward_iterator_tag());
	  }
  }


  template <class _ForwardIter> _Self& replace(iterator __first, iterator __last,
                        _ForwardIter __f, _ForwardIter __l, 
                        const forward_iterator_tag &)  {
	  difference_type __n = _STLP_STD::distance(__f, __l);
	  const difference_type __len = __last - __first;
	  if (__len >= __n) {
	    _M_copy(__f, __l, __first);
	    erase(__first + __n, __last);
	  }
	  else {
	    _ForwardIter __m = __f;
	    advance(__m, __len);
	    _M_copy(__f, __m, __first);
	    insert(__last, __m, __l);
	  }
	  return *this;
	}

#endif /* _STLP_MEMBER_TEMPLATES */

public:                         // Other modifier member functions.

  size_type copy(_CharT* __s, size_type __n, size_type __pos = 0) const {
    _STLP_FIX_LITERAL_BUG(__s)
    if (__pos > size())
      this->_M_throw_out_of_range();
    const size_type __len = (min) (__n, size() - __pos);
    _Traits::copy(__s, this->_M_start + __pos, __len);
    return __len;
  }

  void swap(_Self& __s) {
    _STLP_STD::swap(this->_M_start, __s._M_start);
    _STLP_STD::swap(this->_M_finish, __s._M_finish);
    _STLP_STD::swap(this->_M_end_of_storage, __s._M_end_of_storage);
  }

public:                         // Conversion to C string.

  const _CharT* c_str() const { return this->_M_start; }
  const _CharT* data()  const { return this->_M_start; }

public:                         // find.

  size_type find(const _Self& __s, size_type __pos = 0) const 
    { return find(__s._M_start, __pos, __s.size()); }

  size_type find(const _CharT* __s, size_type __pos = 0) const 
    { _STLP_FIX_LITERAL_BUG(__s) return find(__s, __pos, _Traits::length(__s)); }

  size_type find(const _CharT* __s, size_type __pos, size_type __n) const;

  // WIE: Versant schema compiler 5.2.2 ICE workaround
  size_type find(_CharT __c) const
    { return find(__c, 0) ; }
  size_type find(_CharT __c, size_type __pos /* = 0 */) const;

public:                         // rfind.

  size_type rfind(const _Self& __s, size_type __pos = npos) const 
    { return rfind(__s._M_start, __pos, __s.size()); }

  size_type rfind(const _CharT* __s, size_type __pos = npos) const 
    { _STLP_FIX_LITERAL_BUG(__s) return rfind(__s, __pos, _Traits::length(__s)); }

  size_type rfind(const _CharT* __s, size_type __pos, size_type __n) const;
  size_type rfind(_CharT __c, size_type __pos = npos) const;

public:                         // find_first_of
  
  size_type find_first_of(const _Self& __s, size_type __pos = 0) const 
    { return find_first_of(__s._M_start, __pos, __s.size()); }

  size_type find_first_of(const _CharT* __s, size_type __pos = 0) const 
    { _STLP_FIX_LITERAL_BUG(__s) return find_first_of(__s, __pos, _Traits::length(__s)); }

  size_type find_first_of(const _CharT* __s, size_type __pos, 
                          size_type __n) const;

  size_type find_first_of(_CharT __c, size_type __pos = 0) const 
    { return find(__c, __pos); }

public:                         // find_last_of

  size_type find_last_of(const _Self& __s,
                         size_type __pos = npos) const
    { return find_last_of(__s._M_start, __pos, __s.size()); }

  size_type find_last_of(const _CharT* __s, size_type __pos = npos) const 
    { _STLP_FIX_LITERAL_BUG(__s) return find_last_of(__s, __pos, _Traits::length(__s)); }

  size_type find_last_of(const _CharT* __s, size_type __pos, 
                         size_type __n) const;

  size_type find_last_of(_CharT __c, size_type __pos = npos) const {
    return rfind(__c, __pos);
  }

public:                         // find_first_not_of

  size_type find_first_not_of(const _Self& __s, 
                              size_type __pos = 0) const 
    { return find_first_not_of(__s._M_start, __pos, __s.size()); }

  size_type find_first_not_of(const _CharT* __s, size_type __pos = 0) const 
    { _STLP_FIX_LITERAL_BUG(__s) return find_first_not_of(__s, __pos, _Traits::length(__s)); }

  size_type find_first_not_of(const _CharT* __s, size_type __pos,
                              size_type __n) const;

  size_type find_first_not_of(_CharT __c, size_type __pos = 0) const;

public:                         // find_last_not_of

  size_type find_last_not_of(const _Self& __s, 
                             size_type __pos = npos) const
    { return find_last_not_of(__s._M_start, __pos, __s.size()); }

  size_type find_last_not_of(const _CharT* __s, size_type __pos = npos) const
    { _STLP_FIX_LITERAL_BUG(__s) return find_last_not_of(__s, __pos, _Traits::length(__s)); }

  size_type find_last_not_of(const _CharT* __s, size_type __pos,
                             size_type __n) const;

  size_type find_last_not_of(_CharT __c, size_type __pos = npos) const;

public:                         // Substring.

  _Self substr(size_type __pos = 0, size_type __n = npos) const {
    if (__pos > size())
      this->_M_throw_out_of_range();
    return _Self(this->_M_start + __pos, 
                        this->_M_start + __pos + (min) (__n, size() - __pos));
  }

public:                         // Compare

  int compare(const _Self& __s) const 
    { return _M_compare(this->_M_start, this->_M_finish, __s._M_start, __s._M_finish); }

  int compare(size_type __pos1, size_type __n1,
              const _Self& __s) const {
    if (__pos1 > size())
      this->_M_throw_out_of_range();
    return _M_compare(this->_M_start + __pos1, 
                      this->_M_start + __pos1 + (min) (__n1, size() - __pos1),
                      __s._M_start, __s._M_finish);
  }
    
  int compare(size_type __pos1, size_type __n1,
              const _Self& __s,
              size_type __pos2, size_type __n2) const {
    if (__pos1 > size() || __pos2 > __s.size())
      this->_M_throw_out_of_range();
    return _M_compare(this->_M_start + __pos1, 
                      this->_M_start + __pos1 + (min) (__n1, size() - __pos1),
                      __s._M_start + __pos2, 
                      __s._M_start + __pos2 + (min) (__n2, __s.size() - __pos2));
  }

  int compare(const _CharT* __s) const {
    _STLP_FIX_LITERAL_BUG(__s) 
      return _M_compare(this->_M_start, this->_M_finish, __s, __s + _Traits::length(__s));
  }

  int compare(size_type __pos1, size_type __n1, const _CharT* __s) const {
    _STLP_FIX_LITERAL_BUG(__s)
    if (__pos1 > size())
      this->_M_throw_out_of_range();
    return _M_compare(this->_M_start + __pos1, 
                      this->_M_start + __pos1 + (min) (__n1, size() - __pos1),
                      __s, __s + _Traits::length(__s));
  }

  int compare(size_type __pos1, size_type __n1, const _CharT* __s,
              size_type __n2) const {
    _STLP_FIX_LITERAL_BUG(__s)
    if (__pos1 > size())
      this->_M_throw_out_of_range();
    return _M_compare(this->_M_start + __pos1, 
                      this->_M_start + __pos1 + (min) (__n1, size() - __pos1),
                      __s, __s + __n2);
  }

public:                        // Helper functions for compare.
  
  static int _STLP_CALL _M_compare(const _CharT* __f1, const _CharT* __l1,
                        const _CharT* __f2, const _CharT* __l2) {
    const ptrdiff_t __n1 = __l1 - __f1;
    const ptrdiff_t __n2 = __l2 - __f2;
    const int cmp = _Traits::compare(__f1, __f2, (min) (__n1, __n2));
    return cmp != 0 ? cmp : (__n1 < __n2 ? -1 : (__n1 > __n2 ? 1 : 0));
  }
};

#if ! defined (__STLP_STATIC_CONST_INIT_BUG) && \
  __GNUC__ == 2 && __GNUC_MINOR__ == 96
template <class _CharT, class _Traits, class _Alloc>
const size_t basic_string<_CharT, _Traits, _Alloc>::npos = ~(size_t) 0;
#endif

# if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE_CLASS basic_string<char, char_traits<char>, allocator<char> >;
#  if defined (_STLP_HAS_WCHAR_T)
_STLP_EXPORT_TEMPLATE_CLASS basic_string<wchar_t, char_traits<wchar_t>, allocator<wchar_t> >;
#  endif
# endif /* _STLP_USE_TEMPLATE_EXPORT */

// ------------------------------------------------------------
// Non-member functions.

template <class _CharT, class _Traits, class _Alloc> inline basic_string<_CharT,_Traits,_Alloc> _STLP_CALL
operator+(const basic_string<_CharT,_Traits,_Alloc>& __s,
          const basic_string<_CharT,_Traits,_Alloc>& __y)
{
  typedef basic_string<_CharT,_Traits,_Alloc> _Str;
  typedef typename _Str::_Reserve_t _Reserve_t;
# ifdef __GNUC__
  // gcc counts this as a function
  _Str __result  = _Str(_Reserve_t(),__s.size() + __y.size());
# else
  _Str __result(_Reserve_t(), __s.size() + __y.size());
# endif
  __result.append(__s);
  __result.append(__y);
  return __result;
}

# if defined (__GNUC__) || defined (__MLCCPP__)
#  define _STLP_INIT_AMBIGUITY 1
# endif

template <class _CharT, class _Traits, class _Alloc> inline basic_string<_CharT,_Traits,_Alloc> _STLP_CALL
operator+(const _CharT* __s,
          const basic_string<_CharT,_Traits,_Alloc>& __y) {
  _STLP_FIX_LITERAL_BUG(__s)
  typedef basic_string<_CharT,_Traits,_Alloc> _Str;
  typedef typename _Str::_Reserve_t _Reserve_t;
  const size_t __n = _Traits::length(__s);
# ifdef _STLP_INIT_AMBIGUITY
  _Str __result = _Str(_Reserve_t(), __n + __y.size());
# else
  _Str __result(_Reserve_t(), __n + __y.size());
# endif
  __result.append(__s, __s + __n);
  __result.append(__y);
  return __result;
}

template <class _CharT, class _Traits, class _Alloc> inline basic_string<_CharT,_Traits,_Alloc> _STLP_CALL
operator+(_CharT __c,
          const basic_string<_CharT,_Traits,_Alloc>& __y) {
  typedef basic_string<_CharT,_Traits,_Alloc> _Str;
  typedef typename _Str::_Reserve_t _Reserve_t;
# ifdef _STLP_INIT_AMBIGUITY
  _Str __result = _Str(_Reserve_t(), 1 + __y.size());
# else
  _Str __result(_Reserve_t(), 1 + __y.size());
# endif
  __result.push_back(__c);
  __result.append(__y);
  return __result;
}

template <class _CharT, class _Traits, class _Alloc> inline basic_string<_CharT,_Traits,_Alloc> _STLP_CALL
operator+(const basic_string<_CharT,_Traits,_Alloc>& __x,
          const _CharT* __s) {
  _STLP_FIX_LITERAL_BUG(__s)
  typedef basic_string<_CharT,_Traits,_Alloc> _Str;
  typedef typename _Str::_Reserve_t _Reserve_t;
  const size_t __n = _Traits::length(__s);
# ifdef _STLP_INIT_AMBIGUITY
  _Str __result = _Str(_Reserve_t(), __x.size() + __n, __x.get_allocator());
# else
  _Str __result(_Reserve_t(), __x.size() + __n, __x.get_allocator());
# endif
  __result.append(__x);
  __result.append(__s, __s + __n);
  return __result;
}

template <class _CharT, class _Traits, class _Alloc> inline basic_string<_CharT,_Traits,_Alloc> _STLP_CALL
operator+(const basic_string<_CharT,_Traits,_Alloc>& __x,
          const _CharT __c) {
  typedef basic_string<_CharT,_Traits,_Alloc> _Str;
  typedef typename _Str::_Reserve_t _Reserve_t;
# ifdef _STLP_INIT_AMBIGUITY
  _Str __result = _Str(_Reserve_t(), __x.size() + 1, __x.get_allocator());
# else
  _Str __result(_Reserve_t(), __x.size() + 1, __x.get_allocator());
# endif
  __result.append(__x);
  __result.push_back(__c);
  return __result;
}

# undef _STLP_INIT_AMBIGUITY

// Operator== and operator!=

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator==(const basic_string<_CharT,_Traits,_Alloc>& __x,
           const basic_string<_CharT,_Traits,_Alloc>& __y) {
  return __x.size() == __y.size() && _Traits::compare(__x.data(), __y.data(), __x.size()) == 0;
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator==(const _CharT* __s,
           const basic_string<_CharT,_Traits,_Alloc>& __y) {
  _STLP_FIX_LITERAL_BUG(__s)
  size_t __n = _Traits::length(__s);
  return __n == __y.size() && _Traits::compare(__s, __y.data(), __n) == 0;
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator==(const basic_string<_CharT,_Traits,_Alloc>& __x,
           const _CharT* __s) {
  _STLP_FIX_LITERAL_BUG(__s)
  size_t __n = _Traits::length(__s);
  return __x.size() == __n && _Traits::compare(__x.data(), __s, __n) == 0;
}

// Operator< (and also >, <=, and >=).

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator<(const basic_string<_CharT,_Traits,_Alloc>& __x,
          const basic_string<_CharT,_Traits,_Alloc>& __y) {
  return basic_string<_CharT,_Traits,_Alloc> ::_M_compare(__x.begin(), __x.end(), 
		 __y.begin(), __y.end()) < 0;
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator<(const _CharT* __s,
          const basic_string<_CharT,_Traits,_Alloc>& __y) {
  _STLP_FIX_LITERAL_BUG(__s)
  size_t __n = _Traits::length(__s);
  return basic_string<_CharT,_Traits,_Alloc> ::_M_compare(__s, __s + __n, __y.begin(), __y.end()) < 0;
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator<(const basic_string<_CharT,_Traits,_Alloc>& __x,
          const _CharT* __s) {
  _STLP_FIX_LITERAL_BUG(__s)
  size_t __n = _Traits::length(__s);
  return basic_string<_CharT,_Traits,_Alloc> ::_M_compare(__x.begin(), __x.end(), __s, __s + __n) < 0;
}

#ifdef _STLP_USE_SEPARATE_RELOPS_NAMESPACE

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator!=(const basic_string<_CharT,_Traits,_Alloc>& __x,
           const basic_string<_CharT,_Traits,_Alloc>& __y) {
  return !(__x == __y);
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator>(const basic_string<_CharT,_Traits,_Alloc>& __x,
          const basic_string<_CharT,_Traits,_Alloc>& __y) {
  return __y < __x;
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator<=(const basic_string<_CharT,_Traits,_Alloc>& __x,
           const basic_string<_CharT,_Traits,_Alloc>& __y) {
  return !(__y < __x);
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator>=(const basic_string<_CharT,_Traits,_Alloc>& __x,
           const basic_string<_CharT,_Traits,_Alloc>& __y) {
  return !(__x < __y);
}

#endif /* _STLP_USE_SEPARATE_RELOPS_NAMESPACE */

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL 
operator!=(const _CharT* __s,
           const basic_string<_CharT,_Traits,_Alloc>& __y) {
  _STLP_FIX_LITERAL_BUG(__s)
  return !(__s == __y);
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL 
operator!=(const basic_string<_CharT,_Traits,_Alloc>& __x,
           const _CharT* __s) {
  _STLP_FIX_LITERAL_BUG(__s)
  return !(__x == __s);
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator>(const _CharT* __s,
          const basic_string<_CharT,_Traits,_Alloc>& __y) {
  _STLP_FIX_LITERAL_BUG(__s)
  return __y < __s;
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator>(const basic_string<_CharT,_Traits,_Alloc>& __x,
          const _CharT* __s) {
  _STLP_FIX_LITERAL_BUG(__s)
  return __s < __x;
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator<=(const _CharT* __s,
           const basic_string<_CharT,_Traits,_Alloc>& __y) {
  _STLP_FIX_LITERAL_BUG(__s)
  return !(__y < __s);
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator<=(const basic_string<_CharT,_Traits,_Alloc>& __x,
           const _CharT* __s) {
  _STLP_FIX_LITERAL_BUG(__s)
  return !(__s < __x);
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator>=(const _CharT* __s,
           const basic_string<_CharT,_Traits,_Alloc>& __y) {
  _STLP_FIX_LITERAL_BUG(__s)
  return !(__s < __y);
}

template <class _CharT, class _Traits, class _Alloc> inline bool _STLP_CALL
operator>=(const basic_string<_CharT,_Traits,_Alloc>& __x,
           const _CharT* __s) {
  _STLP_FIX_LITERAL_BUG(__s)
  return !(__x < __s);
}


// Swap.

#ifdef _STLP_FUNCTION_TMPL_PARTIAL_ORDER

template <class _CharT, class _Traits, class _Alloc> inline void _STLP_CALL
swap(basic_string<_CharT,_Traits,_Alloc>& __x,
     basic_string<_CharT,_Traits,_Alloc>& __y) {
  __x.swap(__y);
}

#endif /* _STLP_FUNCTION_TMPL_PARTIAL_ORDER */

template <class _CharT, class _Traits, class _Alloc> void  _STLP_CALL _S_string_copy(const basic_string<_CharT,_Traits,_Alloc>& __s,
                    _CharT* __buf,
                    size_t __n);

# undef basic_string

#if defined(_STLP_WINCE)
// A couple of functions to transfer between ASCII/Unicode

wstring __ASCIIToWide(const char *ascii);
string __WideToASCII(const wchar_t *wide);
#endif

_STLP_END_NAMESPACE

# ifdef _STLP_DEBUG
#  include <stl/debug/_string.h> 
# endif

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/_string.c> 
# endif

# include <stl/_string_io.h>  
# include <stl/_string_hash.h>  

#endif /* _STLP_STRING_H */


// Local Variables:
// mode:C++
// End:
