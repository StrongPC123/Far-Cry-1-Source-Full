////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   trackviewdialog.h
//  Version:     v1.00
//  Created:     24/4/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __trackviewdialog_h__
#define __trackviewdialog_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "ToolbarDialog.h"
#include "TrackViewSplitter.h"
#include "TrackViewNodes.h"
#include "TrackViewKeyList.h"
#include "TrackViewGraph.h"
#include "TrackViewTimeBar.h"
#include "TrackViewSpline.h"
#include "TVTrackPropsDialog.h"
#include "IMovieSystem.h"

class CMovieCallback;

// CTrackViewDialog dialog

class CTrackViewDialog : public CToolbarDialog, public IDocListener
{
	DECLARE_DYNAMIC(CTrackViewDialog)

public:
	friend CMovieCallback;

	CTrackViewDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrackViewDialog();

// Dialog Data
	enum { IDD = IDD_TRACKVIEWDIALOG };

	void InvalidateTrackList();

	// Must be called by main frame.
	void Update();

	void SetCurrentSequence( IAnimSequence *curr );
	void ReloadSequences();

	//////////////////////////////////////////////////////////////////////////
	// IDocListenerer implementation.
	//////////////////////////////////////////////////////////////////////////
	virtual	void OnNewDocument();
	virtual	void OnLoadDocument();
	virtual void OnCloseDocument();
	virtual void OnMissionChange();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CTrackViewSplitter m_wndSplitter;
	CTrackViewNodes	m_wndNodes;
	CTrackViewKeyList	m_wndKeys;
	CTrackViewGraph	m_wndGraph;
	CTrackViewSpline m_wndSpline;
	CTVTrackPropsDialog m_wndTrackProps;
	CDlgToolBar m_cDlgToolBar;
	CComboBox m_sequences;
	CStatic m_CursorPos;
	CMovieCallback *m_pMovieCallback;
	IMovieSystem *m_movieSystem;
	IAnimSequence *m_currSequence;

	bool m_bRecord;
	bool m_bAutoRecord;
	bool m_bPlay;
	bool m_bPause;
	bool m_bReloading;

	float m_fLastTime;
	float m_fAutoRecordStep;
	CString m_aviFile;

	GUID m_defaulCameraObjectId;
	static CString m_currSequenceName;

	void InitToolbar();
	void InitSequences();
	void OnSetCamera( const SCameraParams &camParams );
	void OnPlayMenu( CPoint pos );
	void OnRecordMenu( CPoint pos );
	IMovieSystem* GetMovieSystem();

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();

	afx_msg void OnGoToPrevKey();
	afx_msg void OnGoToNextKey();
	afx_msg void OnAddKey();
	afx_msg void OnDelKey();
	afx_msg void OnMoveKey();
	afx_msg void OnSlideKey();
	afx_msg void OnScaleKey();
	afx_msg void OnAddSequence();
	afx_msg void OnDelSequence();
	afx_msg void OnEditSequence();
	afx_msg void OnChangeSequence();
	afx_msg void OnAddSelectedNode();
	afx_msg void OnAddSceneTrack();

	afx_msg void OnRecord();
	afx_msg void OnAutoRecord();
	afx_msg void OnGoToStart();
	afx_msg void OnGoToEnd();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnPause();
	afx_msg void OnLoop();
	afx_msg void OnCopyPasteKeys();

	afx_msg void OnToolbarDropDown(NMHDR *pnhdr, LRESULT *plr);

	afx_msg void OnUpdateRecord( CCmdUI* pCmdUI );
	afx_msg void OnUpdateRecordAuto( CCmdUI* pCmdUI );
	afx_msg void OnUpdatePlay( CCmdUI* pCmdUI );
	afx_msg void OnUpdatePause( CCmdUI* pCmdUI );
	afx_msg void OnUpdateLoop( CCmdUI* pCmdUI );
	afx_msg void OnUpdateCopyPasteKeys( CCmdUI* pCmdUI );
	afx_msg void OnAddSelectedNodeUpdate( CCmdUI* pCmdUI );

	afx_msg void OnExportSequence();
	afx_msg void OnImportSequence();

	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnDestroy();
};

#endif // __trackviewdialog_h__