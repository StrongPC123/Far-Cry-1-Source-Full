#if !defined(AFX_TERRAINLIGHTING_H__4CAA5295_2647_42FD_8334_359F55EBA4F8__INCLUDED_)
#define AFX_TERRAINLIGHTING_H__4CAA5295_2647_42FD_8334_359F55EBA4F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TerrainLighting.h : header file
//

#include "TerrainTexGen.h"
#include "afxcmn.h"

/////////////////////////////////////////////////////////////////////////////
// CTerrainLighting dialog

enum eLightAlgorithm							// correstpond to the radio buttons in the lighting dialog
{
	eHemisphere = 0,
	eDP3 = 1,
	ePrecise = 2,
};

struct LightingSettings
{
	uint iAmbient;						// Ambient level of lighting.
	DWORD dwSunColor;					// Color of the sun
	DWORD dwSkyColor;					// Color of the sky 
	eLightAlgorithm eAlgo;		// Algorithm to use for the lighting calculations
	bool bTerrainShadows;			// Calculate shadows from hills (default false)
	bool bObjectShadows;			// Calculate shadows from objects (default true)
	bool bLighting;						// Calculate lighting (default true)
	int iSunRotation;					// Rotation of the sun (default 0) 0..360
	int iSunHeight;						// Height of the sun (default 25) 0..100
	int iShadowIntensity;			// Intensity of the shadows
	int iShadowBlur;					// bluriness (no unit) 0=non..100=strong
	float sunMultiplier;			// Multiplier of sun color.
	int iHemiSamplQuality;		// 0=no sampling (=ambient, might be brighter), 1=low (faster) .. 10=highest(slow)

	//! constructor
	LightingSettings()
	{
		iAmbient = 0;
		dwSkyColor = 0x00aa4040;
		dwSunColor = 0x00FFFFFF;
		eAlgo = ePrecise;
		bTerrainShadows = true;
		bObjectShadows = true;
		bLighting = true;
		iSunRotation = 0;
		iSunHeight = 10;
		iShadowIntensity = 50;
		iShadowBlur=1;
		sunMultiplier = 1;
		iHemiSamplQuality=5;
	}

	Vec3 GetSunVector() const
	{
		Vec3 v;

		v.z = - ((float)iSunHeight / 100.0f); 

		float r=sqrtf(1.0f-v.z*v.z);

		v.x = sinf(iSunRotation * gf_DEGTORAD)*r;
		v.y = cosf(iSunRotation * gf_DEGTORAD)*r;
		
		// Normalize the light vector
		return GetNormalized(v);
	}

	void Serialize( XmlNodeRef &node,bool bLoading )
	{
		////////////////////////////////////////////////////////////////////////
		// Load or restore the layer settings from an XML
		////////////////////////////////////////////////////////////////////////
		if (bLoading)
		{
			XmlNodeRef light = node->findChild( "Lighting" );
			if (!light)
				return;

			int algo = 0;
			light->getAttr( "Ambient",iAmbient );
			light->getAttr( "SkyColor",dwSkyColor );
			light->getAttr( "SunColor",dwSunColor );
			light->getAttr( "SunRotation",iSunRotation );
			light->getAttr( "SunHeight",iSunHeight );
			light->getAttr( "Algorithm",algo );
			light->getAttr( "Lighting",bLighting );
			light->getAttr( "Shadows",bTerrainShadows );
			light->getAttr( "ObjShadows",bObjectShadows );
			light->getAttr( "ShadowIntensity",iShadowIntensity );
			light->getAttr( "SunMultiplier",sunMultiplier );
			light->getAttr( "HemiSamplQuality",iHemiSamplQuality );
			light->getAttr( "ShadowBlur",iShadowBlur );

			switch(algo)
			{
				case 0: 	eAlgo = eHemisphere;break;
				case 1:		eAlgo = eDP3;break;
				case 2:		eAlgo = ePrecise;break;			// legacy - for backward compatibility
				case 3:		eAlgo = ePrecise;break;
			}
		}
		else
		{
			int algo = 0;

			switch(eAlgo)
			{
				case eHemisphere: 	algo = 0;break;
				case eDP3:					algo = 1;break;
//													       2						// don't use 2
				case ePrecise:			algo = 3;break;
			}


			// Storing
			XmlNodeRef light = node->newChild( "Lighting" );
			light->setAttr( "Ambient",iAmbient );
			light->setAttr( "SkyColor",dwSkyColor );
			light->setAttr( "SunColor",dwSunColor );
			light->setAttr( "SunRotation",iSunRotation );
			light->setAttr( "SunHeight",iSunHeight );
			light->setAttr( "Algorithm",algo );
			light->setAttr( "Lighting",bLighting );
			light->setAttr( "Shadows",bTerrainShadows );
			light->setAttr( "ObjShadows",bObjectShadows );
			light->setAttr( "ShadowIntensity",iShadowIntensity );
			light->setAttr( "SunMultiplier",sunMultiplier );
			light->setAttr( "HemiSamplQuality",iHemiSamplQuality );
			light->setAttr( "ShadowBlur",iShadowBlur );
			
			Vec3 sunVector = GetSunVector();
			//std::swap(sunVector.x,sunVector.y);
			// Swap X/Y.
			light->setAttr( "SunVector",sunVector );
		}
	}
};

class CTerrainLighting : public CDialog
{
// Construction
public:
	CTerrainLighting(CWnd* pParent = NULL);   // standard constructor
	~CTerrainLighting();

// Dialog Data
	//{{AFX_DATA(CTerrainLighting)
	enum { IDD = IDD_LIGHTING };
	int		m_sldAmbient;
	int		m_optLightingAlgo;
	int		m_sldSunDirection;
	int		m_sldSunHeight;
	BOOL	m_bTerrainShadows;
	BOOL	m_bObjectShadows;
	BOOL	m_bTextured;
	int		m_sldShadowIntensity;
	int		m_sldShadowBlur;					// in degrees 0..100
	int		m_sldSkyQuality;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTerrainLighting)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CBitmap m_bmpLightmap;
	CDC m_dcLightmap;
	CTerrainTexGen m_texGen;
	CImage m_lightmap;

	virtual void OnOK();
	virtual void OnCancel();

	bool m_bDraggingSunDirection;

	// Generated message map functions
	//{{AFX_MSG(CTerrainLighting)
	afx_msg void OnPaint();
	afx_msg void OnGenerate();
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTerrainShadows();
	afx_msg void OnObjectShadows();
	afx_msg void OnFileExport();
	afx_msg void OnFileImport();
	afx_msg void OnSkyFog();
	afx_msg void OnViewWithTexturing();
	afx_msg void OnSunColor();
	afx_msg void OnSkyColor();
	afx_msg void OnHemisphere();
	afx_msg void OnDp3();
	afx_msg void OnSunMultiplier();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedTextured();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void UpdateControls();

	// Permanently allocated for better speed
	float *m_pHeightmapData;

	CNumberCtrl m_sunMult;
	LightingSettings m_originalLightSettings;
public:
	afx_msg void OnBnClickedPreciselighting2();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TERRAINLIGHTING_H__4CAA5295_2647_42FD_8334_359F55EBA4F8__INCLUDED_)
