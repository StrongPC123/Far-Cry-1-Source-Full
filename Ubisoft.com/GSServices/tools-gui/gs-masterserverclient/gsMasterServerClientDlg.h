// gsMasterServerClientDlg.h : header file
//

#if !defined(AFX_GSMASTERSERVERCLIENTDLG_H__14918F3D_F9D7_4962_9438_7CB4C4CDEDB4__INCLUDED_)
#define AFX_GSMASTERSERVERCLIENTDLG_H__14918F3D_F9D7_4962_9438_7CB4C4CDEDB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MSClientClass.h"

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerClientDlg dialog

class CGsMasterServerClientDlg : public CDialog, public clMSClientClass
{
// Construction
public:
	void Shutdown();
	CGsMasterServerClientDlg(CWnd* pParent = NULL);	// standard constructor
	~CGsMasterServerClientDlg();
// Dialog Data
	//{{AFX_DATA(CGsMasterServerClientDlg)
	enum { IDD = IDD_GSMASTERSERVERCLIENT_DIALOG };

	CEdit	m_editGamePassword;
	CListCtrl	m_listGameServers;
	CEdit	m_editVersion;
	CEdit	m_editUsername;
	CEdit	m_editServerIP;
	CEdit	m_editPort;
	CEdit	m_editPassword;
	CEdit	m_editGameName;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGsMasterServerClientDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CGsMasterServerClientDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnExit();
	afx_msg void OnBUTTONInitialize();
	afx_msg void OnBUTTONRequest();
	afx_msg void OnBUTTONJoin();
	afx_msg void OnDblclkLISTGameServers(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBUTTONLeave();
	afx_msg void OnBUTTONUninitialize();
	afx_msg void OnBUTTONDownload();
	afx_msg void OnBUTTONAbout();
	afx_msg void OnBUTTONRefresh();
	afx_msg void OnBUTTONScores();
	afx_msg void OnBUTTONGetAlt();
	afx_msg void OnBUTTONStartMatch();
	afx_msg void OnBUTTONConnected();
	afx_msg void OnButtonGetMOTD();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	GSvoid GameServerCB(GSint iLobbyID,GSint iRoomID,
		GSshort siGroupType,GSchar *szGroupName, GSint iConfig,
		GSchar *szMaster,GSchar *szAllowedGames,GSchar *szGames,
		GSchar *szGameVersion,GSchar *szGSVersion,GSvoid *vpInfo,GSint iSize,
		GSuint uiMaxPlayer,GSuint uiNbrPlayer,GSuint uiMaxVisitor,
		GSuint uiNbrVisitor,GSchar *szIPAddress,GSchar *szAltIPAddress,
		GSint iEventID);

	GSvoid AlternateInfoCB(GSint iLobbyID,GSint iRoomID,const GSvoid* pcAltGroupInfo,
				GSint iAltGroupInfoSize);

	GSvoid ErrorCB(GSint iReason,GSint iLobbyId,GSint iRoomID);
	GSvoid InitFinishedCB(GSubyte ubType,GSint iError,GSchar *szUserName);
	GSvoid LoginDisconnectCB();
	GSvoid LobbyDisconnectCB();

	GSvoid RequestFinishedCB();

	GSvoid JoinFinishedCB(GSint iLobbyID,GSint iRoomID,GSvoid *vpGameData,GSint iSize,
			GSchar *szIPAddress,GSchar *szAltIPAddress,GSushort usPort);

	GSvoid AccountCreationCB(GSubyte ucType,GSint iReason);
	GSvoid ModifyAccountCB(GSubyte ucType,GSint iReason);

	GSvoid MatchStartedCB(GSint iLobbyID,GSint iRoomID,GSuint uiMatchID);
	GSvoid SubmitMatchCB(GSubyte ucType,GSint iReason,GSuint uiMatchID);

	GSvoid RequestMOTDCB(GSubyte ubType, GSchar *szUbiMOTD,
			GSchar *szGameMOTD, GSint iReason);

	GSbool m_bInitialized;
private:
	GSint m_iNbrGameServers;
	GSuint m_uiMatchID;
	GSint m_iLobbyID;
	GSint m_iRoomID;

	void JoinGameServer(GSint iItem);
	void LeaveGameServer(GSint iItem);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GSMASTERSERVERCLIENTDLG_H__14918F3D_F9D7_4962_9438_7CB4C4CDEDB4__INCLUDED_)
