#ifndef __GSTYPES_H__
#define __GSTYPES_H__

#if defined ( GS_LINUX )

#include <stdlib.h>
// calling methods
#define __stdcall
#define __cdecl

// Special types
typedef unsigned char GSbool;
typedef void GSvoid;
typedef size_t GSsize_t;

// Signed types
typedef char GSbyte;
typedef char GSchar;
typedef short GSshort;
typedef int GSint;
typedef long long GSlong;
typedef float GSfloat;
typedef double GSdouble;

// Unsigned types
typedef unsigned char GSubyte;
typedef unsigned char GSuchar;
typedef unsigned short GSushort;
typedef unsigned int GSuint;
typedef unsigned long long GSulong;

#elif defined ( GS_WIN32 )

// Special types
typedef unsigned char GSbool;
typedef void GSvoid;
typedef size_t GSsize_t;

// Signed types
typedef char GSbyte;
typedef char GSchar;
typedef short GSshort;
typedef int GSint;
typedef __int64 GSlong;
typedef float GSfloat;
typedef double GSdouble;

// Unsigned types
typedef unsigned char GSubyte;
typedef unsigned char GSuchar;
typedef unsigned short GSushort;
typedef unsigned int GSuint;
typedef unsigned __int64 GSulong;

#elif defined ( GS_PSX2 )
#include <stdlib.h>
// calling methods
#define __stdcall
#define __cdecl

// Special types
typedef unsigned char GSbool;
typedef void GSvoid;
typedef size_t GSsize_t;

// Signed types
typedef char GSbyte;
typedef char GSchar;
typedef short GSshort;
typedef int GSint;
typedef long GSlong;
typedef float GSfloat;
typedef double GSdouble;

// Unsigned types
typedef unsigned char GSubyte;
typedef unsigned char GSuchar;
typedef unsigned short GSushort;
typedef unsigned int GSuint;
typedef unsigned long GSulong;

#elif defined ( GS_XBOX )

// Special types
typedef unsigned char GSbool;
typedef void GSvoid;
typedef size_t GSsize_t;

// Signed types
typedef char GSbyte;
typedef char GSchar;
typedef short GSshort;
typedef int GSint;
typedef __int64 GSlong;
typedef float GSfloat;
typedef double GSdouble;

// Unsigned types
typedef unsigned char GSubyte;
typedef unsigned char GSuchar;
typedef unsigned short GSushort;
typedef unsigned int GSuint;
typedef unsigned __int64 GSulong;

#elif defined ( GS_WIN64 )

// Special types
typedef unsigned char GSbool;
typedef void GSvoid;
typedef size_t GSsize_t;

// Signed types
typedef char GSbyte;
typedef char GSchar;
typedef short GSshort;
typedef int GSint;
typedef __int64 GSlong;
typedef float GSfloat;
typedef double GSdouble;

// Unsigned types
typedef unsigned char GSubyte;
typedef unsigned char GSuchar;
typedef unsigned short GSushort;
typedef unsigned int GSuint;
typedef unsigned __int64 GSulong;

#endif

// For GSbool
#define GS_TRUE                           1
#define GS_FALSE                          0

// Special type for instances identification
#ifdef GSvoid
typedef GSvoid* GShandle;
#else
typedef void* GShandle;
#endif

#endif // __GSTYPES_H__



