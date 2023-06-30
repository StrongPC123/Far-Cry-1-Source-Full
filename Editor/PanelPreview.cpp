////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   panelpreview.cpp
//  Version:     v1.00
//  Created:     29/3/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PanelPreview.h"

// CPanelPreview dialog

IMPLEMENT_DYNAMIC(CPanelPreview, CXTResizeDialog)
CPanelPreview::CPanelPreview(CWnd* pParent /*=NULL*/)
	: CXTResizeDialog(CPanelPreview::IDD, pParent)
{
	Create( IDD,pParent );
}

CPanelPreview::~CPanelPreview()
{
}

void CPanelPreview::DoDataExchange(CDataExchange* pDX)
{
	CXTResizeDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPanelPreview, CXTResizeDialog)
END_MESSAGE_MAP()


// CPanelPreview message handlers
BOOL CPanelPreview::OnInitDialog()
{
	CXTResizeDialog::OnInitDialog();

	CRect rc;
	GetDlgItem(IDC_PREVIEW)->GetWindowRect(rc);
	ScreenToClient( rc );
	GetDlgItem(IDC_PREVIEW)->ShowWindow(SW_HIDE);
	
	m_previewCtrl.Create( this,rc,WS_VISIBLE|WS_CHILD|WS_BORDER );
	m_previewCtrl.EnableWindow(TRUE);
	SetResize( &m_previewCtrl,SZ_RESIZE(1),rc );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CPanelPreview::LoadFile( const CString &filename )
{
	if (!filename.IsEmpty())
		m_previewCtrl.LoadFile( filename,false );
}