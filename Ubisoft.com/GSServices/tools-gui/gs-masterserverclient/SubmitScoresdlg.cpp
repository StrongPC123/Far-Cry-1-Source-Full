// SubmitScoresdlg.cpp : implementation file
//

#include "stdafx.h"
#include "gsMasterServerClient.h"
#include "SubmitScoresdlg.h"
#include "GSTypes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSubmitScoresdlg dialog


CSubmitScoresdlg::CSubmitScoresdlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSubmitScoresdlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSubmitScoresdlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_uiMatchID = 0;
	m_iLobbyID = 0;
	m_iRoomID = 0;
	m_pParentDlg = (CGsMasterServerClientDlg*)pParent;

}


void CSubmitScoresdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSubmitScoresdlg)
	DDX_Control(pDX, IDC_EDITUsername, m_editUsername);
	DDX_Control(pDX, IDC_EDITLobbyID, m_editLobbyID);
	DDX_Control(pDX, IDC_EDITRoomID, m_editRoomID);
	DDX_Control(pDX, IDC_EDITMatchID, m_editMatchID);
	DDX_Control(pDX, IDC_EDITFieldValue, m_editFieldValue);
	DDX_Control(pDX, IDC_EDITFieldID, m_editFieldID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSubmitScoresdlg, CDialog)
	//{{AFX_MSG_MAP(CSubmitScoresdlg)
	ON_BN_CLICKED(IDC_BUTTONinit, OnBUTTONinit)
	ON_BN_CLICKED(IDC_BUTTONSet, OnBUTTONSet)
	ON_BN_CLICKED(IDC_BUTTONSubmit, OnBUTTONSubmit)
	ON_BN_CLICKED(IDC_BUTTONUninit, OnBUTTONUninit)
	ON_BN_CLICKED(IDC_BUTTONFinished, OnBUTTONFinished)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSubmitScoresdlg message handlers

GSvoid CSubmitScoresdlg::SetServerID(GSint iLobbyID,GSint iRoomID)
{
	m_iLobbyID = iLobbyID;
	m_iRoomID = iRoomID;
}

GSvoid CSubmitScoresdlg::SetMatchID(GSuint uiMatchID)
{
	m_uiMatchID = uiMatchID;
}

BOOL CSubmitScoresdlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	GSchar szText[1024];

	_snprintf(szText,1024,"%i",m_iLobbyID);

	m_editLobbyID.SetWindowText(szText);
	
	_snprintf(szText,1024,"%i",m_iRoomID);

	m_editRoomID.SetWindowText(szText);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSubmitScoresdlg::OnBUTTONinit() 
{
	m_pParentDlg->InitMatchResult(m_uiMatchID);
}

void CSubmitScoresdlg::OnBUTTONSet() 
{
	GSchar szUsername[NICKNAMELENGTH];
	GSchar szText[1024];
	GSuint uiFieldID;
	GSint iFieldValue;

	m_editUsername.GetWindowText(szUsername,NICKNAMELENGTH);

	m_editFieldID.GetWindowText(szText,1024);
	uiFieldID = atol(szText);

	m_editFieldValue.GetWindowText(szText,1024);
	iFieldValue = atol(szText);

	m_pParentDlg->SetMatchResult(szUsername,uiFieldID,iFieldValue);	
}

void CSubmitScoresdlg::OnBUTTONSubmit() 
{
	m_pParentDlg->SubmitMatchResult(m_iLobbyID,m_iRoomID);	
}

void CSubmitScoresdlg::OnBUTTONUninit() 
{
	m_pParentDlg->UninitMatchResult();
}

void CSubmitScoresdlg::OnBUTTONFinished() 
{
	m_pParentDlg->MatchFinished(m_iLobbyID,m_iRoomID);
}
