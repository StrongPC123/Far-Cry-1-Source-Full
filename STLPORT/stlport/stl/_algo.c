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

#ifndef _STLP_ALGO_C
# define _STLP_ALGO_C

# if !defined (_STLP_INTERNAL_ALGO_H)
#  include <stl/_algo.h>
# endif

_STLP_BEGIN_NAMESPACE

template <class _BidirectionalIter, class _Distance, class _Compare>
void __merge_without_buffer(_BidirectionalIter __first,
                            _BidirectionalIter __middle,
                            _BidirectionalIter __last,
                            _Distance __len1, _Distance __len2,
                            _Compare __comp);


template <class _BidirectionalIter1, class _BidirectionalIter2,
          class _BidirectionalIter3, class _Compare>
_BidirectionalIter3 __merge_backward(_BidirectionalIter1 __first1,
                                     _BidirectionalIter1 __last1,
                                     _BidirectionalIter2 __first2,
                                     _BidirectionalIter2 __last2,
                                     _BidirectionalIter3 __result,
                                     _Compare __comp);

template <class _Tp>
# if !(defined (__SUNPRO_CC) && (__SUNPRO_CC < 0x420 ))
inline 
# endif
const _Tp& __median(const _Tp& __a, const _Tp& __b, const _Tp& __c) {
  if (__a < __b)
    if (__b < __c)
      return __b;
    else if (__a < __c)
      return __c;
    else
      return __a;
  else if (__a < __c)
    return __a;
  else if (__b < __c)
    return __c;
  else
    return __b;
}

template <class _Tp, class _Compare>
# if !(defined (__SUNPRO_CC) && (__SUNPRO_CC < 0x420 ))
inline 
# endif
const _Tp&
__median(const _Tp& __a, const _Tp& __b, const _Tp& __c, _Compare __comp) {
  if (__comp(__a, __b))
    if (__comp(__b, __c))
      return __b;
    else if (__comp(__a, __c))
      return __c;
    else
      return __a;
  else if (__comp(__a, __c))
    return __a;
  else if (__comp(__b, __c))
    return __c;
  else
    return __b;
}

template <class _ForwardIter1, class _ForwardIter2>
_ForwardIter1 search(_ForwardIter1 __first1, _ForwardIter1 __last1,
                     _ForwardIter2 __first2, _ForwardIter2 __last2) 
{
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  // Test for empty ranges
  if (__first1 == __last1 || __first2 == __last2)
    return __first1;

  // Test for a pattern of length 1.
  _ForwardIter2 __tmp(__first2);
  ++__tmp;
  if (__tmp == __last2)
    return find(__first1, __last1, *__first2);

  // General case.
  _ForwardIter2 __p1 = __first2; 
  ++__p1;

  _ForwardIter1 __current = __first1;

  while (__first1 != __last1) {
    __first1 = find(__first1, __last1, *__first2);
    if (__first1 == __last1)
      return __last1;

    _ForwardIter2 __p = __p1;
    __current = __first1; 
    if (++__current == __last1)
      return __last1;

    while (*__current == *__p) {
      if (++__p == __last2)
        return __first1;
      if (++__current == __last1)
        return __last1;
    }

    ++__first1;
  }
  return __first1;
}

// search_n.  Search for __count consecutive copies of __val.

template <class _ForwardIter, class _Integer, class _Tp>
_ForwardIter search_n(_ForwardIter __first, _ForwardIter __last,
                      _Integer __count, const _Tp& __val) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__count <= 0)
    return __first;
  else {
    __first = find(__first, __last, __val);
    while (__first != __last) {
      _Integer __n = __count - 1;
      _ForwardIter __i = __first;
      ++__i;
      while (__i != __last && __n != 0 && *__i == __val) {
        ++__i;
        --__n;
      }
      if (__n == 0)
        return __first;
      else
        __first = find(__i, __last, __val);
    }
    return __last;
  }
}

template <class _ForwardIter, class _Integer, class _Tp, class _BinaryPred>
_ForwardIter search_n(_ForwardIter __first, _ForwardIter __last,
                      _Integer __count, const _Tp& __val,
                      _BinaryPred __binary_pred) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__count <= 0)
    return __first;
  else {
    while (__first != __last) {
      if (__binary_pred(*__first, __val))
        break;
      ++__first;
    }
    while (__first != __last) {
      _Integer __n = __count - 1;
      _ForwardIter __i = __first;
      ++__i;
      while (__i != __last && __n != 0 && __binary_pred(*__i, __val)) {
        ++__i;
        --__n;
      }
      if (__n == 0)
        return __first;
      else {
        while (__i != __last) {
          if (__binary_pred(*__i, __val))
            break;
          ++__i;
        }
        __first = __i;
      }
    }
    return __last;
  }
} 

template <class _ForwardIter1, class _ForwardIter2>
_ForwardIter1 
find_end(_ForwardIter1 __first1, _ForwardIter1 __last1, 
         _ForwardIter2 __first2, _ForwardIter2 __last2)
{
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  return __find_end(__first1, __last1, __first2, __last2,
# if defined (_STLP_CLASS_PARTIAL_SPECIALIZATION)
                    _STLP_ITERATOR_CATEGORY(__first1, _ForwardIter1),
                    _STLP_ITERATOR_CATEGORY(__first2, _ForwardIter2),
# else
		    forward_iterator_tag(),
                    forward_iterator_tag(),
# endif
                    __equal_to(_STLP_VALUE_TYPE(__first1, _ForwardIter1))
    );
}

// unique and unique_copy
template <class _InputIterator, class _OutputIterator, class _BinaryPredicate,
					    class _Tp>
_STLP_INLINE_LOOP _OutputIterator 
__unique_copy(_InputIterator __first, _InputIterator __last,
              _OutputIterator __result,
              _BinaryPredicate __binary_pred, _Tp*) {
  _Tp __val = *__first;
  *__result = __val;
  while (++__first != __last)
    if (!__binary_pred(__val, *__first)) {
      __val = *__first;
      *++__result = __val;
    }
  return ++__result;
}

template <class _InputIter, class _OutputIter, class _BinaryPredicate>
inline _OutputIter 
__unique_copy(_InputIter __first, _InputIter __last,_OutputIter __result,
              _BinaryPredicate __binary_pred, const output_iterator_tag &) {
  return __unique_copy(__first, __last, __result, __binary_pred, _STLP_VALUE_TYPE(__first, _InputIter));
}

template <class _InputIter, class _ForwardIter, class _BinaryPredicate>
_STLP_INLINE_LOOP _ForwardIter 
__unique_copy(_InputIter __first, _InputIter __last, _ForwardIter __result, 
              _BinaryPredicate __binary_pred, const forward_iterator_tag &) {
  *__result = *__first;
  while (++__first != __last)
    if (!__binary_pred(*__result, *__first)) *++__result = *__first;
  return ++__result;
}

# if defined (_STLP_NONTEMPL_BASE_MATCH_BUG)
template <class _InputIterator, class _BidirectionalIterator, class _BinaryPredicate>
inline _BidirectionalIterator 
__unique_copy(_InputIterator __first, _InputIterator __last,
              _BidirectionalIterator __result, _BinaryPredicate __binary_pred,
              const bidirectional_iterator_tag &) {
  return __unique_copy(__first, __last, __result, __binary_pred, forward_iterator_tag());
}

template <class _InputIterator, class _RandomAccessIterator, class _BinaryPredicate>
inline _RandomAccessIterator 
__unique_copy(_InputIterator __first, _InputIterator __last,
              _RandomAccessIterator __result, _BinaryPredicate __binary_pred,
              const random_access_iterator_tag &) {
  return __unique_copy(__first, __last, __result, __binary_pred, forward_iterator_tag());
}
# endif /* _STLP_NONTEMPL_BASE_MATCH_BUG */


template <class _InputIter, class _OutputIter>
_OutputIter 
unique_copy(_InputIter __first, _InputIter __last, _OutputIter __result) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last) return __result;
  return __unique_copy(__first, __last, __result, __equal_to(_STLP_VALUE_TYPE(__first, _InputIter)),
                       _STLP_ITERATOR_CATEGORY(__result, _OutputIter));
}

template <class _InputIter, class _OutputIter, class _BinaryPredicate>
_OutputIter 
unique_copy(_InputIter __first, _InputIter __last,_OutputIter __result,
            _BinaryPredicate __binary_pred) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last) return __result;
  return __unique_copy(__first, __last, __result, __binary_pred,
                       _STLP_ITERATOR_CATEGORY(__result, _OutputIter));
}

// rotate and rotate_copy, and their auxiliary functions

template <class _ForwardIter, class _Distance>
_ForwardIter __rotate(_ForwardIter __first,
                      _ForwardIter __middle,
                      _ForwardIter __last,
                      _Distance*,
                      const forward_iterator_tag &) {
  if (__first == __middle)
    return __last;
  if (__last  == __middle)
    return __first;

  _ForwardIter __first2 = __middle;
  do {
    swap(*__first++, *__first2++);
    if (__first == __middle)
      __middle = __first2;
  } while (__first2 != __last);

  _ForwardIter __new_middle = __first;

  __first2 = __middle;

  while (__first2 != __last) {
    swap (*__first++, *__first2++);
    if (__first == __middle)
      __middle = __first2;
    else if (__first2 == __last)
      __first2 = __middle;
  }

  return __new_middle;
}

template <class _BidirectionalIter, class _Distance>
_BidirectionalIter __rotate(_BidirectionalIter __first,
                            _BidirectionalIter __middle,
                            _BidirectionalIter __last,
                            _Distance*,
                            const bidirectional_iterator_tag &) {
  if (__first == __middle)
    return __last;
  if (__last  == __middle)
    return __first;

  __reverse(__first,  __middle, bidirectional_iterator_tag());
  __reverse(__middle, __last,   bidirectional_iterator_tag());

  while (__first != __middle && __middle != __last)
    swap (*__first++, *--__last);

  if (__first == __middle) {
    __reverse(__middle, __last,   bidirectional_iterator_tag());
    return __last;
  }
  else {
    __reverse(__first,  __middle, bidirectional_iterator_tag());
    return __first;
  }
}

template <class _RandomAccessIter, class _Distance, class _Tp>
_RandomAccessIter __rotate(_RandomAccessIter __first,
                           _RandomAccessIter __middle,
                           _RandomAccessIter __last,
                           _Distance *, _Tp *) {

  _Distance __n = __last   - __first;
  _Distance __k = __middle - __first;
  _Distance __l = __n - __k;
  _RandomAccessIter __result = __first + (__last - __middle);

  if (__k==0)  /* __first == middle */
    return __last;

  if (__k == __l) {
    swap_ranges(__first, __middle, __middle);
    return __result;
  }

  _Distance __d = __gcd(__n, __k);

  for (_Distance __i = 0; __i < __d; __i++) {
    _Tp __tmp = *__first;
    _RandomAccessIter __p = __first;

    if (__k < __l) {
      for (_Distance __j = 0; __j < __l/__d; __j++) {
	if (__p > __first + __l) {
          *__p = *(__p - __l);
          __p -= __l;
        }

        *__p = *(__p + __k);
        __p += __k;
      }
    }

    else {
      for (_Distance __j = 0; __j < __k/__d - 1; __j ++) {
        if (__p < __last - __k) {
          *__p = *(__p + __k);
          __p += __k;
        }

        *__p = * (__p - __l);
        __p -= __l;
      }
    }

    *__p = __tmp;
    ++__first;
  }

  return __result;
}

template <class _RandomAccessIter, class _Distance>
inline _RandomAccessIter 
__rotate(_RandomAccessIter __first, _RandomAccessIter __middle, _RandomAccessIter __last,
         _Distance * __dis, const random_access_iterator_tag &) {
  return __rotate(__first, __middle, __last,
                  __dis, _STLP_VALUE_TYPE(__first, _RandomAccessIter));
}

template <class _ForwardIter>
_ForwardIter 
rotate(_ForwardIter __first, _ForwardIter __middle, _ForwardIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __middle))
  _STLP_DEBUG_CHECK(__check_range(__middle, __last))
  return __rotate(__first, __middle, __last,
                  _STLP_DISTANCE_TYPE(__first, _ForwardIter),
                  _STLP_ITERATOR_CATEGORY(__first, _ForwardIter));
}

// Return a random number in the range [0, __n).  This function encapsulates
// whether we're using rand (part of the standard C library) or lrand48
// (not standard, but a much better choice whenever it's available).

template <class _Distance>
inline _Distance __random_number(_Distance __n) {
#ifdef _STLP_NO_DRAND48
  return rand() % __n;
#else
  return lrand48() % __n;
#endif
}

template <class _RandomAccessIter>
void random_shuffle(_RandomAccessIter __first,
		    _RandomAccessIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last) return;
  for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
    iter_swap(__i, __first + __random_number((__i - __first) + 1));
}

template <class _RandomAccessIter, class _RandomNumberGenerator>
void random_shuffle(_RandomAccessIter __first, _RandomAccessIter __last,
                    _RandomNumberGenerator& __rand) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last) return;
  for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
    iter_swap(__i, __first + __rand((__i - __first) + 1));
}

# ifndef _STLP_NO_EXTENSIONS

// random_sample and random_sample_n (extensions, not part of the standard).

template <class _ForwardIter, class _OutputIter, class _Distance>
_OutputIter random_sample_n(_ForwardIter __first, _ForwardIter __last,
                            _OutputIter __out, const _Distance __n)
{
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  _Distance __remaining = distance(__first, __last);
  _Distance __m = (min) (__n, __remaining);

  while (__m > 0) {
    if (__random_number(__remaining) < __m) {
      *__out = *__first;
      ++__out;
      --__m;
    }

    --__remaining;
    ++__first;
  }
  return __out;
}


template <class _ForwardIter, class _OutputIter, class _Distance,
          class _RandomNumberGenerator>
_OutputIter random_sample_n(_ForwardIter __first, _ForwardIter __last,
                            _OutputIter __out, const _Distance __n,
                            _RandomNumberGenerator& __rand)
{
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  _Distance __remaining = distance(__first, __last);
  _Distance __m = (min) (__n, __remaining);

  while (__m > 0) {
    if (__rand(__remaining) < __m) {
      *__out = *__first;
      ++__out;
      --__m;
    }

    --__remaining;
    ++__first;
  }
  return __out;
}

template <class _InputIter, class _RandomAccessIter, class _Distance>
_RandomAccessIter __random_sample(_InputIter __first, _InputIter __last,
                                  _RandomAccessIter __out,
                                  const _Distance __n)
{
  _Distance __m = 0;
  _Distance __t = __n;
  for ( ; __first != __last && __m < __n; ++__m, ++__first) 
    __out[__m] = *__first;

  while (__first != __last) {
    ++__t;
    _Distance __M = __random_number(__t);
    if (__M < __n)
      __out[__M] = *__first;
    ++__first;
  }

  return __out + __m;
}

template <class _InputIter, class _RandomAccessIter,
          class _RandomNumberGenerator, class _Distance>
_RandomAccessIter __random_sample(_InputIter __first, _InputIter __last,
                                  _RandomAccessIter __out,
                                  _RandomNumberGenerator& __rand,
                                  const _Distance __n)
{
  _Distance __m = 0;
  _Distance __t = __n;
  for ( ; __first != __last && __m < __n; ++__m, ++__first)
    __out[__m] = *__first;

  while (__first != __last) {
    ++__t;
    _Distance __M = __rand(__t);
    if (__M < __n)
      __out[__M] = *__first;
    ++__first;
  }

  return __out + __m;
}

template <class _InputIter, class _RandomAccessIter>
_RandomAccessIter
random_sample(_InputIter __first, _InputIter __last,
              _RandomAccessIter __out_first, _RandomAccessIter __out_last) 
{
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  _STLP_DEBUG_CHECK(__check_range(__out_first, __out_last))
  return __random_sample(__first, __last,
                         __out_first, __out_last - __out_first);
}

template <class _InputIter, class _RandomAccessIter, class _RandomNumberGenerator>
_RandomAccessIter
random_sample(_InputIter __first, _InputIter __last,
              _RandomAccessIter __out_first, _RandomAccessIter __out_last,
              _RandomNumberGenerator& __rand) 
{
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  _STLP_DEBUG_CHECK(__check_range(__out_first, __out_last))
  return __random_sample(__first, __last,
                         __out_first, __rand,
                         __out_last - __out_first);
}

# endif /* _STLP_NO_EXTENSIONS */

// partition, stable_partition, and their auxiliary functions

template <class _ForwardIter, class _Predicate>
_STLP_INLINE_LOOP _ForwardIter __partition(_ForwardIter __first,
                                           _ForwardIter __last,
                                           _Predicate   __pred,
                                           const forward_iterator_tag &) {
  if (__first == __last) return __first;

  while (__pred(*__first))
    if (++__first == __last) return __first;

  _ForwardIter __next = __first;

  while (++__next != __last)
    if (__pred(*__next)) {
      swap(*__first, *__next);
      ++__first;
    }
  return __first;
}

template <class _BidirectionalIter, class _Predicate>
_STLP_INLINE_LOOP _BidirectionalIter __partition(_BidirectionalIter __first,
                                                 _BidirectionalIter __last,
                                                 _Predicate __pred,
                                                 const bidirectional_iterator_tag &) {
  while (true) {
    while (true)
      if (__first == __last)
        return __first;
      else if (__pred(*__first))
        ++__first;
      else
        break;
    --__last;
    while (true)
      if (__first == __last)
        return __first;
      else if (!__pred(*__last))
        --__last;
      else
        break;
    iter_swap(__first, __last);
    ++__first;
  }
}

# if defined (_STLP_NONTEMPL_BASE_MATCH_BUG)
template <class _BidirectionalIter, class _Predicate>
inline
_BidirectionalIter __partition(_BidirectionalIter __first,
                               _BidirectionalIter __last,
			       _Predicate __pred,
			       const random_access_iterator_tag &) {
  return __partition(__first, __last, __pred, bidirectional_iterator_tag());
}
# endif

template <class _ForwardIter, class _Predicate>
_ForwardIter partition(_ForwardIter __first, _ForwardIter __last, _Predicate   __pred) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  return __partition(__first, __last, __pred, _STLP_ITERATOR_CATEGORY(__first, _ForwardIter));
}


template <class _ForwardIter, class _Predicate, class _Distance>
_ForwardIter __inplace_stable_partition(_ForwardIter __first,
                                        _ForwardIter __last,
                                        _Predicate __pred, _Distance __len) {
  if (__len == 1)
    return __pred(*__first) ? __last : __first;
  _ForwardIter __middle = __first;
  advance(__middle, __len / 2);
  return rotate(__inplace_stable_partition(__first, __middle, __pred, 
                                           __len / 2),
                __middle,
                __inplace_stable_partition(__middle, __last, __pred,
                                           __len - __len / 2));
}

template <class _ForwardIter, class _Pointer, class _Predicate, 
          class _Distance>
_ForwardIter __stable_partition_adaptive(_ForwardIter __first,
                                         _ForwardIter __last,
                                         _Predicate __pred, _Distance __len,
                                         _Pointer __buffer,
                                         _Distance __buffer_size) 
{
  if (__len <= __buffer_size) {
    _ForwardIter __result1 = __first;
    _Pointer __result2 = __buffer;
    for ( ; __first != __last ; ++__first)
      if (__pred(*__first)) {
        *__result1 = *__first;
        ++__result1;
      }
      else {
        *__result2 = *__first;
        ++__result2;
      }
    copy(__buffer, __result2, __result1);
    return __result1;
  }
  else {
    _ForwardIter __middle = __first;
    advance(__middle, __len / 2);
    return rotate(__stable_partition_adaptive(
                          __first, __middle, __pred,
                          __len / 2, __buffer, __buffer_size),
                    __middle,
                    __stable_partition_adaptive(
                          __middle, __last, __pred,
                          __len - __len / 2, __buffer, __buffer_size));
  }
}

template <class _ForwardIter, class _Predicate, class _Tp, class _Distance>
inline _ForwardIter
__stable_partition_aux(_ForwardIter __first, _ForwardIter __last, 
                       _Predicate __pred, _Tp*, _Distance*)
{
  _Temporary_buffer<_ForwardIter, _Tp> __buf(__first, __last);
  _STLP_MPWFIX_TRY		//*TY 06/01/2000 - they forget to call dtor for _Temporary_buffer if no try/catch block is present
  return (__buf.size() > 0) ?
    __stable_partition_adaptive(__first, __last, __pred,
				_Distance(__buf.requested_size()),
				__buf.begin(), __buf.size())  :
    __inplace_stable_partition(__first, __last, __pred, 
			       _Distance(__buf.requested_size()));
  _STLP_MPWFIX_CATCH	//*TY 06/01/2000 - they forget to call dtor for _Temporary_buffer if no try/catch block is present
}

template <class _ForwardIter, class _Predicate>
_ForwardIter 
stable_partition(_ForwardIter __first, _ForwardIter __last, _Predicate __pred) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last)
    return __first;
  else
    return __stable_partition_aux(__first, __last, __pred,
                                  _STLP_VALUE_TYPE(__first, _ForwardIter),
                                  _STLP_DISTANCE_TYPE(__first, _ForwardIter));
}

template <class _RandomAccessIter, class _Tp, class _Compare>
_RandomAccessIter __unguarded_partition(_RandomAccessIter __first, 
                                        _RandomAccessIter __last, 
                                        _Tp __pivot, _Compare __comp) 
{
  while (true) {
    while (__comp(*__first, __pivot))
      ++__first;
    --__last;
    while (__comp(__pivot, *__last))
      --__last;
    if (!(__first < __last))
      return __first;
    _STLP_STD::iter_swap(__first, __last);
    ++__first;
  }
}

// sort() and its auxiliary functions. 

# define  __stl_threshold  16

template <class _RandomAccessIter, class _Tp, class _Compare>
void __unguarded_linear_insert(_RandomAccessIter __last, _Tp __val, 
                               _Compare __comp) {
  _RandomAccessIter __next = __last;
  --__next;  
  while (__comp(__val, *__next)) {
    *__last = *__next;
    __last = __next;
    --__next;
  }
  *__last = __val;
}

template <class _RandomAccessIter, class _Tp, class _Compare>
inline void __linear_insert(_RandomAccessIter __first, 
                            _RandomAccessIter __last, _Tp __val, _Compare __comp) {		
  //*TY 12/26/1998 - added __val as a paramter
  //  _Tp __val = *__last;		    //*TY 12/26/1998 - __val supplied by caller
  if (__comp(__val, *__first)) {
    _STLP_STD::copy_backward(__first, __last, __last + 1);
    *__first = __val;
  }
  else
    __unguarded_linear_insert(__last, __val, __comp);
}

template <class _RandomAccessIter, class _Compare>
void __insertion_sort(_RandomAccessIter __first,
                      _RandomAccessIter __last, _Compare __comp) {
  if (__first == __last) return;
  for (_RandomAccessIter __i = __first + 1; __i != __last; ++__i)
    __linear_insert(__first, __i, *__i, __comp);	//*TY 12/26/1998 - supply *__i as __val
}

template <class _RandomAccessIter, class _Tp, class _Compare>
void __unguarded_insertion_sort_aux(_RandomAccessIter __first, 
                                    _RandomAccessIter __last,
                                    _Tp*, _Compare __comp) {
  for (_RandomAccessIter __i = __first; __i != __last; ++__i)
    __unguarded_linear_insert(__i, _Tp(*__i), __comp);
}

template <class _RandomAccessIter, class _Compare>
inline void __unguarded_insertion_sort(_RandomAccessIter __first, 
                                       _RandomAccessIter __last,
                                       _Compare __comp) {
  __unguarded_insertion_sort_aux(__first, __last, _STLP_VALUE_TYPE(__first, _RandomAccessIter), __comp);
}

template <class _RandomAccessIter, class _Compare>
void __final_insertion_sort(_RandomAccessIter __first, 
                            _RandomAccessIter __last, _Compare __comp) {
  if (__last - __first > __stl_threshold) {
    __insertion_sort(__first, __first + __stl_threshold, __comp);
    __unguarded_insertion_sort(__first + __stl_threshold, __last, __comp);
  }
  else
    __insertion_sort(__first, __last, __comp);
}

template <class _RandomAccessIter, class _Tp, class _Size, class _Compare>
void __introsort_loop(_RandomAccessIter __first,
                      _RandomAccessIter __last, _Tp*,
                      _Size __depth_limit, _Compare __comp)
{
  while (__last - __first > __stl_threshold) {
    if (__depth_limit == 0) {
      _STLP_STD::partial_sort(__first, __last, __last, __comp);
      return;
    }
    --__depth_limit;
    _RandomAccessIter __cut =
      __unguarded_partition(__first, __last,
                            _Tp(__median(*__first,
                                         *(__first + (__last - __first)/2),
                                         *(__last - 1), __comp)),
       __comp);
    __introsort_loop(__cut, __last, (_Tp*) 0, __depth_limit, __comp);
    __last = __cut;
  }
}

template <class _RandomAccessIter>
void sort(_RandomAccessIter __first, _RandomAccessIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first != __last) {
    __introsort_loop(__first, __last,
                     _STLP_VALUE_TYPE(__first, _RandomAccessIter),
                     __lg(__last - __first) * 2, __less(_STLP_VALUE_TYPE(__first, _RandomAccessIter)) );
    __final_insertion_sort(__first, __last, __less(_STLP_VALUE_TYPE(__first, _RandomAccessIter)));
  }
}

template <class _RandomAccessIter, class _Compare>
void sort(_RandomAccessIter __first, _RandomAccessIter __last, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first != __last) {
    __introsort_loop(__first, __last,
                     _STLP_VALUE_TYPE(__first, _RandomAccessIter),
                     __lg(__last - __first) * 2,
                     __comp);
    __final_insertion_sort(__first, __last, __comp);
  }
}

// stable_sort() and its auxiliary functions.

template <class _RandomAccessIter, class _Compare>
void __inplace_stable_sort(_RandomAccessIter __first,
                           _RandomAccessIter __last, _Compare __comp) {
  if (__last - __first < 15) {
    __insertion_sort(__first, __last, __comp);
    return;
  }
  _RandomAccessIter __middle = __first + (__last - __first) / 2;
  __inplace_stable_sort(__first, __middle, __comp);
  __inplace_stable_sort(__middle, __last, __comp);
  __merge_without_buffer(__first, __middle, __last,
                         __middle - __first,
                         __last - __middle,
                         __comp);
}

template <class _RandomAccessIter1, class _RandomAccessIter2,
          class _Distance, class _Compare>
void __merge_sort_loop(_RandomAccessIter1 __first,
                       _RandomAccessIter1 __last, 
                       _RandomAccessIter2 __result, _Distance __step_size,
                       _Compare __comp) {
  _Distance __two_step = 2 * __step_size;

  while (__last - __first >= __two_step) {
    __result = merge(__first, __first + __step_size,
                     __first + __step_size, __first + __two_step,
                     __result,
                     __comp);
    __first += __two_step;
  }
  __step_size = (min) (_Distance(__last - __first), __step_size);

  merge(__first, __first + __step_size,
        __first + __step_size, __last,
        __result,
        __comp);
}

const int __stl_chunk_size = 7;
        
template <class _RandomAccessIter, class _Distance, class _Compare>
void __chunk_insertion_sort(_RandomAccessIter __first, 
                            _RandomAccessIter __last,
                            _Distance __chunk_size, _Compare __comp)
{
  while (__last - __first >= __chunk_size) {
    __insertion_sort(__first, __first + __chunk_size, __comp);
    __first += __chunk_size;
  }
  __insertion_sort(__first, __last, __comp);
}

template <class _RandomAccessIter, class _Pointer, class _Distance,
          class _Compare>
void __merge_sort_with_buffer(_RandomAccessIter __first, 
                              _RandomAccessIter __last, _Pointer __buffer,
                              _Distance*, _Compare __comp) {
  _Distance __len = __last - __first;
  _Pointer __buffer_last = __buffer + __len;

  _Distance __step_size = __stl_chunk_size;
  __chunk_insertion_sort(__first, __last, __step_size, __comp);

  while (__step_size < __len) {
    __merge_sort_loop(__first, __last, __buffer, __step_size, __comp);
    __step_size *= 2;
    __merge_sort_loop(__buffer, __buffer_last, __first, __step_size, __comp);
    __step_size *= 2;
  }
}

template <class _BidirectionalIter1, class _BidirectionalIter2,
          class _Distance>
_BidirectionalIter1 __rotate_adaptive(_BidirectionalIter1 __first,
                                      _BidirectionalIter1 __middle,
                                      _BidirectionalIter1 __last,
                                      _Distance __len1, _Distance __len2,
                                      _BidirectionalIter2 __buffer,
                                      _Distance __buffer_size) {
  if (__len1 > __len2 && __len2 <= __buffer_size) {
    _BidirectionalIter2 __buffer_end = copy(__middle, __last, __buffer);
    copy_backward(__first, __middle, __last);
    return copy(__buffer, __buffer_end, __first);
  }
  else if (__len1 <= __buffer_size) {
    _BidirectionalIter2 __buffer_end = copy(__first, __middle, __buffer);
    copy(__middle, __last, __first);
    return copy_backward(__buffer, __buffer_end, __last);
  }
  else
    return rotate(__first, __middle, __last);
}

template <class _BidirectionalIter, class _Distance, class _Pointer,
          class _Compare>
void __merge_adaptive(_BidirectionalIter __first, 
                      _BidirectionalIter __middle, 
                      _BidirectionalIter __last,
                      _Distance __len1, _Distance __len2,
                      _Pointer __buffer, _Distance __buffer_size,
                      _Compare __comp) {
  if (__len1 <= __len2 && __len1 <= __buffer_size) {
    _Pointer __buffer_end = copy(__first, __middle, __buffer);
    merge(__buffer, __buffer_end, __middle, __last, __first, __comp);
  }
  else if (__len2 <= __buffer_size) {
    _Pointer __buffer_end = copy(__middle, __last, __buffer);
    __merge_backward(__first, __middle, __buffer, __buffer_end, __last,
                     __comp);
  }
  else {
    _BidirectionalIter __first_cut = __first;
    _BidirectionalIter __second_cut = __middle;
    _Distance __len11 = 0;
    _Distance __len22 = 0;
    if (__len1 > __len2) {
      __len11 = __len1 / 2;
      advance(__first_cut, __len11);
      __second_cut = lower_bound(__middle, __last, *__first_cut, __comp);
      __len22 += distance(__middle, __second_cut);   
    }
    else {
      __len22 = __len2 / 2;
      advance(__second_cut, __len22);
      __first_cut = upper_bound(__first, __middle, *__second_cut, __comp);
      __len11 += distance(__first, __first_cut);
    }
    _BidirectionalIter __new_middle =
      __rotate_adaptive(__first_cut, __middle, __second_cut, __len1 - __len11,
                        __len22, __buffer, __buffer_size);
    __merge_adaptive(__first, __first_cut, __new_middle, __len11,
                     __len22, __buffer, __buffer_size, __comp);
    __merge_adaptive(__new_middle, __second_cut, __last, __len1 - __len11,
                     __len2 - __len22, __buffer, __buffer_size, __comp);
  }
}

template <class _RandomAccessIter, class _Pointer, class _Distance, 
          class _Compare>
void __stable_sort_adaptive(_RandomAccessIter __first, 
                            _RandomAccessIter __last, _Pointer __buffer,
                            _Distance __buffer_size, _Compare __comp) {
  _Distance __len = (__last - __first + 1) / 2;
  _RandomAccessIter __middle = __first + __len;
  if (__len > __buffer_size) {
    __stable_sort_adaptive(__first, __middle, __buffer, __buffer_size, 
                           __comp);
    __stable_sort_adaptive(__middle, __last, __buffer, __buffer_size, 
                           __comp);
  }
  else {
    __merge_sort_with_buffer(__first, __middle, __buffer, (_Distance*)0,
                               __comp);
    __merge_sort_with_buffer(__middle, __last, __buffer, (_Distance*)0,
                               __comp);
  }
  __merge_adaptive(__first, __middle, __last, _Distance(__middle - __first), 
                   _Distance(__last - __middle), __buffer, __buffer_size,
                   __comp);
}

template <class _RandomAccessIter, class _Tp, class _Distance, class _Compare>
void __stable_sort_aux(_RandomAccessIter __first,
			      _RandomAccessIter __last, _Tp*, _Distance*,
			      _Compare __comp) {
  _Temporary_buffer<_RandomAccessIter, _Tp> buf(__first, __last);
  if (buf.begin() == 0)
    __inplace_stable_sort(__first, __last, __comp);
  else 
    __stable_sort_adaptive(__first, __last, buf.begin(),
                           _Distance(buf.size()),
                           __comp);
}

template <class _RandomAccessIter>
void stable_sort(_RandomAccessIter __first,
		 _RandomAccessIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  __stable_sort_aux(__first, __last,
                    _STLP_VALUE_TYPE(__first, _RandomAccessIter),
                    _STLP_DISTANCE_TYPE(__first, _RandomAccessIter),
                    __less(_STLP_VALUE_TYPE(__first, _RandomAccessIter)));
}

template <class _RandomAccessIter, class _Compare>
void stable_sort(_RandomAccessIter __first,
		 _RandomAccessIter __last, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  __stable_sort_aux(__first, __last,
                    _STLP_VALUE_TYPE(__first, _RandomAccessIter),
                    _STLP_DISTANCE_TYPE(__first, _RandomAccessIter), 
                    __comp);
}

// partial_sort, partial_sort_copy, and auxiliary functions.

template <class _RandomAccessIter, class _Tp, class _Compare>
void __partial_sort(_RandomAccessIter __first, _RandomAccessIter __middle,
                    _RandomAccessIter __last, _Tp*, _Compare __comp) {
  _STLP_STD::make_heap(__first, __middle, __comp);
  for (_RandomAccessIter __i = __middle; __i < __last; ++__i)
    if (__comp(*__i, *__first))
      __pop_heap(__first, __middle, __i, _Tp(*__i), __comp,
                 _STLP_DISTANCE_TYPE(__first, _RandomAccessIter));
  _STLP_STD::sort_heap(__first, __middle, __comp);
}


template <class _RandomAccessIter>
void 
partial_sort(_RandomAccessIter __first,_RandomAccessIter __middle, _RandomAccessIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __middle))
  _STLP_DEBUG_CHECK(__check_range(__middle, __last))
  __partial_sort(__first, __middle, __last, _STLP_VALUE_TYPE(__first, _RandomAccessIter), 
                 __less(_STLP_VALUE_TYPE(__first, _RandomAccessIter)));
}

template <class _RandomAccessIter, class _Compare>
void partial_sort(_RandomAccessIter __first,_RandomAccessIter __middle,
                  _RandomAccessIter __last, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __middle))
  _STLP_DEBUG_CHECK(__check_range(__middle, __last))
  __partial_sort(__first, __middle, __last, _STLP_VALUE_TYPE(__first, _RandomAccessIter), __comp);
}

template <class _InputIter, class _RandomAccessIter, class _Compare,
          class _Distance, class _Tp>
_RandomAccessIter __partial_sort_copy(_InputIter __first,
                                         _InputIter __last,
                                         _RandomAccessIter __result_first,
                                         _RandomAccessIter __result_last,
                                         _Compare __comp, _Distance*, _Tp*) {
  if (__result_first == __result_last) return __result_last;
  _RandomAccessIter __result_real_last = __result_first;
  while(__first != __last && __result_real_last != __result_last) {
    *__result_real_last = *__first;
    ++__result_real_last;
    ++__first;
  }
  make_heap(__result_first, __result_real_last, __comp);
  while (__first != __last) {
    if (__comp(*__first, *__result_first))
      __adjust_heap(__result_first, _Distance(0),
                    _Distance(__result_real_last - __result_first),
                    _Tp(*__first),
                    __comp);
    ++__first;
  }
  sort_heap(__result_first, __result_real_last, __comp);
  return __result_real_last;
}

template <class _InputIter, class _RandomAccessIter>
_RandomAccessIter
partial_sort_copy(_InputIter __first, _InputIter __last,
                  _RandomAccessIter __result_first, _RandomAccessIter __result_last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  _STLP_DEBUG_CHECK(__check_range(__result_first, __result_last))
  return __partial_sort_copy(__first, __last, __result_first, __result_last, 
                             __less(_STLP_VALUE_TYPE(__first, _InputIter)),
                             _STLP_DISTANCE_TYPE(__result_first, _RandomAccessIter),
                             _STLP_VALUE_TYPE(__first, _InputIter));
}

template <class _InputIter, class _RandomAccessIter, class _Compare>
_RandomAccessIter
partial_sort_copy(_InputIter __first, _InputIter __last,
                  _RandomAccessIter __result_first,
                  _RandomAccessIter __result_last, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  _STLP_DEBUG_CHECK(__check_range(__result_first, __result_last))
  return __partial_sort_copy(__first, __last, __result_first, __result_last,
                             __comp,
                             _STLP_DISTANCE_TYPE(__result_first, _RandomAccessIter),
                             _STLP_VALUE_TYPE(__first, _InputIter));
}

// nth_element() and its auxiliary functions.  

template <class _RandomAccessIter, class _Tp, class _Compare>
void __nth_element(_RandomAccessIter __first, _RandomAccessIter __nth,
                   _RandomAccessIter __last, _Tp*, _Compare __comp) {
  while (__last - __first > 3) {
    _RandomAccessIter __cut =
      __unguarded_partition(__first, __last,
                            _Tp(__median(*__first,
                                         *(__first + (__last - __first)/2), 
                                         *(__last - 1),
                                         __comp)),
                            __comp);
    if (__cut <= __nth)
      __first = __cut;
    else 
      __last = __cut;
  }
  __insertion_sort(__first, __last, __comp);
}


template <class _RandomAccessIter>
void nth_element(_RandomAccessIter __first, _RandomAccessIter __nth,
                 _RandomAccessIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __nth))
  _STLP_DEBUG_CHECK(__check_range(__nth, __last))
  __nth_element(__first, __nth, __last, _STLP_VALUE_TYPE(__first, _RandomAccessIter), 
                __less(_STLP_VALUE_TYPE(__first, _RandomAccessIter)));
}

template <class _RandomAccessIter, class _Compare>
void nth_element(_RandomAccessIter __first, _RandomAccessIter __nth,
                 _RandomAccessIter __last, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __nth))
  _STLP_DEBUG_CHECK(__check_range(__nth, __last))
  __nth_element(__first, __nth, __last, _STLP_VALUE_TYPE(__first, _RandomAccessIter), __comp);
}

// Binary search (lower_bound, upper_bound, equal_range, binary_search).

template <class _ForwardIter, class _Tp, class _Compare, class _Distance>
_ForwardIter __upper_bound(_ForwardIter __first, _ForwardIter __last,
                           const _Tp& __val, _Compare __comp, _Distance*)
{
  _Distance __len = distance(__first, __last);
  _Distance __half;

  while (__len > 0) {
    __half = __len >> 1;
    _ForwardIter __middle = __first;
    advance(__middle, __half);
    if (__comp(__val, *__middle))
      __len = __half;
    else {
      __first = __middle;
      ++__first;
      __len = __len - __half - 1;
    }
  }
  return __first;
}

template <class _ForwardIter, class _Tp, class _Compare, class _Distance>
pair<_ForwardIter, _ForwardIter>
__equal_range(_ForwardIter __first, _ForwardIter __last, const _Tp& __val,
              _Compare __comp, _Distance*)
{
  _Distance __len = distance(__first, __last);
  _Distance __half;

  while (__len > 0) {
    __half = __len >> 1;
    _ForwardIter __middle = __first;
    advance(__middle, __half);
    if (__comp(*__middle, __val)) {
      __first = __middle;
      ++__first;
      __len = __len - __half - 1;
    }
    else if (__comp(__val, *__middle))
      __len = __half;
    else {
      _ForwardIter __left = lower_bound(__first, __middle, __val, __comp);
      advance(__first, __len);
      _ForwardIter __right = upper_bound(++__middle, __first, __val, __comp);
      return pair<_ForwardIter, _ForwardIter>(__left, __right);
    }
  }
  return pair<_ForwardIter, _ForwardIter>(__first, __first);
}           

template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter merge(_InputIter1 __first1, _InputIter1 __last1,
                  _InputIter2 __first2, _InputIter2 __last2,
                  _OutputIter __result) {
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  while (__first1 != __last1 && __first2 != __last2) {
    if (*__first2 < *__first1) {
      *__result = *__first2;
      ++__first2;
    }
    else {
      *__result = *__first1;
      ++__first1;
    }
    ++__result;
  }
  return copy(__first2, __last2, copy(__first1, __last1, __result));
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
          class _Compare>
_OutputIter merge(_InputIter1 __first1, _InputIter1 __last1,
                  _InputIter2 __first2, _InputIter2 __last2,
                  _OutputIter __result, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  while (__first1 != __last1 && __first2 != __last2) {
    if (__comp(*__first2, *__first1)) {
      *__result = *__first2;
      ++__first2;
    }
    else {
      *__result = *__first1;
      ++__first1;
    }
    ++__result;
  }
  return copy(__first2, __last2, copy(__first1, __last1, __result));
}

template <class _BidirectionalIter, class _Distance, class _Compare>
void __merge_without_buffer(_BidirectionalIter __first,
                            _BidirectionalIter __middle,
                            _BidirectionalIter __last,
                            _Distance __len1, _Distance __len2,
                            _Compare __comp) {
  if (__len1 == 0 || __len2 == 0)
    return;
  if (__len1 + __len2 == 2) {
    if (__comp(*__middle, *__first))
      iter_swap(__first, __middle);
    return;
  }
  _BidirectionalIter __first_cut = __first;
  _BidirectionalIter __second_cut = __middle;
  _Distance __len11 = 0;
  _Distance __len22 = 0;
  if (__len1 > __len2) {
    __len11 = __len1 / 2;
    advance(__first_cut, __len11);
    __second_cut = lower_bound(__middle, __last, *__first_cut, __comp);
    __len22 += distance(__middle, __second_cut);
  }
  else {
    __len22 = __len2 / 2;
    advance(__second_cut, __len22);
    __first_cut = upper_bound(__first, __middle, *__second_cut, __comp);
    __len11 +=distance(__first, __first_cut);
  }
  _BidirectionalIter __new_middle
    = rotate(__first_cut, __middle, __second_cut);
  __merge_without_buffer(__first, __first_cut, __new_middle, __len11, __len22,
                         __comp);
  __merge_without_buffer(__new_middle, __second_cut, __last, __len1 - __len11,
                         __len2 - __len22, __comp);
}

template <class _BidirectionalIter1, class _BidirectionalIter2,
          class _BidirectionalIter3, class _Compare>
_BidirectionalIter3 __merge_backward(_BidirectionalIter1 __first1,
                                     _BidirectionalIter1 __last1,
                                     _BidirectionalIter2 __first2,
                                     _BidirectionalIter2 __last2,
                                     _BidirectionalIter3 __result,
                                     _Compare __comp) {
  if (__first1 == __last1)
    return copy_backward(__first2, __last2, __result);
  if (__first2 == __last2)
    return copy_backward(__first1, __last1, __result);
  --__last1;
  --__last2;
  while (true) {
    if (__comp(*__last2, *__last1)) {
      *--__result = *__last1;
      if (__first1 == __last1)
        return copy_backward(__first2, ++__last2, __result);
      --__last1;
    }
    else {
      *--__result = *__last2;
      if (__first2 == __last2)
        return copy_backward(__first1, ++__last1, __result);
      --__last2;
    }
  }
}

template <class _BidirectionalIter, class _Tp, 
          class _Distance, class _Compare>
inline void __inplace_merge_aux(_BidirectionalIter __first,
                                _BidirectionalIter __middle,
                                _BidirectionalIter __last, _Tp*, _Distance*,
                                _Compare __comp) {
  _Distance __len1 = distance(__first, __middle);
  _Distance __len2 = distance(__middle, __last);

  _Temporary_buffer<_BidirectionalIter, _Tp> __buf(__first, __last);
  if (__buf.begin() == 0)
    __merge_without_buffer(__first, __middle, __last, __len1, __len2, __comp);
  else
    __merge_adaptive(__first, __middle, __last, __len1, __len2,
                     __buf.begin(), _Distance(__buf.size()),
                     __comp);
}

template <class _BidirectionalIter>
void inplace_merge(_BidirectionalIter __first,
		   _BidirectionalIter __middle,
		   _BidirectionalIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __middle))
  _STLP_DEBUG_CHECK(__check_range(__middle, __last))
  if (__first == __middle || __middle == __last)
    return;
  __inplace_merge_aux(__first, __middle, __last,
                      _STLP_VALUE_TYPE(__first, _BidirectionalIter), _STLP_DISTANCE_TYPE(__first, _BidirectionalIter),
                      __less(_STLP_VALUE_TYPE(__first, _BidirectionalIter)));
}

template <class _BidirectionalIter, class _Compare>
void inplace_merge(_BidirectionalIter __first,
		   _BidirectionalIter __middle,
		   _BidirectionalIter __last, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __middle))
  _STLP_DEBUG_CHECK(__check_range(__middle, __last))
  if (__first == __middle || __middle == __last)
    return;
  __inplace_merge_aux(__first, __middle, __last,
                      _STLP_VALUE_TYPE(__first, _BidirectionalIter), _STLP_DISTANCE_TYPE(__first, _BidirectionalIter),
                      __comp);
}


template <class _InputIter1, class _InputIter2, class _Compare>
bool __includes(_InputIter1 __first1, _InputIter1 __last1,
                _InputIter2 __first2, _InputIter2 __last2, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  while (__first1 != __last1 && __first2 != __last2)
    if (__comp(*__first2, *__first1))
      return false;
    else if(__comp(*__first1, *__first2)) 
      ++__first1;
    else
      ++__first1, ++__first2;

  return __first2 == __last2;
}

template <class _InputIter1, class _InputIter2, class _Compare>
bool includes(_InputIter1 __first1, _InputIter1 __last1,
              _InputIter2 __first2, _InputIter2 __last2, _Compare __comp) {
  return __includes(__first1, __last1, __first2, __last2, __comp);
}

template <class _InputIter1, class _InputIter2>
bool includes(_InputIter1 __first1, _InputIter1 __last1,
              _InputIter2 __first2, _InputIter2 __last2) {
  return __includes(__first1, __last1, __first2, __last2, __less(_STLP_VALUE_TYPE(__first1, _InputIter1)));
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
          class _Compare>
_OutputIter __set_union(_InputIter1 __first1, _InputIter1 __last1,
                        _InputIter2 __first2, _InputIter2 __last2,
                        _OutputIter __result, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  while (__first1 != __last1 && __first2 != __last2) {
    if (__comp(*__first1, *__first2)) {
      *__result = *__first1;
      ++__first1;
    }
    else if (__comp(*__first2, *__first1)) {
      *__result = *__first2;
      ++__first2;
    }
    else {
      *__result = *__first1;
      ++__first1;
      ++__first2;
    }
    ++__result;
  }
  return copy(__first2, __last2, copy(__first1, __last1, __result));
}

template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter set_union(_InputIter1 __first1, _InputIter1 __last1,
                      _InputIter2 __first2, _InputIter2 __last2,
                      _OutputIter __result) {
  return __set_union(__first1, __last1, __first2, __last2, __result, __less(_STLP_VALUE_TYPE(__first1, _InputIter1)));  
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
          class _Compare>
_OutputIter set_union(_InputIter1 __first1, _InputIter1 __last1,
                      _InputIter2 __first2, _InputIter2 __last2,
                      _OutputIter __result, _Compare __comp) {
  return __set_union(__first1, __last1, __first2, __last2, __result, __comp);
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
          class _Compare>
_OutputIter __set_intersection(_InputIter1 __first1, _InputIter1 __last1,
                               _InputIter2 __first2, _InputIter2 __last2,
                               _OutputIter __result, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  while (__first1 != __last1 && __first2 != __last2)
    if (__comp(*__first1, *__first2))
      ++__first1;
    else if (__comp(*__first2, *__first1))
      ++__first2;
    else {
      *__result = *__first1;
      ++__first1;
      ++__first2;
      ++__result;
    }
  return __result;
}

template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter set_intersection(_InputIter1 __first1, _InputIter1 __last1,
                             _InputIter2 __first2, _InputIter2 __last2,
                             _OutputIter __result) {
  return __set_intersection(__first1, __last1, __first2, __last2, __result, __less(_STLP_VALUE_TYPE(__first1, _InputIter1)));  
}

template <class _InputIter1, class _InputIter2, class _OutputIter,
          class _Compare>
_OutputIter set_intersection(_InputIter1 __first1, _InputIter1 __last1,
                             _InputIter2 __first2, _InputIter2 __last2,
                             _OutputIter __result, _Compare __comp) {
  return __set_intersection(__first1, __last1, __first2, __last2, __result, __comp);
}

template <class _InputIter1, class _InputIter2, class _OutputIter, 
          class _Compare>
_OutputIter __set_difference(_InputIter1 __first1, _InputIter1 __last1,
                             _InputIter2 __first2, _InputIter2 __last2, 
                             _OutputIter __result, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  while (__first1 != __last1 && __first2 != __last2)
    if (__comp(*__first1, *__first2)) {
      *__result = *__first1;
      ++__first1;
      ++__result;
    }
    else if (__comp(*__first2, *__first1))
      ++__first2;
    else {
      ++__first1;
      ++__first2;
    }
  return copy(__first1, __last1, __result);
}

template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter set_difference(_InputIter1 __first1, _InputIter1 __last1,
                           _InputIter2 __first2, _InputIter2 __last2,
                           _OutputIter __result) {
  return __set_difference(__first1, __last1, __first2, __last2, __result, 
                          __less(_STLP_VALUE_TYPE(__first1, _InputIter1)));  
}

template <class _InputIter1, class _InputIter2, class _OutputIter, 
          class _Compare>
_OutputIter set_difference(_InputIter1 __first1, _InputIter1 __last1,
                           _InputIter2 __first2, _InputIter2 __last2, 
                           _OutputIter __result, _Compare __comp) {
  return __set_difference(__first1, __last1, __first2, __last2, __result, __comp);
}

template <class _InputIter1, class _InputIter2, class _OutputIter, class _Compare>
_OutputIter 
__set_symmetric_difference(_InputIter1 __first1, _InputIter1 __last1,
                           _InputIter2 __first2, _InputIter2 __last2,
                           _OutputIter __result, _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first1, __last1))
  _STLP_DEBUG_CHECK(__check_range(__first2, __last2))
  while (__first1 != __last1 && __first2 != __last2)
    if (__comp(*__first1, *__first2)) {
      *__result = *__first1;
      ++__first1;
      ++__result;
    }
    else if (__comp(*__first2, *__first1)) {
      *__result = *__first2;
      ++__first2;
      ++__result;
    }
    else {
      ++__first1;
      ++__first2;
    }
  return copy(__first2, __last2, copy(__first1, __last1, __result));
}

template <class _InputIter1, class _InputIter2, class _OutputIter>
_OutputIter 
set_symmetric_difference(_InputIter1 __first1, _InputIter1 __last1,
                         _InputIter2 __first2, _InputIter2 __last2,
                         _OutputIter __result) {
  return __set_symmetric_difference(__first1, __last1, __first2, __last2, __result,
                                    __less(_STLP_VALUE_TYPE(__first1, _InputIter1)));  
}

template <class _InputIter1, class _InputIter2, class _OutputIter, class _Compare>
_OutputIter 
set_symmetric_difference(_InputIter1 __first1, _InputIter1 __last1,
                         _InputIter2 __first2, _InputIter2 __last2,
                         _OutputIter __result,
                         _Compare __comp) {
  return __set_symmetric_difference(__first1, __last1, __first2, __last2, __result, __comp);
}

// min_element and max_element, with and without an explicitly supplied
// comparison function.

template <class _ForwardIter, class _Compare>
_ForwardIter max_element(_ForwardIter __first, _ForwardIter __last,
                            _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last) return __first;
  _ForwardIter __result = __first;
  while (++__first != __last) 
    if (__comp(*__result, *__first)) __result = __first;
  return __result;
}

template <class _ForwardIter>
_ForwardIter max_element(_ForwardIter __first, _ForwardIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last) return __first;
  _ForwardIter __result = __first;
  while (++__first != __last) 
    if (*__result < *__first)
      __result = __first;
  return __result;
}

template <class _ForwardIter>
_ForwardIter min_element(_ForwardIter __first, _ForwardIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last) return __first;
  _ForwardIter __result = __first;
  while (++__first != __last) 
    if (*__first < *__result)
      __result = __first;
  return __result;
}

template <class _ForwardIter, class _Compare>
_ForwardIter min_element(_ForwardIter __first, _ForwardIter __last,
                            _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last) return __first;
  _ForwardIter __result = __first;
  while (++__first != __last) 
    if (__comp(*__first, *__result)) __result = __first;
  return __result;
}

// next_permutation and prev_permutation, with and without an explicitly 
// supplied comparison function.
template <class _BidirectionalIter, class _Compare>
bool __next_permutation(_BidirectionalIter __first, _BidirectionalIter __last,
                        _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last)
    return false;
  _BidirectionalIter __i = __first;
  ++__i;
  if (__i == __last)
    return false;
  __i = __last;
  --__i;

  for(;;) {
    _BidirectionalIter __ii = __i;
    --__i;
    if (__comp(*__i, *__ii)) {
      _BidirectionalIter __j = __last;
      while (!__comp(*__i, *--__j))
        {}
      iter_swap(__i, __j);
      reverse(__ii, __last);
      return true;
    }
    if (__i == __first) {
      reverse(__first, __last);
      return false;
    }
  }
#if defined(_STLP_NEED_UNREACHABLE_RETURN)
    return 0;
#endif
}

template <class _BidirectionalIter>
bool next_permutation(_BidirectionalIter __first, _BidirectionalIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  return __next_permutation(__first, __last, __less(_STLP_VALUE_TYPE(__first, _BidirectionalIter)));
}

template <class _BidirectionalIter, class _Compare>
bool next_permutation(_BidirectionalIter __first, _BidirectionalIter __last,
                      _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  return __next_permutation(__first, __last, __comp);
}

template <class _BidirectionalIter, class _Compare>
bool __prev_permutation(_BidirectionalIter __first, _BidirectionalIter __last,
                      _Compare __comp) {
  if (__first == __last)
    return false;
  _BidirectionalIter __i = __first;
  ++__i;
  if (__i == __last)
    return false;
  __i = __last;
  --__i;

  for(;;) {
    _BidirectionalIter __ii = __i;
    --__i;
    if (__comp(*__ii, *__i)) {
      _BidirectionalIter __j = __last;
      while (!__comp(*--__j, *__i))
        {}
      iter_swap(__i, __j);
      reverse(__ii, __last);
      return true;
    }
    if (__i == __first) {
      reverse(__first, __last);
      return false;
    }
  }
#if defined(_STLP_NEED_UNREACHABLE_RETURN)
    return 0;
#endif
}

template <class _BidirectionalIter>
bool prev_permutation(_BidirectionalIter __first, _BidirectionalIter __last) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  return __prev_permutation(__first, __last, __less(_STLP_VALUE_TYPE(__first, _BidirectionalIter)));
}

template <class _BidirectionalIter, class _Compare>
bool prev_permutation(_BidirectionalIter __first, _BidirectionalIter __last,
                      _Compare __comp) {
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  return __prev_permutation(__first, __last, __comp);
}

# ifndef _STLP_NO_EXTENSIONS

// is_heap, a predicate testing whether or not a range is
// a heap.  This function is an extension, not part of the C++
// standard.


template <class _RandomAccessIter, class _Distance, class _StrictWeakOrdering>
bool __is_heap(_RandomAccessIter __first, _StrictWeakOrdering __comp,
               _Distance __n)
{
  _Distance __parent = 0;
  for (_Distance __child = 1; __child < __n; ++__child) {
    if (__comp(__first[__parent], __first[__child]))
      return false;
    if ((__child & 1) == 0)
      ++__parent;
  }
  return true;
}

template <class _RandomAccessIter>
bool is_heap(_RandomAccessIter __first, _RandomAccessIter __last)
{
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  return __is_heap(__first, __less(_STLP_VALUE_TYPE(__first, _RandomAccessIter)), __last - __first);
}

template <class _RandomAccessIter, class _StrictWeakOrdering>
bool is_heap(_RandomAccessIter __first, _RandomAccessIter __last,
	     _StrictWeakOrdering __comp)
{
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  return __is_heap(__first, __comp, __last - __first);
}


template <class _ForwardIter, class _StrictWeakOrdering>
bool __is_sorted(_ForwardIter __first, _ForwardIter __last,
                 _StrictWeakOrdering __comp)
{
  _STLP_DEBUG_CHECK(__check_range(__first, __last))
  if (__first == __last)
    return true;

  _ForwardIter __next = __first;
  for (++__next; __next != __last; __first = __next, ++__next) {
    if (__comp(*__next, *__first))
      return false;
  }

  return true;
}

# endif /* _STLP_NO_EXTENSIONS */

_STLP_END_NAMESPACE

# undef __stl_threshold

#endif /* _STLP_ALGO_C */
// Local Variables:
// mode:C++
// End:
