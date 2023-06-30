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

#ifndef _STLP_INTERNAL_CONSTRUCT_H
#define _STLP_INTERNAL_CONSTRUCT_H

# if defined (_STLP_DEBUG_UNINITIALIZED) && ! defined (_STLP_CSTRING)
# include <cstring>
# endif

# ifndef _STLP_INTERNAL_NEW_HEADER
#  include <stl/_new.h>
# endif


#ifndef _STLP_INTERNAL_ITERATOR_BASE_H
# include <stl/_iterator_base.h>
#endif

_STLP_BEGIN_NAMESPACE

# ifdef _STLP_TRIVIAL_DESTRUCTOR_BUG
template <class _Tp>
inline void __destroy_aux(_Tp* __pointer, const __false_type&) { __pointer->~_Tp(); }
template <class _Tp>
inline void __destroy_aux(_Tp* __pointer, const __true_type&) {}
# endif

template <class _Tp>
inline void _Destroy(_Tp* __pointer) {
# if _MSC_VER >= 1010
  __pointer;
# endif	// _MSC_VER >= 1000
# ifdef _STLP_TRIVIAL_DESTRUCTOR_BUG
  typedef typename __type_traits<_Tp>::has_trivial_destructor _Trivial_destructor;
  __destroy_aux(__pointer, _Trivial_destructor());
# else
#  if ( defined (__BORLANDC__) && ( __BORLANDC__ < 0x500 ) )
    __pointer->_Tp::~_Tp();
#  else
    __pointer->~_Tp();
#  endif
# endif
# ifdef _STLP_DEBUG_UNINITIALIZED
	memset((char*)__pointer, _STLP_SHRED_BYTE, sizeof(_Tp));
# endif
}

# if defined (new)
#   define _STLP_NEW_REDEFINE new
#   undef new
# endif 

# ifdef _STLP_DEFAULT_CONSTRUCTOR_BUG
template <class _T1>
inline void _Construct_aux (_T1* __p, const __false_type&) {
_STLP_PLACEMENT_NEW (__p) _T1();
}

template <class _T1>
inline void _Construct_aux (_T1* __p, const __true_type&) {
_STLP_PLACEMENT_NEW (__p) _T1(0);
}
# endif

template <class _T1, class _T2>
inline void _Construct(_T1* __p, const _T2& __val) {
# ifdef _STLP_DEBUG_UNINITIALIZED
	memset((char*)__p, _STLP_SHRED_BYTE, sizeof(_T1));
# endif
    _STLP_PLACEMENT_NEW (__p) _T1(__val);
}

template <class _T1>
inline void _Construct(_T1* __p) {
# ifdef _STLP_DEBUG_UNINITIALIZED
  memset((char*)__p, _STLP_SHRED_BYTE, sizeof(_T1));
# endif
# ifdef _STLP_DEFAULT_CONSTRUCTOR_BUG
typedef typename _Is_integer<_T1>::_Integral _Is_Integral;
_Construct_aux (__p, _Is_Integral() );
# else
  _STLP_PLACEMENT_NEW (__p) _T1();
# endif
}

# if defined(_STLP_NEW_REDEFINE)
# ifdef DEBUG_NEW
#  define new DEBUG_NEW
# endif
#  undef _STLP_NEW_REDEFINE
# endif 

template <class _ForwardIterator>
_STLP_INLINE_LOOP void
__destroy_aux(_ForwardIterator __first, _ForwardIterator __last, const __false_type&) {
  for ( ; __first != __last; ++__first)
    _STLP_STD::_Destroy(&*__first);
}

template <class _ForwardIterator> 
inline void __destroy_aux(_ForwardIterator, _ForwardIterator, const __true_type&) {}

template <class _ForwardIterator, class _Tp>
inline void 
__destroy(_ForwardIterator __first, _ForwardIterator __last, _Tp*) {
  typedef typename __type_traits<_Tp>::has_trivial_destructor _Trivial_destructor;
  __destroy_aux(__first, __last, _Trivial_destructor());
}

template <class _ForwardIterator>
inline void _Destroy(_ForwardIterator __first, _ForwardIterator __last) {
  __destroy(__first, __last, _STLP_VALUE_TYPE(__first, _ForwardIterator));
}

inline void _Destroy(char*, char*) {}
# ifdef _STLP_HAS_WCHAR_T // dwa 8/15/97
inline void _Destroy(wchar_t*, wchar_t*) {}
inline void _Destroy(const wchar_t*, const wchar_t*) {}
# endif

# ifndef _STLP_NO_ANACHRONISMS
// --------------------------------------------------
// Old names from the HP STL.

template <class _T1, class _T2>
inline void construct(_T1* __p, const _T2& __val) {_Construct(__p, __val); }
template <class _T1>
inline void construct(_T1* __p) { _Construct(__p); }
template <class _Tp>
inline void destroy(_Tp* __pointer) {  _STLP_STD::_Destroy(__pointer); }
template <class _ForwardIterator>
inline void destroy(_ForwardIterator __first, _ForwardIterator __last) { _STLP_STD::_Destroy(__first, __last); }
# endif
_STLP_END_NAMESPACE

#endif /* _STLP_INTERNAL_CONSTRUCT_H */

// Local Variables:
// mode:C++
// End:
