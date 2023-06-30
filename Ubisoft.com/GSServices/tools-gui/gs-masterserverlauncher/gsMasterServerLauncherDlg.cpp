// gsMasterServerLauncherDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gsMasterServerLauncher.h"
#include "gsMasterServerLauncherDlg.h"
//#include "GSHttpInterface.h"
#include "wininet.h"
#include "define.h"
#include "LobbyDefines.h"
#include "Registry.h"

extern HANDLE g_Mutex;
bool g_bRun = GS_TRUE;

#include "version_manager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//UBI.COM: This thread just calls the Launcher_Engine function.  We do have to
//         make sure no to call it at the same time as one of the other functions,
//         therefore we use a Mutex.
DWORD ThreadEngine(void* pArg)
{
	CGsMasterServerLauncherDlg *pDlg = (CGsMasterServerLauncherDlg*)pArg;
	while (true)
	{
		Sleep(10);
		LOCKMUTEX();
		pDlg->RegServer_Engine();
		UNLOCKMUTEX();
	}
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerLauncherDlg dialog

CGsMasterServerLauncherDlg::CGsMasterServerLauncherDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGsMasterServerLauncherDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGsMasterServerLauncherDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_iNbrGroups = 0;
	m_bLoggedIn = false;
}

CGsMasterServerLauncherDlg::~CGsMasterServerLauncherDlg()
{
	//UBI.COM: Stop the thread from calling Launcher_Engine;
	LOCKMUTEX();
	//RegServerLibrary_Uninitialize();
}

void CGsMasterServerLauncherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGsMasterServerLauncherDlg)
	DDX_Control(pDX, IDC_EDITAltData, m_editAltData);
	DDX_Control(pDX, IDC_EDITGamePort, m_editGamePort);
	DDX_Control(pDX, IDC_EDITLobbyPort, m_editLobbyPort);
	DDX_Control(pDX, IDC_EDITLobbyIP, m_editLobbyIP);
	DDX_Control(pDX, IDC_EDITGroupID, m_editGroupID);
	DDX_Control(pDX, IDC_EDITServerName, m_editServerName);
	DDX_Control(pDX, IDC_EDITMaxVisitors, m_editMaxVisitors);
	DDX_Control(pDX, IDC_EDITMaxPlayers, m_editMaxPlayers);
	DDX_Control(pDX, IDC_EDITGSVersion, m_editGSVersion);
	DDX_Control(pDX, IDC_EDITGrpData, m_editGrpData);
	DDX_Control(pDX, IDC_EDITGameVersion, m_editGameVersion);
	DDX_Control(pDX, IDC_EDITGamePassword, m_editGamePassword);
	DDX_Control(pDX, IDC_EDITGameData, m_editGameData);
	DDX_Control(pDX, IDC_COMBOServerType, m_cboServerType);
	DDX_Control(pDX, IDC_LISTGroups, m_listGroups);
	DDX_Control(pDX, IDC_EDITExtIP, m_editExtIP);
	DDX_Control(pDX, IDC_EDITVersion, m_editVersion);
	DDX_Control(pDX, IDC_EDITUsername, m_editUsername);
	DDX_Control(pDX, IDC_EDITPort, m_editPort);
	DDX_Control(pDX, IDC_EDITPassword, m_editPassword);
	DDX_Control(pDX, IDC_EDITIPAddress, m_editIPAddress);
	DDX_Control(pDX, IDC_EDITGameName, m_editGamename);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGsMasterServerLauncherDlg, CDialog)
	//{{AFX_MSG_MAP(CGsMasterServerLauncherDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTONLogin, OnBUTTONLogin)
	ON_BN_CLICKED(IDC_BUTTONReqGroups, OnBUTTONReqGroups)
	ON_BN_CLICKED(IDC_BUTTONRegServer, OnBUTTONRegServer)
	ON_BN_CLICKED(IDC_BUTTONTermServer, OnBUTTONTermServer)
	ON_BN_CLICKED(IDC_BUTTONDisconnect, OnBUTTONDisconnect)
	ON_BN_CLICKED(IDC_BUTTONUpdate, OnBUTTONUpdate)
	ON_BN_CLICKED(IDC_BUTTONDownload, OnBUTTONDownload)
	ON_BN_CLICKED(IDC_BUTTONAbout, OnBUTTONAbout)
	ON_BN_CLICKED(IDC_BUTTONStartMatch, OnBUTTONStartMatch)
	ON_BN_CLICKED(IDC_BUTTONFinishMatch, OnBUTTONFinishMatch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerLauncherDlg message handlers

BOOL CGsMasterServerLauncherDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	//UBI.COM: Initialize the library
	//RegServerLibrary_Initialize();

	char szUsername[NICKNAMELENGTH],szGamename[GAMELENGTH],szPassword[PASSWORDLENGTH];
	char szVersion[VERSIONLENGTH];
	char szServerIP[IPADDRESSLENGTH],szServerPort[100];

	char szDirectory[1024];
	GetCurrentDirectory(1024,szDirectory);

	CString csLauncherIniFile = szDirectory;
	csLauncherIniFile += "\\Launcher.ini";

	GetPrivateProfileString("Launcher","Username",
				"user10",szUsername,NICKNAMELENGTH,csLauncherIniFile);
	GetPrivateProfileString("Launcher","Gamename",
				"GHOSTRECON",szGamename,GAMELENGTH,csLauncherIniFile);
	GetPrivateProfileString("Launcher","Password",
				"testtest",szPassword,PASSWORDLENGTH,csLauncherIniFile);
	GetPrivateProfileString("Launcher","Version",
				"PC4.2",szVersion,VERSIONLENGTH,csLauncherIniFile);
	GetPrivateProfileString("Launcher","ServerIP",
				"",szServerIP,IPADDRESSLENGTH,csLauncherIniFile);
	GetPrivateProfileString("Launcher","ServerPort",
				"",szServerPort,100,csLauncherIniFile);

	m_editGamename.SetWindowText(szGamename);
	m_editIPAddress.SetWindowText(szServerIP);
	m_editPassword.SetWindowText(szPassword);
	m_editPort.SetWindowText(szServerPort);
	m_editVersion.SetWindowText(szVersion);
	m_editUsername.SetWindowText(szUsername);

	char szServerName[NAMELENGTH],szMaxPlayers[100],szMaxVisitors[100];
	int iServerType;
	char szGamePassword[PASSWORDLENGTH],szGroupData[100];
	char szGameData[100],szGameVersion[VERSIONLENGTH];
	char szGSVersion[VERSIONLENGTH];
	char szGamePort[100];

	GetPrivateProfileString("GameServer","Name",
				"Test Server",szServerName,NAMELENGTH,csLauncherIniFile);
	iServerType = GetPrivateProfileInt("GameServer","Type",
				0,csLauncherIniFile);
	GetPrivateProfileString("GameServer","MaxPlayers",
				"4",szMaxPlayers,100,csLauncherIniFile);
	GetPrivateProfileString("GameServer","MaxVisitors",
				"0",szMaxVisitors,100,csLauncherIniFile);
	GetPrivateProfileString("GameServer","Password",
				"",szGamePassword,PASSWORDLENGTH,csLauncherIniFile);
	GetPrivateProfileString("GameServer","GroupData",
				"123",szGroupData,100,csLauncherIniFile);
	GetPrivateProfileString("GameServer","GameData",
				"456",szGameData,100,csLauncherIniFile);
	GetPrivateProfileString("GameServer","GameVersion",
				"1.0",szGameVersion,VERSIONLENGTH,csLauncherIniFile);
	GetPrivateProfileString("GameServer","GSVersion",
				"2.0",szGSVersion,VERSIONLENGTH,csLauncherIniFile);
	GetPrivateProfileString("GameServer","GamePort",
				"6667",szGamePort,100,csLauncherIniFile);


	m_editServerName.SetWindowText(szServerName);
	m_cboServerType.SetCurSel(iServerType);
	m_editMaxPlayers.SetWindowText(szMaxPlayers);
	m_editMaxVisitors.SetWindowText(szMaxVisitors);
	m_editGamePassword.SetWindowText(szGamePassword);
	m_editGrpData.SetWindowText(szGroupData);
	m_editGameData.SetWindowText(szGameData);
	m_editGameVersion.SetWindowText(szGameVersion);
	m_editGSVersion.SetWindowText(szGSVersion);
	m_editGamePort.SetWindowText(szGamePort);


	m_listGroups.InsertColumn(0,"Server ID",LVCFMT_LEFT,60);
	m_listGroups.InsertColumn(1,"Group ID",LVCFMT_LEFT,60);
	m_listGroups.InsertColumn(2,"Group Name",LVCFMT_LEFT,75);
	m_listGroups.InsertColumn(3,"Players",LVCFMT_LEFT,47);
	m_listGroups.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	//UBI.COM: Create the thread that keeps calling Launcher_Engine
	DWORD iThread;
	CreateThread(NULL,0,ThreadEngine,this,0,&iThread);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGsMasterServerLauncherDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGsMasterServerLauncherDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


//UBI.COM: Receive the LoginRouter result
GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LoginRouterResult(GSubyte ucType, GSint lReason,
		const GSchar *szIPAddress)
{
	if (ucType == GSSUCCESS)
	{
		MessageBox("Login Successful");
		m_editExtIP.SetWindowText(szIPAddress);
		m_bLoggedIn = true;
	}
	else
	{
		char szText[100];
		sprintf(szText,"Login Failed: %i",lReason);
		MessageBox(szText);
	}

}

//UBI.COM: Receive a disconnection from the Router
GSvoid CGsMasterServerLauncherDlg::RegServerRcv_RouterDisconnection()
{
	MessageBox("Login Disconnected");
	m_bLoggedIn = false;
}

//UBI.COM: Receive a RegisterServer result
GSvoid CGsMasterServerLauncherDlg::RegServerRcv_RegisterServerResult(GSubyte ucType,GSint lReason,
		GSint iGroupID,const GSchar *szAddress,GSushort usPort,const GSchar *szSessionName)
{
	if (ucType == GSFAIL)
	{
		char szText[100];
		sprintf(szText,"Register Failed: %i",lReason);
		MessageBox(szText);
		return;
	}



	int iConfig,iMaxPlayers,iMaxVisitors,iGroupDataSize,iGameDataSize;
	char szGroupData[100],szGameData[100],szPassword[PASSWORDLENGTH];
	char szText[100], szUsername[NICKNAMELENGTH];

	MessageBox("Register Server Sucessful");

	m_editGroupID.SetWindowText(ltoa(iGroupID,szText,10));
	m_editLobbyIP.SetWindowText(szAddress);
	m_editLobbyPort.SetWindowText(ltoa(usPort,szText,10));
	m_editServerName.SetWindowText(szSessionName);


	iConfig = 0;

	m_editMaxPlayers.GetWindowText(szText,100);
	iMaxPlayers = atol(szText);

	m_editMaxVisitors.GetWindowText(szText,100);
	iMaxVisitors = atol(szText);

	m_editGamePassword.GetWindowText(szPassword,100);

	m_editGrpData.GetWindowText(szGroupData,100);
	iGroupDataSize = strlen(szGroupData) + 1;

	m_editGameData.GetWindowText(szGameData,100);
	iGameDataSize = strlen(szGameData) + 1;

	m_editUsername.GetWindowText(szUsername,NICKNAMELENGTH);

	LOCKMUTEX();

	if (!RegServerSend_LobbyServerConnection(szAddress, usPort))
		MessageBox("RegServerSend_LobbyServerConnection failed");
	else
		RegServerSend_LobbyServerLogin(szUsername, iGroupID);

	UNLOCKMUTEX();

}

//UBI.COM: Receive a TerminateServer Result
/*GSvoid CGsMasterServerLauncherDlg::LauncherRcv_TerminateServerResult(GSubyte ucType, GSint lReason)
{
	if (ucType == GSSUCCESS)
	{
		MessageBox("Terminate Server Successful");
		m_editGroupID.SetWindowText("");
		m_editLobbyIP.SetWindowText("");
		m_editLobbyPort.SetWindowText("");
	}
	else
	{
		char szText[100];
		sprintf(szText,"Terminate Server Failed: %i",lReason);
		MessageBox(szText);
	}
}*/

//UBI.COM: Receive a RequestParentGroup result
GSvoid CGsMasterServerLauncherDlg::RegServerRcv_RequestParentGroupResult(GSubyte ucType, GSint lReason,
		GSint iServerID,GSint iGroupID, const GSchar *szGroupName, GSuint uiNbPlayers,
		GSuint uiMaxPlayers)
{
	if (ucType == GSSUCCESS)
	{
		//UBI.COM: If the iServerID is greater then 0 its a valid Server.
		//         Negative means we've received the complete list
		if (iServerID > 0)
		{
			char szText[100];

			//UBI.COM: Add the parent group to the control list
			m_listGroups.InsertItem(m_iNbrGroups,ltoa(iServerID,szText,10));
			m_listGroups.SetItemText(m_iNbrGroups,1,ltoa(iGroupID,szText,10));
			m_listGroups.SetItemText(m_iNbrGroups,2,szGroupName);
			m_listGroups.SetItemText(m_iNbrGroups,3,ltoa(uiNbPlayers,szText,10));
			m_iNbrGroups++;
		}
		else
		{
			MessageBox("Request Finished");
		}
	}
	else
	{
		char szText[100];
		sprintf(szText,"Request Failed: %i",lReason);
		MessageBox(szText);
	}

}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerLoginResults(
		GSubyte ucType, GSint iReason, GSint iLobbyServerID, GSint iGroupID )
{
	if (ucType == GSSUCCESS)
	{
		MessageBox("Lobby Server Login Success");
	}
	else
		MessageBox("Lobby Server Login Fail");

}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerUpdateGroupSettingsResults(
		GSubyte ucType, GSint iReason, GSint iGroupID )
{
	if (ucType == GSSUCCESS)
	{
		MessageBox("Update Success");
	}
	else
		MessageBox("Update Fail");
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerDisconnection()
{
	MessageBox("Lobby Server Disconnection");
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerMemberNew( const GSchar* szMember, GSbool bSpectator,
				const GSchar* szIPAddress, const GSchar* szAltIPAddress, const GSvoid* pPlayerInfo,
				GSuint uiPlayerInfoSize, GSushort usPlayerStatus )
{
	char szText[1024];
	sprintf(szText,"New Member %s",szMember);
	MessageBox(szText);
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerMatchStartReply( GSubyte ucType, GSint iReason, GSint iGroupID )
{
	char szText[1024];
	sprintf(szText,"Match Start");
	MessageBox(szText);
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerMemberLeft( const GSchar* szMember )
{
	char szText[1024];
	sprintf(szText,"Member Left %s",szMember);
	MessageBox(szText);
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerMatchFinishReply( GSubyte ucType, GSint iReason, GSint iGroupID )
{
	char szText[1024];
	sprintf(szText,"Match Finish ");
	MessageBox(szText);
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerMemberUpdateInfo( const GSchar* szMember,
		const GSvoid* pPlayerInfo, GSuint uiPlayerInfoSize )
{
	char szText[1024];
	sprintf(szText,"Member Info Update: %s",szMember);
	MessageBox(szText);
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerGroupConfigUpdate( GSuint uiGroupConfig, GSint iGroupID )
{
	char szText[1024];
	sprintf(szText,"Group Config Update: %i %i",uiGroupConfig,iGroupID);
	MessageBox(szText);
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerMemberUpdateStatus( const GSchar* szPlayer,
		GSushort usPlayerStatus )
{
	char szText[1024];
	sprintf(szText,"Member Status Update : %s %i",szPlayer,usPlayerStatus);
	MessageBox(szText);
}

GSvoid CGsMasterServerLauncherDlg::RegServerRcv_LobbyServerNewGroup ( GSushort usRoomType,
			const GSchar* szRoomName,GSint iGroupID,GSint iLobbyServerID,GSint iParentGroupID,
			GSint uiGroupConfig,GSshort sGroupLevel,const GSchar* szMaster,const GSchar* szAllowedGames,
			const GSchar* szGame,const GSvoid* pGroupInfo,GSuint GroupInfoSize,GSuint uiMatchEventID,
			GSuint uiMaxPlayers,GSuint uiNbPlayers,	GSuint uiMaxSpectators,	GSuint uiNbSpectators,
			const GSchar* szGameVersion,const GSchar* szGSGameVersion,const GSchar* szIPAddress,
			const GSchar* szAltIPAddress )
{
	char szText[1024];
	sprintf(szText,"Group Update : %s",szRoomName);
	MessageBox(szText);	
}


//UBI.COM: Connect to the GSRouter and Login
void CGsMasterServerLauncherDlg::OnBUTTONLogin() 
{
	char szIPAddress[IPADDRESSLENGTH];
	char szPort[100];
	GSushort usPort;
	char szUsername[NICKNAMELENGTH];
	char szPassword[PASSWORDLENGTH];
	char szVersion[VERSIONLENGTH];

	m_editIPAddress.GetWindowText(szIPAddress,IPADDRESSLENGTH);
	m_editPort.GetWindowText(szPort,100);
	usPort = (GSushort)atol(szPort);

	LOCKMUTEX();
	if (!RegServerSend_RouterConnect(szIPAddress,usPort))
	{
		MessageBox("RegServerSend_Connect Failed");
		UNLOCKMUTEX();
		return;
	}
	UNLOCKMUTEX();

	m_editUsername.GetWindowText(szUsername,NICKNAMELENGTH);
	m_editPassword.GetWindowText(szPassword,PASSWORDLENGTH);
	m_editVersion.GetWindowText(szVersion,VERSIONLENGTH);

	LOCKMUTEX();
	RegServerSend_LoginRouter(szUsername,szPassword,szVersion);
	UNLOCKMUTEX();
	
}

//UBI.COM: Request the list of Parent Groups
void CGsMasterServerLauncherDlg::OnBUTTONReqGroups() 
{
	char szGamename[GAMELENGTH];

	if (!m_bLoggedIn)
	{
		MessageBox("Not Logged In");
		return;
	}

	m_editGamename.GetWindowText(szGamename,GAMELENGTH);

	m_listGroups.DeleteAllItems();
	m_iNbrGroups = 0;

	LOCKMUTEX();
	RegServerSend_RequestParentGroupOnLobby(szGamename);
	UNLOCKMUTEX();

	
}

//UBI.COM: Register a server on the selected Parent Group
void CGsMasterServerLauncherDlg::OnBUTTONRegServer() 
{
	int iServerID;
	int iParentID;
	char szServerName[NAMELENGTH];
	char szGamename[GAMELENGTH];
	int iServerType;
	char szText[100];
	int iMaxPlayers;
	int iMaxVisitors;
	char szPassword[PASSWORDLENGTH];
	char szGroupData[100];
	int iGroupDataSize;
	char szAltData[100];
	int iAltDataSize;
	char szGameData[100];
	int iGameDataSize;
	char szGameVersion[VERSIONLENGTH];
	char szGSVersion[VERSIONLENGTH];
	int iGamePort;

	if (!m_bLoggedIn)
	{
		MessageBox("Not Logged In");
		return;
	}

	POSITION pos = m_listGroups.GetFirstSelectedItemPosition();
	if (pos == NULL)
	{
		MessageBox("No Group Seleted");
		return;
	}
	else
	{
		int iItem = m_listGroups.GetNextSelectedItem(pos);

		m_listGroups.GetItemText(iItem,0,szText,100);
		iServerID = atol(szText);
		
		m_listGroups.GetItemText(iItem,1,szText,100);
		iParentID = atol(szText);
	}


	m_editServerName.GetWindowText(szServerName,NAMELENGTH);

	m_editGamename.GetWindowText(szGamename,NAMELENGTH);

	int iSelection = m_cboServerType.GetCurSel();

	switch(iSelection)
	{
		case 0:
			iServerType = ROOM_HYBRID_REGSERVER;
			break;
		case 1:
			iServerType = ROOM_UBI_CLIENTHOST_REGSERVER;
			break;
		case 2:
			iServerType = ROOM_UBI_GAMESERVER_REGSERVER;
			break;
	}
	
	m_editMaxPlayers.GetWindowText(szText,100);
	iMaxPlayers = atol(szText);

	m_editMaxVisitors.GetWindowText(szText,100);
	iMaxVisitors = atol(szText);

	m_editGamePassword.GetWindowText(szPassword,100);

	m_editGrpData.GetWindowText(szGroupData,100);
	iGroupDataSize = strlen(szGroupData) + 1;

	m_editAltData.GetWindowText(szAltData,100);
	iAltDataSize = strlen(szAltData) + 1;

	m_editGameData.GetWindowText(szGameData,100);
	iGameDataSize = strlen(szGameData) + 5;

	GSubyte *pubGameData = (GSubyte*)malloc(iGameDataSize);

	memset(pubGameData,0,iGameDataSize);

	memcpy(pubGameData+4,szGameData,strlen(szGameData));


	m_editGameVersion.GetWindowText(szGameVersion,VERSIONLENGTH);
	m_editGSVersion.GetWindowText(szGSVersion,VERSIONLENGTH);

	m_editGamePort.GetWindowText(szText,100);
	iGamePort = atol(szText);

	LOCKMUTEX();
	RegServerSend_RegisterServerOnLobby(iServerID,iParentID,szServerName,szGamename,
			iServerType,iMaxPlayers,iMaxVisitors,szPassword,(GSubyte*)szGroupData,
			iGroupDataSize,(GSubyte*)szAltData,iAltDataSize,(GSubyte*)pubGameData,
			iGameDataSize,iGamePort,szGameVersion,szGSVersion,GS_FALSE,GS_FALSE);
	UNLOCKMUTEX();
	free(pubGameData);
}

//UBI.COM: Terminate the registered server.
void CGsMasterServerLauncherDlg::OnBUTTONTermServer() 
{
	LOCKMUTEX();
	RegServerSend_LobbyServerClose();
	UNLOCKMUTEX();	
}

//UBI.COM: Disconnect from the GSRouter
void CGsMasterServerLauncherDlg::OnBUTTONDisconnect() 
{
	LOCKMUTEX();
	//RegServerSend_LobbyServerClose();
	RegServerSend_RouterDisconnect();	
	UNLOCKMUTEX();
}

//UBI.COM: Change the registered server info
void CGsMasterServerLauncherDlg::OnBUTTONUpdate() 
{
	char szIPAddress[IPADDRESSLENGTH];
	GSushort usPort;
	int iMaxPlayers,iMaxVisitors,iGroupDataSize;
	int iGroupID,iGameDataSize,iGamePort,iAltDataSize;
	char szText[100],szPassword[PASSWORDLENGTH],szGroupData[100];
	char szGameData[100];
	char szAltData[100];

	m_editLobbyIP.GetWindowText(szIPAddress,IPADDRESSLENGTH);
	m_editLobbyPort.GetWindowText(szText,100);
	usPort = (GSushort)atol(szText);

	m_editGroupID.GetWindowText(szText,100);
	iGroupID = atol(szText);

	m_editMaxPlayers.GetWindowText(szText,100);
	iMaxPlayers = atol(szText);

	m_editMaxVisitors.GetWindowText(szText,100);
	iMaxVisitors = atol(szText);

	m_editGamePassword.GetWindowText(szPassword,100);

	m_editGrpData.GetWindowText(szGroupData,100);
	iGroupDataSize = strlen(szGroupData) + 1;

	m_editAltData.GetWindowText(szAltData,100);
	iAltDataSize = strlen(szAltData) + 1;

	m_editGameData.GetWindowText(szGameData,100);
	iGameDataSize = strlen(szGameData) + 1;

	m_editGamePort.GetWindowText(szText,100);
	iGamePort = atol(szText);

	LOCKMUTEX();
	RegServerSend_UpdateGroupSettings(iGroupID,-1,-1,-1,
			iMaxPlayers,iMaxVisitors,szPassword,(GSubyte*)szGroupData,iGroupDataSize,
			(GSubyte*)szAltData,iAltDataSize,(GSubyte*)szGameData,iGameDataSize,iGamePort);
	UNLOCKMUTEX();
}

void CGsMasterServerLauncherDlg::OnBUTTONDownload() 
{
	CRegistry registry(HKEY_LOCAL_MACHINE, CString("SOFTWARE\\Ubi Soft\\Game Service"),
			false,KEY_READ);
	CString csGSIniURL;

	char szDirectory[100];
	GetCurrentDirectory(100,szDirectory);

	CString csGSIniFile = szDirectory;
	csGSIniFile += "\\GS.ini";

	CString csLauncherIniFile = szDirectory;
	csLauncherIniFile += "\\Launcher.ini";

	registry.OpenKey();

	//UBI.COM: Try to read the ConnectURL from the registry
	//         If it fails we'll look in Launcher.ini
	if (!registry.ReadString(CString("ConnectURL"), &csGSIniURL))
	{
		char szUrl[1024];

		MessageBox("Registry failed; using Launcher.ini");

		GetPrivateProfileString("Launcher","ConnectURL",
				"http://gsconnect.ubisoft.com/gsinit.php?user=%s&dp=%s",szUrl,
				1024,csLauncherIniFile);
		csGSIniURL = szUrl;

	}
	registry.CloseKey();

	char szUsername[NICKNAMELENGTH],szGamename[GAMELENGTH];

	m_editUsername.GetWindowText(szUsername,NICKNAMELENGTH);
	m_editGamename.GetWindowText(szGamename,GAMELENGTH);

	//UBI.COM: Replace the strings in the URL
	csGSIniURL.Replace("user=%s",(CString("user=")+szUsername));
	csGSIniURL.Replace("dp=%s",(CString("dp=")+szGamename));

	//UBI.COM: Try to download the file GS.ini file if it fails
	//         we can use the old local one.
	HINTERNET hNet = InternetOpen("",INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,NULL);
	if (!hNet)
		MessageBox("InternetOpen Failed");
	else
	{
		HINTERNET hURL = InternetOpenUrl(hNet,csGSIniURL,NULL,0,INTERNET_FLAG_HYPERLINK,NULL);

		if (!hURL)
			MessageBox("InternetOpenUrl Failed");
		else
		{
			GSchar szBuffer[1024];
			GSint iSize = 1024;
			DWORD iRead = 0;

			FILE *pFile = fopen(csGSIniFile,"w");
			while (InternetReadFile(hURL,szBuffer,iSize,&iRead))
			{
				if (iRead != 0)
				{
					fwrite(szBuffer,sizeof(GSchar),iRead,pFile);
				}
				else
				{
					fclose(pFile);
					break;
				}
			}
		}
	}



	char szIPAddress[IPADDRESSLENGTH];
	char szPort[100];

	//UBI.COM: Read in the Address of the GSRouter.
	//         We'll only read in the first address, in a real application
	//         you should get both so that if one fails you can try the
	//         other.
	GetPrivateProfileString("Servers","RouterIP0","srv-gstest",szIPAddress,
			IPADDRESSLENGTH,csGSIniFile);
	//UBI.COM: Read in the Launcher Port of the GSRouter
	GetPrivateProfileString("Servers","RouterLauncherPort0","41005",szPort,
			100,csGSIniFile);

	m_editIPAddress.SetWindowText(szIPAddress);
	m_editPort.SetWindowText(szPort);

	MessageBox("Download Finished");
	return;
}

void CGsMasterServerLauncherDlg::OnBUTTONAbout() 
{
	char *szBuffer;
	unsigned int iBufferSize;

	CGSVersion ver;
	ver.PrintModulesVersion(NULL,&iBufferSize);

	szBuffer = (char*)malloc(iBufferSize);

	ver.PrintModulesVersion(szBuffer,&iBufferSize);

	MessageBox(szBuffer);
	free(szBuffer);	
}

void CGsMasterServerLauncherDlg::OnBUTTONStartMatch() 
{
	//GSchar szText[1024];
	//GSint iGroupID;

//	m_editGroupID.GetWindowText(szText,100);
//	iGroupID = atol(szText);


	//Send the mode
	RegServerSend_MatchStart(1);
}

void CGsMasterServerLauncherDlg::OnBUTTONFinishMatch() 
{

	RegServerSend_MatchFinish();
	
}
