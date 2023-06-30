// gsMasterServerClient.h : main header file for the GSMASTERSERVERCLIENT application
//

#if !defined(AFX_GSMASTERSERVERCLIENT_H__105658B5_BC80_41F6_9297_FE8034DAE519__INCLUDED_)
#define AFX_GSMASTERSERVERCLIENT_H__105658B5_BC80_41F6_9297_FE8034DAE519__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include "GSTypes.h"

//UBI.COM: Macros to help with locking the Mutex defined in gsMasterServerClient.cpp
#define LOCKMUTEX() WaitForSingleObject(g_Mutex,INFINITE)
#define UNLOCKMUTEX() ReleaseMutex(g_Mutex)

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerClientApp:
// See gsMasterServerClient.cpp for the implementation of this class
//
class CGsMasterServerClientApp : public CWinApp
{
public:
	CGsMasterServerClientApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGsMasterServerClientApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CGsMasterServerClientApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


//UBI.COM: The callbacks to register.

GSvoid __stdcall MSGameServer(GSuint uiID,
		GSshort siGroupType,GSchar *szGroupName, GSint iConfig,
		GSchar *szMaster,GSchar *szAllowedGames,GSchar *szGames,
		GSchar *szGameVersion,GSchar *szGSVersion,GSvoid *vpInfo,GSint iSize,
		GSuint uiMaxPlayer,GSuint uiNbrPlayer,GSuint uiMaxVisitor,
		GSuint uiNbrVisitor,GSchar *szIPAddress,GSchar *szAltIPAddress,
		GSint iEventID);

GSvoid __stdcall MSError(GSint iReason);

GSvoid __stdcall MSInitFinished();

GSvoid __stdcall MSRequestFinished();

GSvoid __stdcall MSJoinFinished(GSuint uiID,GSvoid *vpGameData,GSint iSize,
			GSchar *szIPAddress,GSchar *szAltIPAddress,GSushort usPort);
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GSMASTERSERVERCLIENT_H__105658B5_BC80_41F6_9297_FE8034DAE519__INCLUDED_)
