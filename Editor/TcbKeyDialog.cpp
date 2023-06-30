// TcbKeyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TcbKeyDialog.h"

#include "AnimationContext.h"

#include "IMovieSystem.h"


// CTcbKeyDialog dialog

IMPLEMENT_DYNAMIC(CTcbKeyDialog, CDialog)
CTcbKeyDialog::CTcbKeyDialog(CWnd* pParent /*=NULL*/)
	: IKeyDlg(CTcbKeyDialog::IDD, pParent)
{
	m_track = 0;
	m_node = 0;
	m_key = -1;
}

CTcbKeyDialog::~CTcbKeyDialog()
{
}

void CTcbKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TCBPREVIEW, m_tcbPreview);
}


BEGIN_MESSAGE_MAP(CTcbKeyDialog, CDialog)
	ON_EN_UPDATE(IDC_XVALUE, OnUpdateValue)
	ON_EN_UPDATE(IDC_YVALUE, OnUpdateValue)
	ON_EN_UPDATE(IDC_ZVALUE, OnUpdateValue)
	ON_EN_UPDATE(IDC_WVALUE, OnUpdateValue)
	ON_EN_UPDATE(IDC_TENSION, OnUpdateValue)
	ON_EN_UPDATE(IDC_CONTINUITY, OnUpdateValue)
	ON_EN_UPDATE(IDC_BIAS, OnUpdateValue)
	ON_EN_UPDATE(IDC_EASETO, OnUpdateValue)
	ON_EN_UPDATE(IDC_EASEFROM, OnUpdateValue)
END_MESSAGE_MAP()


// CTcbKeyDialog message handlers

BOOL CTcbKeyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_value[0].Create( this,IDC_XVALUE );
	m_value[1].Create( this,IDC_YVALUE );
	m_value[2].Create( this,IDC_ZVALUE );
	//m_value[3].Create( this,IDC_WVALUE );

	m_tcb[0].Create( this,IDC_TENSION );
	m_tcb[1].Create( this,IDC_CONTINUITY );
	m_tcb[2].Create( this,IDC_BIAS );
	m_tcb[3].Create( this,IDC_EASETO );
	m_tcb[4].Create( this,IDC_EASEFROM );

	m_value[0].SetRange( -10000,10000 );
	m_value[1].SetRange( -10000,10000 );
	m_value[2].SetRange( -10000,10000 );
	//m_value[3].SetRange( -10000,10000 );

	m_tcb[0].SetRange( -1,1 );
	m_tcb[0].SetInternalPrecision(3);
	m_tcb[0].SetValue(0.0f, 0.001f);
	m_tcb[1].SetRange( -1,1 );
	m_tcb[1].SetInternalPrecision(3);
	m_tcb[1].SetValue(0.0f, 0.001f);
	m_tcb[2].SetRange( -1,1 );
	m_tcb[2].SetInternalPrecision(3);
	m_tcb[2].SetValue(0.0f, 0.001f);
	m_tcb[3].SetRange( 0,1 );
	m_tcb[3].SetInternalPrecision(3);
	m_tcb[3].SetValue(0.0f, 0.001f);
	m_tcb[4].SetRange( 0,1 );
	m_tcb[4].SetInternalPrecision(3);
	m_tcb[4].SetValue(0.0f, 0.001f);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CTcbKeyDialog::SetKey( IAnimNode *node,IAnimTrack *track,int nkey )
{
	assert( track );

	// Store previous data.
	if (node != m_node || track != m_track || nkey != m_key)
		OnUpdateValue();

	EAnimTrackType trType = track->GetType();
	assert( trType == ATRACK_TCB_FLOAT || trType == ATRACK_TCB_VECTOR || trType == ATRACK_TCB_QUAT );

	m_track = track;
	m_node = node;
	m_key = nkey;

	SetKeyControls( nkey );
}

//////////////////////////////////////////////////////////////////////////
void CTcbKeyDialog::SetKeyControls( int nkey )
{
	ITcbKey key;

	if (nkey < 0)
		nkey = 0;
	if (nkey > m_track->GetNumKeys()-1)
		nkey = m_track->GetNumKeys()-1;

	EAnimValue valueType = m_track->GetValueType();
	m_track->GetKey( nkey,&key );
	if (valueType == AVALUE_FLOAT)
	{
		m_value[0].SetValue( key.GetFloat() );
		m_value[0].EnableWindow(TRUE);
		m_value[1].EnableWindow(FALSE);
		m_value[2].EnableWindow(FALSE);
		//m_value[3].EnableWindow(FALSE);
	}
	else if (valueType == AVALUE_VECTOR)
	{
		m_value[0].SetValue( key.GetVec3().x );
		m_value[1].SetValue( key.GetVec3().y );
		m_value[2].SetValue( key.GetVec3().z );

		m_value[0].EnableWindow(TRUE);
		m_value[1].EnableWindow(TRUE);
		m_value[2].EnableWindow(TRUE);
		//m_value[3].EnableWindow(FALSE);
	}
	else if (valueType == AVALUE_QUAT)
	{
		Vec3 angles = Ang3::GetAnglesXYZ(Matrix33(key.GetQuat())) * 180.0f/gf_PI;
		m_value[0].SetValue( angles.x );
		m_value[1].SetValue( angles.y );
		m_value[2].SetValue( angles.z );
		//m_value[3].SetValue( key.GetQuat().w );

		m_value[0].EnableWindow(TRUE);
		m_value[1].EnableWindow(TRUE);
		m_value[2].EnableWindow(TRUE);
		//m_value[3].EnableWindow(TRUE);
	}

	m_tcb[0].SetValue( key.tens );
	m_tcb[1].SetValue( key.cont );
	m_tcb[2].SetValue( key.bias );
	m_tcb[3].SetValue( key.easeto );
	m_tcb[4].SetValue( key.easefrom );
	
	m_tcbPreview.SetTcb( key.tens,key.cont,key.bias,key.easeto,key.easefrom );
}

void CTcbKeyDialog::OnUpdateValue()
{
	if (!m_node)
		return;
	if (!m_track)
		return;
	if (m_key < 0)
		return;

	ITcbKey key;
	m_track->GetKey( m_key,&key );

	bool bUndoRecorded = false;

	EAnimValue valueType = m_track->GetValueType();
	m_track->GetKey( m_key,&key );
	if (valueType == AVALUE_FLOAT)
	{
		float val = m_value[0].GetValue();
		if (key.GetFloat() != val)
		{
			bUndoRecorded = true;
			RecordTrackUndo();
		}

		key.SetValue(val);
	}
	else if (valueType == AVALUE_VECTOR)
	{
		Vec3 vec(m_value[0].GetValue(),m_value[1].GetValue(),m_value[2].GetValue());

		if (vec.x != key.GetVec3().x || vec.y != key.GetVec3().y || vec.z != key.GetVec3().z)
		{
			bUndoRecorded = true;
			RecordTrackUndo();
		}

		key.SetValue(vec);
	}
	else if (valueType == AVALUE_QUAT)
	{
		Vec3 angles( m_value[0].GetValue(),m_value[1].GetValue(),m_value[2].GetValue() );
		Quat quat;
		quat.SetRotationXYZ( angles * gf_PI/180.0f );

		if (quat.v.x != key.GetQuat().v.x || quat.v.y != key.GetQuat().v.y || quat.v.z != key.GetQuat().v.z || quat.w != key.GetQuat().w)
		{
			bUndoRecorded = true;
			RecordTrackUndo();
		}

		key.SetValue(quat);
	}
	
	if (!bUndoRecorded)
	{
		if (key.tens != m_tcb[0].GetValue() ||
				key.cont != m_tcb[1].GetValue() ||
				key.bias != m_tcb[2].GetValue() ||
				key.easeto != m_tcb[3].GetValue() ||
				key.easefrom != m_tcb[4].GetValue()
				)
		{
			bUndoRecorded = true;
			RecordTrackUndo();
		}
	}
	key.tens =  m_tcb[0].GetValue();
	key.cont = m_tcb[1].GetValue();
	key.bias = m_tcb[2].GetValue();
	key.easeto = m_tcb[3].GetValue();
	key.easefrom = m_tcb[4].GetValue();

	m_tcbPreview.SetTcb( key.tens,key.cont,key.bias,key.easeto,key.easefrom );

	m_track->SetKey( m_key,&key );
	GetIEditor()->GetAnimation()->ForceAnimation();
	RefreshTrackView();
}