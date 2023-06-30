// TVSoundKeyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TVSoundKeyDialog.h"
#include <ISound.h>
#include ".\tvsoundkeydialog.h"

CString CTVSoundKeyDialog::m_sLastPath="sounds\\";

// CTVSoundKeyDialog dialog

IMPLEMENT_DYNAMIC(CTVSoundKeyDialog, CDialog)
CTVSoundKeyDialog::CTVSoundKeyDialog(CWnd* pParent /*=NULL*/)
	: IKeyDlg(CTVSoundKeyDialog::IDD, pParent)
{
	m_soundType = 0;
	m_track = 0;
	m_node = 0;
	m_key = 0;
}

CTVSoundKeyDialog::~CTVSoundKeyDialog()
{
}

void CTVSoundKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILENAME, m_browse);
	DDX_Radio(pDX,IDC_STEREO,m_soundType);
	DDX_Control(pDX, IDC_STREAM, m_streamBtn);
	DDX_Control(pDX, IDC_LOOP, m_loopBtn);
	DDX_Control(pDX, IDC_FILE, m_filenameCtrl );
}

BEGIN_MESSAGE_MAP(CTVSoundKeyDialog, CDialog)
	ON_EN_UPDATE(IDC_VOLUME, OnUpdateValue)
	ON_EN_CHANGE(IDC_FILENAME, OnUpdateValue)
	ON_EN_UPDATE(IDC_PAN, OnUpdateValue)
	ON_EN_UPDATE(IDC_INRADIUS, OnUpdateValue)
	ON_EN_UPDATE(IDC_OUTRADIUS, OnUpdateValue)
	ON_BN_CLICKED(IDC_STEREO, OnBnClickedStereo)
	ON_BN_CLICKED(IDC_3D, OnBnClicked3d)
	ON_BN_CLICKED(IDC_STREAM, OnBnClickedStream)
	ON_BN_CLICKED(IDC_LOOP, OnBnClickedLoop)
	ON_BN_CLICKED(IDC_BROWSE, OnBnClickedBrowse)
END_MESSAGE_MAP()

void CTVSoundKeyDialog::SetKey( IAnimNode *node,IAnimTrack *track,int nkey )
{
	assert( track );
	m_track = track;
	m_node = node;
	m_key = nkey;
	if (!m_track)
		return;
	ISoundKey key;
	m_track->GetKey( m_key,&key );
	m_filenameCtrl.SetWindowText(key.pszFilename);
	m_Volume.SetValue(key.nVolume);
	m_Pan.SetValue(key.nPan-127);
	m_InRadius.SetValue( key.inRadius );
	m_OutRadius.SetValue( key.outRadius );
	m_loopBtn.SetCheck( (key.bLoop)?BST_CHECKED:BST_UNCHECKED );
	m_streamBtn.SetCheck( (key.bStream)?BST_CHECKED:BST_UNCHECKED );
	m_soundType = (key.b3DSound)?1:0;
	UpdateData(FALSE);
	OnUpdateValue();
}

// CTVSoundKeyDialog message handlers
BOOL CTVSoundKeyDialog::OnInitDialog()
{
	IKeyDlg::OnInitDialog();

	m_Volume.Create(this, IDC_VOLUME);
	m_Volume.SetRange(0, 255);
	m_Volume.SetInteger(true);

	m_Pan.Create(this, IDC_PAN);
	m_Pan.SetRange(-127, +127);
	m_Pan.SetInteger(true);

	m_InRadius.Create(this, IDC_INRADIUS);
	m_OutRadius.Create(this, IDC_OUTRADIUS);
	m_InRadius.SetRange( 0,100000 );
	m_OutRadius.SetRange( 0,100000 );

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CTVSoundKeyDialog::OnUpdateValue()
{
	ISoundKey key;
	CString sFilename;
	m_track->GetKey( m_key,&key );
	m_filenameCtrl.GetWindowText(sFilename);
	strncpy(key.pszFilename, sFilename, sizeof(key.pszFilename));
	key.pszFilename[sizeof(key.pszFilename)-1]=0;
	key.nVolume=m_Volume.GetValue();
	key.nPan=m_Pan.GetValue()+127;
	key.inRadius = m_InRadius.GetValue();
	key.outRadius = m_OutRadius.GetValue();
	key.b3DSound = (m_soundType==1)?true:false;
	key.bStream = m_streamBtn.GetCheck() != 0;
	key.bLoop = m_loopBtn.GetCheck() != 0;
	_smart_ptr<ISound> pSound = GetIEditor()->GetSystem()->GetISoundSystem()->LoadSound(key.pszFilename, FLAG_SOUND_2D|FLAG_SOUND_LOAD_SYNCHRONOUSLY);
	if (pSound)
	{
		key.fDuration=(float)pSound->GetLengthMs()/1000.0f;
	}else
		key.fDuration=0.0f;
	m_track->SetKey(m_key, &key);
	GetIEditor()->GetAnimation()->ForceAnimation();
	RefreshTrackView();

	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CTVSoundKeyDialog::OnBnClickedStereo()
{
	UpdateData(TRUE);
	OnUpdateValue();
}

//////////////////////////////////////////////////////////////////////////
void CTVSoundKeyDialog::OnBnClicked3d()
{
	UpdateData(TRUE);
	OnUpdateValue();
}

//////////////////////////////////////////////////////////////////////////
void CTVSoundKeyDialog::OnBnClickedStream()
{
	UpdateData(TRUE);
	OnUpdateValue();
}

//////////////////////////////////////////////////////////////////////////
void CTVSoundKeyDialog::OnBnClickedLoop()
{
	UpdateData(TRUE);
	OnUpdateValue();
}

//////////////////////////////////////////////////////////////////////////
void CTVSoundKeyDialog::OnBnClickedBrowse()
{
	CString sRelFilename;
	m_filenameCtrl.GetWindowText(sRelFilename);
	if (CFileUtil::SelectSingleFile( EFILE_TYPE_SOUND,sRelFilename ))
	{
		CString sPath = Path::GetPath(sRelFilename);
		m_sLastPath=sPath;
		m_filenameCtrl.SetWindowText(sRelFilename);
		OnUpdateValue();
	}
	RefreshTrackView();
}
