////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tvtrackpropsdialog.cpp
//  Version:     v1.00
//  Created:     28/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVTrackPropsDialog.h"
#include "TVSelectKeyDialog.h"
#include "TVSoundKeyDialog.h"
#include "TVEventKeyDialog.h"
#include "TVCharacterKeyDialog.h"
#include "TVExprKeyDialog.h"
#include "TVConsoleKeyDialog.h"
#include "TVMusicKeyDialog.h"
#include "AnimationContext.h"
#include "IMovieSystem.h"

// CTVTrackPropsDialog dialog

IMPLEMENT_DYNAMIC(CTVTrackPropsDialog, CDialog)
CTVTrackPropsDialog::CTVTrackPropsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTVTrackPropsDialog::IDD, pParent)
{
	m_track = 0;
	m_node = 0;
	m_key = -1;
	m_currentDlg = -1;
}

CTVTrackPropsDialog::~CTVTrackPropsDialog()
{
}

void CTVTrackPropsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PREVNEXT, m_keySpinBtn);
	DDX_Control(pDX, IDC_KEYNUM, m_keynum);
}


BEGIN_MESSAGE_MAP(CTVTrackPropsDialog, CDialog)
	ON_NOTIFY(UDN_DELTAPOS, IDC_PREVNEXT, OnDeltaposPrevnext)
	ON_EN_UPDATE(IDC_TIME, OnUpdateTime)
	ON_BN_CLICKED(IDC_CONSTANT, OnBnClickedConstant)
	ON_BN_CLICKED(IDC_CYCLE, OnBnClickedCycle)
	ON_BN_CLICKED(IDC_LOOP, OnBnClickedLoop)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CTVTrackPropsDialog message handlers

BOOL CTVTrackPropsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_time.Create( this,IDC_TIME );
	m_time.SetRange( -10000,10000 );

	m_keySpinBtn.SetPos(0);
	m_keySpinBtn.SetBuddy( &m_keynum );
	m_keySpinBtn.SetRange( -10000,10000 );

	CRect rc;
	CWnd *keyPropsWnd = GetDlgItem(IDC_KEY);
	if (keyPropsWnd)
	{
		keyPropsWnd->GetWindowRect(rc);
		ScreenToClient(rc);
	}

	SKeyDlgInfo DlgInfo;

	DlgInfo.pDlg=new CTVEventKeyDialog;
	DlgInfo.pDlg->Create( CTVEventKeyDialog::IDD,this );
	DlgInfo.pDlg->MoveWindow( rc );
	DlgInfo.pDlg->ShowWindow( SW_HIDE );
	DlgInfo.Tracks.clear();
	DlgInfo.Tracks.push_back(ATRACK_EVENT);
	m_KeyDlg.push_back(DlgInfo);

	DlgInfo.pDlg=new CTcbKeyDialog;
	DlgInfo.pDlg->Create( CTcbKeyDialog::IDD,this );
	DlgInfo.pDlg->MoveWindow( rc );
	DlgInfo.pDlg->ShowWindow( SW_HIDE );
	DlgInfo.Tracks.clear();
	DlgInfo.Tracks.push_back(ATRACK_TCB_FLOAT);
	DlgInfo.Tracks.push_back(ATRACK_TCB_VECTOR);
	DlgInfo.Tracks.push_back(ATRACK_TCB_QUAT);
	m_KeyDlg.push_back(DlgInfo);

	DlgInfo.pDlg=new CTVSelectKeyDialog;
	DlgInfo.pDlg->Create( CTVSelectKeyDialog::IDD,this );
	DlgInfo.pDlg->MoveWindow( rc );
	DlgInfo.pDlg->ShowWindow( SW_HIDE );
	DlgInfo.Tracks.clear();
	DlgInfo.Tracks.push_back(ATRACK_SELECT);
	m_KeyDlg.push_back(DlgInfo);

	DlgInfo.pDlg=new CTVSoundKeyDialog;
	DlgInfo.pDlg->Create( CTVSoundKeyDialog::IDD,this );
	DlgInfo.pDlg->MoveWindow( rc );
	DlgInfo.pDlg->ShowWindow( SW_HIDE );
	DlgInfo.Tracks.clear();
	DlgInfo.Tracks.push_back(ATRACK_SOUND);
	m_KeyDlg.push_back(DlgInfo);

	DlgInfo.pDlg=new CTVCharacterKeyDialog;
	DlgInfo.pDlg->Create( CTVCharacterKeyDialog::IDD,this );
	DlgInfo.pDlg->MoveWindow( rc );
	DlgInfo.pDlg->ShowWindow( SW_HIDE );
	DlgInfo.Tracks.clear();
	DlgInfo.Tracks.push_back(ATRACK_CHARACTER);
	m_KeyDlg.push_back(DlgInfo);

	DlgInfo.pDlg=new CTVExprKeyDialog;
	DlgInfo.pDlg->Create( CTVExprKeyDialog::IDD,this );
	DlgInfo.pDlg->MoveWindow( rc );
	DlgInfo.pDlg->ShowWindow( SW_HIDE );
	DlgInfo.Tracks.clear();
	DlgInfo.Tracks.push_back(ATRACK_EXPRESSION);
	m_KeyDlg.push_back(DlgInfo);

	DlgInfo.pDlg=new CTVConsoleKeyDialog;
	DlgInfo.pDlg->Create( CTVConsoleKeyDialog::IDD,this );
	DlgInfo.pDlg->MoveWindow( rc );
	DlgInfo.pDlg->ShowWindow( SW_HIDE );
	DlgInfo.Tracks.clear();
	DlgInfo.Tracks.push_back(ATRACK_CONSOLE);
	m_KeyDlg.push_back(DlgInfo);

	DlgInfo.pDlg=new CTVMusicKeyDialog;
	DlgInfo.pDlg->Create( CTVMusicKeyDialog::IDD,this );
	DlgInfo.pDlg->MoveWindow( rc );
	DlgInfo.pDlg->ShowWindow( SW_HIDE );
	DlgInfo.Tracks.clear();
	DlgInfo.Tracks.push_back(ATRACK_MUSIC);
	m_KeyDlg.push_back(DlgInfo);

/*
	m_dlgTcb.Create( CTcbKeyDialog::IDD,this );
	m_dlgTcb.MoveWindow( rc );
	m_dlgTcb.ShowWindow( SW_HIDE );

	m_dlgEntity.Create( CTVEntityKeyDialog::IDD,this );
	m_dlgEntity.MoveWindow( rc );
	m_dlgEntity.ShowWindow( SW_HIDE );
*/
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CTVTrackPropsDialog::SetKey( IAnimNode *node,int paramId,IAnimTrack *track,int nkey )
{
	assert( track );

	m_currentDlg = -1;
	EAnimTrackType trType = track->GetType();
	for (int i=0;i<(int)m_KeyDlg.size();i++)
	{
		IKeyDlg *pDlg=m_KeyDlg[i].pDlg;
		SKeyDlgInfo::TTracks::iterator It=std::find(m_KeyDlg[i].Tracks.begin(), m_KeyDlg[i].Tracks.end(), trType);
		if (It!=m_KeyDlg[i].Tracks.end())
		{
			m_currentDlg = i;
			pDlg->ShowWindow(SW_SHOW);
		}
		else
			pDlg->ShowWindow(SW_HIDE);
	}
	/*switch (trType)
	{
	case ATRACK_TCB_FLOAT:
	case ATRACK_TCB_VECTOR:
	case ATRACK_TCB_QUAT:
		m_dlgTcb.ShowWindow( SW_SHOW );
		m_dlgEntity.ShowWindow( SW_HIDE );
		break;
	case ATRACK_ENTITY:
		m_dlgTcb.ShowWindow( SW_HIDE );
		m_dlgEntity.ShowWindow( SW_SHOW );
		break;
	default:
		m_dlgTcb.ShowWindow( SW_HIDE );
		m_dlgEntity.ShowWindow( SW_HIDE );
	}*/

	m_track = track;
	m_node = node;
	m_key = nkey;

	CString title;
	title = node->GetName();
	title += "\\";

	IAnimBlock *ablock = node->GetAnimBlock();
	if (ablock)
	{
		IAnimNode::SParamInfo info;
		if (node->GetParamInfo( paramId,info ))
		{
			title += info.name;
		}
	}

	if (IsWindow(m_hWnd))
	{
		SetCurrKey(nkey);

		// Set track properties.
		if (m_track->GetFlags() & ATRACK_LOOP)
		{
			CheckDlgButton( IDC_CONSTANT,BST_UNCHECKED );
			CheckDlgButton( IDC_CYCLE,BST_UNCHECKED );
			CheckDlgButton( IDC_LOOP,BST_CHECKED );
		}
		else if (m_track->GetFlags() & ATRACK_CYCLE)
		{
			CheckDlgButton( IDC_CONSTANT,BST_UNCHECKED );
			CheckDlgButton( IDC_CYCLE,BST_CHECKED );
			CheckDlgButton( IDC_LOOP,BST_UNCHECKED );
		}
		else
		{
			CheckDlgButton( IDC_CONSTANT,BST_CHECKED );
			CheckDlgButton( IDC_CYCLE,BST_UNCHECKED );
			CheckDlgButton( IDC_LOOP,BST_UNCHECKED );
		}
	}
}

void CTVTrackPropsDialog::OnDeltaposPrevnext(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	if (!m_track)
		return;
	
	int nkey = m_key + pNMUpDown->iDelta;
	if (nkey < 0)
		nkey = m_track->GetNumKeys()-1;
	if (nkey > m_track->GetNumKeys()-1)
		nkey = 0;

	SetCurrKey( nkey );

	*pResult = 1;
}

void CTVTrackPropsDialog::OnUpdateTime()
{
	if (!m_track)
		return;

	if (m_key < 0 || m_key >= m_track->GetNumKeys())
		return;

	float time = m_time.GetValue();
	m_track->SetKeyTime( m_key,time );
	m_track->SortKeys();

	int k = m_track->FindKey( time );
	if (k != m_key)
	{
		SetCurrKey( k );
	}
	GetIEditor()->GetAnimation()->ForceAnimation();
}

void CTVTrackPropsDialog::SetCurrKey( int nkey )
{
	m_key = nkey;
	m_keySpinBtn.SetRange( 1,m_track->GetNumKeys() );
	m_keySpinBtn.SetPos( nkey+1 );

	if (m_key >= 0 && m_key < m_track->GetNumKeys())
	{
		m_time.SetValue( m_track->GetKeyTime(nkey) );
		for (int i=0;i<(int)m_KeyDlg.size();i++)
		{
			IKeyDlg *pDlg=m_KeyDlg[i].pDlg;
			if (pDlg->IsWindowVisible())
				pDlg->SetKey(m_node, m_track, nkey);
		}
/*
		if (m_dlgTcb.IsWindowVisible())
			m_dlgTcb.SetKey( m_node,m_track,nkey );
		if (m_dlgEntity.IsWindowVisible())
			m_dlgEntity.SetKey( m_node,m_track,nkey );*/
	}
}

//////////////////////////////////////////////////////////////////////////
void CTVTrackPropsDialog::OnCancel()
{
	DestroyWindow();
}

void CTVTrackPropsDialog::OnBnClickedConstant()
{
	if (m_track)
		m_track->SetFlags( m_track->GetFlags() & ~(ATRACK_LOOP|ATRACK_CYCLE) );
}

void CTVTrackPropsDialog::OnBnClickedCycle()
{
	if (m_track)
	{
		m_track->SetFlags( m_track->GetFlags() & ~(ATRACK_LOOP|ATRACK_CYCLE) );
		m_track->SetFlags( m_track->GetFlags() | ATRACK_CYCLE );
	}
}

void CTVTrackPropsDialog::OnBnClickedLoop()
{
	if (m_track)
	{
		m_track->SetFlags( m_track->GetFlags() & ~(ATRACK_LOOP|ATRACK_CYCLE) );
		m_track->SetFlags( m_track->GetFlags() | ATRACK_LOOP );
	}
}

void CTVTrackPropsDialog::OnDestroy()
{
	CDialog::OnDestroy();

	m_track = 0;
	m_node = 0;
	m_key = -1;

	for (int i=0;i<(int)m_KeyDlg.size();i++)
	{
		delete m_KeyDlg[i].pDlg;
	}
	m_KeyDlg.clear();
}

//////////////////////////////////////////////////////////////////////////
void CTVTrackPropsDialog::ReloadKey()
{
	if (m_currentDlg < 0)
		return;

	IKeyDlg *keyDlg = m_KeyDlg[m_currentDlg].pDlg;

	if (!keyDlg->CanReloadKey())
		return;

	IAnimNode *node = keyDlg->GetNode();
	IAnimTrack *track = keyDlg->GetTrack();
	if (!node || !track)
		return;
	int paramId = node->FindTrack(track);

	SetKey( node,paramId,track,keyDlg->GetKey() );
}