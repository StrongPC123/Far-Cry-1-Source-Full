/*
 * Copyright (c) 1999, 2000
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

#ifndef _STLP_INTERNAL_WRAP_HASH_MAP_H
#define _STLP_INTERNAL_WRAP_HASH_MAP_H

#ifndef _STLP_INTERNAL_HASH_MAP_H
# include <stl/_hash_map.h>
#endif

# ifdef _STLP_USE_NAMESPACES
namespace STLPORT { 
# endif

// provide a "default" hash_map adaptor
#  if defined (_STLP_MINIMUM_DEFAULT_TEMPLATE_PARAMS)
#   define __HM_TEMPLATE_HEADER  template <class _Key, class _Tp>
#   define __HM_ARGUMENTS        _Key, _Tp
#   define __HM_BASE_ARGUMENTS   _Key, _Tp, hash<_Key>, equal_to<_Key>, _STLP_DEFAULT_PAIR_ALLOCATOR(const _Key, _Tp)
#  else
#   define __HM_TEMPLATE_HEADER  template <class _Key, class _Tp, class _HashFcn, class _EqualKey >
#   define __HM_ARGUMENTS        _Key, _Tp, _HashFcn, _EqualKey
#   define __HM_BASE_ARGUMENTS   _Key, _Tp, _HashFcn, _EqualKey, _STLP_DEFAULT_PAIR_ALLOCATOR(const _Key, _Tp)
#  endif


# define __HM_SUPER  __hash_map< __HM_BASE_ARGUMENTS >
# define __HMM_SUPER __hash_multimap< __HM_BASE_ARGUMENTS >

__HM_TEMPLATE_HEADER
class hash_map : public __HM_SUPER
{
  typedef hash_map< __HM_ARGUMENTS > _Self;
public:
  typedef __HM_SUPER _Super;
  __IMPORT_WITH_ITERATORS(_Super)
  typedef typename _Super::key_type key_type;
  typedef typename _Super::hasher hasher;
  typedef typename _Super::key_equal key_equal;
  typedef _Tp data_type;
  hash_map() {}
  hash_map(size_type __n) : __HM_SUPER(__n) {}
  hash_map(size_type __n, const hasher& __hf) : __HM_SUPER(__n, __hf) {}
  hash_map(size_type __n, const hasher& __hf, const key_equal& __eql): __HM_SUPER(__n, __hf, __eql) {}
  hash_map(const value_type* __f, const value_type* __l) : __HM_SUPER(__f,__l) {}
  hash_map(const value_type* __f, const value_type* __l, size_type __n): __HM_SUPER(__f,__l,__n) {}
  hash_map(const value_type* __f, const value_type* __l, size_type __n, 
           const hasher& __hf) : __HM_SUPER(__f,__l,__n,__hf) {}
  hash_map(const value_type* __f, const value_type* __l, size_type __n,
           const hasher& __hf, const key_equal& __eql) : __HM_SUPER(__f,__l,__n,__hf, __eql) {}
  hash_map(const_iterator __f, const_iterator __l) : __HM_SUPER(__f,__l) { }
  hash_map(const_iterator __f, const_iterator __l, size_type __n) : __HM_SUPER(__f,__l,__n) { }
  hash_map(const_iterator __f, const_iterator __l, size_type __n,
           const hasher& __hf) : __HM_SUPER(__f, __l, __n, __hf) { }
  hash_map(const_iterator __f, const_iterator __l, size_type __n,
           const hasher& __hf, const key_equal& __eql) : __HM_SUPER(__f, __l, __n, __hf, __eql) { }
# if defined (_STLP_BASE_MATCH_BUG)
  friend inline bool operator== _STLP_NULL_TMPL_ARGS (const _Self& __hm1, const _Self& __hm2);
# endif
};


# if defined (_STLP_BASE_MATCH_BUG)
__HM_TEMPLATE_HEADER
inline bool operator==(const hash_map< __HM_ARGUMENTS >& __hm1, 
                       const hash_map< __HM_ARGUMENTS >& __hm2)
{
    typedef __HM_SUPER _Super;
    return (const _Super&)__hm1 == (const _Super&)__hm2; 
}
# endif

// provide a "default" hash_multimap adaptor
__HM_TEMPLATE_HEADER
class hash_multimap : public __HMM_SUPER
{
  typedef hash_multimap< __HM_ARGUMENTS > _Self;
public:
  typedef __HMM_SUPER _Super;
  __IMPORT_WITH_ITERATORS(_Super)
  typedef typename _Super::key_type key_type;
  typedef typename _Super::hasher hasher;
  typedef typename _Super::key_equal key_equal;
  typedef _Tp data_type;
  hash_multimap() {}
  hash_multimap(size_type __n) : __HMM_SUPER(__n) {}
  hash_multimap(size_type __n, const hasher& __hf) : __HMM_SUPER(__n, __hf) {}
  hash_multimap(size_type __n, const hasher& __hf, const key_equal& __eql): __HMM_SUPER(__n, __hf, __eql) {}
  hash_multimap(const value_type* __f, const value_type* __l) : __HMM_SUPER(__f,__l) {}
  hash_multimap(const value_type* __f, const value_type* __l, size_type __n): __HMM_SUPER(__f,__l,__n) {}
  hash_multimap(const value_type* __f, const value_type* __l, size_type __n, 
           const hasher& __hf) : __HMM_SUPER(__f,__l,__n,__hf) {}
  hash_multimap(const value_type* __f, const value_type* __l, size_type __n,
           const hasher& __hf, const key_equal& __eql) : __HMM_SUPER(__f,__l,__n,__hf, __eql) {}

  hash_multimap(const_iterator __f, const_iterator __l) : __HMM_SUPER(__f,__l) { }
  hash_multimap(const_iterator __f, const_iterator __l, size_type __n) : __HMM_SUPER(__f,__l,__n) { }
  hash_multimap(const_iterator __f, const_iterator __l, size_type __n,
           const hasher& __hf) : __HMM_SUPER(__f, __l, __n, __hf) { }
  hash_multimap(const_iterator __f, const_iterator __l, size_type __n,
           const hasher& __hf, const key_equal& __eql) : __HMM_SUPER(__f, __l, __n, __hf, __eql) { }
# if defined (_STLP_BASE_MATCH_BUG)
  friend inline bool operator== _STLP_NULL_TMPL_ARGS (const _Self& __hm1, const _Self& __hm2);
# endif
};

# if defined (_STLP_BASE_MATCH_BUG)
__HM_TEMPLATE_HEADER
inline bool operator==(const hash_multimap< __HM_ARGUMENTS >& __hm1, 
                       const hash_multimap< __HM_ARGUMENTS >& __hm2)
{
    typedef __HMM_SUPER _Super;
    return (const _Super&)__hm1 == (const _Super&)__hm2; 
}
# endif

# undef __HM_SUPER
# undef __HMM_SUPER
# undef __HM_TEMPLATE_HEADER
# undef __HM_ARGUMENTS
# undef __HM_BASE_ARGUMENTS

# ifdef _STLP_USE_NAMESPACES
} /* namespace STLPORT */
# endif

#endif /* _STLP_INTERNAL_HASH_SET_H */

// Local Variables:
// mode:C++
// End:
