// TVSequenceProps.cpp : implementation file
//

#include "stdafx.h"
#include "TVSequenceProps.h"
#include "IMovieSystem.h"
#include "TrackViewUndo.h"

// CTVSequenceProps dialog

IMPLEMENT_DYNAMIC(CTVSequenceProps, CDialog)

CTVSequenceProps::CTVSequenceProps( IAnimSequence *seq,CWnd* pParent /* = NULL */ )
	: CDialog(CTVSequenceProps::IDD, pParent)
	, m_outOfRange(0)
{
	assert( seq );
	m_sequence = seq;
}

CTVSequenceProps::~CTVSequenceProps()
{
}

void CTVSequenceProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NAME, m_nameEdit);
	DDX_Control(pDX, IDC_ALWAYS_PLAY, m_alwaysPlayingBtn);
	DDX_Control(pDX, IDC_CUT_SCENE, m_cutSceneBtn);
	DDX_Control(pDX, IDC_DISABLEHUD, m_NoHUDBtn);
	DDX_Control(pDX, IDC_DISABLEPLAYER, m_NoPlayerBtn);
	DDX_Control(pDX, IDC_DISABLEPHYICS, m_NoPhysicsBtn);
	DDX_Control(pDX, IDC_DISABLEAI, m_NoAIBtn);
	DDX_Control(pDX, IDC_16TO9, m_16To9);
	DDX_Control(pDX, IDC_DISABLESOUNDS, m_NoSoundsBtn);
	DDX_Radio(pDX, IDC_ORT_ONCE, m_outOfRange);
}


BEGIN_MESSAGE_MAP(CTVSequenceProps, CDialog)
	ON_BN_CLICKED(IDC_RESCALE_TIME, OnBnClickedRescaleTime)
END_MESSAGE_MAP()


// CTVSequenceProps message handlers

BOOL CTVSequenceProps::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nameEdit.SetWindowText( m_sequence->GetName() );
	int seqFlags = m_sequence->GetFlags();
	
	if (seqFlags & IAnimSequence::PLAY_ONRESET)
		m_alwaysPlayingBtn.SetCheck( BST_CHECKED );
	else
		m_alwaysPlayingBtn.SetCheck( BST_UNCHECKED );

	if (seqFlags & IAnimSequence::CUT_SCENE)
		m_cutSceneBtn.SetCheck( BST_CHECKED );
	else
		m_cutSceneBtn.SetCheck( BST_UNCHECKED );

	if (seqFlags & IAnimSequence::NO_HUD)
		m_NoHUDBtn.SetCheck( BST_CHECKED );
	else
		m_NoHUDBtn.SetCheck( BST_UNCHECKED );

	if (seqFlags & IAnimSequence::NO_PLAYER)
		m_NoPlayerBtn.SetCheck( BST_CHECKED );
	else
		m_NoPlayerBtn.SetCheck( BST_UNCHECKED );

	if (seqFlags & IAnimSequence::NO_PHYSICS)
		m_NoPhysicsBtn.SetCheck( BST_CHECKED );
	else
		m_NoPhysicsBtn.SetCheck( BST_UNCHECKED );

	if (seqFlags & IAnimSequence::NO_AI)
		m_NoAIBtn.SetCheck( BST_CHECKED );
	else
		m_NoAIBtn.SetCheck( BST_UNCHECKED );

	if (seqFlags & IAnimSequence::IS_16TO9)
		m_16To9.SetCheck( BST_CHECKED );
	else
		m_16To9.SetCheck( BST_UNCHECKED );

	m_NoSoundsBtn.SetCheck( (seqFlags&IAnimSequence::NO_GAMESOUNDS) ? BST_CHECKED : BST_UNCHECKED );

	m_startTime.Create( this,IDC_START_TIME );
	m_endTime.Create( this,IDC_END_TIME );
	m_length.Create( this,IDC_LENGTH );

	Range timeRange = m_sequence->GetTimeRange();
	m_startTime.SetValue( timeRange.start );
	m_endTime.SetValue( timeRange.end );

	m_length.SetValue( timeRange.Length() );

	m_outOfRange = 0;
	if (m_sequence->GetFlags() & IAnimSequence::ORT_CONSTANT)
	{
		m_outOfRange = 1;
	}
	else if (m_sequence->GetFlags() & IAnimSequence::ORT_LOOP)
	{
		m_outOfRange = 2;
	}
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTVSequenceProps::OnBnClickedRescaleTime()
{
	CUndo undo( "AnimSequence Scale Time" );
	CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );
	// Rescale sequence to a new length.
	Range timeRange;
	timeRange.start = m_startTime.GetValue();
	timeRange.end = m_endTime.GetValue();

	float oldLength = timeRange.Length();
	float newLength = m_length.GetValue();
	if (oldLength != newLength)
	{
		//m_sequence->SetTimeRange( timeRange.start
		timeRange.end = timeRange.start + newLength;
		m_sequence->ScaleTimeRange( timeRange );
		m_startTime.SetValue( timeRange.start );
		m_endTime.SetValue( timeRange.end );
	}
}

void CTVSequenceProps::OnOK()
{
	UpdateData(TRUE);

	CUndo undo( "AnimSequence Modified" );
	CUndo::Record( new CUndoAnimSequenceObject(m_sequence) );

	Range timeRange;
	timeRange.start = m_startTime.GetValue();
	timeRange.end = m_endTime.GetValue();
	m_sequence->SetTimeRange( timeRange );

	CString name;
	m_nameEdit.GetWindowText( name );
	if (name != m_sequence->GetName())
	{
		// Rename sequence.
		m_sequence->SetName( name );
	}

	int seqFlags = m_sequence->GetFlags();
	seqFlags &= ~(IAnimSequence::ORT_CONSTANT | IAnimSequence::ORT_LOOP);

	if (m_alwaysPlayingBtn.GetCheck() == BST_CHECKED)
		seqFlags |= IAnimSequence::PLAY_ONRESET;
	else
		seqFlags &= (~IAnimSequence::PLAY_ONRESET);

	if (m_cutSceneBtn.GetCheck() == BST_CHECKED)
		seqFlags |= IAnimSequence::CUT_SCENE;
	else
		seqFlags &= (~IAnimSequence::CUT_SCENE);

	if (m_NoHUDBtn.GetCheck() == BST_CHECKED)
		seqFlags |= IAnimSequence::NO_HUD;
	else
		seqFlags &= (~IAnimSequence::NO_HUD);

	if (m_NoPlayerBtn.GetCheck() == BST_CHECKED)
		seqFlags |= IAnimSequence::NO_PLAYER;
	else
		seqFlags &= (~IAnimSequence::NO_PLAYER);

	if (m_NoPhysicsBtn.GetCheck() == BST_CHECKED)
		seqFlags |= IAnimSequence::NO_PHYSICS;
	else
		seqFlags &= (~IAnimSequence::NO_PHYSICS);

	if (m_NoAIBtn.GetCheck() == BST_CHECKED)
		seqFlags |= IAnimSequence::NO_AI;
	else
		seqFlags &= (~IAnimSequence::NO_AI);

	if (m_16To9.GetCheck() == BST_CHECKED)
		seqFlags |= IAnimSequence::IS_16TO9;
	else
		seqFlags &= (~IAnimSequence::IS_16TO9);
	
	if (m_outOfRange == 1)
		seqFlags |= IAnimSequence::ORT_CONSTANT;
	else if (m_outOfRange == 2)
		seqFlags |= IAnimSequence::ORT_LOOP;

	if (m_NoSoundsBtn.GetCheck() == BST_CHECKED)
		seqFlags |= IAnimSequence::NO_GAMESOUNDS;
	else
		seqFlags &= (~IAnimSequence::NO_GAMESOUNDS);
	
	m_sequence->SetFlags( seqFlags );
	
	CDialog::OnOK();
}
