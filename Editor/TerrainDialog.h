#if !defined(AFX_TERRAINDIALOG_H__7DCC65C5_79C7_4B64_BDAA_6D8F3A43F7B8__INCLUDED_)
#define AFX_TERRAINDIALOG_H__7DCC65C5_79C7_4B64_BDAA_6D8F3A43F7B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TerrainDialog.h : header file
//

#include "ToolbarDialog.h"
#include "DrawWnd.h"

struct SNoiseParams;
class CHeightmap;

/////////////////////////////////////////////////////////////////////////////
// CTerrainDialog dialog

class CTerrainDialog : public CToolbarDialog
{
// Construction
public:
	CTerrainDialog(CWnd* pParent = NULL);   // standard constructor
	~CTerrainDialog();
	SNoiseParams* GetLastParam() { return m_sLastParam; };

// Dialog Data
	//{{AFX_DATA(CTerrainDialog)
	enum { IDD = IDD_TERRAIN };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void Flatten(float fFactor);
	void UpdateBorderCaption();
	float ExpCurve(float v, unsigned int iCover, float fSharpness);
	void Refresh();

	// Generated message map functions
	//{{AFX_MSG(CTerrainDialog)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTerrainLower();
	afx_msg void OnTerrainRaise();
	virtual BOOL OnInitDialog();
	afx_msg void OnTerrainLoad();
	afx_msg void OnTerrainErase();
	afx_msg void OnBrush1();
	afx_msg void OnBrush2();
	afx_msg void OnBrush3();
	afx_msg void OnBrush4();
	afx_msg void OnBrush5();
	afx_msg void OnTerrainResize();
	afx_msg void OnTerrainLight();
	afx_msg void OnTerrainSurface();
	afx_msg void OnTerrainGenerate();
	afx_msg void OnTerrainInvert();
	afx_msg void OnExportHeightmap();
	afx_msg void OnModifyMakeisle();
	afx_msg void OnModifyFlattenLight();
	afx_msg void OnModifyFlattenHeavy();
	afx_msg void OnModifySmooth();
	afx_msg void OnModifyRemovewater();
	afx_msg void OnModifySmoothSlope();
	afx_msg void OnHeightmapShowLargePreview();
	afx_msg void OnModifySmoothBeachesOrCoast();
	afx_msg void OnModifyNoise();
	afx_msg void OnModifyNormalize();
	afx_msg void OnModifyReduceRange();
	afx_msg void OnModifyReduceRangeLight();
	afx_msg void OnModifyRandomize();
	afx_msg void OnLowOpacity();
	afx_msg void OnMediumOpacity();
	afx_msg void OnHighOpacity();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnHold();
	afx_msg void OnFetch();
	afx_msg void OnOptionsShowMapObjects();
	afx_msg void OnOptionsShowWater();
	afx_msg void OnSetToHeight();
	afx_msg void OnNoiseBrush();
	afx_msg void OnNormalBrush();
	afx_msg void OnExportTerrainAsGeometrie();
	afx_msg void OnOptionsEditTerrainCurve();
	afx_msg void OnSetWaterLevel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CDlgToolBar m_cDlgToolBar;
	CDlgToolBar m_cDlgBrushToolBar;
	CDrawWnd m_cDrawHeightmap;

	SNoiseParams* m_sLastParam;

	CHeightmap *m_heightmap;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINDIALOG_H__7DCC65C5_79C7_4B64_BDAA_6D8F3A43F7B8__INCLUDED_)
