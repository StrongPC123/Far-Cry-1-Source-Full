 /* stl_icc.h
  * *
  * * A list of Intel compiler for Linux portion of STLport settings.
  * * This file is being included by stlcomp.h
  * */
# ifndef _STLP_ICC_H
# define _STLP_ICC_H

# define _STLP_UINT32_T unsigned long
# define _STLP_LONG_LONG long long
# define _STLP_TYPENAME_ON_RETURN_TYPE typename

// Edit relative path below (or put full path) to get native
// compiler headers included. Default is "../include".
// C headers may reside in different directory, so separate macro is provided.
# if (__INTEL_COMPILER < 800)
# define _STLP_NATIVE_INCLUDE_PATH ../include
# else
// The header of files have moved to a new location on Linux Intel C++ compiler 
// starting with version 8, which has GCC 3.2 compatability.
# define _STLP_NATIVE_INCLUDE_PATH ../include/c++
# define _STLP_NATIVE_OLD_STREAMS_INCLUDE_PATH _STLP_NATIVE_INCLUDE_PATH/backward
#   ifndef __GNUC__ 
//  If GCC compatability is diabled (-no-gcc is specified) STD needs to be redefined.
#   define _STLP_REDEFINE_STD 1
#   endif
# endif
# define _STLP_NATIVE_C_INCLUDE_PATH ../include
# define _STLP_NATIVE_CPP_C_INCLUDE_PATH ../include

// This macro constructs header path from directory and name.
# define _STLP_MAKE_HEADER(path, header) <path/header>
// This macro constructs native include header path from include path and name.
# define _STLP_NATIVE_HEADER(header) _STLP_MAKE_HEADER(_STLP_NATIVE_INCLUDE_PATH,header)

# define _STLP_NATIVE_CPP_C_HEADER(header) _STLP_MAKE_HEADER(_STLP_NATIVE_INCLUDE_PATH,header)

// Same for C headers
# define _STLP_NATIVE_C_HEADER(header) _STLP_MAKE_HEADER(_STLP_NATIVE_C_INCLUDE_PATH,header)

# undef _STLP_WINCE

# ifndef __GNUC__ 
# define __GNUC__ 1
# endif

# define _STLP_NO_NATIVE_WIDE_FUNCTIONS 1
# endif

