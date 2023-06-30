// SelectSoundPresetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectEAXPresetDlg.h"
#include "EAXPresetMgr.h"

// CSelectSoundPresetDlg dialog

IMPLEMENT_DYNAMIC(CSelectEAXPresetDlg, CDialog)
CSelectEAXPresetDlg::CSelectEAXPresetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectEAXPresetDlg::IDD, pParent)
{
	m_pEAXPresetMgr=GetIEditor()->GetEAXPresetMgr();
	m_sCurrPreset="";
}

CSelectEAXPresetDlg::~CSelectEAXPresetDlg()
{
}

void CSelectEAXPresetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRESETS, m_wndPresets);
}


BEGIN_MESSAGE_MAP(CSelectEAXPresetDlg, CDialog)
	ON_LBN_SELCHANGE(IDC_PRESETS, OnLbnSelchangePresets)
END_MESSAGE_MAP()


// CSelectSoundPresetDlg message handlers

BOOL CSelectEAXPresetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_wndPresets.ResetContent();
	XmlNodeRef pRootNode=m_pEAXPresetMgr->GetRootNode();
	CString sName;
	for (int i=0;i<pRootNode->getChildCount();i++)
	{
		XmlNodeRef pPresetNode=pRootNode->getChild(i);
		int nIdx=m_wndPresets.AddString(pPresetNode->getTag());
		if (m_sCurrPreset==CString(pPresetNode->getTag()))
			m_wndPresets.SetCurSel(nIdx);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectEAXPresetDlg::OnLbnSelchangePresets()
{
	int nIdx=m_wndPresets.GetCurSel();
	if (nIdx<0)
		m_sCurrPreset="";
	else
		m_wndPresets.GetText(nIdx, m_sCurrPreset);
}
