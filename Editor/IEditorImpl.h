////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   IEditorImpl.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: IEditor interface implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IEditorImpl_h__
#define __IEditorImpl_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IEditor.h"
#include "MainFrm.h"

#define GET_PLUGIN_ID_FROM_MENU_ID(ID) (((ID) & 0x000000FF))
#define GET_UI_ELEMENT_ID_FROM_MENU_ID(ID) ((((ID) & 0x0000FF00) >> 8))

class CObjectManager;
class CUndoManager;
class CGameEngine;
class CEquipPackLib;
class CSoundPresetMgr;
class CEAXPresetMgr;

/*!
 *  CEditorImpl implements IEditor interface.	
 */
class CEditorImpl : public IEditor
{
public:
	//////////////////////////////////////////////////////////////////////////
	CEditorImpl();
	virtual ~CEditorImpl();

	void SetGameEngine( CGameEngine *ge );

	void DeleteThis() { delete this; };

	CClassFactory* GetClassFactory();
	CCommandManager* GetCommandManager() { return m_pCommandManager; };

	void SetDocument( CCryEditDoc *pDoc );
	//! Get active document
	CCryEditDoc* GetDocument();
	void SetModifiedFlag( bool modified = true );
	bool IsModified();
	bool SaveDocument();

	//////////////////////////////////////////////////////////////////////////
	// Generic
	//////////////////////////////////////////////////////////////////////////
	ISystem*	GetSystem();
	IGame*		GetGame();
	I3DEngine*	Get3DEngine();
	IRenderer*	GetRenderer();


	void WriteToConsole(const char * pszString) { CLogFile::WriteLine(pszString); };

	// Change the message in the status bar
	void SetStatusText(const char * pszString) 
	{
		if (AfxGetMainWnd())
			((CMainFrame *) (AfxGetMainWnd()))->SetStatusText(pszString);
	};

	bool ShowConsole( bool show ) {
		if (AfxGetMainWnd())
			return ((CMainFrame *) (AfxGetMainWnd()))->ShowConsole(show);
		return false;
	}
	
	void SetConsoleVar( const char *var,float value );
	float GetConsoleVar( const char *var );

	// Query main window of the editor
	HWND GetEditorMainWnd()
	{
		if (AfxGetMainWnd())
			return AfxGetMainWnd()->m_hWnd;
		return 0;
	};

	//////////////////////////////////////////////////////////////////////////
	// Paths.
	//////////////////////////////////////////////////////////////////////////
	// Returns the path of the editors Master CD folder
	const char * GetMasterCDFolder();
	CString GetRelativePath( const CString &fullPath );
	CString GetLevelFolder();
	//////////////////////////////////////////////////////////////////////////

	bool ExecuteConsoleApp( const CString &CommandLine, CString &OutputText );

	//! Check if editor running in gaming mode.
	bool IsInGameMode();
	//! Set game mode of editor.
	void SetInGameMode( bool inGame );

	bool IsInTestMode();

	bool IsInPreviewMode();

	//! Enables/Disable updates of editor.
	virtual void EnableUpdate( bool enable ) { m_bUpdates = enable; };

	//! Enable/Disable accelerator table, (Enabled by default).
	virtual void EnableAcceleratos( bool bEnable );

	//////////////////////////////////////////////////////////////////////////
	// Game Engine.
	//////////////////////////////////////////////////////////////////////////
	/** Retrieve pointer to game engine instance.
	*/
	CGameEngine* GetGameEngine() { return m_gameEngine; };

	//////////////////////////////////////////////////////////////////////////
	//! Display Settings.
	//////////////////////////////////////////////////////////////////////////
	CDisplaySettings*	GetDisplaySettings() { return m_displaySettings; };
	
	//////////////////////////////////////////////////////////////////////////
	// Objects management.
	//////////////////////////////////////////////////////////////////////////
	CBaseObject* NewObject( const CString &typeName,const CString &file="" );
  void DeleteObject( CBaseObject *obj );
	CBaseObject* CloneObject( CBaseObject *obj );
	void StartObjectCreation( const CString &type,const CString &file="" );

	IObjectManager* GetObjectManager();
	CSelectionGroup*	GetSelection();
	int ClearSelection();
	CBaseObject* GetSelectedObject();
	void SelectObject( CBaseObject *obj );

	void LockSelection( bool bLock );
	bool IsSelectionLocked();

	void PickObject( IPickObjectCallback *callback,CRuntimeClass *targetClass=0,const char *statusText=0,bool bMultipick=false );
	void CancelPick();
	bool IsPicking();

	//////////////////////////////////////////////////////////////////////////
	// Get DataBase Managers.
	//////////////////////////////////////////////////////////////////////////
	virtual CEntityPrototypeManager* GetEntityProtManager() { return m_entityManager; };
	virtual CMaterialManager* GetMaterialManager() { return m_materialManager; };
	virtual CParticleManager* GetParticleManager() { return m_particleManager; };
	virtual CMusicManager* GetMusicManager() { return m_pMusicManager; };
	virtual CPrefabManager* GetPrefabManager() { return m_pPrefabManager; };

	//////////////////////////////////////////////////////////////////////////
	// Icon manager.
	//////////////////////////////////////////////////////////////////////////
	CIconManager* GetIconManager() { return m_iconManager; };

	//////////////////////////////////////////////////////////////////////////
	// Terrain related.
	//////////////////////////////////////////////////////////////////////////
	float GetTerrainElevation( float x,float y );
	CHeightmap* GetHeightmap();
	CVegetationMap* GetVegetationMap();

	//////////////////////////////////////////////////////////////////////////
	// AI
	//////////////////////////////////////////////////////////////////////////
	CAIManager*	GetAI();

	//////////////////////////////////////////////////////////////////////////
	// Access to IMovieSystem.
	//////////////////////////////////////////////////////////////////////////
	IMovieSystem* GetMovieSystem() {
		if (m_system)
			return m_system->GetIMovieSystem();
		return NULL; };

	//////////////////////////////////////////////////////////////////////////
	// EquipPackLib
	//////////////////////////////////////////////////////////////////////////
	CEquipPackLib* GetEquipPackLib() { return m_pEquipPackLib; }

	//////////////////////////////////////////////////////////////////////////
	// Plugins
	//////////////////////////////////////////////////////////////////////////
	CPluginManager* GetPluginManager() { return m_pluginMan; };

	//////////////////////////////////////////////////////////////////////////
	// Sound/EAX-Presets
	//////////////////////////////////////////////////////////////////////////
	CSoundPresetMgr* GetSoundPresetMgr() { return m_pSoundPresetMgr; }
	CEAXPresetMgr* GetEAXPresetMgr() { return m_pEAXPresetMgr; }

	//////////////////////////////////////////////////////////////////////////
	// Views related methods.
	//////////////////////////////////////////////////////////////////////////
	CViewManager* GetViewManager();

	CViewport* GetActiveView();

	void UpdateViews( int flags,BBox *updateRegion );
	void ResetViews();

	void UpdateTrackView( bool bOnlyKeys=false );

	// Current position marker
	Vec3d GetMarkerPosition() { return m_marker; };
	void SetMarkerPosition( const Vec3d &pos ) { m_marker = pos; };

	void	SetSelectedRegion( const BBox &box );
	void	GetSelectedRegion( BBox &box );

//	void	MoveViewer( const Vec3d &dir );
	void	SetViewerPos( const Vec3d &pos );
	void	SetViewerAngles( const Vec3d &angles );
	Vec3d	GetViewerPos();
	Vec3d	GetViewerAngles();

	//////////////////////////////////////////////////////////////////////////
	// UI Interface
	//////////////////////////////////////////////////////////////////////////
	bool AddMenuItem(uint8 iId, bool bIsSeparator, eMenuInsertLocation eParent, IUIEvent *pIHandler);
	bool AddToolbarItem(uint8 iId, IUIEvent *pIHandler);

	bool CreateRootMenuItem(const char *pszName);

	// Serializatation state
	void SetDataModified();

	// Name of the current editor document
	const char * GetEditorDocumentName();

	// Roll up bar interface	
	
	int SelectRollUpBar( int rollupBarId );

	// Insert a new dialog page into the roll up bar
	int AddRollUpPage(int rollbarId,LPCTSTR pszCaption, class CDialog *pwndTemplate, 
		bool bAutoDestroyTpl = true, int iIndex = -1);

	// Remove a dialog page from the roll up bar
	void RemoveRollUpPage(int rollbarId,int iIndex);

	// Expand one of the rollup pages
	void ExpandRollUpPage(int rollbarId,int iIndex, BOOL bExpand = true);

	// Enable or disable one of the rollup pages
	void EnableRollUpPage(int rollbarId,int iIndex, BOOL bEnable = true);
	
	// Get the window handle of the roll up page container. All CDialog classes
	// which are passed to InsertRollUpPage() need to have this handle as
	// the parent window
	HWND GetRollUpContainerWnd(int rollbarId);

	//////////////////////////////////////////////////////////////////////////
	// Editing modes.
	//////////////////////////////////////////////////////////////////////////
	void SetEditMode( int editMode );
	int GetEditMode();

	//////////////////////////////////////////////////////////////////////////
	// Edit tools.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetEditTool( CEditTool *tool );
	//! Returns current edit tool.
	virtual CEditTool* GetEditTool();

	//////////////////////////////////////////////////////////////////////////
	// Transformation methods.
	//////////////////////////////////////////////////////////////////////////
	void SetAxisConstrains( AxisConstrains axis );
	AxisConstrains GetAxisConstrains();

	void SetTerrainAxisIgnoreObjects( bool bIgnore );
	bool IsTerrainAxisIgnoreObjects();

	void SetReferenceCoordSys( RefCoordSys refCoords );
	RefCoordSys GetReferenceCoordSys();

	//////////////////////////////////////////////////////////////////////////
	// XmlTemplates
	//////////////////////////////////////////////////////////////////////////
	XmlNodeRef FindTemplate( const CString &templateName );
	void AddTemplate( const CString &templateName,XmlNodeRef &tmpl );

	//////////////////////////////////////////////////////////////////////////
	// Standart Dialogs.
	//////////////////////////////////////////////////////////////////////////
	CBaseLibraryDialog* OpenDataBaseLibrary( EDataBaseLibraries dbLib,CBaseLibraryItem *pItem=NULL );
	bool SelectColor( COLORREF &color,CWnd *parent=0 );

	// Own methods.
	void Update();

	Version GetFileVersion() { return m_fileVersion; };
	Version GetProductVersion() { return m_productVersion; };

	//////////////////////////////////////////////////////////////////////////
	// Installed Shaders enumerations.
	//////////////////////////////////////////////////////////////////////////
	//! Get shader enumerator.
	virtual CShaderEnum* GetShaderEnum();

	//////////////////////////////////////////////////////////////////////////
	// Undo
	//////////////////////////////////////////////////////////////////////////
	virtual CUndoManager* GetUndoManager() { return m_undoManager; };
	//! Begin operataion requiering Undo.
	virtual void BeginUndo();
	virtual void RestoreUndo( bool undo );
	virtual void AcceptUndo( const CString &name );
	virtual void CancelUndo();
	virtual void SuperBeginUndo();
	virtual void SuperAcceptUndo( const CString &name );
	virtual void SuperCancelUndo();
	virtual void SuspendUndo();
	virtual void ResumeUndo();
	virtual void Undo();
	virtual void Redo();
	virtual bool IsUndoRecording();
	virtual void RecordUndo( IUndoObject *obj );
	virtual void FlushUndo();

	//////////////////////////////////////////////////////////////////////////
	// Animation related.
	//////////////////////////////////////////////////////////////////////////
	//! Retrieve current animation context.
	CAnimationContext* GetAnimation();

	CExternalToolsManager* GetExternalToolsManager() { return m_externalToolsManager; };

	virtual CErrorReport* GetErrorReport() { return m_pErrorReport; }

	//////////////////////////////////////////////////////////////////////////
	// Listeners.
	//////////////////////////////////////////////////////////////////////////
	//! Register document notifications listener.
	void RegisterDocListener( IDocListener *listener );
	//! Unregister document notifications listener.
	void UnregisterDocListener( IDocListener *listener );
		
protected:
	///int FindMenuItem(CMenu *pMenu, LPCTSTR pszMenuString);
	void DetectVersion();
	void RegisterTools();
	void SetMasterCDFolder();

	CRollupCtrl *GetRollUpControl( int rollupId )
	{
		CMainFrame *wnd = (CMainFrame*)AfxGetMainWnd();
		if (wnd)
			return wnd->GetRollUpControl(rollupId);
		return 0;
	};
	
	eEditMode m_currEditMode;
	eEditMode m_prevEditMode;

	ISystem *m_system;

	CClassFactory* m_classFactory;

	//! Command manager.
	CCommandManager* m_pCommandManager;

	//! Manager of objects.
	CObjectManager* m_objectMan;
	
	//! Manager of plugins.
	CPluginManager* m_pluginMan;

	//! Manager of viewport.
	CViewManager*	m_viewMan;

	CUndoManager* m_undoManager;

	CSoundPresetMgr *m_pSoundPresetMgr;

	CEAXPresetMgr *m_pEAXPresetMgr;

	//! Current position marker.
	Vec3d m_marker;

	//! Currently selected region.
	BBox m_selectedRegion;

	//! Selected axis flags.
	AxisConstrains m_selectedAxis;
	RefCoordSys m_refCoordsSys;

	// Axis constrains for all edit modes.
	AxisConstrains m_lastAxis[16];
	RefCoordSys m_lastCoordSys[16];

	bool m_bUpdates;
	bool m_bTerrainAxisIgnoreObjects;

	Vec3d m_viewerPos;
	Vec3d m_viewerAngles;

	Version m_fileVersion;
	Version m_productVersion;

	CXmlTemplateRegistry m_templateRegistry;

	//! Current display settins.
	CDisplaySettings* m_displaySettings;
	
	//! Current shader enumerator.
	CShaderEnum *m_shaderEnum;

	//! Currently enabled Edit Tool.
	CEditTool* m_editTool;

	class CIconManager *m_iconManager;

	CString m_masterCDFolder;

	//! True when selection is locked.
	bool m_bSelectionLocked;

	CEditTool *m_pickTool;

	CAIManager *m_AI;

	CEquipPackLib *m_pEquipPackLib;

//	IMovieSystem* m_movieSystem;

	//! Pointer to game engine class.
	CGameEngine *m_gameEngine;

	//! Animation context.
	CAnimationContext* m_animationContext;

	//! External tools manager.
	CExternalToolsManager* m_externalToolsManager;

	//! Entity prototype manager.
	CEntityPrototypeManager* m_entityManager;

	//! Materials Manager.
	CMaterialManager* m_materialManager;

	//! Particles manager.
	CParticleManager* m_particleManager;

	//! Music Manager.
	CMusicManager* m_pMusicManager;

	//! Prefabs Manager.
	CPrefabManager* m_pPrefabManager;

	//! Global instance of error report class.
	CErrorReport *m_pErrorReport;
};

#endif // __IEditorImpl_h__