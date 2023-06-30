////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2004.
// -------------------------------------------------------------------------
//  File name:   Linux32Specific.h
//  Version:     v1.00
//  Created:     05/03/2004 by MarcoK.
//  Compilers:   Visual Studio.NET, GCC 3.2
//  Description: Specific to Linux declarations, inline functions etc.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_COMMON_LINUX64_SPECIFIC_HDR_
#define _CRY_COMMON_LINUX64_SPECIFIC_HDR_

#include "LinuxSpecific.h"
#include </usr/include/stdint.h>

typedef signed char         int8;
typedef signed short        int16;
typedef signed int					int32;
typedef signed long long		int64;
typedef unsigned char				uint8;
typedef unsigned short			uint16;
typedef unsigned int				uint32;
typedef unsigned long long	uint64;

typedef float               f32;
typedef double              f64;

// old-style (will be removed soon)
typedef signed char         s8;
typedef signed short        s16;
typedef signed int	        s32;
typedef signed long long		s64;
typedef unsigned char				u8;
typedef unsigned short			u16;
typedef unsigned int				u32;
typedef unsigned long long	u64;


typedef uint64							DWORD_PTR;
typedef intptr_t INT_PTR, *PINT_PTR;
typedef uintptr_t UINT_PTR, *PUINT_PTR;
typedef char *LPSTR, *PSTR;
typedef unsigned long long __uint64;
typedef signed long long __int64;

typedef signed long long		INT64;
typedef long long						UINT64;

typedef long LONG_PTR, *PLONG_PTR, *PLONG;
typedef unsigned long ULONG_PTR, *PULONG_PTR;

typedef unsigned char				BYTE;
typedef unsigned short			WORD;
typedef void*								HWND;
typedef UINT_PTR 						WPARAM;
typedef LONG_PTR 						LPARAM;
typedef LONG_PTR 						LRESULT;
#define PLARGE_INTEGER LARGE_INTEGER*
typedef const char *LPCSTR, *PCSTR;
typedef long long						LONGLONG;
typedef	ULONG_PTR						SIZE_T;
typedef unsigned char				byte;

#endif //_CRY_COMMON_LINUX64_SPECIFIC_HDR_
