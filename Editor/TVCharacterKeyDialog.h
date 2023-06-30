////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tvcharacterkeydialog.h
//  Version:     v1.00
//  Created:     20/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __tvcharacterkeydialog_h__
#define __tvcharacterkeydialog_h__

#if _MSC_VER > 1000
#pragma once
#endif

struct IAnimNode;
struct IAnimTrack;

#include "ikeydlg.h"

// CTVCharacterKeyDialog dialog

class CTVCharacterKeyDialog : public IKeyDlg
{
	DECLARE_DYNAMIC(CTVCharacterKeyDialog)

public:
	CTVCharacterKeyDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVCharacterKeyDialog();

// Dialog Data
	enum { IDD = IDD_TV_CHARACTER_KEY };

	void SetKey( IAnimNode *node,IAnimTrack *track,int key );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	IAnimTrack* m_track;
	IAnimNode* m_node;
	int m_key;
public:
	CComboBox m_animation;
	CNumberCtrl m_blendTime;
	CNumberCtrl m_timeScale;
	CNumberCtrl m_startTime;
	CButton m_loopBtn;
	CButton m_unloadBtn;

	virtual BOOL OnInitDialog();
	///CButton m_hide;
	afx_msg void ControlsToKey();
	afx_msg void OnBnClickedLoop();
	afx_msg void OnBnClickedUnload();
};

#endif // __tvcharacterkeydialog_h__