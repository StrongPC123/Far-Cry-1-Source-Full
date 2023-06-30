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

#ifndef _STLP_FUNCTION_H
#define _STLP_FUNCTION_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa008
#  include <stl/_prolog.h>
# endif

#ifndef _STLP_CSTDDEF
# include <cstddef>
#endif

#ifndef _STLP_INTERNAL_ALGOBASE_H
#include <stl/_algobase.h>
#endif

#ifndef _STLP_INTERNAL_FUNCTION_H
#include <stl/_function.h>
#endif

#ifdef _STLP_USE_NAMESPACES

# ifdef _STLP_BROKEN_USING_DIRECTIVE
using namespace STLPORT;
#ifndef _STLP_NO_RELOPS_NAMESPACE
using namespace STLPORT_RELOPS;
#endif /* _STLP_USE_NAMESPACE_FOR_RELOPS */

# else /* _STLP_BROKEN_USING_DIRECTIVE */

// Names from stl_function.h
using _STLP_STD::unary_function; 
using _STLP_STD::binary_function; 
using _STLP_STD::plus; 
using _STLP_STD::minus; 
using _STLP_STD::multiplies; 
using _STLP_STD::divides; 
using _STLP_STD::identity_element; 
using _STLP_STD::modulus; 
using _STLP_STD::negate; 
using _STLP_STD::equal_to; 
using _STLP_STD::not_equal_to; 
using _STLP_STD::greater; 
using _STLP_STD::less; 
using _STLP_STD::greater_equal; 
using _STLP_STD::less_equal; 
using _STLP_STD::logical_and; 
using _STLP_STD::logical_or; 
using _STLP_STD::logical_not; 
using _STLP_STD::unary_negate; 
using _STLP_STD::binary_negate; 
using _STLP_STD::not1; 
using _STLP_STD::not2; 
using _STLP_STD::binder1st; 
using _STLP_STD::binder2nd; 
using _STLP_STD::bind1st; 
using _STLP_STD::bind2nd; 
using _STLP_STD::unary_compose; 
using _STLP_STD::binary_compose; 
using _STLP_STD::compose1; 
using _STLP_STD::compose2; 
using _STLP_STD::pointer_to_unary_function; 
using _STLP_STD::pointer_to_binary_function; 
using _STLP_STD::ptr_fun; 
using _STLP_STD::identity; 
using _STLP_STD::select1st; 
using _STLP_STD::select2nd; 
using _STLP_STD::project1st; 
using _STLP_STD::project2nd; 
using _STLP_STD::constant_void_fun; 
using _STLP_STD::constant_unary_fun; 
using _STLP_STD::constant_binary_fun; 
using _STLP_STD::constant0; 
using _STLP_STD::constant1; 
using _STLP_STD::constant2; 
using _STLP_STD::subtractive_rng; 
using _STLP_STD::mem_fun_t; 
using _STLP_STD::const_mem_fun_t; 
using _STLP_STD::mem_fun_ref_t; 
using _STLP_STD::const_mem_fun_ref_t; 
using _STLP_STD::mem_fun1_t; 
using _STLP_STD::const_mem_fun1_t; 
using _STLP_STD::mem_fun1_ref_t; 
using _STLP_STD::const_mem_fun1_ref_t; 
using _STLP_STD::mem_fun; 
using _STLP_STD::mem_fun_ref; 
using _STLP_STD::mem_fun1; 
using _STLP_STD::mem_fun1_ref; 
# endif
#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa008)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_FUNCTION_H */

// Local Variables:
// mode:C++
// End:
