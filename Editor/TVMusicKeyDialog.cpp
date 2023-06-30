// TVSoundKeyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TVMusicKeyDialog.h"
#include <ISound.h>

// CTVMusicKeyDialog dialog

IMPLEMENT_DYNAMIC(CTVMusicKeyDialog, CDialog)
CTVMusicKeyDialog::CTVMusicKeyDialog(CWnd* pParent /*=NULL*/)
	: IKeyDlg(CTVMusicKeyDialog::IDD, pParent)
{
	m_track = 0;
	m_node = 0;
	m_key = 0;
}

CTVMusicKeyDialog::~CTVMusicKeyDialog()
{
}

void CTVMusicKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NAME, m_Mood);
}

BEGIN_MESSAGE_MAP(CTVMusicKeyDialog, CDialog)
	ON_EN_UPDATE(IDC_NAME, OnUpdateValue)
	ON_EN_UPDATE(IDC_TIME, OnUpdateValue)
	ON_BN_CLICKED(IDC_SELMOOD, OnUpdateValue)
	ON_BN_CLICKED(IDC_VOLRAMP, OnUpdateValue)
END_MESSAGE_MAP()

void CTVMusicKeyDialog::SetKey( IAnimNode *node,IAnimTrack *track,int nkey )
{
	assert( track );
	m_track = track;
	m_node = node;
	m_key = nkey;
	if (!m_track)
		return;
	IMusicKey key;
	m_track->GetKey( m_key,&key );
	((CButton*)GetDlgItem(IDC_SELMOOD))->SetCheck((key.eType==eMusicKeyType_SetMood) ? BST_CHECKED : BST_UNCHECKED);
	((CButton*)GetDlgItem(IDC_VOLRAMP))->SetCheck((key.eType==eMusicKeyType_VolumeRamp) ? BST_CHECKED : BST_UNCHECKED);
	m_Mood.SetWindowText(key.szMood);
	m_Time.SetValue(key.fTime);
	OnUpdateValue();
}

// CTVMusicKeyDialog message handlers
BOOL CTVMusicKeyDialog::OnInitDialog()
{
	IKeyDlg::OnInitDialog();

	m_Time.Create(this, IDC_TIME);
	m_Time.SetRange(0, 60);
	m_Time.SetInteger(false);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTVMusicKeyDialog::OnUpdateValue()
{
	IMusicKey key;
	m_track->GetKey( m_key,&key );
	if (((CButton*)GetDlgItem(IDC_SELMOOD))->GetState() & 0x0003)
		key.eType=eMusicKeyType_SetMood;
	else if (((CButton*)GetDlgItem(IDC_VOLRAMP))->GetState() & 0x0003)
		key.eType=eMusicKeyType_VolumeRamp;
	CString sMood;
	m_Mood.GetWindowText(sMood);
	strncpy(key.szMood, sMood, sizeof(key.szMood));
	key.szMood[sizeof(key.szMood)-1]=0;
	key.fTime=m_Time.GetValue();
	m_track->SetKey(m_key, &key);
	GetIEditor()->GetAnimation()->ForceAnimation();
	RefreshTrackView();
}