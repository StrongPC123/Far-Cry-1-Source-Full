// TerrainDialog.cpp : implementation file
//

#include "stdafx.h"
#include "TerrainDialog.h"
#include "DimensionsDialog.h"
#include "CryEditDoc.h"
#include "TerrainCurve.h"
#include "NumberDlg.h"
#include "GenerationParam.h"
#include "Noise.h"

#include "SizeDialog.h"
#include "TerrainLighting.h"
#include "TerrainTexture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTerrainDialog dialog

CTerrainDialog::CTerrainDialog(CWnd* pParent /*=NULL*/)
	: CToolbarDialog(CTerrainDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTerrainDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	// We don't have valid recent terrain generation paramters yet
	m_sLastParam = new SNoiseParams;
	m_sLastParam->bValid = false;
	m_heightmap = 0;
}

CTerrainDialog::~CTerrainDialog()
{
	delete m_sLastParam;
}

void CTerrainDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTerrainDialog, CToolbarDialog)
	//{{AFX_MSG_MAP(CTerrainDialog)
	ON_WM_CREATE()
	ON_COMMAND(ID_TERRAIN_LOAD, OnTerrainLoad)
	ON_COMMAND(ID_TERRAIN_ERASE, OnTerrainErase)
	ON_COMMAND(ID_BRUSH_1, OnBrush1)
	ON_COMMAND(ID_BRUSH_2, OnBrush2)
	ON_COMMAND(ID_BRUSH_3, OnBrush3)
	ON_COMMAND(ID_BRUSH_4, OnBrush4)
	ON_COMMAND(ID_BRUSH_5, OnBrush5)
	ON_COMMAND(ID_TERRAIN_RESIZE, OnTerrainResize)
	ON_COMMAND(ID_TERRAIN_LIGHT, OnTerrainLight)
	ON_COMMAND(ID_TERRAIN_SURFACE, OnTerrainSurface)
	ON_COMMAND(ID_TERRAIN_GENERATE, OnTerrainGenerate)
	ON_COMMAND(ID_TERRAIN_INVERT, OnTerrainInvert)
	ON_COMMAND(ID_FILE_EXPORTHEIGHTMAP, OnExportHeightmap)
	ON_COMMAND(ID_MODIFY_MAKEISLE, OnModifyMakeisle)
	ON_COMMAND(ID_MODIFY_FLATTEN_LIGHT, OnModifyFlattenLight)
	ON_COMMAND(ID_MODIFY_FLATTEN_HEAVY, OnModifyFlattenHeavy)
	ON_COMMAND(ID_MODIFY_SMOOTH, OnModifySmooth)
	ON_COMMAND(ID_MODIFY_REMOVEWATER, OnModifyRemovewater)
	ON_COMMAND(ID_MODIFY_SMOOTHSLOPE, OnModifySmoothSlope)
	ON_COMMAND(ID_HEIGHTMAP_SHOWLARGEPREVIEW, OnHeightmapShowLargePreview)
	ON_COMMAND(ID_MODIFY_SMOOTHBEACHESCOAST, OnModifySmoothBeachesOrCoast)
	ON_COMMAND(ID_MODIFY_NOISE, OnModifyNoise)
	ON_COMMAND(ID_MODIFY_NORMALIZE, OnModifyNormalize)
	ON_COMMAND(ID_MODIFY_REDUCERANGE, OnModifyReduceRange)
	ON_COMMAND(ID_MODIFY_REDUCERANGELIGHT, OnModifyReduceRangeLight)
	ON_COMMAND(ID_MODIFY_RANDOMIZE, OnModifyRandomize)
	ON_COMMAND(ID_LOW_OPACITY, OnLowOpacity)
	ON_COMMAND(ID_MEDIUM_OPACITY, OnMediumOpacity)
	ON_COMMAND(ID_HIGH_OPACITY, OnHighOpacity)
	ON_WM_MOUSEWHEEL()
	ON_COMMAND(ID_HOLD, OnHold)
	ON_COMMAND(ID_FETCH, OnFetch)
	ON_COMMAND(ID_OPTIONS_SHOWMAPOBJECTS, OnOptionsShowMapObjects)
	ON_COMMAND(ID_OPTIONS_SHOWWATER, OnOptionsShowWater)
	ON_COMMAND(ID_SET_TO_HEIGHT, OnSetToHeight)
	ON_COMMAND(ID_NOISE_BRUSH, OnNoiseBrush)
	ON_COMMAND(ID_NORMAL_BRUSH, OnNormalBrush)
	ON_COMMAND(ID_TOOLS_EXPORTTERRAINASGEOMETRIE, OnExportTerrainAsGeometrie)
	ON_COMMAND(ID_OPTIONS_EDITTERRAINCURVE, OnOptionsEditTerrainCurve)
	ON_COMMAND(ID_SETWATERLEVEL, OnSetWaterLevel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerrainDialog message handlers

BOOL CTerrainDialog::OnInitDialog() 
{
	////////////////////////////////////////////////////////////////////////
	// Create nd setup the heightmap edit window and the toolbars
	////////////////////////////////////////////////////////////////////////

	TBBUTTONINFO tbbiInfo;
	RECT rcClient;
	UINT i;
	static CWnd wndStatic;

	CLogFile::WriteLine("Loading terrain dialog...");

	m_heightmap = GetIEditor()->GetHeightmap();

	// We call this function of the base class here because our direct
	// base class' OnInitDialog() needs to be called after we created the tool bar
	CDialog::OnInitDialog();

	// Create the toolbar
	if (!m_cDlgToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_cDlgToolBar.LoadToolBar24(IDR_TERRAIN,20))
	{
		ASSERT(0);
		return -1;      // fail to create
	}

	// Resize the toolbar
	GetClientRect(&rcClient);
	m_cDlgToolBar.SetWindowPos(NULL, 0, 0, rcClient.right, 70, SWP_NOZORDER);
	
	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_cDlgToolBar.SetBarStyle(m_cDlgToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	// Create the brush toolbar
	if (!m_cDlgBrushToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_WRAPABLE, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_cDlgBrushToolBar.LoadToolBar24(IDR_BRUSHES,26))
	{
		return -1;      // fail to create
	}

	// Resize the toolbar
	GetClientRect(&rcClient);
	m_cDlgBrushToolBar.SetWindowPos(NULL, 0, 0, rcClient.right, 70, SWP_NOZORDER);
	
	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_cDlgBrushToolBar.SetBarStyle(m_cDlgBrushToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	// Set the buton styles
	for (i=0; i<5; i++)
		m_cDlgBrushToolBar.SetButtonStyle(i, TBBS_CHECKGROUP);
	m_cDlgBrushToolBar.SetButtonStyle(6, TBBS_CHECKGROUP);
	m_cDlgBrushToolBar.SetButtonStyle(7, TBBS_CHECKGROUP);
	m_cDlgBrushToolBar.SetButtonStyle(8, TBBS_CHECKGROUP);
	m_cDlgBrushToolBar.SetButtonStyle(10, TBBS_CHECKGROUP);
	m_cDlgBrushToolBar.SetButtonStyle(11, TBBS_CHECKGROUP);
	m_cDlgBrushToolBar.SetButtonStyle(12, TBBS_CHECKGROUP);
			
	// Check the default toolbar buttons
	tbbiInfo.cbSize = sizeof(TBBUTTONINFO);
	tbbiInfo.dwMask = TBIF_STATE;
	tbbiInfo.fsState = TBSTATE_CHECKED | TBSTATE_ENABLED;
	m_cDlgToolBar.GetToolBarCtrl().SetButtonInfo(ID_TERRAIN_BRUSH, &tbbiInfo);
	m_cDlgBrushToolBar.GetToolBarCtrl().SetButtonInfo(ID_BRUSH_3, &tbbiInfo);
	m_cDlgBrushToolBar.GetToolBarCtrl().SetButtonInfo(ID_MEDIUM_OPACITY, &tbbiInfo);

	m_cDlgBrushToolBar.GetToolBarCtrl().SetButtonInfo(ID_NORMAL_BRUSH, &tbbiInfo);

	// Create the drawing window for the heightmap
	m_cDrawHeightmap.Create(this);
	m_cDrawHeightmap.SetWindowPos(NULL, 20, 20, 512, 512, SWP_NOZORDER);
	
	// Update the caption of the border surrounding the heigthmap
	UpdateBorderCaption();

	// The base class is responsible for moving the controls and placing the toolbar. This
	// does not call CDialog::OnInitDialog()
	VERIFY(CToolbarDialog::OnInitDialog());

	// Set the output window for displaying coordinates
	wndStatic.Detach();
	wndStatic.Attach(GetDlgItem(IDC_STATUS)->m_hWnd);
	m_cDrawHeightmap.SetCoordinateWindow(&wndStatic);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTerrainDialog::Refresh()
{
	// All layers need to be generated from scratch
	GetIEditor()->GetDocument()->InvalidateLayers();
	// Refresh the picture
	m_cDrawHeightmap.RedrawWindow();
}

void CTerrainDialog::OnTerrainLoad() 
{
	////////////////////////////////////////////////////////////////////////
	// Load a heightmap from a BMP file
	////////////////////////////////////////////////////////////////////////
	
	char szFilters[] = "All Images Files|*.bmp;*.pgm;*.raw|8-bit Bitmap Files (*.bmp)|*.bmp|16-bit PGM Files (*.pgm)|*.pgm|16-bit RAW Files (*.raw)|*.raw|All files (*.*)|*.*||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR, szFilters);

	if (dlg.DoModal() == IDOK) 
	{
		char ext[_MAX_EXT];
		_splitpath( dlg.GetPathName(),NULL,NULL,NULL,ext );
		
		BeginWaitCursor();

		if (stricmp(ext,".pgm") == 0)
		{
			m_heightmap->LoadPGM( dlg.GetPathName() );
		}
		else if (stricmp(ext,".raw") == 0)
		{
			m_heightmap->LoadRAW( dlg.GetPathName() );
		}
		else
		{
			// Load the heightmap
			m_heightmap->LoadBMP(dlg.GetPathName());
		}

		Refresh();
		// We modified the document
		GetIEditor()->SetModifiedFlag();

		UpdateBorderCaption();
		
		EndWaitCursor();
	}
}

void CTerrainDialog::UpdateBorderCaption()
{
	////////////////////////////////////////////////////////////////////////
	// Update the caption of the border control
	////////////////////////////////////////////////////////////////////////

	char szCaption[128];

	sprintf(szCaption, "Heightmap (%ix%i viewed in 512x512)", 
		m_heightmap->GetWidth(), m_heightmap->GetHeight());

	SetDlgItemText(IDC_HEIGHTMAP_BORDER, szCaption);
}

void CTerrainDialog::OnTerrainErase() 
{
	////////////////////////////////////////////////////////////////////////
	// Erase the heightmap
	////////////////////////////////////////////////////////////////////////

	// Ask first
	if (AfxMessageBox("Really erase the heightmap ?", MB_ICONQUESTION | MB_YESNO, NULL) != IDYES)
		return;

	// Erase it
	m_heightmap->Clear();

	// Refresh the picture
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	GetIEditor()->GetDocument()->InvalidateLayers();
}

void CTerrainDialog::OnTerrainResize() 
{
	////////////////////////////////////////////////////////////////////////
	// Query a new terrain size from the user and set it
	////////////////////////////////////////////////////////////////////////

	CDimensionsDialog cDialog;
	
	// Set the current size
	cDialog.SetDimensions(m_heightmap->GetWidth());
	
	// Show the dialog
	if (cDialog.DoModal() != IDOK)
		return;

	// Set the new size
	m_heightmap->Resize(cDialog.GetDimensions(), cDialog.GetDimensions(),m_heightmap->GetUnitSize() );
	
	// Update frame caption
	UpdateBorderCaption();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	GetIEditor()->GetDocument()->InvalidateLayers();

	// Repaint
	m_cDrawHeightmap.RedrawWindow();
}

void CTerrainDialog::OnTerrainInvert() 
{
	////////////////////////////////////////////////////////////////////////
	// Invert the heightmap
	////////////////////////////////////////////////////////////////////////
	
	// Invert the terrain
	m_heightmap->Invert();

	// Repaint
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	GetIEditor()->GetDocument()->InvalidateLayers();
}

void CTerrainDialog::OnTerrainGenerate() 
{
	////////////////////////////////////////////////////////////////////////
	// Generate a terrain
	////////////////////////////////////////////////////////////////////////

	SNoiseParams sParam;
	CGenerationParam cDialog;
	
	if (GetLastParam()->bValid)
	{
		// Use last parameters
		cDialog.LoadParam(GetLastParam());
	}
	else
	{
		// Set default parameters for the dialog
		cDialog.m_sldCover = 0;
		cDialog.m_sldFade = (int) (0.46f * 10);
		cDialog.m_sldFrequency = (int) (7.0f * 10);
		cDialog.m_sldFrequencyStep = (int) (2.0f * 10);
		cDialog.m_sldPasses = 8;
		cDialog.m_sldRandomBase = 1;
		cDialog.m_sldSharpness = (int) (0.999f * 1000);
		cDialog.m_sldBlur = 0;
	}
				
	// Show the generation parameter dialog
	if (cDialog.DoModal() == IDCANCEL)
		return;

	CLogFile::WriteLine("Generating new terrain...");

	// Fill the parameter structure for the terrain generation
	cDialog.FillParam(&sParam);
	sParam.iWidth = m_heightmap->GetWidth();
	sParam.iHeight = m_heightmap->GetHeight();
	sParam.bBlueSky = false;

	// Save the paramters
	ZeroStruct( *m_sLastParam );

	// Generate
	m_heightmap->GenerateTerrain(sParam);

	// Redraw the bitmap
	m_cDrawHeightmap.RedrawWindow();
}

void CTerrainDialog::OnExportHeightmap()
{
	////////////////////////////////////////////////////////////////////////
	// Export the heightmap to BMP
	////////////////////////////////////////////////////////////////////////

	char szFilters[] = "8-bit Bitmap (*.bmp)|*.bmp|16-bit PGM (*.pgm)|*.pgm|16-bit RAW (*.raw)|*.raw||";
	CFileDialog dlg(FALSE, "bmp", NULL, OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR, szFilters);
	
	// Show the dialog
	if (dlg.DoModal() == IDOK) 
	{
		BeginWaitCursor();

		CLogFile::WriteLine("Exporting heightmap...");


		char ext[_MAX_EXT];
		_splitpath( dlg.GetPathName(),NULL,NULL,NULL,ext );
		if (stricmp(ext,".pgm") == 0)
		{
			// PGM
			m_heightmap->SavePGM( dlg.GetPathName() );
		}
		else if (stricmp(ext,".raw") == 0)
		{
			// PGM
			m_heightmap->SaveRAW( dlg.GetPathName() );
		}
		else
		{
			// BMP or others
			m_heightmap->SaveImage( dlg.GetPathName() );
		}
		
		EndWaitCursor();
	}
}

void CTerrainDialog::OnModifySmoothBeachesOrCoast() 
{
	////////////////////////////////////////////////////////////////////////
	// Make smooth beaches or a smooth coast
	////////////////////////////////////////////////////////////////////////

	BeginWaitCursor();

	// Call the smooth beaches function of the heightmap class
	m_heightmap->MakeBeaches();

	// Redraw the bitmap
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	EndWaitCursor();
}

void CTerrainDialog::OnModifyMakeisle() 
{
	////////////////////////////////////////////////////////////////////////
	// Convert the heightmap to an island
	////////////////////////////////////////////////////////////////////////

	BeginWaitCursor();

	// Call the make isle fucntion of the heightmap class
	m_heightmap->MakeIsle();

	// Redraw the bitmap
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	EndWaitCursor();
}

void CTerrainDialog::Flatten(float fFactor)
{
	////////////////////////////////////////////////////////////////////////
	// Increase the number of flat areas on the heightmap
	////////////////////////////////////////////////////////////////////////

	BeginWaitCursor();

	// Call the flatten function of the heigtmap class
	m_heightmap->Flatten(fFactor);

	// Redraw the bitmap
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	EndWaitCursor();
}

void CTerrainDialog::OnModifyFlattenLight() 
{
	Flatten(0.75f);
}

void CTerrainDialog::OnModifyFlattenHeavy() 
{
	Flatten(0.5f);
}

void CTerrainDialog::OnModifyRemovewater() 
{
	//////////////////////////////////////////////////////////////////////
	// Remove all water areas from the heightmap
	//////////////////////////////////////////////////////////////////////

	CLogFile::WriteLine("Removing water areas from heightmap...");

	BeginWaitCursor();

	// Remove the water
	m_heightmap->RemoveWater();

	// Redraw the bitmap
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	EndWaitCursor();
}

void CTerrainDialog::OnModifySmoothSlope() 
{
	//////////////////////////////////////////////////////////////////////
	// Remove areas with high slope from the heightmap
	//////////////////////////////////////////////////////////////////////

	BeginWaitCursor();

	// Call the smooth slope function of the heightmap class
	m_heightmap->SmoothSlope();

	// Redraw the bitmap
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	EndWaitCursor();
}

void CTerrainDialog::OnModifySmooth() 
{
	//////////////////////////////////////////////////////////////////////
	// Smooth the heightmap
	//////////////////////////////////////////////////////////////////////

	m_heightmap->Smooth();
}

void CTerrainDialog::OnModifyNoise() 
{
	////////////////////////////////////////////////////////////////////////
	// Noise the heightmap
	////////////////////////////////////////////////////////////////////////
	
	m_heightmap->Noise();

	// Repaint
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	GetIEditor()->GetDocument()->InvalidateLayers();
}

void CTerrainDialog::OnModifyNormalize() 
{
	////////////////////////////////////////////////////////////////////////
	// Normalize the heightmap
	////////////////////////////////////////////////////////////////////////
	
	m_heightmap->Normalize();

	// Repaint
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	m_heightmap->InvalidateLayers();
}

void CTerrainDialog::OnModifyReduceRange() 
{
	////////////////////////////////////////////////////////////////////////
	// Reduce the value range of the heightmap (Heavy)
	////////////////////////////////////////////////////////////////////////
	
	m_heightmap->LowerRange(0.8f);

	// Repaint
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	m_heightmap->InvalidateLayers();
}

void CTerrainDialog::OnModifyReduceRangeLight() 
{
	////////////////////////////////////////////////////////////////////////
	// Reduce the value range of the heightmap (Light)
	////////////////////////////////////////////////////////////////////////
	
	m_heightmap->LowerRange(0.95f);

	// Repaint
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	m_heightmap->InvalidateLayers();
}

void CTerrainDialog::OnModifyRandomize() 
{
	////////////////////////////////////////////////////////////////////////
	// Add a small amount of random noise
	////////////////////////////////////////////////////////////////////////
	
	m_heightmap->Randomize();

	// Repaint
	m_cDrawHeightmap.RedrawWindow();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	m_heightmap->InvalidateLayers();
}


void CTerrainDialog::OnHeightmapShowLargePreview() 
{
	////////////////////////////////////////////////////////////////////////
	// Show a full-size version of the heightmap
	////////////////////////////////////////////////////////////////////////

	DWORD *pImageData = NULL;
	unsigned int i, j;
	uint8 iColor;
	t_hmap *pHeightmap = NULL;

	BeginWaitCursor();

	CLogFile::WriteLine("Exporting heightmap...");

	CHeightmap *heightmap = GetIEditor()->GetHeightmap();
	UINT iWidth = heightmap->GetWidth();
	UINT iHeight = heightmap->GetHeight();

	CImage image;
	image.Allocate( heightmap->GetWidth(),heightmap->GetHeight() );
	// Allocate memory to export the heightmap
	pImageData = (DWORD*)image.GetData();

	// Get a pointer to the heightmap data
	pHeightmap = heightmap->GetData();

	// Write heightmap into the image data array
	for (j=0; j<iHeight; j++)
		for (i=0; i<iWidth; i++)
		{
			// Get a normalized grayscale value from the heigthmap
			iColor = (uint8)__min(pHeightmap[i + j * iWidth], 255.0f);

			// Create a BGR grayscale value and store it in the image
			// data array
			pImageData[i + j * iWidth] =
				(iColor << 16) | (iColor << 8) | iColor;
		}

	// Save the heightmap into the bitmap
	CFileUtil::CreateDirectory( "Temp" );
	bool bOk = CImageUtil::SaveImage( "Temp\\HeightmapPreview.bmp",image );

	EndWaitCursor();
	
	if (bOk)
	{
		CString dir = Path::AddBackslash(GetIEditor()->GetMasterCDFolder()) + "Temp\\";
		// Show the heightmap
		::ShellExecute(::GetActiveWindow(), "open", "HeightmapPreview.bmp", 
			"", dir, SW_SHOWMAXIMIZED);
	}
}

BOOL CTerrainDialog::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	////////////////////////////////////////////////////////////////////////
	// Forward mouse wheel messages to the drawing window
	////////////////////////////////////////////////////////////////////////

	return m_cDrawHeightmap.OnMouseWheel(nFlags, zDelta, pt);
}

void CTerrainDialog::OnTerrainLight() 
{
	////////////////////////////////////////////////////////////////////////
	// Show the terrain lighting dialog
	////////////////////////////////////////////////////////////////////////

	CTerrainLighting cDialog;

	cDialog.DoModal();
}

void CTerrainDialog::OnTerrainSurface() 
{
	////////////////////////////////////////////////////////////////////////
	// Show the terrain texture dialog
	////////////////////////////////////////////////////////////////////////

	CTerrainTexture cDialog;

	cDialog.DoModal();
}

int CTerrainDialog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CToolbarDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here

	return 0;
}

void CTerrainDialog::OnHold()
{
	// Hold the current heightmap state
	m_heightmap->Hold();
}

void CTerrainDialog::OnFetch()
{
	int iResult;

	// Did we modify the heigthmap ?
	if (GetIEditor()->IsModified())
	{
		// Ask first
		iResult = MessageBox("Do you really want to restore the previous heightmap state ?", 
			"Fetch", MB_YESNO | MB_ICONQUESTION);

		// Abort
		if (iResult == IDNO)
			return;
	}

	// Restore the old heightmap state
	m_heightmap->Fetch();

	// We modified the document
	GetIEditor()->SetModifiedFlag();

	// All layers need to be generated from scratch
	m_heightmap->InvalidateLayers();

	// Refresh the picture
	m_cDrawHeightmap.RedrawWindow();
}

/////////////////////////////////////////////////////////////////////////////
// Options

void CTerrainDialog::OnOptionsShowMapObjects() 
{
	UINT iState;

	// Get the current state of the menu item
	iState = GetMenu()->GetSubMenu(4)->GetMenuState(ID_OPTIONS_SHOWMAPOBJECTS, MF_CHECKED);
	ASSERT(iState != 0xFFFFFFFF);

	// Update the draw window
	m_cDrawHeightmap.SetShowMapObj((int) (iState & MF_CHECKED) == 0);
	m_cDrawHeightmap.RedrawWindow();

	// Toggle it
	GetMenu()->GetSubMenu(4)->CheckMenuItem(ID_OPTIONS_SHOWMAPOBJECTS, 
		(iState & MF_CHECKED) ? MF_UNCHECKED : MF_CHECKED);
}

void CTerrainDialog::OnOptionsShowWater() 
{
	UINT iState;

	// Get the current state of the menu item
	iState = GetMenu()->GetSubMenu(4)->GetMenuState(ID_OPTIONS_SHOWWATER, MF_CHECKED);
	ASSERT(iState != 0xFFFFFFFF);

	// Update the draw window
	m_cDrawHeightmap.SetShowWater((int) (iState & MF_CHECKED) == 0);
	m_cDrawHeightmap.RedrawWindow();

	// Toggle it
	GetMenu()->GetSubMenu(4)->CheckMenuItem(ID_OPTIONS_SHOWWATER, 
		(iState & MF_CHECKED) ? MF_UNCHECKED : MF_CHECKED);
}

/////////////////////////////////////////////////////////////////////////////
// Brushes

void CTerrainDialog::OnBrush1() 
{
	m_cDrawHeightmap.SetCurrentBrush(0);
}

void CTerrainDialog::OnBrush2() 
{
	m_cDrawHeightmap.SetCurrentBrush(1);
}

void CTerrainDialog::OnBrush3() 
{
	m_cDrawHeightmap.SetCurrentBrush(2);
}

void CTerrainDialog::OnBrush4() 
{
	m_cDrawHeightmap.SetCurrentBrush(3);
}

void CTerrainDialog::OnBrush5() 
{
	m_cDrawHeightmap.SetCurrentBrush(4);	
}

void CTerrainDialog::OnLowOpacity()
{
	m_cDrawHeightmap.SetOpacity(8);
}

void CTerrainDialog::OnMediumOpacity()
{
	m_cDrawHeightmap.SetOpacity(16);
}

void CTerrainDialog::OnHighOpacity()
{
	m_cDrawHeightmap.SetOpacity(24);
}

void CTerrainDialog::OnSetToHeight()
{
	////////////////////////////////////////////////////////////////////////
	// Activate the set to height feature. Let the uer specify the height 
	////////////////////////////////////////////////////////////////////////

	TBBUTTONINFO tbbiInfo;

	// Fill in the structure to query if the button is checked
	tbbiInfo.cbSize = sizeof(TBBUTTONINFO);
	tbbiInfo.dwMask = TBIF_STATE;
	
	// Query the status of this button
	m_cDlgBrushToolBar.GetToolBarCtrl().GetButtonInfo(ID_SET_TO_HEIGHT, &tbbiInfo);

	// Did the user check or uncheck the button ?
	if (tbbiInfo.fsState & TBSTATE_CHECKED)
	{
		// Query the new height from the user and set it to the class
		CNumberDlg cDialog( this,m_cDrawHeightmap.GetSetToHeight(),"Set brush Height" );
		if (cDialog.DoModal() == IDOK)
		{
			m_cDrawHeightmap.SetSetToHeight( cDialog.GetValue() );
			// Disable the noise brush
			m_cDrawHeightmap.SetUseNoiseBrush(false);
		}
	}
	else
	{
		// The user just disabled the set to height feature
		m_cDrawHeightmap.SetSetToHeight(-1.0f);
	}
}

void CTerrainDialog::OnNoiseBrush()
{
	////////////////////////////////////////////////////////////////////////
	// Activate the noise brush
	////////////////////////////////////////////////////////////////////////

	// Disable set to height brush and enable noise brush
	m_cDrawHeightmap.SetSetToHeight(-1.0f);
	m_cDrawHeightmap.SetUseNoiseBrush(true);
}

void CTerrainDialog::OnNormalBrush()
{
	////////////////////////////////////////////////////////////////////////
	// Activate the normal brush
	////////////////////////////////////////////////////////////////////////

	// Disable noise brush and set to heigth brush
	m_cDrawHeightmap.SetSetToHeight(-1.0f);
	m_cDrawHeightmap.SetUseNoiseBrush(false);
}

void CTerrainDialog::OnExportTerrainAsGeometrie()
{
	////////////////////////////////////////////////////////////////////////
	// Store the terrain in the OBJ format on the disk
	////////////////////////////////////////////////////////////////////////

	RECT rcExport;
	char szFilters[] = "Object files (*.obj)|*.obj|All files (*.*)|*.*||";
	CFileDialog dlg(FALSE, "obj", "*.obj", OFN_OVERWRITEPROMPT|OFN_NOCHANGEDIR, szFilters);
	CPoint ptMarker;
	const int iAreaSize = 64;

	// Obtain the position of the terrain marker
	ptMarker = m_cDrawHeightmap.GetMarkerPos();

	if (dlg.DoModal() == IDOK) 
	{
		// Export a rectangle of iAreaSize x iAreaSize
		::SetRect(&rcExport, ptMarker.x - (iAreaSize / 2), 
			ptMarker.y - (iAreaSize / 2), 
			ptMarker.x + (iAreaSize / 2), 
			ptMarker.y + (iAreaSize / 2));

		VERIFY(GetIEditor()->GetDocument()->OnExportTerrainAsGeometrie(dlg.GetPathName().GetBuffer(1), rcExport));
	}
}

void CTerrainDialog::OnOptionsEditTerrainCurve()
{
	/*
	////////////////////////////////////////////////////////////////////////
	// Show the terrian curve dialog
	////////////////////////////////////////////////////////////////////////
	return; // Ignore it.

	CTerrainCurve cDialog;

	cDialog.DoModal();

	unsigned int i, j;
	CCurveObject *pCurveObj = &GLOBAL_GET_DOC->m_cTerrainCurve;
	CHeightmap *pHeightmap = &GLOBAL_GET_DOC->m_cHeightmap;
	HEIGHTMAP_DATA *pData = pHeightmap->GetData();
	UINT iWidth = pHeightmap->GetWidth();
	UINT iHeight = pHeightmap->GetHeight();

	BeginWaitCursor();

	for (i=0; i<iWidth; i++)
		for (j=0; j<iHeight; j++)
		{
			//pData[i + j * iWidth] = 65535.0f - pCurveObj->GetCurveY(pData[i + j * iWidth] / 511.0f) * 511.0f;
			
			pData[i + j * iWidth] = 65535.0f - pCurveObj->GetCurveY(pData[i + j * iWidth]);
		}

	EndWaitCursor();

	m_cDrawHeightmap.RedrawWindow();
	*/
}

void CTerrainDialog::OnSetWaterLevel() 
{
	////////////////////////////////////////////////////////////////////////
	// Let the user change the current water level
	////////////////////////////////////////////////////////////////////////
	// Get the water level from the document and set it as default into
	// the dialog
	float waterLevel = GetIEditor()->GetHeightmap()->GetWaterLevel();
	CNumberDlg cDialog( this,waterLevel,"Set Water Height" );

	// Show the dialog
	if (cDialog.DoModal() == IDOK)
	{
		// Retrive the new water level from the dialog and save it in the document
		waterLevel = cDialog.GetValue();
		GetIEditor()->GetHeightmap()->SetWaterLevel(waterLevel);

		// We modified the document
		GetIEditor()->SetModifiedFlag();

		Refresh();
	}
}
