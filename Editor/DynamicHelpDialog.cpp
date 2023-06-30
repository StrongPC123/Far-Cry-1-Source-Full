////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   dynamichelpdialog.cpp
//  Version:     v1.00
//  Created:     12/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "DynamicHelpDialog.h"
#include "MainFrm.h"


#define HELP_FOLDER "Editor\\Help"

static HHOOK hMouseHook = 0;
static CDynamicHelpDialog *gDynamicHelpDialog = 0;

///////////////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK DynamicHelpMouseProc(
		int nCode,      // hook code
		WPARAM wParam,  // message identifier
		LPARAM lParam   // mouse coordinates
	)
{
	if (nCode >= 0)
	{
		if (gDynamicHelpDialog)
		{
			if (wParam == WM_MOUSEMOVE)
			{
				gDynamicHelpDialog->DisplayDynamicHelp();
			}
			else if (wParam == WM_LBUTTONDOWN)
			{
				// Check if also CTRL+ALT+SHIFT pressed.
				GetAsyncKeyState(VK_MENU);
				GetAsyncKeyState(VK_CONTROL);
				GetAsyncKeyState(VK_SHIFT);
				if (GetAsyncKeyState(VK_MENU) && GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_SHIFT))
				{
					gDynamicHelpDialog->EditItem();
					return TRUE;
				}
			}
		}
	}
	return CallNextHookEx( hMouseHook,nCode,wParam,lParam );
}

///////////////////////////////////////////////////////////////////////////////////
// CDynamicHelpDialog dialog

CDynamicHelpDialog* CDynamicHelpDialog::m_instance = 0;

void CDynamicHelpDialog::Open()
{
	if (!m_instance)
	{
		m_instance = new CDynamicHelpDialog;
		m_instance->Create( CDynamicHelpDialog::IDD,AfxGetMainWnd() );
	}
	m_instance->ShowWindow( SW_SHOW );
}

void CDynamicHelpDialog::OnIdle()
{
	if (m_instance)
	{
		m_instance->OnKickIdle( 0,0 );
	}
}

IMPLEMENT_DYNAMIC(CDynamicHelpDialog, CXTCBarDialog)


CDynamicHelpDialog::CDynamicHelpDialog(CWnd* pParent /*=NULL*/)
	: CXTCBarDialog(CDynamicHelpDialog::IDD, pParent,SZ_NOSIZEICON )
{
	xtAfxData.bControlBarMenus = TRUE; // Turned off in constructor of CXTCBarDialog.

	m_bDynamicHelp = false;
	m_lastWindow = NULL;
	m_bEditMode = false;
	m_bHelpNotFound = false;
	m_bSkipSameWindow = true;
}

CDynamicHelpDialog::~CDynamicHelpDialog()
{
	m_instance = 0;
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::PostNcDestroy()
{
	CXTCBarDialog::PostNcDestroy();
	delete this;
	m_instance = 0;
}


void CDynamicHelpDialog::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_HTMLVIEW, m_text);
	CXTCBarDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDynamicHelpDialog, CXTCBarDialog)
	//ON_BN_CLICKED(IDC_SAVE, OnBnClickedSave)
	//ON_BN_CLICKED(IDC_CHECK1, OnBnClickedHelp)
	ON_EN_CHANGE( IDC_HTMLVIEW,OnTextChange )
	ON_COMMAND( ID_HELP_SAVE,OnBnClickedSave )
	ON_UPDATE_COMMAND_UI( ID_HELP_SAVE,OnUpdateSave )
	ON_COMMAND( ID_HELP_GET,OnBnClickedHelp )
	ON_UPDATE_COMMAND_UI( ID_HELP_GET,OnUpdateHelp )
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CDynamicHelpDialog message handlers

BOOL CDynamicHelpDialog::OnInitDialog()
{
	CXTCBarDialog::OnInitDialog();

	gDynamicHelpDialog = this;

	UINT indicators[] =	{
			ID_SEPARATOR,           // status line indicator
	};
	m_status.Create( this );
	m_status.SetIndicators( indicators,sizeof(indicators)/sizeof(UINT) );
	
	m_text.SetBkColor( RGB(255,255,220) ); // yellow.

	m_toolbar.CreateEx( this,TBSTYLE_TRANSPARENT|TBSTYLE_FLAT|TBSTYLE_LIST,WS_CHILD|WS_VISIBLE|CBRS_TOP|CBRS_TOOLTIPS|CBRS_FLYBY );
	m_toolbar.LoadToolBar( IDR_DYNAMIC_HELP );

	// Set mouse hook.
	hMouseHook = SetWindowsHookEx( WH_MOUSE,DynamicHelpMouseProc,AfxGetInstanceHandle(),GetCurrentThreadId() );
	m_bDynamicHelp = true;

	SetResize( IDC_HTMLVIEW,SZ_RESIZE(1) );

	AutoLoadPlacement( "Dialogs\\DynamicHelp" );
	RecalcBarLayout();


	return TRUE;  // return TRUE unless you set the focus to a control
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::OpenHelpFile( const CString &fname,const CString &tooltipText )
{
	m_currentUrl = fname;

	CString baseDir = Path::AddBackslash(GetIEditor()->GetMasterCDFolder());

	CString filename = baseDir + HELP_FOLDER + "\\" + fname;
	m_currentFile = filename;

	m_status.SetPaneText(0,m_currentUrl);
	m_bHelpNotFound = false;

	//HRESULT hres;
	//if (!m_bEditMode)
	{
		// Design mode.
		CFile file;
		if (file.Open( filename,CFile::modeRead ))
		{
			char *str = new char[file.GetLength()+1];
			file.Read( str,file.GetLength() );
			str[file.GetLength()] = 0;
			m_text.SetWindowText( str );
			delete []str;
		}
		else
		{
			CString text;
			if (tooltipText.IsEmpty())
			{
				text = "Help not found.";
				text += "\r\nPress Ctrl+Alt+Shift+LMB to edit help for this item";
			}
			else
			{
				text = tooltipText;
			}
			m_text.SetWindowText( text );
		}
	}
	m_text.SetReadOnly(TRUE);
	m_text.SetModify(FALSE);
	m_toolbar.SetFocus();

	/*
	else
	{
		//m_pHtmlEdit->SetDesignMode(TRUE);
		//m_pHtmlEdit->SetDisableEditFocusUI(true);
		//m_pHtmlEdit->SetDesignMode(FALSE);
		// Design mode.
		CFile file;
		if (file.Open( filename,CFile::modeRead ))
		{
			char *str = new char[file.GetLength()+1];
			file.Read( str,file.GetLength() );
			str[file.GetLength()] = 0;
			hres = m_pHtmlEdit->NewDocument();
			hres = m_pHtmlEdit->SetDocumentHTML( str );
			delete []str;
		}
		else
		{
			NavigateBrowser( notFoundFilename );
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::OnUpdateSave( CCmdUI* pCmdUI )
{
	if (m_bEditMode && m_text.GetModify())
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->Enable( FALSE );
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::OnUpdateHelp( CCmdUI* pCmdUI )
{
	if (m_bEditMode)
		pCmdUI->Enable( TRUE );
	else
		pCmdUI->Enable( FALSE );
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::OnTextChange()
{
	
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::OnBnClickedSave()
{
	if (!m_text.GetModify())
		return;

	CFile file;
	if (!m_currentFile.IsEmpty())
	{
		CString text;
		m_text.GetWindowText(text);
		if (file.Open( m_currentFile,CFile::modeWrite|CFile::modeCreate ))
		{
			file.Write( (const char*)text,text.GetLength() );
		}
	}
	/*
	// Save url file.
	IHTMLDocument2* pHtmlDoc = NULL;
	if (m_pHtmlEdit->GetDHtmlDocument( &pHtmlDoc ) == TRUE)
	{

		CComBSTR bstrHtml;
		pHtmlDoc->toString( &bstrHtml );
		CW2CT szMyString( bstrHtml );
	}
	*/

	SetEditMode(false);
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::OnBnClickedHelp()
{
	//SetCapture();
	//m_pHtmlEdit->SetDesignMode(FALSE);
	//m_helpBtn.SetCheck( 1 );

	SetEditMode( false );
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::OnDestroy()
{
	UnhookWindowsHookEx( hMouseHook );
	gDynamicHelpDialog = 0;

	m_lastWindow = NULL;
	if (GetCapture() == this)
		ReleaseCapture();
	CXTCBarDialog::OnDestroy();
}

void CDynamicHelpDialog::SetEditMode( bool bEditMode )
{
	if (m_bEditMode == bEditMode)
		return;

	m_bEditMode = bEditMode;
	if (m_bEditMode)
	{
		m_bDynamicHelp = false;
		SetWindowText( "Dynamic Help (Editing)" );
		m_text.SetReadOnly(FALSE);
	}
	else
	{
		OpenHelpFile( m_currentUrl,"" );
		m_bDynamicHelp = true;
		SetWindowText( "Dynamic Help" );
	}
	m_text.SetModify(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::EditItem()
{
	// Edit item.
	SetEditMode(true);

	if (m_bHelpNotFound)
	{
	}
	m_bHelpNotFound = false;
	
	//m_pHtmlEdit->SetDesignMode(FALSE);
}

//////////////////////////////////////////////////////////////////////////
CString CDynamicHelpDialog::GetParentText( CWnd *pWnd,const CString &text )
{
	CWnd *pMain = AfxGetMainWnd();

	CString temp;
	CString helpId = text;

	CWnd *pParent = pWnd->GetParent();
	std::set<CString> duplicates;
	while (pParent && pParent != pMain)
	{
		//id = pParent->GetDlgCtrlID();
		//temp.Format("%d",id);
		pParent->GetWindowText(temp);
		if (!temp.IsEmpty())
		{
			if (duplicates.find(temp) == duplicates.end())
			{
				helpId = temp + CString("-") + helpId;
				duplicates.insert(temp);
			}
		}
		pParent = pParent->GetParent();
	}
	return helpId;
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::DisplayDynamicHelp()
{
	if (!m_bDynamicHelp || m_bEditMode)
		return;

	CString helpId;
	CString temp;
	char className[1024];

	CPoint p;
	GetCursorPos(&p);

	CRect selfRect;
	GetWindowRect( selfRect );
	if (selfRect.PtInRect(p))
	{
		// Point is inside Help dialog itself.
		return;
	}

	CWnd *pWnd = WindowFromPoint(p);

	// Same window as before.
	if (m_lastWindow == pWnd && m_bSkipSameWindow)
		return;
	m_lastWindow = pWnd;
	m_bSkipSameWindow = true;

	CWnd *pMain = AfxGetMainWnd();
	if (!pWnd || pWnd == pMain)
		return;

	// See if parent will be mainframe eventually.
	CMainFrame *pMainFrame = (CMainFrame*)AfxGetMainWnd();

	//////////////////////////////////////////////////////////////////////////
	// Check if target window is child of main frame.
	bool bChildWindow = false;
	CWnd *pParent = pWnd->GetParent();
	while (pParent)
	{
		if (pParent == pMain)
		{
			bChildWindow = true;
			break;
		}
		pParent = pParent->GetParent();
	}
	if (!bChildWindow && !pMainFrame->IsDockedWindowChild(pWnd))
		return;
	//////////////////////////////////////////////////////////////////////////

	bool bHelpIdFound = false;

	CString tooltipText;

	// Check if target window is toolbar.
	GetClassName( pWnd->GetSafeHwnd(),className,sizeof(className) );
	//CLogFile::FormatLine( "Class Name: %s",(const char*)className );
	if (strcmp(className,TOOLBARCLASSNAME) == 0)
	{
		m_bSkipSameWindow = false;
		CPoint tp = p;
		::ScreenToClient( pWnd->GetSafeHwnd(),&tp );
		int id = (int)::SendMessage(pWnd->GetSafeHwnd(),TB_HITTEST,0,(LPARAM)&tp );
		if (id >= 0)
		{
			TBBUTTONINFO btn;
			ZeroStruct(btn);
			btn.cbSize = sizeof(btn);
			btn.dwMask = TBIF_COMMAND|TBIF_BYINDEX;
			int res = ::SendMessage( pWnd->GetSafeHwnd(),TB_GETBUTTONINFO,id,(LPARAM)&btn );
			temp.Format("Command-%d",btn.idCommand);
			helpId += temp;

			CString szFullText;
			CString strTipText;

			// don't handle the message if no string resource found
			if (szFullText.LoadString((UINT)btn.idCommand) != 0)
			{
				// this is the command id, not the button index
				AfxExtractSubString(strTipText, szFullText, 1, '\n');
				tooltipText = strTipText;
			}

			helpId = GetParentText( pWnd,helpId );
			bHelpIdFound = true;
		}
	}

	if (strcmp(className,WC_TABCONTROL) == 0)
	{
		m_bSkipSameWindow = false;
		CPoint tp = p;
		::ScreenToClient( pWnd->GetSafeHwnd(),&tp );
		TCHITTESTINFO hitinfo;
		hitinfo.pt = tp;
		hitinfo.flags = 0;
		int id = (int)::SendMessage(pWnd->GetSafeHwnd(),TCM_HITTEST,0,(LPARAM)&hitinfo );
		if (id >= 0)
		{
			temp.Format("Tab-%d",id);
			helpId += temp;
			if (GetParentText( pWnd,helpId ) != helpId)
			{
				helpId = GetParentText( pWnd,helpId );
				bHelpIdFound = true;
			}
		}
	}

	if (!bHelpIdFound)
	{

		int id = pWnd->GetDlgCtrlID();
		if (id >= 0x8000)
		{
			temp.Format("Command-%d",id);
			helpId = temp;
		}
		else
		{
			temp.Format("%d",id);
			helpId = temp;
			helpId = GetParentText( pWnd,temp );
		}
	}

	CString url = helpId + ".txt";
	
	if (url != m_currentUrl)
	{
		//CLogFile::FormatLine( "HelpId: %s",(const char*)helpId );

		CWnd *pForeground = GetForegroundWindow();
		CWnd *pWndActive = GetActiveWindow();
		OpenHelpFile( url,tooltipText );
		if (pWndActive)
			pWndActive->SetActiveWindow();
		if (pForeground)
			pForeground->SetForegroundWindow();
	}
}

//////////////////////////////////////////////////////////////////////////
void CDynamicHelpDialog::OnCancel()
{
	DestroyWindow();
}