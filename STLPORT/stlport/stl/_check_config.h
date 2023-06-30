// This file is reserved to site configuration purpose
// and should NEVER be overridden by user

# if defined ( _STLP_NO_OWN_IOSTREAMS )

// User choose not to use SGI iostreams, which means no
// precompiled library will be used and he is free to override
// any STLport configuration flags

# else

// The following will be defined in stl_config.h :
// # define _STLP_OWN_IOSTREAMS 1
# endif

/*
 *  Consistency check : if we use SGI iostreams, we have to use consistent
 *  thread model (single-threaded or multi-threaded) with the compiled library
 *  
 *  Default is multithreaded build. If you want to build and use single-threaded
 *  STLport, please change _STLP_NOTHREADS configuration setting above and rebuild the library
 *
 */

# if defined (_STLP_OWN_IOSTREAMS) \
  && !defined (_STLP_NO_THREADS) && !defined (_REENTRANT)

#  if defined(_MSC_VER) && !defined(__MWERKS__) && !defined (__COMO__) && !defined(_MT)
#   error "Only multi-threaded runtime library may be linked with STLport!"  
#  endif

// boris : you may change that to build non-threadsafe STLport library
#  if defined (__BUILDING_STLPORT) /* || defined (_STLP_DEBUG) */
#   define _REENTRANT 1
#  endif

# endif
