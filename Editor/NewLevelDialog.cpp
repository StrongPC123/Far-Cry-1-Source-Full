// NewLevelDialog.cpp : implementation file
//

#include "stdafx.h"
#include "NewLevelDialog.h"


// CNewLevelDialog dialog

IMPLEMENT_DYNAMIC(CNewLevelDialog, CDialog)
CNewLevelDialog::CNewLevelDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNewLevelDialog::IDD, pParent)
	, m_level(_T(""))
	, m_useTerrain(FALSE)
	, m_terrainResolution(0)
	, m_terrainUnits(0)
{

	// Default is 1024x1024
	m_terrainResolution = 3;
	// 2 meters per unit.
	m_terrainUnits = 1;
}

CNewLevelDialog::~CNewLevelDialog()
{
}

void CNewLevelDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LEVEL, m_level);
	DDX_Check(pDX, IDC_USE_TERRAIN, m_useTerrain);
	DDX_CBIndex(pDX, IDC_TERRAIN_RESOLUTION, m_terrainResolution);
	DDX_CBIndex(pDX, IDC_TERRANI_UNITS, m_terrainUnits);
	DDX_Control(pDX, IDC_TERRAIN_INFO, m_cTerrainInfo);
	DDX_Control(pDX, IDC_TERRAIN_RESOLUTION, m_cTerrainResolution);
	DDX_Control(pDX, IDC_TERRANI_UNITS, m_cTerrainUnits);
}


BEGIN_MESSAGE_MAP(CNewLevelDialog, CDialog)
	ON_BN_CLICKED(IDC_USE_TERRAIN, OnBnClickedUseTerrain)
	ON_CBN_SELENDOK(IDC_TERRAIN_RESOLUTION, OnCbnSelendokTerrainResolution)
	ON_CBN_SELENDOK(IDC_TERRANI_UNITS, OnCbnSelendokTerraniUnits)
END_MESSAGE_MAP()


#define START_TERRAIN_RESOLUTION 128
#define START_TERRAIN_UNITS 1

// CNewLevelDialog message handlers

BOOL CNewLevelDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_useTerrain = TRUE;

	// Inititialize terrain values.
	CString str;

	int i;
	int resolution = START_TERRAIN_RESOLUTION;
	for (i = 0; i < 10; i++)
	{
		str.Format( "%dx%d",resolution,resolution );
		m_cTerrainResolution.AddString( str );
		resolution *= 2;
	}

	int units = START_TERRAIN_UNITS;
	for (i = 0; i < 6; i++)
	{
		str.Format( "%d",units );
		m_cTerrainUnits.AddString( str );
		units *= 2;
	}

	UpdateTerrainInfo();

	// Save data.
	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CNewLevelDialog::UpdateTerrainInfo()
{
	CString str;
	int size = GetTerrainResolution() * GetTerrainUnits();

	str.Format( _T("Terrain Size: %dx%d Meters"),size,size );
	m_cTerrainInfo.SetWindowText( str );
}

//////////////////////////////////////////////////////////////////////////
CString CNewLevelDialog::GetLevel() const
{
	return m_level;
}

//////////////////////////////////////////////////////////////////////////
bool CNewLevelDialog::IsUseTerrain() const
{
	return m_useTerrain != FALSE;
}

//////////////////////////////////////////////////////////////////////////
int CNewLevelDialog::GetTerrainResolution() const
{
	return START_TERRAIN_RESOLUTION*(1 << m_terrainResolution);
}
	
int CNewLevelDialog::GetTerrainUnits() const
{
	return START_TERRAIN_UNITS*(1 << m_terrainUnits);
}

//////////////////////////////////////////////////////////////////////////
void CNewLevelDialog::OnBnClickedUseTerrain()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if (m_useTerrain)
	{
		m_cTerrainResolution.EnableWindow(TRUE);
		m_cTerrainUnits.EnableWindow(TRUE);
	}
	else
	{
		m_cTerrainResolution.EnableWindow(FALSE);
		m_cTerrainUnits.EnableWindow(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
void CNewLevelDialog::OnCbnSelendokTerrainResolution()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	UpdateTerrainInfo();
}

//////////////////////////////////////////////////////////////////////////
void CNewLevelDialog::OnCbnSelendokTerraniUnits()
{
	// TODO: Add your control notification handler code here
	UpdateData();
	UpdateTerrainInfo();
}

//////////////////////////////////////////////////////////////////////////
void CNewLevelDialog::SetTerrainResolution( int res )
{
	int i = 0;
	int dim = res / START_TERRAIN_RESOLUTION;
	for (i = 0; i < 32; i++)
	{
		if ((dim >> i) == 1)
		{
			m_terrainResolution = i;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CNewLevelDialog::SetTerrainUnits( int units )
{
	int i = 0;
	int dim = units / START_TERRAIN_UNITS;
	for (i = 0; i < 32; i++)
	{
		if ((dim >> i) == 1)
		{
			m_terrainUnits = i;
			break;
		}
	}
}
