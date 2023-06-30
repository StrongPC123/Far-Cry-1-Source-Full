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

#ifndef _STLP_INTERNAL_WRAP_LIST_H
#define _STLP_INTERNAL_WRAP_LIST_H

#ifndef _STLP_INTERNAL_LIST_H
# include <stl/_list.h>
#endif

# ifdef _STLP_USE_NAMESPACES
namespace STLPORT { 
# endif

# if defined (_STLP_DEBUG)
#  define __LIST_SUPER _DBG_list<_Tp, _STLP_DEFAULT_ALLOCATOR(_Tp) >
# else
#  define __LIST_SUPER __list__<_Tp, _STLP_DEFAULT_ALLOCATOR(_Tp) >
# endif


// provide a "default" list adaptor
template <class _Tp>
class list : public __LIST_SUPER
{
public:
    typedef __LIST_SUPER _Super;
    __IMPORT_WITH_REVERSE_ITERATORS(_Super)
    __IMPORT_SUPER_COPY_ASSIGNMENT(list, list<_Tp>, __LIST_SUPER)
    list() { }
    explicit list(size_type __n, const _Tp& __value) : __LIST_SUPER(__n, __value) { }
    explicit list(size_type __n) :  __LIST_SUPER(__n) { } 
    list(const _Tp* __first, const _Tp* __last) : __LIST_SUPER(__first, __last) { } 
    list(const_iterator __first, const_iterator __last) : __LIST_SUPER(__first, __last) { }
# undef __LIST_SUPER
};

#  if defined (_STLP_BASE_MATCH_BUG)
template <class _Tp>
inline bool operator==(const list<_Tp>& __x, const list<_Tp>& __y) {
    typedef typename list<_Tp>::_Super _Super;
    return operator == ((const _Super&)__x,(const _Super&)__y);
}

template <class _Tp>
inline bool operator<(const list<_Tp>& __x, const list<_Tp>& __y) {
  return lexicographical_compare(__x.begin(), __x.end(),
                                 __y.begin(), __y.end());
}
#  endif

# ifdef _STLP_USE_NAMESPACES
} /* namespace STLPORT */
# endif

#endif /* _STLP_INTERNAL_LIST_H */

// Local Variables:
// mode:C++
// End:
