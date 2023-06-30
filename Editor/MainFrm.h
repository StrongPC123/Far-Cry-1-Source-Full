// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__88B37B80_D04F_46F1_8FEF_A09696002A81__INCLUDED_)
#define AFX_MAINFRM_H__88B37B80_D04F_46F1_8FEF_A09696002A81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <XTToolkit.h>

#include "Controls\ConsoleSCB.h"

#include "TrackViewDialog.h"
#include "DataBaseDialog.h"

#include "Controls\RollupBar.h"
#include "Controls\RollupCtrl.h"

#include "Controls\HiColorToolBar.h"
#include "Controls\ToolbarTab.h"
#include "Controls\EditModeToolbar.h"
#include "InfoBarHolder.h"

#include "SoundPresetsDlg.h"
#include "EAXPresetsDlg.h"
#include "MusicInfoDlg.h"

// forward declaration.
class CMission;
class CLayoutWnd;

class CMainFrame : public CXTFrameWnd
{
	
public: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

	//! Show window and restore saved state.
	void ShowWindowEx(int nCmdShow);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
	virtual void ActivateFrame(int nCmdShow);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext);

	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Access the status bar to display toolbar tooltips and status messages
	void SetStatusText( LPCTSTR pszText );
	void SetStatusText(CString strText) { SetStatusText(strText); };

	int SelectRollUpBar( int rollupBarId );
	CRollupCtrl* GetRollUpControl( int rollupBarId=ROLLUP_OBJECTS );

	//! Access to track view dialog.
	CTrackViewDialog* GetTrackView();

	void EnableProgressBar( bool bEnable );
	CInfoProgressBar* GetProgressBar();

	CLayoutWnd*	GetLayout() { return m_layoutWnd; };
	CString GetSelectionName();
	void SetSelectionName( const CString &name );
	void AddSelectionName( const CString &name );
	void RemoveSelectionName( const CString &name );

	void UncheckMainTools();

	bool ShowConsole( bool enable );
//	void EnableAccelerator( bool bEnable );

	void IdleUpdate();

	void OnMissionUpdate();

	//! Enable/Disable keyboard accelerator.
	void EnableAccelerator( bool bEnable );
	//! Edit keyboard shortcuts.
	void EditAccelerator();

	// Check if dock state is valid with this window.
	BOOL VerifyBarState( CDockState &state );

	//! Save current window configuration.
	void SaveConfig();

	//! Put external tools to menu.
	void UpdateTools();

	//! Returnns pointer to data base dialog.
	CDataBaseDialog* GetDataBaseDialog() { return &m_wndDataBase; };
	void ShowDataBaseDialog( bool bShow );

	// Check if some window is child of ouw docking windows.
	bool IsDockedWindowChild( CWnd *pWnd );

protected:
	void CreateMissionsBar();
	void CreateRollupBar();
	void DockControlBarLeftOf(CControlBar *Bar, CControlBar *LeftOf);
	void DockControlBarNextTo(CControlBar* pBar,CControlBar* pTargetBar);
	bool IsPreview() const;

	bool FindMenuPos(CMenu *pBaseMenu, UINT myID, CMenu * & pMenu, int & mpos);
	void DeleteToolsFromMenu( CMenu *menu );

	void AddToolbarToAccel( const CString &name,CXTToolBar *toolbar );

	void LoadTrueColorToolbar( CXTToolBar &bar,UINT nImageResource );

	//////////////////////////////////////////////////////////////////////////
	// ControlBars
	//////////////////////////////////////////////////////////////////////////
	/*
	CExtMenuControlBar m_wndMenuBar;
	CExtStatusControlBar m_wndStatusBar;
	CExtToolControlBar	m_wndToolBar;
	CEditModeToolBar m_editModeBar;
	CExtToolControlBar m_objectModifyBar;
	CExtToolControlBar m_missionToolBar;
	CExtToolControlBar m_wndTerrainToolBar;
	CExtControlBar m_wndRollUpBar;
	CExtControlBar m_wndConsoleBar;
	CExtControlBar m_wndTrackViewBar;
	*/

	CXTStatusBar	m_wndStatusBar;
	CXTReBar			m_wndReBar;
	CEditModeToolBar m_editModeBar;
	CXTToolBar m_wndToolBar;
	CXTToolBar m_objectModifyBar;
	CXTToolBar m_missionToolBar;
	CXTToolBar m_wndTerrainToolBar;
	CXTToolBar m_wndAvoToolBar;
	
	CXTDockWindow m_wndRollUpBar;
	CXTDockWindow m_wndConsoleBar;
	CXTDockWindow m_wndTrackViewBar;
	CXTDockWindow m_wndDataBaseBar;

	//////////////////////////////////////////////////////////////////////////

	CLayoutWnd *m_layoutWnd;
		
	//! Console dialog
	CConsoleSCB m_cConsole;

	//! Track view dialog.
	CTrackViewDialog m_wndTrackView;
	CDataBaseDialog m_wndDataBase;

	// Rollup sizing bar
	CRollupBar m_wndRollUp;
	CRollupCtrl m_objectRollupCtrl; // Rollup itself
	CRollupCtrl m_terrainRollupCtrl; // Rollup itself
	CRollupCtrl m_displayRollupCtrl; // Rollup itself
	CRollupCtrl m_layersRollupCtrl; // Rollup itself

	CSoundPresetsDlg m_wndSoundPresets;
	CEAXPresetsDlg m_wndEAXPresets;
	CMusicInfoDlg m_wndMusicInfo;

	//CToolBar m_terrain;
	// Info dialog.
	CInfoBarHolder m_infoBarHolder;
	CXTFlatComboBox m_missions;
	//CComboBox m_missions;

	class CTerrainPanel* m_terrainPanel;
	class CMainTools* m_mainTools;

	CString		m_selectionName;
	//CDateTimeCtrl m_missionTime;
	CMission*	m_currentMission;
	class CObjectLayer *m_currentLayer;

	bool m_consoleVisible;
	float m_gridSize;

	bool m_bXPLook;

	//! Saves mainframe position.
	CXTWindowPos m_wndPosition;

	int m_autoSaveTimer;
	int m_autoRemindTimer;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateToolbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateConsole(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRollUpBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTrackView(CCmdUI* pCmdUI);
	afx_msg void OnToolbar();
	afx_msg void OnStatusBar();
	afx_msg void OnRollUpBar();
	afx_msg void OnConsoleWindow();
	afx_msg void OnTrackView();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg void OnClose();
	afx_msg void OnMissionChanged();
	afx_msg void OnMissionCancelChanged();
	afx_msg void OnMissionDropDown();
	afx_msg void OnXPLook();
	afx_msg void OnUpdateXPLook(CCmdUI* pCmdUI);
	afx_msg void OnSoundPresets();
	afx_msg void OnEAXPresets();
	afx_msg void OnMusicInfo();
	afx_msg void OnProgressBarCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnEditNextSelectionMask();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__88B37B80_D04F_46F1_8FEF_A09696002A81__INCLUDED_)
