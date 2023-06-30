// 
// This file defines site configuration.
//
//

/* 
 * _STLP_NO_THREADS: if defined, STLport don't use any 
 * multithreading support. Synonym is _NOTHREADS
 */
//#define _NOTHREADS
//#define _STLP_NO_THREADS

/* _PTHREADS: if defined, use Posix threads for multithreading support. */
// #define _PTHREADS

// compatibility section

# if defined (_STLP_NO_IOSTREAMS) || defined (_STLP_NO_NEW_IOSTREAMS) && ! defined ( _STLP_NO_OWN_IOSTREAMS )
#  define _STLP_NO_OWN_IOSTREAMS
# endif

# if !defined (_STLP_NO_OWN_IOSTREAMS) &&  ! defined (_STLP_OWN_IOSTREAMS)
#  define _STLP_OWN_IOSTREAMS
# endif

# if (defined (_STLP_NOTHREADS) || defined (_STLP_NO_THREADS) || defined (NOTHREADS))
#  if ! defined (_NOTHREADS)
#   define _NOTHREADS
#  endif
#  if ! defined (_STLP_NO_THREADS)
#   define _STLP_NO_THREADS
#  endif
# endif

/*
 * Turn _STLP_USE_DYNAMIC_LIB to enforce use of .dll version of STLport library.
 * NOTE : please do that only if you know what you are doing !
 * Changing default will require you to change makefile in "src" accordingly
 * and to rebuild STLPort library !
 * On UNIX, this has no effect. 
 *
 */
// # define _STLP_USE_DYNAMIC_LIB

/*
 * Turn _STLP_USE_STATIC_LIB to enforce use of static version of STLport library.
 * NOTE : please do that only if you know what you are doing !
 * Changing default will require you to change makefile in "src" accordingly
 * and to rebuild STLPort library !
 * On UNIX, this has no effect. 
 *
 */
// # define _STLP_USE_STATIC_LIB


/* 
 * Edit relative path below (or put full path) to get native 
 * compiler vendor's headers included. Default is "../include"
 * Hint : never install STLport in the directory that ends with "include"
 */
// #  undef _STLP_NATIVE_INCLUDE_PATH
// #  define _STLP_NATIVE_INCLUDE_PATH ../include
// same for C library headers like <cstring>
// #  undef _STLP_NATIVE_CPP_C_INCLUDE_PATH
// #  define _STLP_NATIVE_CPP_C_INCLUDE_PATH ../include
// same for C headers like <string.h>
// #  undef _STLP_NATIVE_C_INCLUDE_PATH
// #  define _STLP_NATIVE_C_INCLUDE_PATH ../include


/* 
 * _STLP_USE_OWN_NAMESPACE/_STLP_NO_OWN_NAMESPACE
 * If defined, STLport uses _STL:: namespace, else std::
 * The reason you have to use separate namespace in wrapper mode is that new-style IO
 * compiled library may have its own idea about STL stuff (string, vector, etc.),
 * so redefining them in the same namespace would break ODR and may cause
 * undefined behaviour. Rule of thumb is - if new-style iostreams are
 * available, there WILL be a conflict. Otherwise you should be OK.
 * In STLport iostreams mode, there is no need for this flag other than to facilitate
 * link with third-part libraries compiled with different standard library implementation.
 */
// #  define _STLP_USE_OWN_NAMESPACE 1
// #  define _STLP_NO_OWN_NAMESPACE  1


/* 
 * Uncomment _STLP_USE_NEWALLOC to force allocator<T> to use plain "new"
 * instead of STLport optimized node allocator engine.
 */
// #define   _STLP_USE_NEWALLOC   1

/* 
 * Uncomment _STLP_USE_MALLOC to force allocator<T> to use plain "malloc" 
 * instead of STLport optimized node allocator engine.
 */
// #define   _STLP_USE_MALLOC 1

/*
 * Set _STLP_DEBUG_ALLOC to use allocators that perform memory debugging,
 * such as padding/checking for memory consistency 
 */
// #define   _STLP_DEBUG_ALLOC 1


/*
 * Uncomment this to force all debug diagnostic to be directed through a
 * user-defined global function:
 *	void __stl_debug_message(const char * format_str, ...)
 * instead of predefined STLport routine. 
 * This allows you to take control of debug message output.
 * Default routine calls fprintf(stderr,...)
 * Note : If you set this macro, you must supply __stl_debug_message 
 * function definition somewhere.
 */
//#define _STLP_DEBUG_MESSAGE 1

/*
 * Uncomment this to force all failed assertions to be executed through
 * user-defined global function:
 *	void __stl_debug_terminate(void). This allows
 * you to take control of assertion behaviour for debugging purposes.
 * Default routine throws unique exception if _STLP_USE_EXCEPTIONS is set,
 * calls _STLP_ABORT() otherwise.
 * Note : If you set this macro, you must supply __stl_debug_terminate 
 * function definition somewhere.
 */
//#define _STLP_DEBUG_TERMINATE 1

/*
 * Comment this out to enable throwing exceptions from default __stl_debug_terminate()
 * instead of calling _STLP_ABORT().
 */
#define   _STLP_NO_DEBUG_EXCEPTIONS 1

/* 
 * Uncomment that to disable exception handling code 
 */
// #define   _STLP_NO_EXCEPTIONS 1

/*
 * _STLP_NO_NAMESPACES: if defined, don't put the library in namespace
 * stlport:: or std::, even if the compiler supports namespaces
 */

// #define   _STLP_NO_NAMESPACES 1

//==========================================================
// Compatibility section
//==========================================================

/* 
 * Use abbreviated class names for linker benefit (don't affect interface).
 * This option is obsolete, but should work in this release.
 *
 */
// # define _STLP_USE_ABBREVS

/* 
 * This definition precludes STLport reverse_iterator to be compatible with
 * other parts of MSVC library. (With partial specialization, it just
 * has no effect).
 * Use it _ONLY_ if you use SGI-style reverse_iterator<> template explicitly
 */
// #    define _STLP_NO_MSVC50_COMPATIBILITY 1

/*
 * _STLP_USE_RAW_SGI_ALLOCATORS is a hook so that users can disable use of
 * allocator<T> as default parameter for containers, and use SGI
 * raw allocators as default ones, without having to edit library headers.
 * Use of this macro is strongly discouraged.
 */
// #define   _STLP_USE_RAW_SGI_ALLOCATORS 1

/*
 * Use obsolete overloaded template functions iterator_category(), value_type(), distance_type()
 * for querying iterator properties. Please note those names are non-standard and are not guaranteed
 * to be used by every implementation. However, this setting is on by default when partial specialization
 * is not implemented in the compiler and cannot be sumulated (only if _STLP_NO_ANACHRONISMS is not set). 
 * Use of those interfaces for user-defined iterators is strongly discouraged: 
 * please use public inheritance from iterator<> template to achieve desired effect. 
 * Second form is to disable old-style queries in any case.
 */
// # define _STLP_USE_OLD_HP_ITERATOR_QUERIES
// # define _STLP_NO_OLD_HP_ITERATOR_QUERIES


//==========================================================================

// This section contains swithes which should be off by default,
// but so few compilers would have it undefined, so that we set them here,
// with the option to be turned off later in compiler-specific file

# define _STLP_INCOMPLETE_EXCEPTION_HEADER

