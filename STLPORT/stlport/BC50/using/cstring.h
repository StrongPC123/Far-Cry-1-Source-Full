using _STLP_VENDOR_CSTD::size_t;

#  ifndef _STLP_NO_CSTD_FUNCTION_IMPORTS
#   if defined(__MSL__) && __MC68K__ && !_No_BlockMove && __dest_os == __mac_os
#    undef memcpy
#    undef memmove
inline void* memcpy(void* dst, const void* src, size_t len)
{
	return _STLP_VENDOR_CSTD::__memcpy(dst, src, len);
}
inline void* memmove(void* dst, const void* src, size_t len)
{
	return _STLP_VENDOR_CSTD::__memmove(dst, src, len);
}
#   else

 using _STLP_VENDOR_CSTD::memmove;
 using _STLP_VENDOR_CSTD::memcpy;

#   endif

# if ! defined (__BORLANDC__)
using _STLP_VENDOR_CSTD::memchr;
using _STLP_VENDOR_CSTD::strchr;
using _STLP_VENDOR_CSTD::strpbrk;
using _STLP_VENDOR_CSTD::strrchr;
using _STLP_VENDOR_CSTD::strstr;
# endif

using _STLP_VENDOR_CSTD::memcmp;
using _STLP_VENDOR_CSTD::memset;

using _STLP_VENDOR_CSTD::strcat;

# if !defined (strcmp)
using _STLP_VENDOR_CSTD::strcmp;
# endif

using _STLP_VENDOR_CSTD::strcoll;
# if !defined (strcpy)
using _STLP_VENDOR_CSTD::strcpy;
# endif
using _STLP_VENDOR_CSTD::strcspn;
using _STLP_VENDOR_CSTD::strerror;
using _STLP_VENDOR_CSTD::strlen;
using _STLP_VENDOR_CSTD::strncat;
using _STLP_VENDOR_CSTD::strncmp;

using _STLP_VENDOR_CSTD::strncpy;
using _STLP_VENDOR_CSTD::strspn;

using _STLP_VENDOR_CSTD::strtok;
using _STLP_VENDOR_CSTD::strxfrm;
#  endif /* _STLP_NO_CSTD_FUNCTION_IMPORTS */
