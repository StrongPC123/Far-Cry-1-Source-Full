/*
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

#ifndef _STLP_STLCONF_H
# define _STLP_STLCONF_H

# undef __AUTO_CONFIGURED

//==========================================================
// Getting proper values of autoconf flags
// if you ran 'configure', __AUTO_CONFIGURED is set to 1 and
// specific compiler features will be used.
// Otherwise, the <stlcomp.h> header will be included for per-version
// features recognition.
//==========================================================
# if defined (__AUTO_CONFIGURED)
// auto-configured section

# undef _STLP_NO_EXCEPTIONS
# undef _STLP_NO_NAMESPACES
# undef _STLP_NO_RELOPS_NAMESPACE
# undef _STLP_NO_NEW_NEW_HEADER 

# undef _STLP_NO_NEW_IOSTREAMS

// select threads strategy
# undef _PTHREADS
# undef _NOTHREADS

// select SGI-style alloc instead of allocator<T>
# undef _STLP_USE_SGI_ALLOCATORS

// select allocation method you like
# undef _STLP_USE_MALLOC
# undef _STLP_USE_NEWALLOC

// this one is not mandatory, just enabled
# undef _STLP_USE_DEFALLOC

// define _STLP_USE_ABBREVS if your linker has trouble with long 
// external symbols
# undef _STLP_USE_ABBREVS


// unsigned 32-bit integer type
#  define _STLP_UINT32_T unsigned
#  undef _STLP_NO_BOOL
#  undef _STLP_DONT_USE_BOOL_TYPEDEF
#  undef _STLP_YVALS_H
#  undef _STLP_LIMITED_DEFAULT_TEMPLATES
#  undef _STLP_DEFAULT_TYPE_PARAM
#  undef _STLP_NO_STATIC_TEMPLATE_DATA
#  undef _STLP_RAND48
#  undef _STLP_LOOP_INLINE_PROBLEMS

#  undef _STLP_HAS_NO_NAMESPACES

#  undef _STLP_NEED_TYPENAME
#  undef _STLP_NEED_EXPLICIT
#  undef _STLP_HAS_NO_EXCEPTIONS
#  undef _STLP_NO_EXCEPTION_SPEC
#  undef _STLP_WEAK_ATTRIBUTE
#  undef _STLP_BASE_MATCH_BUG
#  undef _STLP_NONTEMPL_BASE_MATCH_BUG
#  undef _STLP_NESTED_TYPE_PARAM_BUG
#  undef _STLP_NO_ARROW_OPERATOR
#  undef _STLP_UNINITIALIZABLE_PRIVATE
#  undef _STLP_BASE_TYPEDEF_BUG
#  undef _STLP_BASE_TYPEDEF_OUTSIDE_BUG
#  undef _STLP_CONST_CONSTRUCTOR_BUG

#  undef _STLP_NO_NEW_STYLE_CASTS
#  undef _STLP_NO_WCHAR_T
#  undef _STLP_WCHAR_T_IS_USHORT
#  undef _STLP_LONG_LONG
#  undef _STLP_NO_LONG_DOUBLE
#  undef _STLP_NEED_MUTABLE
#  undef _STLP_NO_PARTIAL_SPECIALIZATION_SYNTAX
#  undef _STLP_NO_BAD_ALLOC
#  undef _STLP_DEBUG_ALLOC
#  undef _STLP_NO_MEMBER_TEMPLATES
#  undef _STLP_NO_MEMBER_TEMPLATE_CLASSES
#  undef _STLP_NO_MEMBER_TEMPLATE_KEYWORD
#  undef _STLP_NO_FRIEND_TEMPLATES
#  undef _STLP_NO_QUALIFIED_FRIENDS
#  undef _STLP_NO_CLASS_PARTIAL_SPECIALIZATION
#  undef _STLP_NO_FUNCTION_TMPL_PARTIAL_ORDER
#  undef _STLP_AUTOMATIC_TYPE_TRAITS
#  undef _STLP_MEMBER_POINTER_PARAM_BUG
#  undef _STLP_NON_TYPE_TMPL_PARAM_BUG
#  undef _STLP_NO_DEFAULT_NON_TYPE_PARAM
#  undef _STLP_NO_METHOD_SPECIALIZATION
#  undef _STLP_STATIC_ARRAY_BUG
#  undef _STLP_STATIC_CONST_INIT_BUG
#  undef _STLP_TRIVIAL_CONSTRUCTOR_BUG
#  undef _STLP_TRIVIAL_DESTRUCTOR_BUG
#  undef _STLP_BROKEN_USING_DIRECTIVE
#  undef _STLP_NO_EXPLICIT_FUNCTION_TMPL_ARGS
#  undef _STLP_NO_EXCEPTION_HEADER
#  undef _STLP_DEFAULT_CONSTRUCTOR_BUG

#  undef _STLP_HAS_NO_NEW_IOSTREAMS
#  undef _STLP_HAS_NO_NEW_C_HEADERS 
#  undef _STLP_STATIC_CONST_INIT_BUG
// new ones
#  undef _STLP_THROW_RETURN_BUG
// unimp
#  undef _STLP_LINK_TIME_INSTANTIATION
#  undef _STLP_PARTIAL_SPEC_NEEDS_TEMPLATE_ARGS
// unimp
#  undef _STLP_NO_TEMPLATE_CONVERSIONS
# endif /* AUTO_CONFIGURED */

//==========================================================


#endif /* __STLCONF_H */

