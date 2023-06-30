
#ifndef _STLP_NEW_H_HEADER
# define _STLP_NEW_H_HEADER

# ifdef _STLP_NO_BAD_ALLOC

# ifndef _STLP_NEW_DONT_THROW
#   define _STLP_NEW_DONT_THROW 1
# endif /* _STLP_NEW_DONT_THROW */

#  include <exception>

_STLP_BEGIN_NAMESPACE

struct nothrow_t {};

# ifdef _STLP_OWN_IOSTREAMS
extern _STLP_DECLSPEC const nothrow_t nothrow;
# else
#  define nothrow nothrow_t()
# endif

class bad_alloc : public _STLP_EXCEPTION_BASE { 
public:
  bad_alloc () _STLP_NOTHROW_INHERENTLY { }
  bad_alloc(const bad_alloc&) _STLP_NOTHROW_INHERENTLY { }
  bad_alloc& operator=(const bad_alloc&) _STLP_NOTHROW_INHERENTLY {return *this;}
  ~bad_alloc () _STLP_NOTHROW_INHERENTLY { }
  const char* what() const _STLP_NOTHROW_INHERENTLY { return "bad alloc"; }
};

_STLP_END_NAMESPACE

#endif /* _STLP_NO_BAD_ALLOC */

#ifdef _STLP_WINCE
_STLP_BEGIN_NAMESPACE

inline void* _STLP_CALL __stl_new(size_t __n) {
  return ::malloc(__n);
}

inline void _STLP_CALL __stl_delete(void* __p) {
  free(__p);
}
_STLP_END_NAMESPACE

#else /* _STLP_WINCE */

#include <new>

# ifndef _STLP_NO_BAD_ALLOC
#  ifdef _STLP_USE_OWN_NAMESPACE

    _STLP_BEGIN_NAMESPACE
    using _STLP_VENDOR_EXCEPT_STD::bad_alloc;
    using _STLP_VENDOR_EXCEPT_STD::nothrow_t;
    using _STLP_VENDOR_EXCEPT_STD::nothrow;

#  if defined (_STLP_GLOBAL_NEW_HANDLER)
    using ::new_handler;
    using ::set_new_handler;
#  else
    using _STLP_VENDOR_EXCEPT_STD::new_handler;
    using _STLP_VENDOR_EXCEPT_STD::set_new_handler;
#  endif
    
    _STLP_END_NAMESPACE

#  endif /* _STLP_OWN_NAMESPACE */

# endif /* _STLP_NO_BAD_ALLOC */

# if defined (_STLP_NO_NEW_NEW_HEADER) || defined (_STLP_NEW_DONT_THROW) && ! defined (_STLP_CHECK_NULL_ALLOC)
#  define _STLP_CHECK_NULL_ALLOC(__x) void* __y = __x;if (__y == 0){_STLP_THROW(bad_alloc());}return __y
# else
#  define _STLP_CHECK_NULL_ALLOC(__x) return __x
# endif

_STLP_BEGIN_NAMESPACE

#if (( defined(__IBMCPP__)|| defined(__OS400__) || defined (__xlC__) || defined (qTidyHeap)) && defined(__DEBUG_ALLOC__) )
inline void*  _STLP_CALL __stl_new(size_t __n) {  _STLP_CHECK_NULL_ALLOC(::operator _STLP_NEW(__n, __FILE__, __LINE__)); }
inline void _STLP_CALL __stl_delete(void* __p) { ::operator delete(__p, __FILE__, __LINE__); }
#else
inline void*  _STLP_CALL __stl_new(size_t __n)   { _STLP_CHECK_NULL_ALLOC(::operator _STLP_NEW(__n)); }
inline void   _STLP_CALL __stl_delete(void* __p) { ::operator delete(__p); }
#endif
_STLP_END_NAMESPACE

# endif /* _STLP_WINCE */

#endif /* _STLP_NEW_H_HEADER */
