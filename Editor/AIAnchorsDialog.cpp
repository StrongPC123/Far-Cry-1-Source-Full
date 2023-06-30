// AIAnchorsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "AIAnchorsDialog.h"
#include "AI\AIManager.h"

// CAIAnchorsDialog dialog

IMPLEMENT_DYNAMIC(CAIAnchorsDialog, CDialog)
CAIAnchorsDialog::CAIAnchorsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAIAnchorsDialog::IDD, pParent)
{
	m_sAnchor="";
}

CAIAnchorsDialog::~CAIAnchorsDialog()
{
}

void CAIAnchorsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANCHORS, m_wndAnchorsList);
}


BEGIN_MESSAGE_MAP(CAIAnchorsDialog, CDialog)
	ON_LBN_SELCHANGE(IDC_ANCHORS, OnLbnSelchangeAnchors)
END_MESSAGE_MAP()


// CAIAnchorsDialog message handlers

void CAIAnchorsDialog::OnLbnSelchangeAnchors()
{
	int nSel=m_wndAnchorsList.GetCurSel();
	if (nSel==LB_ERR)
		return;
	m_wndAnchorsList.GetText(nSel, m_sAnchor);
}


BOOL CAIAnchorsDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_wndAnchorsList.ResetContent();
	CAIManager *pAIMgr=GetIEditor()->GetAI();
	ASSERT(pAIMgr);
	typedef std::vector<CString> TAnchorActionsVec;
	TAnchorActionsVec vecAnchorActions;
	pAIMgr->GetAnchorActions(vecAnchorActions);
	int nSel=-1;
	for (TAnchorActionsVec::iterator It=vecAnchorActions.begin();It!=vecAnchorActions.end();++It)
	{
		int nLastId=m_wndAnchorsList.AddString(*It);
		if (m_sAnchor.CompareNoCase(*It)==0)
			nSel=nLastId;
	}
	m_wndAnchorsList.SetCurSel(nSel);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}