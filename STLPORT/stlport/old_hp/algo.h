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

#ifndef _STLP_ALGO_H
#define _STLP_ALGO_H

# ifndef _STLP_OUTERMOST_HEADER_ID
#  define _STLP_OUTERMOST_HEADER_ID 0xa001
#  include <stl/_prolog.h>
# endif

# ifndef _STLP_ALGOBASE_H
#  include <algobase.h>
# endif

# ifndef _STLP_TEMPBUF_H
#  include <tempbuf.h>
# endif

# ifndef _STLP_INTERNAL_HEAP_H
#  include <stl/_heap.h>
# endif

# ifndef _STLP_ITERATOR_H
#  include <iterator.h>
# endif

# ifndef _STLP_INTERNAL_ALGO_H
#  include <stl/_algo.h>
# endif

# ifndef _STLP_NUMERIC_H
#  include <stl/_numeric.h>
# endif

#ifdef _STLP_USE_NAMESPACES

# ifdef _STLP_BROKEN_USING_DIRECTIVE
using namespace STLPORT;
# else
// Names from <stl/_algo.h>
using STLPORT::for_each; 
using STLPORT::find; 
using STLPORT::find_if; 
using STLPORT::adjacent_find; 
using STLPORT::count; 
using STLPORT::count_if; 
using STLPORT::search; 
using STLPORT::search_n; 
using STLPORT::swap_ranges; 
using STLPORT::transform; 
using STLPORT::replace; 
using STLPORT::replace_if; 
using STLPORT::replace_copy; 
using STLPORT::replace_copy_if; 
using STLPORT::generate; 
using STLPORT::generate_n; 
// using STLPORT::remove; 
using STLPORT::remove_if; 
using STLPORT::remove_copy; 
using STLPORT::remove_copy_if; 
using STLPORT::unique; 
using STLPORT::unique_copy; 
using STLPORT::reverse; 
using STLPORT::reverse_copy; 
using STLPORT::rotate; 
using STLPORT::rotate_copy; 
using STLPORT::random_shuffle; 
using STLPORT::random_sample; 
using STLPORT::random_sample_n; 
using STLPORT::partition; 
using STLPORT::stable_partition; 
using STLPORT::sort; 
using STLPORT::stable_sort; 
using STLPORT::partial_sort; 
using STLPORT::partial_sort_copy; 
using STLPORT::nth_element; 
using STLPORT::lower_bound; 
using STLPORT::upper_bound; 
using STLPORT::equal_range; 
using STLPORT::binary_search; 
using STLPORT::merge; 
using STLPORT::inplace_merge; 
using STLPORT::includes; 
using STLPORT::set_union; 
using STLPORT::set_intersection; 
using STLPORT::set_difference; 
using STLPORT::set_symmetric_difference; 
using STLPORT::min_element; 
using STLPORT::max_element; 
using STLPORT::next_permutation; 
using STLPORT::prev_permutation; 
using STLPORT::find_first_of; 
using STLPORT::find_end; 
using STLPORT::is_sorted; 
using STLPORT::is_heap; 

// Names from stl_heap.h
using STLPORT::push_heap;
using STLPORT::pop_heap;
using STLPORT::make_heap;
using STLPORT::sort_heap;

// Names from <stl/_numeric.h>
using STLPORT::accumulate; 
using STLPORT::inner_product; 
using STLPORT::partial_sum; 
using STLPORT::adjacent_difference; 
using STLPORT::power; 
using STLPORT::iota; 

# endif /* _STLP_BROKEN_USING_DIRECTIVE */
#endif /* _STLP_USE_NAMESPACES */

# if (_STLP_OUTERMOST_HEADER_ID == 0xa001)
#  include <stl/_epilog.h>
#  undef _STLP_OUTERMOST_HEADER_ID
# endif

#endif /* _STLP_ALGO_H */

// Local Variables:
// mode:C++
// End:
