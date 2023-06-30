////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   aicharactersdialog.cpp
//  Version:     v1.00
//  Created:     13/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AICharactersDialog.h"

#include "Controls\PropertyCtrl.h"

#include "AI\AIManager.h"
#include "AI\AIGoalLibrary.h"
#include "AI\AIBehaviorLibrary.h"

// CAICharactersDialog dialog

IMPLEMENT_DYNAMIC(CAICharactersDialog, CDialog)
CAICharactersDialog::CAICharactersDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAICharactersDialog::IDD, pParent)
{
	//m_propWnd = 0;
}

CAICharactersDialog::~CAICharactersDialog()
{
}

void CAICharactersDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_editBtn);
	DDX_Control(pDX, IDC_RELOAD, m_reloadBtn);
	DDX_Control(pDX, IDC_BEHAVIOR, m_list);
	DDX_Control(pDX, IDC_DESCRIPTION, m_description);
}


BEGIN_MESSAGE_MAP(CAICharactersDialog, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_EDIT, OnBnClickedEdit)
	ON_BN_CLICKED(IDC_RELOAD, OnBnClickedReload)
	ON_LBN_SELCHANGE(IDC_BEHAVIOR, OnLbnSelchangeBehavior)
END_MESSAGE_MAP()


// CAICharactersDialog message handlers

BOOL CAICharactersDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	ReloadCharacters();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAICharactersDialog::OnDestroy()
{
	CDialog::OnDestroy();
	//delete m_propWnd;
}


//////////////////////////////////////////////////////////////////////////
void CAICharactersDialog::ReloadCharacters()
{
	CAIBehaviorLibrary *lib = GetIEditor()->GetAI()->GetBehaviorLibrary();
	std::vector<CAICharacterPtr> characters;
	lib->GetCharacters( characters );
	for (int i = 0; i < characters.size(); i++)
	{
		m_list.AddString( characters[i]->GetName() );
	}
	m_list.SelectString( -1,m_aiCharacter );

	CAICharacter *character = GetIEditor()->GetAI()->GetBehaviorLibrary()->FindCharacter( m_aiCharacter );
	if (character)
	{
		m_description.SetWindowText( character->GetDescription() );
	}
	else
	{
		m_description.SetWindowText( "" );
	}
}

void CAICharactersDialog::SetAICharacter( const CString &character )
{
	m_aiCharacter = character;
}

void CAICharactersDialog::OnBnClickedEdit()
{
	CAICharacter *character = GetIEditor()->GetAI()->GetBehaviorLibrary()->FindCharacter( m_aiCharacter );
	if (!character)
		return;

	character->Edit();
}

void CAICharactersDialog::OnBnClickedReload()
{
	CAICharacter *character = GetIEditor()->GetAI()->GetBehaviorLibrary()->FindCharacter( m_aiCharacter );
	if (!character)
		return;
	character->ReloadScript();
}

void CAICharactersDialog::OnLbnSelchangeBehavior()
{
	int sel = m_list.GetCurSel();
	if (sel == LB_ERR)
		return;
	m_list.GetText( sel,m_aiCharacter );
	
	CAICharacter *character = GetIEditor()->GetAI()->GetBehaviorLibrary()->FindCharacter( m_aiCharacter );
	if (character)
	{
		m_description.SetWindowText( character->GetDescription() );
	}
	else
	{
		m_description.SetWindowText( "" );
	}
}
