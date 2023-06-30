/*
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
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

#ifndef _STLP_ALLOC_H
#define _STLP_ALLOC_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa003
#  include <stl/_prolog.h>
# endif

#if defined  (_STLP_DEBUG) || defined (_STLP_ASSERTIONS) && !defined (_STLP_DEBUG_H)
# include <stl/debug/_debug.h>
#endif

# ifndef _STLP_CSTDDEF
#  include <cstddef>
# endif
# ifndef _STLP_CLIMITS
#  include <climits>
# endif
# ifndef _STLP_CSTDLIB
#  include <cstdlib>
# endif
# ifndef _STLP_CSTRING
#  include <cstring>
# endif
# ifndef _STLP_CASSERT
#  include <cassert>
# endif

#ifndef _STLP_INTERNAL_ALLOC_H
#include <stl/_alloc.h>
#endif

// Old SGI names
_STLP_BEGIN_NAMESPACE

typedef __sgi_alloc alloc;
typedef __malloc_alloc<0> malloc_alloc;
#ifdef _STLP_USE_NEWALLOC
typedef __new_alloc new_alloc;
#endif

#define simple_alloc __simple_alloc
typedef __single_client_alloc  single_client_alloc; 
typedef __multithreaded_alloc  multithreaded_alloc; 

_STLP_END_NAMESPACE

#ifdef _STLP_USE_NAMESPACES
# ifdef _STLP_BROKEN_USING_DIRECTIVE

using namespace STLPORT;

# else

# ifdef _STLP_USE_RAW_SGI_ALLOCATORS
using _STLP_STD::simple_alloc;
using _STLP_STD::alloc;
# endif

using _STLP_STD::malloc_alloc; 
# ifdef _STLP_DEBUG_ALLOC
using _STLP_STD::__debug_alloc;
# endif 
#ifdef _STLP_USE_NEWALLOC
using _STLP_STD::new_alloc;
#endif

using _STLP_STD::single_client_alloc; 
using _STLP_STD::multithreaded_alloc; 
using _STLP_STD::allocator;

# endif /* _STLP_BROKEN_USING_DIRECTIVE */
#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa003)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_ALLOC_H */

// Local Variables:
// mode:C++
// End:

