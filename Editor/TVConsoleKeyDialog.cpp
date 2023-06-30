////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tvconsolekeydialog.cpp
//  Version:     v1.00
//  Created:     12/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVConsoleKeyDialog.h"
#include "CryEditDoc.h"
#include "Objects\EntityScript.h"
#include "mission.h"
#include "missionscript.h"

#include "IMovieSystem.h"
#include "IEntitySystem.h"

// CTVEntityKeyDialog dialog

IMPLEMENT_DYNAMIC(CTVConsoleKeyDialog, CDialog)
CTVConsoleKeyDialog::CTVConsoleKeyDialog(CWnd* pParent /*=NULL*/)
	: IKeyDlg(CTVConsoleKeyDialog::IDD, pParent)
{
	m_node = 0;
	m_track = 0;
	m_key = 0;
}

CTVConsoleKeyDialog::~CTVConsoleKeyDialog()
{
}

void CTVConsoleKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_command);
}


BEGIN_MESSAGE_MAP(CTVConsoleKeyDialog, CDialog)
	//ON_BN_CLICKED(IDC_HIDE, ControlsToKey)
	ON_EN_CHANGE(IDC_EDIT1, ControlsToKey)
END_MESSAGE_MAP()


// CTVEntityKeyDialog message handlers

void CTVConsoleKeyDialog::SetKey( IAnimNode *node,IAnimTrack *track,int nkey )
{
	m_node = node;
	m_track = track;
	m_key = nkey;

	if (m_key < 0 || m_key >= m_track->GetNumKeys())
		return;

	if (track->GetType() != ATRACK_CONSOLE)
		return;

	IConsoleKey key;
	m_track->GetKey( m_key,&key );

	m_command.SetWindowText( key.command );
}

//////////////////////////////////////////////////////////////////////////
BOOL CTVConsoleKeyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CTVConsoleKeyDialog::ControlsToKey()
{
	if (!m_track || m_key < 0 || m_key >= m_track->GetNumKeys())
		return;

	//bool hidden = m_hide.GetCheck() == BST_CHECKED;

	CString command;
	m_command.GetWindowText(command);

	IConsoleKey key;
	m_track->GetKey( m_key,&key );
	
	//key.hidden = hidden;
	strncpy( key.command,command,sizeof(key.command) );
	key.command[sizeof(key.command)-1] = 0;
	m_track->SetKey( m_key,&key );

	RefreshTrackView();
}