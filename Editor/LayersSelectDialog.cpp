// LayersSelectDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LayersSelectDialog.h"
#include "Objects\ObjectManager.h"

// CLayersSelectDialog dialog

IMPLEMENT_DYNAMIC(CLayersSelectDialog, CDialog)
CLayersSelectDialog::CLayersSelectDialog( CPoint origin, CWnd* pParent /*=NULL*/)
	: CDialog(CLayersSelectDialog::IDD, pParent)
{
	m_origin = origin;
}

CLayersSelectDialog::~CLayersSelectDialog()
{
}

void CLayersSelectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LAYERS, m_layers);
}


BEGIN_MESSAGE_MAP(CLayersSelectDialog, CDialog)
	ON_LBN_SELCHANGE(IDC_LAYERS, OnLbnSelchangeLayers)
	ON_LBN_SELCANCEL(IDC_LAYERS, OnLbnSelcancelLayers)
END_MESSAGE_MAP()


// CLayersSelectDialog message handlers

//////////////////////////////////////////////////////////////////////////
void CLayersSelectDialog::OnLbnSelchangeLayers()
{
	CObjectLayer *pLayer = m_layers.GetCurrentLayer();
	if (pLayer)
	{
		m_selectedLayer = pLayer->GetName();
	}
	else
	{
		m_selectedLayer = "";
	}

	// Layer selected.
	EndDialog(IDOK);
}

//////////////////////////////////////////////////////////////////////////
LRESULT CLayersSelectDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	LRESULT res = CDialog::WindowProc(message, wParam, lParam);

	if (message == WM_NCACTIVATE)
	{
		if (wParam == FALSE)
		{
			PostMessage( WM_COMMAND,MAKEWPARAM(IDCANCEL,0), 0 );
		}
	}
	
	return res;
}

//////////////////////////////////////////////////////////////////////////
void CLayersSelectDialog::ReloadLayers()
{

	std::vector<CObjectLayer*> layers;
	GetIEditor()->GetObjectManager()->GetLayersManager()->GetLayers( layers );

	int numLayers = layers.size();
	if (numLayers > 10)
		numLayers = 10;
	int height = (m_layers.GetItemHeight(0)+1)*numLayers + 2;
	CRect layersRc;
	m_layers.GetClientRect(layersRc);
	m_layers.SetWindowPos( NULL,0,0,layersRc.right-layersRc.left,height,SWP_NOMOVE );
	
	m_layers.GetWindowRect(layersRc);

	CRect rc;
	GetWindowRect(rc);
	SetWindowPos(NULL,0,0,rc.Width(),layersRc.Height()+8,SWP_NOMOVE );

	m_layers.ReloadLayers();
	m_layers.SelectLayer( m_selectedLayer );
}

//////////////////////////////////////////////////////////////////////////
BOOL CLayersSelectDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowPos( NULL,m_origin.x,m_origin.y,0,0,SWP_NOSIZE );
	m_layers.SetBkColor( GetSysColor(COLOR_BTNFACE) );
	ReloadLayers();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CLayersSelectDialog::OnLbnSelcancelLayers()
{
	EndDialog(IDCANCEL);
}