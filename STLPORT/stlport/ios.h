/*
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

#ifndef _STLP_IOS_H
# define _STLP_IOS_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0x1033
#  include <stl/_prolog.h>
# endif

# if defined (_STLP_OWN_IOSTREAMS)

#ifdef __BORLANDC__
#  include <ios.>
#else
#  include <ios>
#endif

# if defined (_STLP_USE_NAMESPACES)
# include <using/ios>
# endif

# elif !defined (_STLP_USE_NO_IOSTREAMS)

# include _STLP_NATIVE_OLD_STREAMS_HEADER(ios.h)
# if defined (_STLP_USE_NAMESPACES) && !defined (_STLP_BROKEN_USING_DIRECTIVE)
_STLP_BEGIN_NAMESPACE
using _STLP_OLD_IO_NAMESPACE::ios;
_STLP_END_NAMESPACE
# endif /* _STLP_USE_OWN_NAMESPACE */

# endif

# if (_STLP_OUTERMOST_HEADER_ID == 0x1033)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_IOS_H */

// Local Variables:
// mode:C++
// End:

