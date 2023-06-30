#if !defined(AFX_TERRAINTEXTURE_H__44644B20_29E9_4005_9F28_003914D5FE37__INCLUDED_)
#define AFX_TERRAINTEXTURE_H__44644B20_29E9_4005_9F28_003914D5FE37__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TerrainTexture.h : header file
//
#include <XTToolkit.h>
#include "TerrainTexGen.h"

// forward declarations.
class CLayer;

// Internal resolution of the final texture preview
#define FINAL_TEX_PREVIEW_PRECISION_CX 256
#define FINAL_TEX_PREVIEW_PRECISION_CY 256

// Hold / fetch temp file
#define HOLD_FETCH_FILE_TTS "Temp\\HoldStateTemp.lay"

/////////////////////////////////////////////////////////////////////////////
// CTerrainTexture dialog

enum SurfaceGenerateFlags {
	GEN_USE_LIGHTING		= 1,
	GEN_SHOW_WATER			= 1<<2,
	GEN_SHOW_WATERCOLOR	= 1<<3,
	GEN_KEEP_LAYERMASKS = 1<<4,
	GEN_ABGR            = 1<<5,
	GEN_STATOBJ_SHADOWS = 1<<6
};

class CTerrainTexture : public CDialog
{
// Construction
public:
	static void ClearTexturePreview();
	bool GenerateSurface(DWORD *pSurface, UINT iWidth, UINT iHeight, int flags,CBitArray *pLightingBits = NULL, float **ppHeightmapData = NULL);
	void ReloadLayerList();
	void ExportLargePreview();
	
	CTerrainTexture(CWnd* pParent = NULL);   // standard constructor
	~CTerrainTexture();

// Dialog Data
	//{{AFX_DATA(CTerrainTexture)
	enum { IDD = IDD_TERRAIN_TEXTURE };
	CComboBox	m_surfaceType;
	CXTEditListBox m_lstLayers;
	//}}AFX_DATA

	CSliderCtrl m_slopeSlider;
	CSliderCtrl m_altSlider;
	CEdit m_slopeSliderPos;
	CEdit m_altSliderPos;

	CCustomButton m_slopeStartBtn;
	CCustomButton m_slopeEndBtn;
	CCustomButton m_altStartBtn;
	CCustomButton m_altEndBtn;
	CCustomButton m_editSrurfaceTypesBtn;
	CCustomButton m_loadTextureBtn;
	CCustomButton m_importMaskBtn;
	CCustomButton m_exportMaskBtn;
	CButton m_smoothBtn;

	CNumberCtrl m_slopeStart;
	CNumberCtrl m_slopeEnd;
	CNumberCtrl m_altStart;
	CNumberCtrl m_altEnd;

	CStatic m_texInfo;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainTexture)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Dialog control related functions
	void UpdateControlData();
	void EnableControls();
	void DrawLayerPreview( LPRECT rcPos, CDC *pDC );

	// The currently active layer (syncronized with the list box selection)
	CLayer *m_pCurrentLayer;
	CCryEditDoc* m_doc;

	CTerrainTexGen m_texGen;

	// Preview of the final texture
	static CBitmap m_bmpFinalTexPrev;
	static CDC m_dcFinalTexPrev;

	// Apply lighting to the previews ?
	static bool m_bUseLighting;

	// Show water in the preview ?
	static bool m_bShowWater;

	bool m_bLayerTexClicked;
	bool m_bLayerTexSelected;

	// Layer mask preview
	CBitmap m_bmpLayerPreview;
	CDC m_dcLayerPreview;
	bool m_bMaskPreviewValid;

	// Generated message map functions
	//{{AFX_MSG(CTerrainTexture)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeLayerList();
	afx_msg void OnSelStart();
	afx_msg void OnSelEnd();
	afx_msg void OnLoadTexture();
	afx_msg void OnPaint();
	afx_msg void OnSlopeSelStart();
	afx_msg void OnSlopeSelEnd();
	afx_msg void OnImport();
	afx_msg void OnExport();
	afx_msg void OnFileExportLargePreview();
	afx_msg void OnGeneratePreview();
	afx_msg void OnApplyLighting();
	afx_msg void OnSetWaterLevel();
	afx_msg void OnLayerExportTexture();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnHold();
	afx_msg void OnFetch();
	afx_msg void OnUseLayer();
	afx_msg void OnOptionsSetLayerBlending();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnAutoGenMask();
	afx_msg void OnLoadMask();
	afx_msg void OnExportMask();
	afx_msg void OnShowWater();
	afx_msg void OnEditSurfaceTypes();
	afx_msg void OnSelendokSurfaceType();
	afx_msg void OnLayerSetWaterColor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	afx_msg void OnDestroy();

	afx_msg void OnLayersLabelEditEnd();
	afx_msg void OnLayersLabelEditCancel();
	afx_msg void OnLayersNewItem();
	afx_msg void OnLayersDeleteItem();
	afx_msg void OnLayersMoveItemUp();
	afx_msg void OnLayersMoveItemDown();
		
	afx_msg void OnLayerValuesChange();
	afx_msg void OnLayerValuesUpdate();
	afx_msg void OnBnClickedSmooth();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINTEXTURE_H__44644B20_29E9_4005_9F28_003914D5FE37__INCLUDED_)
