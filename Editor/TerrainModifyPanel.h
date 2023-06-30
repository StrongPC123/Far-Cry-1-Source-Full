#if !defined(AFX_TERRAINMODIFYPANEL_H__341477A9_CD41_422F_9D74_C674FD62D9C3__INCLUDED_)
#define AFX_TERRAINMODIFYPANEL_H__341477A9_CD41_422F_9D74_C674FD62D9C3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TerrainModifyPanel.h : header file
//

struct CTerrainBrush;

/////////////////////////////////////////////////////////////////////////////
// CTerrainModifyPanel dialog
class CTerrainModifyTool;

class CTerrainModifyPanel : public CDialog
{
// Construction
public:
	CTerrainModifyPanel( CTerrainModifyTool *tool,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTerrainModifyPanel)
	enum { IDD = IDD_PANEL_TERRAIN_MODIFY };
	CComboBox	m_brushType;
	CSliderCtrl	m_noiseFreqSlider;
	CSliderCtrl	m_noiseScaleSlider;
	CSliderCtrl	m_hardnessSlider;
	CSliderCtrl	m_heightSlider;
	CSliderCtrl	m_radiusSlider;
	CSliderCtrl	m_radiusSlider2;
	//}}AFX_DATA

	void SetBrush( CTerrainBrush &brush );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainModifyPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnOK() {};
	virtual void OnCancel() {};

	// Generated message map functions
	//{{AFX_MSG(CTerrainModifyPanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdateNumbers();
	afx_msg void OnBrushNoise();
	afx_msg void OnSelendokBrushType();
	afx_msg void OnRepositionObjects();
	afx_msg void OnHScroll( UINT nSBCode,UINT nPos,CScrollBar* pScrollBar );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CNumberCtrl m_brushRadius;
	CNumberCtrl m_brushRadius2;
	CNumberCtrl m_brushHeight;
	CNumberCtrl m_brushHardness;
	CNumberCtrl m_noiseScale;
	CNumberCtrl m_noiseFreq;
	CTerrainModifyTool*	m_tool;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINMODIFYPANEL_H__341477A9_CD41_422F_9D74_C674FD62D9C3__INCLUDED_)
