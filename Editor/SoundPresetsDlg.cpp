// SoundPresetsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SoundPresetsDlg.h"
#include "SoundPresetMgr.h"
#include "StringDlg.h"
#include "Controls\PropertyItem.h"

#define WM_UPDATEPROPERTIES	WM_APP+1

// CSoundPresetsDlg dialog

IMPLEMENT_DYNAMIC(CSoundPresetsDlg, CToolbarDialog)
CSoundPresetsDlg::CSoundPresetsDlg(CWnd* pParent /*=NULL*/)
	: CToolbarDialog(CSoundPresetsDlg::IDD, pParent)
{
	m_pSoundPresetMgr=NULL;
}

CSoundPresetsDlg::~CSoundPresetsDlg()
{
}

void CSoundPresetsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRESETS, m_wndPresets);
	DDX_Control(pDX, IDC_SOUNDS, m_wndSounds);
}


BEGIN_MESSAGE_MAP(CSoundPresetsDlg, CToolbarDialog)
	ON_LBN_SELCHANGE(IDC_PRESETS, OnLbnSelchangePresets)
	ON_COMMAND(ID_SAVE, OnSavePreset)
	ON_COMMAND(ID_ADDPRESET, OnAddPreset)
	ON_COMMAND(ID_DELPRESET, OnDelPreset)
	ON_COMMAND(ID_ADDSOUND, OnAddSound)
	ON_COMMAND(ID_DELSOUND, OnDelSound)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_UPDATEPROPERTIES, OnUpdateProperties)
END_MESSAGE_MAP()

BOOL CSoundPresetsDlg::OnInitDialog()
{
	m_pSoundPresetMgr=GetIEditor()->GetSoundPresetMgr();
	m_pSoundPresetMgr->Load();
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
	m_wndSounds.SetUpdateCallback(functor(*this, &CSoundPresetsDlg::OnSoundsChanged));
	m_wndSounds.SetSelChangeCallback(functor(*this, &CSoundPresetsDlg::OnSoundsSelChanged));
	Update();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CSoundPresetsDlg::InitToolbar()
{
	// Create the toolbar
	m_cDlgToolBar.CreateEx(this, TBSTYLE_FLAT|TBSTYLE_WRAPABLE,	WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC);
	m_cDlgToolBar.LoadToolBar24(IDR_SOUNDPRESETS);
	m_cDlgToolBar.GetToolBarCtrl().SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	m_cDlgToolBar.CalcFixedLayout(0, TRUE);
}

//////////////////////////////////////////////////////////////////////////
void CSoundPresetsDlg::Update()
{
	XmlNodeRef pRootNode=m_pSoundPresetMgr->GetRootNode();
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
	CPropertyItem *pItem=m_wndSounds.GetSelectedItem();
	if (pItem)
		OnSoundsSelChanged(pItem->GetXmlNode());
}

//////////////////////////////////////////////////////////////////////////
LRESULT CSoundPresetsDlg::OnUpdateProperties(WPARAM wParam, LPARAM lParam)
{
	int nCurSel=m_wndPresets.GetCurSel();
	if (nCurSel<0)
		return -1;
	CString sCurSel;
	m_wndPresets.GetText(nCurSel, sCurSel);
	m_wndSounds.EnableUpdateCallback(false);
	m_wndSounds.CreateItems(m_pSoundPresetMgr->GetRootNode()->findChild(sCurSel));
	m_wndSounds.EnableUpdateCallback(true);
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CSoundPresetsDlg::OnSoundsChanged(XmlNodeRef pNode)
{
	if (strcmp(pNode->getTag(), "Sound")==0)
	{
		CString sPropValue;
		if (pNode->getAttr("value", sPropValue))
		{
			XmlNodeRef pParent=pNode->getParent();
			if (pParent)
			{
				XmlNodeRef pParentParent=pParent->getParent();
				if (pParentParent)
				{
					pParent->setTag(sPropValue);
					m_pSoundPresetMgr->MakeTagUnique(pParentParent, pParent);
				}
			}
		}
		m_pSoundPresetMgr->Reload();
		Update();
	}else
	{
		if (!m_pSoundPresetMgr->UpdateParameter(pNode))
			AfxMessageBox("An error occured while updating script-tables.", MB_ICONEXCLAMATION | MB_OK);
	}
}

//////////////////////////////////////////////////////////////////////////
void CSoundPresetsDlg::OnSoundsSelChanged(XmlNodeRef pNode)
{
	if (pNode)
		m_cDlgToolBar.GetToolBarCtrl().EnableButton(ID_DELSOUND, TRUE);
	else
		m_cDlgToolBar.GetToolBarCtrl().EnableButton(ID_DELSOUND, FALSE);
}

// CSoundPresetsDlg message handlers
void CSoundPresetsDlg::OnLbnSelchangePresets()
{
	int nCurSel=m_wndPresets.GetCurSel();
	if (nCurSel<0)
	{
		m_cDlgToolBar.GetToolBarCtrl().EnableButton(ID_DELPRESET, FALSE);
		m_cDlgToolBar.GetToolBarCtrl().EnableButton(ID_ADDSOUND, FALSE);
		XmlNodeRef pEmptyRoot=new CXmlNode("Root");
		m_wndSounds.CreateItems(pEmptyRoot);
		return;
	}else
	{
		m_cDlgToolBar.GetToolBarCtrl().EnableButton(ID_DELPRESET, TRUE);
		m_cDlgToolBar.GetToolBarCtrl().EnableButton(ID_ADDSOUND, TRUE);
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

void CSoundPresetsDlg::OnSavePreset()
{
	if (!m_pSoundPresetMgr->Save())
		AfxMessageBox("An error occured while saving sound-presets. Read-only ?", MB_ICONEXCLAMATION | MB_OK);
}

void CSoundPresetsDlg::OnAddPreset()
{
	CStringDlg dlgName("Please enter name for preset:");
	if (dlgName.DoModal()==IDOK)
	{
		if (!m_pSoundPresetMgr->AddPreset(dlgName.GetString()))
			AfxMessageBox("Cannot add preset. Check if a preset with that name exist already and if the name is valid (A-Z, 0-9 (not first character)).", MB_ICONEXCLAMATION | MB_OK);
		else
		{
			m_pSoundPresetMgr->Reload();
			Update();
		}
	}
}

void CSoundPresetsDlg::OnDelPreset()
{
	int nCurSel=m_wndPresets.GetCurSel();
	if (nCurSel<0)
		return;
	if (AfxMessageBox("Are you sure you want to delete the sound-preset ?", MB_ICONQUESTION | MB_YESNO)==IDNO)
		return;
	CString sCurSel;
	m_wndPresets.GetText(nCurSel, sCurSel);
	if (!m_pSoundPresetMgr->DelPreset(sCurSel))
		AfxMessageBox("Cannot delete preset.", MB_ICONEXCLAMATION | MB_OK);
	else
		Update();
}

void CSoundPresetsDlg::OnAddSound()
{
	int nCurSel=m_wndPresets.GetCurSel();
	if (nCurSel<0)
		return;
	CString sCurSel;
	m_wndPresets.GetText(nCurSel, sCurSel);
	if (!m_pSoundPresetMgr->AddSound(sCurSel))
		AfxMessageBox("Cannot add sound.", MB_ICONEXCLAMATION | MB_OK);
	else
	{
		m_pSoundPresetMgr->Reload();
		Update();
	}
}

void CSoundPresetsDlg::OnDelSound()
{
	CPropertyItem *pItem=m_wndSounds.GetSelectedItem();
	if (!pItem)
		return;
	int nCurSel=m_wndPresets.GetCurSel();
	if (nCurSel<0)
		return;
	if (AfxMessageBox("Are you sure you want to delete the sound from the preset ?", MB_ICONQUESTION | MB_YESNO)==IDNO)
		return;
	CString sCurSel;
	m_wndPresets.GetText(nCurSel, sCurSel);
	if (!m_pSoundPresetMgr->DelSound(sCurSel, pItem->GetXmlNode()))
		AfxMessageBox("Cannot delete sound.", MB_ICONEXCLAMATION | MB_OK);
	else
		Update();
}

void CSoundPresetsDlg::OnClose()
{
/*	if (!*/m_pSoundPresetMgr->Save();//)
//		AfxMessageBox("An error occured while saving sound-presets.", MB_ICONEXCLAMATION | MB_OK);
	CToolbarDialog::OnClose();
}

void CSoundPresetsDlg::OnDestroy()
{
/*	if (!*/m_pSoundPresetMgr->Save();//)
//		AfxMessageBox("An error occured while saving sound-presets.", MB_ICONEXCLAMATION | MB_OK);
	CToolbarDialog::OnDestroy();
}
