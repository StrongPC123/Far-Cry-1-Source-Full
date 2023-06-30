# if !defined (_STLP_NO_OWN_IOSTREAMS)

#  if ! defined (_STLP_LIB_STATIC_SUFFIX)
#   define _STLP_LIB_STATIC_SUFFIX ""
#  endif

// Note : the code below is intended to make use of compiled
// STLport iostreams easier. If you are with to change names used for
// STLport libraries , please also change RELEASE_NAME and DEBUG_NAME
// macros in makefile ../../src/vc6.mak (or whatever .mak you are using to build
// STLport). If you are using binaries, you may just rename the binaries.
#    if ! defined (__BUILDING_STLPORT) && ! defined (_STLP_DONT_FORCE_MSVC_LIB_NAME)
#     if defined (_STLP_USE_DECLSPEC)
#      ifdef _STLP_DEBUG
#       pragma comment(lib, _STLP_LIB_BASENAME"_stldebug.lib")
#      elif (defined (_DEBUG) || defined (__DEBUG)) && defined (_STLP_USE_DEBUG_LIB)
#       pragma comment(lib, _STLP_LIB_BASENAME"_debug.lib")
#      else
#       pragma comment(lib, _STLP_LIB_BASENAME".lib")
#      endif
#     else /* _STLP_USE_DECLSPEC */
// fbp : for static linking, debug setting _MUST_ correspond to what
// has been compiled into binary lib
#      ifdef _STLP_DEBUG
#       if (! defined (_DEBUG))
#        error "For static link with STLport library, _DEBUG setting MUST be on when _STLP_DEBUG is on. (/MTd forces _DEBUG)"
#       endif
#       pragma comment(lib, _STLP_LIB_BASENAME"_stldebug"_STLP_LIB_STATIC_SUFFIX".lib")
#      elif (defined (_DEBUG) || defined (__DEBUG)) && defined (_STLP_USE_DEBUG_LIB)
#       pragma comment(lib, _STLP_LIB_BASENAME"_debug"_STLP_LIB_STATIC_SUFFIX".lib")
#      else
#       pragma comment(lib, _STLP_LIB_BASENAME""_STLP_LIB_STATIC_SUFFIX".lib")
#      endif
#     endif /* _STLP_USE_DECLSPEC */
#    endif /* __BUILDING_STLPORT */
#   endif /* _STLP_OWN_IOSTREAMS */
