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

#ifndef _STLP_INTERNAL_DEQUE_H
# include <stl/_deque.h>
#endif

# if defined (_STLP_DEBUG)
#  define _DEQUE_SUPER_NAME _DBG_deque
# else
#  define _DEQUE_SUPER_NAME __deque__
# endif

# define _DEQUE_SUPER   _DEQUE_SUPER_NAME<_Tp, _STLP_DEFAULT_ALLOCATOR(_Tp) >

_STLP_BEGIN_NAMESPACE

// provide a "default" deque adaptor
template <class _Tp>
class deque : public _DEQUE_SUPER {
public:
  typedef deque<_Tp> _Self;
  typedef _DEQUE_SUPER _Super;
  __IMPORT_WITH_REVERSE_ITERATORS(_Super)
    __IMPORT_SUPER_COPY_ASSIGNMENT(deque, _Self, _DEQUE_SUPER)
    deque() : _DEQUE_SUPER() { }
  deque(size_type __n, const _Tp& __value) : _DEQUE_SUPER(__n, __value) { }
  explicit deque(size_type __n) : _DEQUE_SUPER(__n) { }
  deque(const _Tp* __first, const _Tp* __last) : _DEQUE_SUPER(__first, __last) { }
  deque(const_iterator __first, const_iterator __last) : _DEQUE_SUPER(__first, __last) { }
  ~deque() { }
};

#  if defined (_STLP_BASE_MATCH_BUG)
template <class _Tp>
inline bool 
operator==(const deque<_Tp>& __x, const deque<_Tp>& __y) {
    return __x.size() == __y.size() && equal(__x.begin(), __x.end(), __y.begin());
}

template <class _Tp>
inline bool 
operator<(const deque<_Tp>& __x, const deque<_Tp>& __y) {
    return lexicographical_compare(__x.begin(), __x.end(), __y.begin(), __y.end());
}
#  endif /* BASE_MATCH_BUG */

# undef _DEQUE_SUPER

_STLP_END_NAMESPACE  

// Local Variables:
// mode:C++
// End:
