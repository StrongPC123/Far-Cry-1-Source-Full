////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tvcharacterkeydialog.cpp
//  Version:     v1.00
//  Created:     20/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVCharacterKeyDialog.h"
#include "CryEditDoc.h"
#include "Objects\EntityScript.h"

#include "IMovieSystem.h"
#include "IEntitySystem.h"

// CTVCharacterKeyDialog dialog

IMPLEMENT_DYNAMIC(CTVCharacterKeyDialog, CDialog)
CTVCharacterKeyDialog::CTVCharacterKeyDialog(CWnd* pParent /*=NULL*/)
	: IKeyDlg(CTVCharacterKeyDialog::IDD, pParent)
{
	m_node = 0;
	m_track = 0;
	m_key = 0;
}

CTVCharacterKeyDialog::~CTVCharacterKeyDialog()
{
}

void CTVCharacterKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANIMATION, m_animation);
	DDX_Control(pDX, IDC_LOOP, m_loopBtn);
	DDX_Control(pDX, IDC_UNLOAD, m_unloadBtn);
}


BEGIN_MESSAGE_MAP(CTVCharacterKeyDialog, CDialog)
	ON_CBN_SELENDOK(IDC_ANIMATION, ControlsToKey)
	ON_EN_UPDATE(IDC_ANIMATION, ControlsToKey)
	ON_EN_UPDATE(IDC_BLEND_TIME, ControlsToKey)
	ON_EN_UPDATE(IDC_TIME_SCALE, ControlsToKey)
	ON_BN_CLICKED(IDC_LOOP, OnBnClickedLoop)
	ON_BN_CLICKED(IDC_UNLOAD, OnBnClickedUnload)
END_MESSAGE_MAP()


// CTVCharacterKeyDialog message handlers

void CTVCharacterKeyDialog::SetKey( IAnimNode *node,IAnimTrack *track,int nkey )
{
	m_node = node;
	m_track = track;
	m_key = nkey;

	if (m_key < 0 || m_key >= m_track->GetNumKeys())
		return;

	if (track->GetType() != ATRACK_CHARACTER)
		return;

	m_animation.ResetContent();

	// Find editor object who owns this node.
	IEntity *entity = node->GetEntity();
	if (entity)
	{
		// Add available animations.
		ICryCharInstance *pCharacter = entity->GetCharInterface()->GetCharacter(0);
		if (pCharacter)
		{
			IAnimationSet* pAnimations = pCharacter->GetModel()->GetAnimationSet();
			assert (pAnimations);
			int numAnimations = pAnimations->Count();
			m_animation.AddString("");
			for (int i = 0; i < numAnimations; i++)
			{
				m_animation.AddString ( pAnimations->GetName(i));
			}
		}
	}

	ICharacterKey key;
	m_track->GetKey( m_key,&key );

	m_animation.SelectString( -1,key.animation );
	m_blendTime.SetValue( key.blendTime );
	m_startTime.SetValue( key.startTime );
	m_timeScale.SetValue( key.speed );
	m_startTime.SetValue( key.startTime );
	//m_hide.SetCheck( (key.hidden)?BST_CHECKED:BST_UNCHECKED );
	m_loopBtn.SetCheck( (key.bLoop)?BST_CHECKED:BST_UNCHECKED );
	m_unloadBtn.SetCheck(key.bUnload ? BST_CHECKED : BST_UNCHECKED);
}

//////////////////////////////////////////////////////////////////////////
BOOL CTVCharacterKeyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_blendTime.Create( this,IDC_BLEND_TIME );
	m_blendTime.SetRange(0,100);
	m_blendTime.SetValue(0);
	
	m_startTime.Create( this,IDC_START_TIME );

	m_timeScale.Create( this,IDC_TIME_SCALE );
	m_timeScale.SetValue(1);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CTVCharacterKeyDialog::ControlsToKey()
{
	if (!m_track || m_key < 0 || m_key >= m_track->GetNumKeys())
		return;

	//bool hidden = m_hide.GetCheck() == BST_CHECKED;

	CString animation;
	m_animation.GetWindowText(animation);

	ICharacterKey key;
	m_track->GetKey( m_key,&key );
	
	//key.hidden = hidden;
	strncpy( key.animation,animation,sizeof(key.animation) );
	key.animation[sizeof(key.animation)-1] = '\0';
	
	key.blendTime = m_blendTime.GetValue();
	key.speed = m_timeScale.GetValue();
	key.startTime = m_startTime.GetValue();

	key.duration = 0;
	key.bLoop = m_loopBtn.GetCheck() != 0;
	key.bUnload = m_unloadBtn.GetCheck() != 0;

	if (strlen(key.animation) > 0)
	{
		IEntity *entity = m_node->GetEntity();
		if (entity)
		{
			ICryCharInstance *pCharacter = entity->GetCharInterface()->GetCharacter(0);
			if (pCharacter)
			{
				IAnimationSet* pAnimations = pCharacter->GetModel()->GetAnimationSet();
				assert (pAnimations);

				key.duration = pAnimations->GetLength( key.animation ) + pAnimations->GetStart( key.animation );
			}
		}
	}

	m_track->SetKey( m_key,&key );

	RefreshTrackView();
}

//////////////////////////////////////////////////////////////////////////
void CTVCharacterKeyDialog::OnBnClickedLoop()
{
	ControlsToKey();
}

//////////////////////////////////////////////////////////////////////////
void CTVCharacterKeyDialog::OnBnClickedUnload()
{
	ControlsToKey();
}
