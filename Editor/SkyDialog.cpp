// SkyDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SkyDialog.h"
#include "CryEditDoc.h"
#include "PropertiesDialog.h"
#include "GenerationParam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSkyDialog dialog


CSkyDialog::CSkyDialog(CWnd* pParent /*=NULL*/)
	: CToolbarDialog(CSkyDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSkyDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_PD = PDNorth;
}


void CSkyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSkyDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSkyDialog, CToolbarDialog)
	//{{AFX_MSG_MAP(CSkyDialog)
	ON_COMMAND(ID_SKY_NORTH, OnSkyNorth)
	ON_COMMAND(ID_SKY_SOUTH, OnSkySouth)
	ON_COMMAND(ID_SKY_WEST, OnSkyWest)
	ON_COMMAND(ID_SKY_EAST, OnSkyEast)
	ON_WM_PAINT()
	ON_COMMAND(ID_SKY_CLOUDS, OnSkyClouds)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSkyDialog message handlers

BOOL CSkyDialog::OnInitDialog() 
{
	////////////////////////////////////////////////////////////////////////
	// Create and setup the toolbar and other controls
	////////////////////////////////////////////////////////////////////////

	RECT rcClient;
	TBBUTTONINFO tbbiInfo;

	CLogFile::WriteLine("Loading sky dialog...");

	// We call this function of the base class here because our direct
	// base class' OnInitDialog() needs to be called after we created the tool bar
	CDialog::OnInitDialog();
	
	// Create the toolbar
	if (!m_cDlgToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_WRAPABLE, WS_CHILD | 
		WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_cDlgToolBar.LoadToolBar(IDR_SKY))
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

	// Direction buttons are checkbuttons
	m_cDlgToolBar.SetButtonStyle(4, TBBS_CHECKGROUP);
	m_cDlgToolBar.SetButtonStyle(5, TBBS_CHECKGROUP);
	m_cDlgToolBar.SetButtonStyle(6, TBBS_CHECKGROUP);
	m_cDlgToolBar.SetButtonStyle(7, TBBS_CHECKGROUP);

	// Disable unimplemented features
	tbbiInfo.cbSize = sizeof(TBBUTTONINFO);
	tbbiInfo.dwMask = TBIF_STATE;
	tbbiInfo.fsState = NULL;
	m_cDlgToolBar.GetToolBarCtrl().SetButtonInfo(ID_SKY_WEATHER, &tbbiInfo);
	m_cDlgToolBar.GetToolBarCtrl().SetButtonInfo(ID_SKY_NORTH, &tbbiInfo);
	m_cDlgToolBar.GetToolBarCtrl().SetButtonInfo(ID_SKY_EAST, &tbbiInfo);
	m_cDlgToolBar.GetToolBarCtrl().SetButtonInfo(ID_SKY_SOUTH, &tbbiInfo);
	m_cDlgToolBar.GetToolBarCtrl().SetButtonInfo(ID_SKY_WEST, &tbbiInfo);

	// Check the default buttons
	tbbiInfo.cbSize = sizeof(TBBUTTONINFO);
	tbbiInfo.dwMask = TBIF_STATE;
	tbbiInfo.fsState = TBSTATE_CHECKED | TBSTATE_ENABLED;
	m_cDlgToolBar.GetToolBarCtrl().SetButtonInfo(ID_SKY_NORTH, &tbbiInfo);


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	CRect rc;
	GetDlgItem(IDC_PROPERTIES)->GetWindowRect( rc );
	GetDlgItem(IDC_PROPERTIES)->ShowWindow( SW_HIDE );
	ScreenToClient( rc );
	m_propWnd.Create( WS_CHILD|WS_VISIBLE,rc,this );
	
	XmlNodeRef &templ = GetIEditor()->GetDocument()->GetFogTemplate();
	if (templ)
	{
		XmlNodeRef rootNode = GetIEditor()->GetDocument()->GetEnvironmentTemplate();
		
		m_propWnd.CreateItems( rootNode );
		m_propWnd.ExpandAll();
	}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	
	// The base class is responsible for moving the controls and placing the toolbar. This
	// does not call CDialog::OnInitDialog()
	VERIFY(CToolbarDialog::OnInitDialog());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSkyDialog::OnPaint() 
{
	////////////////////////////////////////////////////////////////////////
	// Draw the cloud and sky preview
	////////////////////////////////////////////////////////////////////////

	CPaintDC dc(this); // device context for painting
	RECT rcDest;
	
	SetRect(&rcDest, 19, 54, 256, 256);

	// Paint the clouds
	GetIEditor()->GetDocument()->m_cClouds.DrawClouds(&dc, &rcDest);

	// Do not call CToolbarDialog::OnPaint() for painting messages
}

void CSkyDialog::OnSkyClouds() 
{
	////////////////////////////////////////////////////////////////////////
	// Generate new clouds
	////////////////////////////////////////////////////////////////////////

	SNoiseParams sParams;
	CGenerationParam cDialog;

	if (GetIEditor()->GetDocument()->m_cClouds.GetLastParam()->bValid)
	{
		// Use last parameters
		cDialog.LoadParam(GetIEditor()->GetDocument()->m_cClouds.GetLastParam());
	}
	else
	{
		// Set default parameters for the dialog
		cDialog.m_sldCover = 50;
		cDialog.m_sldFade = (int) (0.8f * 10);
		cDialog.m_sldFrequency = (int) (3.0f * 10);
		cDialog.m_sldFrequencyStep = (int) (2.0f * 10);
		cDialog.m_sldPasses = 8;
		cDialog.m_sldRandomBase = 1;
		cDialog.m_sldSharpness = (int) (0.999f * 1000);
		cDialog.m_sldBlur = 0;
	}
				
	// Show the generation parameter dialog
	if (cDialog.DoModal() == IDCANCEL)
		return;

	CLogFile::WriteLine("Generating cloud layer...");

	// Fill the parameter structure for the cloud generation
	cDialog.FillParam(&sParams);
	sParams.iWidth = 512;
	sParams.iHeight = 512;
	sParams.bBlueSky = true;

	BeginWaitCursor();

	// Call the generator function
	GetIEditor()->GetDocument()->m_cClouds.GenerateClouds(&sParams, GetDlgItem(IDC_GEN_STATUS));

	// Update the window
	RedrawWindow();

	// Remove the status indicator
	GetDlgItem(IDC_GEN_STATUS)->SetWindowText("");

	// We modified the document
	GetIEditor()->GetDocument()->SetModifiedFlag();

	EndWaitCursor();

	if (m_propWnd.m_hWnd)
		m_propWnd.Invalidate();
}

/////////////////////////////////////////////////////////////////////////////
// CSkyDialog direction buttons

void CSkyDialog::OnSkyNorth() 
{
	m_PD = PDNorth;
}

void CSkyDialog::OnSkySouth() 
{
	m_PD = PDSouth;
}

void CSkyDialog::OnSkyWest() 
{
	m_PD = PDWest;
}

void CSkyDialog::OnSkyEast() 
{
	m_PD = PDEast;
}

void CSkyDialog::OnDestroy() 
{
	CToolbarDialog::OnDestroy();
}
