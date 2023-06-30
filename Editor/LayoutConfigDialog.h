////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   layoutconfigdialog.h
//  Version:     v1.00
//  Created:     26/9/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __layoutconfigdialog_h__
#define __layoutconfigdialog_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "LayoutWnd.h"

// CLayoutConfigDialog dialog

class CLayoutConfigDialog : public CDialog
{
	DECLARE_DYNAMIC(CLayoutConfigDialog)

public:
	CLayoutConfigDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLayoutConfigDialog();

	void SetLayout( EViewLayout layout ) { m_layout = layout; };
	EViewLayout GetLayout() const { return m_layout; };

// Dialog Data
	enum { IDD = IDD_LAYOUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

	CListCtrl m_layouts;
	CImageList m_imageList;
	EViewLayout m_layout;
};

#endif // __layoutconfigdialog_h__