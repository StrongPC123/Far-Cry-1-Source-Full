/*
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

#ifndef _STLP_HASHTABLE_H
#define _STLP_HASHTABLE_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa011
#  include <stl/_prolog.h>
# endif

#ifndef _STLP_ALGO_H
#include <algo.h>
#endif
#ifndef _STLP_ALLOC_H
#include <alloc.h>
#endif
#ifndef _STLP_VECTOR_H
#include <vector.h>
#endif
#ifndef _STLP_HASH_FUN_H
#include <stl/_hash_fun.h>
#endif
#ifndef _STLP_INTERNAL_HASHTABLE_H
#include <stl/_hashtable.h>
#endif

#ifdef _STLP_USE_NAMESPACES
# ifdef _STLP_BROKEN_USING_DIRECTIVE
using namespace STLPORT;
# else
using _STLP_STD::hash;
using STLPORT::hashtable;
# endif
#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa011)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif
#endif /* _STLP_HASHTABLE_H */

// Local Variables:
// mode:C++
// End:
