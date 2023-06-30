/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
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

#ifndef _STLP_VECTOR_H
#define _STLP_VECTOR_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa031
#  include <stl/_prolog.h>
# endif

#ifndef _STLP_ALGOBASE_H
# include <algobase.h>
#endif

#ifndef _STLP_ALLOC_H
# include <alloc.h>
#endif

# ifndef _STLP_RANGE_ERRORS_H
#  include <stl/_range_errors.h>
# endif

#ifndef _STLP_INTERNAL_VECTOR_H
# include <stl/_vector.h>
#endif

#ifdef _STLP_USE_NAMESPACES
# ifdef _STLP_BROKEN_USING_DIRECTIVE
using namespace STLPORT;
# else
using STLPORT::vector;
using STLPORT::__vector__;
# endif
#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa031)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_VECTOR_H */

// Local Variables:
// mode:C++
// End:
