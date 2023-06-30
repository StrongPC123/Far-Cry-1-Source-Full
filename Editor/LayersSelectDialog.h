////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   layersselectdialog.h
//  Version:     v1.00
//  Created:     11/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __layersselectdialog_h__
#define __layersselectdialog_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Controls\LayersListBox.h"

// CLayersSelectDialog dialog

class CLayersSelectDialog : public CDialog
{
	DECLARE_DYNAMIC(CLayersSelectDialog)

public:
	CLayersSelectDialog( CPoint point,CWnd* pParent = NULL);   // standard constructor
	virtual ~CLayersSelectDialog();

// Dialog Data
	enum { IDD = IDD_LAYERS_SELECT };

	void SetSelectedLayer( const CString &sel ) { m_selectedLayer = sel; };
	CString GetSelectedLayer() { return m_selectedLayer; };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void ReloadLayers();

	DECLARE_MESSAGE_MAP()

	virtual void OnOK() { EndDialog(IDCANCEL);};
	//virtual void OnCancel() {};

	CColorCtrl<CLayersListBox> m_layers;
	afx_msg void OnLbnSelchangeLayers();
	virtual BOOL OnInitDialog();

	CString m_selectedLayer;
	CPoint m_origin;
	afx_msg void OnLbnSelcancelLayers();
};

#endif // __layersselectdialog_h__