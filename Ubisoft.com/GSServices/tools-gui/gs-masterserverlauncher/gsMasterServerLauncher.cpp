// gsMasterServerLauncher.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "gsMasterServerLauncher.h"
#include "gsMasterServerLauncherDlg.h"

#include "GSTypes.h"
#include "InitSockets.h"
#include "define.h"


//UBI.COM: These functions are called by the GSlibraries to allocate/free memory
extern "C"{
void * __stdcall ExtAlloc_Malloc(int iSize)
{
	return malloc(iSize);
}

void __stdcall ExtAlloc_Free(void * ptr)
{
	free(ptr);
}
void * __stdcall ExtAlloc_Realloc(void * ptr,int iSize)
{
	return realloc( ptr, iSize);
}

}

//UBI.COM: The GSLibraries are not thread safe.  This mutex will stop us from 
//         calling into the library twice at the same time
HANDLE g_Mutex;



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerLauncherApp

BEGIN_MESSAGE_MAP(CGsMasterServerLauncherApp, CWinApp)
	//{{AFX_MSG_MAP(CGsMasterServerLauncherApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerLauncherApp construction

CGsMasterServerLauncherApp::CGsMasterServerLauncherApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGsMasterServerLauncherApp object

CGsMasterServerLauncherApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerLauncherApp initialization

BOOL CGsMasterServerLauncherApp::InitInstance()
{
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif


	//UBI.COM: Initialize the Socket library.  InitializeSockets will return the 
	//         local IPAddress that is found.
	GSchar szIPAddress[IPADDRESSLENGTH]="";

	g_Mutex = CreateMutex(NULL,false,NULL);
	LOCKMUTEX();
	InitializeSockets(szIPAddress);
	UNLOCKMUTEX();

	CGsMasterServerLauncherDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();

	//UBI.COM: Uninitialize the socket library and exit the program
	LOCKMUTEX();
	UninitializeSockets();
	UNLOCKMUTEX();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
