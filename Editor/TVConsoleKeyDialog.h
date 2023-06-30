////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tvconsolekeydialog.h
//  Version:     v1.00
//  Created:     12/6/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __tvconsolekeydialog_h__
#define __tvconsolekeydialog_h__
#pragma once

struct IAnimNode;
struct IAnimTrack;

#include "ikeydlg.h"

// CTVEntityKeyDialog dialog

class CTVConsoleKeyDialog : public IKeyDlg
{
	DECLARE_DYNAMIC(CTVConsoleKeyDialog)

public:
	CTVConsoleKeyDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTVConsoleKeyDialog();

// Dialog Data
	enum { IDD = IDD_TV_CONSOLE_KEY };

	void SetKey( IAnimNode *node,IAnimTrack *track,int key );

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	IAnimTrack* m_track;
	IAnimNode* m_node;
	int m_key;
public:
	CEdit m_command;

	virtual BOOL OnInitDialog();
	///CButton m_hide;
	afx_msg void ControlsToKey();
};

#endif // __tvconsolekeydialog_h__
