////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   waitprogress.cpp
//  Version:     v1.00
//  Created:     10/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "waitprogress.h"

#ifdef _DEBUG
#undef THIS_FILE
#define new DEBUG_NEW
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

bool CWaitProgress::m_bInProgressNow = false;
bool CWaitProgress::m_bCancel = false;

CWaitProgress::CWaitProgress(UINT nIDText, bool bStart)
:	m_bStarted(false)
,	m_hwndProgress(NULL)
{
	char szText[1024];
	VERIFY(LoadString( AfxGetInstanceHandle(),AFX_IDS_IDLEMESSAGE,szText,sizeof(szText) ));

	m_bCancel = false;

	m_strText = szText;
	m_bIgnore = false;
	if (bStart)
		Start();
}

CWaitProgress::CWaitProgress(LPCTSTR lpszText, bool bStart)
:	m_strText(lpszText)
,	m_bStarted(false)
,	m_hwndProgress(NULL)
{
	m_bCancel = false;
	m_bIgnore = false;
	if (bStart)
		Start();
}

CWaitProgress::~CWaitProgress()
{
	if (m_bStarted)
		Stop();
}

void CWaitProgress::Start()
{
	if (m_bStarted)
		Stop();

	if (m_bInProgressNow)
	{
		// Do not affect previously started progress bar.
		m_bIgnore = true;
		m_bStarted = false;
		return;
	}

	// display text in the status bar
	GetIEditor()->SetStatusText( m_strText );

	// switch on wait cursor
	::AfxGetApp()->BeginWaitCursor();

	m_bStarted = true;
	m_percent = 0;
}

void CWaitProgress::Stop()
{
	if (!m_bStarted)
		return;

	if (m_hwndProgress)
	{
		// clean up and destroy progress bar
		CStatusBar* pStatusBar = DYNAMIC_DOWNCAST(CStatusBar, CWnd::FromHandle(::GetParent(m_hwndProgress)));
		ASSERT_VALID(pStatusBar);

		::DestroyWindow(m_hwndProgress);
		m_hwndProgress = NULL;

		m_cancelButton.DestroyWindow();

		// remove progress bar pane
		int anPart[32];
		int nParts = pStatusBar->GetStatusBarCtrl().GetParts(31, anPart);
		nParts--;
		pStatusBar->GetStatusBarCtrl().SetParts(nParts, anPart+1);
	}

	// switch back to standard text in the status bar
	char szIdle[1024];
	LoadString( AfxGetInstanceHandle(),AFX_IDS_IDLEMESSAGE,szIdle,sizeof(szIdle) );
	GetIEditor()->SetStatusText( szIdle );

	// switch off wait cursor
	::AfxGetApp()->EndWaitCursor();

	m_bInProgressNow = false;

	m_bStarted = false;
}

bool CWaitProgress::Step(int nPercentage)
{
	if (m_bIgnore)
		return true;

	if (m_bCancel)
		return false;

	if (!m_bStarted)
		Start();

	if (m_percent == nPercentage)
		return true;
	
	m_percent = nPercentage;

	::AfxGetApp()->RestoreWaitCursor();

	if (nPercentage >= 0)
	{
		ASSERT(nPercentage <= 100);
		// create or update a progress control in the status bar
		if (m_hwndProgress == NULL)
			CreateProgressControl();

		if (m_hwndProgress)
			::SendMessage(m_hwndProgress, PBM_SETPOS, (WPARAM)nPercentage, 0);
	}

	// Use the oportunity to process windows messages here.
	MSG msg;
	while( FALSE != ::PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
	{ 
		::TranslateMessage( &msg );
		::DispatchMessage( &msg );
	}
	return true;
}

void CWaitProgress::SetText(LPCTSTR lpszText)
{
	if (m_bIgnore)
		return;
	m_strText = lpszText;
	GetIEditor()->SetStatusText( m_strText );
}

void CWaitProgress::CreateProgressControl()
{
	ASSERT(m_hwndProgress == NULL);

	// find status bar
	CWnd* pMainWnd = ::AfxGetMainWnd();
	if (pMainWnd == NULL)
		return;
	CStatusBar* pStatusBar = DYNAMIC_DOWNCAST(CStatusBar, 
		pMainWnd->GetDescendantWindow(AFX_IDW_STATUS_BAR, TRUE));
	if (pStatusBar == NULL || pStatusBar->m_hWnd == NULL)
		return;

	CRect rc; // this will be the location for the progress bar pane
	pStatusBar->GetItemRect(0, rc);
	if (!m_strText.IsEmpty())
	{
		// adjust so that the text in the leftmost pane will not be covered
		CClientDC dc(pStatusBar);
		dc.SelectObject(pStatusBar->GetFont());
		CSize sz = dc.GetTextExtent(m_strText);
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		rc.left += sz.cx + 2*tm.tmAveCharWidth;
	}
	int cx = rc.Width();
	if (cx < 20)
	{
		// no sense in displaying such a small progress bar
		//assert("ProgressDisplay would be too small\n");
		return;
	}
	/*
	else if (cx > 500)
	{
		// arbitrarily limiting progress bar width to 500 pixel
		cx = 500;
		rc.left = rc.right - cx;
	}
	*/
	// Arbitary add 50 pixel to left of text.
	rc.left += 50;

	// add a pane between the text and the currently leftmost pane
	int anPart[32];
	int nParts = pStatusBar->GetStatusBarCtrl().GetParts(31, anPart+1);
	anPart[0] = rc.left;
	nParts++;
	pStatusBar->GetStatusBarCtrl().SetParts(nParts, anPart);
	pStatusBar->GetStatusBarCtrl().GetRect(1, rc);

	int btnWidth = 60;

	// create progress bar control
	m_hwndProgress = ::CreateWindow(PROGRESS_CLASS, "",
		WS_CHILD | WS_VISIBLE, rc.left+btnWidth, rc.top, rc.Width()-btnWidth, rc.Height(),
		pStatusBar->m_hWnd, (HMENU)1, AfxGetInstanceHandle(), NULL);

	// Specify Owner to be main window, to recieve cancel event.
	m_cancelButton.Create( "Cancel",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,CRect(rc.left,rc.top,rc.left+btnWidth-1,rc.bottom),AfxGetMainWnd(),(UINT)ID_PROGRESSBAR_CANCEL );
	m_cancelButton.SetParent( pStatusBar );

	pStatusBar->UpdateWindow();
	m_bInProgressNow = true;
}
