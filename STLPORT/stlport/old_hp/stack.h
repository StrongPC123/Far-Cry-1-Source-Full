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

#ifndef _STLP_STACK_H
#define _STLP_STACK_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa027
#  include <stl/_prolog.h>
# endif

#ifndef _STLP_VECTOR_H
#include <vector.h>
#endif

#ifndef _STLP_DEQUE_H
#include <deque.h>
#endif

#ifndef _STLP_HEAP_H
#include <heap.h>
#endif

#ifndef _STLP_INTERNAL_STACK_H
#include <stl/_stack.h>
#endif

#ifndef _STLP_INTERNAL_QUEUE_H
#include <stl/_queue.h>
#endif

#ifdef _STLP_USE_NAMESPACES
# ifdef _STLP_BROKEN_USING_DIRECTIVE
using namespace STLPORT;
# else
using _STLP_STD::stack;
using _STLP_STD::queue;
using _STLP_STD::priority_queue;
# endif
#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa027)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_STACK_H */

// Local Variables:
// mode:C++
// End:
