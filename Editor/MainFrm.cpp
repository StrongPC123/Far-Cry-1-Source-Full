// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MainFrm.h"

#include "CryEdit.h"
#include "CryEditDoc.h"
#include "TerrainDialog.h"
#include "MainTools.h"
#include "TerrainPanel.h"
#include "PanelDisplayHide.h"
#include "PanelDisplayRender.h"
#include "PanelDisplayLayer.h"
#include "Mission.h"
#include "ViewManager.h"
#include "LayoutWnd.h"
#include "ExternalTools.h"
#include "Settings.h"

#include "PropertiesPanel.h"

#include "Objects\ObjectManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  IDW_VIEW_EDITMODE_BAR		AFX_IDW_CONTROLBAR_FIRST+10
#define  IDW_VIEW_OBJECT_BAR			AFX_IDW_CONTROLBAR_FIRST+11
#define  IDW_VIEW_MISSION_BAR			AFX_IDW_CONTROLBAR_FIRST+12
#define  IDW_VIEW_TERRAIN_BAR			AFX_IDW_CONTROLBAR_FIRST+13
#define  IDW_VIEW_AVI_RECORD_BAR	AFX_IDW_CONTROLBAR_FIRST+14

#define  IDW_VIEW_ROLLUP_BAR			AFX_IDW_CONTROLBAR_FIRST+20
#define  IDW_VIEW_CONSOLE_BAR			AFX_IDW_CONTROLBAR_FIRST+21
#define  IDW_VIEW_INFO_BAR				AFX_IDW_CONTROLBAR_FIRST+22
#define  IDW_VIEW_TRACKVIEW_BAR		AFX_IDW_CONTROLBAR_FIRST+23
#define  IDW_VIEW_DIALOGTOOL_BAR	AFX_IDW_CONTROLBAR_FIRST+24
#define  IDW_VIEW_DATABASE_BAR		AFX_IDW_CONTROLBAR_FIRST+25

#define BAR_SECTION _T("Bars")

#define AUTOSAVE_TIMER_EVENT 200
#define AUTOREMIND_TIMER_EVENT 201

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CXTFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CXTFrameWnd)
	ON_WM_CREATE()

	ON_COMMAND_EX(ID_VIEW_MENUBAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(ID_VIEW_MENUBAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(ID_VIEW_STATUS_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_EDITMODE_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_EDITMODE_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_OBJECT_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_OBJECT_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_MISSION_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_MISSION_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_TERRAIN_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_TERRAIN_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_AVI_RECORD_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_AVI_RECORD_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_ROLLUP_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_ROLLUP_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_CONSOLE_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_CONSOLE_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_INFO_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_INFO_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_TRACKVIEW_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_TRACKVIEW_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_DATABASE_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_DATABASE_BAR, OnUpdateControlBarMenu)

	ON_COMMAND_EX(IDW_VIEW_DIALOGTOOL_BAR, OnBarCheck )
	ON_UPDATE_COMMAND_UI(IDW_VIEW_DIALOGTOOL_BAR, OnUpdateControlBarMenu)

	//ON_COMMAND_EX(ID_VIEW_ROLLUPBAR, OnBarCheck )
	//ON_UPDATE_COMMAND_UI(ID_VIEW_ROLLUPBAR, OnUpdateControlBarMenu)

	//ON_COMMAND_EX(ID_VIEW_CONSOLEWINDOW, OnBarCheck )
	//ON_UPDATE_COMMAND_UI(ID_VIEW_CONSOLEWINDOW, OnUpdateControlBarMenu)

	//ON_COMMAND_EX(ID_VIEW_TOOLBAR, OnBarCheck )
	//ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateControlBarMenu)

	ON_COMMAND(ID_SOUND_PRESETS, OnSoundPresets)
	ON_COMMAND(ID_EAX_PRESETS, OnEAXPresets)
	ON_COMMAND(ID_SOUND_SHOWMUSICINFO, OnMusicInfo)

	ON_UPDATE_COMMAND_UI(ID_VIEW_XPLOOK, OnUpdateXPLook)
	ON_COMMAND(ID_VIEW_XPLOOK, OnXPLook)

	// Sent by Progress bar cancel button.
	ON_COMMAND(ID_PROGRESSBAR_CANCEL, OnProgressBarCancel)

//  ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateToolbar)
//  ON_UPDATE_COMMAND_UI(ID_VIEW_ROLLUPBAR, OnUpdateRollUpBar)
//	ON_UPDATE_COMMAND_UI(ID_VIEW_CONSOLEWINDOW, OnUpdateConsole)
//	ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateStatusBar)
//	ON_UPDATE_COMMAND_UI(ID_VIEW_TRACKVIEW, OnUpdateTrackView)

	//ON_COMMAND(ID_VIEW_ROLLUPBAR, OnRollUpBar)
	//ON_COMMAND(ID_VIEW_CONSOLEWINDOW, OnConsoleWindow)
	//ON_COMMAND(ID_VIEW_TOOLBAR, OnToolbar)
	//ON_COMMAND(ID_VIEW_STATUS_BAR, OnStatusBar)
	//ON_COMMAND(ID_VIEW_TRACKVIEW, OnTrackView)

	ON_CBN_SELENDOK( IDC_MISSION,OnMissionChanged )
	ON_CBN_SELENDCANCEL( IDC_MISSION,OnMissionCancelChanged )
	ON_CBN_DROPDOWN( IDC_MISSION,OnMissionDropDown )
	ON_WM_SIZE()
	ON_WM_COPYDATA()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_COMMAND(ID_EDIT_NEXTSELECTIONMASK, OnEditNextSelectionMask)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_bXPLook = true;

	m_autoSaveTimer = 0;
	m_autoRemindTimer = 0;

	//////////////////////////////////////////////////////////////////////////
	m_currentMission = 0;
	m_currentLayer = 0;

	m_terrainPanel = 0;
	m_mainTools = 0;
	m_consoleVisible = false;
	m_gridSize = -1;

	m_layoutWnd = 0;

	// Enable/Disable XP GUI Mode
	CXTRegistryManager regMgr;
	xtAfxData.bXPMode = regMgr.GetProfileInt(_T("Settings"), _T("bXPMode"), TRUE);

	// Enable/Disable Menu Shadows
	xtAfxData.bMenuShadows = TRUE;
}

CMainFrame::~CMainFrame()
{
	if (m_layoutWnd)
		delete m_layoutWnd;
	m_layoutWnd = 0;
	CLogFile::WriteLine("Main frame destroied");
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CXTFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//////////////////////////////////////////////////////////////////////////
	// Start autosave timer.
	//////////////////////////////////////////////////////////////////////////
	if (gSettings.autoBackupTime > 0 && gSettings.autoBackupEnabled)
		m_autoSaveTimer = SetTimer( AUTOSAVE_TIMER_EVENT,gSettings.autoBackupTime*1000*60,0 );
	if (gSettings.autoRemindTime > 0)
    m_autoRemindTimer = SetTimer( AUTOREMIND_TIMER_EVENT,gSettings.autoRemindTime*1000*60,0 );

	//////////////////////////////////////////////////////////////////////////
	// Load state.
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create Menu.
	VERIFY(m_wndMenuBar.CreateEx(this, TBSTYLE_TRANSPARENT| TBSTYLE_FLAT) );
	VERIFY(m_wndMenuBar.LoadMenuBar(IDR_MAINFRAME) );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create ReBar.
	m_wndReBar.Create(this,RBS_BANDBORDERS|RBS_VARHEIGHT|RBS_DBLCLKTOGGLE);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create the status bar
	VERIFY(m_wndStatusBar.Create(this) );
	VERIFY(m_wndStatusBar.SetIndicators( indicators,sizeof(indicators)/sizeof(UINT) ) );
	//////////////////////////////////////////////////////////////////////////

	DWORD dwToolBarFlags = WS_CHILD|WS_VISIBLE|CBRS_ALIGN_ANY|CBRS_TOOLTIPS|CBRS_FLYBY;
	CRect nullRc(0,0,0,0);

	//////////////////////////////////////////////////////////////////////////
	// Create standart toolbar.
	VERIFY(m_wndToolBar.CreateEx(this, TBSTYLE_TRANSPARENT|TBSTYLE_FLAT,dwToolBarFlags ));
	LoadTrueColorToolbar(m_wndToolBar,IDR_MAINFRAME);
	m_wndToolBar.SetWindowText( _T("Standart ToolBar") );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	VERIFY(m_wndTerrainToolBar.CreateEx(this, TBSTYLE_TRANSPARENT|TBSTYLE_FLAT|TBSTYLE_LIST,dwToolBarFlags,nullRc,IDW_VIEW_TERRAIN_BAR ) );
	LoadTrueColorToolbar(m_wndTerrainToolBar,IDR_TERRAIN_BAR);
	m_wndTerrainToolBar.SetButtonText( m_wndTerrainToolBar.CommandToIndex(ID_TERRAIN),_T("Terrain") );
	m_wndTerrainToolBar.SetButtonText( m_wndTerrainToolBar.CommandToIndex(ID_GENERATORS_TEXTURE),_T("Texture") );
	m_wndTerrainToolBar.SetButtonText( m_wndTerrainToolBar.CommandToIndex(ID_GENERATORS_LIGHTING),_T("Lighting") );
	m_wndTerrainToolBar.SetWindowText( _T("Terrain ToolBar") );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	VERIFY(m_wndAvoToolBar.CreateEx(this, TBSTYLE_TRANSPARENT|TBSTYLE_FLAT|TBSTYLE_LIST,dwToolBarFlags,nullRc,IDW_VIEW_AVI_RECORD_BAR ) );
	LoadTrueColorToolbar(m_wndAvoToolBar,IDR_AVI_RECORDER_BAR);
	m_wndAvoToolBar.SetWindowText( _T("AVI Recorder ToolBar") );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create EditMode Toolbar.
	VERIFY(m_editModeBar.Create( this,dwToolBarFlags,IDW_VIEW_EDITMODE_BAR ));
	m_editModeBar.SetWindowText( _T("EditMode ToolBar") );
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// Object Modify Toolbar
	VERIFY(m_objectModifyBar.CreateEx( this,TBSTYLE_TRANSPARENT|TBSTYLE_FLAT,dwToolBarFlags,nullRc,IDW_VIEW_OBJECT_BAR ));
	LoadTrueColorToolbar(m_objectModifyBar,IDR_OBJECT_MODIFY);
	m_objectModifyBar.SetWindowText( _T("Object ToolBar") );

	//////////////////////////////////////////////////////////////////////////
	// Initialize Mission ToolBar.
	//////////////////////////////////////////////////////////////////////////
	CreateMissionsBar();

	//////////////////////////////////////////////////////////////////////////
	// Create the RollupBar.
	CreateRollupBar();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create console.
	// Create the console
	m_wndConsoleBar.Create( this,IDW_VIEW_CONSOLE_BAR,_T("Log"),CSize(500,70),CBRS_BOTTOM );
	m_cConsole.Create( NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),&m_wndConsoleBar,m_wndConsoleBar.GetDlgCtrlID() );
	m_wndConsoleBar.SetChild( &m_cConsole );
	//m_wndConsoleBar.SetInitDesiredSizeHorizontal( CSize(500, 70) );
	//m_wndConsoleBar.SetInitDesiredSizeFloating( CSize(300, 400) );
	//m_wndConsoleBar.SetInitDesiredSizeVertical( CSize(300, 400) );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create TackView.
	//////////////////////////////////////////////////////////////////////////
	//m_wndTrackViewBar.Create( _T("TrackView"),this,IDW_VIEW_TRACKVIEW_BAR );
	m_wndTrackViewBar.Create( this,IDW_VIEW_TRACKVIEW_BAR,_T("TrackView"),CSize(500,300),CBRS_BOTTOM );
	m_wndTrackView.Create( CTrackViewDialog::IDD,&m_wndTrackViewBar );
	m_wndTrackViewBar.SetChild( &m_wndTrackView );

	ShowControlBar(&m_wndTrackViewBar, FALSE, TRUE);
	//m_wndTrackViewBar.SetInitDesiredSizeHorizontal( CSize(500,300) );
	//m_wndTrackViewBar.SetInitDesiredSizeFloating( CSize(500,300) );
	//m_wndTrackViewBar.SetInitDesiredSizeVertical( CSize(400,300) );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create DataBase Dialog.
	//////////////////////////////////////////////////////////////////////////
	m_wndDataBaseBar.Create( this,IDW_VIEW_DATABASE_BAR,_T("DataBaseView"),CSize(500,300),CBRS_BOTTOM );
	//m_wndDataBase.Create( CDataBaseDialog::IDD,&m_wndDataBaseBar );
	CRect rc(0,0,600,400);
	m_wndDataBase.Create( CDataBaseDialog::IDD,&m_wndDataBaseBar );
	//m_wndDataBase.CreateEx( 0,"Dialog",NULL,WS_POPUP|WS_VISIBLE,rc,&m_wndDataBaseBar,m_wndDataBaseBar.GetDlgCtrlID() );
	m_wndDataBaseBar.SetChild( &m_wndDataBase );


	ShowControlBar(&m_wndDataBaseBar, FALSE, TRUE);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create Sound-Preset-Ed
	m_wndSoundPresets.Create(CSoundPresetsDlg::IDD, this);

	//////////////////////////////////////////////////////////////////////////
	// Create Sound-Preset-Ed
	m_wndEAXPresets.Create(CEAXPresetsDlg::IDD, this);

	//////////////////////////////////////////////////////////////////////////
	// Create Music-Info-Dlg
	m_wndMusicInfo.Create(CMusicInfoDlg::IDD, this);

	//////////////////////////////////////////////////////////////////////////
	// Create info bar.
	m_infoBarHolder.Create( this, CBRS_BOTTOM|CBRS_TOOLTIPS|CBRS_FLYBY,IDW_VIEW_INFO_BAR );
	m_infoBarHolder.EnableDocking( CBRS_ALIGN_BOTTOM );
	//////////////////////////////////////////////////////////////////////////

	m_wndReBar.AddBar( &m_wndToolBar,0,0,RBBS_GRIPPERALWAYS|RBBS_FIXEDBMP );
	m_wndReBar.AddBar( &m_editModeBar,0,0,RBBS_GRIPPERALWAYS|RBBS_FIXEDBMP );
	m_wndReBar.AddBar( &m_objectModifyBar,0,0,RBBS_GRIPPERALWAYS|RBBS_FIXEDBMP );
	m_wndReBar.AddBar( &m_missionToolBar,0,0,RBBS_GRIPPERALWAYS|RBBS_FIXEDBMP|RBBS_BREAK );
	m_wndReBar.AddBar( &m_wndTerrainToolBar,0,0,RBBS_GRIPPERALWAYS|RBBS_FIXEDBMP );
	m_wndReBar.AddBar( &m_wndAvoToolBar,0,0,RBBS_GRIPPERALWAYS|RBBS_FIXEDBMP );
	
	
	//////////////////////////////////////////////////////////////////////////
	// AVI Tool bar is not visible by default.
	ShowControlBar( &m_wndAvoToolBar,FALSE,0 );

	// Set size of edit mode bar in ReBar control.
	REBARBANDINFO rbi;
	ZeroStruct(rbi);
	rbi.fMask = RBBIM_SIZE|RBBIM_CHILDSIZE|RBBIM_IDEALSIZE;
	rbi.cbSize = sizeof(rbi);
	m_wndReBar.GetReBarCtrl().GetBandInfo( 1,&rbi );
	rbi.cx = rbi.cx + 40;
	rbi.cxMinChild = rbi.cx;
	rbi.cxIdeal = rbi.cx + 50;
	m_wndReBar.GetReBarCtrl().SetBandInfo( 1,&rbi );

	// TODO: Remove this if you don't want tool tips
	//m_wndMenuBar.SetBarStyle(m_wndMenuBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	//m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |  CBRS_TOOLTIPS | CBRS_FLYBY);
	//m_wndReBar.SetBarStyle(m_wndReBar.GetBarStyle() | CBRS_BORDER_ANY );

	m_wndConsoleBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndRollUpBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndTrackViewBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndDataBaseBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);

	//EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_SEMIFLAT );

	EnableDockingEx(CBRS_ALIGN_TOP, CBRS_XT_SEMIFLAT );
	EnableDockingEx(CBRS_ALIGN_RIGHT, CBRS_XT_SEMIFLAT );
	EnableDockingEx(CBRS_ALIGN_LEFT, CBRS_XT_SEMIFLAT );
	EnableDockingEx(CBRS_ALIGN_BOTTOM, CBRS_XT_SEMIFLAT );


	DockControlBar(&m_infoBarHolder);
	DockControlBar(&m_wndConsoleBar);
	DockControlBar(&m_wndRollUpBar);
	DockControlBar(&m_wndTrackViewBar);
	DockControlBar(&m_wndDataBaseBar);

#ifndef WIN64
	UINT toolbarIds[] = {	IDR_MAINFRAME,IDR_EDIT_MODE,IDR_MISSION_BAR,IDR_OBJECT_MODIFY,IDR_TERRAIN_BAR	};
	InstallCoolMenus( toolbarIds,_countof(toolbarIds) );
#endif

	if (m_wndReBar.m_hWnd)
		m_wndReBar.LoadState(BAR_SECTION);

	if (IsPreview())
	{
		// Hide all menus.
		ShowControlBar( &m_wndMenuBar,FALSE,0 );
		ShowControlBar( &m_wndReBar,FALSE,0 );
		ShowControlBar( &m_wndTrackViewBar,FALSE,0 );
		ShowControlBar( &m_wndDataBaseBar,FALSE,0 );
	}
	else
	{
		// Update tools menu,
		UpdateTools();
	}

//////////////////////////////////////////////////////////////////////////
	// Load toolbar buttons for main menu.
	/*
	m_DefaultNewMenu.LoadToolBar(IDR_MAINFRAME);
	m_DefaultNewMenu.LoadToolBar(IDR_EDIT_MODE);
	m_DefaultNewMenu.LoadToolBar(IDR_MISSION_BAR);
	m_DefaultNewMenu.LoadToolBar(IDR_OBJECT_MODIFY);
	*/

/*
	////////////////////////////////////////////////////////////////////////
	// Create and set up all control bars
	// WARNING! Creation order of controls is very important for the look.
	////////////////////////////////////////////////////////////////////////

	HICON hIcon = LoadIcon( AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_SMALLICON) );
	SetIcon( hIcon,FALSE );

	//////////////////////////////////////////////////////////////////////////
	CWinApp * pApp = ::AfxGetApp();
	ASSERT( pApp != NULL );
	ASSERT( pApp->m_pszRegistryKey != NULL );
	ASSERT( pApp->m_pszRegistryKey[0] != _T('\0') );
	ASSERT( pApp->m_pszProfileName != NULL );
	ASSERT( pApp->m_pszProfileName[0] != _T('\0') );

	ASSERT( pApp->m_pszProfileName != NULL );
	VERIFY( g_CmdManager->ProfileSetup( pApp->m_pszProfileName, GetSafeHwnd() ) );
	VERIFY( g_CmdManager->UpdateFromMenu( pApp->m_pszProfileName, IDR_MAINFRAME )	);

	m_bXPLook = pApp->GetProfileInt("Window","XPLook",1) != 0;

	// Turn off menu expanding.
	CExtPopupMenuWnd::g_bMenuExpanding = false;
	CExtPopupMenuWnd::g_bMenuHighlightRarely = false;
	CExtPopupMenuWnd::g_bMenuExpandAnimation = false;
	CExtPopupMenuWnd::g_DefAnimationType = CExtPopupMenuWnd::__AT_NONE;

	if (m_bXPLook)
	{
		// Install XP paint manager.
		if (!g_PaintManager->IsKindOf( RUNTIME_CLASS(CExtPaintManagerXP) ) )
			VERIFY(	g_PaintManager.InstallPaintManager( new CExtPaintManagerXP ) );
		CExtPopupMenuWnd::g_bMenuWithShadows = true;
	}
	else
	{
		// Install w2k paint manager.
		if (g_PaintManager->IsKindOf( RUNTIME_CLASS(CExtPaintManagerXP) ) )
			VERIFY(	g_PaintManager.InstallPaintManager( new CExtPaintManager ) );
		CExtPopupMenuWnd::g_bMenuWithShadows = false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Create Menu.
	VERIFY( m_wndMenuBar.Create( NULL,this,ID_VIEW_MENUBAR ) );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create the status bar
	VERIFY( m_wndStatusBar.Create(this) );
	VERIFY( m_wndStatusBar.SetIndicators( indicators,sizeof(indicators)/sizeof(UINT) ) );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create standart toolbar.
	VERIFY( m_wndToolBar.Create( _T("Standart"),this,AFX_IDW_TOOLBAR ) );
	VERIFY( m_wndToolBar.LoadToolBar(IDR_MAINFRAME) );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	VERIFY( m_wndTerrainToolBar.Create( _T("TerrainTools"),this,IDW_VIEW_TERRAIN_BAR ) );
	VERIFY( m_wndTerrainToolBar.LoadToolBar(IDR_TERRAIN_BAR) );
	CExtCmdManager::cmd_t * p_cmd;
	p_cmd =	g_CmdManager->CmdGetPtr( pApp->m_pszProfileName,ID_TERRAIN	);
	ASSERT( p_cmd != 0 );
	p_cmd->m_sToolbarText = "Terrain";
	p_cmd =	g_CmdManager->CmdGetPtr( pApp->m_pszProfileName,ID_GENERATORS_TEXTURE	);
	ASSERT( p_cmd != 0 );
	p_cmd->m_sToolbarText = "Texture";
	p_cmd =	g_CmdManager->CmdGetPtr( pApp->m_pszProfileName,ID_GENERATORS_LIGHTING );
	ASSERT( p_cmd != 0 );
	p_cmd->m_sToolbarText = "Lighting";
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create EditMode Toolbar.
	m_editModeBar.Create( _T("EditMode"),this,IDW_VIEW_EDITMODE_BAR );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Object Modify Toolbar
	m_objectModifyBar.Create( _T("Object"),this,IDW_VIEW_OBJECT_BAR );
	m_objectModifyBar.LoadToolBar( IDR_OBJECT_MODIFY );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Initialize Mission ToolBar.
	//////////////////////////////////////////////////////////////////////////
	CreateMissionsBar();

	//////////////////////////////////////////////////////////////////////////
	// Create the RollupBar.
	CreateRollupBar();
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create console.
	// Create the console
	m_wndConsoleBar.Create( _T("Log"),this,IDW_VIEW_CONSOLE_BAR );
	m_cConsole.Create( NULL,NULL,WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),&m_wndConsoleBar,m_wndConsoleBar.GetDlgCtrlID() );
	m_wndConsoleBar.SetInitDesiredSizeHorizontal( CSize(500, 70) );
	m_wndConsoleBar.SetInitDesiredSizeFloating( CSize(300, 400) );
	m_wndConsoleBar.SetInitDesiredSizeVertical( CSize(300, 400) );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create TackView.
	//////////////////////////////////////////////////////////////////////////
	m_wndTrackViewBar.Create( _T("TrackView"),this,IDW_VIEW_TRACKVIEW_BAR );
	m_wndTrackView.Create( CTrackViewDialog::IDD,&m_wndTrackViewBar );
	m_wndTrackViewBar.SetInitDesiredSizeHorizontal( CSize(500,300) );
	m_wndTrackViewBar.SetInitDesiredSizeFloating( CSize(500,300) );
	m_wndTrackViewBar.SetInitDesiredSizeVertical( CSize(400,300) );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Create Sound-Preset-Ed
	m_wndSoundPresets.Create(CSoundPresetsDlg::IDD, this);

	//////////////////////////////////////////////////////////////////////////
	// Create info bar.
	m_infoBarHolder.Create( this, CBRS_BOTTOM|CBRS_TOOLTIPS|CBRS_FLYBY,IDW_VIEW_INFO_BAR );
	m_infoBarHolder.EnableDocking( CBRS_ALIGN_BOTTOM );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Docking.
	//////////////////////////////////////////////////////////////////////////
	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_editModeBar.EnableDocking(CBRS_ALIGN_ANY);
	m_objectModifyBar.EnableDocking(CBRS_ALIGN_ANY);
	m_missionToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndRollUpBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndConsoleBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndTrackViewBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndTerrainToolBar.EnableDocking(CBRS_ALIGN_ANY);

	// This is correct priority order.
	CExtControlBar::FrameEnableDocking(this,CBRS_ALIGN_TOP);
	CExtControlBar::FrameEnableDocking(this,CBRS_ALIGN_RIGHT);
	CExtControlBar::FrameEnableDocking(this,CBRS_ALIGN_LEFT);
	CExtControlBar::FrameEnableDocking(this,CBRS_ALIGN_BOTTOM);

	DockControlBar(&m_wndMenuBar);
	DockControlBar(&m_wndToolBar);

	RecalcLayout();
	CRect wrAlredyDockedBar;
	m_wndToolBar.GetWindowRect( &wrAlredyDockedBar );
	wrAlredyDockedBar.OffsetRect( 1, 0 );
	DockControlBar(&m_editModeBar,AFX_IDW_DOCKBAR_TOP,&wrAlredyDockedBar);

	RecalcLayout();
	m_editModeBar.GetWindowRect( &wrAlredyDockedBar );
	wrAlredyDockedBar.OffsetRect( 1, 0 );
	DockControlBar(&m_objectModifyBar,AFX_IDW_DOCKBAR_TOP,&wrAlredyDockedBar);

	RecalcLayout();
	m_objectModifyBar.GetWindowRect( &wrAlredyDockedBar );
	wrAlredyDockedBar.OffsetRect( 1, 0 );
	DockControlBar(&m_missionToolBar,AFX_IDW_DOCKBAR_TOP,&wrAlredyDockedBar);

	RecalcLayout();
	m_missionToolBar.GetWindowRect( &wrAlredyDockedBar );
	wrAlredyDockedBar.OffsetRect( 1, 0 );
	DockControlBar(&m_wndTerrainToolBar,AFX_IDW_DOCKBAR_TOP,&wrAlredyDockedBar);

	//CRect rc(1,1,328, 300);
	DockControlBar(&m_wndRollUpBar,AFX_IDW_DOCKBAR_RIGHT);
	DockControlBar(&m_infoBarHolder,AFX_IDW_DOCKBAR_BOTTOM);
	DockControlBar(&m_wndConsoleBar,AFX_IDW_DOCKBAR_BOTTOM);
	DockControlBar(&m_wndTrackViewBar,AFX_IDW_DOCKBAR_BOTTOM);

	ShowControlBar( &m_wndTrackViewBar,FALSE,0 );
	RecalcLayout();

/*
	// Create rebar.
	m_ReBar.Create( this,RBS_BANDBORDERS|RBS_AUTOSIZE );


	EnableDocking(CBRS_ALIGN_TOP);
	EnableDocking(CBRS_ALIGN_LEFT);
  EnableDocking(CBRS_ALIGN_RIGHT);
	EnableDocking(CBRS_ALIGN_BOTTOM);

	// Create the console
	m_cConsole.Create("Console", this, CSize(500, 70),TRUE, AFX_IDW_CONTROLBAR_FIRST + 32);
	m_cConsole.SetBarStyle(m_cConsole.GetBarStyle() |CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_cConsole.EnableDocking(CBRS_ALIGN_ANY);

	if (!GetIEditor()->IsInPreviewMode())
	{
		m_wndTrackView.Create( "TrackView",this,CSize(500,300),TRUE,AFX_IDW_CONTROLBAR_FIRST + 33 );
		ShowControlBar( &m_wndTrackView, FALSE,FALSE );
		m_wndTrackView.SetBarStyle(m_cConsole.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC );
		m_wndTrackView.EnableDocking( CBRS_ALIGN_BOTTOM|CBRS_ALIGN_TOP );
	}

	// Create the standard toolbar
	m_wndToolBar.CreateEx( &m_ReBar, TBSTYLE_FLAT, WS_CHILD|WS_VISIBLE|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC|CBRS_ALIGN_TOP);
	m_wndToolBar.LoadToolBar(IDR_MAINFRAME);
//	m_ReBar.AddBar( &m_wndToolBar,RGB(0,0,0),RGB(255,255,255),"Main" );
	m_ReBar.AddBar( &m_wndToolBar );


	// Add editmode toolbar.
	m_editModeBar.Create( &m_ReBar, WS_CHILD|WS_VISIBLE|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_FIXED|CBRS_ALIGN_TOP);
	m_ReBar.AddBar( &m_editModeBar );

	REBARBANDINFO rbi;
	ZeroStruct(rbi);
	rbi.fMask = RBBIM_SIZE|RBBIM_CHILDSIZE|RBBIM_IDEALSIZE;
	rbi.cbSize = sizeof(rbi);
	m_ReBar.GetReBarCtrl().GetBandInfo( 1,&rbi );
	rbi.cx = rbi.cx + 80;
	rbi.cxMinChild = rbi.cx;
	rbi.cxIdeal = rbi.cx + 50;
	m_ReBar.GetReBarCtrl().SetBandInfo( 1,&rbi );



	//m_tools.CreateEx( this, CCS_VERT, WS_BORDER|WS_CHILD|WS_VISIBLE|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC|CBRS_ORIENT_VERT);
	//m_tools.LoadToolBar( IDR_EDIT_MODE1 );
	//m_tools.EnableDocking(CBRS_ALIGN_LEFT);


	m_objectModifyBar.CreateEx( &m_ReBar, TBSTYLE_FLAT, WS_CHILD|WS_VISIBLE|CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_FIXED|CBRS_ALIGN_TOP);
	m_objectModifyBar.LoadToolBar( IDR_OBJECT_MODIFY );
	m_ReBar.AddBar( &m_objectModifyBar );

	InitMissionsBar();

	// Create the dialog bar at the top of the window
	m_cDialogBar.Create(this, IDD_DIALOGBAR, CBRS_ALIGN_BOTTOM, AFX_IDW_DIALOGBAR);
	m_ReBar.AddBar( &m_cDialogBar );

	//m_infoBar.Create(this, CBRS_BOTTOM|CBRS_TOOLTIPS|CBRS_FLYBY, CInfoBar::IDD);
	m_infoBarHolder.Create(this, CBRS_BOTTOM|CBRS_TOOLTIPS|CBRS_FLYBY,AFX_IDW_CONTROLBAR_FIRST + 34 );
	m_infoBarHolder.EnableDocking( CBRS_ALIGN_BOTTOM );
	//m_infoBarHolder.Create( CInfoBarHolder::IDD, this );


	// Create the rollup bar
	m_wndRollUp.Create("RollupBar", this, CSize(228, 300),TRUE, AFX_IDW_CONTROLBAR_FIRST + 64);

	// Set styles of the rollup bar
	DWORD oldStyle = m_wndRollUp.GetBarStyle();
	m_wndRollUp.SetBarStyle( CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_SIZE_DYNAMIC|CBRS_ALIGN_RIGHT );
	m_wndRollUp.EnableDocking(CBRS_ALIGN_ANY);

	// Create our RollupCtrl into DialogBar and register it
	m_objectRollupCtrl.Create(WS_VISIBLE | WS_CHILD, CRect(4, 4, 187, 362), &m_wndRollUp, NULL);
	m_wndRollUp.SetRollUpCtrl( ROLLUP_OBJECTS,&m_objectRollupCtrl);

	m_terrainRollupCtrl.Create(WS_VISIBLE | WS_CHILD, CRect(4, 4, 187, 362), &m_wndRollUp, NULL);
	m_wndRollUp.SetRollUpCtrl( ROLLUP_TERRAIN,&m_terrainRollupCtrl );

	m_displayRollupCtrl.Create(WS_VISIBLE | WS_CHILD, CRect(4, 4, 187, 362), &m_wndRollUp, NULL);
	m_wndRollUp.SetRollUpCtrl( ROLLUP_DISPLAY,&m_displayRollupCtrl );

	// Dock stuff.
	DockControlBar(&m_wndRollUp, AFX_IDW_DOCKBAR_RIGHT);
	DockControlBar(&m_cConsole, AFX_IDW_DOCKBAR_BOTTOM);
	if (m_wndTrackView.m_hWnd)
		DockControlBar(&m_wndTrackView, AFX_IDW_DOCKBAR_BOTTOM);

	//FloatControlBar(&m_cConsole, CPoint(100,80),CBRS_ALIGN_TOP );

	if (!GetIEditor()->IsInPreviewMode())
	{
		// Insert the main rollup
		m_mainTools = new CMainTools;
		m_mainTools->Create(MAKEINTRESOURCE(CMainTools::IDD),this);
		m_objectRollupCtrl.InsertPage("Objects",m_mainTools);

		//	m_objectRollupCtrl.InsertPage("Main Tools",MAKEINTRESOURCE(CMainTools::IDD),RUNTIME_CLASS(CMainTools) );

		m_terrainPanel = new CTerrainPanel(this);
		m_terrainRollupCtrl.InsertPage("Terrain",m_terrainPanel );

		CPanelDisplayHide *hidePanel = new CPanelDisplayHide(this);
		m_displayRollupCtrl.InsertPage("Hide by Category",hidePanel );

		CPanelDisplayRender *renderPanel = new CPanelDisplayRender(this);
		m_displayRollupCtrl.InsertPage("Render Settings",renderPanel );

		CPanelDisplayLayer *layerPanel = new CPanelDisplayLayer(this);
		m_displayRollupCtrl.InsertPage( "Layers Settings",layerPanel );

		m_objectRollupCtrl.ExpandAllPages( TRUE );
		m_terrainRollupCtrl.ExpandAllPages(TRUE);
		m_displayRollupCtrl.ExpandAllPages(TRUE);
	}

	SelectRollUpBar( ROLLUP_OBJECTS );

	//FloatControlBar(&m_editModeBar, CPoint(100,80),CBRS_ALIGN_TOP );

	if (GetIEditor()->IsInPreviewMode())
	{
		m_ReBar.ShowWindow( SW_HIDE );
		SetMenu( NULL );
	}
	*/

/*
	if (!IsPreview())
	{
		//////////////////////////////////////////////////////////////////////////
		VERIFY( g_CmdManager->SetBasicCommands( pApp->m_pszProfileName,g_statBasicCommands ) );

		CExtControlBar::ProfileBarStateLoad(
			this,
			pApp->m_pszRegistryKey,
			pApp->m_pszProfileName,
			pApp->m_pszProfileName,
			&m_dataFrameWP
			);
		//////////////////////////////////////////////////////////////////////////
	}
	else
	{
    CExtControlBar::ProfileBarStateLoad( this,pApp->m_pszRegistryKey,pApp->m_pszProfileName,"Preview",&m_dataFrameWP);
		// Hide all menus.
		ShowControlBar( &m_wndMenuBar,FALSE,0 );
		ShowControlBar( &m_wndToolBar,FALSE,0 );
		ShowControlBar( &m_editModeBar,FALSE,0 );
		ShowControlBar( &m_missionToolBar,FALSE,0 );
		ShowControlBar( &m_objectModifyBar,FALSE,0 );
		ShowControlBar( &m_wndTrackViewBar,FALSE,0 );
		ShowControlBar( &m_wndTerrainToolBar,FALSE,0 );
	}
*/

	m_bXPLook = xtAfxData.bXPMode;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::LoadTrueColorToolbar( CXTToolBar &bar,UINT nImageResource )
{
	CBitmap toolbarBitmap;
	CImageList toolbarImageList;

	VERIFY(bar.LoadToolBar(nImageResource));

	/*
	//////////////////////////////////////////////////////////////////////////
	// Use 24Bit toolbars.
	//////////////////////////////////////////////////////////////////////////
	toolbarBitmap.LoadBitmap(nImageResource);
	toolbarImageList.Create(16, 15, ILC_COLORDDB|ILC_MASK, 13, 1);
	toolbarImageList.Add(&toolbarBitmap,TOOLBAR_TRANSPARENT_COLOR);
	bar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)toolbarImageList.m_hImageList);
	*/
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnSoundPresets()
{
	m_wndSoundPresets.ShowWindow(true);
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnEAXPresets()
{
	m_wndEAXPresets.ShowWindow(true);
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnMusicInfo()
{
	m_wndMusicInfo.ShowWindow(true);
}

//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	//if( m_wndMenuBar.TranslateMainFrameMessage(pMsg) )
//		return TRUE;

	return CXTFrameWnd::PreTranslateMessage(pMsg);
}

//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::DestroyWindow()
{
	if (m_autoSaveTimer)
		KillTimer( m_autoSaveTimer );
	if (m_autoRemindTimer)
		KillTimer( m_autoRemindTimer );

	SaveConfig();

	return CXTFrameWnd::DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::SaveConfig()
{
	CXTRegistryManager regMgr;
	regMgr.WriteProfileInt(_T("Settings"), _T("bXPMode"), xtAfxData.bXPMode);

	// Save frame window size and position.
	m_wndPosition.SaveWindowPos(this);

	if (!IsPreview())
	{
		m_wndReBar.SaveState(BAR_SECTION);

		// Save control bar postion.
		SaveBarState(_T("Bar State"));

		if (m_layoutWnd)
			m_layoutWnd->SaveConfig();
	}
}

//////////////////////////////////////////////////////////////////////////
// WARNING uses undocumented MFC code!!!
//////////////////////////////////////////////////////////////////////////
void CMainFrame::DockControlBarNextTo(CControlBar* pBar,
                                      CControlBar* pTargetBar)
{
    ASSERT(pBar != NULL);
    ASSERT(pTargetBar != NULL);
    ASSERT(pBar != pTargetBar);

    // the neighbour must be already docked
    CDockBar* pDockBar = pTargetBar->m_pDockBar;
    ASSERT(pDockBar != NULL);
    UINT nDockBarID = pTargetBar->m_pDockBar->GetDlgCtrlID();
    ASSERT(nDockBarID != AFX_IDW_DOCKBAR_FLOAT);

    bool bHorz = (nDockBarID == AFX_IDW_DOCKBAR_TOP ||
        nDockBarID == AFX_IDW_DOCKBAR_BOTTOM);

    // dock normally (inserts a new row)
    DockControlBar(pBar, nDockBarID);

    // delete the new row (the bar pointer and the row end mark)
    pDockBar->m_arrBars.RemoveAt(pDockBar->m_arrBars.GetSize() - 1);
    pDockBar->m_arrBars.RemoveAt(pDockBar->m_arrBars.GetSize() - 1);

    // find the target bar
    for (int i = 0; i < pDockBar->m_arrBars.GetSize(); i++)
    {
        void* p = pDockBar->m_arrBars[i];
        if (p == pTargetBar) // and insert the new bar after it
            pDockBar->m_arrBars.InsertAt(i + 1, pBar);
    }

    // move the new bar into position
    CRect rBar;
    pTargetBar->GetWindowRect(rBar);
    rBar.OffsetRect(bHorz ? 1 : 0, bHorz ? 0 : 1);
    pBar->MoveWindow(rBar);
}

//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::VerifyBarState( CDockState &state )
{
	for (int i = 0; i < state.m_arrBarInfo.GetSize(); i++)
	{
		CControlBarInfo* pInfo = (CControlBarInfo*)state.m_arrBarInfo[i];
		ASSERT(pInfo != NULL);
		int nDockedCount = pInfo->m_arrBarID.GetSize();
		if (nDockedCount > 0)
		{
			// dockbar
			for (int j = 0; j < nDockedCount; j++)
			{
				UINT_PTR nID = (UINT_PTR) pInfo->m_arrBarID[j];
				if (nID == 0) continue; // row separator
				if (nID > 0xFFFF)
					nID &= 0xFFFF; // placeholder - get the ID
				if (GetControlBar(nID) == NULL)
					return FALSE;
			}
		}

		if (!pInfo->m_bFloating) // floating dockbars can be created later
			if (GetControlBar(pInfo->m_nBarID) == NULL)
				return FALSE; // invalid bar ID
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::CreateRollupBar()
{
	CSize sz(220,500);
	CRect rc(CPoint(0,0),sz);
	m_wndRollUpBar.Create( this,IDW_VIEW_ROLLUP_BAR,_T("RollupBar"),sz,CBRS_RIGHT );
	//m_wndRollUpBar.Create( _T("RollupBar"),this,IDW_VIEW_ROLLUP_BAR );
	//m_wndRollUpBar.SetInitDesiredSizeVertical(sz);
	//m_wndRollUpBar.SetInitDesiredSizeFloating(sz);

	m_wndRollUp.Create( NULL,NULL,WS_CHILD|WS_VISIBLE,rc,&m_wndRollUpBar,m_wndRollUpBar.GetDlgCtrlID() );
	m_wndRollUpBar.SetChild( &m_wndRollUp );

	//////////////////////////////////////////////////////////////////////////
	// Create our RollupCtrl into DialogBar and register it
	m_objectRollupCtrl.Create(WS_VISIBLE | WS_CHILD, CRect(4, 4, 187, 362), &m_wndRollUp, NULL);
	m_wndRollUp.SetRollUpCtrl( ROLLUP_OBJECTS,&m_objectRollupCtrl);

	m_terrainRollupCtrl.Create(WS_VISIBLE | WS_CHILD, CRect(4, 4, 187, 362), &m_wndRollUp, NULL);
	m_wndRollUp.SetRollUpCtrl( ROLLUP_TERRAIN,&m_terrainRollupCtrl );

	m_displayRollupCtrl.Create(WS_VISIBLE | WS_CHILD, CRect(4, 4, 187, 362), &m_wndRollUp, NULL);
	m_wndRollUp.SetRollUpCtrl( ROLLUP_DISPLAY,&m_displayRollupCtrl );

	m_layersRollupCtrl.Create(WS_VISIBLE | WS_CHILD, CRect(4, 4, 187, 362), &m_wndRollUp, NULL);
	m_wndRollUp.SetRollUpCtrl( ROLLUP_LAYERS,&m_layersRollupCtrl );

	//////////////////////////////////////////////////////////////////////////
	if (!IsPreview())
	{
		// Insert the main rollup
		m_mainTools = new CMainTools;
		m_mainTools->Create(MAKEINTRESOURCE(CMainTools::IDD),this);
		m_objectRollupCtrl.InsertPage("Objects",m_mainTools);

		//	m_objectRollupCtrl.InsertPage("Main Tools",MAKEINTRESOURCE(CMainTools::IDD),RUNTIME_CLASS(CMainTools) );

		m_terrainPanel = new CTerrainPanel(this);
		m_terrainRollupCtrl.InsertPage("Terrain",m_terrainPanel );

		CPanelDisplayHide *hidePanel = new CPanelDisplayHide(this);
		m_displayRollupCtrl.InsertPage("Hide by Category",hidePanel );

		CPanelDisplayRender *renderPanel = new CPanelDisplayRender(this);
		m_displayRollupCtrl.InsertPage("Render Settings",renderPanel );

		CPanelDisplayLayer *layerPanel = new CPanelDisplayLayer(this);
		m_layersRollupCtrl.InsertPage( "Layers Settings",layerPanel );

		m_objectRollupCtrl.ExpandAllPages( TRUE );
		m_terrainRollupCtrl.ExpandAllPages(TRUE);
		m_displayRollupCtrl.ExpandAllPages(TRUE);
		m_layersRollupCtrl.ExpandAllPages(TRUE);

		SelectRollUpBar( ROLLUP_OBJECTS );
	}
	else
	{
		// Hide all menus.
	}
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::CreateMissionsBar()
{
	DWORD dwToolBarFlags = WS_CHILD|WS_VISIBLE|CBRS_ALIGN_ANY|CBRS_TOOLTIPS|CBRS_FLYBY;
	CRect nullRc(0,0,0,0);
	m_missionToolBar.CreateEx( this,TBSTYLE_TRANSPARENT|TBSTYLE_FLAT,dwToolBarFlags,CRect(0,0,0,0),IDW_VIEW_MISSION_BAR );
	m_missionToolBar.LoadToolBar(IDR_MISSION_BAR);
	m_missionToolBar.SetWindowText( _T("Mission ToolBar") );

	//////////////////////////////////////////////////////////////////////////
	// Use 24Bit toolbar.
	CImageList	toolbarImageList;
	CBitmap	toolbarBitmap;
	toolbarBitmap.LoadBitmap(IDR_MISSION_BAR);
	toolbarImageList.Create(16, 15, ILC_COLORDDB|ILC_MASK, 13, 1);
	toolbarImageList.Add(&toolbarBitmap,TOOLBAR_TRANSPARENT_COLOR);
	m_objectModifyBar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)toolbarImageList.m_hImageList);
	//////////////////////////////////////////////////////////////////////////


	// Create controls in the mission bar
	CRect rect(0,0,100,200);
  // Get the index of the keyframe slider position in the toolbar
	int iIndex = m_missionToolBar.CommandToIndex(IDC_MISSION);
	assert( iIndex >= 0 );

	m_missions.Create( WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|CBS_SORT,rect,this,IDC_MISSION );
	m_missions.SetParent( &m_missionToolBar );

	m_missionToolBar.InsertControl( &m_missions );
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnMissionUpdate()
{
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnMissionChanged()
{
	if (IsPreview())
		return;

	int sel = m_missions.GetCurSel();
	if (sel != LB_ERR)
	{
		CString str;
		m_missions.GetLBText( sel,str );
		CCryEditDoc *doc = (CCryEditDoc*)GetActiveDocument();
		CMission *mission = doc->FindMission(str);
		if (mission)
		{
			doc->SetCurrentMission( mission );

			m_currentMission = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnMissionCancelChanged()
{
	if (IsPreview())
		return;

	CCryEditDoc *doc = GetIEditor()->GetDocument();
	if (!doc)
		return;
	int sel = m_missions.FindStringExact( -1,doc->GetCurrentMission()->GetName() );
	if (sel != LB_ERR)
		m_missions.SetCurSel(sel);
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnMissionDropDown()
{
	// When mission control is about to display all missions items.
	m_missions.ResetContent();
	CCryEditDoc *doc = GetIEditor()->GetDocument();
	if (!doc)
		return;
	for (int i = 0; i < doc->GetMissionCount(); i++)
	{
		m_missions.AddString( doc->GetMission(i)->GetName() );
	}

	int sel = m_missions.FindStringExact( -1,doc->GetCurrentMission()->GetName() );
	if (sel != LB_ERR)
		m_missions.SetCurSel(sel);
}

//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// Use our own window class (Needed to dected single application instance).
	cs.lpszClass = _T("CryEditorClass");

	// Init the window with the lowest possible resolution
	cs.cx = 800;
	cs.cy = 600;
	cs.x = 10;
	cs.y = 10;

	if( !CXTFrameWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle = 0;

	//cs.lpszClass = AfxRegisterWndClass( 0, NULL, NULL,AfxGetApp()->LoadIcon(IDR_MAINFRAME));
	//cs.style &= ~FWS_ADDTOTITLE;

	return TRUE;
}

void CMainFrame::IdleUpdate()
{
	if (m_infoBarHolder.m_hWnd)
		m_infoBarHolder.IdleUpdate();

	IEditor *iface = GetIEditor();

	if (IsPreview())
		return;

	if (m_wndTrackView.m_hWnd)
	{
		m_wndTrackView.Update();
	}
	if (m_wndDataBase.m_hWnd)
		m_wndDataBase.Update();

	if (m_terrainPanel)
	{
		if (::IsWindowVisible(m_terrainPanel->GetSafeHwnd()))
			m_terrainPanel->OnIdleUpdate();
	}

	if (GetIEditor()->GetSelection()->GetName() != m_selectionName)
	{
		m_selectionName = GetIEditor()->GetSelection()->GetName();
		SetSelectionName( m_selectionName );
	}

	if (m_missions.m_hWnd)
	{
		CCryEditDoc *doc = GetIEditor()->GetDocument();
		if (doc->GetCurrentMission() != m_currentMission)
		{
			m_currentMission = GetIEditor()->GetDocument()->GetCurrentMission();
			//CString str;
			//m_missions.SetWindowText( m_currentMission->GetName() );
			if (m_missions.m_hWnd)
			{
				m_missions.ResetContent();
				for (int i = 0; i < doc->GetMissionCount(); i++)
				{
					m_missions.AddString( doc->GetMission(i)->GetName() );
				}
				int sel = m_missions.FindStringExact( -1,m_currentMission->GetName() );
				if (sel != LB_ERR)
					m_missions.SetCurSel(sel);
			}
		}
	}

	if (m_editModeBar.m_hWnd)
	{
		if (iface->GetObjectManager()->GetLayersManager()->GetCurrentLayer() != m_currentLayer)
		{
			m_currentLayer = iface->GetObjectManager()->GetLayersManager()->GetCurrentLayer();
			m_editModeBar.SetCurrentLayer( m_currentLayer->GetName() );
			RecalcLayout();
		}

		float gridSize = iface->GetViewManager()->GetGrid()->size;
		if (gridSize != m_gridSize)
		{
			m_gridSize = gridSize;
			m_editModeBar.SetGridSize( m_gridSize );
			RecalcLayout();
		}
	}
}

void CMainFrame::UncheckMainTools()
{
	if (m_mainTools)
	{
		m_mainTools->UncheckAll();
	}
}

void CMainFrame::DockControlBarLeftOf(CControlBar *Bar, CControlBar *LeftOf)
{
	////////////////////////////////////////////////////////////////////////
	// Dock a control bar left of another
	////////////////////////////////////////////////////////////////////////

	CRect rect;
	DWORD dw;
	UINT n;

	// Get MFC to adjust the dimensions of all docked ToolBars
	// so that GetWindowRect will be accurate
	RecalcLayout(TRUE);

	LeftOf->GetWindowRect(&rect);
	rect.OffsetRect(1, 0);
	dw = LeftOf->GetBarStyle();
	n = 0;
	n = (dw & CBRS_ALIGN_TOP) ? AFX_IDW_DOCKBAR_TOP : n;
	n = (dw & CBRS_ALIGN_BOTTOM && n == 0) ? AFX_IDW_DOCKBAR_BOTTOM : n;
	n = (dw & CBRS_ALIGN_LEFT && n == 0) ? AFX_IDW_DOCKBAR_LEFT : n;
	n = (dw & CBRS_ALIGN_RIGHT && n == 0) ? AFX_IDW_DOCKBAR_RIGHT : n;

	// When we take the default parameters on rect, DockControlBar will dock
	// each Toolbar on a seperate line. By calculating a rectangle, we
	// are simulating a Toolbar being dragged to that location and docked.
	DockControlBar(Bar, n, &rect);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CXTFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CXTFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CXTFrameWnd::OnSize(nType, cx, cy);
}

void CMainFrame::OnUpdateToolbar(CCmdUI* pCmdUI)
{
	//pCmdUI->SetCheck(m_ReBar.IsWindowVisible());
}

void CMainFrame::OnToolbar()
{
	//ShowControlBar(&m_ReBar, !m_ReBar.IsWindowVisible(), FALSE);
}

void CMainFrame::OnUpdateConsole(CCmdUI* pCmdUI)
{
	//pCmdUI->SetCheck(m_cConsole.IsWindowVisible());
}

void CMainFrame::OnConsoleWindow()
{
	/*
	if (GetIEditor()->IsInGameMode())
		return;

	// Control the visibility of the console
	ShowControlBar(&m_cConsole, !m_cConsole.IsVisible(), FALSE);
	if (m_cConsole.IsVisible())
	{
		m_cConsole.SetInputFocus();
	}
	*/
}

bool CMainFrame::ShowConsole( bool enable )
{
	/*
	// Control the visibility of the console
	if (m_cConsole.IsFloating())
	{
		ShowControlBar(&m_cConsole, enable, FALSE);
		bool result = m_consoleVisible;
		m_consoleVisible = enable;
	}
	m_consoleVisible = true;
	*/
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateTrackView(CCmdUI* pCmdUI)
{
	//pCmdUI->SetCheck(m_wndTrackView.IsWindowVisible());
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnTrackView()
{
	//ShowControlBar(&m_wndTrackView, !m_wndTrackView.IsVisible(), FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateStatusBar(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_wndStatusBar.IsWindowVisible());
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnStatusBar()
{
	ShowControlBar(&m_wndStatusBar, !m_wndStatusBar.IsVisible(), TRUE);
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateRollUpBar(CCmdUI* pCmdUI)
{
	//pCmdUI->SetCheck(m_wndRollUp.IsWindowVisible());
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnRollUpBar()
{
	//ShowControlBar(&m_wndRollUp, !m_wndRollUp.IsVisible(), FALSE);
}

//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class
	//m_layoutWnd.Create( this,2,2,CSize(10,10),pContext );
	m_layoutWnd = new CLayoutWnd;
	CRect rc;
	m_layoutWnd->CreateEx( 0,NULL,NULL,WS_CHILD|WS_VISIBLE,rc,this,AFX_IDW_PANE_FIRST );
	if (IsPreview())
	{
		m_layoutWnd->CreateLayout( ET_Layout0,true,ET_ViewportModel );
	}
	else
	{
		if (!m_layoutWnd->LoadConfig())
			m_layoutWnd->CreateLayout( ET_Layout2 );
	}

	return TRUE;
	//return CXTFrameWnd::OnCreateClient(lpcs, pContext);
}

//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	// TODO: Add your message handler code here and/or call default
	if (pCopyDataStruct->dwData = 100 && pCopyDataStruct->lpData != NULL)
	{
		char str[1024];
		memcpy( str,pCopyDataStruct->lpData,pCopyDataStruct->cbData );
		str[pCopyDataStruct->cbData] = 0;

		// Load this file.
		((CCryEditApp*)AfxGetApp())->LoadFile( str );
	}

	return CXTFrameWnd::OnCopyData(pWnd, pCopyDataStruct);
}

//////////////////////////////////////////////////////////////////////////
CString CMainFrame::GetSelectionName()
{
	return m_editModeBar.GetSelection();
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::AddSelectionName( const CString &name )
{
	m_editModeBar.AddSelection( name );
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::RemoveSelectionName( const CString &name )
{
	m_editModeBar.RemoveSelection( name );
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::SetSelectionName( const CString &name )
{
	if (m_editModeBar.m_hWnd)
		m_editModeBar.SetSelection( name );
}

//////////////////////////////////////////////////////////////////////////
int CMainFrame::SelectRollUpBar( int rollupBarId )
{
	if (m_wndRollUp.m_hWnd)
		m_wndRollUp.Select( rollupBarId );
	return rollupBarId;
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnClose()
{
	if (!GetIEditor()->GetDocument()->CanCloseFrame(this))
		return;
	GetIEditor()->GetDocument()->SetModifiedFlag(FALSE);

	GetIEditor()->GetSystem()->Quit();

	// Close all edit panels.
	GetIEditor()->ClearSelection();
	GetIEditor()->SetEditTool(0);
	GetIEditor()->GetObjectManager()->EndEditParams();

	SaveConfig();

	CXTFrameWnd::OnClose();
}

//////////////////////////////////////////////////////////////////////////
CRollupCtrl* CMainFrame::GetRollUpControl( int rollupBarId )
{
	if (m_hWnd == 0)
		return 0;
	if (rollupBarId == ROLLUP_OBJECTS)
	{
		return &m_objectRollupCtrl;
	} else if (rollupBarId == ROLLUP_TERRAIN)
	{
		return &m_terrainRollupCtrl;
	} else if (rollupBarId == ROLLUP_DISPLAY)
	{
		return &m_displayRollupCtrl;
	} else if (rollupBarId == ROLLUP_LAYERS)
	{
		return &m_layersRollupCtrl;
	}
	// Default.
	return &m_objectRollupCtrl;
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::EnableProgressBar( bool bEnable )
{
	m_infoBarHolder.EnableProgressBar(bEnable);
}

//////////////////////////////////////////////////////////////////////////
CInfoProgressBar* CMainFrame::GetProgressBar()
{
	return m_infoBarHolder.GetProgressBar();
};

//////////////////////////////////////////////////////////////////////////
CTrackViewDialog* CMainFrame::GetTrackView()
{
	return &m_wndTrackView;
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::SetStatusText( LPCTSTR pszText)
{
	if (m_wndStatusBar.m_hWnd)
		m_wndStatusBar.SetPaneText(0, pszText);
};

//////////////////////////////////////////////////////////////////////////
void CMainFrame::ActivateFrame(int nCmdShow)
{
	CXTFrameWnd::ActivateFrame(nCmdShow);
}

void CMainFrame::ShowWindowEx(int nCmdShow)
{
	if (!IsPreview())
	{
		// Restore control bar postion.
		LoadBarState(_T("Bar State"));
	}

	// Restore frame window size and position.
	if (!m_wndPosition.LoadWindowPos(this))
	{
		nCmdShow = m_wndPosition.showCmd;
	}

	CXTFrameWnd::ShowWindow(nCmdShow);
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnXPLook()
{
	xtAfxData.bXPMode = !xtAfxData.bXPMode;

	m_bXPLook = xtAfxData.bXPMode;

	RedrawWindow( NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN );

	/*
	m_bXPLook = !m_bXPLook;
	if (m_bXPLook)
	{
		// Install XP paint manager.
		VERIFY(	g_PaintManager.InstallPaintManager( new CExtPaintManagerXP ) );
		CExtPopupMenuWnd::g_bMenuWithShadows = true;
	}
	else
	{
		// Install w2k paint manager.
		VERIFY(	g_PaintManager.InstallPaintManager( new CExtPaintManager ) );
		CExtPopupMenuWnd::g_bMenuWithShadows = false;
	}
	RecalcLayout();
	RedrawWindow( NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_FRAME|RDW_ALLCHILDREN );
	*/
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateXPLook( CCmdUI* pCmdUI )
{
	if (m_bXPLook)
		pCmdUI->SetCheck(TRUE);
	else
		pCmdUI->SetCheck(FALSE);
}

//////////////////////////////////////////////////////////////////////////
bool CMainFrame::IsPreview() const
{
	return GetIEditor()->IsInPreviewMode();
}

//////////////////////////////////////////////////////////////////////////
BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
	if (!CXTFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
		return FALSE;

	CXTAccelManager &accelManager = CXTAccelManager::Get();
	/*
	AddToolbarToAccel( _T("Toolbar EditMode"),&m_editModeBar );
	AddToolbarToAccel( _T("Toolbar ObjectModify"),&m_objectModifyBar );
	AddToolbarToAccel( _T("Toolbar Mission"),&m_missionToolBar );
	AddToolbarToAccel( _T("Toolbar Terrain"),&m_wndTerrainToolBar );

	// Add tag points.
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC1,_T("Tag Location 1") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC2,_T("Tag Location 2") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC3,_T("Tag Location 3") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC4,_T("Tag Location 4") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC5,_T("Tag Location 5") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC6,_T("Tag Location 6") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC7,_T("Tag Location 7") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC8,_T("Tag Location 8") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC9,_T("Tag Location 9") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC10,_T("Tag Location 10") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC11,_T("Tag Location 11") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_TAG_LOC12,_T("Tag Location 12") );

	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC1,_T("Goto Tagged Location 1") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC2,_T("Goto Tagged Location 2") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC3,_T("Goto Tagged  Location 3") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC4,_T("Goto Tagged Location 4") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC5,_T("Goto Tagged Location 5") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC6,_T("Goto Tagged Location 6") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC7,_T("Goto Tagged Location 7") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC8,_T("Goto Tagged Location 8") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC9,_T("Goto Tagged Location 9") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC10,_T("Goto Tagged Location 10") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC11,_T("Goto Tagged Location 11") );
	accelManager.AddExtraCommand( _T("Tag Location"),ID_GOTO_LOC12,_T("Goto Tagged Location 12") );
	*/

	/*
	// Tools extra commands.
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL1,_T("Tool1") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL2,_T("Tool2") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL3,_T("Tool3") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL4,_T("Tool4") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL5,_T("Tool5") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL6,_T("Tool6") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL7,_T("Tool7") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL8,_T("Tool8") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL9,_T("Tool9") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL10,_T("Tool10") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL11,_T("Tool11") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL12,_T("Tool12") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL13,_T("Tool13") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL14,_T("Tool14") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL15,_T("Tool15") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL16,_T("Tool16") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL17,_T("Tool17") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL18,_T("Tool18") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL19,_T("Tool19") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL20,_T("Tool20") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL21,_T("Tool21") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL22,_T("Tool22") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL23,_T("Tool23") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL24,_T("Tool24") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL25,_T("Tool25") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL26,_T("Tool26") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL27,_T("Tool27") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL28,_T("Tool28") );
	accelManager.AddExtraCommand( _T("Tools"),ID_TOOL29,_T("Tool29") );
*/

	// Initialize accelerator key manager.
	//accelManager.Init(this, IDR_MAINFRAME, _T("Main Frame"), _T("MainFrameKeys"));
	// initialize accelerator manager.
	if (!InitAccelManager() )
	{
		return FALSE;
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::EnableAccelerator( bool bEnable )
{
	if (bEnable)
	{
		CXTAccelManager &accelManager = CXTAccelManager::Get();
		//LoadAccelTable( MAKEINTRESOURCE(IDR_MAINFRAME) );
		accelManager.UpdateWindowAccelerator();
		CLogFile::WriteLine( "Enable Accelerators" );
	}
	else
	{
		if (m_hAccelTable)
			DestroyAcceleratorTable( m_hAccelTable );
		m_hAccelTable = NULL;
		LoadAccelTable( MAKEINTRESOURCE(IDR_GAMEACCELERATOR) );
		CLogFile::WriteLine( "Disable Accelerators" );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::EditAccelerator()
{
	// Open accelerator key manager dialog.
	//accelManager.EditKeyboardShortcuts(this);
	OnCustomizeBar();
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::AddToolbarToAccel( const CString &name,CXTToolBar *toolbar )
{
	assert( toolbar );
	CXTAccelManager &accelManager = CXTAccelManager::Get();
	for (int i = 0; i < toolbar->GetButtonCount(); i++)
	{
		CString str;
		UINT cmdId = toolbar->GetItemID(i);
		if (str.LoadString(cmdId) != 0)
		{
			accelManager.AddExtraCommand( name,cmdId,str );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CMainFrame::FindMenuPos(CMenu *pBaseMenu, UINT myID, CMenu * & pMenu, int & mpos)
{
	// REMARK: pMenu is a pointer to a Cmenu-Pointer
	int myPos;
	if( pBaseMenu == NULL )
	{
		// Sorry, Wrong Number
		pMenu = NULL;
		mpos = -1;
		return false;
	}
	for( myPos = pBaseMenu->GetMenuItemCount() -1; myPos >= 0; myPos-- )
	{
		int Status = pBaseMenu->GetMenuState( myPos, MF_BYPOSITION);
		CMenu* mNewMenu;

		if( Status == 0xFFFFFFFF )
		{
			// That was not an legal Menu/Position-Combination
			pMenu = NULL;
			mpos = -1;
			return false;
		}
		// Is this the real one?
		if( pBaseMenu->GetMenuItemID(myPos) == myID )
		{
			// Yep!
			pMenu = pBaseMenu;
			mpos = myPos;
			return true;
		}
		// Maybe a subMenu?
		mNewMenu = pBaseMenu->GetSubMenu(myPos);
		// This function will return NULL if ther is NO SubMenu
		if( mNewMenu != NULL )
		{
			// rekursive!
			bool found = FindMenuPos( mNewMenu, myID, pMenu, mpos);
			if(found)
				return true;	// return this loop
		}
		// I have to check the next in my loop
	}
	return false; // iterate in the upper stackframe
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::DeleteToolsFromMenu( CMenu *menu )
{
	int pos = 0;
	while (pos < menu->GetMenuItemCount())
	{
		int status = menu->GetMenuState( pos, MF_BYPOSITION);
		if (status == 0xFFFFFFFF)
		{
			// That was not an legal Menu/Position-Combination
			return;
		}
		UINT mId = menu->GetMenuItemID(pos);
		if (mId >= ID_TOOL2 && mId <= ID_TOOL30)
		{
			// Delete this item.
			menu->DeleteMenu( pos,MF_BYPOSITION );
		}
		else
		{
			pos++;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::UpdateTools()
{
	CMenu *pMenu = m_wndMenuBar.GetMenu();
	int m_Pos = 0;
	CMenu *pmMenu = pMenu;

	bool res = FindMenuPos( pMenu, ID_TOOL1, pmMenu, m_Pos );
	if (pmMenu && res)
	{
		DeleteToolsFromMenu( pmMenu );
		//pmMenu->DeleteMenu( m_Pos,MF_BYPOSITION );
		CExternalToolsManager *pTools = GetIEditor()->GetExternalToolsManager();
		for (int i = 0; i < pTools->GetToolsCount(); i++)
		{
			if (i == 0)
			{
				// Replace Tool1.
				pmMenu->ModifyMenu( m_Pos+i,MF_BYPOSITION|MF_STRING, ID_TOOL1+i, pTools->GetTool(i)->m_title );
			}
			else
        pmMenu->InsertMenu( m_Pos+i,MF_BYPOSITION|MF_STRING, ID_TOOL1+i, pTools->GetTool(i)->m_title );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == AUTOSAVE_TIMER_EVENT && gSettings.autoBackupEnabled)
	{
		// Call autosave function of CryEditApp.
		((CCryEditApp*)AfxGetApp())->SaveAutoBackup();
	}
	if (nIDEvent == AUTOREMIND_TIMER_EVENT && gSettings.autoRemindTime > 0)
	{
		// Remind to save.
		((CCryEditApp*)AfxGetApp())->SaveAutoRemind();
	}

	CXTFrameWnd::OnTimer(nIDEvent);
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnEditNextSelectionMask()
{
	m_editModeBar.NextSelectMask();
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::ShowDataBaseDialog( bool bShow )
{
	ShowControlBar( &m_wndDataBaseBar,bShow,TRUE );
	m_wndDataBaseBar.Invalidate();
	//m_wndDataBase.RedrawWindow( NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN );
}

//////////////////////////////////////////////////////////////////////////
bool CMainFrame::IsDockedWindowChild( CWnd *pWnd )
{
	if (!pWnd)
		return false;
	if (pWnd->IsChild(&m_wndDataBaseBar))
		return true;
	if (pWnd->IsChild(&m_wndRollUpBar))
		return true;
	if (pWnd->IsChild(&m_wndConsoleBar))
		return true;
	if (pWnd->IsChild(&m_wndTrackViewBar))
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CMainFrame::OnProgressBarCancel()
{
	CWaitProgress::CancelCurrent();
}
