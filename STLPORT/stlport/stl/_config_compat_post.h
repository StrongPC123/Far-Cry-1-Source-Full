//==========================================

# define __SGI_STL_PORT  _STLPORT_VERSION

# if defined (_STLP_DEBUG) && ! defined ( __STL_DEBUG )
#  define __STL_DEBUG _STLP_DEBUG
# endif

# if defined (_STLP_USE_NAMESPACES)
#  undef  __STL_USE_NAMESPACES 
#  define __STL_USE_NAMESPACES _STLP_USE_NAMESPACES
# endif

# if defined (_STLP_USE_EXCEPTIONS)
#  undef  __STL_USE_EXCEPTIONS 
#  define __STL_USE_EXCEPTIONS _STLP_USE_EXCEPTIONS
# endif

# if defined (_STLP_USE_NEW_IOSTREAMS) && ! defined ( __STL_USE_NEW_IOSTREAMS )
#  define __STL_USE_NEW_IOSTREAMS _STLP_USE_NEW_IOSTREAMS
# endif

# if defined (_STLP_BEGIN_NAMESPACE) && ! defined ( __STL_BEGIN_NAMESPACE )
#  define __STL_BEGIN_NAMESPACE _STLP_BEGIN_NAMESPACE
#  define __STL_END_NAMESPACE _STLP_END_NAMESPACE
#  define __STL_VENDOR_STD _STLP_VENDOR_STD
#  define __STL_VENDOR_CSTD _STLP_VENDOR_CSTD
# endif

/*
# if defined (_STLP_XXX) && ! defined ( __STL_XXX )
#  define __STL_XXX _STLP_XXX
# endif
*/

