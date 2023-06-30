////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   gridsettingsdialog.h
//  Version:     v1.00
//  Created:     26/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __gridsettingsdialog_h__
#define __gridsettingsdialog_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Controls\NumberCtrl.h"

// CGridSettingsDialog dialog

class CGridSettingsDialog : public CDialog
{
	DECLARE_DYNAMIC(CGridSettingsDialog)

public:
	CGridSettingsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGridSettingsDialog();

// Dialog Data
	enum { IDD = IDD_GRID_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()
private:
	CNumberCtrl m_gridSize;
	CNumberCtrl m_gridScale;
	CNumberCtrl m_angleSnapScale;
	CButton m_angleSnap;
	CButton m_snapToGrid;
};

#endif // __gridsettingsdialog_h__