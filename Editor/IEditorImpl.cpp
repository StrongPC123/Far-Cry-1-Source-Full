////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   IEditorImpl.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CEditorImpl class implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "IEditorImpl.h"
#include "CryEdit.h"
#include "CryEditDoc.h"
#include "SelectFileDlg.h"
#include "CustomColorDialog.h"
#include "Plugin.h"

#include "PluginManager.h"
#include "IconManager.h"
#include "ViewManager.h"
#include "SoundPresetMgr.h"
#include "ViewPane.h"
#include "InfoProgressBar.h"
#include "Objects\ObjectManager.h"
#include "DisplaySettings.h"
#include "ShaderEnum.h"
#include "EditTool.h"
#include "PickObjectTool.h"
#include "ObjectCreateTool.h"
#include "SoundPresetsDlg.h"
#include "EAXPresetMgr.h"

#include "EquipPackLib.h"

#include "AI\AIManager.h"

#include "Undo\Undo.h"

#include "Material\MaterialManager.h"
#include "EntityPrototypeManager.h"
#include "AnimationContext.h"
#include "GameEngine.h"
#include "ExternalTools.h"
#include "Settings.h"
#include "BaseLibraryDialog.h"
#include "Material\Material.h"
#include "EntityPrototype.h"
#include "Particles\ParticleManager.h"
#include "Music\MusicManager.h"
#include "Prefabs\PrefabManager.h"

//////////////////////////////////////////////////////////////////////////
// Tools.
//////////////////////////////////////////////////////////////////////////
#include "TerrainModifyTool.h"
#include "VegetationTool.h"

//////////////////////////////////////////////////////////////////////////
#include <I3DEngine.h>
#include <IConsole.h>
#include <IEntitySystem.h>
#include <IGame.h>
#include <IMovieSystem.h>

#pragma comment(lib, "version.lib")

//////////////////////////////////////////////////////////////////////////
// Pointer to global document instance.
//////////////////////////////////////////////////////////////////////////
static CCryEditDoc* theDocument;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CEditorImpl::CEditorImpl()
{
	m_system = 0;

	m_currEditMode = eEditModeSelect;
	m_prevEditMode = m_currEditMode;
	m_editTool = 0;

	SetMasterCDFolder();

	gSettings.Load();

	m_gameEngine = 0;
	m_pErrorReport = new CErrorReport;
	m_classFactory = CClassFactory::Instance();
	m_pCommandManager = new CCommandManager;
	m_displaySettings = new CDisplaySettings;
	m_shaderEnum = new CShaderEnum;
	m_displaySettings->LoadRegistry();
	m_pluginMan = new CPluginManager;
	m_objectMan = new CObjectManager;
	m_viewMan = new CViewManager;
	m_iconManager = new CIconManager;
	m_undoManager = new CUndoManager;
	m_pSoundPresetMgr = new CSoundPresetMgr;
	m_pEAXPresetMgr = new CEAXPresetMgr;
	m_AI = new CAIManager;
	m_animationContext = new CAnimationContext;
	m_pEquipPackLib = new CEquipPackLib;
	m_externalToolsManager = new CExternalToolsManager;
	m_materialManager = new CMaterialManager;
	m_entityManager = new CEntityPrototypeManager;
	m_particleManager = new CParticleManager;
	m_pMusicManager = new CMusicManager;
	m_pPrefabManager = new CPrefabManager;

	m_marker(0,0,0);
	m_selectedRegion.min=Vec3d(0,0,0);
	m_selectedRegion.max=Vec3d(0,0,0);

	m_selectedAxis = AXIS_TERRAIN;
	m_refCoordsSys = COORDS_LOCAL;

	ZeroStruct(m_lastAxis);
	m_lastAxis[eEditModeSelect] = AXIS_TERRAIN;
	m_lastAxis[eEditModeSelectArea] = AXIS_TERRAIN;
	m_lastAxis[eEditModeMove] = AXIS_TERRAIN;
	m_lastAxis[eEditModeRotate] = AXIS_Z;
	m_lastAxis[eEditModeScale] = AXIS_XY;

	ZeroStruct(m_lastCoordSys);
	m_lastCoordSys[eEditModeSelect] = COORDS_LOCAL;
	m_lastCoordSys[eEditModeSelectArea] = COORDS_LOCAL;
	m_lastCoordSys[eEditModeMove] = COORDS_VIEW;
	m_lastCoordSys[eEditModeRotate] = COORDS_LOCAL;
	m_lastCoordSys[eEditModeScale] = COORDS_LOCAL;

	m_bUpdates = true;

	m_viewerPos(0,0,0);
	m_viewerAngles(0,0,0);

	CLogFile::WriteLine("Editor plugin interface created");
	DetectVersion();

	m_bSelectionLocked = true;
	m_bTerrainAxisIgnoreObjects = true;

	RegisterTools();

	m_pickTool = 0;
}

//////////////////////////////////////////////////////////////////////////
CEditorImpl::~CEditorImpl()
{
//	if (m_movieSystem)
//		delete m_movieSystem;
	gSettings.Save();

	SAFE_DELETE( m_pPrefabManager );

	if (m_pMusicManager)
		delete m_pMusicManager;

	if (m_particleManager)
		delete m_particleManager;

	if (m_entityManager)
		delete m_entityManager;

	if (m_materialManager)
		delete m_materialManager;

	if (m_AI)
		delete m_AI;

	if (m_pEquipPackLib)
		delete m_pEquipPackLib;

	if (m_undoManager)
		delete m_undoManager;
	m_undoManager = 0;

	if (m_iconManager)
		delete m_iconManager;

	if (m_pSoundPresetMgr)
		delete m_pSoundPresetMgr;

	if (m_pEAXPresetMgr)
		delete m_pEAXPresetMgr;

	if (m_viewMan)
		delete m_viewMan;

	if (m_objectMan)
		delete m_objectMan;
	
	if (m_pluginMan)
		delete m_pluginMan;

	if (m_animationContext)
		delete m_animationContext;

	if (m_displaySettings)
	{
		m_displaySettings->SaveRegistry();
		delete m_displaySettings;
	}

	if (m_shaderEnum)
		delete m_shaderEnum;

	if (m_gameEngine)
		delete m_gameEngine;

	delete m_externalToolsManager;
	
	delete m_pCommandManager;

	delete m_classFactory;

	delete m_pErrorReport;

	CLogFile::WriteLine("Editor plugin interface destroyed");
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetMasterCDFolder()
{
	// Set`s root folder of the Editor.
	char szExeFileName[_MAX_PATH];
	// Get the path of the executable
	GetModuleFileName( GetModuleHandle(NULL), szExeFileName, sizeof(szExeFileName));
	// MasterCD folder is a one before executable folder (Executable is in MasterCD\Bin32 or MasterCD\Bin64).
	CString exePath = Path::GetPath( szExeFileName );
	exePath = Path::AddBackslash(exePath) + ".."; // previous path.
	// Set MasterCD folder as current.
	SetCurrentDirectory( exePath );
	char szMasterCDPath[_MAX_PATH];
	GetCurrentDirectory( sizeof(szMasterCDPath),szMasterCDPath );
	m_masterCDFolder = Path::AddBackslash(szMasterCDPath);
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetGameEngine( CGameEngine *ge )
{
	m_system = ge->GetSystem();
	m_gameEngine = ge;
	if (m_pSoundPresetMgr)
		m_pSoundPresetMgr->Load();
	if (m_pEAXPresetMgr)
		m_pEAXPresetMgr->Load();

	m_templateRegistry.LoadTemplates( "Editor" );
	m_objectMan->LoadClassTemplates( "Editor" );
}

//////////////////////////////////////////////////////////////////////////
// Call registrations.
//////////////////////////////////////////////////////////////////////////
void CEditorImpl::RegisterTools()
{
	CRegistrationContext rc;
	rc.pCommandManager = m_pCommandManager;
	rc.pClassFactory = m_classFactory;

	//////////////////////////////////////////////////////////////////////////
	// Register various tool classes and commands.
	CTerrainModifyTool::RegisterTool( rc );
	CVegetationTool::RegisterTool( rc );
}


//////////////////////////////////////////////////////////////////////////
void CEditorImpl::Update()
{
	if (!m_bUpdates)
		return;
	
	FUNCTION_PROFILER( GetSystem(),PROFILE_EDITOR );

	//@FIXME: Restore this latter.
	//if (GetGameEngine() && GetGameEngine()->IsLevelLoaded())
	{	
		m_animationContext->Update();
		m_viewMan->Update();
		m_objectMan->Update();
	}
	if (IsInPreviewMode())
	{
		SetModifiedFlag(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
ISystem* CEditorImpl::GetSystem()
{
	return m_system;
}

//////////////////////////////////////////////////////////////////////////
I3DEngine* CEditorImpl::Get3DEngine()
{
	return m_system->GetI3DEngine();
}

//////////////////////////////////////////////////////////////////////////	
IRenderer*	CEditorImpl::GetRenderer()
{
	return m_system->GetIRenderer();
}

//////////////////////////////////////////////////////////////////////////
IGame*	CEditorImpl::GetGame()
{
	return m_system->GetIGame();
}

CClassFactory* CEditorImpl::GetClassFactory()
{
	return m_classFactory;
}

//////////////////////////////////////////////////////////////////////////
CCryEditDoc* CEditorImpl::GetDocument()
{
	/*
	CFrameWnd *wnd = dynamic_cast<CFrameWnd*>( AfxGetMainWnd() );
	if (wnd)
		return dynamic_cast<CCryEditDoc*>(wnd->GetActiveDocument());
		*/
	return theDocument;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetDocument( CCryEditDoc *pDoc )
{
	theDocument = pDoc;
	// Register document callbacks with managers.
	RegisterDocListener( m_entityManager );
	RegisterDocListener( m_materialManager );
	RegisterDocListener( m_particleManager );
	RegisterDocListener( m_pMusicManager );
	RegisterDocListener( m_pPrefabManager );
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetModifiedFlag( bool modified )
{
	if (GetDocument())
	{
		GetDocument()->SetModifiedFlag( modified );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::IsModified()
{
	if (GetDocument())
	{
		return GetDocument()->IsModified();
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::SaveDocument()
{
	if (GetDocument())
		return GetDocument()->Save();
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
const char * CEditorImpl::GetMasterCDFolder()
{ 
	return m_masterCDFolder;
}

//////////////////////////////////////////////////////////////////////////
CString CEditorImpl::GetRelativePath( const CString &fullPath )
{
	if (fullPath.IsEmpty())
		return "";

	// Create relative path
	CString relPath;
	char path[_MAX_PATH];
	char srcpath[_MAX_PATH];
	strcpy( srcpath,fullPath );
	PathRelativePathTo( path,GetMasterCDFolder(),FILE_ATTRIBUTE_DIRECTORY,srcpath, NULL);

	relPath = path;
	int len = strlen(path);
	if (len > 0 && path[0] == '\\')
	{
		// Remove the leading backslash
		relPath = path+1;
	}
	return relPath;
}

//////////////////////////////////////////////////////////////////////////
CString CEditorImpl::GetLevelFolder()
{
	CString folder;
	if (GetDocument())
	{
		char str[_MAX_PATH];
		strcpy( str,GetDocument()->GetPathName() );
		PathRemoveFileSpec( str );
		PathRemoveBackslash( str );
		folder = str;
	}
	return folder;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetDataModified()
{ 
	GetDocument()->SetModifiedFlag(); 
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::AddMenuItem(uint8 iId, bool bIsSeparator,
																				 eMenuInsertLocation eParent, 
																				 IUIEvent *pIHandler)
{
	//////////////////////////////////////////////////////////////////////
	// Adds a plugin menu item and binds an event handler to it
	//////////////////////////////////////////////////////////////////////
	/*

	PluginIt it;
	CMenu *pMainMenu = NULL;
	CMenu *pLastPluginMenu = NULL;
	DWORD dwMenuID;

	IPlugin *pIAssociatedPlugin = GetPluginManager()->GetAssociatedPlugin();
	uint8 iAssociatedPluginUIID = GetPluginManager()->GetAssociatedPluginUIID();

	ASSERT(!IsBadReadPtr(pIAssociatedPlugin, sizeof(IPlugin)));
	if (!bIsSeparator)
		ASSERT(!IsBadReadPtr(pIHandler, sizeof(IUIEvent)));

	if (pIAssociatedPlugin == NULL || pIHandler == NULL)
		return false;

	// Get the main menu
	pMainMenu = AfxGetMainWnd()->GetMenu();
	ASSERT(pMainMenu);

	// Create the menu ID. The first 8 bits of the upper 16 bits contain the
	// UI ID of the plugin which owns the UI element, the second 8 bits
	// contain the user interface element ID. Set the lower 16 bits to
	// a high number in order to avoid conflicts
	dwMenuID = (iAssociatedPluginUIID | (iId << 8)) | 0xFFFF0000;
	
	switch (eParent)
	{
		// Custom plugin menu
		case eMenuPlugin:
			pLastPluginMenu = pMainMenu->GetSubMenu(pMainMenu->GetMenuItemCount() - 1);
			break;
		// Pre-defined editor menus
		case eMenuFile:
			pLastPluginMenu = pMainMenu->GetSubMenu(FindMenuItem(pMainMenu, "&File"));
			break;
		case eMenuEdit:
			pLastPluginMenu = pMainMenu->GetSubMenu(FindMenuItem(pMainMenu, "&Edit"));
			break;
		case eMenuInsert:
			pLastPluginMenu = pMainMenu->GetSubMenu(FindMenuItem(pMainMenu, "&Insert"));
			break;
		case eMenuGenerators:
			pLastPluginMenu = pMainMenu->GetSubMenu(FindMenuItem(pMainMenu, "&Generators"));
			break;
		case eMenuScript:
			pLastPluginMenu = pMainMenu->GetSubMenu(FindMenuItem(pMainMenu, "&Script"));
			break;
		case eMenuView:
			pLastPluginMenu = pMainMenu->GetSubMenu(FindMenuItem(pMainMenu, "&View"));
			break;
		case eMenuHelp:
			pLastPluginMenu = pMainMenu->GetSubMenu(FindMenuItem(pMainMenu, "&Help"));
			break;
		// Unknown identifier or parent ID passed
		default:
			CLogFile::WriteLine("AddMenuItem(): Unknown parent menu ID passed, incompatible plugin version ?");
			break;
	}

	// Add an menu item to the menu

	ASSERT(pLastPluginMenu);

	if (pLastPluginMenu)
	{
		// Insert the menu
		if (!pLastPluginMenu->AppendMenu(MF_STRING | bIsSeparator ? MF_SEPARATOR : NULL, dwMenuID, 
			pIHandler->GetUIElementName(iId)))
		{
			return false;
		}

		// Register the associated event and ID
		GetPluginManager()->AddHandlerForCmdID(pIAssociatedPlugin, iId, pIHandler);

		return true;
	}
	else
	{
		CLogFile::WriteLine("Can't find specified menu !");
		return false;
	}
	*/

	return true;
}

/*
//////////////////////////////////////////////////////////////////////////
int CEditorImpl::FindMenuItem(CMenu *pMenu, LPCTSTR pszMenuString)
{
	//////////////////////////////////////////////////////////////////////
	// FindMenuItem() will find a menu item string from the specified
	// popup menu and returns its position (0-based) in the specified 
	// popup menu. It returns -1 if no such menu item string is found
	//////////////////////////////////////////////////////////////////////

	int iCount, i;
	CString str;

  ASSERT(pMenu);
  ASSERT(::IsMenu(pMenu->GetSafeHmenu()));

  iCount = pMenu->GetMenuItemCount();

  for (i=0; i<iCount; i++)
  {
		if (pMenu->GetMenuString(i, str, MF_BYPOSITION) &&
			(strcmp(str, pszMenuString) == 0))
		{
      return i;
		}
  }

  return -1;
}
*/

/*
bool CEditorImpl::RegisterPluginToolTab(HWND hwndContainer, char *pszName)
{
	//////////////////////////////////////////////////////////////////////
	// Register a new tab with an UI container
	//////////////////////////////////////////////////////////////////////

	ASSERT(::IsWindow(hwndContainer));
	ASSERT(pszName);

	(CCryEditView *) ((CFrameWnd *) (AfxGetMainWnd())->
		GetActiveView())->RegisterPluginToolTab(hwndContainer, pszName);

	return true;
}

HWND CEditorImpl::GetToolbarTabContainer()
{
	//////////////////////////////////////////////////////////////////////
	// Return the window handle of the toolbar container
	//////////////////////////////////////////////////////////////////////

	return (CCryEditView *) ((CFrameWnd *) (AfxGetMainWnd())->
		GetActiveView())->GetToolTabContainerWnd();
}
*/

bool CEditorImpl::CreateRootMenuItem(const char *pszName)
{
	//////////////////////////////////////////////////////////////////////
	// Create the root menu for the plugin
	//////////////////////////////////////////////////////////////////////
	/*

	CMenu *pMainMenu;
	
	IPlugin *pIAssociatedPlugin = GetPluginManager()->GetAssociatedPlugin();
	ASSERT(!IsBadReadPtr(pIAssociatedPlugin, sizeof(IPlugin)));

	// Get the main menu
	pMainMenu = AfxGetMainWnd()->GetMenu();

	// Insert the menu
	if (!pMainMenu->AppendMenu(MF_BYPOSITION | MF_POPUP | MF_STRING, 
		(UINT) pMainMenu->GetSafeHmenu(), pszName))
	{
		return false;
	}

	CLogFile::FormatLine("Root menu for plugin created ('%s')", pszName);
	*/

	return true;
}

//////////////////////////////////////////////////////////////////////////
const char * CEditorImpl::GetEditorDocumentName()
{ 
	//////////////////////////////////////////////////////////////////////
	// Return the path of the editor's current document
	//////////////////////////////////////////////////////////////////////

	static char szPath[_MAX_PATH];
	CString strNonConst(GetDocument()->GetPathName());

	strcpy(szPath, strNonConst.GetBuffer(1));

	return szPath; 
}

//////////////////////////////////////////////////////////////////////////
int CEditorImpl::SelectRollUpBar( int rollupBarId )
{
	return ((CMainFrame*)AfxGetMainWnd())->SelectRollUpBar( rollupBarId );
}

//////////////////////////////////////////////////////////////////////////
int CEditorImpl::AddRollUpPage(int rollbarId,LPCTSTR pszCaption, class CDialog *pwndTemplate, 
		                                         bool bAutoDestroyTpl, int iIndex)
{
	//CLogFile::FormatLine("Rollup page inserted ('%s')", pszCaption);
	if (!GetRollUpControl(rollbarId))
		return 0;
	// Preserve Focused window.
	HWND hFocusWnd = GetFocus();
	int id = GetRollUpControl(rollbarId)->InsertPage(pszCaption, pwndTemplate, bAutoDestroyTpl, iIndex);
	//GetRollUpControl(rollbarId)->ExpandPage(id,true,false);
	// Make sure focus stay in main wnd.
	if (hFocusWnd && GetFocus() != hFocusWnd)
		SetFocus(hFocusWnd);
	return id;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::RemoveRollUpPage(int rollbarId,int iIndex)
{
	if (!GetRollUpControl(rollbarId))
		return;
	GetRollUpControl(rollbarId)->RemovePage(iIndex);
}

//////////////////////////////////////////////////////////////////////////	
void CEditorImpl::ExpandRollUpPage(int rollbarId,int iIndex, BOOL bExpand)
{
	if (!GetRollUpControl(rollbarId))
		return;

	// Preserve Focused window.
	HWND hFocusWnd = GetFocus();

	GetRollUpControl(rollbarId)->ExpandPage(iIndex, bExpand);
	
	// Preserve Focused window.
	if (hFocusWnd && GetFocus() != hFocusWnd)
		SetFocus(hFocusWnd);
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::EnableRollUpPage(int rollbarId,int iIndex, BOOL bEnable)
{
	if (!GetRollUpControl(rollbarId))
		return;

	// Preserve Focused window.
	HWND hFocusWnd = GetFocus();

	GetRollUpControl(rollbarId)->EnablePage(iIndex, bEnable);
	
	// Preserve Focused window.
	if (hFocusWnd && GetFocus() != hFocusWnd)
		SetFocus(hFocusWnd);
}

//////////////////////////////////////////////////////////////////////////
HWND CEditorImpl::GetRollUpContainerWnd(int rollbarId)
{
	if (!GetRollUpControl(rollbarId))
		return 0;
	return GetRollUpControl(rollbarId)->GetSafeHwnd();
}		

//////////////////////////////////////////////////////////////////////////
int CEditorImpl::GetEditMode()
{
	return m_currEditMode;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetEditMode( int editMode )
{
	//! Disable editing tool.
	SetEditTool(0);
	
	m_currEditMode = (eEditMode)editMode;
	m_prevEditMode = m_currEditMode;
	BBox box( Vec3(0,0,0),Vec3(0,0,0) );
	SetSelectedRegion( box );
	
	if (editMode == eEditModeMove || editMode == eEditModeRotate || editMode == eEditModeScale)
	{
		SetAxisConstrains( m_lastAxis[editMode] );
		SetReferenceCoordSys( m_lastCoordSys[editMode] );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetEditTool( CEditTool *tool )
{
	if (m_editTool != tool && m_editTool != 0)
	{
		m_editTool->EndEditParams();
		m_editTool->Release();
		SetStatusText( "Ready" );
	}
	if (tool != 0 && m_editTool == 0)
	{
		// Tool set.
		m_currEditMode = eEditModeTool;
	}
	if (tool == 0 && m_editTool != 0)
	{
		// Tool set.
		m_currEditMode = m_prevEditMode;
	}
	m_editTool = tool;
	if (m_editTool)
	{
		m_editTool->BeginEditParams( this,0 );
	}

	// Make sure pick is aborted.
	if (tool != m_pickTool)
	{
		m_pickTool = 0;
	}
}
	
//////////////////////////////////////////////////////////////////////////
CEditTool* CEditorImpl::GetEditTool()
{
	return m_editTool;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetAxisConstrains( AxisConstrains axisFlags )
{
	m_selectedAxis = axisFlags;
	if (!m_editTool)
		m_lastAxis[m_currEditMode] = m_selectedAxis;
	m_viewMan->SetAxisConstrain( axisFlags );

	// Update all views.
	UpdateViews( eUpdateObjects,NULL );
}

//////////////////////////////////////////////////////////////////////////
AxisConstrains CEditorImpl::GetAxisConstrains()
{
	return m_selectedAxis;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetTerrainAxisIgnoreObjects( bool bIgnore )
{
	m_bTerrainAxisIgnoreObjects = bIgnore;
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::IsTerrainAxisIgnoreObjects()
{
	return m_bTerrainAxisIgnoreObjects;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetReferenceCoordSys( RefCoordSys refCoords )
{
	m_refCoordsSys = refCoords;
	if (!m_editTool)
		m_lastCoordSys[m_currEditMode] = m_refCoordsSys;

	// Update all views.
	UpdateViews( eUpdateObjects,NULL );
}

//////////////////////////////////////////////////////////////////////////
RefCoordSys CEditorImpl::GetReferenceCoordSys()
{
	return m_refCoordsSys;
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CEditorImpl::NewObject( const CString &typeName,const CString &file )
{
	SetModifiedFlag();
	return GetObjectManager()->NewObject( typeName,0,file );
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::DeleteObject( CBaseObject *obj )
{
	SetModifiedFlag();
	GetObjectManager()->DeleteObject( obj );
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CEditorImpl::CloneObject( CBaseObject *obj )
{
	SetModifiedFlag();
	return GetObjectManager()->CloneObject( obj );
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::StartObjectCreation( const CString &type,const CString &file )
{
	CObjectCreateTool *pTool = new CObjectCreateTool;
	GetIEditor()->SetEditTool( pTool );
	pTool->StartCreation( type,file );
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CEditorImpl::GetSelectedObject()
{
	CBaseObject *obj = 0;
	if (m_objectMan->GetSelection()->GetCount() != 1)
		return 0;
	return m_objectMan->GetSelection()->GetObject(0);
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SelectObject( CBaseObject *obj )
{
	GetObjectManager()->SelectObject( obj );
}

//////////////////////////////////////////////////////////////////////////
IObjectManager* CEditorImpl::GetObjectManager()
{
	return m_objectMan;
};

//////////////////////////////////////////////////////////////////////////
CSelectionGroup* CEditorImpl::GetSelection()
{
	return m_objectMan->GetSelection();
}

//////////////////////////////////////////////////////////////////////////
int CEditorImpl::ClearSelection()
{
	return m_objectMan->ClearSelection();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::LockSelection( bool bLock )
{
	// Selection must be not empty to enable selection lock.
	if (!GetSelection()->IsEmpty())
		m_bSelectionLocked = bLock;
	else
		m_bSelectionLocked = false;
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::IsSelectionLocked()
{
	return m_bSelectionLocked;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::PickObject( IPickObjectCallback *callback,CRuntimeClass *targetClass,const char *statusText,bool bMultipick )
{
	m_pickTool = new CPickObjectTool(callback,targetClass);
	((CPickObjectTool*)m_pickTool)->SetMultiplePicks( bMultipick );
	if (statusText)
		m_pickTool->SetStatusText(statusText);

	SetEditTool( m_pickTool );
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::CancelPick()
{
	SetEditTool(0);
	m_pickTool = 0;
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::IsPicking()
{
	if (GetEditTool() == m_pickTool && m_pickTool != 0)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
CViewManager* CEditorImpl::GetViewManager()
{
	return m_viewMan;
}

CViewport* CEditorImpl::GetActiveView()
{
	CViewPane* viewPane = (CViewPane*) ((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
	if (viewPane) {
		return viewPane->GetViewport();
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::UpdateViews( int flags,BBox *updateRegion )
{
	BBox prevRegion = m_viewMan->GetUpdateRegion();
	if (updateRegion)
		m_viewMan->SetUpdateRegion( *updateRegion );
	m_viewMan->UpdateViews( flags );
	if (updateRegion)
		m_viewMan->SetUpdateRegion( prevRegion );
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::UpdateTrackView( bool bOnlyKeys )
{
	CTrackViewDialog *trackDlg = ((CMainFrame*)AfxGetMainWnd())->GetTrackView();
	if (trackDlg)
	{
		if (bOnlyKeys)
			trackDlg->InvalidateTrackList();
		else
			trackDlg->ReloadSequences();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::ResetViews()
{
	m_viewMan->ResetViews();

	m_displaySettings->SetRenderFlags( m_displaySettings->GetRenderFlags() );
}
/*//////////////////////////////////////////////////////////////////////////
void	CEditorImpl::MoveViewer( const Vec3d &dir )
{
	if (GetIEditor()->GetDisplaySettings()->GetSettings() & SETTINGS_NOCOLLISION)
	{
		SetViewerPos(m_viewerPos+dir);
		return;
	}
	pe_action_move hike;
	hike.iJump=0;
	hike.dir=dir;
	IGame *game = GetGame();
	if (game)
	{
		IEntity *myPlayer = game->GetMyPlayer();
		if (myPlayer)
		{
			IPhysicalEntity *pPhysEnt=myPlayer->GetPhysics();
			if (pPhysEnt)
			{
				pPhysEnt->Action(&hike);
				// turn on flying to avoid the camera to fall while moving...
				ICVar *pCVar=GetSystem()->GetIConsole()->GetCVar("p_fly_mode");
				int nOldFlymode=pCVar->GetIVal();
				// restore old flymode...
				if (!nOldFlymode)
					pCVar->Set(1);
				pPhysEnt->Step(1.0f);
				if (!nOldFlymode)
					pCVar->Set(0);
				Vec3d pos=myPlayer->GetPos();
				m_viewerPos = pos;
				GetSystem()->GetViewCamera().SetPos( pos );
			}
		}
	}
}*/
//////////////////////////////////////////////////////////////////////////
void	CEditorImpl::SetViewerPos( const Vec3d &pos )
{
	m_viewerPos = pos;
	GetSystem()->GetViewCamera().SetPos( pos );
	
	if (m_gameEngine)
		m_gameEngine->SetPlayerPos( pos );
	SetModifiedFlag();
}
//////////////////////////////////////////////////////////////////////////
void	CEditorImpl::SetViewerAngles( const Vec3d &angles )
{
	m_viewerAngles = angles;
	if (m_viewerAngles.y!=0.0f)
	{
		CLogFile::WriteLine("Weird camera-angles ! Correcting...");
		m_viewerAngles.y=0.0f;
	}
	GetSystem()->GetViewCamera().SetAngle( angles );
	
	if (m_gameEngine)
	{
		m_gameEngine->SetPlayerAngles( angles );
	}
	SetModifiedFlag();
}
//////////////////////////////////////////////////////////////////////////
Vec3d	CEditorImpl::GetViewerPos()
{
	return m_viewerPos;
}
//////////////////////////////////////////////////////////////////////////
Vec3d	CEditorImpl::GetViewerAngles()
{
	return m_viewerAngles;
}

//////////////////////////////////////////////////////////////////////////
float CEditorImpl::GetTerrainElevation( float x,float y )
{
	I3DEngine *engine = m_system->GetI3DEngine();
	if (!engine)
		return 0;
	return engine->GetTerrainElevation( x,y );
}

CHeightmap* CEditorImpl::GetHeightmap()
{
	return &GetDocument()->m_cHeightmap;
}

CVegetationMap* CEditorImpl::GetVegetationMap()
{
	assert( GetHeightmap() != 0 );
	return GetHeightmap()->GetVegetationMap();
}

void CEditorImpl::SetSelectedRegion( const BBox &box )
{
	m_selectedRegion = box;
}
	
void CEditorImpl::GetSelectedRegion( BBox &box )
{
	box = m_selectedRegion;
}

//////////////////////////////////////////////////////////////////////////
CBaseLibraryDialog* CEditorImpl::OpenDataBaseLibrary( EDataBaseLibraries dbLib,CBaseLibraryItem *pItem )
{
	((CMainFrame*)AfxGetMainWnd())->ShowDataBaseDialog( true );
	CDataBaseDialog *dlgDB = ((CMainFrame*)AfxGetMainWnd())->GetDataBaseDialog();
	dlgDB->Select( dbLib );
	CBaseLibraryDialog *dlg = dlgDB->GetCurrent();
	if (dlg && pItem)
	{
		if (dlg->CanSelectItem( pItem ))
		{
			dlg->SelectItem( pItem );
		}
	}
	return dlg;
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::SelectColor( COLORREF &color,CWnd *parent )
{
	COLORREF col = color;
	CCustomColorDialog dlg( col,CC_FULLOPEN,parent );
	if (dlg.DoModal() == IDOK)
	{
		color = dlg.GetColor();
		return true;
	}
	return false;
}

void CEditorImpl::SetInGameMode( bool inGame )
{
	if (m_gameEngine)
		m_gameEngine->SetGameMode(inGame);
}

bool CEditorImpl::IsInGameMode()
{
	if (m_gameEngine)
		return m_gameEngine->IsInGameMode();
	return false;
}

bool CEditorImpl::IsInTestMode()
{
	return ((CCryEditApp*)AfxGetApp())->IsInTestMode();
}

bool CEditorImpl::IsInPreviewMode()
{
	return ((CCryEditApp*)AfxGetApp())->IsInPreviewMode();
}

void CEditorImpl::EnableAcceleratos( bool bEnable )
{
	((CMainFrame*)AfxGetMainWnd())->EnableAccelerator( bEnable );
	//((CCryEditApp*)AfxGetApp())->EnableAccelerator( bEnable );
}

void CEditorImpl::DetectVersion()
{
	char exe[_MAX_PATH];
	DWORD dwHandle;
	UINT len;
	
	char ver[1024*8];
	
	GetModuleFileName( NULL, exe, _MAX_PATH );
	
	int verSize = GetFileVersionInfoSize( exe,&dwHandle );
	if (verSize > 0)
	{
		GetFileVersionInfo( exe,dwHandle,1024*8,ver );
		VS_FIXEDFILEINFO *vinfo;
		VerQueryValue( ver,"\\",(void**)&vinfo,&len );
		
		m_fileVersion.v[0] = vinfo->dwFileVersionLS & 0xFFFF;
		m_fileVersion.v[1] = vinfo->dwFileVersionLS >> 16;
		m_fileVersion.v[2] = vinfo->dwFileVersionMS & 0xFFFF;
		m_fileVersion.v[3] = vinfo->dwFileVersionMS >> 16;
		
		m_productVersion.v[0] = vinfo->dwProductVersionLS & 0xFFFF;
		m_productVersion.v[1] = vinfo->dwProductVersionLS >> 16;
		m_productVersion.v[2] = vinfo->dwProductVersionMS & 0xFFFF;
		m_productVersion.v[3] = vinfo->dwProductVersionMS >> 16;
		
		//debug( "<kernel> FileVersion: %d.%d.%d.%d",s_fileVersion.v[3],s_fileVersion.v[2],s_fileVersion.v[1],s_fileVersion.v[0] );
		//debug( "<kernel> ProductVersion: %d.%d.%d.%d",s_productVersion.v[3],s_productVersion.v[2],s_productVersion.v[1],s_productVersion.v[0] );
	}
}

XmlNodeRef CEditorImpl::FindTemplate( const CString &templateName )
{
	return m_templateRegistry.FindTemplate( templateName );
}
	
void CEditorImpl::AddTemplate( const CString &templateName,XmlNodeRef &tmpl )
{
	m_templateRegistry.AddTemplate( templateName,tmpl );
}

//////////////////////////////////////////////////////////////////////////
CShaderEnum* CEditorImpl::GetShaderEnum()
{
	return m_shaderEnum;
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::ExecuteConsoleApp( const CString &CommandLine, CString &OutputText )
{
	////////////////////////////////////////////////////////////////////////
	// Execute a console application and redirect its output to the
	// console window
	////////////////////////////////////////////////////////////////////////

	SECURITY_ATTRIBUTES sa = { 0 };
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	HANDLE hPipeOutputRead = NULL;
	HANDLE hPipeOutputWrite = NULL;
	HANDLE hPipeInputRead = NULL;
	HANDLE hPipeInputWrite = NULL;
	BOOL bTest = FALSE;
	bool bReturn = true;
	DWORD dwNumberOfBytesRead = 0;
	DWORD dwStartTime = 0;
	char szCharBuffer[65];
	char szOEMBuffer[65];

	CLogFile::FormatLine("Executing console application '%s'", (const char*)CommandLine);
	
	// Initialize the SECURITY_ATTRIBUTES structure
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	// Create a pipe for standard output redirection
	VERIFY(CreatePipe(&hPipeOutputRead, &hPipeOutputWrite, &sa, 0));

	// Create a pipe for standard inout redirection
	VERIFY(CreatePipe(&hPipeInputRead, &hPipeInputWrite, &sa, 0));

	// Make a child process useing hPipeOutputWrite as standard out. Also
	// make sure it is not shown on the screen
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdInput = hPipeInputRead;
	si.hStdOutput = hPipeOutputWrite;
	si.hStdError = hPipeOutputWrite;

	// Save the process start time
	dwStartTime = GetTickCount();

	// Launch the console application
	char cmdLine[1024];
	strcpy( cmdLine,CommandLine );
	if (!CreateProcess(NULL, cmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
		return false;

	// If no process was spawned
	if (!pi.hProcess)
		bReturn = false;

	// Now that the handles have been inherited, close them
	CloseHandle(hPipeOutputWrite);
	CloseHandle(hPipeInputRead);

	// Capture the output of the console application by reading from hPipeOutputRead
	while (true)
	{
		// Read from the pipe
		bTest = ReadFile(hPipeOutputRead, &szOEMBuffer, 64, &dwNumberOfBytesRead, NULL);

		// Break when finished
		if (!bTest)
			break;

		// Break when timeout has been exceeded
		if (GetTickCount() - dwStartTime > 5000)
			break;

		// Null terminate string
		szOEMBuffer[dwNumberOfBytesRead] = '\0';

		// Translate into ANSI
		VERIFY(OemToChar(szOEMBuffer, szCharBuffer));

		// Add it to the output text
		OutputText += szCharBuffer;
	}

	// Wait for the process to finish
	WaitForSingleObject(pi.hProcess, 1000);

	return bReturn;
}

//////////////////////////////////////////////////////////////////////////
// Undo
//////////////////////////////////////////////////////////////////////////
void CEditorImpl::BeginUndo()
{
	if (m_undoManager)
		m_undoManager->Begin();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::RestoreUndo( bool undo )
{
	if (m_undoManager)
		m_undoManager->Restore(undo);
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::AcceptUndo( const CString &name )
{
	if (m_undoManager)
		m_undoManager->Accept(name);
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::CancelUndo()
{
	if (m_undoManager)
		m_undoManager->Cancel();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SuperBeginUndo()
{
	if (m_undoManager)
		m_undoManager->SuperBegin();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SuperAcceptUndo( const CString &name )
{
	if (m_undoManager)
		m_undoManager->SuperAccept(name);
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SuperCancelUndo()
{
	if (m_undoManager)
		m_undoManager->SuperCancel();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SuspendUndo()
{
	if (m_undoManager)
		m_undoManager->Suspend();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::ResumeUndo()
{
	if (m_undoManager)
		m_undoManager->Resume();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::Undo()
{
	if (m_undoManager)
		m_undoManager->Undo();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::Redo()
{
	if (m_undoManager)
		m_undoManager->Redo();
}

//////////////////////////////////////////////////////////////////////////
bool CEditorImpl::IsUndoRecording()
{
	if (m_undoManager)
		return m_undoManager->IsUndoRecording();
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::RecordUndo( IUndoObject *obj )
{
	if (m_undoManager)
		m_undoManager->RecordUndo( obj );
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::FlushUndo()
{
	if (m_undoManager)
		m_undoManager->Flush();
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::SetConsoleVar( const char *var,float value )
{
	ICVar *ivar = GetSystem()->GetIConsole()->GetCVar( var );
	if (ivar)
		ivar->Set( value );
}

//////////////////////////////////////////////////////////////////////////
float CEditorImpl::GetConsoleVar( const char *var )
{
	ICVar *ivar = GetSystem()->GetIConsole()->GetCVar( var );
	if (ivar)
	{
		return ivar->GetFVal();
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CAIManager*	CEditorImpl::GetAI()
{
	return m_AI;
}

//////////////////////////////////////////////////////////////////////////
CAnimationContext* CEditorImpl::GetAnimation()
{
	return m_animationContext;
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::RegisterDocListener( IDocListener *listener )
{
	CCryEditDoc *doc = GetDocument();
	if (doc)
	{
		doc->RegisterListener( listener );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEditorImpl::UnregisterDocListener( IDocListener *listener )
{
	CCryEditDoc *doc = GetDocument();
	if (doc)
	{
		doc->UnregisterListener( listener );
	}
}
