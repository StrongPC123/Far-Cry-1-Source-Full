// SCO UDK 7 compiler (UnixWare 7x, OSR 5, UnixWare 2x)


#  define _STLP_NO_BAD_ALLOC 1

// allocator::rebind used not to work properly
// #  define _STLP_NO_MEMBER_TEMPLATE_CLASSES 1
// #  define _STLP_NO_MEMBER_TEMPLATE_KEYWORD 1

#  define _STLP_NO_FRIEND_TEMPLATES 1
#  define _STLP_NO_QUALIFIED_FRIENDS 1


// #  define _STLP_NO_DEFAULT_NON_TYPE_PARAM 1

//#  define _STLP_HAS_NO_NEW_IOSTREAMS 1
//#  define _STLP_HAS_NO_NEW_C_HEADERS 1

// ???
//#  define _STLP_STATIC_CONST_INIT_BUG 1

// ???
//#  define _STLP_LINK_TIME_INSTANTIATION 1

// ???
#  define _STLP_NO_TEMPLATE_CONVERSIONS 1

#     define _STLP_LONG_LONG long long

#     if defined(_REENTRANT)
#           define _UITHREADS     /* if      UnixWare < 7.0.1 */
#           define _STLP_UITHREADS
#     endif /* _REENTRANT */

# define _STLP_SCO_OPENSERVER
