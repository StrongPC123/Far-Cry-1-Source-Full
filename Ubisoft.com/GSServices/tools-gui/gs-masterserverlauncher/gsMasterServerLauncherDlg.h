// gsMasterServerLauncherDlg.h : header file
//

#if !defined(AFX_GSMASTERSERVERLAUNCHERDLG_H__18BF17A9_2273_4EE5_84FE_2AAA288837D1__INCLUDED_)
#define AFX_GSMASTERSERVERLAUNCHERDLG_H__18BF17A9_2273_4EE5_84FE_2AAA288837D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "RegServerLib.h"

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerLauncherDlg dialog

class CGsMasterServerLauncherDlg : public CDialog , public CRegisterServer
{
// Construction
public:
	bool m_bLoggedIn;
	CGsMasterServerLauncherDlg(CWnd* pParent = NULL);	// standard constructor
	~CGsMasterServerLauncherDlg();

// Dialog Data
	//{{AFX_DATA(CGsMasterServerLauncherDlg)
	enum { IDD = IDD_GSMASTERSERVERLAUNCHER_DIALOG };
	CEdit	m_editAltData;
	CEdit	m_editGamePort;
	CEdit	m_editLobbyPort;
	CEdit	m_editLobbyIP;
	CEdit	m_editGroupID;
	CEdit	m_editServerName;
	CEdit	m_editMaxVisitors;
	CEdit	m_editMaxPlayers;
	CEdit	m_editGSVersion;
	CEdit	m_editGrpData;
	CEdit	m_editGameVersion;
	CEdit	m_editGamePassword;
	CEdit	m_editGameData;
	CComboBox	m_cboServerType;
	CListCtrl	m_listGroups;
	CEdit	m_editExtIP;
	CEdit	m_editVersion;
	CEdit	m_editUsername;
	CEdit	m_editPort;
	CEdit	m_editPassword;
	CEdit	m_editIPAddress;
	CEdit	m_editGamename;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGsMasterServerLauncherDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation

		//UBI.COM: The Callbacks inherited from CRegisterServer
		GSvoid RegServerRcv_LoginRouterResult(GSubyte ucType, GSint lReason,
				const GSchar *szIPAddress);
		GSvoid RegServerRcv_RouterDisconnection();
		GSvoid RegServerRcv_RegisterServerResult(GSubyte pucType,GSint plReason,
				GSint iGroupID,const GSchar *szAddress,GSushort usPort,const GSchar *szSessionName);
		//GSvoid LauncherRcv_TerminateServerResult(GSubyte pucType, GSint plReason);
		GSvoid RegServerRcv_RequestParentGroupResult(GSubyte ucType, GSint lReason,
				GSint iServerID,GSint iGroupID, const GSchar *szGroupName, GSuint uiNbPlayers,
				GSuint uiMaxPlayers);	

		GSvoid RegServerRcv_LobbyServerLoginResults( GSubyte ucType, GSint iReason,
				GSint iLobbyServerID,	GSint iGroupID );
		GSvoid RegServerRcv_LobbyServerUpdateGroupSettingsResults( GSubyte ucType,
				GSint iReason, GSint iGroupID );
		GSvoid RegServerRcv_LobbyServerDisconnection();
		GSvoid RegServerRcv_LobbyServerMemberNew( const GSchar* szMember, GSbool bSpectator,
				const GSchar* szIPAddress, const GSchar* szAltIPAddress, const GSvoid* pPlayerInfo,
				GSuint uiPlayerInfoSize, GSushort usPlayerStatus );
		GSvoid RegServerRcv_LobbyServerMatchStartReply( GSubyte ucType, GSint iReason, GSint iGroupID );
		GSvoid RegServerRcv_LobbyServerMemberLeft( const GSchar* szMember );
		GSvoid RegServerRcv_LobbyServerMatchFinishReply( GSubyte ucType, GSint iReason, GSint iGroupID );
		GSvoid RegServerRcv_LobbyServerNewGroup ( GSushort usRoomType,
			const GSchar* szRoomName,GSint iGroupID,GSint iLobbyServerID,GSint iParentGroupID,
			GSint uiGroupConfig,GSshort sGroupLevel,const GSchar* szMaster,const GSchar* szAllowedGames,
			const GSchar* szGame,const GSvoid* pGroupInfo,GSuint GroupInfoSize,GSuint uiMatchEventID,
			GSuint uiMaxPlayers,GSuint uiNbPlayers,	GSuint uiMaxSpectators,	GSuint uiNbSpectators,
			const GSchar* szGameVersion,const GSchar* szGSGameVersion,const GSchar* szIPAddress,
			const GSchar* szAltIPAddress );
		GSvoid RegServerRcv_LobbyServerMemberUpdateInfo( const GSchar* szMember,
			const GSvoid* pPlayerInfo, GSuint uiPlayerInfoSize );
		GSvoid RegServerRcv_LobbyServerGroupConfigUpdate( GSuint uiGroupConfig, GSint iGroupID );
		GSvoid RegServerRcv_LobbyServerMemberUpdateStatus( const GSchar* szPlayer, GSushort usPlayerStatus );

private:
	int m_iNbrGroups;

protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CGsMasterServerLauncherDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBUTTONLogin();
	afx_msg void OnBUTTONReqGroups();
	afx_msg void OnBUTTONRegServer();
	afx_msg void OnBUTTONTermServer();
	afx_msg void OnBUTTONDisconnect();
	afx_msg void OnBUTTONUpdate();
	afx_msg void OnBUTTONDownload();
	afx_msg void OnBUTTONAbout();
	afx_msg void OnBUTTONStartMatch();
	afx_msg void OnBUTTONFinishMatch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GSMASTERSERVERLAUNCHERDLG_H__18BF17A9_2273_4EE5_84FE_2AAA288837D1__INCLUDED_)
