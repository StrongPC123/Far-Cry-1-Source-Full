// STLport configuration file for Digital Mars C++

#ifndef _STLP_DMC_H
# define _STLP_DMC_H

#if __DMC__ < 0x832
#error "Digital Mars C++ versions prior to 8.32 are not supported!"
#endif


#ifndef _CPPUNWIND
# define _STLP_NO_EXCEPTIONS
#endif
# undef _STLP_NO_NAMESPACES
# define _STLP_NO_RELOPS_NAMESPACE
# define _STLP_VENDOR_GLOBAL_CSTD
# define _STLP_VENDOR_GLOBAL_STD
# define _STLP_VENDOR_EXCEPT_STD std

# if !defined(_WIN32)
// it's not fully supported on non-Win32 platforms
#  define _STLP_NO_NEW_IOSTREAMS
#  define _STLP_NO_NATIVE_WIDE_FUNCTIONS
# endif

# if defined(_STLP_NO_NEW_IOSTREAMS) || defined(_STLP_NO_OWN_IOSTREAMS)
#  define _STLP_OWN_NAMESPACE
# else
#  define _STLP_NO_OWN_NAMESPACE
# endif


// select threads strategy
# if defined (_MT) && !defined (_NOTHREADS)
#  define _REENTRANT
# else
#  define _NOTHREADS
# endif

// select SGI-style alloc instead of allocator<T>
# define _STLP_USE_SGI_ALLOCATORS

// select allocation method you like
# undef _STLP_USE_MALLOC
# define _STLP_USE_NEWALLOC

// this one is not mandatory, just enabled
# undef _STLP_USE_DEFALLOC

// define _STLP_USE_ABBREVS if your linker has trouble with long 
// external symbols
# undef _STLP_USE_ABBREVS


// unsigned 32-bit integer type
#  define _STLP_UINT32_T unsigned

#  ifndef _BOOL_DEFINED
#   define _STLP_NO_BOOL
#  else
#   define _STLP_DONT_USE_BOOL_TYPEDEF
#  endif

#  undef _STLP_YVALS_H
#  undef _STLP_LIMITED_DEFAULT_TEMPLATES
#  define _STLP_DEFAULT_TYPE_PARAM
#  undef _STLP_NO_STATIC_TEMPLATE_DATA
#  undef _STLP_RAND48
#  undef _STLP_LOOP_INLINE_PROBLEMS

#  undef _STLP_HAS_NO_NAMESPACES

#  undef _STLP_NEED_TYPENAME
#  undef _STLP_NEED_EXPLICIT
#  undef _STLP_HAS_NO_EXCEPTIONS
#  undef _STLP_NO_EXCEPTION_SPEC
#  undef _STLP_WEAK_ATTRIBUTE
#  undef _STLP_BASE_MATCH_BUG
#  undef _STLP_NONTEMPL_BASE_MATCH_BUG
#  undef _STLP_NESTED_TYPE_PARAM_BUG
#  undef _STLP_NO_ARROW_OPERATOR
#  undef _STLP_UNINITIALIZABLE_PRIVATE
#  undef _STLP_BASE_TYPEDEF_BUG
#  undef _STLP_BASE_TYPEDEF_OUTSIDE_BUG
#  undef _STLP_CONST_CONSTRUCTOR_BUG

#  undef _STLP_NO_NEW_STYLE_CASTS

#  ifndef _WCHAR_T_DEFINED
#   define _STLP_NO_WCHAR_T
#  endif
#  define _STLP_HAS_NO_UNIX98_WCHAR_EXTENSIONS

#  undef _STLP_WCHAR_T_IS_USHORT

#  if _INTEGRAL_MAX_BITS >= 64
#   define _STLP_LONG_LONG long long
#  endif

#  undef _STLP_NO_LONG_DOUBLE
#  undef _STLP_NEED_MUTABLE
#  undef _STLP_NO_PARTIAL_SPECIALIZATION_SYNTAX
#  undef _STLP_NO_BAD_ALLOC
#  undef _STLP_DEBUG_ALLOC
#  undef _STLP_NO_MEMBER_TEMPLATES
#  undef _STLP_NO_MEMBER_TEMPLATE_CLASSES
#  define _STLP_NO_MEMBER_TEMPLATE_KEYWORD
#  undef _STLP_NO_FRIEND_TEMPLATES
#  undef _STLP_NO_QUALIFIED_FRIENDS
#  undef _STLP_NO_CLASS_PARTIAL_SPECIALIZATION
#  undef _STLP_NO_FUNCTION_TMPL_PARTIAL_ORDER
#  undef _STLP_AUTOMATIC_TYPE_TRAITS
#  undef _STLP_MEMBER_POINTER_PARAM_BUG
#  undef _STLP_NON_TYPE_TMPL_PARAM_BUG
#  undef _STLP_NO_DEFAULT_NON_TYPE_PARAM
#  undef _STLP_NO_METHOD_SPECIALIZATION
#  undef _STLP_STATIC_ARRAY_BUG
#  undef _STLP_STATIC_CONST_INIT_BUG
#  undef _STLP_TRIVIAL_CONSTRUCTOR_BUG
#  undef _STLP_TRIVIAL_DESTRUCTOR_BUG
#  undef _STLP_BROKEN_USING_DIRECTIVE
#  undef _STLP_NO_EXPLICIT_FUNCTION_TMPL_ARGS
#  undef _STLP_NO_EXCEPTION_HEADER
#  undef _STLP_DEFAULT_CONSTRUCTOR_BUG

#  define _STLP_HAS_NO_NEW_IOSTREAMS
#  define _STLP_HAS_NO_NEW_C_HEADERS
#  define _STLP_THROW_RETURN_BUG
#  undef _STLP_LINK_TIME_INSTANTIATION
#  undef _STLP_PARTIAL_SPEC_NEEDS_TEMPLATE_ARGS
#  undef _STLP_NO_TEMPLATE_CONVERSIONS
#  undef _STLP_NEEDS_EXTRA_TEMPLATE_CONSTRUCTORS


#  define _STLP_NO_NATIVE_MBSTATE_T


#  define _STLP_EXPORT_DECLSPEC __declspec(dllexport)
#  define _STLP_IMPORT_DECLSPEC __declspec(dllimport)

#  define _STLP_CLASS_EXPORT_DECLSPEC __declspec(dllexport)
#  define _STLP_CLASS_IMPORT_DECLSPEC __declspec(dllimport)

#  define _STLP_IMPORT_TEMPLATE_KEYWORD __declspec(dllimport)
#  define _STLP_EXPORT_TEMPLATE_KEYWORD __declspec(dllexport)

#define _STLP_NATIVE_HEADER(header)    <../include/##header>
#define _STLP_NATIVE_C_HEADER(header)    <../include/##header>
#define _STLP_NATIVE_CPP_C_HEADER(header)    <../include/##header>
#define _STLP_NATIVE_OLD_STREAMS_HEADER(header) <../include/##header>
#define _STLP_NATIVE_CPP_RUNTIME_HEADER(header) <../include/##header>


# if defined(__BUILDING_STLPORT) && defined(_WINDLL)
#  define _STLP_CALL __export

#  undef _STLP_USE_DECLSPEC
#  define _STLP_USE_DECLSPEC 1
# endif

# if !defined (__BUILDING_STLPORT) && !defined (_STLP_NO_OWN_IOSTREAMS)
#  if (defined (_DLL) && !defined (_STLP_DONT_USE_DLL)) || defined (_STLP_USE_DLL)
#   undef  _STLP_USE_DECLSPEC
#   define _STLP_USE_DECLSPEC 1
#  endif

#  if defined (_STLP_DEBUG)
#   if defined (_DLL)
#    if !defined (_STLP_DONT_USE_DLL)
#     pragma comment(lib,"stlp45dm_stldebug.lib")
#    else
#     pragma comment(lib,"stlp45dm_stldebug_staticx.lib")
#    endif
#   else
#    if defined (_STLP_USE_DLL)
#     pragma comment(lib,"stlp45dms_stldebug.lib")
#    else
#     pragma comment(lib,"stlp45dm_stldebug_static.lib")
#    endif
#   endif
#  elif defined (DEBUG)
#   if defined (_DLL)
#    if !defined (_STLP_DONT_USE_DLL)
#     pragma comment(lib,"stlp45dm_debug.lib")
#    else
#     pragma comment(lib,"stlp45dm_debug_staticx.lib")
#    endif
#   else
#    if defined (_STLP_USE_DLL)
#     pragma comment(lib,"stlp45dms_debug.lib")
#    else
#     pragma comment(lib,"stlp45dm_debug_static.lib")
#    endif
#   endif
#  else
#   if defined (_DLL)
#    if !defined (_STLP_DONT_USE_DLL)
#     pragma comment(lib,"stlp45dm.lib")
#    else
#     pragma comment(lib,"stlp45dm_staticx.lib")
#    endif
#   else
#    if defined (_STLP_USE_DLL)
#     pragma comment(lib,"stlp45dms.lib")
#    else
#     pragma comment(lib,"stlp45dm_static.lib")
#    endif
#   endif
#  endif
# endif


#ifdef __BUILDING_STLPORT
// still needed for building STLport itself, but isn't needed for
// compiling applications anymore
# undef __SC__
#endif

#undef __STLP_NO_KEYWORDS_WORKAROUND
#if !defined (__STLP_NO_KEYWORDS_WORKAROUND)
# define __in __stl_in
# define __out __stl_out
#endif

#endif
