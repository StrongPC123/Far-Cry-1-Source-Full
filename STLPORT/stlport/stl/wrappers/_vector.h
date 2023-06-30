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

#ifndef _STLP_INTERNAL_WRAP_VECTOR_H
#define _STLP_INTERNAL_WRAP_VECTOR_H

#ifndef _STLP_INTERNAL_VECTOR_H
# include <stl/_vector.h>
#endif

# if defined (_STLP_DEBUG)
#  define _VEC_SUPER _DBG_vector<_Tp, _STLP_DEFAULT_ALLOCATOR(_Tp) >
# else
#  define _VEC_SUPER __vector__<_Tp, _STLP_DEFAULT_ALLOCATOR(_Tp) >
# endif

# ifdef _STLP_USE_NAMESPACES
namespace STLPORT { 
# endif

template <class _Tp>
class vector : public _VEC_SUPER
{
public:
    typedef _VEC_SUPER  _Super;
    __IMPORT_WITH_REVERSE_ITERATORS(_Super)
    __IMPORT_SUPER_COPY_ASSIGNMENT(vector, vector<_Tp>, _VEC_SUPER)
    vector() {}
    explicit vector(size_type __n, const _Tp& __value) : _VEC_SUPER(__n, __value) { }
    explicit vector(size_type __n) : _VEC_SUPER(__n) { }
    vector(const_iterator __first, const_iterator __last) : _VEC_SUPER(__first,__last) { }
# ifdef _STLP_DEBUG
  // certainly, no member templates here !
    vector(const _Tp* __first, const _Tp* __last) : _VEC_SUPER(__first,__last) { }    
# endif
    ~vector() {}
};

#  if defined (_STLP_BASE_MATCH_BUG)
template <class _Tp>
inline bool operator==(const vector<_Tp>& __x, const vector<_Tp>& __y) {
  return __x.size() == __y.size() &&
    equal(__x.begin(), __x.end(), __y.begin());
}

template <class _Tp>
inline bool operator<(const vector<_Tp>& __x, const vector<_Tp>& __y) {
  return lexicographical_compare(__x.begin(), __x.end(), 
				 __y.begin(), __y.end());
}
#  endif /* _STLP_BASE_MATCH_BUG */
#  undef _VEC_SUPER

// close std namespace
# ifdef _STLP_USE_NAMESPACES
}
# endif

#endif /* _STLP_WRAP_VECTOR_H */

// Local Variables:
// mode:C++
// End:
