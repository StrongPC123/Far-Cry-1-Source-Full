#if !defined(AFX_SUBMITSCORESDLG_H__95720E33_2611_47EF_BA0C_4DD5EC46E091__INCLUDED_)
#define AFX_SUBMITSCORESDLG_H__95720E33_2611_47EF_BA0C_4DD5EC46E091__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gsMasterServerClientDlg.h"

// SubmitScoresdlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSubmitScoresdlg dialog

class CSubmitScoresdlg : public CDialog
{
// Construction
public:
	CSubmitScoresdlg(CWnd* pParent = NULL);   // standard constructor

	GSvoid SetServerID(GSint iLobbyID, GSint iRoomID);
	GSvoid SetMatchID(GSuint uiMatchID);

// Dialog Data
	//{{AFX_DATA(CSubmitScoresdlg)
	enum { IDD = IDD_DIALOGAddScores };
	CEdit	m_editUsername;
	CEdit	m_editLobbyID;
	CEdit	m_editRoomID;
	CEdit	m_editMatchID;
	CEdit	m_editFieldValue;
	CEdit	m_editFieldID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSubmitScoresdlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	GSint m_iLobbyID;
	GSint m_iRoomID;
	GSuint m_uiMatchID;
	CGsMasterServerClientDlg *m_pParentDlg;

	// Generated message map functions
	//{{AFX_MSG(CSubmitScoresdlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBUTTONinit();
	afx_msg void OnBUTTONSet();
	afx_msg void OnBUTTONSubmit();
	afx_msg void OnBUTTONUninit();
	afx_msg void OnBUTTONFinished();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SUBMITSCORESDLG_H__95720E33_2611_47EF_BA0C_4DD5EC46E091__INCLUDED_)
