
#if defined (_STLP_MSVC) || defined (__ICL) || defined (__BORLANDC__)

# if defined (__BORLANDC__)
#  if (__BORLANDC__ >= 0x510)
#  pragma option push -Vx- -Ve- -a8 -b -pc -w-inl -w-aus -w-sig -w-8062 -w-8041 -w-8008 -w-8012 -w-8027 -w-8057 -w-8091 -w-8092 -w-8066  /* P_O_1 */
#  endif
# else
# if !(defined (_STLP_MSVC) && (_STLP_MSVC < 1200))
#  pragma warning(push)
# endif
# pragma pack(push,8)
# include <config/_msvc_warnings_off.h>
# endif


#elif defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)

#pragma set woff 1209
#pragma set woff 1174
#pragma set woff 1375
// from iterator_base.h
#pragma set woff 1183

#elif defined(__DECCXX)

# ifdef __PRAGMA_ENVIRONMENT
#  pragma __environment __save
#  pragma __environment __header_defaults
# endif

#elif defined(__IBMCPP__)
// supress EDC3130: A constant is being used as a conditional expression
#pragma info(nocnd)

#elif defined (__HP_aCC)
/* _REENTRANT selects Posix 1c threads unless draft4 selected.
 *  * This usage is obsolescent, "-D_POSIX_C_SOURCE=199506" is preferred */
# if 0 /* defined (_REENTRANT) && ! defined (_POSIX_C_SOURCE) */
#  define _POSIX_C_SOURCE 199506
# endif
#elif defined (__WATCOMCPLUSPLUS__)
# pragma warning 604 10 // must lookahead to determine...
# pragma warning 594 10 // resolved as declaration/type
# pragma warning 595 10 // resolved as an expression
#endif
