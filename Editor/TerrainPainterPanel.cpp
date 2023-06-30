////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terrainpainterpanel.cpp
//  Version:     v1.00
//  Created:     25/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "TerrainPainterPanel.h"
#include "TerrainTexturePainter.h"

#include "CryEditDoc.h"
#include "Layer.h"
#include ".\terrainpainterpanel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTerrainPainterPanel dialog


CTerrainPainterPanel::CTerrainPainterPanel(CTerrainTexturePainter *tool,CWnd* pParent /*=NULL*/)
	: CDialog(CTerrainPainterPanel::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTerrainPainterPanel)
	//}}AFX_DATA_INIT

	Create( IDD,pParent );

	assert( tool != 0 );
	m_tool = tool;
}

void CTerrainPainterPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainPainterPanel)
	DDX_Control(pDX, IDC_BRUSH_TYPE, m_brushType);
	DDX_Control(pDX, IDC_BRUSH_HARDNESS_SLIDER, m_hardnessSlider);
	DDX_Control(pDX, IDC_BRUSH_HEIGHT_SLIDER, m_heightSlider);
	DDX_Control(pDX, IDC_BRUSH_RADIUS_SLIDER, m_radiusSlider);
	DDX_Control(pDX, IDC_LAYERS, m_layers);
	DDX_Control(pDX, IDC_PAINTVEGETATION, m_paintVegetation);
	DDX_Control(pDX, IDC_PAINT_SIMPLELIGHTING, m_optLighting);
	DDX_Control(pDX, IDC_PAINT_TERRAIN_SHADOWS, m_optTerrainShadows);
	DDX_Control(pDX, IDC_PAINT_OBJECT_SHADOWS, m_optObjectShadows);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainPainterPanel, CDialog)
	//{{AFX_MSG_MAP(CTerrainPainterPanel)
	ON_EN_UPDATE(IDC_BRUSH_RADIUS, OnUpdateNumbers)

	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BRUSH_HARDNESS_SLIDER, OnHardnessSlider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BRUSH_HEIGHT_SLIDER, OnHeightSlider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_BRUSH_RADIUS_SLIDER, OnRadiusSlider)


	ON_CBN_SELENDOK(IDC_BRUSH_TYPE, OnSelendokBrushType)
	ON_EN_UPDATE(IDC_BRUSH_HEIGHT, OnUpdateNumbers)
	ON_EN_UPDATE(IDC_BRUSH_HARDNESS, OnUpdateNumbers)
	ON_EN_UPDATE(IDC_NOISE_SCALE, OnUpdateNumbers)
	ON_EN_UPDATE(IDC_NOISE_FREQ, OnUpdateNumbers)

	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PAINTVEGETATION, OnBnClickedPaintvegetation)
	ON_BN_CLICKED(IDC_PAINT_SIMPLELIGHTING, OnBnClickedPaintSimplelighting)
	ON_BN_CLICKED(IDC_PAINT_TERRAIN_SHADOWS, OnBnClickedPaintTerrainShadows)
	ON_BN_CLICKED(IDC_PAINT_OBJECT_SHADOWS, OnBnClickedPaintObjectShadows)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerrainPainterPanel message handlers

BOOL CTerrainPainterPanel::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_brushRadius.Create( this,IDC_BRUSH_RADIUS );
	m_brushRadius.SetRange( 0,32 );
	
	m_brushHeight.Create( this,IDC_BRUSH_HEIGHT );
	m_brushHeight.SetRange( 0,255 );

	m_brushHardness.Create( this,IDC_BRUSH_HARDNESS );
	m_brushHardness.SetRange( 0,1 );

	m_radiusSlider.SetRange( 1,32 );
	m_heightSlider.SetRange( 0,255 );
	m_hardnessSlider.SetRange( 0,100 );

	m_brushType.AddString( _T("Paint") );
	m_brushType.AddString( _T("Smooth") );

	// Fill layers.
	ReloadLayers();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::SetBrush( CTextureBrush &br )
{
	m_brushRadius.SetRange( br.minRadius,br.maxRadius );
	m_brushRadius.SetValue( br.radius );
	m_brushHeight.SetValue( br.value );
	m_brushHardness.SetValue( br.hardness );
	m_paintVegetation.SetCheck( (br.bUpdateVegetation)?BST_CHECKED:BST_UNCHECKED );
	m_optLighting.SetCheck( (br.bPreciseLighting)?BST_CHECKED:BST_UNCHECKED );
	m_optTerrainShadows.SetCheck( (br.bTerrainShadows)?BST_CHECKED:BST_UNCHECKED );
	m_optObjectShadows.SetCheck( (br.bObjectShadows)?BST_CHECKED:BST_UNCHECKED );

	m_radiusSlider.SetPos( br.radius );
	m_heightSlider.SetPos( br.value );
	m_hardnessSlider.SetPos( br.hardness*100.0f );

	if (br.type == ET_BRUSH_PAINT)
	{
		m_brushType.SelectString( -1,_T("Paint") );
		m_heightSlider.EnableWindow(TRUE);
		m_brushHeight.EnableWindow(TRUE);
	}
	if (br.type == ET_BRUSH_SMOOTH)
	{
		m_brushType.SelectString( -1,_T("Smooth") );
		m_heightSlider.EnableWindow(FALSE);
		m_brushHeight.EnableWindow(FALSE);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnUpdateNumbers() 
{
	CTextureBrush br;
	m_tool->GetBrush(br);
	br.radius = m_brushRadius.GetValue();
	br.value = m_brushHeight.GetValue();
	br.hardness = m_brushHardness.GetValue();
	SetBrush( br );
	m_tool->SetBrush(br);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnHardnessSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CTextureBrush br;
	m_tool->GetBrush(br);
	br.hardness = (float)m_hardnessSlider.GetPos()/100.0f;
	SetBrush( br );
	m_tool->SetBrush(br);

	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnHeightSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CTextureBrush br;
	m_tool->GetBrush(br);
	br.value = m_heightSlider.GetPos();
	SetBrush( br );
	m_tool->SetBrush(br);
	
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnRadiusSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CTextureBrush br;
	m_tool->GetBrush(br);
	br.radius = m_radiusSlider.GetPos();
	SetBrush( br );
	m_tool->SetBrush(br);
	
	*pResult = 0;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnSelendokBrushType() 
{
	int sel = m_brushType.GetCurSel();
	if (sel != LB_ERR)
	{
		CTextureBrush br;
		m_tool->GetBrush(br);
		switch (sel)
		{
		case 0:
			br.type = ET_BRUSH_PAINT;
			break;
		case 1:
			br.type = ET_BRUSH_SMOOTH;
			break;
		}
		SetBrush( br );
		m_tool->SetBrush(br);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::ReloadLayers()
{
	CString selected;
	m_layers.ResetContent();
	CCryEditDoc *pDoc = GetIEditor()->GetDocument();
	for (int i = 0; i < pDoc->GetLayerCount(); i++)
	{
		CLayer *pLayer = pDoc->GetLayer(i);
		if (pLayer->IsSelected())
		{
			selected = pLayer->GetLayerName();
		}
		if (!pLayer->IsAutoGen() && i > 0)
		{
			m_layers.AddString( pLayer->GetLayerName() );
		}
	}
	m_layers.SelectString( -1,selected );
}

//////////////////////////////////////////////////////////////////////////
CString CTerrainPainterPanel::GetSelectedLayer()
{
	int curSel = m_layers.GetCurSel();
	if (curSel < 0)
		return "";
	CString selStr;
	m_layers.GetText(curSel,selStr );
	return selStr;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnBnClickedPaintvegetation()
{
	CTextureBrush br;
	m_tool->GetBrush(br);
	br.bUpdateVegetation = !br.bUpdateVegetation;
	m_tool->SetBrush(br);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnBnClickedPaintSimplelighting()
{
	CTextureBrush br;
	m_tool->GetBrush(br);
	br.bPreciseLighting = !br.bPreciseLighting;
	m_tool->SetBrush(br);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnBnClickedPaintTerrainShadows()
{
	CTextureBrush br;
	m_tool->GetBrush(br);
	br.bTerrainShadows = !br.bTerrainShadows;
	m_tool->SetBrush(br);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainPainterPanel::OnBnClickedPaintObjectShadows()
{
	CTextureBrush br;
	m_tool->GetBrush(br);
	br.bObjectShadows= !br.bObjectShadows;
	m_tool->SetBrush(br);
}
