// TVSelectKeyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TVSelectKeyDialog.h"
#include "ientitysystem.h"
#include "objects/objectmanager.h"
#include "objects/entity.h"
#include "objects/CameraObject.h"

// CTVSelectKeyDialog dialog

IMPLEMENT_DYNAMIC(CTVSelectKeyDialog, CDialog)
CTVSelectKeyDialog::CTVSelectKeyDialog(CWnd* pParent /*=NULL*/)
	: IKeyDlg(CTVSelectKeyDialog::IDD, pParent)
{
	m_track = 0;
	m_node = 0;
	m_key = 0;
}

CTVSelectKeyDialog::~CTVSelectKeyDialog()
{
}

void CTVSelectKeyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_NAME, m_name);
	DDX_Control(pDX, IDC_ID, m_id);
}


BEGIN_MESSAGE_MAP(CTVSelectKeyDialog, CDialog)
	ON_CBN_EDITCHANGE(IDC_NAME, OnUpdateValue)
	ON_CBN_SELCHANGE(IDC_NAME, OnCbnSelchangeName)
END_MESSAGE_MAP()


// CTVSelectKeyDialog message handlers

BOOL CTVSelectKeyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTVSelectKeyDialog::SetKey( IAnimNode *node,IAnimTrack *track,int nkey )
{
	assert( track );

	m_track = track;
	m_node = node;
	m_key = nkey;
	if (!m_track)
		return;

	int nParamId=node->FindTrack(track);
	assert(nParamId!=-1);

	std::vector<CBaseObject*> objects;
	IMovieSystem *pMovieSystem;
	ISequenceIt *pSeqIt;
	IAnimSequence *pSeq;
	// fill list with available items
	m_name.ResetContent();
	switch (nParamId)
	{
		case APARAM_CAMERA:
			// Get All entity nodes
			GetIEditor()->GetObjectManager()->GetObjects( objects );
			for (int i = 0; i < objects.size(); i++)
			{
				if (objects[i]->IsKindOf(RUNTIME_CLASS(CCameraObject)))
					m_name.AddString( objects[i]->GetName() );
			}
			break;
		case APARAM_SEQUENCE:
			pMovieSystem=GetIEditor()->GetSystem()->GetIMovieSystem();
			pSeqIt=pMovieSystem->GetSequences();
			pSeq=pSeqIt->first();
			while (pSeq)
			{
				if (pSeq!=GetIEditor()->GetAnimation()->GetSequence())
					m_name.AddString(pSeq->GetName());
				pSeq=pSeqIt->next();
			}
			pSeqIt->Release();
			break;
	}
	
	ISelectKey key;
	m_track->GetKey( m_key,&key );

	m_name.SetWindowText( key.szSelection);
	IAnimNode *camNode = node->GetMovieSystem()->FindNode(key.szSelection);
	if (camNode)
	{
		char sId[32];
		sprintf( sId,"%u",camNode->GetId() );
		m_id.SetWindowText( sId );
	}
	else
	{
		m_id.SetWindowText("0");
	}
}

void CTVSelectKeyDialog::OnUpdateValue()
{
	if (!m_track)
		return;
	//EAnimValue valueType = m_track->GetValueType();
	ISelectKey key;
	m_track->GetKey( m_key,&key );

	CString sName;
	m_name.GetWindowText(sName);
	m_id.SetWindowText("0");

	strncpy( key.szSelection,sName,sizeof(key.szSelection) );
	key.szSelection[sizeof(key.szSelection)-1] = '\0';

	if (sName.GetLength())
	{
		CBaseObject *pObj=GetIEditor()->GetObjectManager()->FindObject(sName);
		if (pObj && (pObj->GetType()==OBJTYPE_ENTITY))
		{
			char sId[32];
			sprintf( sId,"%u",pObj->GetId() );
			m_id.SetWindowText( sId );
		}
	}
	IAnimSequence *pSequence = GetIEditor()->GetSystem()->GetIMovieSystem()->FindSequence(key.szSelection);
	if (pSequence)
	{
		key.fDuration = pSequence->GetTimeRange().Length();
	}

	m_track->SetKey( m_key,&key );
	GetIEditor()->GetAnimation()->ForceAnimation();
	RefreshTrackView();
}

void CTVSelectKeyDialog::OnCbnSelchangeName()
{
	CString sStr;
	m_name.GetLBText(m_name.GetCurSel(), sStr);
	m_name.SetWindowText(sStr);
	OnUpdateValue();
}
