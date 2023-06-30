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

#ifndef _STLP_TEMPBUF_H
#define _STLP_TEMPBUF_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa028
#  include <stl/_prolog.h>
# endif

# ifndef _STLP_CLIMITS
#  include <climits>
# endif

#ifndef _STLP_CSTDDEF
# include <cstddef>
#endif

#ifndef _STLP_CSTDLIB
# include <cstdlib>
#endif

#if !defined (_STLP_NEW)
# include <new>
#endif

#ifndef __TYPE_TRAITS_H
#include <stl/type_traits.h>
#endif

#ifndef _STLP_PAIR_H
#include <pair.h>
#endif

#ifndef _STLP_INTERNAL_TEMPBUF_H
#include <stl/_tempbuf.h>
#endif

#ifdef _STLP_USE_NAMESPACES

# ifdef _STLP_BROKEN_USING_DIRECTIVE
using namespace STLPORT;
# else
using _STLP_STD::get_temporary_buffer;
using _STLP_STD::return_temporary_buffer;
using _STLP_STD::temporary_buffer;
# endif
#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa028)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_TEMPBUF_H */

// Local Variables:
// mode:C++
// End:
