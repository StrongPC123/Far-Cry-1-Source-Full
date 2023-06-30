# if !( defined(_STLP_WINCE) )
#  define _STLP_EXPORT_DECLSPEC __declspec(dllexport)
#  define _STLP_IMPORT_DECLSPEC __declspec(dllimport)
# endif

#  if !( defined(_STLP_MSVC) && _STLP_MSVC < 1100)
#   define _STLP_CLASS_EXPORT_DECLSPEC __declspec(dllexport)
#   define _STLP_CLASS_IMPORT_DECLSPEC __declspec(dllimport)
#  endif

#  if !defined (_STLP_NO_OWN_IOSTREAMS)

#    if ( defined (__DLL) || defined (_DLL) || defined (_WINDLL) || defined (_RTLDLL) \
     || defined(_AFXDLL) || defined (_STLP_USE_DYNAMIC_LIB) ) \
       && ! defined (_STLP_USE_STATIC_LIB)
#      undef  _STLP_USE_DECLSPEC
#      define _STLP_USE_DECLSPEC 1
#    endif

#  ifndef _STLP_IMPORT_TEMPLATE_KEYWORD
#   define _STLP_IMPORT_TEMPLATE_KEYWORD  extern
#  endif
#  define _STLP_EXPORT_TEMPLATE_KEYWORD

# if defined (_RTLDLL) && defined (_STLP_USE_STATIC_LIB)
#    define _STLP_LIB_STATIC_SUFFIX "_staticx"
# else
#    define _STLP_LIB_STATIC_SUFFIX "_static"
# endif

#    include <config/stl_select_lib.h>

#  endif /* _STLP_OWN_IOSTREAMS */


