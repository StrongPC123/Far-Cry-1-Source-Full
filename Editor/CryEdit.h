// CryEdit.h : main header file for the CRYEDIT application
//

#if !defined(AFX_CRYEDIT_H__41D56446_54D7_49B2_8EF6_884EA7A42365__INCLUDED_)
#define AFX_CRYEDIT_H__41D56446_54D7_49B2_8EF6_884EA7A42365__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "Util\FileChangeMonitor.h"


class CCryEditDoc;
/////////////////////////////////////////////////////////////////////////////
// CCryEditApp:
// See CryEdit.cpp for the implementation of this class
//

class CCryEditApp : public CWinApp
{
public:
	CRecentFileList * GetRecentFileList() { return m_pRecentFileList; };

	CCryEditApp();
	~CCryEditApp();
	
	void	LoadFile( const CString &fileName );

	bool IsInTestMode() { return m_bTestMode; };
	bool IsInPreviewMode() { return m_bPreviewMode; };
	void EnableAccelerator( bool bEnable );
	void SaveAutoBackup();
	void SaveAutoRemind();

public:
	IEditor* m_IEditor;

	void InitDirectory();
	BOOL FirstInstance();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCryEditApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CCryEditApp)
	afx_msg void OnCreateLevel();
	afx_msg void OnOpenLevel();
	afx_msg void OnAppAbout();
	afx_msg void ToolTerrain();
	afx_msg void ToolSky();
	afx_msg void ToolLighting();
	afx_msg void ToolTexture();
	afx_msg void ExportToGame();
	afx_msg void OnEditHold();
	afx_msg void OnEditFetch();
	afx_msg void OnCancelMode();
	afx_msg void OnGeneratorsStaticobjects();
	afx_msg void OnFileCreateopenlevel();
	afx_msg void OnFileExportToGameNoSurfaceTexture();
	afx_msg void OnEditInsertObject();
	afx_msg void OnViewSwitchToGame();
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditSelectNone();
	afx_msg void OnEditDelete();
	afx_msg void OnMoveObject();
	afx_msg void OnSelectObject();
	afx_msg void OnRenameObj();
	afx_msg void OnSetHeight();
	afx_msg void OnScriptCompileScript();
	afx_msg void OnScriptEditScript();
	afx_msg void OnEditmodeMove();
	afx_msg void OnEditmodeRotate();
	afx_msg void OnEditmodeScale();
	afx_msg void OnEditToolLink();
	afx_msg void OnUpdateEditToolLink(CCmdUI* pCmdUI);
	afx_msg void OnEditToolUnlink();
	afx_msg void OnUpdateEditToolUnlink(CCmdUI* pCmdUI);
	afx_msg void OnEditmodeSelect();
	afx_msg void OnSelectionDelete();
	afx_msg void OnEditEscape();
	afx_msg void OnObjectSetArea();
	afx_msg void OnObjectSetHeight();
	afx_msg void OnUpdateEditmodeSelect(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditmodeMove(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditmodeRotate(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditmodeScale(CCmdUI* pCmdUI);
	afx_msg void OnObjectmodifyFreeze();
	afx_msg void OnObjectmodifyUnfreeze();
	afx_msg void OnEditmodeSelectarea();
	afx_msg void OnUpdateEditmodeSelectarea(CCmdUI* pCmdUI);
	afx_msg void OnSelectAxisX();
	afx_msg void OnSelectAxisY();
	afx_msg void OnSelectAxisZ();
	afx_msg void OnSelectAxisXy();
	afx_msg void OnUpdateSelectAxisX(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectAxisXy(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectAxisY(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectAxisZ(CCmdUI* pCmdUI);
	afx_msg void OnUndo();
	afx_msg void OnEditClone();
	afx_msg void OnExportTerrainGeom();
	afx_msg void OnUpdateExportTerrainGeom(CCmdUI* pCmdUI);
	afx_msg void OnSelectionSave();
	afx_msg void OnSelectionLoad();
	afx_msg void OnGotoSelected();
	afx_msg void OnUpdateSelected(CCmdUI* pCmdUI);
	afx_msg void OnAlignObject();
	afx_msg void OnAlignToGrid();
	afx_msg void OnUpdateAlignObject(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFreezed(CCmdUI* pCmdUI);
	afx_msg void OnGroupAttach();
	afx_msg void OnUpdateGroupAttach(CCmdUI* pCmdUI);
	afx_msg void OnGroupClose();
	afx_msg void OnUpdateGroupClose(CCmdUI* pCmdUI);
	afx_msg void OnGroupDetach();
	afx_msg void OnUpdateGroupDetach(CCmdUI* pCmdUI);
	afx_msg void OnGroupMake();
	afx_msg void OnUpdateGroupMake(CCmdUI* pCmdUI);
	afx_msg void OnGroupOpen();
	afx_msg void OnUpdateGroupOpen(CCmdUI* pCmdUI);
	afx_msg void OnGroupUngroup();
	afx_msg void OnUpdateGroupUngroup(CCmdUI* pCmdUI);
	afx_msg void OnMissionNew();
	afx_msg void OnMissionDelete();
	afx_msg void OnMissionDuplicate();
	afx_msg void OnMissionProperties();
	afx_msg void OnMissionRename();
	afx_msg void OnMissionSelect();
	afx_msg void OnMissionReload();
	afx_msg void OnMissionEdit();
	afx_msg void OnShowTips();
	afx_msg void OnLockSelection();
	afx_msg void OnEditLevelData();
	afx_msg void OnFileEditLogFile();
	afx_msg void OnFileEditEditorini();
	afx_msg void OnSelectAxisTerrain();
	afx_msg void OnSelectAxisSnapToAll();
	afx_msg void OnUpdateSelectAxisTerrain(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSelectAxisSnapToAll(CCmdUI* pCmdUI);
	afx_msg void OnPreferences();
	afx_msg void OnReloadTextures();
	afx_msg void OnReloadScripts();
	afx_msg void OnReloadGeometry();
	afx_msg void OnReloadTerrain();
	afx_msg void OnRedo();
	afx_msg void OnUpdateRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
	afx_msg void OnLayerSelect();
	afx_msg void OnTerrainCollision();
	afx_msg void OnTerrainCollisionUpdate( CCmdUI *pCmdUI );
	afx_msg void OnGenerateCgfThumbnails();
	afx_msg void OnAiGenerateTriangulation();
	afx_msg void OnSwitchPhysics();
	afx_msg void OnSwitchPhysicsUpdate( CCmdUI *pCmdUI );
	afx_msg void OnSyncPlayer();
	afx_msg void OnSyncPlayerUpdate( CCmdUI *pCmdUI );
	afx_msg void OnRefCoordsSys();
	afx_msg void OnUpdateRefCoordsSys(CCmdUI *pCmdUI);
	afx_msg void OnResourcesReduceworkingset();
	afx_msg void OnToolsGeneratelightmaps();
	afx_msg void OnToolsEquipPacksEdit();
	afx_msg void OnDummyCommand() {};
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	//! Initialize accelerator manager
	void InitAccelManager();

	void ReadConfig();
	void WriteConfig();
	
	void TagLocation( int index );
	void GotoTagLocation( int index );
	void LoadTagLocations();
	void MonitorDirectories();
	void ExportLevel( bool bExportToGame,bool bExportTexture,bool bExportLM );

	CCryEditDoc* GetDocument();
	class CSelectObjectDlg *m_selectObjDialog;

	CFileChangeMonitor *m_pFileChangeMonitor;

	//! True if editor is in test mode.
	//! Test mode is a special mode enabled when Editor ran with /test command line.
	//! In this mode editor starts up, but exit immidiatly after all initialzation.
	bool m_bTestMode;

	//! In this mode editor will load specified cry file, export t, and then close.
	bool m_bExportMode;
	CString m_exportFile;

	//! If application exiting.
	bool m_bExiting;

	//! True if editor is in preview mode.
	//! In this mode only very limited functionality is available and only for fast preview of models.
	bool m_bPreviewMode;

	//! Current file in preview mode.
	char m_sPreviewFile[_MAX_PATH];

	//! This variable rised when autosave must be done on next application update cycle.
	bool m_bSaveAutobackup;

	Vec3 m_tagLocations[12];
	Vec3 m_tagAngles[12];
	float m_fastRotateAngle;
	float m_moveSpeedStep;
	CString m_aviFilename;

private:
	afx_msg void OnEditHide();
	afx_msg void OnUpdateEditHide(CCmdUI *pCmdUI);
	afx_msg void OnEditUnhideall();
	afx_msg void OnEditFreeze();
	afx_msg void OnUpdateEditFreeze(CCmdUI *pCmdUI);
	afx_msg void OnEditUnfreezeall();

	afx_msg void OnSnap();
	afx_msg void OnUpdateEditmodeSnap(CCmdUI* pCmdUI);

	afx_msg void OnWireframe();
	afx_msg void OnUpdateWireframe(CCmdUI *pCmdUI);
	afx_msg void OnViewGridsettings();
	afx_msg void OnViewConfigureLayout();

	afx_msg void OnFileMonitorChange( WPARAM wParam, LPARAM lParam );

	//////////////////////////////////////////////////////////////////////////
	// Tag Locations.
	afx_msg void OnTagLocation1();
	afx_msg void OnTagLocation2();
	afx_msg void OnTagLocation3();
	afx_msg void OnTagLocation4();
	afx_msg void OnTagLocation5();
	afx_msg void OnTagLocation6();
	afx_msg void OnTagLocation7();
	afx_msg void OnTagLocation8();
	afx_msg void OnTagLocation9();
	afx_msg void OnTagLocation10();
	afx_msg void OnTagLocation11();
	afx_msg void OnTagLocation12();
	//////////////////////////////////////////////////////////////////////////
	afx_msg void OnGotoLocation1();
	afx_msg void OnGotoLocation2();
	afx_msg void OnGotoLocation3();
	afx_msg void OnGotoLocation4();
	afx_msg void OnGotoLocation5();
	afx_msg void OnGotoLocation6();
	afx_msg void OnGotoLocation7();
	afx_msg void OnGotoLocation8();
	afx_msg void OnGotoLocation9();
	afx_msg void OnGotoLocation10();
	afx_msg void OnGotoLocation11();
	afx_msg void OnGotoLocation12();
	afx_msg void OnToolsLogMemoryUsage();
	afx_msg void OnTerrainExportblock();
	afx_msg void OnTerrainImportblock();
	afx_msg void OnUpdateTerrainExportblock(CCmdUI *pCmdUI);
	afx_msg void OnUpdateTerrainImportblock(CCmdUI *pCmdUI);
	afx_msg void OnCustomizeKeyboard();
	afx_msg void OnToolsConfiguretools();
	afx_msg void OnExecuteTool( UINT nID );
	afx_msg void OnBrushTool();
	afx_msg void OnUpdateBrushTool(CCmdUI *pCmdUI);
	afx_msg void OnExportIndoors();
	afx_msg void OnViewCycle2dviewport();
	afx_msg void OnSnapangle();
	afx_msg void OnUpdateSnapangle(CCmdUI *pCmdUI);
	afx_msg void OnRotateselectionXaxis();
	afx_msg void OnRotateselectionYaxis();
	afx_msg void OnRotateselectionZaxis();
	afx_msg void OnRotateselectionRotateangle();
	afx_msg void OnConvertselectionTobrushes();
	afx_msg void OnConvertselectionTosimpleentity();
	afx_msg void OnEditRenameobject();
	afx_msg void OnChangemovespeedIncrease();
	afx_msg void OnChangemovespeedDecrease();
	afx_msg void OnChangemovespeedChangestep();
	afx_msg void OnModifyAipointPicklink();
	afx_msg void OnGenLightmapsSelected();
	afx_msg void OnMaterialAssigncurrent();
	afx_msg void OnMaterialResettodefault();
	afx_msg void OnMaterialGetmaterial();
	afx_msg void OnToolsUpdatelightmaps();
	afx_msg void OnPhysicsGetState();
	afx_msg void OnPhysicsResetState();
	afx_msg void OnFileSourcesafesettings();
	afx_msg void OnFileSavelevelresources();
	afx_msg void OnValidatelevel();
	afx_msg void OnHelpDynamichelp();
	afx_msg void OnFileChangemod();
	afx_msg void OnTerrainResizeterrain();
	afx_msg void OnToolsPreferences();
	afx_msg void OnEditInvertselection();
	afx_msg void OnPrefabsMakeFromSelection();
	afx_msg void OnPrefabsRefreshAll();
public:
	afx_msg void OnToolterrainmodifySmooth();
	afx_msg void OnTerrainmodifySmooth();
	afx_msg void OnTerrainVegetation();
	afx_msg void OnTerrainPaintlayers();
	afx_msg void OnAvirecorderStartavirecording();
	afx_msg void OnAviRecorderStop();
	afx_msg void OnAviRecorderPause();
	afx_msg void OnAviRecorderOutputFilename();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CRYEDIT_H__41D56446_54D7_49B2_8EF6_884EA7A42365__INCLUDED_)
