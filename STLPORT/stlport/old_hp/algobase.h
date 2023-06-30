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

#ifndef _STLP_ALGOBASE_H
#define _STLP_ALGOBASE_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa002
#  include <stl/_prolog.h>
# endif

#ifndef _STLP_PAIR_H
#include <pair.h>
#endif

// memmove
#ifndef _STLP_CSTRING
# include <cstring>
#endif

// CHAR_MAX
#ifndef _STLP_CLIMITS
# include <climits>
#endif

#ifndef _STLP_ITERATOR_H
#include <iterator.h>
#endif

#ifndef _STLP_INTERNAL_ALGOBASE_H
#include <stl/_algobase.h>
#endif

#ifndef _STLP_INTERNAL_UNINITIALIZED_H
#include <stl/_uninitialized.h>
#endif

#ifdef _STLP_USE_NAMESPACES

# ifdef _STLP_BROKEN_USING_DIRECTIVE
using namespace STLPORT;
# else
// Names from stl_algobase.h
using STLPORT::iter_swap; 
using STLPORT::swap; 
using STLPORT::min; 
using STLPORT::max; 
using STLPORT::copy; 
using STLPORT::copy_backward; 
using STLPORT::copy_n; 
using STLPORT::fill; 
using STLPORT::fill_n; 
using STLPORT::mismatch; 
using STLPORT::equal; 
using STLPORT::lexicographical_compare; 
using STLPORT::lexicographical_compare_3way; 

// Names from stl_uninitialized.h
using STLPORT::uninitialized_copy;
using STLPORT::uninitialized_copy_n;
using STLPORT::uninitialized_fill;
using STLPORT::uninitialized_fill_n;
# endif /* _STLP_BROKEN_USING_DIRECTIVE */
#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa002)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_ALGOBASE_H */

// Local Variables:
// mode:C++
// End:
