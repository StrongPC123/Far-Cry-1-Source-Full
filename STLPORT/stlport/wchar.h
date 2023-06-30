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

# if !defined (_STLP_OUTERMOST_HEADER_ID)
#  define _STLP_OUTERMOST_HEADER_ID 0x278
#  include <stl/_prolog.h>
# elif (_STLP_OUTERMOST_HEADER_ID == 0x278) && ! defined (_STLP_DONT_POP_0x278)
#  define _STLP_DONT_POP_0x278
# endif

# if ! defined (_STLP_WINCE) && ! defined (_STLP_NO_WCHAR_T)

# if defined ( __BORLANDC__ ) && (__BORLANDC__) >= 0x530
// #  include <cstring>
# include _STLP_NATIVE_CPP_C_HEADER(_str.h)
using _STLP_VENDOR_CSTD::strlen;
using _STLP_VENDOR_CSTD::strspn;
# endif

# if defined (__GNUC__) && defined (__APPLE__)
#  include _STLP_NATIVE_C_HEADER(stddef.h)
# else
#  include _STLP_NATIVE_C_HEADER(wchar.h)
# endif

# endif /* WINCE */

# if (_STLP_OUTERMOST_HEADER_ID == 0x278)
#  if ! defined (_STLP_DONT_POP_0x278)
#   include <stl/_epilog.h>
#   undef  _STLP_OUTERMOST_HEADER_ID
#   endif
#   undef  _STLP_DONT_POP_0x278
# endif

// Local Variables:
// mode:C++
// End:
