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

#ifndef _STLP_PTHREAD_ALLOC_H
#define _STLP_PTHREAD_ALLOC_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa021
#  include <stl/_prolog.h>
# endif

#if defined  (_STLP_DEBUG) || defined (_STLP_ASSERTIONS)
# include <stl/debug/_debug.h>
#endif

#include <pthread_alloc>

#ifdef _STLP_USE_NAMESPACES

using STLPORT::_Pthread_alloc_template;
using STLPORT::pthread_alloc;

#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa021)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_PTHREAD_ALLOC_H */

// Local Variables:
// mode:C++
// End:

