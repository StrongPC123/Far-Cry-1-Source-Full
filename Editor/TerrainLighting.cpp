// TerrainLighting.cpp : implementation file
//

#include "stdafx.h"
#include "TerrainLighting.h"
#include "CryEditDoc.h"
#include "TerrainTexture.h"
#include "Heightmap.h"
#include "WaitProgress.h"
#include "GameEngine.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LIGHTING_PREVIEW_RESOLUTION 256

/////////////////////////////////////////////////////////////////////////////
// CTerrainLighting dialog


CTerrainLighting::CTerrainLighting(CWnd* pParent /*=NULL*/)
	: CDialog(CTerrainLighting::IDD, pParent),
	m_texGen(LIGHTING_PREVIEW_RESOLUTION)
{
	//{{AFX_DATA_INIT(CTerrainLighting)
	m_sldAmbient = 0;
	m_optLightingAlgo = -1;
	m_sldSunDirection = 0;
	m_sldSunHeight = 0;
	m_bTerrainShadows = FALSE;
	m_bObjectShadows = FALSE;
	m_bTextured = FALSE;
	m_sldShadowIntensity = 0;
	m_sldShadowBlur=0;
	m_sldSkyQuality=0;
	//}}AFX_DATA_INIT
	m_bDraggingSunDirection = false;
}

CTerrainLighting::~CTerrainLighting()
{
}

void CTerrainLighting::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainLighting)
	DDX_Slider(pDX, IDC_AMBIENT, m_sldAmbient);
	DDX_Radio(pDX, IDC_HEMISPHERE, m_optLightingAlgo);
	DDX_Slider(pDX, IDC_SUN_DIRECTION, m_sldSunDirection);
	DDX_Slider(pDX, IDC_SUN_HEIGHT, m_sldSunHeight);
	DDX_Check(pDX, IDC_TERRAINSHADOWS, m_bTerrainShadows);
	DDX_Check(pDX, IDC_OBJECTSHADOWS, m_bObjectShadows);
	DDX_Check(pDX, IDC_TEXTURED,m_bTextured );
	DDX_Slider(pDX, IDC_SHADOW_INTENSITY, m_sldShadowIntensity);
	DDX_Slider(pDX, IDC_SHADOW_BLUR, m_sldShadowBlur);
	DDX_Slider(pDX, IDC_HEMISAMPLEQ, m_sldSkyQuality);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainLighting, CDialog)
	//{{AFX_MSG_MAP(CTerrainLighting)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_GENERATE, OnGenerate)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_TERRAINSHADOWS, OnTerrainShadows)
	ON_BN_CLICKED(IDC_OBJECTSHADOWS, OnObjectShadows)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_FILE_IMPORT, OnFileImport)
	ON_BN_CLICKED(IDC_VIEW_WITH_TEXTURING, OnViewWithTexturing)
	ON_BN_CLICKED(IDC_SUN_COLOR, OnSunColor)
	ON_BN_CLICKED(IDC_SKY_COLOR, OnSkyColor)
	ON_BN_CLICKED(IDC_HEMISPHERE, OnHemisphere)
	ON_BN_CLICKED(IDC_DP3, OnDp3)
	ON_EN_UPDATE(IDC_SUNCOLOR_SCALE,OnSunMultiplier)
	//}}AFX_MSG_MAP
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_TEXTURED, OnBnClickedTextured)
	ON_BN_CLICKED(IDC_PRECISELIGHTING2, OnBnClickedPreciselighting2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerrainLighting message handlers

void CTerrainLighting::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	RECT rcClient;
	CPen cGrayPen(PS_SOLID, 1, 0x007F7F7F);
	CPen cWhitePen(PS_SOLID, 1, 0x00FFFFFF);
	CPen cRedPen(PS_SOLID, 1, 0x000000FF);

	// Generate a preview if we don't have one
	if (!m_dcLightmap.m_hDC)
		OnGenerate();
	
	// Draw the preview of the lightmap
	dc.BitBlt(19, 24, LIGHTING_PREVIEW_RESOLUTION, LIGHTING_PREVIEW_RESOLUTION, &m_dcLightmap, 0, 0, SRCCOPY);

	CRect rc(19,24,19+LIGHTING_PREVIEW_RESOLUTION,24+LIGHTING_PREVIEW_RESOLUTION);

	// Draw Sun direction.
	CPen *prevPen = dc.SelectObject(&cRedPen);

	CPoint center = rc.CenterPoint();
	Vec3 sunVector = GetIEditor()->GetDocument()->GetLighting()->GetSunVector();
	sunVector.z = 0;
	sunVector.Normalize();
	CPoint source = center + CPoint(-sunVector.x*120,-sunVector.y*120);
	dc.MoveTo( center );
	dc.LineTo( source );


	// Draw a menu bar separator
	GetClientRect(&rcClient);
	dc.SelectObject(&cGrayPen);

	dc.MoveTo(0, 0);
	dc.LineTo(rcClient.right, 0);
	dc.SelectObject(&cWhitePen);
	dc.MoveTo(0, 1);
	dc.LineTo(rcClient.right, 1);

	dc.SelectObject(prevPen);
		
	// Do not call CDialog::OnPaint() for painting messages
}

void CTerrainLighting::OnGenerate() 
{
	////////////////////////////////////////////////////////////////////////
	// Generate a preview of the lightmap
	////////////////////////////////////////////////////////////////////////
	
	RECT rcUpdate;

	BeginWaitCursor();

	// Create a DC and a bitmap
	if (!m_dcLightmap.m_hDC)
		VERIFY(m_dcLightmap.CreateCompatibleDC(NULL));
	if (!m_bmpLightmap.m_hObject)
		VERIFY(m_bmpLightmap.CreateBitmap(LIGHTING_PREVIEW_RESOLUTION, LIGHTING_PREVIEW_RESOLUTION, 1, 32, NULL));
	m_dcLightmap.SelectObject(&m_bmpLightmap);

	// Calculate the lighting.
	m_texGen.InvalidateLighting();
	if (m_bTextured)
		m_texGen.GenerateSurfaceTexture( ETTG_LIGHTING|ETTG_QUIET|ETTG_ABGR|ETTG_INVALIDATE_LAYERS,m_lightmap );
	 else
		m_texGen.GenerateSurfaceTexture( ETTG_LIGHTING|ETTG_NOTEXTURE|ETTG_QUIET|ETTG_ABGR|ETTG_INVALIDATE_LAYERS,m_lightmap );

	m_bmpLightmap.SetBitmapBits( m_lightmap.GetSize(),(DWORD*)m_lightmap.GetData() );

	// Update the preview
	::SetRect(&rcUpdate, 10, 10, 20 + LIGHTING_PREVIEW_RESOLUTION, 30 + LIGHTING_PREVIEW_RESOLUTION);
	InvalidateRect(&rcUpdate, FALSE);
	UpdateWindow();

	EndWaitCursor();
}

BOOL CTerrainLighting::OnInitDialog() 
{
	////////////////////////////////////////////////////////////////////////
	// Initialize the dialog with the settings from the document
	////////////////////////////////////////////////////////////////////////
	m_originalLightSettings = *GetIEditor()->GetDocument()->GetLighting();

	CSliderCtrl ctrlSlider;

	CLogFile::WriteLine("Loading lighting dialog...");

	CDialog::OnInitDialog();
	
	// Set the ranges for the slider controls
	VERIFY(ctrlSlider.Attach(GetDlgItem(IDC_SUN_DIRECTION)->m_hWnd));
	ctrlSlider.SetRange(0, 360, TRUE);
	ctrlSlider.Detach();
	VERIFY(ctrlSlider.Attach(GetDlgItem(IDC_SUN_HEIGHT)->m_hWnd));
	ctrlSlider.SetRange(0, 100, TRUE);
	ctrlSlider.Detach();
	VERIFY(ctrlSlider.Attach(GetDlgItem(IDC_SHADOW_BLUR)->m_hWnd));
	ctrlSlider.SetRange(0, 100, TRUE);
	ctrlSlider.Detach();
	VERIFY(ctrlSlider.Attach(GetDlgItem(IDC_SHADOW_INTENSITY)->m_hWnd));
	ctrlSlider.SetRange(0, 100, TRUE);
	ctrlSlider.Detach();
	VERIFY(ctrlSlider.Attach(GetDlgItem(IDC_AMBIENT)->m_hWnd));
	ctrlSlider.SetRange(0, 255, TRUE);
	ctrlSlider.Detach();
	VERIFY(ctrlSlider.Attach(GetDlgItem(IDC_HEMISAMPLEQ)->m_hWnd));
	ctrlSlider.SetRange(0, 10, TRUE);
	ctrlSlider.Detach();

	m_sunMult.Create( this,IDC_SUNCOLOR_SCALE );

	m_lightmap.Allocate( LIGHTING_PREVIEW_RESOLUTION,LIGHTING_PREVIEW_RESOLUTION );
	m_bDraggingSunDirection = false;

	// Synchronize the controls with the values from the document
	UpdateControls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTerrainLighting::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	////////////////////////////////////////////////////////////////////////
	// Update the document with the values from the sliders
	////////////////////////////////////////////////////////////////////////

	UpdateData(TRUE);

	LightingSettings *ls = GetIEditor()->GetDocument()->GetLighting();

	ls->iAmbient = m_sldAmbient;
	ls->iSunRotation = m_sldSunDirection;
	ls->iSunHeight = m_sldSunHeight;
	ls->iShadowIntensity = m_sldShadowIntensity;
	ls->iShadowBlur=m_sldShadowBlur;
	ls->iHemiSamplQuality = m_sldSkyQuality;
			
	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Update the preview
	OnGenerate();

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTerrainLighting::OnSunMultiplier()
{
	GetIEditor()->GetDocument()->GetLighting()->sunMultiplier = m_sunMult.GetValue();
	// We modified the document
	GetIEditor()->SetModifiedFlag();

	OnGenerate();
}

void CTerrainLighting::OnTerrainShadows() 
{
	////////////////////////////////////////////////////////////////////////
	// Synchronize value in the document
	////////////////////////////////////////////////////////////////////////

	UpdateData(TRUE);

	GetIEditor()->GetDocument()->GetLighting()->bTerrainShadows = m_bTerrainShadows;

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Update the preview
	OnGenerate();
}


void CTerrainLighting::OnObjectShadows() 
{
	////////////////////////////////////////////////////////////////////////
	// Synchronize value in the document
	////////////////////////////////////////////////////////////////////////

	UpdateData(TRUE);

	GetIEditor()->GetDocument()->GetLighting()->bObjectShadows = m_bObjectShadows;

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Update the preview
	OnGenerate();
}


void CTerrainLighting::OnFileExport() 
{
	////////////////////////////////////////////////////////////////////////
	// Export the lighting settings
	////////////////////////////////////////////////////////////////////////
	
	char szFilters[] = "Light Settings (*.lgt)|*.lgt||";
	CString fileName;
	if (CFileUtil::SelectSaveFile( szFilters,"lgt",GetIEditor()->GetLevelFolder(),fileName ))
	{
		CLogFile::WriteLine("Exporting light settings...");

		// Write the light settings into the archive
		XmlNodeRef node = new CXmlNode( "LightSettings" );
		GetIEditor()->GetDocument()->GetLighting()->Serialize( node,false );
		node->saveToFile( fileName );
	}
}

void CTerrainLighting::OnFileImport() 
{
	////////////////////////////////////////////////////////////////////////
	// Import the lighting settings
	////////////////////////////////////////////////////////////////////////
	
	char szFilters[] = "Light Settings (*.lgt)|*.lgt||";
	CString fileName;

	if (CFileUtil::SelectFile( szFilters,GetIEditor()->GetLevelFolder(),fileName ))
	{
		CLogFile::WriteLine("Importing light settings...");

		XmlParser parser;
		XmlNodeRef node = parser.parse( fileName );
		GetIEditor()->GetDocument()->GetLighting()->Serialize( node,true );

		// We modified the document
		GetIEditor()->SetModifiedFlag();

		// Update the controls with the settings from the document
		UpdateControls();

		// Update the preview
		OnGenerate();
	}
}

void CTerrainLighting::UpdateControls()
{
	////////////////////////////////////////////////////////////////////////	
	// Update the controls with the settings from the document
	////////////////////////////////////////////////////////////////////////

	LightingSettings *ls = GetIEditor()->GetDocument()->GetLighting();
	m_sldAmbient = ls->iAmbient;
	m_bTerrainShadows = ls->bTerrainShadows;
	m_bObjectShadows = ls->bObjectShadows;
	m_optLightingAlgo = ls->eAlgo;
	m_sldSunDirection = ls->iSunRotation;
	m_sldSunHeight = ls->iSunHeight;
	m_sldShadowIntensity = ls->iShadowIntensity;
	m_sldShadowBlur=ls->iShadowBlur;
	m_sldSkyQuality = ls->iHemiSamplQuality;

	m_sunMult.SetValue( ls->sunMultiplier );

	switch (ls->eAlgo)
	{
		case eDP3:
			GetDlgItem(IDC_HEMISAMPLEQ)->EnableWindow(FALSE);
			GetDlgItem(IDC_SKY_COLOR)->EnableWindow(FALSE);
			GetDlgItem(IDC_AMBIENT)->EnableWindow(TRUE);
			break;
		case eHemisphere:
			GetDlgItem(IDC_HEMISAMPLEQ)->EnableWindow(FALSE);
			GetDlgItem(IDC_SKY_COLOR)->EnableWindow(TRUE);
			GetDlgItem(IDC_AMBIENT)->EnableWindow(FALSE);
			break;
		case ePrecise:
			GetDlgItem(IDC_HEMISAMPLEQ)->EnableWindow(TRUE);
			GetDlgItem(IDC_SKY_COLOR)->EnableWindow(TRUE);
			GetDlgItem(IDC_AMBIENT)->EnableWindow(FALSE);
			break;

		default: assert(0);
	}

	UpdateData(FALSE);
}

void CTerrainLighting::OnViewWithTexturing() 
{
	////////////////////////////////////////////////////////////////////////
	// Show a textured preview
	////////////////////////////////////////////////////////////////////////

	CTerrainTexture cTexture;

	cTexture.ExportLargePreview();
}

void CTerrainLighting::OnSunColor() 
{
	////////////////////////////////////////////////////////////////////////
	// Let the user choose a new sun color
	////////////////////////////////////////////////////////////////////////

	CCryEditDoc *doc = GetIEditor()->GetDocument();
	// Color selection dialog box
	COLORREF color = doc->GetLighting()->dwSunColor;
	if (GetIEditor()->SelectColor( color,this ))
	{
		// Save the new color value in the document
		doc->GetLighting()->dwSunColor = color;

		// We modified the document
		GetIEditor()->SetModifiedFlag();

		// Update the preview
		OnGenerate();
	}
}

void CTerrainLighting::OnSkyColor() 
{
	////////////////////////////////////////////////////////////////////////
	// Let the user choose a new sky color
	////////////////////////////////////////////////////////////////////////

	CCryEditDoc *doc = GetIEditor()->GetDocument();
	// Color selection dialog box
	COLORREF color = doc->GetLighting()->dwSkyColor;
	if (GetIEditor()->SelectColor( color,this ))
	{
		// Save the new color value in the document
		doc->GetLighting()->dwSkyColor = color;

		// We modified the document
		GetIEditor()->SetModifiedFlag();

		// Update the preview
		OnGenerate();
	}
}

void CTerrainLighting::OnHemisphere() 
{
	GetIEditor()->GetDocument()->GetLighting()->eAlgo = eHemisphere;
	GetIEditor()->SetModifiedFlag();
	UpdateControls();
	OnGenerate();
}

void CTerrainLighting::OnDp3() 
{
	GetIEditor()->GetDocument()->GetLighting()->eAlgo = eDP3;
	GetIEditor()->SetModifiedFlag();
	UpdateControls();
	OnGenerate();
}


void CTerrainLighting::OnBnClickedPreciselighting2()
{
	GetIEditor()->GetDocument()->GetLighting()->eAlgo = ePrecise;
	GetIEditor()->SetModifiedFlag();
	UpdateControls();
	OnGenerate();
}


BOOL CTerrainLighting::DestroyWindow() 
{
	return CDialog::DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainLighting::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDraggingSunDirection)
	{
		LightingSettings *ls = GetIEditor()->GetDocument()->GetLighting();
		CRect rcImage(19,24,19+LIGHTING_PREVIEW_RESOLUTION,24+LIGHTING_PREVIEW_RESOLUTION);
		CPoint center = rcImage.CenterPoint();
		Vec3 dir = Vec3(point.x - center.x,point.y-center.y,0 );
		dir.Normalize();
		Vec3 horizont = Vec3(0,-1,0);
		float cosa = dir.Dot(horizont);
		float alpha = RAD2DEG(acos(cosa));
		if (point.x > center.x)
			alpha = 360-alpha;
		ls->iSunRotation = alpha;
		UpdateControls();

		// Update the preview
		//::SetRect(&rcUpdate, 10, 10, 20 + LIGHTING_PREVIEW_RESOLUTION, 30 + LIGHTING_PREVIEW_RESOLUTION);
		InvalidateRect( &rcImage, FALSE);
	}

	CDialog::OnMouseMove(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainLighting::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CRect rcImage(19,24,19+LIGHTING_PREVIEW_RESOLUTION,24+LIGHTING_PREVIEW_RESOLUTION);
	if (rcImage.PtInRect(point))
	{
		m_bDraggingSunDirection = true;
	}

	CDialog::OnLButtonDown(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainLighting::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bDraggingSunDirection)
	{
		m_bDraggingSunDirection = false;
		OnGenerate();
	}

	CDialog::OnLButtonUp(nFlags, point);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainLighting::OnBnClickedTextured()
{
	UpdateData(TRUE);
	OnGenerate();
}

void CTerrainLighting::OnOK()
{
	CDialog::OnOK();

	CWaitCursor wait;
	// When exiting lighting dialog.
	GetIEditor()->GetGameEngine()->ReloadEnvironment();
}

void CTerrainLighting::OnCancel()
{
	LightingSettings *ls = GetIEditor()->GetDocument()->GetLighting();
	*ls = m_originalLightSettings;
	CDialog::OnCancel();
}
