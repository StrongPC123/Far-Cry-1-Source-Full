// SoundPresetsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EAXPresetsDlg.h"
#include "EAXPresetMgr.h"
#include "StringDlg.h"
#include "Controls\PropertyItem.h"

#define WM_UPDATEPROPERTIES	WM_APP+1

// CSoundPresetsDlg dialog

IMPLEMENT_DYNAMIC(CEAXPresetsDlg, CToolbarDialog)
CEAXPresetsDlg::CEAXPresetsDlg(CWnd* pParent /*=NULL*/)
	: CToolbarDialog(CEAXPresetsDlg::IDD, pParent)
{
	m_pEAXPresetMgr=NULL;
}

CEAXPresetsDlg::~CEAXPresetsDlg()
{
}

void CEAXPresetsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRESETS, m_wndPresets);
	DDX_Control(pDX, IDC_PARAMS, m_wndParams);
}


BEGIN_MESSAGE_MAP(CEAXPresetsDlg, CToolbarDialog)
	ON_LBN_SELCHANGE(IDC_PRESETS, OnLbnSelchangePresets)
	ON_COMMAND(ID_SAVE, OnSavePreset)
	ON_COMMAND(ID_ADDPRESET, OnAddPreset)
	ON_COMMAND(ID_DELPRESET, OnDelPreset)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_UPDATEPROPERTIES, OnUpdateProperties)
END_MESSAGE_MAP()

BOOL CEAXPresetsDlg::OnInitDialog()
{
	m_pEAXPresetMgr=GetIEditor()->GetEAXPresetMgr();
	m_pEAXPresetMgr->Load();
	UpdateData(false);
	CToolbarDialog::OnInitDialog();
	CRect rc;
	InitToolbar();
	// Resize the toolbar
	GetClientRect(rc);
	m_cDlgToolBar.SetWindowPos(NULL, 0, 0, rc.right, 70, SWP_NOZORDER);
	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_cDlgToolBar.SetBarStyle(m_cDlgToolBar.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
	RecalcLayout();
	m_wndParams.SetUpdateCallback(functor(*this, &CEAXPresetsDlg::OnParamsChanged));
	Update();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CEAXPresetsDlg::InitToolbar()
{
	// Create the toolbar
	m_cDlgToolBar.CreateEx(this, TBSTYLE_FLAT|TBSTYLE_WRAPABLE,	WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	m_cDlgToolBar.LoadToolBar(IDR_EAXPRESETS);
	m_cDlgToolBar.GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	m_cDlgToolBar.CalcFixedLayout(0, TRUE);
}

//////////////////////////////////////////////////////////////////////////
void CEAXPresetsDlg::Update()
{
	XmlNodeRef pRootNode=m_pEAXPresetMgr->GetRootNode();
	if (!pRootNode)
		return;
	int nCurSel=m_wndPresets.GetCurSel();
	m_wndPresets.ResetContent();
	CString sName;
	for (int i=0;i<pRootNode->getChildCount();i++)
	{
		XmlNodeRef pPresetNode=pRootNode->getChild(i);
		m_wndPresets.AddString(pPresetNode->getTag());
	}
	m_wndPresets.SetCurSel(nCurSel);
	OnLbnSelchangePresets();
}

//////////////////////////////////////////////////////////////////////////
LRESULT CEAXPresetsDlg::OnUpdateProperties(WPARAM wParam, LPARAM lParam)
{
	int nCurSel=m_wndPresets.GetCurSel();
	if (nCurSel<0)
		return -1;
	CString sCurSel;
	m_wndPresets.GetText(nCurSel, sCurSel);
	m_wndParams.EnableUpdateCallback(false);
	m_wndParams.CreateItems(m_pEAXPresetMgr->GetRootNode()->findChild(sCurSel));
	m_wndParams.EnableUpdateCallback(true);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CEAXPresetsDlg::OnParamsChanged(XmlNodeRef pNode)
{
	if (!m_pEAXPresetMgr->UpdateParameter(pNode))
		AfxMessageBox("An error occured while updating script-tables.", MB_ICONEXCLAMATION | MB_OK);
}

// CSoundPresetsDlg message handlers
void CEAXPresetsDlg::OnLbnSelchangePresets()
{
	int nCurSel=m_wndPresets.GetCurSel();
	if (nCurSel<0)
	{
		m_cDlgToolBar.GetToolBarCtrl().EnableButton(ID_DELPRESET, FALSE);
		XmlNodeRef pEmptyRoot=new CXmlNode("Root");
		m_wndParams.CreateItems(pEmptyRoot);
		return;
	}else
	{
		m_cDlgToolBar.GetToolBarCtrl().EnableButton(ID_DELPRESET, TRUE);
	}
	PostMessage(WM_UPDATEPROPERTIES, 0, 0);
/*	if (m_bUpdateSoundProperties)
	{
		CString sCurSel;
		m_wndPresets.GetText(nCurSel, sCurSel);
		m_wndSounds.CreateItems(m_pSoundPresetMgr->GetRootNode()->findChild(sCurSel));
	}else
	{
		m_wndSounds.ReloadValues();
	}*/
}

void CEAXPresetsDlg::OnSavePreset()
{
	if (!m_pEAXPresetMgr->Save())
		AfxMessageBox("An error occured while saving eax-presets. Read-only ?", MB_ICONEXCLAMATION | MB_OK);
}

void CEAXPresetsDlg::OnAddPreset()
{
	CStringDlg dlgName("Please enter name for preset:");
	if (dlgName.DoModal()==IDOK)
	{
		if (!m_pEAXPresetMgr->AddPreset(dlgName.GetString()))
			AfxMessageBox("Cannot add preset. Check if a preset with that name exist already and if the name is valid (A-Z, 0-9 (not first character)).", MB_ICONEXCLAMATION | MB_OK);
		else
		{
			m_pEAXPresetMgr->Reload();
			Update();
		}
	}
}

void CEAXPresetsDlg::OnDelPreset()
{
	int nCurSel=m_wndPresets.GetCurSel();
	if (nCurSel<0)
		return;
	if (AfxMessageBox("Are you sure you want to delete the eax-preset ?", MB_ICONQUESTION | MB_YESNO)==IDNO)
		return;
	CString sCurSel;
	m_wndPresets.GetText(nCurSel, sCurSel);
	if (!m_pEAXPresetMgr->DelPreset(sCurSel))
		AfxMessageBox("Cannot delete preset.", MB_ICONEXCLAMATION | MB_OK);
	else
		Update();
}

void CEAXPresetsDlg::OnClose()
{
/*	if (!*/m_pEAXPresetMgr->Save();//)
//		AfxMessageBox("An error occured while saving sound-presets.", MB_ICONEXCLAMATION | MB_OK);
	CToolbarDialog::OnClose();
}

void CEAXPresetsDlg::OnDestroy()
{
/*	if (!*/m_pEAXPresetMgr->Save();//)
//		AfxMessageBox("An error occured while saving sound-presets.", MB_ICONEXCLAMATION | MB_OK);
	CToolbarDialog::OnDestroy();
}
