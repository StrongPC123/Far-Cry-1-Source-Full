// TerrainTexture.cpp : implementation file
//

#include "stdafx.h"
#include "TerrainTexture.h"
#include "CryEditDoc.h"
#include "StringDlg.h"
#include "TerrainLighting.h"
#include "DimensionsDialog.h"
#include "NumberDlg.h"
#include "SurfaceTypesDialog.h"
#include "SurfaceType.h"
#include "Heightmap.h"
#include "Layer.h"
#include "VegetationMap.h"

#include "TerrainTexGen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Static member variables
CBitmap CTerrainTexture::m_bmpFinalTexPrev;
CDC CTerrainTexture::m_dcFinalTexPrev;
bool CTerrainTexture::m_bUseLighting = true;
bool CTerrainTexture::m_bShowWater = true;


/////////////////////////////////////////////////////////////////////////////
// CTerrainTexture dialog


//////////////////////////////////////////////////////////////////////////
CTerrainTexture::CTerrainTexture(CWnd* pParent /*=NULL*/)
	: CDialog(CTerrainTexture::IDD, pParent)
{
	static bool bFirstInstance = true;

	//{{AFX_DATA_INIT(CTerrainTexture)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// No current layer at first
	m_pCurrentLayer = NULL;

	// Create the bitmap and the DC for the texture preview
	if (!m_dcFinalTexPrev.m_hDC)
		m_dcFinalTexPrev.CreateCompatibleDC(NULL);
	if (!m_bmpFinalTexPrev.m_hObject)
		VERIFY(m_bmpFinalTexPrev.CreateBitmap(FINAL_TEX_PREVIEW_PRECISION_CX,
			FINAL_TEX_PREVIEW_PRECISION_CY, 1, 32, NULL));
	m_dcFinalTexPrev.SelectObject(&m_bmpFinalTexPrev);

	// Paint it gray
	if (bFirstInstance)
	{
		ClearTexturePreview();

		bFirstInstance = false; // Leave it the next time
	}

	m_bLayerTexClicked = false;
	m_bLayerTexSelected = false;

	m_dcLayerPreview.CreateCompatibleDC(NULL);
	// Create a new bitmap for the preview
	VERIFY(m_bmpLayerPreview.CreateBitmap(FINAL_TEX_PREVIEW_PRECISION_CX, FINAL_TEX_PREVIEW_PRECISION_CX, 1, 32, NULL));
	m_dcLayerPreview.SelectObject(&m_bmpLayerPreview);
	m_bMaskPreviewValid = false;
}

//////////////////////////////////////////////////////////////////////////
CTerrainTexture::~CTerrainTexture()
{
	m_dcLayerPreview.SelectObject( (CBitmap*)NULL );
}

void CTerrainTexture::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainTexture)
	DDX_Control(pDX, IDC_SURFACE_TYPE, m_surfaceType);
	DDX_Control(pDX, IDC_LAYER_LIST, m_lstLayers);
	DDX_Control(pDX, IDC_SEL_START, m_altStartBtn);
	DDX_Control(pDX, IDC_SEL_END, m_altEndBtn);
	DDX_Control(pDX, IDC_SLOPE_SEL_START, m_slopeStartBtn);
	DDX_Control(pDX, IDC_SLOPE_SEL_END, m_slopeEndBtn);
	DDX_Control(pDX, IDC_EDIT_SURFACETYPES, m_editSrurfaceTypesBtn);
	DDX_Control(pDX, IDC_LAYER_TEX_INFO, m_texInfo);
	DDX_Control(pDX, IDC_LOAD_TEXTURE, m_loadTextureBtn );
	DDX_Control(pDX, IDC_SMOOTH, m_smoothBtn );
	DDX_Control(pDX, IDC_LAYER_START_END, m_altSlider );
	DDX_Control(pDX, IDC_LAYER_SLOPE, m_slopeSlider );
	DDX_Control(pDX, IDC_SEL_START_END_NUM, m_altSliderPos );
	DDX_Control(pDX, IDC_SLOPE_SEL_MIN_MAX_NUM, m_slopeSliderPos );
	
	DDX_Control(pDX, IDC_EXPORT_MASK, m_exportMaskBtn );
	DDX_Control(pDX, IDC_LOAD_MASK, m_importMaskBtn );
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainTexture, CDialog)
	//{{AFX_MSG_MAP(CTerrainTexture)
	ON_LBN_SELCHANGE(IDC_LAYER_LIST, OnSelchangeLayerList)
	ON_BN_CLICKED(IDC_SEL_START, OnSelStart)
	ON_BN_CLICKED(IDC_SEL_END, OnSelEnd)
	ON_BN_CLICKED(IDC_LOAD_TEXTURE, OnLoadTexture)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_SLOPE_SEL_START, OnSlopeSelStart)
	ON_BN_CLICKED(IDC_SLOPE_SEL_END, OnSlopeSelEnd)
	ON_BN_CLICKED(IDC_IMPORT, OnImport)
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	ON_BN_CLICKED(ID_FILE_EXPORTLARGEPREVIEW, OnFileExportLargePreview)
	ON_COMMAND(ID_PREVIEW_APPLYLIGHTING, OnApplyLighting)
	ON_COMMAND(ID_LAYER_SETWATERLEVEL, OnSetWaterLevel)
	ON_COMMAND(ID_LAYER_EXPORTTEXTURE, OnLayerExportTexture)
	ON_WM_HSCROLL()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_TTS_HOLD, OnHold)
	ON_BN_CLICKED(IDC_TTS_FETCH, OnFetch)
	ON_BN_CLICKED(IDC_USE_LAYER, OnUseLayer)
	ON_COMMAND(ID_OPTIONS_SETLAYERBLENDING, OnOptionsSetLayerBlending)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_BN_CLICKED(IDC_AUTO_GEN_MASK, OnAutoGenMask)
	ON_BN_CLICKED(IDC_LOAD_MASK, OnLoadMask)
	ON_BN_CLICKED(IDC_EXPORT_MASK, OnExportMask)
	ON_COMMAND(ID_PREVIEW_SHOWWATER, OnShowWater)
	ON_BN_CLICKED(IDC_EDIT_SURFACETYPES, OnEditSurfaceTypes)
	ON_CBN_SELENDOK(IDC_SURFACE_TYPE, OnSelendokSurfaceType)
	ON_COMMAND(ID_LAYER_SETWATERCOLOR, OnLayerSetWaterColor)
	ON_COMMAND(ID_EDIT_SURFACETYPES, OnEditSurfaceTypes)
	//}}AFX_MSG_MAP
	ON_WM_DESTROY()

	ON_LBN_XT_LABELEDITEND(IDC_LAYER_LIST, OnLayersLabelEditEnd)
	ON_LBN_XT_LABELEDITCANCEL(IDC_LAYER_LIST, OnLayersLabelEditCancel)
	ON_LBN_XT_NEWITEM(IDC_LAYER_LIST, OnLayersNewItem)
	ON_LBN_XT_DELETEITEM(IDC_LAYER_LIST, OnLayersDeleteItem)
	ON_LBN_XT_MOVEITEMUP(IDC_LAYER_LIST, OnLayersMoveItemUp)
	ON_LBN_XT_MOVEITEMDOWN(IDC_LAYER_LIST, OnLayersMoveItemDown)
	ON_BN_CLICKED(IDC_SMOOTH, OnBnClickedSmooth)

	ON_EN_UPDATE( IDC_SLOPE_START,OnLayerValuesUpdate )
	ON_EN_UPDATE( IDC_SLOPE_END,OnLayerValuesUpdate )
	ON_EN_UPDATE( IDC_ALT_START,OnLayerValuesUpdate )
	ON_EN_UPDATE( IDC_ALT_END,OnLayerValuesUpdate )
	
	ON_EN_CHANGE( IDC_SLOPE_START,OnLayerValuesChange )
	ON_EN_CHANGE( IDC_SLOPE_END,OnLayerValuesChange )
	ON_EN_CHANGE( IDC_ALT_START,OnLayerValuesChange )
	ON_EN_CHANGE( IDC_ALT_END,OnLayerValuesChange )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerrainTexture message handlers

BOOL CTerrainTexture::OnInitDialog() 
{
	m_bLayerTexClicked =  false;
	m_bLayerTexSelected = false;

	m_pCurrentLayer = 0;
	m_doc = GetIEditor()->GetDocument();

	CSliderCtrl ctrlSlider;

	CDialog::OnInitDialog();

	CWaitCursor wait;

	m_lstLayers.SetListEditStyle( _T(" &Layers:"),LBS_XT_DEFAULT );
	
	// Update the menus
	GetMenu()->GetSubMenu(2)->CheckMenuItem(ID_PREVIEW_APPLYLIGHTING, 
		m_bUseLighting ? MF_CHECKED : MF_UNCHECKED);
	GetMenu()->GetSubMenu(2)->CheckMenuItem(ID_PREVIEW_SHOWWATER, 
		m_bShowWater ? MF_CHECKED : MF_UNCHECKED);
	
	// Set range for the layer attlitude and slope sliders
	m_altSlider.SetRange(0, 255, TRUE);
	m_slopeSlider.SetRange(0, 255, TRUE);

	m_altStart.Create( this,IDC_ALT_START );
	m_altEnd.Create( this,IDC_ALT_END );
	m_slopeStart.Create( this,IDC_SLOPE_START );
	m_slopeEnd.Create( this,IDC_SLOPE_END );

	m_altStart.SetRange( 0,255 );
	m_altEnd.SetRange( 0,255 );
	m_slopeStart.SetRange( 0,255 );
	m_slopeEnd.SetRange( 0,255 );

	m_surfaceType.ResetContent();
	m_surfaceType.AddString( "" );
	int i;
	for (i = 0; i < m_doc->GetSurfaceTypeCount(); i++)
	{
		m_surfaceType.AddString( m_doc->GetSurfaceType(i)->GetName() );
	}

	// Invalidate layer masks.
	for (i=0; i < m_doc->GetLayerCount(); i++)
	{
		// Add the name of the layer
		m_doc->GetLayer(i)->InvalidateMask();
	}

	// Load the layer list from the document
	ReloadLayerList();
	EnableControls();

	OnGeneratePreview();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTerrainTexture::ReloadLayerList()
{
	////////////////////////////////////////////////////////////////////////
	// Fill the layer list box with the data from the document
	////////////////////////////////////////////////////////////////////////

	unsigned int i;
	int iIndex;

	CLogFile::WriteLine("Retrieving layer data from document...");

	// Clear the listbox first
	m_lstLayers.ResetContent();

	// Add the layer objects
	for (i=0; i < m_doc->GetLayerCount(); i++)
	{
		// Add the name of the layer
		iIndex = m_lstLayers.AddString(LPCTSTR(m_doc->GetLayer(i)->GetLayerName()));

		ASSERT(iIndex != LB_ERR);

		CLayer *layer = m_doc->GetLayer(i);

		// Associate the pointer to the layer with the name
		m_lstLayers.SetItemData(iIndex, (DWORD_PTR)layer );
	}

	// We now have layers and some more options should be enabled
	EnableControls();
}

void CTerrainTexture::OnSelchangeLayerList()
{
	////////////////////////////////////////////////////////////////////////
	// Change the current layer as response to a listbox click
	////////////////////////////////////////////////////////////////////////
	
	CLayer *pLayer = NULL;
	
	// Get the layer associated with the selection
	int sel = m_lstLayers.GetCurSel();
	if (sel != LB_ERR)
		pLayer = (CLayer*)m_lstLayers.GetItemData(sel);

	// Unselect all layers.
	for (int i = 0; i < m_doc->GetLayerCount(); i++)
	{
		m_doc->GetLayer(i)->SetSelected(false);
	}

	// Set it as the current one
	m_pCurrentLayer = pLayer;

	if (m_pCurrentLayer)
		m_pCurrentLayer->SetSelected(true);

	m_bMaskPreviewValid = false;

	// Update the controls with the data from the layer
	UpdateControlData();
	
	// We now have a selected layer
	EnableControls();
}

void CTerrainTexture::EnableControls()
{
	////////////////////////////////////////////////////////////////////////
	// Enable / disable the current based of if at least one layer is
	// present and activated
	////////////////////////////////////////////////////////////////////////

	BOOL bEnable = m_pCurrentLayer && m_lstLayers.GetCurSel() != LB_ERR;

	// Enable layer related selections when we have an activated layer
	GetDlgItem(IDC_CAPTION1)->EnableWindow(bEnable);
	GetDlgItem(IDC_CAPTION2)->EnableWindow(bEnable);
	GetDlgItem(IDC_USE_LAYER)->EnableWindow(bEnable);
	GetDlgItem(IDC_AUTO_GEN_MASK)->EnableWindow(bEnable);

	m_loadTextureBtn.EnableWindow(bEnable);
	m_texInfo.EnableWindow(bEnable);

	m_editSrurfaceTypesBtn.EnableWindow(bEnable);
	m_loadTextureBtn.EnableWindow(bEnable);
	m_surfaceType.EnableWindow(bEnable);

	m_importMaskBtn.EnableWindow(bEnable);
	m_exportMaskBtn.EnableWindow(bEnable);

	GetMenu()->GetSubMenu(1)->EnableMenuItem(IDC_LOAD_TEXTURE, 
		bEnable ? MF_ENABLED : MF_GRAYED);
	GetMenu()->GetSubMenu(1)->EnableMenuItem(ID_LAYER_EXPORTTEXTURE, 
		bEnable ? MF_ENABLED : MF_GRAYED);
	GetMenu()->GetSubMenu(1)->EnableMenuItem(IDC_REMOVE_LAYER, 
		bEnable ? MF_ENABLED : MF_GRAYED);

	// Only enable export and generate preview option when we have layer(s)
	GetDlgItem(IDC_EXPORT)->EnableWindow(m_lstLayers.GetCount());
	GetMenu()->GetSubMenu(0)->EnableMenuItem(IDC_EXPORT, 
		m_lstLayers.GetCount() ? MF_ENABLED : MF_GRAYED);
	GetMenu()->GetSubMenu(2)->EnableMenuItem(IDC_GENERATE_PREVIEW,m_lstLayers.GetCount() ? MF_ENABLED : MF_GRAYED);
	GetMenu()->GetSubMenu(2)->EnableMenuItem(ID_FILE_EXPORTLARGEPREVIEW,
		m_lstLayers.GetCount() ? MF_ENABLED : MF_GRAYED);

	bEnable = false;
	if (m_pCurrentLayer)
	{
		bEnable = true;
		if (m_pCurrentLayer->IsAutoGen())
		{	
			m_importMaskBtn.EnableWindow( FALSE );
		}
		else
			bEnable = false;
		if (m_pCurrentLayer == m_doc->GetLayer(0))
		{
			bEnable = false;
		}
	}
	m_altStartBtn.EnableWindow(bEnable);
	m_altEndBtn.EnableWindow(bEnable);
	m_slopeStartBtn.EnableWindow(bEnable);
	m_slopeEndBtn.EnableWindow(bEnable);
	m_altSlider.EnableWindow(bEnable);
	m_slopeSlider.EnableWindow(bEnable);
	m_altStart.EnableWindow(bEnable);
	m_altEnd.EnableWindow(bEnable);
	m_slopeStart.EnableWindow(bEnable);
	m_slopeEnd.EnableWindow(bEnable);
	m_altSlider.EnableWindow(bEnable);
	m_slopeSlider.EnableWindow(bEnable);
	m_altSliderPos.EnableWindow(bEnable);
	m_slopeSliderPos.EnableWindow(bEnable);
	m_smoothBtn.EnableWindow(bEnable);
}

void CTerrainTexture::OnSelStart() 
{
	////////////////////////////////////////////////////////////////////////
	// Set the new start for the layer
	////////////////////////////////////////////////////////////////////////	
	if (!m_pCurrentLayer)
		return;
	
	m_pCurrentLayer->SetLayerStart(m_altSlider.GetPos());
	
	// Update he controls with the data from the current layer
	UpdateControlData();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Regenerate the preview
	OnGeneratePreview();
}

void CTerrainTexture::OnSelEnd() 
{
	////////////////////////////////////////////////////////////////////////
	// Set the new end for the layer
	////////////////////////////////////////////////////////////////////////	
	if (!m_pCurrentLayer)
		return;
	
	m_pCurrentLayer->SetLayerEnd(m_altSlider.GetPos());
	
	// Update he controls with the data from the current layer
	UpdateControlData();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Regenerate the preview
	OnGeneratePreview();
}

void CTerrainTexture::OnSlopeSelStart() 
{
	////////////////////////////////////////////////////////////////////////
	// Set the new slope minmum for the layer
	////////////////////////////////////////////////////////////////////////	
	if (!m_pCurrentLayer)
		return;
	
	m_pCurrentLayer->SetLayerMinSlope(m_slopeSlider.GetPos());
	
	// Update he controls with the data from the current layer
	UpdateControlData();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Regenerate the preview
	OnGeneratePreview();
}

void CTerrainTexture::OnSlopeSelEnd() 
{
	////////////////////////////////////////////////////////////////////////
	// Set the new slope maximum for the layer
	////////////////////////////////////////////////////////////////////////	
	if (!m_pCurrentLayer)
		return;
	
	m_pCurrentLayer->SetLayerMaxSlope(m_slopeSlider.GetPos());
	
	// Update he controls with the data from the current layer
	UpdateControlData();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Regenerate the preview
	OnGeneratePreview();
}

void CTerrainTexture::UpdateControlData()
{
	////////////////////////////////////////////////////////////////////////	
	// Update the controls with the data from the current layer
	////////////////////////////////////////////////////////////////////////	

	CSliderCtrl ctrlSlider;
	CButton ctrlButton;
	char szBuffer[256];
	RECT rc;

	CString maskInfoText;

	if (m_pCurrentLayer)
	{
		int maskRes = m_pCurrentLayer->GetMaskResolution();
		maskInfoText.Format( "(%dx%d)",maskRes,maskRes );

		// Layer range slider
		m_altSlider.SetSelection( m_pCurrentLayer->GetLayerStart(),m_pCurrentLayer->GetLayerEnd() );
		m_slopeSlider.SetSelection( m_pCurrentLayer->GetLayerMinSlope(),m_pCurrentLayer->GetLayerMaxSlope() );
		m_altSlider.Invalidate();
		m_slopeSlider.Invalidate();

		// Texture information
		if (m_pCurrentLayer->GetTextureFilename().GetLength())
			sprintf(szBuffer, "%s\n%i x %i", LPCTSTR(m_pCurrentLayer->GetTextureFilename()), 
				m_pCurrentLayer->GetTextureWidth(), m_pCurrentLayer->GetTextureHeight());
		else
			szBuffer[0] = '\0';
		m_texInfo.SetWindowText(szBuffer);

		// Use layer check box
		VERIFY(ctrlButton.Attach(GetDlgItem(IDC_USE_LAYER)->m_hWnd));
		ctrlButton.SetCheck(m_pCurrentLayer->IsInUse());
		ctrlButton.Detach();

		VERIFY(ctrlButton.Attach(GetDlgItem(IDC_AUTO_GEN_MASK)->m_hWnd));
		ctrlButton.SetCheck(m_pCurrentLayer->IsAutoGen());
		ctrlButton.Detach();

		if (m_surfaceType.SelectString( -1,m_pCurrentLayer->GetSurfaceType()) == LB_ERR)
			m_surfaceType.SetCurSel(0);

		m_smoothBtn.SetCheck( m_pCurrentLayer->IsSmooth()?BST_CHECKED:BST_UNCHECKED );

		m_slopeStart.SetValue( m_pCurrentLayer->GetLayerMinSlope() );
		m_slopeEnd.SetValue( m_pCurrentLayer->GetLayerMaxSlope() );
		m_altStart.SetValue( m_pCurrentLayer->GetLayerStart() );
		m_altEnd.SetValue( m_pCurrentLayer->GetLayerEnd() );
	}
	else
	{
		m_surfaceType.SetCurSel(0);
		m_texInfo.SetWindowText( _T("No Texture") );
	}

	// Update the controls
	EnableControls();

	GetDlgItem(IDC_INFO_TEXT)->SetWindowText( maskInfoText );

	// Redraw the window
	GetDlgItem(IDC_LAYER_MASK_PREVIEW)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	InvalidateRect(&rc);
	GetDlgItem(IDC_LAYER_TEX_PREVIEW)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	InvalidateRect(&rc);
}

void CTerrainTexture::OnLoadTexture() 
{
	if (!m_pCurrentLayer)
		return;
	////////////////////////////////////////////////////////////////////////
	// Load a texture from a BMP file
	////////////////////////////////////////////////////////////////////////
	CString file;
	bool res = CFileUtil::SelectSingleFile( EFILE_TYPE_TEXTURE,file );

	/*
	char szFilters[] = "Bitmaps (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0";
	CFileDialog dlg(TRUE, "bmp", "*.bmp", OFN_HIDEREADONLY|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR, szFilters);
	RECT rc;
*/
	if (res) 
	{
		// Load the texture
		if (!m_pCurrentLayer->LoadTexture( file ))
			AfxMessageBox("Error while loading the texture !");

		// Update the texture preview
		CRect rc;
		GetDlgItem(IDC_LAYER_TEX_PREVIEW)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		InvalidateRect(&rc);

		// Update the texture information files
		UpdateControlData();

		// We modified the document
		GetIEditor()->SetModifiedFlag();
	}

	// Regenerate the preview
	OnGeneratePreview();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnPaint() 
{
	////////////////////////////////////////////////////////////////////////
	// Draw the previews
	////////////////////////////////////////////////////////////////////////

	RECT rcTex, rcClient;
	CPaintDC dc(this); // device context for painting
	CPen cGrayPen(PS_SOLID, 1, 0x007F7F7F);
	CPen cWhitePen(PS_SOLID, 1, 0x00FFFFFF);

	// Get the rect for the layer texture preview
	GetDlgItem(IDC_LAYER_TEX_PREVIEW)->GetWindowRect(&rcTex);
	ScreenToClient(&rcTex);

	// Draw the preview of the texture
	if (!m_bLayerTexSelected)
		dc.DrawFrameControl(&rcTex, DFC_BUTTON, DFCS_BUTTONPUSH);
	else
		dc.DrawFrameControl(&rcTex, DFC_BUTTON, DFCS_BUTTONPUSH|DFCS_PUSHED);

	if (m_pCurrentLayer)
	{
		rcTex.left += 2;
		rcTex.top += 2;
		rcTex.right -= 2;
		rcTex.bottom -= 2;
		if (m_bLayerTexSelected)
		{
			rcTex.left += 2;
			rcTex.top += 2;
			//rcTex.right += 2;
			//rcTex.bottom += 2;
		}
		m_pCurrentLayer->DrawLayerTexturePreview(&rcTex, &dc);
	}

	// Get the rect for the layer mask preview
	GetDlgItem(IDC_LAYER_MASK_PREVIEW)->GetWindowRect(&rcTex);
	ScreenToClient(&rcTex);

	// Draw the mask preview
	dc.DrawFrameControl(&rcTex, DFC_BUTTON, DFCS_FLAT | DFCS_BUTTONPUSH);
	
	rcTex.left += 2;
	rcTex.top += 2;
	rcTex.right -= 2;
	rcTex.bottom -= 2;
	DrawLayerPreview(&rcTex, &dc);

	// Draw the final texture preview in all cases
	GetDlgItem(IDC_FINAL_TEX_PREVIEW)->GetWindowRect(&rcTex);
	ScreenToClient(&rcTex);
	dc.DrawFrameControl(&rcTex, DFC_BUTTON, DFCS_FLAT | DFCS_BUTTONPUSH);
	rcTex.left += 2;
	rcTex.top += 2; 
	rcTex.right -= 2;
	rcTex.bottom -= 2;
	dc.SetStretchBltMode(HALFTONE);
	dc.StretchBlt(rcTex.left, rcTex.top, rcTex.right - rcTex.left, 
		rcTex.bottom - rcTex.top, &m_dcFinalTexPrev, 0, 0,
		FINAL_TEX_PREVIEW_PRECISION_CX, FINAL_TEX_PREVIEW_PRECISION_CY, 
		SRCCOPY);
 
	// Draw a menu bar separator
	GetClientRect(&rcClient);
	dc.SelectObject(&cGrayPen);
	dc.MoveTo(0, 0);
	dc.LineTo(rcClient.right, 0);
	dc.SelectObject(&cWhitePen);
	dc.MoveTo(0, 1);
	dc.LineTo(rcClient.right, 1);
	
	// Do not call CDialog::OnPaint() for painting messages
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::DrawLayerPreview( LPRECT rcPos, CDC *pDC )
{
	////////////////////////////////////////////////////////////////////////
	// Draw the mask layer preview
	////////////////////////////////////////////////////////////////////////
	if (!m_pCurrentLayer)
		return;

	if (!m_bMaskPreviewValid)
	{
		CByteImage *mask = m_texGen.GetLayerMask( m_pCurrentLayer );
		if (mask && mask->IsValid())
		{
			// Mark as valid.
			m_bMaskPreviewValid = true;
			
			int w = FINAL_TEX_PREVIEW_PRECISION_CX;
			unsigned char *pLayerData = mask->GetData();

			CImage previewData;
			previewData.Allocate( w,w );
			DWORD *pData = (DWORD*)previewData.GetData();
			DWORD iColor;
			// Generate the data for the preview image
			for (int i = 0; i < w*w; i++)
			{
				// Get the data from the real layer mask and write it to the preview bitmap
				iColor = *pLayerData++;
				*pData++ = RGB(iColor,iColor,iColor);
			}
			// Load the preview image into the bitmap		
			m_bmpLayerPreview.SetBitmapBits( w*w*sizeof(DWORD), previewData.GetData() );
		}
	}

	ASSERT(rcPos);
	ASSERT(pDC);
	CBrush brshGray;

	if (m_bmpLayerPreview.m_hObject && m_bMaskPreviewValid)
	{
		pDC->SetStretchBltMode(HALFTONE);
		pDC->StretchBlt(rcPos->left, rcPos->top, rcPos->right - rcPos->left, 
			rcPos->bottom - rcPos->top, &m_dcLayerPreview, 0, 0,
			FINAL_TEX_PREVIEW_PRECISION_CX, FINAL_TEX_PREVIEW_PRECISION_CX, SRCCOPY);
	}
	else
	{
		brshGray.CreateSysColorBrush(COLOR_BTNFACE);
		pDC->FillRect(rcPos, &brshGray);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnGeneratePreview()
{
	////////////////////////////////////////////////////////////////////////
	// Generate all layer mask and create a preview of the texture
	////////////////////////////////////////////////////////////////////////
	RECT rcTex;

	// Allocate the memory for the texture
	CImage preview;
	preview.Allocate( FINAL_TEX_PREVIEW_PRECISION_CX,FINAL_TEX_PREVIEW_PRECISION_CY );

	// Generate the surface texture
	int tflags = ETTG_ABGR|ETTG_KEEP_LAYERMASKS|ETTG_USE_LIGHTMAPS;
	if (m_bUseLighting)
		tflags |= ETTG_LIGHTING;
	if (m_bShowWater)
		tflags |= ETTG_SHOW_WATER;

	m_texGen.GenerateSurfaceTexture( ETTG_INVALIDATE_LAYERS|tflags,preview );

	m_bMaskPreviewValid = false;

	// Write the texture data into the bitmap
	m_bmpFinalTexPrev.SetBitmapBits( preview.GetSize(),(DWORD*)preview.GetData() );

	// Redraw the window
	GetDlgItem(IDC_FINAL_TEX_PREVIEW)->GetWindowRect(&rcTex);
	ScreenToClient(&rcTex);
	InvalidateRect(&rcTex);
	GetDlgItem(IDC_LAYER_MASK_PREVIEW)->GetWindowRect(&rcTex);
	ScreenToClient(&rcTex);
	InvalidateRect(&rcTex);
}

void CTerrainTexture::ExportLargePreview()
{
	////////////////////////////////////////////////////////////////////////
	// Show a large preview of the final texture
	////////////////////////////////////////////////////////////////////////
	
	bool bReturn;
	CDimensionsDialog cDialog;
	
	// 1024x1024 is default
	cDialog.SetDimensions(1024);

	// Query the size of the preview
	if (cDialog.DoModal() == IDCANCEL)
		return;

	CLogFile::FormatLine("Exporting large surface texture preview (%ix%i)...", 
		cDialog.GetDimensions(), cDialog.GetDimensions());

	// Allocate the memory for the texture
	CImage image;
	if (!image.Allocate( cDialog.GetDimensions(),cDialog.GetDimensions() ))
		return;

	// Generate the surface texture
	// Generate the surface texture
	int tflags = ETTG_INVALIDATE_LAYERS|ETTG_STATOBJ_SHADOWS;
	if (m_bUseLighting)
		tflags |= ETTG_LIGHTING;
	if (m_bShowWater)
		tflags |= ETTG_SHOW_WATER;

	CTerrainTexGen texGen;
	bReturn = texGen.GenerateSurfaceTexture( tflags,image );
 
	if (!bReturn)
	{
		CLogFile::WriteLine("Error while generating surface texture preview");
		AfxMessageBox("Can't generate preview !");
	}
	else
	{
		GetIEditor()->SetStatusText("Saving preview...");
		
		/*
		int w = cDialog.GetDimensions();
		for (int y = 0; y < w; y++)
		{
			for (int x = 0; x < w; x++)
			{
				//pSurface[x+y*w] = pSurface[x+y*w] & 0xFFFFFF00;
			}
		}
		*/

		// Save the texture to disk
		CFileUtil::CreateDirectory( "Temp" );
		bReturn = CImageUtil::SaveImage( "Temp\\TexturePreview.bmp", image );

		if (!bReturn)
			AfxMessageBox("Can't save preview bitmap !");
	}

	if (bReturn)
	{
		CString dir = Path::AddBackslash(GetIEditor()->GetMasterCDFolder()) + "Temp\\";
		// Show the texture
		::ShellExecute(::GetActiveWindow(), "open", "TexturePreview.bmp","",dir, SW_SHOWMAXIMIZED);
	}

	// Reset the status text
	GetIEditor()->SetStatusText("Ready");
}

void CTerrainTexture::OnFileExportLargePreview()
{
	////////////////////////////////////////////////////////////////////////
	// Show a large preview of the final texture
	////////////////////////////////////////////////////////////////////////

	ExportLargePreview();
}

bool CTerrainTexture::GenerateSurface(DWORD *pSurface, UINT iWidth, UINT iHeight, int flags,CBitArray *pLightingBits, float **ppHeightmapData)
{
	////////////////////////////////////////////////////////////////////////
	// Generate the surface texture with the current layer and lighting
	// configuration and write the result to pSurface. Also give out the
	// results of the terrain lighting if pLightingBit is not NULL. Also,
	// if ppHeightmapData is not NULL, the allocated heightmap data will
	// be stored there instead destroing it at the end of the function
	////////////////////////////////////////////////////////////////////////
	bool bUseLighting = flags & GEN_USE_LIGHTING;
	bool bShowWater = flags & GEN_SHOW_WATER;
	bool bShowWaterColor = flags & GEN_SHOW_WATERCOLOR;
  bool bConvertToABGR = flags & GEN_ABGR;
	bool bCalcStatObjShadows = flags & GEN_STATOBJ_SHADOWS;
	bool bKeepLayerMasks = flags & GEN_KEEP_LAYERMASKS;

	CHeightmap *heightmap = GetIEditor()->GetHeightmap();
	float waterLevel = heightmap->GetWaterLevel();

	uint i, iTexX, iTexY;
	char szStatusBuffer[128];
	bool bGenPreviewTexture = true;
	COLORREF crlfNewCol;
	CBrush brshBlack(BLACK_BRUSH);
 	int iBlend;
	CTerrainLighting cLighting;
	DWORD *pTextureDataWrite = NULL;
	CLayer *pLayerShortcut = NULL;
	int iFirstUsedLayer;
	float *pHeightmapData = NULL;

	ASSERT(iWidth);
	ASSERT(iHeight);
	ASSERT(!IsBadWritePtr(pSurface, iWidth * iHeight * sizeof(DWORD)));

	if (iWidth == 0 || iHeight == 0)
		return false;

	m_doc = GetIEditor()->GetDocument();


	// Display an hourglass cursor
	BeginWaitCursor();

	CLogFile::WriteLine("Generating texture surface...");

	////////////////////////////////////////////////////////////////////////
	// Search for the first layer that is used
	////////////////////////////////////////////////////////////////////////

	iFirstUsedLayer = -1;

	for (i=0; i < m_doc->GetLayerCount(); i++)
	{
		// Have we founf the first used layer ?
		if (m_doc->GetLayer(i)->IsInUse())
		{
			iFirstUsedLayer = i;
			break;
		}
	}

	// Abort if there is no used layer
	if (iFirstUsedLayer == -1)
		return false;

	////////////////////////////////////////////////////////////////////////
	// Generate the layer masks
	////////////////////////////////////////////////////////////////////////
	
	// Status message
	GetIEditor()->SetStatusText("Scaling heightmap...");
		
	// Allocate memory for the heightmap data
	pHeightmapData = new float[iWidth * iHeight];
	assert(pHeightmapData);
	
	// Retrieve the heightmap data
	m_doc->m_cHeightmap.GetDataEx(pHeightmapData, iWidth, true, true);

	int t0 = GetTickCount();

	bool bProgress = iWidth >= 1024;

	CWaitProgress wait( "Blending Layers",bProgress );

	CLayer *tempWaterLayer = 0;
	if (bShowWater)
	{
		// Apply water level.
		// Add a temporary water layer to the list
		tempWaterLayer = new CLayer;
		//water->LoadTexture(MAKEINTRESOURCE(IDB_WATER), 128, 128);
		tempWaterLayer->FillWithColor( m_doc->GetWaterColor(),8,8 );
		tempWaterLayer->GenerateWaterLayer16(pHeightmapData,iWidth, iHeight, waterLevel );
		m_doc->AddLayer( tempWaterLayer );
	}

	CByteImage layerMask;

	////////////////////////////////////////////////////////////////////////
	// Generate the masks and the texture.
	////////////////////////////////////////////////////////////////////////
	int numLayers = m_doc->GetLayerCount();
	for (i=iFirstUsedLayer; i<(int) numLayers; i++)
	{
		CLayer *layer = m_doc->GetLayer(i);

		// Skip the layer if it is not in use
		if (!layer->IsInUse())
			continue;

		if (!layer->HasTexture())
			continue;

		if (bProgress)
		{
			wait.Step( i*100/numLayers );
		}

		// Status message
		sprintf(szStatusBuffer, "Updating layer %i of %i...", i + 1, m_doc->GetLayerCount());
		GetIEditor()->SetStatusText(szStatusBuffer);

		// Cache surface texture in.
		layer->PrecacheTexture();
		
		// Generate the mask for the current layer
		if (i != iFirstUsedLayer)
		{
			CFloatImage hmap;
			hmap.Attach( pHeightmapData,iWidth,iHeight );
			// Generate a normal layer from the user's parameters, stream from disk if it exceeds a given size
			if (!layer->UpdateMask( hmap,layerMask ))
				continue;
		}
		sprintf(szStatusBuffer, "Blending layer %i of %i...", i + 1, m_doc->GetLayerCount());
		GetIEditor()->SetStatusText(szStatusBuffer);

		// Set the write pointer (will be incremented) for the surface data
		DWORD *pTex = pSurface;

		uint layerWidth = layer->GetTextureWidth();
		uint layerHeight = layer->GetTextureHeight();

		if (i == iFirstUsedLayer)
		{
			// Draw the first layer, without layer mask.
			for (iTexY=0; iTexY < iHeight; iTexY++)
			{
				uint layerY = iTexY % layerHeight;
				for (iTexX=0; iTexX < iWidth; iTexX++)
				{
					// Get the color of the tiling texture at this position
					*pTex++ = layer->GetTexturePixel( iTexX % layerWidth,layerY );
				}
			}
		}
		else
		{
			// Draw the current layer with layer mask.
			for (iTexY=0; iTexY < iHeight; iTexY++)
			{
				uint layerY = iTexY % layerHeight;
				for (iTexX=0; iTexX < iWidth; iTexX++)
				{
					// Scale the current preview coordinate to the layer mask and get the value.
					iBlend = layerMask.ValueAt(iTexX,iTexY);
					// Check if this pixel should be drawn.
					if (iBlend == 0)
					{
						pTex++;
						continue;
					}
					
					// Get the color of the tiling texture at this position
					crlfNewCol = layer->GetTexturePixel( iTexX % layerWidth,layerY );
					
					// Just overdraw when the opaqueness of the new layer is maximum
					if (iBlend == 255)
					{
						*pTex = crlfNewCol;
					}
					else
					{
						// Blend the layer into the existing color, iBlend is the blend factor taken from the layer
						*pTex =  (((255 - iBlend) * (*pTex & 0x000000FF)	+  (crlfNewCol & 0x000000FF)        * iBlend) >> 8)      |
							((((255 - iBlend) * (*pTex & 0x0000FF00) >>  8) + ((crlfNewCol & 0x0000FF00) >>  8) * iBlend) >> 8) << 8 |
							((((255 - iBlend) * (*pTex & 0x00FF0000) >> 16) + ((crlfNewCol & 0x00FF0000) >> 16) * iBlend) >> 8) << 16;
					}
					pTex++;
				}
			}
		}

		if (!bKeepLayerMasks)
		{
			layer->ReleaseMask();
		}
	}

	if (tempWaterLayer)
	{
		m_doc->RemoveLayer(tempWaterLayer);
	}

	int t1 = GetTickCount();
	CLogFile::FormatLine( "Texture surface layers blended in %dms",t1-t0 );

	if (bProgress)
		wait.Stop();

//	CString str;
//	str.Format( "Time %dms",t1-t0 );
//	MessageBox( str,"Time",MB_OK );

	////////////////////////////////////////////////////////////////////////
	// Light the texture
	////////////////////////////////////////////////////////////////////////
	
	if (bUseLighting)
	{
		CByteImage *shadowMap = 0;
		if (bCalcStatObjShadows)
		{
			CLogFile::WriteLine("Generating shadows of static objects..." );
			GetIEditor()->SetStatusText( "Generating shadows of static objects..." );
			shadowMap = new CByteImage;
			if (!shadowMap->Allocate( iWidth,iHeight ))
			{
				delete shadowMap;
				return false;
			}
			shadowMap->Clear();
			float shadowAmmount = 255.0f*m_doc->GetLighting()->iShadowIntensity/100.0f;
			Vec3 sunVector = m_doc->GetLighting()->GetSunVector();

			GetIEditor()->GetVegetationMap()->GenerateShadowMap( *shadowMap,shadowAmmount,sunVector );
		}
		
		CLogFile::WriteLine("Generating Terrain Lighting..." );
		GetIEditor()->SetStatusText( "Generating Terrain Lighting..." );


		/*
		// Calculate the lighting. Fucntion will also use pLightingBits if present
		cLighting.LightArray16(iWidth, iHeight, pSurface, pHeightmapData, 
			m_doc->GetLighting(), pLightingBits,shadowMap );
		*/

		if (shadowMap)
			delete shadowMap;

		int t2 = GetTickCount();
		CLogFile::FormatLine( "Texture lighted in %dms",t2-t1 );
	}

	// After lighting add Colored Water layer.
	if (bShowWaterColor)
	{
		// Apply water level.
		// Add a temporary water layer to the list
		CLayer *water = new CLayer;
		//water->LoadTexture(MAKEINTRESOURCE(IDB_WATER), 128, 128);
		water->FillWithColor( m_doc->GetWaterColor(),128,128 );
		water->GenerateWaterLayer16(pHeightmapData,iWidth, iHeight, waterLevel );

		// Set the write pointer (will be incremented) for the surface data
		DWORD *pTex = pSurface;
		
		uint layerWidth = water->GetTextureWidth();
		uint layerHeight = water->GetTextureHeight();
		
		// Draw the first layer, without layer mask.
		for (iTexY=0; iTexY < iHeight; iTexY++)
		{
			uint layerY = iTexY % layerHeight;
			for (iTexX=0; iTexX < iWidth; iTexX++)
			{
				// Get the color of the tiling texture at this position
				if (water->GetLayerMaskPoint(iTexX,iTexY) > 0)
				{
					*pTex = water->GetTexturePixel( iTexX % layerWidth,layerY );
				}
				pTex++;
			}
		}
		delete water;
	}


	if (bConvertToABGR)
	{
		GetIEditor()->SetStatusText( "Convert surface texture to ABGR..." );
		// Set the write pointer (will be incremented) for the surface data
		pTextureDataWrite = pSurface;
		for (iTexY=0; iTexY<(int) iHeight; iTexY++)
		{
			for (iTexX=0; iTexX<(int) iWidth; iTexX++)
			{
				*pTextureDataWrite++ = ((* pTextureDataWrite & 0x00FF0000) >> 16) |
																(* pTextureDataWrite & 0x0000FF00) |
																((* pTextureDataWrite & 0x000000FF) << 16);
			}
		}
	}


	////////////////////////////////////////////////////////////////////////
	// Finished
	////////////////////////////////////////////////////////////////////////

	// Should we return or free the heightmap data ?
	if (ppHeightmapData)
	{
		*ppHeightmapData = pHeightmapData;
		pHeightmapData = NULL;
	}
	else
	{
		// Free the heightmap data
		delete [] pHeightmapData;
		pHeightmapData = NULL;
	}

	// We are finished with the calculations
	EndWaitCursor();
	GetIEditor()->SetStatusText("Ready");

	int t2 = GetTickCount();
	CLogFile::FormatLine( "Texture surface generate in %dms",t2-t0 );

	return true;
}

void CTerrainTexture::OnImport() 
{
	////////////////////////////////////////////////////////////////////////
	// Import layer settings from a file
	////////////////////////////////////////////////////////////////////////

	char szFilters[] = "Layer Files (*.lay)|*.lay||";
	CFileDialog dlg(TRUE, "lay", "*.lay", OFN_EXPLORER|OFN_ENABLESIZING|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR, szFilters);
	CFile cFile;

	if (dlg.DoModal() == IDOK) 
	{
		CLogFile::FormatLine("Importing layer settings from %s", dlg.GetPathName().GetBuffer(0));
		
		CXmlArchive ar;
		ar.Load( dlg.GetPathName() );
		GetIEditor()->GetDocument()->SerializeLayerSettings(ar);
		GetIEditor()->GetDocument()->SerializeSurfaceTypes(ar);

		// We modified the document
		GetIEditor()->SetModifiedFlag();

		// Load the layers into the dialog
		ReloadLayerList();
	}
}

void CTerrainTexture::OnExport() 
{
	////////////////////////////////////////////////////////////////////////
	// Export layer settings to a file
	////////////////////////////////////////////////////////////////////////

	char szFilters[] = "Layer Files (*.lay)|*.lay||";
	CFileDialog dlg(FALSE, "lay", "*.lay", OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR, szFilters);
	CFile cFile;

	if (dlg.DoModal() == IDOK) 
	{
		CLogFile::FormatLine("Exporting layer settings to %s", dlg.GetPathName().GetBuffer(0));

		CXmlArchive ar( "LayerSettings" );
		GetIEditor()->GetDocument()->SerializeSurfaceTypes(ar);
		GetIEditor()->GetDocument()->SerializeLayerSettings(ar);
		ar.Save( dlg.GetPathName() );
	}
}

void CTerrainTexture::OnApplyLighting()
{
	////////////////////////////////////////////////////////////////////////
	// Toggle between the on / off for the apply lighting state
	////////////////////////////////////////////////////////////////////////

	m_bUseLighting = m_bUseLighting ? false : true;
	GetMenu()->GetSubMenu(2)->CheckMenuItem(ID_PREVIEW_APPLYLIGHTING, 
		m_bUseLighting ? MF_CHECKED : MF_UNCHECKED);

	OnGeneratePreview();
}

void CTerrainTexture::OnShowWater()
{
	////////////////////////////////////////////////////////////////////////
	// Toggle between the on / off for the show water state
	////////////////////////////////////////////////////////////////////////

	m_bShowWater = m_bShowWater ? false : true;
	GetMenu()->GetSubMenu(2)->CheckMenuItem(ID_PREVIEW_SHOWWATER, 
		m_bShowWater ? MF_CHECKED : MF_UNCHECKED);

	OnGeneratePreview();
}

void CTerrainTexture::OnSetWaterLevel()
{
	////////////////////////////////////////////////////////////////////////
	// Let the user change the current water level
	////////////////////////////////////////////////////////////////////////

	// the dialog
	CNumberDlg cDialog( this,GetIEditor()->GetHeightmap()->GetWaterLevel(),"Set Water Level" );

	// Show the dialog
	if (cDialog.DoModal() == IDOK)
	{
		// Retrive the new water level from the dialog and save it in the document
		GetIEditor()->GetHeightmap()->SetWaterLevel(cDialog.GetValue());

		// We modified the document
		GetIEditor()->SetModifiedFlag();

		OnGeneratePreview();
	}
}

void CTerrainTexture::OnOptionsSetLayerBlending() 
{
	////////////////////////////////////////////////////////////////////////
	// Let the user change the current layer blending factor
	////////////////////////////////////////////////////////////////////////

	/*
	// Get the layer blending factor from the document and set it as default 
	// into the dialog
	CNumberDlg cDialog( this,m_doc->GetTerrainLayerBlendingFactor(),"Set Layer Blending" );

	// Show the dialog
	if (cDialog.DoModal() == IDOK)
	{
		// Retrieve the new layer blending factor from the dialog and
		// save it in the document
		m_doc->SetTerrainLayerBlendingFactor( cDialog.GetValue() );

		// We modified the document
		m_doc->SetModifiedFlag();

		OnGeneratePreview();
	}
	*/
}

void CTerrainTexture::OnLayerExportTexture() 
{
	////////////////////////////////////////////////////////////////////////
	// Export the texture data, which is associated with the current layer,
	// to a bitmap file
	////////////////////////////////////////////////////////////////////////
	if (!m_pCurrentLayer)
		return;

	char szFilters[] = SUPPORTED_IMAGES_FILTER_SAVE;
	CFileDialog dlg(FALSE, "bmp",NULL, OFN_EXPLORER|OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR, szFilters);
	CFile cFile;

	// Does the current layer have texture data ?
	if (!m_pCurrentLayer->HasTexture())
	{
		AfxMessageBox("Current layer does no have a texture, can't export !");
		return;
	}

	if (dlg.DoModal() == IDOK) 
	{
		BeginWaitCursor();
		// Tell the layer to export its texture
		m_pCurrentLayer->ExportTexture( dlg.GetPathName() );
		EndWaitCursor();
	}
}

void CTerrainTexture::ClearTexturePreview()
{
	////////////////////////////////////////////////////////////////////////
	// Paint the texture preview gray
	////////////////////////////////////////////////////////////////////////

	CBrush brshGray;
	RECT rcPos;

	// Make sure the preview has been created
	if (m_dcFinalTexPrev.m_hDC)
	{
		::SetRect(&rcPos, 0, 0, FINAL_TEX_PREVIEW_PRECISION_CX,
			FINAL_TEX_PREVIEW_PRECISION_CY);

		brshGray.CreateSysColorBrush(COLOR_BTNFACE);

		m_dcFinalTexPrev.FillRect(&rcPos, &brshGray);
	}
}

void CTerrainTexture::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	////////////////////////////////////////////////////////////////////////
	// Update the current position value in the slider's static fields
	////////////////////////////////////////////////////////////////////////
	if (!m_pCurrentLayer)
		return;

	CSliderCtrl ctrlSlider;
	char str[256];
	
	sprintf( str,"%d",m_altSlider.GetPos() );
	m_altSliderPos.SetWindowText(str);

	sprintf( str,"%d",m_slopeSlider.GetPos() );
	m_slopeSliderPos.SetWindowText(str);

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

HBRUSH CTerrainTexture::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	////////////////////////////////////////////////////////////////////////
	// The list box should be drawn with a different font
	////////////////////////////////////////////////////////////////////////

	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	/*
	CFont cLstBoxFont;
	if (pWnd->m_hWnd == GetDlgItem(IDC_LAYER_LIST)->m_hWnd)
	{
		VERIFY(cLstBoxFont.CreatePointFont(60, "Terminal"));
		pDC->SelectObject(&cLstBoxFont);
	}
	*/
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

void CTerrainTexture::OnHold() 
{
	////////////////////////////////////////////////////////////////////////
	// Make a temporary save of the current layer state
	////////////////////////////////////////////////////////////////////////

	CFile cFile;

	CFileUtil::CreateDirectory( "Temp" );
	CXmlArchive ar( "LayerSettings" );
	GetIEditor()->GetDocument()->SerializeLayerSettings(ar);
	ar.Save( HOLD_FETCH_FILE_TTS );
}

void CTerrainTexture::OnFetch() 
{
	////////////////////////////////////////////////////////////////////////
	// Load a previous save of the layer state
	////////////////////////////////////////////////////////////////////////

	CFile cFile;

	if (!PathFileExists(HOLD_FETCH_FILE_TTS))
	{
		AfxMessageBox("You have to use 'Hold' before using 'Fetch' !");
		return;
	}

	// Does the document contain unsaved data ?
	if (m_doc->IsModified())
	{
		if (AfxMessageBox("The document contains unsaved data, really fetch old state ?", 
			MB_ICONQUESTION | MB_YESNO, NULL) != IDYES)
		{
			return;
		}
	}

	CXmlArchive ar;
	ar.Load( HOLD_FETCH_FILE_TTS );
	GetIEditor()->GetDocument()->SerializeLayerSettings(ar);

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Load the layers into the dialog
	ReloadLayerList();
}

void CTerrainTexture::OnUseLayer() 
{
	////////////////////////////////////////////////////////////////////////
	// Click on the 'Use' checkbox of the current layer
	////////////////////////////////////////////////////////////////////////
	if (!m_pCurrentLayer)
		return;

	CButton ctrlButton;

	ASSERT(!IsBadReadPtr(m_pCurrentLayer, sizeof(CLayer)));

	// Change the internal in use value of the selected layer
	VERIFY(ctrlButton.Attach(GetDlgItem(IDC_USE_LAYER)->m_hWnd));
	m_pCurrentLayer->SetInUse((ctrlButton.GetCheck() == 1) ? true : false);
	ctrlButton.Detach();

	// Regenerate the preview
	OnGeneratePreview();
}

void CTerrainTexture::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CRect rc;
	GetDlgItem(IDC_LAYER_TEX_PREVIEW)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	if (rc.PtInRect(point) && m_bLayerTexClicked)
	{
		OnLoadTexture();
		InvalidateRect(&rc);
	}
	m_bLayerTexClicked = false;
	if (m_bLayerTexSelected)
	{
		m_bLayerTexSelected = false;
		InvalidateRect(&rc);
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CTerrainTexture::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rc;
	GetDlgItem(IDC_LAYER_TEX_PREVIEW)->GetWindowRect(&rc);
	ScreenToClient(&rc);
	if (rc.PtInRect(point))
	{
		m_bLayerTexClicked = true;
		m_bLayerTexSelected = true;
		InvalidateRect(&rc);
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CTerrainTexture::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bLayerTexClicked)
	{
		CRect rc;
		GetDlgItem(IDC_LAYER_TEX_PREVIEW)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		if (!rc.PtInRect(point) && m_bLayerTexSelected)
		{
			m_bLayerTexSelected = false;
			InvalidateRect(&rc);
		}
		if (rc.PtInRect(point) && !m_bLayerTexSelected)
		{
			m_bLayerTexSelected = true;
			InvalidateRect(&rc);
		}
	}
	
	CDialog::OnMouseMove(nFlags, point);
}

void CTerrainTexture::OnAutoGenMask() 
{
	if (!m_pCurrentLayer)
		return;

	m_pCurrentLayer->SetAutoGen( !m_pCurrentLayer->IsAutoGen() );
	UpdateControlData();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Regenerate the preview
	OnGeneratePreview();	
}

void CTerrainTexture::OnLoadMask() 
{
	if (!m_pCurrentLayer)
		return;

	RECT rc;

	CString file;
	if (CFileUtil::SelectSingleFile( EFILE_TYPE_TEXTURE,file ))
	{
		// Load the texture
		if (!m_pCurrentLayer->LoadMask( file ))
			AfxMessageBox("Error while loading the texture !");

		// Update the texture preview
		GetDlgItem(IDC_LAYER_MASK_PREVIEW)->GetWindowRect(&rc);
		ScreenToClient(&rc);
		InvalidateRect(&rc);

		// Update the texture information files
		UpdateControlData();

		// We modified the document
		GetIEditor()->SetModifiedFlag();
	}

	// Regenerate the preview
	OnGeneratePreview();
}

void CTerrainTexture::OnExportMask() 
{
	if (!m_pCurrentLayer)
		return;

	// Export current layer mask to bitmap.
	CString filename;
	if (CFileUtil::SelectSaveFile( SUPPORTED_IMAGES_FILTER,"bmp","",filename ))
	{
		// Tell the layer to export its mask.
		m_pCurrentLayer->ExportMask( filename );
	}
}
void CTerrainTexture::OnEditSurfaceTypes() 
{
	CSurfaceTypesDialog cfd;
	if (m_pCurrentLayer)
		cfd.SetSelectedSurfaceType( m_pCurrentLayer->GetSurfaceType() );
	cfd.DoModal();

	m_surfaceType.ResetContent();
	m_surfaceType.AddString( "" );
	for (int i = 0; i < m_doc->GetSurfaceTypeCount(); i++)
	{
		m_surfaceType.AddString( m_doc->GetSurfaceType(i)->GetName() );
	}
	if (m_pCurrentLayer)
	{
		if (m_surfaceType.SelectString( -1,m_pCurrentLayer->GetSurfaceType() ) == LB_ERR)
			m_surfaceType.SetCurSel(0);
	}
}

void CTerrainTexture::OnSelendokSurfaceType() 
{
	CString sfType;
	m_surfaceType.GetWindowText( sfType );
	if (m_pCurrentLayer)
	{
		m_pCurrentLayer->SetSurfaceType( sfType );
	}
}

void CTerrainTexture::OnLayerSetWaterColor() 
{
	COLORREF col = m_doc->GetWaterColor();
	if (GetIEditor()->SelectColor(col,this))
	{
		m_doc->SetWaterColor( col );
		OnGeneratePreview();
	}
}

void CTerrainTexture::OnDestroy()
{
	CDialog::OnDestroy();

	// Release all layer masks.
	for (int i=0; i < m_doc->GetLayerCount(); i++)
	{
		m_doc->GetLayer(i)->ReleaseMask();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnLayersLabelEditEnd()
{
	int sel = m_lstLayers.GetCurrentIndex();
	if (sel == LB_ERR)
		return;

	if (!m_pCurrentLayer)
		return;

	CString name;
	m_lstLayers.GetText(sel,name);

	// Set the changed name
	m_pCurrentLayer->SetLayerName(name);

	// We modified the document
	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnLayersLabelEditCancel()
{
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnLayersNewItem()
{
	int sel = m_lstLayers.GetCurrentIndex();
	if (sel == LB_ERR)
	{
		return;
	}
	CString name;
	m_lstLayers.GetText(sel,name);

	// Add the layer
	CLayer *pNewLayer = new CLayer;
	pNewLayer->SetLayerName( name );
	m_doc->AddLayer( pNewLayer );
	m_lstLayers.SetItemData( sel,(DWORD_PTR)pNewLayer );

	sel = m_lstLayers.GetCurSel();

	// Set it as the current one
	m_pCurrentLayer = pNewLayer;
	m_pCurrentLayer->SetSelected(true);

	// Update the controls with the data from the layer
	UpdateControlData();
	// We now have a selected layer
	EnableControls();

	// Regenerate the preview
	OnGeneratePreview();

	// We modified the document
	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnLayersDeleteItem()
{
	int sel = m_lstLayers.GetCurrentIndex();
	if (sel == LB_ERR)
		return;

	int nextSel = m_lstLayers.GetCurSel();

	if (!m_pCurrentLayer)
		return;

	/*
	// Ask before removing the layer
	int nResult;
	nResult = MessageBox("Do you really want to remove the selected layer ?" , "Remove Layer", MB_ICONQUESTION | 
		MB_YESNO | MB_APPLMODAL | MB_TOPMOST);
	if (nResult != IDYES)
		return;
	*/

	// Find the layer inside the layer list in the document and remove it.
	m_doc->RemoveLayer( m_pCurrentLayer );
	m_pCurrentLayer = 0;

	OnSelchangeLayerList();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// Regenerate the preview
	OnGeneratePreview();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnLayersMoveItemUp()
{
	int sel = m_lstLayers.GetCurrentIndex();
	if (sel < 0)
		return;

	// Move the element one down
	int prev = sel - 1;
	if (prev < 0)
		return;

	m_doc->SwapLayers( sel,prev );

	// We modified the document
	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnLayersMoveItemDown()
{
	int sel = m_lstLayers.GetCurrentIndex();
	if (sel < 0)
		return;

	// Move the element one down
	int next = sel + 1;
	if (next >= m_doc->GetLayerCount())
		return;

	m_doc->SwapLayers( sel,next );

	// We modified the document
	GetIEditor()->SetModifiedFlag();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnBnClickedSmooth()
{
	if (m_pCurrentLayer)
	{
		m_pCurrentLayer->SetSmooth( !m_pCurrentLayer->IsSmooth() );
		OnGeneratePreview();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnLayerValuesChange()
{
	OnLayerValuesUpdate();
	OnGeneratePreview();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexture::OnLayerValuesUpdate()
{
	if (!m_pCurrentLayer)
		return;

	m_pCurrentLayer->SetLayerStart( m_altStart.GetValue() );
	m_pCurrentLayer->SetLayerEnd( m_altEnd.GetValue() );
	m_pCurrentLayer->SetLayerMinSlope( m_slopeStart.GetValue() );
	m_pCurrentLayer->SetLayerMaxSlope( m_slopeEnd.GetValue() );

	m_altSlider.SetSelection( m_pCurrentLayer->GetLayerStart(),m_pCurrentLayer->GetLayerEnd() );
	m_slopeSlider.SetSelection( m_pCurrentLayer->GetLayerMinSlope(),m_pCurrentLayer->GetLayerMaxSlope() );
	m_altSlider.Invalidate();
	m_slopeSlider.Invalidate();

	GetIEditor()->SetModifiedFlag();
}