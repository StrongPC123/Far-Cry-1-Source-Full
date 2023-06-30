// gsMasterServerClientDlg.cpp : implementation file
//

#include "stdafx.h"
#include "gsMasterServerClient.h"
#include "gsMasterServerClientDlg.h"
#include "SubmitScoresdlg.h"
//#include "GSHttpInterface.h"
#include "wininet.h"
#include "define.h"
#include "Registry.h"

#include "version_manager.h"

extern HANDLE g_Mutex;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerClientDlg dialog

CGsMasterServerClientDlg::CGsMasterServerClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGsMasterServerClientDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGsMasterServerClientDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bInitialized = false;
	m_iLobbyID = 0;
	m_iRoomID = 0;
	m_uiMatchID = 0;
}

CGsMasterServerClientDlg::~CGsMasterServerClientDlg()
{
	LOCKMUTEX();
}
void CGsMasterServerClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGsMasterServerClientDlg)
	DDX_Control(pDX, IDC_EDITGamePassword, m_editGamePassword);
	DDX_Control(pDX, IDC_LISTGameServers, m_listGameServers);
	DDX_Control(pDX, IDC_EDITVersion, m_editVersion);
	DDX_Control(pDX, IDC_EDITUsername, m_editUsername);
	DDX_Control(pDX, IDC_EDITServerIP, m_editServerIP);
	DDX_Control(pDX, IDC_EDITPort, m_editPort);
	DDX_Control(pDX, IDC_EDITPassword, m_editPassword);
	DDX_Control(pDX, IDC_EDITGameName, m_editGameName);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGsMasterServerClientDlg, CDialog)
	//{{AFX_MSG_MAP(CGsMasterServerClientDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDEXIT, OnExit)
	ON_BN_CLICKED(IDC_BUTTONInitialize, OnBUTTONInitialize)
	ON_BN_CLICKED(IDC_BUTTONRequest, OnBUTTONRequest)
	ON_BN_CLICKED(IDC_BUTTONJoin, OnBUTTONJoin)
	ON_NOTIFY(NM_DBLCLK, IDC_LISTGameServers, OnDblclkLISTGameServers)
	ON_BN_CLICKED(IDC_BUTTONLeave, OnBUTTONLeave)
	ON_BN_CLICKED(IDC_BUTTONUninitialize, OnBUTTONUninitialize)
	ON_BN_CLICKED(IDC_BUTTONDownload, OnBUTTONDownload)
	ON_BN_CLICKED(IDC_BUTTONAbout, OnBUTTONAbout)
	ON_BN_CLICKED(IDC_BUTTONRefresh, OnBUTTONRefresh)
	ON_BN_CLICKED(IDC_BUTTONScores, OnBUTTONScores)
	ON_BN_CLICKED(IDC_BUTTONGetAlt, OnBUTTONGetAlt)
	ON_BN_CLICKED(IDC_BUTTONStartMatch, OnBUTTONStartMatch)
	ON_BN_CLICKED(IDC_BUTTONConnected, OnBUTTONConnected)
	ON_BN_CLICKED(IDC_BUTTON_GETMOTD,OnButtonGetMOTD)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGsMasterServerClientDlg message handlers

//UBI.COM: This thread just calls the MSClient_Engine function.  It is safe
//         to call this even thought the library has not been initialized,
//         but we do have to make sure no to call it at the same time as
//         one of the other functions, therefore we use a Mutex.
DWORD ThreadEngine(void* pArg)
{
	CGsMasterServerClientDlg *pMSClient = (CGsMasterServerClientDlg *)pArg;
	while (true)
	{
		Sleep(10);
		LOCKMUTEX();
		if (pMSClient->m_bInitialized)
			pMSClient->Engine();
		UNLOCKMUTEX();
	}
	return 0;
}

BOOL CGsMasterServerClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	char szUsername[NICKNAMELENGTH],szGamename[GAMELENGTH],szPassword[PASSWORDLENGTH];
	char szVersion[VERSIONLENGTH],szGamePassword[PASSWORDLENGTH];
	char szServerIP[IPADDRESSLENGTH],szServerPort[100];

	char szDirectory[1024];
	GetCurrentDirectory(1024,szDirectory);

	CString csMSClientIniFile = szDirectory;
	csMSClientIniFile += "\\MSClient.ini";

	GetPrivateProfileString("MSClient","Username",
				"user10",szUsername,NICKNAMELENGTH,csMSClientIniFile);
	GetPrivateProfileString("MSClient","Gamename",
				"GHOSTRECON",szGamename,GAMELENGTH,csMSClientIniFile);
	GetPrivateProfileString("MSClient","Password",
				"testtest",szPassword,PASSWORDLENGTH,csMSClientIniFile);
	GetPrivateProfileString("MSClient","Version",
				"PC4.2",szVersion,VERSIONLENGTH,csMSClientIniFile);
	GetPrivateProfileString("MSClient","GamePassword",
				"",szGamePassword,PASSWORDLENGTH,csMSClientIniFile);
	GetPrivateProfileString("MSClient","ServerIP",
				"",szServerIP,IPADDRESSLENGTH,csMSClientIniFile);
	GetPrivateProfileString("MSClient","ServerPort",
				"",szServerPort,100,csMSClientIniFile);


	m_editGameName.SetWindowText(szGamename);
	m_editPassword.SetWindowText(szPassword);
	m_editPort.SetWindowText(szServerPort);
	m_editServerIP.SetWindowText(szServerIP);
	m_editUsername.SetWindowText(szUsername);
	m_editVersion.SetWindowText(szVersion);
	m_editGamePassword.SetWindowText(szGamePassword);

	m_listGameServers.InsertColumn(0,"LobbyID",LVCFMT_LEFT,60);
	m_listGameServers.InsertColumn(1,"RoomID",LVCFMT_LEFT,60);
	m_listGameServers.InsertColumn(2,"Server Name",LVCFMT_LEFT,85);
	m_listGameServers.InsertColumn(3,"Master Name",LVCFMT_LEFT,75);
	m_listGameServers.InsertColumn(4,"Version",LVCFMT_LEFT,60);
	m_listGameServers.InsertColumn(5,"Players",LVCFMT_LEFT,60);
	m_listGameServers.InsertColumn(6,"IPAddress",LVCFMT_LEFT,60);
	m_listGameServers.InsertColumn(7,"Alt IPAddress",LVCFMT_LEFT,75);
	m_listGameServers.InsertColumn(8,"GroupData",LVCFMT_LEFT,75);

	m_listGameServers.SetExtendedStyle(LVS_EX_FULLROWSELECT);


	//UBI.COM: Create the thread that keeps calling MSClient_Engine
	DWORD iThread;
	CreateThread(NULL,0,ThreadEngine,this,0,&iThread);
	
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGsMasterServerClientDlg::OnPaint() 
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
HCURSOR CGsMasterServerClientDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CGsMasterServerClientDlg::OnExit() 
{
	Shutdown();
}

//UBI.COM: Uninitialize the library and close the window.
void CGsMasterServerClientDlg::Shutdown()
{
	LOCKMUTEX();
	Uninitialize();
	m_bInitialized = false;

	UNLOCKMUTEX();
	this->DestroyWindow();
}

void CGsMasterServerClientDlg::OnBUTTONInitialize() 
{
	GSchar szServer[IPADDRESSLENGTH];
	GSchar szPort[10];
	GSushort usPort;
	GSchar szUsername[NICKNAMELENGTH];
	GSchar szPassword[PASSWORDLENGTH];
	GSchar szVersion[VERSIONLENGTH];
	
	m_editServerIP.GetWindowText(szServer,IPADDRESSLENGTH);
	m_editPort.GetWindowText(szPort,10);
	usPort = (GSushort)atol(szPort);
	m_editUsername.GetWindowText(szUsername,NICKNAMELENGTH);
	m_editPassword.GetWindowText(szPassword,PASSWORDLENGTH);
	m_editVersion.GetWindowText(szVersion,VERSIONLENGTH);


	//UBI.COM: Initialize the library
	LOCKMUTEX();
	if (!Initialize(szServer,usPort,szUsername,szPassword,szVersion))
	{
		MessageBox("Initialize Failed");
		UNLOCKMUTEX();
		return;
	}
	m_bInitialized = true;

	UNLOCKMUTEX();

}

//UBI.COM: Callbacks

//UBI.COM: Add the recieved Game Server to the list box.
GSvoid CGsMasterServerClientDlg::GameServerCB(GSint iLobbyID,GSint iRoomID,
		GSshort siGroupType,GSchar *szGroupName, GSint iConfig,
		GSchar *szMaster,GSchar *szAllowedGames,GSchar *szGames,
		GSchar *szGameVersion,GSchar *szGSVersion,GSvoid *vpInfo,GSint iSize,
		GSuint uiMaxPlayer,GSuint uiNbrPlayer,GSuint uiMaxVisitor,
		GSuint uiNbrVisitor,GSchar *szIPAddress,GSchar *szAltIPAddress,
		GSint iEventID)
{
	GSchar szText[100];

	m_listGameServers.InsertItem(m_iNbrGameServers,ltoa(iLobbyID,szText,10));
	m_listGameServers.SetItemText(m_iNbrGameServers,1,ltoa(iRoomID,szText,10));
	m_listGameServers.SetItemText(m_iNbrGameServers,2,szGroupName);
	m_listGameServers.SetItemText(m_iNbrGameServers,3,szMaster);
	m_listGameServers.SetItemText(m_iNbrGameServers,4,szGSVersion);

	sprintf(szText,"%i/%i",uiNbrPlayer,uiMaxPlayer);
	m_listGameServers.SetItemText(m_iNbrGameServers,5,szText);

	m_listGameServers.SetItemText(m_iNbrGameServers,6,szIPAddress);
	m_listGameServers.SetItemText(m_iNbrGameServers,7,szAltIPAddress);
	m_listGameServers.SetItemText(m_iNbrGameServers,8,(char*)vpInfo);

	m_iNbrGameServers++;
}

GSvoid CGsMasterServerClientDlg::AlternateInfoCB(GSint iLobbyID,GSint iRoomID,const GSvoid* pcAltGroupInfo,
				GSint iAltGroupInfoSize)
{
	GSchar szText[100];
	sprintf(szText,"AltInfo: %s",pcAltGroupInfo);
	MessageBox(szText);
}

//UBI.COM: Display a message box with the error code received
GSvoid CGsMasterServerClientDlg::ErrorCB(GSint iReason,GSint iLobbyID,GSint iRoomID)
{
	GSchar szText[100];
	sprintf(szText,"Error Code: %i %i %i",iReason,iLobbyID,iRoomID);
	MessageBox(szText);
}

//UBI.COM: Display a message box saying that the init has finished
GSvoid CGsMasterServerClientDlg::InitFinishedCB(GSubyte ubType,GSint iError,GSchar *szUserName)
{
	GSchar szText[100];
	if (ubType == GSSUCCESS)
	{
		sprintf(szText,"Initialization Finished %s",szUserName);
		MessageBox(szText);
	}
	else
	{
		sprintf(szText,"Initialization Error : %i",iError);
		MessageBox(szText);
	}
		
}

//UBI.COM: Display a message box that we've been disconnect from the Router
GSvoid CGsMasterServerClientDlg::LoginDisconnectCB()
{
	GSchar szText[100];
	sprintf(szText,"Login Disconnected\n");
	MessageBox(szText);
}

//UBI.COM: Display a message box that we've been disconnect from the Lobby
GSvoid CGsMasterServerClientDlg::LobbyDisconnectCB()
{
	GSchar szText[100];
	sprintf(szText,"Lobby Disconnected\n");
	MessageBox(szText);
}

//UBI.COM: Display a message box saying that the request has finished
GSvoid CGsMasterServerClientDlg::RequestFinishedCB()
{
	MessageBox("Request Finished");
}

//UBI.COM: Display a message box saying that the join has finished
GSvoid CGsMasterServerClientDlg::JoinFinishedCB(GSint iLobbyID,GSint iRoomID,GSvoid *vpGameData,GSint iSize,
			GSchar *szIPAddress,GSchar *szAltIPAddress,GSushort usPort)
{
	char szText[1024];
	sprintf(szText,"Join Finished: %s %s %s %i",vpGameData,szIPAddress,szAltIPAddress,usPort);
	MessageBox(szText);
}

GSvoid CGsMasterServerClientDlg::AccountCreationCB(GSubyte ucType,GSint iReason)
{
	char szText[1024];

	if (ucType == GSSUCCESS)
		sprintf(szText,"Account Creation succeeded");
	else
		sprintf(szText,"Account Creation failed %i",iReason);
	MessageBox(szText);
}


GSvoid CGsMasterServerClientDlg::ModifyAccountCB(GSubyte ucType,GSint iReason)
{
	char szText[1024];

	if (ucType == GSSUCCESS)
		sprintf(szText,"Modify Account Creation succeeded");
	else
		sprintf(szText,"Modify Account failed %i",iReason);
	MessageBox(szText);
}

GSvoid CGsMasterServerClientDlg::MatchStartedCB(GSint iLobbyID,GSint iRoomID,GSuint uiMatchID)
{
	char szText[1024];
	sprintf(szText,"Match Started: %i %i %i",iLobbyID,iRoomID, uiMatchID);
	MessageBox(szText);
	m_iLobbyID = iLobbyID;
	m_iRoomID = iRoomID;
	m_uiMatchID = uiMatchID;
}

GSvoid CGsMasterServerClientDlg::SubmitMatchCB(GSubyte ucType,GSint iReason,GSuint uiMatchID)
{
	char szText[1024];

	if (ucType == GSSUCCESS)
	{
		sprintf(szText,"Submit Match Sucessfull: %i",uiMatchID);
		MessageBox(szText);
	}
	else
	{
		sprintf(szText,"Submit Match Fail: %i %i",iReason,uiMatchID);
		MessageBox(szText);
	}
}


GSvoid CGsMasterServerClientDlg::RequestMOTDCB(GSubyte ubType, GSchar *szUbiMOTD,
			GSchar *szGameMOTD, GSint iReason)
{
	CString csText;
	if (ubType == GSSUCCESS)
	{
		csText.Format("Message of the day request successfull.\r\nUbi motd: %s\r\nGame motd: %s",szUbiMOTD,szGameMOTD);
		AfxMessageBox(csText,MB_OK);
	}
	else
	{
		csText.Format("Message of the day request failure. Error id: %d",iReason);
		AfxMessageBox(csText,MB_ICONERROR);
	}	
	
}



void CGsMasterServerClientDlg::OnBUTTONRequest() 
{
	GSchar szGameName[GAMELENGTH];
	
	m_editGameName.GetWindowText(szGameName,GAMELENGTH);

	//UBI.COM: Request the list of gameserver with the game name
	LOCKMUTEX();
	clMSClientClass::RequestGameServers(szGameName);
	UNLOCKMUTEX();

	m_listGameServers.DeleteAllItems();
	m_iNbrGameServers = 0;
}

void CGsMasterServerClientDlg::OnBUTTONJoin() 
{
	POSITION pos = m_listGameServers.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox("No Game Server Seleted");
	else
	{
		if (pos)
		{
			int iItem = m_listGameServers.GetNextSelectedItem(pos);
			JoinGameServer(iItem);
		}
	}
}

void CGsMasterServerClientDlg::JoinGameServer(int iItem) 
{
	char szText[10];
	char szPassword[PASSWORDLENGTH]="";
	char szVersion[VERSIONLENGTH]="";
	char szGameName[GAMELENGTH]="";
	GSint iLobbyID,iRoomID;

	m_listGameServers.GetItemText(iItem,0,szText,10);
	iLobbyID = atol(szText);

	m_listGameServers.GetItemText(iItem,1,szText,10);
	iRoomID = atol(szText);

	m_listGameServers.GetItemText(iItem,4,szVersion,VERSIONLENGTH);

	m_editGamePassword.GetWindowText(szPassword,PASSWORDLENGTH);
	m_editGameName.GetWindowText(szGameName,GAMELENGTH);

//UBI.COM: Join the selected game server
	LOCKMUTEX();
	clMSClientClass::JoinGameServer(iLobbyID,iRoomID,szPassword,szVersion,szGameName,
		"test",5);
	UNLOCKMUTEX();

}

void CGsMasterServerClientDlg::LeaveGameServer(int iItem) 
{
	char szText[10];
	GSint iLobbyID,iRoomID;

	m_listGameServers.GetItemText(iItem,0,szText,10);
	iLobbyID = atol(szText);

	m_listGameServers.GetItemText(iItem,1,szText,10);
	iRoomID = atol(szText);

//UBI.COM: Leave the selected game server
	LOCKMUTEX();
	clMSClientClass::LeaveGameServer(iLobbyID,iRoomID);
	UNLOCKMUTEX();

}

void CGsMasterServerClientDlg::OnDblclkLISTGameServers(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POSITION pos = m_listGameServers.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox("No Game Server Seleted");
	else
	{
		while (pos)
		{
			int iItem = m_listGameServers.GetNextSelectedItem(pos);
			JoinGameServer(iItem);
		}
	}
	
	*pResult = 0;
}

void CGsMasterServerClientDlg::OnBUTTONLeave() 
{
	POSITION pos = m_listGameServers.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox("No Game Server Seleted");
	else
	{
		if (pos)
		{
			int iItem = m_listGameServers.GetNextSelectedItem(pos);
			LeaveGameServer(iItem);
		}
	}
	
}

//UBI.COM: Uninitialize the Library but don't close the window.
void CGsMasterServerClientDlg::OnBUTTONUninitialize() 
{
	LOCKMUTEX();
	Uninitialize();
	m_bInitialized = false;
	UNLOCKMUTEX();	
}

void CGsMasterServerClientDlg::OnBUTTONDownload() 
{
	CRegistry registry(HKEY_LOCAL_MACHINE, CString("SOFTWARE\\Ubi Soft\\Game Service"),
			false,KEY_READ);
	CString csGSIniURL;

	char szDirectory[100];
	GetCurrentDirectory(100,szDirectory);

	CString csGSIniFile = szDirectory;
	csGSIniFile += "\\GS.ini";

	CString csMSClientIniFile = szDirectory;
	csMSClientIniFile += "\\MSClient.ini";

	registry.OpenKey();

	//UBI.COM: Try to read the ConnectURL from the registry
	//         If it fails we'll look in Launcher.ini
	if (!registry.ReadString(CString("ConnectURL"), &csGSIniURL))
	{
		char szUrl[1024];

		MessageBox("Registry failed; using MSClient.ini");

		GetPrivateProfileString("MSClient","ConnectURL",
				"http://gsconnect.ubisoft.com/gsinit.php?user=%s&dp=%s",szUrl,
				1024,csMSClientIniFile);
		csGSIniURL = szUrl;

	}
	registry.CloseKey();

	char szUsername[NICKNAMELENGTH],szGamename[GAMELENGTH];

	m_editUsername.GetWindowText(szUsername,NICKNAMELENGTH);
	m_editGameName.GetWindowText(szGamename,GAMELENGTH);

	//UBI.COM: Replace the strings in the URL
	csGSIniURL.Replace("user=%s",(CString("user=")+szUsername));
	csGSIniURL.Replace("dp=%s",(CString("dp=")+szGamename));

	//UBI.COM: Try to download the file GS.ini file if it fails
	//         we can use the old local one
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
	//UBI.COM: Read in the Port of the GSRouter
	GetPrivateProfileString("Servers","RouterPort0","41000",szPort,
			100,csGSIniFile);

	m_editServerIP.SetWindowText(szIPAddress);
	m_editPort.SetWindowText(szPort);

	MessageBox("Download Finished");
	return;
	
}

void CGsMasterServerClientDlg::OnBUTTONAbout() 
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

void CGsMasterServerClientDlg::OnBUTTONRefresh() 
{
	POSITION pos = m_listGameServers.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox("No Game Server Seleted");
	else
	{
		GSuint iCount = m_listGameServers.GetSelectedCount();
		GSint iNum=0;

		while (pos)
		{
			char szText[10];
			GSint iLobbyID;
			GSint iRoomID;

			GSint iItem= m_listGameServers.GetNextSelectedItem(pos);
			m_listGameServers.GetItemText(iItem,0,szText,10);
			iLobbyID = atol(szText);

			m_listGameServers.GetItemText(iItem,1,szText,10);
			iRoomID = atol(szText);

			LOCKMUTEX();
			clMSClientClass::RefreshGameServer(iLobbyID,iRoomID);
			UNLOCKMUTEX();
		}

		m_listGameServers.DeleteAllItems();
		m_iNbrGameServers = 0;

	}
}

void CGsMasterServerClientDlg::OnBUTTONScores() 
{
	if (m_iLobbyID)
	{
		CSubmitScoresdlg dlgSubmitScores(this);

		dlgSubmitScores.SetServerID(m_iLobbyID,m_iRoomID);
		dlgSubmitScores.SetMatchID(m_uiMatchID);

		dlgSubmitScores.DoModal();
		m_iLobbyID = 0;
		m_iRoomID = 0;
		m_uiMatchID = 0;
	}
	//else
//		MessageBox("Have not received MatchStarted\n");
	
}

/*void CGsMasterServerClientDlg::OnBUTTONJoinID() 
{
	GSchar szGameName[GAMELENGTH];
	
	m_editGameName.GetWindowText(szGameName,GAMELENGTH);

	LOCKMUTEX();
	clMSClientClass::JoinGameServer(7,-154,"","2.0",szGameName);
	UNLOCKMUTEX();
}*/

void CGsMasterServerClientDlg::OnBUTTONGetAlt() 
{
	POSITION pos = m_listGameServers.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox("No Game Server Seleted");
	else
	{
		while (pos)
		{
			int iItem = m_listGameServers.GetNextSelectedItem(pos);

			GSchar szText[10];
			GSint iLobbyID,iRoomID;

			m_listGameServers.GetItemText(iItem,0,szText,10);
			iLobbyID = atol(szText);

			m_listGameServers.GetItemText(iItem,1,szText,10);
			iRoomID = atol(szText);


			//UBI.COM: Join the request the game servers Alt Info
			LOCKMUTEX();
			clMSClientClass::RequestAlternateInfo(iLobbyID,iRoomID);
			UNLOCKMUTEX();
			
		}
	}
	
}

void CGsMasterServerClientDlg::OnBUTTONStartMatch() 
{
	POSITION pos = m_listGameServers.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox("No Game Server Seleted");
	else
	{
		while (pos)
		{
			int iItem = m_listGameServers.GetNextSelectedItem(pos);

			GSchar szText[10];
			GSint iLobbyID,iRoomID;

			m_listGameServers.GetItemText(iItem,0,szText,10);
			iLobbyID = atol(szText);

			m_listGameServers.GetItemText(iItem,1,szText,10);
			iRoomID = atol(szText);


			//UBI.COM: Join the request the game servers Alt Info
			LOCKMUTEX();
			clMSClientClass::MatchStarted(iLobbyID,iRoomID);
			UNLOCKMUTEX();
			
		}
	}
	
}

void CGsMasterServerClientDlg::OnBUTTONConnected() 
{
	POSITION pos = m_listGameServers.GetFirstSelectedItemPosition();
	if (pos == NULL)
		MessageBox("No Game Server Seleted");
	else
	{
		while (pos)
		{
			int iItem = m_listGameServers.GetNextSelectedItem(pos);

			GSchar szText[10];
			GSint iLobbyID,iRoomID;

			m_listGameServers.GetItemText(iItem,0,szText,10);
			iLobbyID = atol(szText);

			m_listGameServers.GetItemText(iItem,1,szText,10);
			iRoomID = atol(szText);


			//UBI.COM: Tell The Lobby server we've connected to the game
			LOCKMUTEX();
			clMSClientClass::GameServerConnected(iLobbyID,iRoomID);
			UNLOCKMUTEX();
			
		}
	}
	
}

void CGsMasterServerClientDlg::OnButtonGetMOTD()
{	
	LOCKMUTEX();
	RequestMOTD("fr");
	UNLOCKMUTEX();	
}