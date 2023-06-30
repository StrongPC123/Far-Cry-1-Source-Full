////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushpanel.h
//  Version:     v1.00
//  Created:     2/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __BrushPanel_h__
#define __BrushPanel_h__
#include "afxwin.h"
#pragma once

#include "XTToolkit.h"

class CBrushObject;

// CBrushPanel dialog

class CBrushPanel : public CXTResizeDialog
{
	DECLARE_DYNAMIC(CBrushPanel)

public:
	CBrushPanel(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBrushPanel();

	void SetBrush( CBrushObject *obj );

// Dialog Data
	enum { IDD = IDD_PANEL_BRUSH };

protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelendokSides();
	afx_msg void OnBnClickedReload();

	DECLARE_MESSAGE_MAP()

	CBrushObject *m_brushObj;

	// Controls.
	//CComboBox m_sides;
	CCustomButton m_resetSizeBtn;
	CCustomButton m_reloadBtn;

	afx_msg void OnBnClickedResetsize();
};

#endif // __BrushPanel_h__