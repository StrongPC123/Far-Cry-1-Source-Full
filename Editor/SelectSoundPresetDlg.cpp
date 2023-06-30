// SelectSoundPresetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SelectSoundPresetDlg.h"
#include "SoundPresetMgr.h"

// CSelectSoundPresetDlg dialog

IMPLEMENT_DYNAMIC(CSelectSoundPresetDlg, CDialog)
CSelectSoundPresetDlg::CSelectSoundPresetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectSoundPresetDlg::IDD, pParent)
{
	m_pSoundPresetMgr=GetIEditor()->GetSoundPresetMgr();
	m_sCurrPreset="";
}

CSelectSoundPresetDlg::~CSelectSoundPresetDlg()
{
}

void CSelectSoundPresetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRESETS, m_wndPresets);
}


BEGIN_MESSAGE_MAP(CSelectSoundPresetDlg, CDialog)
	ON_LBN_SELCHANGE(IDC_PRESETS, OnLbnSelchangePresets)
END_MESSAGE_MAP()


// CSelectSoundPresetDlg message handlers

BOOL CSelectSoundPresetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_wndPresets.ResetContent();
	XmlNodeRef pRootNode=m_pSoundPresetMgr->GetRootNode();
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

void CSelectSoundPresetDlg::OnLbnSelchangePresets()
{
	int nIdx=m_wndPresets.GetCurSel();
	if (nIdx<0)
		m_sCurrPreset="";
	else
		m_wndPresets.GetText(nIdx, m_sCurrPreset);
}
