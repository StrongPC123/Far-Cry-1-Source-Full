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

#ifndef _STLP_INTERNAL_DBG_HASHTABLE_H
#define _STLP_INTERNAL_DBG_HASHTABLE_H

// Hashtable class, used to implement the hashed associative containers
// hash_set, hash_map, hash_multiset, and hash_multimap.

# include <stl/debug/_iterator.h>

#  undef  hashtable
#  undef  _DBG_hashtable
#  define _DBG_hashtable  hashtable

#  define _STLP_DBG_HT_SUPER \
__WORKAROUND_DBG_RENAME(hashtable) <_Val, _Key, _HF, _ExK, _EqK, _All>

_STLP_BEGIN_NAMESPACE

# ifdef _STLP_DEBUG_USE_DISTINCT_VALUE_TYPE_HELPERS
template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
inline _Val*
value_type(const  _DBG_iter_base< _STLP_DBG_HT_SUPER >&) {
  return (_Val*)0;
}

template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
inline forward_iterator_tag
iterator_category(const  _DBG_iter_base< _STLP_DBG_HT_SUPER >&) {
  return forward_iterator_tag();
}
# endif

template <class _Val, class _Key, class _HF,
          class _ExK, class _EqK, class _All>
class _DBG_hashtable : public _STLP_DBG_HT_SUPER {
  typedef _DBG_hashtable<_Val, _Key, _HF, _ExK, _EqK, _All> _Self;
  typedef _STLP_DBG_HT_SUPER _Base;

public:
  typedef _Key key_type;
  typedef _HF hasher;
  typedef _EqK key_equal;

  __IMPORT_CONTAINER_TYPEDEFS(_Base)

public:
  typedef _DBG_iter<_Base, _Nonconst_traits<value_type> >  iterator;
  typedef _DBG_iter<_Base, _Const_traits<value_type> > const_iterator;
  typedef typename _Base::iterator _Base_iterator;
  typedef typename _Base::const_iterator _Base_const_iterator;

protected:
  void _Invalidate_all() {_M_iter_list._Invalidate_all();}

  void _Invalidate_iterator(const const_iterator& __it) { 
    __invalidate_iterator(&_M_iter_list,__it); 
  }
  void _Invalidate_iterators(const const_iterator& __first, const const_iterator& __last) {
    const_iterator __cur = __first;
    while (__cur != __last) __invalidate_iterator(&_M_iter_list, __cur++); 
  }

  const _Base* _Get_base() const { return (const _Base*)this; }
  _Base* _Get_base() { return (_Base*)this; }

public:
  _DBG_hashtable(size_type __n,
		 const _HF&  __hf,
		 const _EqK& __eql,
		 const _ExK& __ext,
		 const allocator_type& __a = allocator_type()):
    _STLP_DBG_HT_SUPER(__n, __hf, __eql, __ext, __a),
    _M_iter_list(_Get_base()) {}
  
  _DBG_hashtable(size_type __n,
		 const _HF&    __hf,
		 const _EqK&   __eql,
		 const allocator_type& __a = allocator_type()):
    
    _STLP_DBG_HT_SUPER(__n, __hf, __eql, __a),
    _M_iter_list(_Get_base()) {}
  
  _DBG_hashtable(const _Self& __ht):
    _STLP_DBG_HT_SUPER(__ht),
    _M_iter_list(_Get_base()) {}
  
  _Self& operator= (const _Self& __ht) {
    _Invalidate_all();
    _Base::operator=(__ht);
    return *this;
  }
  
  void swap(_Self& __ht)
  {
   _M_iter_list._Swap_owners(__ht._M_iter_list);
   _Base::swap(__ht);
  }

  iterator begin() { return iterator(&_M_iter_list, _Base::begin()); }
  iterator end()   { return iterator(&_M_iter_list, _Base::end()); }

  const_iterator begin() const { return const_iterator(&_M_iter_list, _Base::begin()); }
  const_iterator end() const { return const_iterator(&_M_iter_list, _Base::end()); }

  pair<iterator, bool> insert_unique(const value_type& __obj)
  {
    pair < _Base_iterator, bool> __res =
      _Base::insert_unique(__obj);
    return pair<iterator, bool> ( iterator(&_M_iter_list, __res.first), __res.second);
  }

  iterator insert_equal(const value_type& __obj) {
    return iterator(&_M_iter_list, _Base::insert_equal(__obj));
  }
  
  pair<iterator, bool> insert_unique_noresize(const value_type& __obj) {
    pair < _Base_iterator, bool> __res =
      _Base::insert_unique_noresize(__obj);
    return pair<iterator, bool> ( iterator(&_M_iter_list, __res.first), __res.second);
  }
  
  iterator insert_equal_noresize(const value_type& __obj) {
    return iterator(&_M_iter_list, _Base::insert_equal_noresize(__obj));
  }
  
#ifdef _STLP_MEMBER_TEMPLATES
  template <class _InputIterator>
  void insert_unique(_InputIterator __f, _InputIterator __l) {
    _Base::insert_unique(__f, __l);
  }

  template <class _InputIterator>
  void insert_equal(_InputIterator __f, _InputIterator __l){
    _Base::insert_equal(__f, __l);
  }

#else /* _STLP_MEMBER_TEMPLATES */

  void insert_unique(const value_type* __f, const value_type* __l) {
    _Base::insert_unique(__f, __l);
  }
  
  void insert_equal(const value_type* __f, const value_type* __l) {
    _Base::insert_equal(__f, __l);
  }
  
  void insert_unique(const_iterator __f, const_iterator __l) {
    _Base::insert_unique(__f._M_iterator, __l._M_iterator);
  }
  
  void insert_equal(const_iterator __f, const_iterator __l) {
    _Base::insert_equal(__f._M_iterator, __l._M_iterator);
  }
#endif /*_STLP_MEMBER_TEMPLATES */
  
  iterator find(const key_type& __key) {
    return iterator(&_M_iter_list, _Base::find(__key));
  } 

  const_iterator find(const key_type& __key) const {
    return const_iterator(&_M_iter_list, _Base::find(__key));
  } 

  pair<iterator, iterator> 
  equal_range(const key_type& __key) {
    pair < _Base_iterator, _Base_iterator > __res =
      _Base::equal_range(__key);
    return pair<iterator,iterator> (iterator(&_M_iter_list,__res.first),
				    iterator(&_M_iter_list,__res.second));
  }

  pair<const_iterator, const_iterator> 
  equal_range(const key_type& __key) const {
    pair <  _Base_const_iterator, _Base_const_iterator > __res =
      _Base::equal_range(__key);
    return pair<const_iterator,const_iterator> (const_iterator(&_M_iter_list,__res.first),
				    const_iterator(&_M_iter_list,__res.second));
  }

  size_type erase(const key_type& __key) {
    pair<const_iterator, const_iterator> __p = equal_range(__key);
    size_type __n = distance(__p.first, __p.second);
    _Invalidate_iterators(__p.first, __p.second);
    _Base::erase(__p.first._M_iterator, __p.second._M_iterator);
    return __n;
  }

  void erase(const const_iterator& __it) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __it))
    _STLP_DEBUG_CHECK(_Dereferenceable(__it))
    _Invalidate_iterator(__it);
    _Base::erase(__it._M_iterator);
  }
  void erase(const_iterator __first, const_iterator __last) {
    _STLP_DEBUG_CHECK(__check_if_owner(&_M_iter_list, __first)&&
                      __check_if_owner(&_M_iter_list, __last))
    _Invalidate_iterators(__first, __last);
    _Base::erase(__first._M_iterator, __last._M_iterator);
  }
  void resize(size_type __num_elements_hint) {
    _Base::resize(__num_elements_hint);
  }
  
  void clear() {
    _Invalidate_all();
    _Base::clear();
  }

private:
  __owned_list _M_iter_list;

};

#define _STLP_TEMPLATE_HEADER template <class _Val, class _Key, class _HF, class _ExK, class _EqK, class _All>
#define _STLP_TEMPLATE_CONTAINER _DBG_hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
#define _STLP_TEMPLATE_CONTAINER_BASE hashtable<_Val,_Key,_HF,_ExK,_EqK,_All>
#include <stl/debug/_relops_hash_cont.h>
#undef _STLP_TEMPLATE_CONTAINER_BASE
#undef _STLP_TEMPLATE_CONTAINER
#undef _STLP_TEMPLATE_HEADER

_STLP_END_NAMESPACE
#  undef  hashtable

#endif /* _STLP_INTERNAL_HASHTABLE_H */

// Local Variables:
// mode:C++
// End:


