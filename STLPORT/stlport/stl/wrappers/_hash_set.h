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

#ifndef _STLP_INTERNAL_WRAP_HASH_SET_H
#define _STLP_INTERNAL_WRAP_HASH_SET_H

#ifndef _STLP_INTERNAL_HASH_SET_H
# include <stl/_hash_set.h>
#endif

# ifdef _STLP_USE_NAMESPACES
namespace STLPORT { 
# endif

#  if defined (_STLP_MINIMUM_DEFAULT_TEMPLATE_PARAMS)
#   define __HS_TEMPLATE_HEADER  template <class _Value>
#   define __HS_ARGUMENTS        _Value
#   define __HS_BASE_ARGUMENTS   _Value, hash<_Value>, equal_to<_Value>, _STLP_DEFAULT_ALLOCATOR(_Value)
#  else
#   define __HS_TEMPLATE_HEADER  template <class _Value, class _HashFcn, class _EqualKey >
#   define __HS_ARGUMENTS        _Value, _HashFcn, _EqualKey
#   define __HS_BASE_ARGUMENTS   _Value, _HashFcn, _EqualKey, _STLP_DEFAULT_ALLOCATOR(_Value)
#  endif


#  define __HS_SUPER  __hash_set< __HS_BASE_ARGUMENTS >
#  define __HMS_SUPER __hash_multiset< __HS_BASE_ARGUMENTS >


// provide a "default" hash_set adaptor
__HS_TEMPLATE_HEADER
class hash_set : public __HS_SUPER 
{
  typedef hash_set< __HS_ARGUMENTS > _Self;
public:
  typedef  __HS_SUPER _Super;
  __IMPORT_WITH_ITERATORS(_Super)
  typedef typename _Super::key_type key_type;
  typedef typename _Super::hasher hasher;
  typedef typename _Super::key_equal key_equal;
  hash_set() {}
  hash_set(size_type n) : __HS_SUPER(n) {}
  hash_set(size_type n, const hasher& hf) : __HS_SUPER(n, hf) {}
  hash_set(size_type n, const hasher& hf, const key_equal& eql): __HS_SUPER(n, hf, eql) {}

  hash_set(const value_type* f, const value_type* l) : __HS_SUPER(f,l) {}
  hash_set(const value_type* f, const value_type* l, size_type n): __HS_SUPER(f,l,n) {}
  hash_set(const value_type* f, const value_type* l, size_type n,
           const hasher& hf) : __HS_SUPER(f,l,n,hf) {}
  hash_set(const value_type* f, const value_type* l, size_type n,
           const hasher& hf, const key_equal& eql) : __HS_SUPER(f,l,n,hf, eql) {}

  hash_set(const_iterator f, const_iterator l) : __HS_SUPER(f,l) { }
  hash_set(const_iterator f, const_iterator l, size_type n) : __HS_SUPER(f,l,n) { }
  hash_set(const_iterator f, const_iterator l, size_type n,
           const hasher& hf) : __HS_SUPER(f, l, n, hf) { }
  hash_set(const_iterator f, const_iterator l, size_type n,
           const hasher& hf, const key_equal& eql) : __HS_SUPER(f, l, n, hf, eql) { }
# if defined (_STLP_BASE_MATCH_BUG)
    friend inline bool operator== _STLP_NULL_TMPL_ARGS (const _Self& hs1, const _Self& hs2);
# endif
};

# if defined (_STLP_BASE_MATCH_BUG)
__HS_TEMPLATE_HEADER
inline bool operator==(const hash_set< __HS_ARGUMENTS >& hs1, 
                       const hash_set< __HS_ARGUMENTS >& hs2)
{
    typedef __HS_SUPER _Super;
    return (const _Super&)hs1 == (const _Super&)hs2; 
}
# endif

// provide a "default" hash_multiset adaptor
__HS_TEMPLATE_HEADER
class hash_multiset : public __HMS_SUPER
{
  typedef hash_multiset< __HS_ARGUMENTS > _Self;
public:
  typedef __HMS_SUPER _Super;
  __IMPORT_WITH_ITERATORS(_Super)
  typedef typename _Super::key_type key_type;
  typedef typename _Super::hasher hasher;
  typedef typename _Super::key_equal key_equal;

  hash_multiset() {}
  hash_multiset(size_type __n) : __HMS_SUPER(__n) {}
  hash_multiset(size_type __n, const hasher& __hf) : __HMS_SUPER(__n, __hf) {}
  hash_multiset(size_type __n, const hasher& __hf, const key_equal& __eql): __HMS_SUPER(__n, __hf, __eql) {}

  hash_multiset(const value_type* __f, const value_type* __l) : __HMS_SUPER(__f,__l) {}
  hash_multiset(const value_type* __f, const value_type* __l, size_type __n): __HMS_SUPER(__f,__l,__n) {}
  hash_multiset(const value_type* __f, const value_type* __l, size_type __n,
           const hasher& __hf) : __HMS_SUPER(__f,__l,__n,__hf) {}
  hash_multiset(const value_type* __f, const value_type* __l, size_type __n,
           const hasher& __hf, const key_equal& __eql) : __HMS_SUPER(__f,__l,__n,__hf, __eql) {}

  hash_multiset(const_iterator __f, const_iterator __l) : __HMS_SUPER(__f,__l) { }
  hash_multiset(const_iterator __f, const_iterator __l, size_type __n) : __HMS_SUPER(__f,__l,__n) { }
  hash_multiset(const_iterator __f, const_iterator __l, size_type __n,
           const hasher& __hf) : __HMS_SUPER(__f, __l, __n, __hf) { }
  hash_multiset(const_iterator __f, const_iterator __l, size_type __n,
           const hasher& __hf, const key_equal& __eql) : __HMS_SUPER(__f, __l, __n, __hf, __eql) { }
# if defined (_STLP_BASE_MATCH_BUG)
  friend inline bool operator== _STLP_NULL_TMPL_ARGS (const _Self& __hs1, const _Self& __hs2);
# endif
};

# if defined (_STLP_BASE_MATCH_BUG)
__HS_TEMPLATE_HEADER
inline bool operator==(const hash_multiset< __HS_ARGUMENTS >& __hs1, 
                       const hash_multiset< __HS_ARGUMENTS >& __hs2)
{
    typedef __HMS_SUPER  __s;
    return _STLP_STD::operator==((const __s&)__hs1,(const __s&)__hs2);
}
# endif


# undef __HS_SUPER
# undef __HMS_SUPER
# undef __HS_ARGUMENTS
# undef __HS_BASE_ARGUMENTS
# undef __HS_TEMPLATE_HEADER

# ifdef _STLP_USE_NAMESPACES
} /* namespace STLPORT */
# endif

#endif /* _STLP_INTERNAL_HASH_SET_H */

// Local Variables:
// mode:C++
// End:
