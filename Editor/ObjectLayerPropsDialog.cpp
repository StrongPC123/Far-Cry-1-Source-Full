////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objectlayerpropsdialog.cpp
//  Version:     v1.00
//  Created:     26/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ObjectLayerPropsDialog.h"


// CObjectLayerPropsDialog dialog

IMPLEMENT_DYNAMIC(CObjectLayerPropsDialog, CDialog)
CObjectLayerPropsDialog::CObjectLayerPropsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectLayerPropsDialog::IDD, pParent)
	, m_bVisible(TRUE)
	, m_bFrozen(FALSE)
	, m_bExternal(FALSE)
	, m_bExportToGame(TRUE)
	, m_name(_T(""))
{
	m_bMainLayer = FALSE;
}

CObjectLayerPropsDialog::~CObjectLayerPropsDialog()
{
}

void CObjectLayerPropsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_VISIBLE, m_bVisible);
	DDX_Check(pDX, IDC_FROZEN, m_bFrozen);
	DDX_Check(pDX, IDC_EXTERNAL, m_bExternal);
	DDX_Check(pDX, IDC_EXPORT, m_bExportToGame);
	DDX_Text(pDX, IDC_STRING, m_name);
}


BEGIN_MESSAGE_MAP(CObjectLayerPropsDialog, CDialog)
END_MESSAGE_MAP()


// CObjectLayerPropsDialog message handlers

BOOL CObjectLayerPropsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_bMainLayer)
	{
		GetDlgItem(IDC_EXTERNAL)->EnableWindow(FALSE);
	}
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
