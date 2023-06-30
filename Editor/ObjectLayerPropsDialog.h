////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objectlayerpropsdialog.h
//  Version:     v1.00
//  Created:     26/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __objectlayerpropsdialog_h__
#define __objectlayerpropsdialog_h__
#pragma once

// CObjectLayerPropsDialog dialog

class CObjectLayerPropsDialog : public CDialog
{
	DECLARE_DYNAMIC(CObjectLayerPropsDialog)

public:
	CObjectLayerPropsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CObjectLayerPropsDialog();

// Dialog Data
	enum { IDD = IDD_LAYER_PROPS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

public:
	BOOL m_bVisible;
	BOOL m_bFrozen;
	BOOL m_bExternal;
	BOOL m_bExportToGame;
	BOOL m_bMainLayer;
	CString m_name;
};

#endif // __objectlayerpropsdialog_h__