/*
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

#ifndef _STLP_DEBUG_H
# define _STLP_DEBUG_H

# if defined (_STLP_ASSERTIONS) || defined (_STLP_DEBUG)

#ifndef _STLP_CONFIG_H
# include <stl/_config.h>
#endif

# if !defined (_STLP_EXTRA_OPERATORS_FOR_DEBUG) && \
     ( defined (_STLP_BASE_MATCH_BUG) || (defined (_STLP_MSVC) && _STLP_MSVC < 1100 ) )
#  define _STLP_EXTRA_OPERATORS_FOR_DEBUG
# endif

# if !defined(_STLP_FILE__)
#   define _STLP_FILE__ __FILE__
# endif
 
enum {
  _StlFormat_ERROR_RETURN,
  _StlFormat_ASSERTION_FAILURE,
  _StlFormat_VERBOSE_ASSERTION_FAILURE,
  _StlMsg_INVALID_ARGUMENT,
  _StlMsg_INVALID_CONTAINER,
  _StlMsg_EMPTY_CONTAINER,
  _StlMsg_ERASE_PAST_THE_END,
  _StlMsg_OUT_OF_BOUNDS,
  _StlMsg_NOT_OWNER,
  _StlMsg_INVALID_ITERATOR,
  _StlMsg_INVALID_LEFTHAND_ITERATOR,
  _StlMsg_INVALID_RIGHTHAND_ITERATOR,
  _StlMsg_DIFFERENT_OWNERS     ,
  _StlMsg_NOT_DEREFERENCEABLE  ,
  _StlMsg_INVALID_RANGE        ,
  _StlMsg_NOT_IN_RANGE_1       ,
  _StlMsg_NOT_IN_RANGE_2       ,
  _StlMsg_INVALID_ADVANCE      ,
  _StlMsg_SINGULAR_ITERATOR    ,
  // debug alloc messages
  _StlMsg_DBA_DELETED_TWICE    ,
  _StlMsg_DBA_NEVER_ALLOCATED  ,
  _StlMsg_DBA_TYPE_MISMATCH    ,
  _StlMsg_DBA_SIZE_MISMATCH    ,
  _StlMsg_DBA_UNDERRUN         ,
  _StlMsg_DBA_OVERRUN          ,
  // auto_ptr messages
  _StlMsg_AUTO_PTR_NULL    ,
  _StlMsg_UNKNOWN
  /* _StlMsg_MAX */
};

/* have to hardcode that ;() */
# define _StlMsg_MAX 27

_STLP_BEGIN_NAMESPACE

// This class is unique (not inherited from exception),
// to disallow catch in anything but (...)
struct __stl_debug_exception {
  // no members
};

class _STLP_CLASS_DECLSPEC __owned_link;
class _STLP_CLASS_DECLSPEC __owned_list;

template <class _Dummy>
struct __stl_debug_engine {

  // Basic routine to report any debug message
  // Use _STLP_DEBUG_MESSAGE to override
  static void _STLP_CALL _Message(const char * format_str, ...);

  // Micsellanous function to report indexed error message
  static void _STLP_CALL  _IndexedError(int __ind, const char* __f, int __l);

  // Basic assertion report mechanism.
  // Reports failed assertion via __stl_debug_message and calls _Terminate
  // if _STLP_DEBUG_TERMINATE is specified, calls __stl_debug_terminate instead
  static void _STLP_CALL  _Assert(const char* __expr, const char* __f, int __l);

  // The same, with additional diagnostics
  static void _STLP_CALL  _VerboseAssert(const char* __expr, int __error_ind, const char* __f, int __l);

  // If exceptions are present, sends unique exception
  // If not, calls _STLP_ABORT() to terminate
  // Use _STLP_DEBUG_TERMINATE to override
  static void _STLP_CALL  _Terminate();

# ifdef _STLP_DEBUG

  // owned_list/link delegate non-inline functions here

  static bool _STLP_CALL  _Check_same_owner( const __owned_link& __i1, 
                                             const __owned_link& __i2);
  static bool _STLP_CALL  _Check_same_owner_or_null( const __owned_link& __i1, 
                                                     const __owned_link& __i2);
  static bool  _STLP_CALL _Check_if_owner( const __owned_list*, const __owned_link&);

  static void _STLP_CALL  _Verify(const __owned_list*);
  
  static void _STLP_CALL  _Swap_owners(__owned_list&, __owned_list& /*, bool __swap_roots */ );
 
  static void _STLP_CALL  _Invalidate_all(__owned_list*);
  
  static void _STLP_CALL  _Stamp_all(__owned_list*, __owned_list*);

  static void _STLP_CALL  _M_detach(__owned_list*, __owned_link*);

  static void _STLP_CALL  _M_attach(__owned_list*, __owned_link*);

  // accessor : check and get pointer to the container
  static void* _STLP_CALL  _Get_container_ptr(const __owned_link*);
# endif /* _STLP_DEBUG */

  // debug messages and formats
   _STLP_STATIC_MEMBER_DECLSPEC static const char* _Message_table[_StlMsg_MAX];
};


# if defined (_STLP_USE_TEMPLATE_EXPORT)
_STLP_EXPORT_TEMPLATE struct _STLP_CLASS_DECLSPEC __stl_debug_engine<bool>;
# endif /* _STLP_USE_TEMPLATE_EXPORT */

typedef __stl_debug_engine<bool> __stl_debugger;

_STLP_END_NAMESPACE

#  ifndef _STLP_ASSERT
#   define _STLP_ASSERT(expr) \
     if (!(expr)) {STLPORT::__stl_debugger::_Assert( # expr, _STLP_FILE__, __LINE__);}
#  endif

# endif /* _STLP_ASSERTIONS || _STLP_DEBUG */


// this section is for _STLP_DEBUG only 
#if defined ( _STLP_DEBUG )

# ifndef _STLP_VERBOSE_ASSERT
// fbp : new form not requiring ";"
#  define _STLP_VERBOSE_ASSERT(expr,__diag_num) \
    if (!(expr)) { STLPORT::__stl_debugger::_VerboseAssert\
                                 ( # expr,  __diag_num, _STLP_FILE__, __LINE__ ); \
         }
# endif

#  define _STLP_DEBUG_CHECK(expr) _STLP_ASSERT(expr)
#  define _STLP_DEBUG_DO(expr)    expr;

# ifndef _STLP_VERBOSE_RETURN
#  define _STLP_VERBOSE_RETURN(__expr,__diag_num) if (!(__expr)) { \
       __stl_debugger::_IndexedError(__diag_num, __FILE__ , __LINE__); \
       return false; }
# endif

# ifndef _STLP_VERBOSE_RETURN_0
#  define _STLP_VERBOSE_RETURN_0(__expr,__diag_num) if (!(__expr)) { \
       __stl_debugger::_IndexedError(__diag_num, __FILE__ , __LINE__); \
       return 0; }
# endif

#if ! defined (_STLP_INTERNAL_THREADS_H)
# include <stl/_threads.h>
#endif

#ifndef _STLP_INTERNAL_ITERATOR_BASE_H
# include <stl/_iterator_base.h>
#endif

_STLP_BEGIN_NAMESPACE

//=============================================================
template <class _Iterator>
inline bool  _STLP_CALL __valid_range(const _Iterator& __i1 ,const _Iterator& __i2, 
                                      const random_access_iterator_tag&) { 
    return (__i1< __i2) || (__i1 == __i2);
}

template <class _Iterator>
inline bool  _STLP_CALL __valid_range(const _Iterator& __i1 ,const _Iterator& __i2,
                                      const bidirectional_iterator_tag&) { 
    // check if comparable
    bool __dummy(__i1==__i2);
    return (__dummy==__dummy); 
}

template <class _Iterator>
inline bool  _STLP_CALL __valid_range(const _Iterator& __i1 ,const _Iterator& __i2, const forward_iterator_tag&) { 
    // check if comparable
    bool __dummy(__i1==__i2);
    return (__dummy==__dummy); 
}

template <class _Iterator>
inline bool  _STLP_CALL __valid_range(const _Iterator&,const _Iterator&, const input_iterator_tag&) { 
    return true; 
}

template <class _Iterator>
inline bool  _STLP_CALL __valid_range(const _Iterator&,const _Iterator&, const output_iterator_tag&) { 
    return true; 
}

template <class _Iterator>
inline bool  _STLP_CALL __valid_range(const _Iterator& __i1, const _Iterator& __i2) { 
    return __valid_range(__i1,__i2,_STLP_ITERATOR_CATEGORY(__i1, _Iterator));
}

// Note : that means in range [i1, i2].
template <class _Iterator>
inline bool  _STLP_CALL __in_range(const _Iterator& _It, const _Iterator& __i1,
                                   const _Iterator& __i2) { 
    return __valid_range(__i1,_It,_STLP_ITERATOR_CATEGORY(__i1, _Iterator)) && 
        __valid_range(_It,__i2,_STLP_ITERATOR_CATEGORY(_It, _Iterator));
}

template <class _Iterator>
inline bool  _STLP_CALL __in_range(const _Iterator& __first, const _Iterator& __last, 
                                   const _Iterator& __start, const _Iterator& __finish) { 
    return __valid_range(__first,__last,_STLP_ITERATOR_CATEGORY(__first, _Iterator)) &&
        __valid_range(__start,__first,_STLP_ITERATOR_CATEGORY(__first, _Iterator)) && 
        __valid_range(__last,__finish,_STLP_ITERATOR_CATEGORY(__last, _Iterator));
}

//==========================================================


class _STLP_CLASS_DECLSPEC __owned_link {
public:

  __owned_link() : _M_owner(0) {}
  __owned_link(const __owned_list* __c) : _M_owner(0), _M_next(0) {
    __stl_debugger::_M_attach(__CONST_CAST(__owned_list*,__c), this);
  }
  __owned_link(const __owned_link& __rhs): _M_owner(0) {
    __stl_debugger::_M_attach(__CONST_CAST(__owned_list*,__rhs._M_owner), this);
  }
  __owned_link& operator=(const __owned_link& __rhs) {
    __owned_list* __new_owner = __CONST_CAST(__owned_list*,__rhs._M_owner);
    __owned_list* __old_owner = _M_owner;
    if ( __old_owner != __new_owner ) {
      __stl_debugger::_M_detach(__old_owner, this);
      __stl_debugger::_M_attach(__new_owner, this);
    }
    return *this;
  }
  ~__owned_link() {
    __stl_debugger::_M_detach(_M_owner, this);
    _Invalidate();
  }
  
  const __owned_list* _Owner() const { 
    return _M_owner; 
  }
  __owned_list* _Owner() { 
    return _M_owner; 
  }
  void _Set_owner(const __owned_list* __o)  { 
    _M_owner= __CONST_CAST(__owned_list*,__o); 
  }
  bool  _Valid() const { 
    return _M_owner !=0; 
  }

  void _Invalidate() { _M_owner=0; _M_next = 0; }
  void _Link_to_self() { _M_next= 0; }

  __owned_link* _Next() { return _M_next; }
  const __owned_link* _Next() const { return _M_next; }

public:
  __owned_list* _M_owner;
  __owned_link* _M_next;
};


class _STLP_CLASS_DECLSPEC __owned_list {
public:
  __owned_list(const void* __o) {
    //    fprintf(stderr, "__owned_list(): %p\n",(void*)this);
    _M_node._M_owner = __CONST_CAST(__owned_list*, __REINTERPRET_CAST(const __owned_list*,__o)); 
    _M_node._M_next=0;
  }
  ~__owned_list() {
    //    fprintf(stderr, "~__owned_list(): %p\n",(void*)this);
    _Invalidate_all();
    // that prevents detach
    _M_node._Invalidate();
  }
  const void* _Owner() const { 
    return (const void*)_M_node._M_owner; 
  }
  void* _Owner() { 
    return (void*)_M_node._M_owner; 
  }
  bool  _Valid() const { 
    return _M_node._M_owner!=0; 
  }
  void _Invalidate() { _M_node._M_owner=0; }
  
  __owned_link* _First() { return _M_node._Next(); }
  __owned_link* _Last() { return 0 ; }

  const __owned_link* _First() const { return (__owned_link*)_M_node._M_next; }
  const __owned_link* _Last() const { return 0 ;}
  
  void _Verify() const {
    __stl_debugger::_Verify(this); 
  }
  
  void _Swap_owners(__owned_list& __y) { 
    __stl_debugger::_Swap_owners(*this, __y); 
  }
 
  void _Invalidate_all() { 
    __stl_debugger::_Invalidate_all(this);
  }
  
  mutable __owned_link              _M_node; 
  mutable _STLP_mutex               _M_lock;
  
private:
  // should never be called, should be left undefined,
  // but some compilers complain about it ;(
  __owned_list(const __owned_list&){}
  void operator=(const __owned_list&) {}

  friend class __owned_link;
  friend struct __stl_debug_engine<bool>;
};


//==========================================================

// forward declaratioins

template <class _Iterator>
bool  _STLP_CALL __check_range(const _Iterator&, const _Iterator&);
template <class _Iterator>
bool _STLP_CALL  __check_range(const _Iterator&, 
                               const _Iterator&, const _Iterator&);
template <class _Iterator>
bool _STLP_CALL  __check_range(const _Iterator&, const _Iterator& , 
                               const _Iterator&, const _Iterator& );

template <class _Iterator>
void _STLP_CALL  __invalidate_range(const __owned_list* __base, 
                                    const _Iterator& __first,
                                    const _Iterator& __last);

template <class _Iterator>
void  _STLP_CALL __invalidate_iterator(const __owned_list* __base, 
                                       const _Iterator& __it);

//============================================================

inline bool _STLP_CALL 
__check_same_owner( const __owned_link& __i1, const __owned_link& __i2) {
  return __stl_debugger::_Check_same_owner(__i1,__i2);
}
inline bool _STLP_CALL  
__check_same_owner_or_null( const __owned_link& __i1, const __owned_link& __i2) {
  return __stl_debugger::_Check_same_owner_or_null(__i1,__i2);
}

template <class _Iterator>
inline bool _STLP_CALL  __check_if_owner( const __owned_list* __owner, 
                                          const _Iterator& __it) {
  return __stl_debugger::_Check_if_owner(__owner, (const __owned_link&)__it);
}

_STLP_END_NAMESPACE

# endif /* _STLP_DEBUG */

#if defined ( _STLP_ASSERTIONS )

# ifndef _STLP_ASSERT_MSG_TRAILER
#  define _STLP_ASSERT_MSG_TRAILER
# endif

// dwa 12/30/98 - if _STLP_DEBUG_MESSAGE is defined, the user can supply own definition.
# if !defined( _STLP_DEBUG_MESSAGE )
#   define __stl_debug_message __stl_debugger::_Message
# else
    extern  void __stl_debug_message(const char * format_str, ...);
# endif

// fbp: if _STLP_DEBUG_TERMINATE is defined, the user can supply own definition.
# if !defined( _STLP_DEBUG_TERMINATE )
#   define __stl_debug_terminate __stl_debugger::_Terminate
# else
    extern  void __stl_debug_terminate(void);
# endif

#endif

# if !defined (_STLP_LINK_TIME_INSTANTIATION)
#  include <stl/debug/_debug.c>
# endif

#endif /* DEBUG_H */

// Local Variables:
// mode:C++
// End:

