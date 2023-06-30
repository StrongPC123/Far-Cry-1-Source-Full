////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tvtrackpropsdialog.h
//  Version:     v1.00
//  Created:     28/5/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __tvtrackpropsdialog_h__
#define __tvtrackpropsdialog_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "TcbKeyDialog.h"
#include "IMovieSystem.h"

struct IAnimNode;
struct IAnimTrack;

struct SKeyDlgInfo
{
	IKeyDlg *pDlg;
	typedef std::vector<EAnimTrackType> TTracks;
	TTracks Tracks;
};

// CTVTrackPropsDialog dialog

class CTVTrackPropsDialog : public CDialog
{
	DECLARE_DYNAMIC(CTVTrackPropsDialog)

public:
	CTVTrackPropsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVTrackPropsDialog();

	void SetKey( IAnimNode *node,int paramId,IAnimTrack *track,int key );
	void ReloadKey();

// Dialog Data
	enum { IDD = IDD_TV_TRACK_PROPS };

protected:
	void SetCurrKey( int nkey );

	virtual void OnOK() {};
	virtual void OnCancel();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnDeltaposPrevnext(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnUpdateTime();
	afx_msg void OnBnClickedConstant();
	afx_msg void OnBnClickedCycle();
	afx_msg void OnBnClickedLoop();

	DECLARE_MESSAGE_MAP()

	CSpinButtonCtrl m_keySpinBtn;
	CStatic m_keynum;

	IAnimTrack* m_track;
	IAnimNode* m_node;
	int m_key;
	CNumberCtrl m_time;

	typedef std::vector<SKeyDlgInfo> TKeyDlgs;
	TKeyDlgs m_KeyDlg;

	int m_currentDlg;
//	CTVEntityKeyDialog m_dlgEntity;
//	CTcbKeyDialog m_dlgTcb;
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
};

#endif // __tvtrackpropsdialog_h__