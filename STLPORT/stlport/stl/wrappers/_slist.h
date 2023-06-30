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

#ifndef _STLP_INTERNAL_WRAP_SLIST_H
#define _STLP_INTERNAL_WRAP_SLIST_H

#ifndef _STLP_INTERNAL_SLIST_H
# include <stl/_slist.h>
#endif

# if defined (_STLP_DEBUG)
#  define __SL_SUPER _DBG_slist<_Tp, _STLP_DEFAULT_ALLOCATOR(_Tp) >
# else
#  define __SL_SUPER __slist__<_Tp, _STLP_DEFAULT_ALLOCATOR(_Tp) >
# endif


# ifdef _STLP_USE_NAMESPACES
namespace STLPORT { 
# endif
 
// provide a "default" list adaptor
template <class _Tp>
class slist : public  __SL_SUPER
{
public:
    typedef __SL_SUPER _Super;
    __IMPORT_WITH_ITERATORS(_Super)
    __IMPORT_SUPER_COPY_ASSIGNMENT(slist, slist<_Tp>, __SL_SUPER)
    slist() { }
    explicit slist(size_type __n, const _Tp& __value) : __SL_SUPER(__n, __value) { }
    explicit slist(size_type __n) :  __SL_SUPER(__n) { } 
    slist(const _Tp* __first, const _Tp* __last) : __SL_SUPER(__first, __last) { } 
    slist(const_iterator __first, const_iterator __last) : __SL_SUPER(__first, __last) { }
};

#  if defined (_STLP_BASE_MATCH_BUG)
template <class _Tp>
inline bool operator==(const slist<_Tp>& __x, const slist<_Tp>& __y) {
    typedef typename slist<_Tp>::_Super _Super;
    return operator == ((const _Super&)__x,(const _Super&)__y);
}

template <class _Tp>
inline bool operator<(const slist<_Tp>& __x, const slist<_Tp>& __y) {
    typedef typename slist<_Tp>::_Super _Super;
    return operator < ((const _Super&)__x,(const _Super&)__y);
}
#  endif
#  undef __SL_SUPER

# ifdef _STLP_USE_NAMESPACES
} /* namespace STLPORT */
# endif

#endif /* _STLP_INTERNAL_WRAP_SLIST_H */

// Local Variables:
// mode:C++
// End:
