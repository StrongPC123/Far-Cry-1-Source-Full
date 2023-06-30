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

#ifndef _STLP_ITERATOR_H
#define _STLP_ITERATOR_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa013
#  include <stl/_prolog.h>
# endif

#if defined  (_STLP_DEBUG) || defined (_STLP_ASSERTIONS) && !defined (_STLP_DEBUG_H)
# include <stl/debug/_debug.h>
#endif

#if defined (_STLP_USE_NEW_STYLE_HEADERS)
# include <cstddef>
#else
# include <stddef.h>
#endif

# ifndef _STLP_NEW
#  include <new>
# endif

# ifndef __TYPE_TRAITS_H
#  include <stl/type_traits.h>
# endif

#ifndef _STLP_FUNCTION_H
#include <function.h>
#endif

# ifndef _STLP_IOSFWD
#  include <iosfwd>
# endif

# ifndef _STLP_INTERNAL_ITERATOR_BASE_H
#  include <stl/_iterator_base.h>
# endif

# ifndef _STLP_INTERNAL_ITERATOR_H
#  include <stl/_iterator.h>
# endif

#ifndef _STLP_INTERNAL_CONSTRUCT_H
#include <stl/_construct.h>
#endif

#ifndef _STLP_INTERNAL_RAW_STORAGE_ITERATOR_H
#include <stl/_raw_storage_iter.h>
#endif

# ifndef _STLP_INTERNAL_STREAM_ITERATOR_H
#  include <stl/_stream_iterator.h>
# endif

#ifdef _STLP_USE_NAMESPACES

// Names from stl_iterator.h

# ifdef _STLP_BROKEN_USING_DIRECTIVE
using namespace STLPORT;
# else

using STLPORT::input_iterator_tag;
using STLPORT::output_iterator_tag;
using STLPORT::forward_iterator_tag;
using STLPORT::bidirectional_iterator_tag;
using STLPORT::random_access_iterator_tag;

using STLPORT::input_iterator;
using STLPORT::output_iterator;
using STLPORT::forward_iterator;
using STLPORT::bidirectional_iterator;
using STLPORT::random_access_iterator;

#ifdef _STLP_CLASS_PARTIAL_SPECIALIZATION
using STLPORT::iterator_traits;
#endif

# ifdef _STLP_USE_OLD_HP_ITERATOR_QUERIES
using STLPORT::iterator_category;
using STLPORT::distance_type;
using STLPORT::value_type;
# endif

using STLPORT::distance; 
using STLPORT::advance; 

using STLPORT::insert_iterator;
using STLPORT::front_insert_iterator;
using STLPORT::back_insert_iterator;
using STLPORT::inserter;
using STLPORT::front_inserter;
using STLPORT::back_inserter;

using STLPORT::reverse_iterator;
# if ! defined ( _STLP_CLASS_PARTIAL_SPECIALIZATION ) || defined (_STLP_USE_OLD_HP_ITERATOR_QUERIES)
using STLPORT::reverse_bidirectional_iterator;
# endif
using STLPORT::istream_iterator;
using STLPORT::ostream_iterator;

// Names from stl_construct.h
using STLPORT::construct;
using STLPORT::destroy;

// Names from stl_raw_storage_iter.h
using STLPORT::raw_storage_iterator;
# endif

#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa013)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_ITERATOR_H */

// Local Variables:
// mode:C++
// End:

