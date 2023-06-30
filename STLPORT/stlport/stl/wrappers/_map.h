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

#ifndef _STLP_INTERNAL_WRAP_MAP_H
#define _STLP_INTERNAL_WRAP_MAP_H

#ifndef _STLP_INTERNAL_MAP_H
# include <stl/_map.h>
#endif

# ifdef _STLP_USE_NAMESPACES
namespace STLPORT { 
# endif

#  if defined (_STLP_MINIMUM_DEFAULT_TEMPLATE_PARAMS)
#   define __MAP_TEMPLATE_HEADER  template <class _Key, class _Tp>
#   define __MAP_ARGUMENTS        _Key, _Tp
#   define __MMAP_TEMPLATE_HEADER  template <class _Key, class _Tp>
#   define __MMAP_ARGUMENTS        _Key, _Tp
#   define _Compare less<_Key>
#  else
#   define __MAP_TEMPLATE_HEADER  template <class _Key, class _Tp, class _Compare >
#   define __MAP_ARGUMENTS        _Key, _Tp, _Compare
#   define __MMAP_TEMPLATE_HEADER  template <class _Key, class _Tp, class _Compare >
#   define __MMAP_ARGUMENTS        _Key, _Tp, _Compare
#  endif


#  define __MAP_SUPER  __map< _Key, _Tp, _Compare, _STLP_DEFAULT_PAIR_ALLOCATOR(const _Key, _Tp) >
#  define __MMAP_SUPER  __multimap< _Key, _Tp, _Compare, _STLP_DEFAULT_PAIR_ALLOCATOR(const _Key, _Tp) >

// provide a "default" map adaptor
__MAP_TEMPLATE_HEADER
class map : public __MAP_SUPER
{
  typedef map< __MAP_ARGUMENTS > _Self;
public:
    typedef __MAP_SUPER _Super;
    __IMPORT_WITH_REVERSE_ITERATORS(_Super)
    __IMPORT_SUPER_COPY_ASSIGNMENT(map, _Self, __MAP_SUPER)
    map() : __MAP_SUPER(_Compare()) {}
    explicit map(const _Compare& __comp) : __MAP_SUPER(__comp) {}
    map(const typename _Super::value_type* __first, 
	const typename _Super::value_type* __last) : 
      __MAP_SUPER(__first, __last, _Compare()) { }
    map(const typename _Super::value_type* __first, 
	const typename _Super::value_type* __last, 
        const _Compare& __comp) : __MAP_SUPER(__first, __last, __comp) { }
    map(typename _Super::const_iterator __first, 
	typename _Super::const_iterator __last) : 
        __MAP_SUPER(__first, __last, _Compare()) { }
    map(typename _Super::const_iterator __first, 
	typename _Super::const_iterator __last, 
        const _Compare& __comp) : __MAP_SUPER(__first, __last, __comp) { }
};

#  if defined (_STLP_BASE_MATCH_BUG)
__MAP_TEMPLATE_HEADER
inline bool operator==(const map< __MAP_ARGUMENTS >& __x, 
                       const map< __MAP_ARGUMENTS >& __y) {
  typedef __MAP_SUPER _Super;
  return operator==((const _Super&)__x,(const _Super&)__y);
}

__MAP_TEMPLATE_HEADER
inline bool operator<(const map< __MAP_ARGUMENTS >& __x, 
                      const map< __MAP_ARGUMENTS >& __y) {
  typedef __MAP_SUPER _Super;
  return operator < ((const _Super&)__x,(const _Super&)__y);
}
#  endif /* _STLP_BASE_MATCH_BUG */


// provide a "default" multimap adaptor
__MMAP_TEMPLATE_HEADER
class multimap : public __MMAP_SUPER
{
  typedef multimap< __MMAP_ARGUMENTS > _Self;
public:
    typedef __MMAP_SUPER  _Super;
    __IMPORT_WITH_REVERSE_ITERATORS(_Super)
    // copy & assignment from super
    __IMPORT_SUPER_COPY_ASSIGNMENT(multimap, _Self, __MMAP_SUPER)
    multimap() : __MMAP_SUPER(_Compare()) {}
    explicit multimap(const _Compare& __comp) : __MMAP_SUPER(__comp) {}
    multimap(const typename _Super::value_type* __first, 
	     const typename _Super::value_type* __last) : 
        __MMAP_SUPER(__first, __last, _Compare()) { }
    multimap(const typename _Super::value_type* __first,
	     const typename _Super::value_type* __last, 
	     const _Compare& __comp) : __MMAP_SUPER(__first, __last, __comp) { }
    multimap(typename _Super::const_iterator __first, 
	     typename _Super::const_iterator __last) : 
      __MMAP_SUPER(__first, __last, _Compare()) { }
    multimap(typename _Super::const_iterator __first, 
	     typename _Super::const_iterator __last, 
	     const _Compare& __comp) : __MMAP_SUPER(__first, __last, __comp) { }
};

#  if defined (_STLP_BASE_MATCH_BUG)
__MMAP_TEMPLATE_HEADER
inline bool operator==(const multimap< __MMAP_ARGUMENTS >& __x, 
                       const multimap< __MMAP_ARGUMENTS >& __y) {
  typedef __MMAP_SUPER  _Super;
  return (const _Super&)__x == (const _Super&)__y;
}

__MMAP_TEMPLATE_HEADER
inline bool operator<(const multimap< __MMAP_ARGUMENTS >& __x, 
                      const multimap< __MMAP_ARGUMENTS >& __y) {
  typedef __MMAP_SUPER  _Super;
  return (const _Super&)__x < (const _Super&)__y;
}
#  endif

# undef __MMAP_TEMPLATE_HEADER
# undef __MMAP_ARGUMENTS
# undef __MMAP_SUPER

# undef __MAP_TEMPLATE_HEADER
# undef __MAP_ARGUMENTS
# undef __MAP_SUPER

# undef _Compare

# ifdef _STLP_USE_NAMESPACES
} /* namespace STLPORT */
# endif

#endif /* _STLP_INTERNAL_WRAP_MAP_H */

// Local Variables:
// mode:C++
// End:
