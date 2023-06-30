// gsMasterServerLauncher.h : main header file for the GSMASTERSERVERLAUNCHER application
//

#if !defined(AFX_GSMASTERSERVERLAUNCHER_H__D07D2F27_AD98_4851_A9F7_AF54CD40648E__INCLUDED_)
#define AFX_GSMASTERSERVERLAUNCHER_H__D07D2F27_AD98_4851_A9F7_AF54CD40648E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "GSTypes.h"
//UBI.COM: Macros to help with locking the Mutex defined in gsMasterServerLauncher.cpp
#define LOCKMUTEX() WaitForSingleObject(g_Mutex,INFINITE)
#define UNLOCKMUTEX() ReleaseMutex(g_Mutex)

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerLauncherApp:
// See gsMasterServerLauncher.cpp for the implementation of this class
//

class CGsMasterServerLauncherApp : public CWinApp
{
public:
	CGsMasterServerLauncherApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGsMasterServerLauncherApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGsMasterServerLauncherApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GSMASTERSERVERLAUNCHER_H__D07D2F27_AD98_4851_A9F7_AF54CD40648E__INCLUDED_)
