/*
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
#ifndef _STLP_PTHREAD_ALLOC_C
#define _STLP_PTHREAD_ALLOC_C

#ifdef __WATCOMC__
#pragma warning 13 9
#pragma warning 367 9
#pragma warning 368 9
#endif

#ifndef _STLP_PTHREAD_ALLOC_H
# include <stl/_pthread_alloc.h>
#endif

# if defined (_STLP_EXPOSE_GLOBALS_IMPLEMENTATION)

# include <cerrno>

_STLP_BEGIN_NAMESPACE

template <size_t _Max_size>
void _Pthread_alloc<_Max_size>::_S_destructor(void * __instance)
{
    _M_lock __lock_instance;	// Need to acquire lock here.
    _Pthread_alloc_per_thread_state<_Max_size>* __s =
        (_Pthread_alloc_per_thread_state<_Max_size> *)__instance;
    __s -> __next = _S_free_per_thread_states;
    _S_free_per_thread_states = __s;
}

template <size_t _Max_size>
_Pthread_alloc_per_thread_state<_Max_size> *
_Pthread_alloc<_Max_size>::_S_new_per_thread_state()
{    
    /* lock already held here.	*/
    if (0 != _S_free_per_thread_states) {
        _Pthread_alloc_per_thread_state<_Max_size> *__result =
					_S_free_per_thread_states;
        _S_free_per_thread_states = _S_free_per_thread_states -> __next;
        return __result;
    } else {
        return _STLP_NEW _Pthread_alloc_per_thread_state<_Max_size>;
    }
}

template <size_t _Max_size>
_Pthread_alloc_per_thread_state<_Max_size> *
_Pthread_alloc<_Max_size>::_S_get_per_thread_state()
{

    int __ret_code;
    __state_type* __result;
    
    if (_S_key_initialized && (__result = (__state_type*) pthread_getspecific(_S_key)))
      return __result;
    
    /*REFERENCED*/
    _M_lock __lock_instance;	// Need to acquire lock here.
    if (!_S_key_initialized) {
      if (pthread_key_create(&_S_key, _S_destructor)) {
	__THROW_BAD_ALLOC;  // failed
      }
      _S_key_initialized = true;
    }

    __result = _S_new_per_thread_state();
    __ret_code = pthread_setspecific(_S_key, __result);
    if (__ret_code) {
      if (__ret_code == ENOMEM) {
	__THROW_BAD_ALLOC;
      } else {
	// EINVAL
	_STLP_ABORT();
      }
    }
    return __result;
}

/* We allocate memory in large chunks in order to avoid fragmenting     */
/* the malloc heap too much.                                            */
/* We assume that size is properly aligned.                             */
template <size_t _Max_size>
char *_Pthread_alloc<_Max_size>
::_S_chunk_alloc(size_t __p_size, size_t &__nobjs)
{
  {
    char * __result;
    size_t __total_bytes;
    size_t __bytes_left;
    /*REFERENCED*/
    _M_lock __lock_instance;         // Acquire lock for this routine

    __total_bytes = __p_size * __nobjs;
    __bytes_left = _S_end_free - _S_start_free;
    if (__bytes_left >= __total_bytes) {
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    } else if (__bytes_left >= __p_size) {
        __nobjs = __bytes_left/__p_size;
        __total_bytes = __p_size * __nobjs;
        __result = _S_start_free;
        _S_start_free += __total_bytes;
        return(__result);
    } else {
        size_t __bytes_to_get =
		2 * __total_bytes + _S_round_up(_S_heap_size >> 4);
        // Try to make use of the left-over piece.
        if (__bytes_left > 0) {
            _Pthread_alloc_per_thread_state<_Max_size>* __a = 
                (_Pthread_alloc_per_thread_state<_Max_size>*)
			pthread_getspecific(_S_key);
            __obj * volatile * __my_free_list =
                        __a->__free_list + _S_freelist_index(__bytes_left);

            ((__obj *)_S_start_free) -> __free_list_link = *__my_free_list;
            *__my_free_list = (__obj *)_S_start_free;
        }
#       ifdef _SGI_SOURCE
          // Try to get memory that's aligned on something like a
          // cache line boundary, so as to avoid parceling out
          // parts of the same line to different threads and thus
          // possibly different processors.
          {
            const int __cache_line_size = 128;  // probable upper bound
            __bytes_to_get &= ~(__cache_line_size-1);
            _S_start_free = (char *)memalign(__cache_line_size, __bytes_to_get); 
            if (0 == _S_start_free) {
              _S_start_free = (char *)__malloc_alloc<0>::allocate(__bytes_to_get);
            }
          }
#       else  /* !SGI_SOURCE */
          _S_start_free = (char *)__malloc_alloc<0>::allocate(__bytes_to_get);
#       endif
        _S_heap_size += __bytes_to_get;
        _S_end_free = _S_start_free + __bytes_to_get;
    }
  }
  // lock is released here
  return(_S_chunk_alloc(__p_size, __nobjs));
}


/* Returns an object of size n, and optionally adds to size n free list.*/
/* We assume that n is properly aligned.                                */
/* We hold the allocation lock.                                         */
template <size_t _Max_size>
void *_Pthread_alloc_per_thread_state<_Max_size>
::_M_refill(size_t __n)
{
    size_t __nobjs = 128;
    char * __chunk =
	_Pthread_alloc<_Max_size>::_S_chunk_alloc(__n, __nobjs);
    __obj * volatile * __my_free_list;
    __obj * __result;
    __obj * __current_obj, * __next_obj;
    int __i;

    if (1 == __nobjs)  {
        return(__chunk);
    }
    __my_free_list = __free_list
		 + _Pthread_alloc<_Max_size>::_S_freelist_index(__n);

    /* Build free list in chunk */
      __result = (__obj *)__chunk;
      *__my_free_list = __next_obj = (__obj *)(__chunk + __n);
      for (__i = 1; ; __i++) {
        __current_obj = __next_obj;
        __next_obj = (__obj *)((char *)__next_obj + __n);
        if (__nobjs - 1 == __i) {
            __current_obj -> __free_list_link = 0;
            break;
        } else {
            __current_obj -> __free_list_link = __next_obj;
        }
      }
    return(__result);
}

template <size_t _Max_size>
void *_Pthread_alloc<_Max_size>
::reallocate(void *__p, size_t __old_sz, size_t __new_sz)
{
    void * __result;
    size_t __copy_sz;

    if (__old_sz > _Max_size
	&& __new_sz > _Max_size) {
        return(realloc(__p, __new_sz));
    }
    if (_S_round_up(__old_sz) == _S_round_up(__new_sz)) return(__p);
    __result = allocate(__new_sz);
    __copy_sz = __new_sz > __old_sz? __old_sz : __new_sz;
    memcpy(__result, __p, __copy_sz);
    deallocate(__p, __old_sz);
    return(__result);
}

#if defined (_STLP_STATIC_TEMPLATE_DATA) && (_STLP_STATIC_TEMPLATE_DATA > 0)

template <size_t _Max_size>
_Pthread_alloc_per_thread_state<_Max_size> * _Pthread_alloc<_Max_size>::_S_free_per_thread_states = 0;

template <size_t _Max_size>
pthread_key_t _Pthread_alloc<_Max_size>::_S_key =0;

template <size_t _Max_size>
bool _Pthread_alloc<_Max_size>::_S_key_initialized = false;

template <size_t _Max_size>
_STLP_mutex_base _Pthread_alloc<_Max_size>::_S_chunk_allocator_lock _STLP_MUTEX_INITIALIZER;

template <size_t _Max_size>
char *_Pthread_alloc<_Max_size>::_S_start_free = 0;

template <size_t _Max_size>
char *_Pthread_alloc<_Max_size>::_S_end_free = 0;

template <size_t _Max_size>
size_t _Pthread_alloc<_Max_size>::_S_heap_size = 0;

 # else
 
 __DECLARE_INSTANCE(template <size_t _Max_size> _Pthread_alloc_per_thread_state<_Max_size> *, _Pthread_alloc<_Max_size>::_S_free_per_thread_states, = 0);
 __DECLARE_INSTANCE(template <size_t _Max_size> pthread_key_t, _Pthread_alloc<_Max_size>::_S_key, = 0);
 __DECLARE_INSTANCE(template <size_t _Max_size> bool, _Pthread_alloc<_Max_size>::_S_key_initialized, = false);
 __DECLARE_INSTANCE(template <size_t _Max_size> char *, _Pthread_alloc<_Max_size>::_S_start_free, = 0);
 __DECLARE_INSTANCE(template <size_t _Max_size> char *, _Pthread_alloc<_Max_size>::_S_end_free, = 0);
 __DECLARE_INSTANCE(template <size_t _Max_size> size_t, _Pthread_alloc<_Max_size>::_S_heap_size, = 0);

# endif

_STLP_END_NAMESPACE

# endif /* _STLP_EXPOSE_GLOBALS_IMPLEMENTATION */

#endif /*  _STLP_PTHREAD_ALLOC_C */

// Local Variables:
// mode:C++
// End:
