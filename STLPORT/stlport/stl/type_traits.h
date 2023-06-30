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

#ifndef _STLP_TYPE_TRAITS_H
#define _STLP_TYPE_TRAITS_H

/*
This header file provides a framework for allowing compile time dispatch
based on type attributes. This is useful when writing template code.
For example, when making a copy of an array of an unknown type, it helps
to know if the type has a trivial copy constructor or not, to help decide
if a memcpy can be used.

The class template __type_traits provides a series of typedefs each of
which is either __true_type or __false_type. The argument to
__type_traits can be any type. The typedefs within this template will
attain their correct values by one of these means:
    1. The general instantiation contain conservative values which work
       for all types.
    2. Specializations may be declared to make distinctions between types.
    3. Some compilers (such as the Silicon Graphics N32 and N64 compilers)
       will automatically provide the appropriate specializations for all
       types.

EXAMPLE:

//Copy an array of elements which have non-trivial copy constructors
template <class T> void copy(T* source, T* destination, int n, __false_type);
//Copy an array of elements which have trivial copy constructors. Use memcpy.
template <class T> void copy(T* source, T* destination, int n, __true_type);

//Copy an array of any type by using the most efficient copy mechanism
template <class T> inline void copy(T* source,T* destination,int n) {
   copy(source, destination, n,
        typename __type_traits<T>::has_trivial_copy_constructor());
}
*/

#ifdef __WATCOMC__
# include <stl/_cwchar.h>
#endif

_STLP_BEGIN_NAMESPACE

struct __true_type {};
struct __false_type {};


template <int _Is> struct __bool2type {
  typedef __false_type _Ret; 
};

_STLP_TEMPLATE_NULL
struct __bool2type<1> { typedef __true_type _Ret; };

_STLP_TEMPLATE_NULL
struct __bool2type<0> { typedef __false_type _Ret; };

// logical end of 3 predicated
template <class _P1, class _P2, class _P3>
struct _Land3 {
  typedef __false_type _Ret;
};

_STLP_TEMPLATE_NULL
struct _Land3<__true_type, __true_type, __true_type> {
  typedef __true_type _Ret;
};


// Forward declarations.
template <class _Tp> struct __type_traits; 
template <int _IsPOD> struct __type_traits_aux {
   typedef __false_type    has_trivial_default_constructor;
   typedef __false_type    has_trivial_copy_constructor;
   typedef __false_type    has_trivial_assignment_operator;
   typedef __false_type    has_trivial_destructor;
   typedef __false_type    is_POD_type;
};

_STLP_TEMPLATE_NULL
struct __type_traits_aux<0> {
   typedef __false_type    has_trivial_default_constructor;
   typedef __false_type    has_trivial_copy_constructor;
   typedef __false_type    has_trivial_assignment_operator;
   typedef __false_type    has_trivial_destructor;
   typedef __false_type    is_POD_type;
};

_STLP_TEMPLATE_NULL
struct __type_traits_aux<1> { 
   typedef __true_type    has_trivial_default_constructor;
   typedef __true_type    has_trivial_copy_constructor;
   typedef __true_type    has_trivial_assignment_operator;
   typedef __true_type    has_trivial_destructor;
   typedef __true_type    is_POD_type;
};

# ifdef _STLP_SIMULATE_PARTIAL_SPEC_FOR_TYPE_TRAITS

// Boris : simulation technique is used here according to Adobe Open Source License Version 1.0.
// Copyright 2000 Adobe Systems Incorporated and others. All rights reserved.
// Authors: Mat Marcus and Jesse Jones
// The original version of this source code may be found at
// http://opensource.adobe.com.

struct _PointerShim {
  // Since the compiler only allows at most one non-trivial
  // implicit conversion we can make use of a shim class to
  // be sure that IsPtr below doesn't accept classes with
  // implicit pointer conversion operators
  _PointerShim(const volatile void*); // no implementation
};

// These are the discriminating functions

char _STLP_CALL _IsP(bool, _PointerShim); // no implementation is required
char* _STLP_CALL _IsP(bool, ...);          // no implementation is required

template <class _Tp>
char _STLP_CALL _IsSameFun(bool, _Tp*, _Tp*); // no implementation is required
char* _STLP_CALL _IsSameFun(bool, ...);          // no implementation is required

template <class _Tp1, class _Tp2>
struct _IsSame {
  // boris : check!
  static _Tp1* __null_rep1();
  static _Tp2* __null_rep2();
  enum { _Ret = (sizeof(_IsSameFun(false,__null_rep1(),__null_rep2())) == sizeof(char)) };
};

template <class _Tp>
struct _IsPtr {
  
  // This template meta function takes a type T
  // and returns true exactly when T is a pointer.
  // One can imagine meta-functions discriminating on
  // other criteria.
  static _Tp& __null_rep();
  enum { _Ret = (sizeof(_IsP(false,__null_rep())) == sizeof(char)) };

};

template <class _Tp>
struct _IsPtrType {
  enum { _Is =  _IsPtr<_Tp>::_Ret } ;
  typedef __bool2type< _Is > _BT;
  typedef typename _BT::_Ret _Type;
  static _Type _Ret() { return _Type(); }
};

template <class _Tp1, class _Tp2>
struct _BothPtrType {
  typedef __bool2type< _IsPtr<_Tp1>::_Ret> _B1;
  typedef __bool2type< _IsPtr<_Tp2>::_Ret> _B2;
  typedef typename _B1::_Ret _Type1;
  typedef typename _B2::_Ret _Type2;
  typedef typename _Land3<_Type1, _Type2, __true_type>::_Ret _Type;
  static _Type _Ret() { return _Type(); }
};

// we make general case dependant on the fact the type is actually a pointer.
 
template <class _Tp>
struct __type_traits : __type_traits_aux<_IsPtr<_Tp>::_Ret> {};

# else

template <class _Tp>
struct __type_traits { 
   typedef __true_type     this_dummy_member_must_be_first;
                   /* Do not remove this member. It informs a compiler which
                      automatically specializes __type_traits that this
                      __type_traits template is special. It just makes sure that
                      things work if an implementation is using a template
                      called __type_traits for something unrelated. */

   /* The following restrictions should be observed for the sake of
      compilers which automatically produce type specific specializations 
      of this class:
          - You may reorder the members below if you wish
          - You may remove any of the members below if you wish
          - You must not rename members without making the corresponding
            name change in the compiler
          - Members you add will be treated like regular members unless
            you add the appropriate support in the compiler. */
   typedef __false_type    has_trivial_default_constructor;
   typedef __false_type    has_trivial_copy_constructor;
   typedef __false_type    has_trivial_assignment_operator;
   typedef __false_type    has_trivial_destructor;
   typedef __false_type    is_POD_type;
};


template <class _Tp>  struct _IsPtr { enum { _Ret = 0 }; };
template <class _Tp>  struct _IsPtrType { 
  static __false_type _Ret() { return __false_type();} 
};
template <class _Tp1, class _Tp2>  struct _BothPtrType { 
  static __false_type _Ret() { return __false_type();} 
};

template <class _Tp1, class _Tp2>
struct _IsSame { enum { _Ret = 0 }; };

// template <class _Tp1, class _Tp2>
// struct _IsSameType {   static __false_type _Ret() { return __false_type(); }  };

#  ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION
template <class _Tp>  struct _IsPtr<_Tp*> { enum { _Ret = 1 }; };
template <class _Tp>  struct _IsPtrType<_Tp*> { 
  static __true_type _Ret() { return __true_type();} 
};
template <class _Tp1, class _Tp2>  struct _BothPtrType<_Tp1*, _Tp2*> { 
  static __true_type _Ret() { return __true_type();} 
};
template <class _Tp>
struct _IsSame<_Tp, _Tp> { enum { _Ret = 1 }; };
#  endif

# endif /* _STLP_SIMULATE_PARTIAL_SPEC_FOR_TYPE_TRAITS */

// Provide some specializations.  This is harmless for compilers that
//  have built-in __types_traits support, and essential for compilers
//  that don't.
#ifndef _STLP_NO_BOOL
_STLP_TEMPLATE_NULL struct __type_traits<bool> : __type_traits_aux<1> {};
#endif /* _STLP_NO_BOOL */
_STLP_TEMPLATE_NULL struct __type_traits<char> : __type_traits_aux<1> {};
#ifndef _STLP_NO_SIGNED_BUILTINS
_STLP_TEMPLATE_NULL struct __type_traits<signed char> : __type_traits_aux<1> {};
# endif
_STLP_TEMPLATE_NULL struct __type_traits<unsigned char> : __type_traits_aux<1> {};
#if defined ( _STLP_HAS_WCHAR_T ) && ! defined (_STLP_WCHAR_T_IS_USHORT)
_STLP_TEMPLATE_NULL struct __type_traits<wchar_t> : __type_traits_aux<1> {};
#endif /* _STLP_HAS_WCHAR_T */

_STLP_TEMPLATE_NULL struct __type_traits<short> : __type_traits_aux<1> {};
_STLP_TEMPLATE_NULL struct __type_traits<unsigned short> : __type_traits_aux<1> {};
_STLP_TEMPLATE_NULL struct __type_traits<int> : __type_traits_aux<1> {};
_STLP_TEMPLATE_NULL struct __type_traits<unsigned int> : __type_traits_aux<1> {};
_STLP_TEMPLATE_NULL struct __type_traits<long> : __type_traits_aux<1> {};
_STLP_TEMPLATE_NULL struct __type_traits<unsigned long> : __type_traits_aux<1> {};

#ifdef _STLP_LONG_LONG
_STLP_TEMPLATE_NULL struct __type_traits<_STLP_LONG_LONG> : __type_traits_aux<1> {};
_STLP_TEMPLATE_NULL struct __type_traits<unsigned _STLP_LONG_LONG> : __type_traits_aux<1> {};
#endif /* _STLP_LONG_LONG */

_STLP_TEMPLATE_NULL struct __type_traits<float> : __type_traits_aux<1> {};
_STLP_TEMPLATE_NULL struct __type_traits<double> : __type_traits_aux<1> {};

# if !defined ( _STLP_NO_LONG_DOUBLE )
_STLP_TEMPLATE_NULL struct __type_traits<long double> : __type_traits_aux<1> {};
# endif

#ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION
template <class _Tp> struct __type_traits<_Tp*> : __type_traits_aux<1> {};
#endif

// The following could be written in terms of numeric_limits.  
// We're doing it separately to reduce the number of dependencies.

template <class _Tp> struct _Is_integer {
  typedef __false_type _Integral;
};

#ifndef _STLP_NO_BOOL

_STLP_TEMPLATE_NULL struct _Is_integer<bool> {
  typedef __true_type _Integral;
};

#endif /* _STLP_NO_BOOL */

_STLP_TEMPLATE_NULL struct _Is_integer<char> {
  typedef __true_type _Integral;
};

#ifndef _STLP_NO_SIGNED_BUILTINS

_STLP_TEMPLATE_NULL struct _Is_integer<signed char> {
  typedef __true_type _Integral;
};
#endif

_STLP_TEMPLATE_NULL struct _Is_integer<unsigned char> {
  typedef __true_type _Integral;
};

#if defined ( _STLP_HAS_WCHAR_T ) && ! defined (_STLP_WCHAR_T_IS_USHORT)

_STLP_TEMPLATE_NULL struct _Is_integer<wchar_t> {
  typedef __true_type _Integral;
};

#endif /* _STLP_HAS_WCHAR_T */

_STLP_TEMPLATE_NULL struct _Is_integer<short> {
  typedef __true_type _Integral;
};

_STLP_TEMPLATE_NULL struct _Is_integer<unsigned short> {
  typedef __true_type _Integral;
};

_STLP_TEMPLATE_NULL struct _Is_integer<int> {
  typedef __true_type _Integral;
};

_STLP_TEMPLATE_NULL struct _Is_integer<unsigned int> {
  typedef __true_type _Integral;
};

_STLP_TEMPLATE_NULL struct _Is_integer<long> {
  typedef __true_type _Integral;
};

_STLP_TEMPLATE_NULL struct _Is_integer<unsigned long> {
  typedef __true_type _Integral;
};

#ifdef _STLP_LONG_LONG

_STLP_TEMPLATE_NULL struct _Is_integer<_STLP_LONG_LONG> {
  typedef __true_type _Integral;
};

_STLP_TEMPLATE_NULL struct _Is_integer<unsigned _STLP_LONG_LONG> {
  typedef __true_type _Integral;
};

#endif /* _STLP_LONG_LONG */

template <class _Tp1, class _Tp2>
struct _OKToMemCpy {
  enum { _Same = _IsSame<_Tp1,_Tp2>::_Ret } ;
  typedef typename __type_traits<_Tp1>::has_trivial_assignment_operator _Tr1;
  typedef typename __type_traits<_Tp2>::has_trivial_assignment_operator _Tr2;
  typedef typename __bool2type< _Same >::_Ret _Tr3;
  typedef typename _Land3<_Tr1, _Tr2, _Tr3>::_Ret _Type;
  static _Type _Ret() { return _Type(); }
};

template <class _Tp1, class _Tp2>
inline _OKToMemCpy<_Tp1, _Tp2> _IsOKToMemCpy(_Tp1*, _Tp2*)  {
  return _OKToMemCpy<_Tp1, _Tp2>();
}

template <class _Tp> 
struct _IsPOD {
  typedef typename __type_traits<_Tp>::is_POD_type _Type;
  static _Type _Ret() { return _Type(); }
};

template <class _Tp> 
inline _IsPOD<_Tp>  _Is_POD (_Tp*) { return _IsPOD<_Tp>(); } 

#  ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION
#   if defined (__BORLANDC__) || defined (__SUNPRO_CC) || ( defined (__MWERKS__) && (__MWERKS__ <= 0x2303)) || ( defined (__sgi) && defined (_COMPILER_VERSION)) || defined (__DMC__)
#   define _IS_POD_ITER(_It, _Tp) __type_traits< typename iterator_traits< _Tp >::value_type >::is_POD_type()
#   else
#   define _IS_POD_ITER(_It, _Tp) typename __type_traits< typename iterator_traits< _Tp >::value_type >::is_POD_type()
#   endif
#  else
#   define _IS_POD_ITER(_It, _Tp) _Is_POD( _STLP_VALUE_TYPE( _It, _Tp ) )._Ret()
#  endif

# ifdef _STLP_DEFAULT_CONSTRUCTOR_BUG
// Those adaptors are here to fix common compiler bug regarding builtins:
// expressions like int k = int() should initialize k to 0
template <class _Tp>
inline _Tp __default_constructed_aux(_Tp*, const __false_type&) {
  return _Tp();
}
template <class _Tp>
inline _Tp __default_constructed_aux(_Tp*, const __true_type&) {
  return _Tp(0);
}

template <class _Tp>
inline _Tp __default_constructed(_Tp* __p) {
  typedef typename _Is_integer<_Tp>::_Integral _Is_Integral;
  return __default_constructed_aux(__p, _Is_Integral());
}

#  define _STLP_DEFAULT_CONSTRUCTED(_TTp) __default_constructed((_TTp*)0)
# else
#  define _STLP_DEFAULT_CONSTRUCTED(_TTp) _TTp()
# endif

_STLP_END_NAMESPACE

#endif /* __TYPE_TRAITS_H */

// Local Variables:
// mode:C++
// End:

