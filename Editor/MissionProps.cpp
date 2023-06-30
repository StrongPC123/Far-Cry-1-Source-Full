// PlayerProps.cpp : implementation file
//

#include "stdafx.h"
#include "MissionProps.h"
#include "EquipPackDialog.h"
#include "IGame.h"
#include "CryEditDoc.h"
#include "mission.h"
#include "missionscript.h"

// CMissionProps dialog

IMPLEMENT_DYNAMIC(CMissionProps, CPropertyPage)
CMissionProps::CMissionProps()
	: CPropertyPage(CMissionProps::IDD)
{
}

CMissionProps::~CMissionProps()
{
}

void CMissionProps::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCRIPT, m_btnScript);
	DDX_Control(pDX, IDC_EQUIPPACK, m_btnEquipPack);
	DDX_Control(pDX, IDC_MUSICSCRIPT, m_editMusicScript);
	DDX_Control(pDX, IDC_METHODS, m_lstMethods);
	DDX_Control(pDX, IDC_EVENTS, m_lstEvents);
}


BEGIN_MESSAGE_MAP(CMissionProps, CPropertyPage)
	ON_BN_CLICKED(IDC_EQUIPPACK, OnBnClickedEquippack)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
	ON_BN_CLICKED(IDC_CREATE, OnBnClickedCreate)
	ON_BN_CLICKED(IDC_EDIT, OnBnClickedEdit)
	ON_BN_CLICKED(IDC_RELOADSCRIPT, OnBnClickedReload)
	ON_BN_CLICKED(IDC_BROWSEMUSIC, OnBnClickedBrowsemusic)
	ON_BN_CLICKED(IDC_NOMUSIC, OnBnClickedNomusic)
END_MESSAGE_MAP()

void CMissionProps::LoadScript(CString sFilename)
{
	CMissionScript *pScript=GetIEditor()->GetDocument()->GetCurrentMission()->GetScript();
	pScript->SetScriptFile( sFilename );
	pScript->Load();
	m_btnScript.SetWindowText(sFilename);
	m_lstMethods.ResetContent();
	for (int i=0;i<pScript->GetMethodCount();i++)
	{
		m_lstMethods.AddString(pScript->GetMethod(i));
	}
	m_lstEvents.ResetContent();
	for (int i=0;i<pScript->GetEventCount();i++)
	{
		m_lstEvents.AddString(pScript->GetEvent(i));
	}
}

// CMissionProps message handlers

void CMissionProps::OnBnClickedEquippack()
{
	CEquipPackDialog EPDlg(this);
	CString sText;
	m_btnEquipPack.GetWindowText(sText);
	EPDlg.SetCurrEquipPack(sText);
	if (EPDlg.DoModal()==IDOK)
	{
		m_btnEquipPack.SetWindowText(EPDlg.GetCurrEquipPack());
		GetIEditor()->GetDocument()->GetCurrentMission()->SetPlayerEquipPack(EPDlg.GetCurrEquipPack());

#ifndef _ISNOTFARCRY
			GetIXGame( GetIEditor()->GetGame() )->SetPlayerEquipPackName( EPDlg.GetCurrEquipPack() );
#endif
	}
}

BOOL CMissionProps::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadScript(GetIEditor()->GetDocument()->GetCurrentMission()->GetScript()->GetFilename());
	m_btnScript.SetWindowText(GetIEditor()->GetDocument()->GetCurrentMission()->GetScript()->GetFilename());
	m_btnEquipPack.SetWindowText(GetIEditor()->GetDocument()->GetCurrentMission()->GetPlayerEquipPack());
	m_editMusicScript.SetWindowText(GetIEditor()->GetDocument()->GetCurrentMission()->GetMusicScriptFilename());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CMissionProps::OnBnClickedBrowse()
{
	CString file;
	char szFilters[] = "Script Files (*.lua)|*.lua||";
	if (CFileUtil::SelectSingleFile( EFILE_TYPE_ANY,file,szFilters,"Scripts" ))
	{
		LoadScript( file );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMissionProps::OnBnClickedCreate()
{
	CString file;
	char szFilters[] = "Script Files (*.lua)|*.lua||";
	//CFileDialog Dlg(FALSE, "lua", CString(GetIEditor()->GetMasterCDFolder())+"scripts\\*.lua", OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_ENABLESIZING|OFN_EXPLORER, szFilters);
	if (CFileUtil::SelectSaveFile( szFilters,"*.lua","Scripts",file))
	{
		// copy from template...
		if (!CopyFile(CString(GetIEditor()->GetMasterCDFolder())+"Editor\\MissionTemplate.lua",file, FALSE))
		{
			Warning( CString("MissionScript-Template (expected in ")+GetIEditor()->GetMasterCDFolder()+"editor\\MissionTemplate.lua) couldn't be found !" );
			return;
		}
		SetFileAttributes( file,FILE_ATTRIBUTE_NORMAL );
		file = GetIEditor()->GetRelativePath( file );
		LoadScript( file );
	}
}

void CMissionProps::OnBnClickedReload()
{
	CString sName=GetIEditor()->GetDocument()->GetCurrentMission()->GetScript()->GetFilename();
	if (sName.GetLength())
		LoadScript(sName);
}

void CMissionProps::OnBnClickedEdit()
{
	GetIEditor()->GetDocument()->GetCurrentMission()->GetScript()->Edit();
}

void CMissionProps::OnBnClickedBrowsemusic()
{
	CString file;
	char szFilters[] = "Script Files (*.lua)|*.lua||";
	if (CFileUtil::SelectSingleFile( EFILE_TYPE_ANY,file,szFilters,"Scripts\\Music" ))
	{
		GetIEditor()->GetDocument()->GetCurrentMission()->SetMusicScriptFilename( file );
		m_editMusicScript.SetWindowText(file);
	}
}

void CMissionProps::OnBnClickedNomusic()
{
	GetIEditor()->GetDocument()->GetCurrentMission()->SetMusicScriptFilename("");
	m_editMusicScript.SetWindowText("");
}
