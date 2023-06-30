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

#ifndef FSTREAM_IMPL_H
# define FSTREAM_IMPL_H

#include <stl/_stdio_file.h>

# ifdef _STLP_HAS_NO_NAMESPACES
#  define __SGI_BEGIN_NAMESPACE
#  define __SGI_END_NAMESPACE
#  define _SgI
# else
#  define __SGI_BEGIN_NAMESPACE namespace _SgI {
#  define __SGI_END_NAMESPACE }
# endif

__SGI_BEGIN_NAMESPACE

# ifndef _STLP_HAS_NO_NAMESPACES
using _STLP_STD::streamoff;
using _STLP_STD::ios_base;
using _STLP_STD::streamsize;
using _STLP_STD::streamoff;
using _STLP_STD::char_traits;
# ifndef _STLP_USE_UNIX_IO
using _STLP_VENDOR_CSTD::FILE;
using _STLP_VENDOR_CSTD::ftell;
# endif

using _STLP_STD::_FILE_fd;
# endif

extern bool __is_regular_file(_STLP_fd fd);
extern streamoff __file_size(_STLP_fd fd);

__SGI_END_NAMESPACE

#endif /* FSTREAM_IMPL_H */
