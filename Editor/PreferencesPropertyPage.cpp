////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PreferencesPropertyPage.cpp
//  Version:     v1.00
//  Created:     28/10/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "PreferencesPropertyPage.h"

IMPLEMENT_DYNAMIC(CPreferencesPropertyPage,CWnd)

//////////////////////////////////////////////////////////////////////////
// CPreferencesPropertyPageClassDesc implementation.
//////////////////////////////////////////////////////////////////////////
IPreferencesPage* CPreferencesPropertyPageClassDesc::CreatePage( const CRect &rc,CWnd *pParentWnd )
{
	CPreferencesPropertyPage *pPage = new CPreferencesPropertyPage();
	if (pPage->Create( rc,pParentWnd ) != TRUE)
		return 0;
	return pPage;
}

//////////////////////////////////////////////////////////////////////////
// CPreferencesPropertyPage implementation.
//////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CPreferencesPropertyPage, CWnd)
	ON_WM_SIZE()
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CPreferencesPropertyPage::CPreferencesPropertyPage()
{
	m_pVars = new CVarBlock;
}

//////////////////////////////////////////////////////////////////////////
BOOL CPreferencesPropertyPage::Create( const CRect &rc,CWnd *pParentWnd )
{
	LPCTSTR pszCreateClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW,::LoadCursor(NULL,IDC_ARROW));
	
	if (!CreateEx(0, pszCreateClass, _T(""), WS_TABSTOP|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_CHILD|WS_GROUP, rc, pParentWnd,0 ))
		return FALSE;

	// Create Hidden.
	ShowWindow( SW_HIDE );

	CRect rcp( 0,0,rc.Width(),rc.Height() );
	m_wndProps.Create( WS_CHILD|WS_VISIBLE|WS_BORDER,rcp,this );

	m_wndProps.SetFlags( CPropertyCtrl::F_VS_DOT_NET_STYLE );
	m_wndProps.SetItemHeight( 15 );
	m_wndProps.AddVarBlock( m_pVars );
	m_wndProps.ExpandAll();

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesPropertyPage::AddVariable( CVariableBase &var,const char *varName,unsigned char dataType )
{
	var.AddRef(); // Variables are local and must not be released by CVarBlock.
	if (varName)
		var.SetName(varName);
	var.SetDataType(dataType);
	m_pVars->AddVariable( &var );
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesPropertyPage::AddVariable( CVariableArray &table,CVariableBase &var,const char *varName,unsigned char dataType )
{
	var.AddRef(); // Variables are local and must not be released by CVarBlock.
	if (varName)
		var.SetName(varName);
	var.SetDataType(dataType);
	table.AddChildVar( &var );
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesPropertyPage::OnApply()
{
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesPropertyPage::OnCancel()
{
}

//////////////////////////////////////////////////////////////////////////
bool CPreferencesPropertyPage::OnQueryCancel()
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesPropertyPage::OnSetActive( bool bActive )
{
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesPropertyPage::OnOK()
{
	OnApply();
}

//////////////////////////////////////////////////////////////////////////
void CPreferencesPropertyPage::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (!m_wndProps.GetSafeHwnd())
		return;

	//CRect rc;
	//GetClientRect( rc );

	//m_wndProps.MoveWindow( rc );
	//m_wndProps.Invalidate();
	m_wndProps.SetWindowPos( NULL,0,0,cx,cy,SWP_NOMOVE );
}
