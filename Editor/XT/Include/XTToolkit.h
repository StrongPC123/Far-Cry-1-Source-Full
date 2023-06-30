// XTToolkit.h : header file
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#if !defined(__XTTOOLKIT_H__)
#define __XTTOOLKIT_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __XTINCLUDES_H__
#include "XTIncludes.h"
#endif // __XTINCLUDES_H__

//------------------------------------------------------
// Xtreme link to the appropriate dll or static library:
//------------------------------------------------------

#if defined( _AFXDLL ) && defined( _XT_STATICLINK )
	#if defined( _DEBUG )
		#if defined( _UNICODE )
            #define _XTLIB_FILE_SUFFIX "LibDynStaticud"
		#else
            #define _XTLIB_FILE_SUFFIX "LibDynStaticd"
		#endif // _UNICODE
	#else
		#if defined( _UNICODE )
            #define _XTLIB_FILE_SUFFIX "LibDynStaticu"
		#else
            #define _XTLIB_FILE_SUFFIX "LibDynStatic"
		#endif // _UNICODE
	#endif // _DEBUG
    #define _XTLIB_LINK_TYPE   "lib"
    #define _XTLIB_LINK_IS_DLL 0
#elif !defined( _AFXDLL )
	#if defined( _DEBUG )
		#if defined( _UNICODE )
            #define _XTLIB_FILE_SUFFIX "LibStaticud"
		#else
            #define _XTLIB_FILE_SUFFIX "LibStaticd"
		#endif // _UNICODE
	#else
		#if defined( _UNICODE )
            #define _XTLIB_FILE_SUFFIX "LibStaticu"
		#else
            #define _XTLIB_FILE_SUFFIX "LibStatic"
		#endif // _UNICODE
	#endif // _DEBUG
    #define _XTLIB_LINK_TYPE   "lib"
    #define _XTLIB_LINK_IS_DLL 0
#else
	#if defined( _DEBUG )
		#if defined( _UNICODE )
            #define _XTLIB_FILE_SUFFIX "Libud"
		#else
            #define _XTLIB_FILE_SUFFIX "Libd"
		#endif // _UNICODE
	#else
		#if defined( _UNICODE )
            #define _XTLIB_FILE_SUFFIX "Libu"
		#else
            #define _XTLIB_FILE_SUFFIX "Lib"
		#endif // _UNICODE
	#endif // _DEBUG
    #define _XTLIB_LINK_TYPE   "dll"
    #define _XTLIB_LINK_IS_DLL 1
#endif // !defined( _AFXDLL ) || defined( _XT_STATICLINK )

#if (_XTLIB_LINK_IS_DLL == 0)  &&  defined(_XT_DEMOMODE)
	#pragma message(" ")
	#pragma message("----------------------------------------------------------------------------------------------")
	#pragma message(" The evaluation version of the toolkit only supports DLL configurations.")
	#pragma message(" To purchase the full version (with static link support) please visit http://www.codejock.com")
	#pragma message("----------------------------------------------------------------------------------------------")
	#pragma message(" ")
	#error This build configuration is not supported by the evaluation library
#endif

#if defined(_XT_DEMOMODE)
	#define _XTLIB_FILE_PREFIX_FULL _XTLIB_FILE_PREFIX "Eval"
#else
	#define _XTLIB_FILE_PREFIX_FULL _XTLIB_FILE_PREFIX
#endif

#ifndef _XTLIB_NOAUTOLINK
#if defined( _XTLIB_LINK_TYPE )  &&  defined ( _XTLIB_FILE_SUFFIX )

#ifdef WIN64
		#pragma comment(lib, _XTLIB_FILE_PREFIX_FULL _XTLIB_FILE_SUFFIX "64.lib") 
#else
#if _MFC_VER == 0x0710 // MFC7.1
		#pragma comment(lib, _XTLIB_FILE_PREFIX_FULL _XTLIB_FILE_SUFFIX "_2003.lib") 
#else
    #pragma comment(lib, _XTLIB_FILE_PREFIX_FULL _XTLIB_FILE_SUFFIX ".lib") 
#endif // MFC7.1
#endif
	//#pragma message("Automatically linking with " _XTLIB_FILE_PREFIX_FULL _XTLIB_FILE_SUFFIX "." _XTLIB_LINK_TYPE)
#endif //_XTLIB_LINK_TYPE && _XTLIB_FILE_SUFFIX
#endif //_XTLIB_NOAUTOLINK

/////////////////////////////////////////////////////////////////////////////

#endif // #if !defined(__XTTOOLKIT_H__)
