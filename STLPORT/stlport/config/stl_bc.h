// STLport configuration file
// It is internal STLport header - DO NOT include it directly


// versions ?
# if defined(_STLP_DESIGNATED_DLL)
#   define _STLP_CALL __cdecl __export
# elif defined(_RTLDLL)
#   define  _STLP_CALL __cdecl __import
# else
#   define  _STLP_CALL __cdecl
# endif

# define _USE_STLP
# define _STLP_USE_OWN_MBSTATE_T
// # define _STLP_USE_OWN_NAMESPACE
# define _STLP_DLLEXPORT_NEEDS_PREDECLARATION
# define _STLP_OPERATOR_SPEC_NEEDS_TEMPLATE_ARGS

// most of <exception> contents is still in global namespace
// # define _STLP_VENDOR_UNEXPECTED_STD

// <limits> problem
# define _STLP_STATIC_CONST_INIT_BUG

// <bitset> problems
# define _STLP_MEMBER_SPECIALIZATION_BUG
# define  _STLP_HAS_SPECIFIC_PROLOG_EPILOG 1

#if ( __BORLANDC__ < 0x540 )
// Borland C++ Builder 3 (?)
// those are assumptions, if some of them actually work, please let me know
// #  define _STLP_STATIC_CONST_INIT_BUG 1
// #  define _STLP_THROW_RETURN_BUG 1
#  define _STLP_NO_TEMPLATE_CONVERSIONS 1
#  define _STLP_DEFAULT_CONSTRUCTOR_BUG 1
#  define _STLP_NO_TYPENAME_ON_RETURN_TYPE
#else
#  define _STLP_LONG_LONG  __int64
#endif

// BCB 2 or less (Borland 5.02)
#if ( __BORLANDC__ < 0x530 )

#  define _STLP_NO_OWN_IOSTREAMS
#  undef  _STLP_OWN_IOSTREAMS

#  define _STLP_GLOBAL_VENDOR_CSTD 1
#  define _STLP_HAS_NO_NEW_IOSTREAMS 1
#  define _STLP_HAS_NO_NEW_C_HEADERS 1

#  define _STLP_NO_MEMBER_TEMPLATES 1
#  define _STLP_NO_MEMBER_TEMPLATE_CLASSES 1
#  define _STLP_NO_MEMBER_TEMPLATE_KEYWORD 1
#  define _STLP_NO_FRIEND_TEMPLATES 1
#  define _STLP_NO_QUALIFIED_FRIENDS 1
#  define _STLP_NO_CLASS_PARTIAL_SPECIALIZATION 1
#  define _STLP_NO_FUNCTION_TMPL_PARTIAL_ORDER 1
#  define _STLP_NO_EXPLICIT_FUNCTION_TMPL_ARGS 1

#  define _STLP_NO_PARTIAL_SPECIALIZATION_SYNTAX 1

#  define _STLP_NO_DEFAULT_NON_TYPE_PARAM 1
#  define _STLP_NON_TYPE_TMPL_PARAM_BUG 1
#  define _STLP_MEMBER_SPECIALIZATION_BUG
#  define _STLP_NO_EXCEPTION_HEADER 1
#  define _STLP_NO_EXCEPTION_SPEC 1

#  define _STLP_NO_BAD_ALLOC 1
#  define _STLP_NO_ARROW_OPERATOR 1

typedef char    mbstate_t;

#  define _STLP_LIMITED_DEFAULT_TEMPLATES 1
#  define _STLP_NO_TYPEINFO
#  define _STLP_NO_METHOD_SPECIALIZATION

#endif

// Borland 5.0x
#if ( __BORLANDC__ < 0x520 )

#  define _STLP_BROKEN_USING_DIRECTIVE 1
#  define _STLP_EXPORT_KEYWORD _export
#  define _STLP_IMPORT_KEYWORD _import
#  define _STLP_EXPORT_TEMPLATE_KEYWORD _export
#  define _STLP_IMPORT_TEMPLATE_KEYWORD _import
#endif

#if ( __BORLANDC__ < 0x501 )
#   define  _STLP_NONTEMPL_BASE_MATCH_BUG 1
#   define  _STLP_NO_WCHAR_T 1
#endif

// 4.x
#if ( __BORLANDC__ < 0x500 )
#   define _STLP_NESTED_TYPE_PARAM_BUG 1
#   define _STLP_STATIC_ARRAY_BUG 1
#   define _STLP_NO_BOOL 1
#   define _STLP_HAS_NO_NAMESPACES 1
#   define _STLP_NEED_TYPENAME 1
#   define _STLP_NEED_EXPLICIT 1
#   define _STLP_NEED_MUTABLE 1
#   define _STLP_NO_WCHAR_T 1
#endif

// auto enable thread safety and exceptions:
#   ifndef _CPPUNWIND
#     define _STLP_HAS_NO_EXCEPTIONS
#   endif

#   if defined ( __MT__ ) && !defined (_NOTHREADS) && !defined (_REENTRANT)
#     define _REENTRANT 1
#   endif

#  if defined ( __DEBUG ) && ( __DEBUG > 1 )
#   define _STLP_DEBUG
#  endif

#  define _STLP_LIB_BASENAME "stlp"

# if (__BORLANDC__ < 0x540)

#  define _STLP_EXPORT_DECLSPEC __declspec(dllexport)
#  define _STLP_IMPORT_DECLSPEC __declspec(dllimport)
#  define _STLP_IMPORT_TEMPLATE_KEYWORD  extern
#  define _STLP_EXPORT_TEMPLATE_KEYWORD

#  define _STLP_CLASS_EXPORT_DECLSPEC __declspec(dllexport)
#  define _STLP_CLASS_IMPORT_DECLSPEC __declspec(dllimport)

#   if !defined (_STLP_NO_OWN_IOSTREAMS)

#    if ( defined (__DLL) || defined (_DLL) || defined (_WINDLL) || defined (_RTLDLL) \
     || defined (_STLP_USE_DYNAMIC_LIB) ) \
       && ! defined (_STLP_USE_STATIC_LIB)
#      undef  _STLP_USE_DECLSPEC
#      define _STLP_USE_DECLSPEC
#     endif
#   endif

# else

# define _STLP_EXPORT_DECLSPEC __declspec(dllexport)
# define _STLP_IMPORT_DECLSPEC __declspec(dllimport)

# define _STLP_CLASS_EXPORT_DECLSPEC __declspec(dllexport)
# define _STLP_CLASS_IMPORT_DECLSPEC __declspec(dllimport)

#  if !defined (_STLP_NO_OWN_IOSTREAMS)

#    if ( defined (__DLL) || defined (_DLL) || defined (_WINDLL) || defined (_RTLDLL) \
     || defined(_AFXDLL) || defined (_STLP_USE_DYNAMIC_LIB) )
#      undef  _STLP_USE_DECLSPEC
#      define _STLP_USE_DECLSPEC 1
#    endif

#  ifndef _STLP_IMPORT_TEMPLATE_KEYWORD
#   define _STLP_IMPORT_TEMPLATE_KEYWORD __declspec(dllimport)
#  endif
#  define _STLP_EXPORT_TEMPLATE_KEYWORD __declspec(dllexport)

# if (__BORLANDC__ >= 0x560) && ! defined (_STLP_OWN_IOSTREAMS)
// #  define _STLP_IS_NATIVE_LIB
#  define _STLP_NATIVE_INCLUDE_PATH ../include/oldstl
#  define _STLP_NATIVE_CPP_C_INCLUDE_PATH ../include/oldstl
#  define _STLP_NATIVE_C_INCLUDE_PATH ../include/oldstl
#  define _STLP_NATIVE_CPP_RUNTIME_PATH ../include/oldstl
# endif

# ifndef __BUILDING_STLPORT
#  ifdef _STLP_DEBUG
#   ifdef _MT
#    ifdef _RTLDLL
#      pragma comment(lib,"stlpmt_stldebug45.lib")
#    else
#     pragma comment(lib,"stlpmt_stldebugst.lib")
#    endif
#   else
#    ifdef _RTLDLL
#      pragma comment(lib,"stlp_stldebug45.lib")
#    else
#     pragma comment(lib,"stlp_stldebugst.lib")
#    endif
#   endif
#  else
#   ifdef _MT
#    ifdef _RTLDLL
#      pragma comment(lib,"stlpmt45.lib")
#    else
#     pragma comment(lib,"stlpmtst.lib")
#    endif
#  else
#    ifdef _RTLDLL
#      pragma comment(lib,"stlp45.lib")
#    else
#     pragma comment(lib,"stlpst.lib")
#    endif
#   endif
#  endif
# endif

#  endif /* _STLP_OWN_IOSTREAMS */
# endif
