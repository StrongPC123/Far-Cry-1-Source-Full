// LoadingScreen.cpp: implementation of the CLoadingScreen class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
/*
#include "stdafx.h"
#include "LoadingScreen.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Static member variables
CLoadingDialog CLoadingScreen::m_cLoadingDialog;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoadingScreen::CLoadingScreen()
{
	
}

CLoadingScreen::~CLoadingScreen()
{
	// Hide the screen
	CLoadingScreen::Hide();
}

void CLoadingScreen::Show()
{
	////////////////////////////////////////////////////////////////////////
	// Show the loading screen and register it in CLogFile
	////////////////////////////////////////////////////////////////////////

	// Display the modelless loading dialog
	VERIFY(m_cLoadingDialog.Create(IDD_LOADING));
	m_cLoadingDialog.ShowWindow(SW_SHOWNORMAL);
	m_cLoadingDialog.UpdateWindow();

	// Register the listbox control for receiving the log file entries
	CLogFile::AttachListBox(m_cLoadingDialog.GetDlgItem(IDC_CONSOLE_OUTPUT)->m_hWnd);
 
	::SetWindowPos(m_cLoadingDialog.m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

void CLoadingScreen::Hide()
{
	////////////////////////////////////////////////////////////////////////
	// Hide the loading screen and register it in CLogFile
	////////////////////////////////////////////////////////////////////////

	// Unregister the listbox control
	CLogFile::AttachListBox(NULL);

	// Destroy the dialog window
	m_cLoadingDialog.DestroyWindow();
}
*/