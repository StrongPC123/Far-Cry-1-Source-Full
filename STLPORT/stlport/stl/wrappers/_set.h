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

#ifndef _STLP_INTERNAL_WRAP_SET_H
#define _STLP_INTERNAL_WRAP_SET_H

#ifndef _STLP_INTERNAL_SET_H
# include <stl/_set.h>
#endif

# ifdef _STLP_USE_NAMESPACES
namespace STLPORT { 
# endif

#  if defined (_STLP_MINIMUM_DEFAULT_TEMPLATE_PARAMS)
#   define __SET_TEMPLATE_HEADER  template <class _Key>
#   define __SET_ARGUMENTS        _Key
#   define __MSET_TEMPLATE_HEADER  template <class _Key>
#   define __MSET_ARGUMENTS        _Key
#   define _Compare less<_Key>
#  else
#   define __SET_TEMPLATE_HEADER  template <class _Key, class _Compare >
#   define __SET_ARGUMENTS        _Key, _Compare
#   define __MSET_TEMPLATE_HEADER  template <class _Key, class _Compare >
#   define __MSET_ARGUMENTS        _Key, _Compare
#  endif

#   define __SET_SUPER  __set< _Key, _Compare, _STLP_DEFAULT_ALLOCATOR(_Key) >
#   define __MSET_SUPER __multiset< _Key, _Compare, _STLP_DEFAULT_ALLOCATOR(_Key) >

// provide a "default" set adaptor
__SET_TEMPLATE_HEADER
class set : public __SET_SUPER
{
  typedef set< __SET_ARGUMENTS > _Self;
public:
    typedef __SET_SUPER _Super;
    __IMPORT_WITH_REVERSE_ITERATORS(_Super)
    // copy & assignment from super
    __IMPORT_SUPER_COPY_ASSIGNMENT(set,_Self,__SET_SUPER)
    // specific constructors
    explicit set() : __SET_SUPER(_Compare()) {}
    explicit set(const _Compare& __comp) : __SET_SUPER(__comp) {}
    set(const value_type* __first, const value_type* __last) : 
        __SET_SUPER(__first, __last, _Compare()) { }
    set(const value_type* __first, const value_type* __last, 
        const _Compare& __comp) : __SET_SUPER(__first, __last, __comp) { }
    set(const_iterator __first, const_iterator __last) : 
        __SET_SUPER(__first, __last, _Compare()) { }
    set(const_iterator __first, const_iterator __last, 
        const _Compare& __comp) : __SET_SUPER(__first, __last, __comp) { }
};

#  if defined (_STLP_BASE_MATCH_BUG)
__SET_TEMPLATE_HEADER 
inline bool operator==(const set< __SET_ARGUMENTS >& __x, 
                       const set< __SET_ARGUMENTS >& __y) {
  typedef __SET_SUPER _Super;
  return operator==((const _Super&)__x,(const _Super&)__y);
}

__SET_TEMPLATE_HEADER 
inline bool operator<(const set< __SET_ARGUMENTS >& __x, 
                      const set< __SET_ARGUMENTS >& __y) {
  typedef __SET_SUPER _Super;
  return operator < ((const _Super&)__x , (const _Super&)__y);
}
#  endif

// provide a "default" multiset adaptor
__MSET_TEMPLATE_HEADER 
class multiset : public __MSET_SUPER
{
    typedef multiset< __MSET_ARGUMENTS > _Self;
public:
    typedef __MSET_SUPER _Super;
    __IMPORT_WITH_REVERSE_ITERATORS(_Super)
    // copy & assignment from super
    __IMPORT_SUPER_COPY_ASSIGNMENT(multiset, _Self, __MSET_SUPER)
    explicit multiset() : __MSET_SUPER(_Compare()) {}
    explicit multiset(const _Compare& __comp) : __MSET_SUPER(__comp) {}
    multiset(const value_type* __first, const value_type* __last) : 
        __MSET_SUPER(__first, __last, _Compare()) { }
    multiset(const value_type* __first, const value_type* __last, 
        const _Compare& __comp) : __MSET_SUPER(__first, __last, __comp) { }
    multiset(const_iterator __first, const_iterator __last) : 
        __MSET_SUPER(__first, __last, _Compare()) { }
    multiset(const_iterator __first, const_iterator __last, 
        const _Compare& __comp) : __MSET_SUPER(__first, __last, __comp) { }
};

#  if defined (_STLP_BASE_MATCH_BUG)   
__MSET_TEMPLATE_HEADER 
inline bool operator==(const multiset< __MSET_ARGUMENTS >& __x, 
                       const multiset< __MSET_ARGUMENTS >& __y) {
  typedef __MSET_SUPER  _Super;
  return (const _Super&)__x == (const _Super&)__y;
}

__MSET_TEMPLATE_HEADER 
inline bool operator<(const multiset< __MSET_ARGUMENTS >& __x, 
                      const multiset< __MSET_ARGUMENTS >& __y) {
  typedef __MSET_SUPER _Super;
  return (const _Super&)__x < (const _Super&)__y;
}
#  endif

# undef __MSET_TEMPLATE_HEADER
# undef __MSET_ARGUMENTS
# undef __MSET_SUPER

# undef __SET_TEMPLATE_HEADER
# undef __SET_ARGUMENTS
# undef __SET_SUPER 
# undef _Compare

# ifdef _STLP_USE_NAMESPACES
} /* namespace STLPORT */
# endif

#endif /* _STLP_INTERNAL_WRAP_SET_H */

// Local Variables:
// mode:C++
// End:
