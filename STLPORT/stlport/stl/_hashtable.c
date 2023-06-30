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
#ifndef _STLP_HASHTABLE_C
#define _STLP_HASHTABLE_C

#ifndef _STLP_INTERNAL_HASHTABLE_H
# include <stl/_hashtable.h>
#endif

#ifdef _STLP_DEBUG
#  define hashtable __WORKAROUND_DBG_RENAME(hashtable)
#endif

_STLP_BEGIN_NAMESPACE

# define __PRIME_LIST_BODY { \
  53ul,         97ul,         193ul,       389ul,       769ul,      \
  1543ul,       3079ul,       6151ul,      12289ul,     24593ul,    \
  49157ul,      98317ul,      196613ul,    393241ul,    786433ul,   \
  1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul, \
  50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,\
  1610612741ul, 3221225473ul, 4294967291ul  \
}

#if ( _STLP_STATIC_TEMPLATE_DATA > 0 )
template <class _Tp>
const size_t _Stl_prime<_Tp>::_M_list[__stl_num_primes] = __PRIME_LIST_BODY;
#else
__DECLARE_INSTANCE(const size_t, 
		   _Stl_prime_type::_M_list[], =__PRIME_LIST_BODY);
#endif /* _STLP_STATIC_TEMPLATE_DATA */

# undef __PRIME_LIST_BODY

// fbp: these defines are for outline methods definitions.
// needed to definitions to be portable. Should not be used in method bodies.

# if defined ( _STLP_NESTED_TYPE_PARAM_BUG )
#  define __size_type__       size_t
#  define size_type           size_t
#  define value_type      _Val
#  define key_type        _Key
#  define _Node           _Hashtable_node<_Val>
#  define __reference__       _Val&

#  define __iterator__        _Ht_iterator<_Val, _Nonconst_traits<_Val>, _Key, _HF, _ExK, _EqK, _All>
#  define __const_iterator__  _Ht_iterator<_Val, _Const_traits<_Val>, _Key, _HF, _ExK, _EqK, _All>
# else
#  define __size_type__        _STLP_TYPENAME_ON_RETURN_TYPE hashtable<_Val, _Key, _HF, _ExK, _EqK, _All>::size_type
#  define __reference__        _STLP_TYPENAME_ON_RETURN_TYPE  hashtable<_Val, _Key, _HF, _ExK, _EqK, _All>::reference
#  define __iterator__         _STLP_TYPENAME_ON_RETURN_TYPE hashtable<_Val, _Key, _HF, _ExK, _EqK, _All>::iterator
# endif

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
_Hashtable_node<_Val>*
_Hashtable_iterator<_Val,_Key,_HF,_ExK,_EqK,_All>::_M_skip_to_next() {
  size_t __bucket = _M_ht->_M_bkt_num(_M_cur->_M_val);
  size_t __h_sz;
  __h_sz = this->_M_ht->bucket_count();

  _Node* __i=0;
  while (__i==0 && ++__bucket < __h_sz)
    __i = (_Node*)_M_ht->_M_buckets[__bucket];
  return __i;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, 
          class _All>
__size_type__
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::_M_next_size(size_type __n) const    { 
  const size_type* __first = (const size_type*)_Stl_prime_type::_M_list;
  const size_type* __last =  (const size_type*)_Stl_prime_type::_M_list + (int)__stl_num_primes;
  const size_type* pos = __lower_bound(__first, __last, __n, __less((size_type*)0), (ptrdiff_t*)0);
  return (pos == __last ? *(__last - 1) : *pos);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
bool 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::_M_equal(
						  const hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>& __ht1,
						  const hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>& __ht2)
{
  //  typedef _Hashtable_node<_Val> _Node;
  if (__ht1.bucket_count() != __ht2.bucket_count())
    return false;
  for (size_t __n = 0; __n < __ht1.bucket_count(); ++__n) {
    const _Node* __cur1 = __ht1._M_get_bucket(__n);
    const _Node* __cur2 = __ht2._M_get_bucket(__n);
    for ( ; __cur1 && __cur2 && __cur1->_M_val == __cur2->_M_val;
          __cur1 = __cur1->_M_next, __cur2 = __cur2->_M_next)
      {}
    if (__cur1 || __cur2)
      return false;
  }
  return true;
}  

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
pair< _Ht_iterator<_Val, _Nonconst_traits<_Val>, _Key, _HF, _ExK, _EqK, _All> , bool> 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::insert_unique_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = (_Node*)_M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj)))
      return pair<iterator, bool>(iterator(__cur, this), false);

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements._M_data;
  return pair<iterator, bool>(iterator(__tmp, this), true);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
__iterator__ 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::insert_equal_noresize(const value_type& __obj)
{
  const size_type __n = _M_bkt_num(__obj);
  _Node* __first = (_Node*)_M_buckets[__n];

  for (_Node* __cur = __first; __cur; __cur = __cur->_M_next) 
    if (_M_equals(_M_get_key(__cur->_M_val), _M_get_key(__obj))) {
      _Node* __tmp = _M_new_node(__obj);
      __tmp->_M_next = __cur->_M_next;
      __cur->_M_next = __tmp;
      ++_M_num_elements._M_data;
      return iterator(__tmp, this);
    }

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements._M_data;
  return iterator(__tmp, this);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
__reference__ 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::_M_insert(const value_type& __obj)
{
  resize(_M_num_elements._M_data + 1);

  size_type __n = _M_bkt_num(__obj);
  _Node* __first = (_Node*)_M_buckets[__n];

  _Node* __tmp = _M_new_node(__obj);
  __tmp->_M_next = __first;
  _M_buckets[__n] = __tmp;
  ++_M_num_elements._M_data;
  return __tmp->_M_val;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
__reference__ 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::find_or_insert(const value_type& __obj)
{

  _Node* __first = _M_find(_M_get_key(__obj));
  if (__first)
    return __first->_M_val;
  else
    return _M_insert(__obj);
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
pair< _Ht_iterator<_Val, _Nonconst_traits<_Val>, _Key, _HF, _ExK, _EqK, _All>,
      _Ht_iterator<_Val, _Nonconst_traits<_Val>, _Key, _HF, _ExK, _EqK, _All> > 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::equal_range(const key_type& __key)
{
  typedef pair<iterator, iterator> _Pii;
  const size_type __n = _M_bkt_num_key(__key);

  for (_Node* __first = (_Node*)_M_buckets[__n]; __first; __first = __first->_M_next)
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      for (_Node* __cur = __first->_M_next; __cur; __cur = __cur->_M_next)
        if (!_M_equals(_M_get_key(__cur->_M_val), __key))
          return _Pii(iterator(__first, this), iterator(__cur, this));
      for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
        if (_M_buckets[__m])
          return _Pii(iterator(__first, this),
                     iterator((_Node*)_M_buckets[__m], this));
      return _Pii(iterator(__first, this), end());
    }
  return _Pii(end(), end());
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
pair< _Ht_iterator<_Val, _Const_traits<_Val>, _Key, _HF, _ExK, _EqK, _All>, 
     _Ht_iterator<_Val, _Const_traits<_Val>, _Key, _HF, _ExK, _EqK, _All> > 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::equal_range(const key_type& __key) const
{
  typedef pair<const_iterator, const_iterator> _Pii;
  const size_type __n = _M_bkt_num_key(__key);

  for (const _Node* __first = (_Node*)_M_buckets[__n] ;
       __first; 
       __first = __first->_M_next) {
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      for (const _Node* __cur = __first->_M_next;
           __cur;
           __cur = __cur->_M_next)
        if (!_M_equals(_M_get_key(__cur->_M_val), __key))
          return _Pii(const_iterator(__first, this),
                      const_iterator(__cur, this));
      for (size_type __m = __n + 1; __m < _M_buckets.size(); ++__m)
        if (_M_buckets[__m])
          return _Pii(const_iterator(__first, this),
                      const_iterator((_Node*)_M_buckets[__m], this));
      return _Pii(const_iterator(__first, this), end());
    }
  }
  return _Pii(end(), end());
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
__size_type__ 
hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::erase(const key_type& __key)
{
  const size_type __n = _M_bkt_num_key(__key);
  _Node* __first = (_Node*)_M_buckets[__n];
  size_type __erased = 0;

  if (__first) {
    _Node* __cur = __first;
    _Node* __next = __cur->_M_next;
    while (__next) {
      if (_M_equals(_M_get_key(__next->_M_val), __key)) {
        __cur->_M_next = __next->_M_next;
        _M_delete_node(__next);
        __next = __cur->_M_next;
        ++__erased;
        --_M_num_elements._M_data;
      }
      else {
        __cur = __next;
        __next = __cur->_M_next;
      }
    }
    if (_M_equals(_M_get_key(__first->_M_val), __key)) {
      _M_buckets[__n] = __first->_M_next;
      _M_delete_node(__first);
      ++__erased;
      --_M_num_elements._M_data;
    }
  }
  return __erased;
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::erase(const const_iterator& __it)
{
  // const iterator& __it = __REINTERPRET_CAST(const iterator&,_c_it);
  const _Node* __p = __it._M_cur;
  if (__p) {
    const size_type __n = _M_bkt_num(__p->_M_val);
    _Node* __cur = (_Node*)_M_buckets[__n];

    if (__cur == __p) {
      _M_buckets[__n] = __cur->_M_next;
      _M_delete_node(__cur);
      --_M_num_elements._M_data;
    }
    else {
      _Node* __next = __cur->_M_next;
      while (__next) {
        if (__next == __p) {
          __cur->_M_next = __next->_M_next;
          _M_delete_node(__next);
          --_M_num_elements._M_data;
          break;
        }
        else {
          __cur = __next;
          __next = __cur->_M_next;
        }
      }
    }
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::erase(const_iterator _c_first, const_iterator _c_last)
{
  iterator& __first = (iterator&)_c_first;
  iterator& __last = (iterator&)_c_last;
  size_type __f_bucket = __first._M_cur ? 
    _M_bkt_num(__first._M_cur->_M_val) : _M_buckets.size();
  size_type __l_bucket = __last._M_cur ? 
    _M_bkt_num(__last._M_cur->_M_val) : _M_buckets.size();
  if (__first._M_cur == __last._M_cur)
    return;
  else if (__f_bucket == __l_bucket)
    _M_erase_bucket(__f_bucket, __first._M_cur, __last._M_cur);
  else {
    _M_erase_bucket(__f_bucket, __first._M_cur, 0);
    for (size_type __n = __f_bucket + 1; __n < __l_bucket; ++__n)
      _M_erase_bucket(__n, 0);
    if (__l_bucket != _M_buckets.size())
      _M_erase_bucket(__l_bucket, __last._M_cur);
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::resize(size_type __num_elements_hint)
{
  const size_type __old_n = _M_buckets.size();
  if (__num_elements_hint > __old_n) {
    const size_type __n = _M_next_size(__num_elements_hint);
    if (__n > __old_n) {
      _BucketVector __tmp(__n, (void*)(0),
			  _M_buckets.get_allocator());
      _STLP_TRY {
        for (size_type __bucket = 0; __bucket < __old_n; ++__bucket) {
          _Node* __first = (_Node*)_M_buckets[__bucket];
          while (__first) {
            size_type __new_bucket = _M_bkt_num(__first->_M_val, __n);
            _M_buckets[__bucket] = __first->_M_next;
            __first->_M_next = (_Node*)__tmp[__new_bucket];
            __tmp[__new_bucket] = __first;
            __first = (_Node*)_M_buckets[__bucket];          
          }
        }
        _M_buckets.swap(__tmp);
      }
#         ifdef _STLP_USE_EXCEPTIONS
      catch(...) {
        for (size_type __bucket = 0; __bucket < __tmp.size(); ++__bucket) {
          while (__tmp[__bucket]) {
            _Node* __next = ((_Node*)__tmp[__bucket])->_M_next;
            _M_delete_node((_Node*)__tmp[__bucket]);
            __tmp[__bucket] = __next;
          }
        }
        throw;
      }
#         endif /* _STLP_USE_EXCEPTIONS */
    }
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::_M_erase_bucket(const size_type __n, _Node* __first, _Node* __last)
{
  _Node* __cur = (_Node*)_M_buckets[__n];
  if (__cur == __first)
    _M_erase_bucket(__n, __last);
  else {
    _Node* __next;
    for (__next = __cur->_M_next; 
         __next != __first; 
         __cur = __next, __next = __cur->_M_next)
      ;
    while (__next != __last) {
      __cur->_M_next = __next->_M_next;
      _M_delete_node(__next);
      __next = __cur->_M_next;
      --_M_num_elements._M_data;
    }
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::_M_erase_bucket(const size_type __n, _Node* __last)
{
  _Node* __cur = (_Node*)_M_buckets[__n];
  while (__cur && __cur != __last) {
    _Node* __next = __cur->_M_next;
    _M_delete_node(__cur);
    __cur = __next;
    _M_buckets[__n] = __cur;
    --_M_num_elements._M_data;
  }
}

template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>::clear()
{
  for (size_type __i = 0; __i < _M_buckets.size(); ++__i) {
    _Node* __cur = (_Node*)_M_buckets[__i];
    while (__cur != 0) {
      _Node* __next = __cur->_M_next;
      _M_delete_node(__cur);
      __cur = __next;
    }
    _M_buckets[__i] = 0;
  }
  _M_num_elements._M_data = 0;
}

    
template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
void hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
  ::_M_copy_from(const hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>& __ht)
{
  _M_buckets.clear();
  _M_buckets.reserve(__ht._M_buckets.size());
  _M_buckets.insert(_M_buckets.end(), __ht._M_buckets.size(), (void*) 0);
  _STLP_TRY {
    for (size_type __i = 0; __i < __ht._M_buckets.size(); ++__i) {
      const _Node* __cur = (_Node*)__ht._M_buckets[__i];
      if (__cur) {
        _Node* __xcopy = _M_new_node(__cur->_M_val);
        _M_buckets[__i] = __xcopy;

        for (_Node* __next = __cur->_M_next; 
             __next; 
             __cur = __next, __next = __cur->_M_next) {
          __xcopy->_M_next = _M_new_node(__next->_M_val);
          __xcopy = __xcopy->_M_next;
        }
      }
    }
    _M_num_elements._M_data = __ht._M_num_elements._M_data;
  }
  _STLP_UNWIND(clear());
}

# undef __iterator__ 
# undef const_iterator
# undef __size_type__
# undef __reference__
# undef size_type       
# undef value_type      
# undef key_type        
# undef _Node            
# undef __stl_num_primes
# undef hashtable

_STLP_END_NAMESPACE

#endif /*  _STLP_HASHTABLE_C */

// Local Variables:
// mode:C++
// End:
