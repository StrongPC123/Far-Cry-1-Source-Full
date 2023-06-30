/******************************************************************************************
TINY WINDOWS LIBRARY
copyright by Alberto Demichelis 2001 
email: albertodemichelis@hotmail.com
right now this code is not GPL or LGPL in any way
******************************************************************************************/

#ifndef __TINY_MAIN_H__
#define __TINY_MAIN_H__

#pragma once
#include "CryLibrary.h"

#include <commctrl.h>
#pragma comment (lib , "comctl32.lib")

///////////////////////////////////////////////////////////////////////////////////////////
// Common includes
///////////////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////
// Helper macros
///////////////////////////////////////////////////////////////////////////////////////////
#define _TINY_SIGNED_LOWORD(l) ((int16)((int32)(l) & 0xffff))
#define _TINY_SIGNED_HIWORD(l) ((int16)((int32)(l) >> 16))
#define _T

///////////////////////////////////////////////////////////////////////////////////////////
// Debug functions / macros
///////////////////////////////////////////////////////////////////////////////////////////
#if defined(WIN64) || defined(LINUX64)
#define _TinyVerify(x) { if (!(x)) assert(0); }
#else
#define _TinyVerify(x) { if (!(x)) { DEBUG_BREAK; }; }
#endif

#if defined(_DEBUG) && !defined(WIN64) && !defined(LINUX64)
	#define _TinyAssert(x) { if (!(x)) { DEBUG_BREAK; }; }
#else
	#define _TinyAssert(x) __noop(x);
#endif

__inline void __cdecl _TinyTrace(const char *sFormat, ...)
{
	va_list vl;
	static char sTraceString[1024];
	
	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);

	strcat(sTraceString, "\n");

	::OutputDebugString(sTraceString);	
}

#define _TINY_CHECK_LAST_ERROR _TinyCheckLastError(__FILE__, __LINE__);
__inline void _TinyCheckLastError(const char *pszFile, int iLine)
{
	if (GetLastError() != ERROR_SUCCESS)
	{
		// Format an error message
		char szMessageBuf[2048];
		char szLineFileInfo[_MAX_PATH + 256];
		FormatMessage( 
			FORMAT_MESSAGE_ARGUMENT_ARRAY |
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			szMessageBuf,
			2048,
			NULL 
		);
		sprintf(szLineFileInfo, "Error catched in file %s line %i", pszFile, iLine);
		strcat(szMessageBuf, szLineFileInfo);

#ifdef _DEBUG
		MessageBox(NULL, szMessageBuf, "Tiny Framework Error", MB_OK | MB_ICONERROR);
#else
		_TinyTrace(szMessageBuf);
#endif

		// Error processed
		SetLastError(ERROR_SUCCESS);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
// Gobal variables and acessors
///////////////////////////////////////////////////////////////////////////////////////////
#define _TINY_DECLARE_APP()	\
	LPTSTR g_lpCmdLine;	\
	HINSTANCE g_hResourceInstance;

extern LPTSTR g_lpCmdLine;	
extern HINSTANCE g_hResourceInstance;	

inline HINSTANCE _Tiny_GetInstance()
{
	return (HINSTANCE) GetModuleHandle(NULL);
}

inline HINSTANCE _Tiny_GetResourceInstance()
{
	return g_hResourceInstance;
}

inline LPCTSTR _Tiny_GetCommandLine()
{
	return g_lpCmdLine;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Global structures
///////////////////////////////////////////////////////////////////////////////////////////
class _TinyRect: public RECT{
public:
	_TinyRect(){
		left=0;
		right=0;
		top=0;
		bottom=0;
	}
	_TinyRect(RECT &rect){
		left=rect.left;
		right=rect.right;
		top=rect.top;
		bottom=rect.bottom;
	}
	_TinyRect(int w,int h){
		left=0;
		right=w;
		top=0;
		bottom=h;
	}
	_TinyRect(int x,int y,int w,int h){
		left=x;
		right=x+w;
		top=y;
		bottom=y+h;
	}
	
};

///////////////////////////////////////////////////////////////////////////////////////////
// Main window include
///////////////////////////////////////////////////////////////////////////////////////////
#include "_TinyWindow.h"

///////////////////////////////////////////////////////////////////////////////////////////
// Inititalization
///////////////////////////////////////////////////////////////////////////////////////////
inline BOOL _Tiny_InitApp(HINSTANCE hInstance,HINSTANCE hResourceInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,DWORD nIcon=0)
{
	SetLastError(ERROR_SUCCESS);
	g_lpCmdLine=lpCmdLine;
	g_hResourceInstance=hResourceInstance;
	INITCOMMONCONTROLSEX icc;
	icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC=ICC_TREEVIEW_CLASSES |ICC_BAR_CLASSES |ICC_LISTVIEW_CLASSES|ICC_COOL_CLASSES|ICC_WIN95_CLASSES ;
	CryLoadLibrary("Riched20.dll");
	::InitCommonControlsEx(&icc);
	BOOL bRet = __RegisterSmartClass(hInstance,nIcon) ? TRUE : FALSE;
	return bRet;
}

/*
 if (!TranslateAccelerator( 
                hwndMain,      // handle to receiving window 
                haccel,        // handle to active accelerator table 
                &msg))         // message data 
        {
 */
inline int _Tiny_MainLoop(HACCEL hAccelTable = NULL, HWND hAccelTarget = NULL)
{
	MSG msg;

	//_TinyAssert((hAccelTable == NULL && hAccelTarget == NULL) || 
	//	(hAccelTable != NULL && hAccelTarget != NULL));

	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (hAccelTable == NULL || TranslateAccelerator(hAccelTarget, hAccelTable, &msg) == 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

#endif