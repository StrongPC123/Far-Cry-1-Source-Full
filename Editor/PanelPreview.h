////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   panelpreview.h
//  Version:     v1.00
//  Created:     29/3/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __panelpreview_h__
#define __panelpreview_h__
#pragma once

// CPanelPreview dialog
#include "XTToolkit.h"
#include "Controls\PreviewModelCtrl.h"

class CPanelPreview : public CXTResizeDialog
{
	DECLARE_DYNAMIC(CPanelPreview)

public:
	CPanelPreview(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPanelPreview();

	void LoadFile( const CString &filename );

// Dialog Data
	enum { IDD = IDD_PANEL_PREVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	CPreviewModelCtrl m_previewCtrl;
};

#endif // __panelpreview_h__