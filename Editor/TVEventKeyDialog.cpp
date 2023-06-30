////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   CTVEntityKeyDialog.cpp
//  Version:     v1.00
//  Created:     28/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "TVEventKeyDialog.h"
#include "CryEditDoc.h"
#include "Objects\EntityScript.h"
#include "mission.h"
#include "missionscript.h"

#include "IMovieSystem.h"
#include "IEntitySystem.h"

// CTVEntityKeyDialog dialog

IMPLEMENT_DYNAMIC(CTVEventKeyDialog, CDialog)
CTVEventKeyDialog::CTVEventKeyDialog(CWnd* pParent /*=NULL*/)
	: IKeyDlg(CTVEventKeyDialog::IDD, pParent)
{
	m_node = 0;
	m_track = 0;
	m_key = 0;
}

CTVEventKeyDialog::~CTVEventKeyDialog()
{
}

void CTVEventKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANIMATION, m_animation);
	DDX_Control(pDX, IDC_EVENT, m_event);
	//DDX_Control(pDX, IDC_HIDE, m_hide);
}


BEGIN_MESSAGE_MAP(CTVEventKeyDialog, CDialog)
	//ON_BN_CLICKED(IDC_HIDE, ControlsToKey)
	ON_CBN_SELENDOK(IDC_EVENT, ControlsToKey)
	ON_CBN_SELENDOK(IDC_ANIMATION, ControlsToKey)
END_MESSAGE_MAP()


// CTVEntityKeyDialog message handlers

void CTVEventKeyDialog::SetKey( IAnimNode *node,IAnimTrack *track,int nkey )
{
	m_node = node;
	m_track = track;
	m_key = nkey;

	if (m_key < 0 || m_key >= m_track->GetNumKeys())
		return;

	if (track->GetType() != ATRACK_EVENT)
		return;

	m_event.ResetContent();
	m_animation.ResetContent();

	if (node->GetType()==ANODE_SCENE)
	{
		CMission *pMission=GetIEditor()->GetDocument()->GetCurrentMission();
		if (pMission)
		{
			CMissionScript *pScript=pMission->GetScript();
			if (pScript)
			{
				for (int i=0;i<pScript->GetEventCount();i++)
				{
					m_event.AddString(pScript->GetEvent(i));
				}
			}
		}
	}else
	{
		// Find editor object who owns this node.
		IEntity *entity = node->GetEntity();
		if (entity)
		{
			// Add events.
			const char *className = entity->GetEntityClassName();
			// Find EntityClass.
			CEntityScript *script = CEntityScriptRegistry::Instance()->Find( className );
			if (script)
			{
				m_event.AddString("");
				for (int i = 0; i < script->GetEventCount(); i++)
				{
					m_event.AddString(script->GetEvent(i));
				}
			}

			// Add available animations.
			ICryCharInstance *pCharacter = entity->GetCharInterface()->GetCharacter(0);
			if (pCharacter)
			{
				m_animation.AddString("");
				IAnimationSet* pAnimations = pCharacter->GetModel()->GetAnimationSet();
				assert (pAnimations);
				int numAnimations = pAnimations->Count();
				for (int i = 0; i < numAnimations; ++i)
				{
					m_animation.AddString (pAnimations->GetName(i));
				}
			}
		}
	}
	IEventKey key;
	m_track->GetKey( m_key,&key );

	m_animation.SelectString( -1,key.animation );
	m_event.SelectString( -1,key.event );
	//m_hide.SetCheck( (key.hidden)?BST_CHECKED:BST_UNCHECKED );
}

//////////////////////////////////////////////////////////////////////////
BOOL CTVEventKeyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CTVEventKeyDialog::ControlsToKey()
{
	if (!m_track || m_key < 0 || m_key >= m_track->GetNumKeys())
		return;

	//bool hidden = m_hide.GetCheck() == BST_CHECKED;

	CString event,animation;
	m_event.GetWindowText(event);
	m_animation.GetWindowText(animation);

	IEventKey key;
	m_track->GetKey( m_key,&key );
	
	//key.hidden = hidden;
	strncpy( key.event,event,sizeof(key.event) );
	strncpy( key.animation,animation,sizeof(key.animation) );

	key.duration = 0;

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
				key.duration = pAnimations->GetLength( key.animation );
			}
		}
	}
	m_track->SetKey( m_key,&key );

	RefreshTrackView();
}