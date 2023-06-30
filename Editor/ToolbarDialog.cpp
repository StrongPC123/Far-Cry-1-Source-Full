// ToolbarDialog.cpp: implementation of the CToolbarDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ToolbarDialog.h"

#include <afxpriv.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CToolbarDialog,CDialog)

BEGIN_MESSAGE_MAP(CToolbarDialog, CDialog)
	//{{AFX_MSG_MAP(CTerrainDialog)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
	ON_WM_DESTROY()
	ON_WM_MENUSELECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CToolbarDialog::CToolbarDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
	// TODO: Set IDD of CDialog
{ CDialog(lpszTemplateName, pParentWnd); }

CToolbarDialog::CToolbarDialog(UINT nIDTemplate, CWnd* pParentWnd)
	: CDialog(nIDTemplate, pParentWnd)
{ CDialog(nIDTemplate, pParentWnd); }

CToolbarDialog::~CToolbarDialog()
{

}

BOOL CToolbarDialog::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	////////////////////////////////////////////////////////////////////////
	// Handle tooltip text requests from the toolbars
	////////////////////////////////////////////////////////////////////////

	ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

	// Allow top level routing frame to handle the message
	// if (GetRoutingFrame() != NULL)
	// 	 return FALSE;
		
	// Need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA *pTTTA = (TOOLTIPTEXTA *) pNMHDR;
	TOOLTIPTEXTW *pTTTW = (TOOLTIPTEXTW *) pNMHDR;
	TCHAR szFullText[256];
	CString cstTipText;
	CString cstStatusText;

	UINT_PTR nID = pNMHDR->idFrom;
	if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		nID = ((UINT_PTR) (WORD)::GetDlgCtrlID((HWND) nID));
	}

	if (nID != 0) // will be zero on a separator
	{
		AfxLoadString(nID, szFullText);
		// this is the command id, not the button index
		AfxExtractSubString(cstTipText, szFullText, 1, '\n');
		AfxExtractSubString(cstStatusText, szFullText, 0, '\n');
	}

	// Non-UNICODE Strings only are shown in the tooltip window...
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, cstTipText,
            (sizeof(pTTTA->szText) / sizeof(pTTTA->szText[0])));
	else
		;/*
		_mbstowcsz(pTTTW->szText, cstTipText,
            (sizeof(pTTTW->szText) / sizeof(pTTTW->szText[0])));
						*/
	*pResult = 0;

	// Bring the tooltip window above other popup windows
	::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
		SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	
	// Display the tooltip in the status bar of the main frame window
	GetIEditor()->SetStatusText(cstStatusText);

	return TRUE;    // Message was handled
}

BOOL CToolbarDialog::OnInitDialog() 
{
	RecalcLayout();

	return TRUE;  // Return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CToolbarDialog::RecalcLayout()
{
	////////////////////////////////////////////////////////////////////////
	// Place the toolbars and move the controls in the dialog to make space
	// for them
	////////////////////////////////////////////////////////////////////////

	CRect rcClientStart;
	CRect rcClientNow;
	CRect rcChild;					
	CRect rcWindow;
	CWnd *pwndChild = GetWindow(GW_CHILD);
	if (pwndChild == NULL)
		return;

	CRect clientRect;
	GetClientRect(clientRect);
	// We need to resize the dialog to make room for control bars.
	// First, figure out how big the control bars are.
	GetClientRect(rcClientStart);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 
				   0, reposQuery, rcClientNow);

	// Now move all the controls so they are in the same relative
	// position within the remaining client area as they would be
	// with no control bars.
	CPoint ptOffset(rcClientNow.left - rcClientStart.left,
					rcClientNow.top - rcClientStart.top); 

	while (pwndChild)
	{                               
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		rcChild.IntersectRect( rcChild,clientRect );
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}

	/*
	// Adjust the dialog window dimensions
	GetWindowRect(rcWindow);
	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	MoveWindow(rcWindow, FALSE);
	*/

	// And position the control bars
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}

void CToolbarDialog::OnDestroy()
{
	////////////////////////////////////////////////////////////////////////
	// Set the status bar text back to "Ready"
	////////////////////////////////////////////////////////////////////////

	// TODO: Crashed in static object dialog 
	// GetIEditor()->SetStatusText("Ready");
}

void CToolbarDialog::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu)
{
	////////////////////////////////////////////////////////////////////////
	// Set the menu help text in the status bar of the main frame
	////////////////////////////////////////////////////////////////////////

	CDialog::OnMenuSelect(nItemID, nFlags, hSysMenu);
	
	TCHAR szFullText[256];
	CString cstStatusText;

	// TODO: Add your message handler code here

	// Displays in the mainframe's status bar
	if (nItemID != 0) // Will be zero on a separator
	{
		AfxLoadString(nItemID, szFullText);

		// This is the command id, not the button index
		AfxExtractSubString(cstStatusText, szFullText, 0, '\n');
		GetIEditor()->SetStatusText(cstStatusText);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CToolbarDialog::OnInitMenuPopup
//		OnInitMenuPopup updates the state of items on a popup menu.  
//
//  	This code is based on CFrameWnd::OnInitMenuPopup.  We assume the
//		application does not support context sensitive help.

void CToolbarDialog::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
	if (!bSysMenu)
	{
		ASSERT(pPopupMenu != NULL);
		
		// check the enabled state of various menu items
		CCmdUI state;        
		state.m_pMenu = pPopupMenu;
		ASSERT(state.m_pOther == NULL);
		
		state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
		for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
			 state.m_nIndex++)
		{
			state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
			if (state.m_nID == 0)
				continue; // menu separator or invalid cmd - ignore it
				
			ASSERT(state.m_pOther == NULL);
			ASSERT(state.m_pMenu != NULL);
			if (state.m_nID == (UINT)-1)
			{
				// possibly a popup menu, route to first item of that popup
				state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
				if (state.m_pSubMenu == NULL ||
				    (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				    state.m_nID == (UINT)-1)
				{				    			 
			        continue; // first item of popup can't be routed to
			    }
			    state.DoUpdate(this, FALSE);  // popups are never auto disabled
			}
			else
			{
				// normal menu item
				// Auto enable/disable if command is _not_ a system command
				state.m_pSubMenu = NULL;
				state.DoUpdate(this, state.m_nID < 0xF000);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CToolbarDialog::OnEnterIdle
//		OnEnterIdle updates the status bar when there's nothing better to do.
//  	This code is based on CFrameWnd::OnEnterIdle.

void CToolbarDialog::OnEnterIdle(UINT nWhy, CWnd* pWho) 
{
	/*
	if (nWhy != MSGF_MENU || m_nIDTracking == m_nIDLastMessage)
		return;
		
	OnSetMessageString(m_nIDTracking);
	ASSERT(m_nIDTracking == m_nIDLastMessage);		
	*/
}
