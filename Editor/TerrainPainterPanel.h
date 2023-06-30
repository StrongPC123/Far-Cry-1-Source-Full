////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrainpainterpanel.h
//  Version:     v1.00
//  Created:     25/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __terrainpainterpanel_h__
#define __terrainpainterpanel_h__
#include "afxwin.h"

#if _MSC_VER > 1000
#pragma once
#endif

struct CTextureBrush;

/////////////////////////////////////////////////////////////////////////////
// CTerrainPainterPanel dialog
class CTerrainTexturePainter;

class CTerrainPainterPanel : public CDialog
{
// Construction
public:
	CTerrainPainterPanel( CTerrainTexturePainter *tool,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTerrainPainterPanel)
	enum { IDD = IDD_PANEL_TERRAIN_LAYER };
	CComboBox	m_brushType;
	CSliderCtrl	m_hardnessSlider;
	CSliderCtrl	m_heightSlider;
	CSliderCtrl	m_radiusSlider;
	CListBox m_layers;
	//}}AFX_DATA

	void SetBrush( CTextureBrush &brush );
	CString GetSelectedLayer();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainPainterPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	void ReloadLayers();

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateNumbers();
	afx_msg void OnHardnessSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeightSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRadiusSlider(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelendokBrushType();
	afx_msg void OnBnClickedPaintvegetation();
	afx_msg void OnBnClickedPaintSimplelighting();
	afx_msg void OnBnClickedPaintTerrainShadows();
	afx_msg void OnBnClickedPaintObjectShadows();
	DECLARE_MESSAGE_MAP()

	CNumberCtrl m_brushRadius;
	CNumberCtrl m_brushHeight;
	CNumberCtrl m_brushHardness;
	CTerrainTexturePainter*	m_tool;
	CButton m_paintVegetation;
	CButton m_optLighting;
	CButton m_optTerrainShadows;
	CButton m_optObjectShadows;
};

#endif // __terrainpainterpanel_h__
