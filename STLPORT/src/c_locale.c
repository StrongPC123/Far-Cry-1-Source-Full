/*
 * Copyright (c) 1999
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

# include "stlport_prefix.h" 

# include <stdlib.h>

# ifdef _STLP_REAL_LOCALE_IMPLEMENTED
#  include <limits.h>
#  include "c_locale.h"
#  if defined (WIN32) || defined (_WIN32)
#   include "c_locale_win32/c_locale_win32.c"
#  elif defined (_STLP_USE_GLIBC) && ! defined (__CYGWIN__)
#   if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ > 2)  
#    include "c_locale_glibc/c_locale_glibc2.c"
#   else
#    include "c_locale_glibc/c_locale_glibc.c"
#   endif
#  elif defined __ISCPP__
#   include "c_locale_is/c_locale_is.cpp"
#  endif
# endif
