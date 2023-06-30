// TerrainModifyPanel.cpp : implementation file
//

#include "stdafx.h"
#include "TerrainModifyPanel.h"
#include "TerrainModifyTool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTerrainModifyPanel dialog


CTerrainModifyPanel::CTerrainModifyPanel(CTerrainModifyTool *tool,CWnd* pParent /*=NULL*/)
	: CDialog(CTerrainModifyPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTerrainModifyPanel)
	//}}AFX_DATA_INIT

	Create( IDD,pParent );

	assert( tool != 0 );
	m_tool = tool;
}

void CTerrainModifyPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainModifyPanel)
	DDX_Control(pDX, IDC_BRUSH_TYPE, m_brushType);
	DDX_Control(pDX, IDC_NOISE_FREQ_SLIDER, m_noiseFreqSlider);
	DDX_Control(pDX, IDC_NOISE_SCALE_SLIDER, m_noiseScaleSlider);
	DDX_Control(pDX, IDC_BRUSH_HARDNESS_SLIDER, m_hardnessSlider);
	DDX_Control(pDX, IDC_BRUSH_HEIGHT_SLIDER, m_heightSlider);
	DDX_Control(pDX, IDC_BRUSH_RADIUS_SLIDER, m_radiusSlider);
	DDX_Control(pDX, IDC_BRUSH_RADIUS_SLIDER2, m_radiusSlider2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainModifyPanel, CDialog)
	//{{AFX_MSG_MAP(CTerrainModifyPanel)
	ON_WM_HSCROLL()
	ON_EN_UPDATE(IDC_BRUSH_RADIUS, OnUpdateNumbers)
	ON_EN_UPDATE(IDC_BRUSH_RADIUS2, OnUpdateNumbers)
	ON_BN_CLICKED(IDC_BRUSH_NOISE, OnBrushNoise)
	ON_CBN_SELENDOK(IDC_BRUSH_TYPE, OnSelendokBrushType)
	ON_EN_UPDATE(IDC_BRUSH_HEIGHT, OnUpdateNumbers)
	ON_EN_UPDATE(IDC_BRUSH_HARDNESS, OnUpdateNumbers)
	ON_EN_UPDATE(IDC_NOISE_SCALE, OnUpdateNumbers)
	ON_EN_UPDATE(IDC_NOISE_FREQ, OnUpdateNumbers)
	ON_BN_CLICKED(IDC_REPOSITION_OBJECTS, OnRepositionObjects)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerrainModifyPanel message handlers

BOOL CTerrainModifyPanel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_brushRadius.Create( this,IDC_BRUSH_RADIUS );
	m_brushRadius.SetRange( 1,200 );
	
	m_brushRadius2.Create( this,IDC_BRUSH_RADIUS2 );
	m_brushRadius2.SetRange( 0,200 );
	
	m_brushHeight.Create( this,IDC_BRUSH_HEIGHT );
	m_brushHeight.SetRange( 0,255 );

	m_brushHardness.Create( this,IDC_BRUSH_HARDNESS );
	m_brushHardness.SetRange( 0,1 );

	m_noiseScale.Create( this,IDC_NOISE_SCALE );
	m_noiseScale.SetRange( 0,100 );

	m_noiseFreq.Create( this,IDC_NOISE_FREQ );
	m_noiseFreq.SetRange( 0,100 );

	m_radiusSlider.SetRange( 1,200 );
	m_radiusSlider2.SetRange( 0,200 );
	m_heightSlider.SetRange( 0,255 );
	m_hardnessSlider.SetRange( 0,100 );

	m_noiseScaleSlider.SetRange( 0,100 );
	m_noiseFreqSlider.SetRange( 0,100 );

	m_brushType.AddString( "Flatten" );
	m_brushType.AddString( "Smooth" );
	m_brushType.AddString( "Rise/Lower" );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyPanel::SetBrush( CTerrainBrush &br )
{
	m_brushHeight.SetRange( 0,255 );
	m_heightSlider.SetRange( 0,255 );
	if (br.type == eBrushFlatten)
	{
		m_brushType.SetCurSel(0);
		m_heightSlider.EnableWindow(TRUE);
		m_brushHeight.EnableWindow(TRUE);
		m_radiusSlider2.EnableWindow(TRUE);
		m_brushRadius2.EnableWindow(TRUE);
	}
	if (br.type == eBrushSmooth)
	{
		m_brushType.SetCurSel(1);
		br.bNoise = false;
		m_heightSlider.EnableWindow(FALSE);
		m_brushHeight.EnableWindow(FALSE);
		m_radiusSlider2.EnableWindow(FALSE);
		m_brushRadius2.EnableWindow(FALSE);
	}
	if (br.type == eBrushRiseLower)
	{
		m_brushType.SetCurSel(2);
		m_heightSlider.EnableWindow(TRUE);
		m_brushHeight.EnableWindow(TRUE);
		m_radiusSlider2.EnableWindow(TRUE);
		m_brushRadius2.EnableWindow(TRUE);
		m_brushHeight.SetRange( -255,255 );
		m_heightSlider.SetRange( -50,50 );
	}
	m_brushRadius.SetValue( br.radius );
	m_brushRadius2.SetValue( br.radiusInside );
	m_brushHeight.SetValue( br.height );
	m_brushHardness.SetValue( br.hardness );
	m_noiseFreq.SetValue( br.noiseFreq );
	m_noiseScale.SetValue( br.noiseScale );

	m_radiusSlider.SetPos( br.radius );
	m_radiusSlider2.SetPos( br.radiusInside );
	m_heightSlider.SetPos( br.height );
	m_hardnessSlider.SetPos( br.hardness*100.0f );
	m_noiseScaleSlider.SetPos( br.noiseScale );
	m_noiseFreqSlider.SetPos( br.noiseFreq );

	m_noiseScaleSlider.EnableWindow(br.bNoise);
	m_noiseFreqSlider.EnableWindow(br.bNoise);
	m_noiseScale.EnableWindow(br.bNoise);
	m_noiseFreq.EnableWindow(br.bNoise);

	CheckDlgButton( IDC_BRUSH_NOISE,(br.bNoise)?BST_CHECKED:BST_UNCHECKED );
	CheckDlgButton( IDC_REPOSITION_OBJECTS,(br.bRepositionObjects)?BST_CHECKED:BST_UNCHECKED );
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyPanel::OnUpdateNumbers() 
{
	CTerrainBrush br;
	m_tool->GetBrush(br);
	float prevRadius = br.radius;
	float prevRadiusInside = br.radiusInside;
	br.radius = m_brushRadius.GetValue();
	br.radiusInside = m_brushRadius2.GetValue();
	if (br.radius < br.radiusInside)
	{
		if (prevRadiusInside != br.radiusInside) // Check if changing inside radius.
			br.radius = br.radiusInside;
		else
			br.radiusInside = br.radius; // Changing outside radius;
	}
	br.height = m_brushHeight.GetValue();
	br.hardness = m_brushHardness.GetValue();
	br.noiseFreq = m_noiseFreq.GetValue();
	br.noiseScale = m_noiseScale.GetValue();
	SetBrush( br );
	m_tool->SetBrush(br);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyPanel::OnBrushNoise() 
{
	BOOL noise = IsDlgButtonChecked(IDC_BRUSH_NOISE);
	CTerrainBrush br;
	m_tool->GetBrush(br);
	br.bNoise = (noise)?true:false;
	SetBrush( br );
	m_tool->SetBrush(br);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyPanel::OnHScroll( UINT nSBCode,UINT nPos,CScrollBar* pScrollBar )
{
	CSliderCtrl *pSliderCtrl = (CSliderCtrl*)pScrollBar;
	CTerrainBrush br;
	m_tool->GetBrush(br);
	if (pSliderCtrl == &m_hardnessSlider)
	{
		br.hardness = (float)m_hardnessSlider.GetPos()/100.0f;
	}
	else if (pSliderCtrl == &m_heightSlider)
	{
		br.height = m_heightSlider.GetPos();
	}
	else if (pSliderCtrl == &m_radiusSlider)
	{
		br.radius = m_radiusSlider.GetPos();
		if (br.radius < br.radiusInside)
			br.radiusInside = br.radius;
	}
	else if (pSliderCtrl == &m_radiusSlider2)
	{
		br.radiusInside = m_radiusSlider2.GetPos();
		if (br.radius < br.radiusInside)
			br.radius = br.radiusInside;
	}
	else if (pSliderCtrl == &m_noiseScaleSlider)
	{
		br.noiseScale = m_noiseScaleSlider.GetPos();
	}
	else if (pSliderCtrl == &m_noiseFreqSlider)
	{
		br.noiseFreq = m_noiseFreqSlider.GetPos();
	}
	SetBrush( br );
	m_tool->SetBrush(br);
}

void CTerrainModifyPanel::OnSelendokBrushType() 
{
	int sel = m_brushType.GetCurSel();
	if (sel != LB_ERR)
	{
		switch (sel)
		{
		case 0:
			m_tool->SetActiveBrushType(eBrushFlatten);
			break;
		case 1:
			m_tool->SetActiveBrushType(eBrushSmooth);
			break;
		case 2:
			m_tool->SetActiveBrushType(eBrushRiseLower);
			break;
		}
	}
	CTerrainBrush br;
	m_tool->GetBrush(br);
	SetBrush( br );
}

void CTerrainModifyPanel::OnRepositionObjects() 
{
	BOOL noise = IsDlgButtonChecked(IDC_REPOSITION_OBJECTS);
	CTerrainBrush br;
	m_tool->GetBrush(br);
	br.bRepositionObjects = (noise)?true:false;
	SetBrush( br );
	m_tool->SetBrush(br);
	
}
