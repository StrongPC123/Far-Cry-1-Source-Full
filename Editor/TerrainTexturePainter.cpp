////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   terraintexturepainter.cpp
//  Version:     v1.00
//  Created:     8/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include "TerrainTexturePainter.h"
#include "Viewport.h"
#include "Heightmap.h"
#include "TerrainGrid.h"
#include "VegetationMap.h"
#include "Objects\DisplayContext.h"
#include "Objects\ObjectManager.h"

#include "TerrainPainterPanel.h"

#include "CryEditDoc.h"
#include "Layer.h"

#include "Util\ImagePainter.h"

#include <I3DEngine.h>

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CTerrainTexturePainter,CEditTool)

CTextureBrush CTerrainTexturePainter::m_brush;

namespace {
	int s_toolPanelId = 0;
	CTerrainPainterPanel *s_toolPanel = 0;
};

//////////////////////////////////////////////////////////////////////////
CTerrainTexturePainter::CTerrainTexturePainter()
{
	SetStatusText( _T("Paint Texture Layers") );

	m_heightmap = GetIEditor()->GetHeightmap();
	assert( m_heightmap );

	m_3DEngine = GetIEditor()->Get3DEngine();
	assert( m_3DEngine );

	m_renderer = GetIEditor()->GetRenderer();
	assert( m_renderer );

	m_terrainGrid = m_heightmap->GetTerrainGrid();
	assert(m_terrainGrid);
	
	m_pointerPos(0,0,0);
	GetIEditor()->ClearSelection();

	//////////////////////////////////////////////////////////////////////////
	// Initialize sectors.
	//////////////////////////////////////////////////////////////////////////
	SSectorInfo sectorInfo;
	m_heightmap->GetSectorsInfo( sectorInfo );
	m_numSectors = sectorInfo.numSectors;
	m_sectorSize = sectorInfo.sectorSize;
	m_sectorTexSize = sectorInfo.sectorTexSize;
	m_surfaceTextureSize = sectorInfo.surfaceTextureSize;

	m_pixelsPerMeter = ((float)sectorInfo.sectorTexSize) / sectorInfo.sectorSize;

	// Initialize terrain texture generator.
	//m_terrTexGen.PrepareHeightmap( m_surfaceTextureSize,m_surfaceTextureSize );
	//m_terrTexGen.PrepareLayers( m_surfaceTextureSize,m_surfaceTextureSize );
}

//////////////////////////////////////////////////////////////////////////
CTerrainTexturePainter::~CTerrainTexturePainter()
{
	m_pointerPos(0,0,0);

	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->CancelUndo();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexturePainter::BeginEditParams( IEditor *ie,int flags )
{
	if (!s_toolPanelId)
	{
		s_toolPanel = new CTerrainPainterPanel(this,AfxGetMainWnd());
		s_toolPanelId = GetIEditor()->AddRollUpPage( ROLLUP_TERRAIN,_T("Modify Terrain"),s_toolPanel );
		AfxGetMainWnd()->SetFocus();

		s_toolPanel->SetBrush(m_brush);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexturePainter::EndEditParams()
{
	if (s_toolPanelId)
	{
		GetIEditor()->RemoveRollUpPage(ROLLUP_TERRAIN,s_toolPanelId);
		s_toolPanel = 0;
		s_toolPanelId = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainTexturePainter::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	bool bPainting = false;
	m_pointerPos = view->ViewToWorld( point,0,true );

	m_brush.bErase = false;
	if (flags&MK_CONTROL)
	{
		m_brush.bErase = true;
	}

	if (event == eMouseLDown)
	{
		Paint();
	}

	if (event == eMouseMove && (flags&MK_LBUTTON))
	{
		Paint();
	}

	GetIEditor()->SetStatusText( _T("L-Mouse: Paint Layer  CTRL: Erase Layer  +/-: Change Brush Radius") );

	/*
	if (event == eMouseLDown && (flags&MK_CONTROL))
	{
		m_brush.height = GetIEditor()->GetTerrainElevation(m_pointerPos.x,m_pointerPos.y);
		if (s_toolPanel)
			s_toolPanel->SetBrush(m_brush);
	}
	else if (event == eMouseLDown)
	{
		if (!GetIEditor()->IsUndoRecording())
			GetIEditor()->BeginUndo();
		Paint();
		bPainting = true;
	}

	if (event == eMouseMove && (flags&MK_LBUTTON) && !(flags&MK_CONTROL) && !(flags&MK_SHIFT))
	{
		Paint();
		bPainting = true;
	}

	if (!bPainting && GetIEditor()->IsUndoRecording())
	{
		GetIEditor()->AcceptUndo( "Terrain Modify" );
	}
	*/

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexturePainter::Display( DisplayContext &dc )
{
	if (dc.flags & DISPLAY_2D)
		return;

	dc.renderer->SetMaterialColor( 0,1,0,1 );
	dc.DrawTerrainCircle( m_pointerPos,m_brush.radius,0.2f );
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainTexturePainter::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	bool bProcessed = false;
	if (nChar == VK_ADD)
	{
		m_brush.radius += 1;
		bProcessed = true;
	}
	if (nChar == VK_SUBTRACT)
	{
		if (m_brush.radius > 1)
			m_brush.radius -= 1;
		bProcessed = true;
	}
	if (bProcessed && s_toolPanel)
	{
		//s_toolPanel->SetBrush(m_brush);
	}
	if (m_brush.radius < m_brush.minRadius)
		m_brush.radius = m_brush.minRadius;
	if (m_brush.radius > m_brush.maxRadius)
		m_brush.radius = m_brush.maxRadius;

	if (bProcessed)
	{
		if (s_toolPanel)
			s_toolPanel->SetBrush(m_brush);
	}

	return bProcessed;
}


//////////////////////////////////////////////////////////////////////////
void CTerrainTexturePainter::PaintSector( CPoint sector,CPoint texp,CLayer *pLayer )
{
	// Check for outside valid sectors.
	if (sector.x < 0 || sector.x >= m_numSectors || sector.y < 0 || sector.y >= m_numSectors)
		return;

	// Size of brush radius in pixels.
	float radius = m_brush.radius*m_pixelsPerMeter;

	//////////////////////////////////////////////////////////////////////////
	SSectorInfo sectorInfo;
	m_heightmap->GetSectorsInfo( sectorInfo );

	CPoint ofsp;
	ofsp.x = texp.x - sector.x*m_sectorTexSize;
	ofsp.y = texp.y - sector.y*m_sectorTexSize;

	int x1 = max(ofsp.x-radius,0);
	int y1 = max(ofsp.y-radius,0);
	int x2 = min(ofsp.x+radius,m_sectorTexSize);
	int y2 = min(ofsp.y+radius,m_sectorTexSize);

	int sx = x2 - x1;
	int sy = y2 - y1;

	if (sx <= 0 || sy <= 0)
		return;

	// Invalidate sector on this layer.
	pLayer->InvalidateMaskSector(sector);

	// Texture id already present, need to load to it uncompressed copy.
	CImage image;
	image.Allocate( sx,sy );

	int commonGenFlags = ETTG_QUIET|ETTG_LIGHTING|ETTG_KEEP_LAYERMASKS|ETTG_USE_LIGHTMAPS;
	if (m_brush.bUpdateVegetation)
	{
		commonGenFlags |= ETTG_STATOBJ_PAINTBRIGHTNESS;
		GetIEditor()->GetVegetationMap()->SetUpdateOnPaintBrightness(true);
	}
	if (!m_brush.bPreciseLighting)
		commonGenFlags |= ETTG_FAST_LLIGHTING;
	if (!m_brush.bTerrainShadows)
		commonGenFlags |= ETTG_NO_TERRAIN_SHADOWS;
	if (m_brush.bObjectShadows)
		commonGenFlags |= ETTG_STATOBJ_SHADOWS;
	
	bool bFirstLock = false;
	CTerrainSector *terrSect = m_terrainGrid->GetSector( sector );
	if (terrSect->textureId == 0)
		bFirstLock = true;
	int texId = m_terrainGrid->LockSectorTexture( sector,m_terrTexGen,commonGenFlags );

	int sectorGenFlags = commonGenFlags;
	CRect rect( x1,y1,x2,y2 );
	m_terrTexGen.GenerateSectorTexture( sector,rect,sectorGenFlags|ETTG_ABGR,image );

	m_renderer->UpdateTextureInVideoMemory( texId,(unsigned char*)image.GetData(),x1,y1,sx,sy,eTF_8888 );

	// Update vegetation on this sector.
	if (m_brush.bUpdateVegetation)
	{
		GetIEditor()->GetVegetationMap()->SetUpdateOnPaintBrightness(false);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexturePainter::Paint()
{
	SSectorInfo sectorInfo;
	m_heightmap->GetSectorsInfo( sectorInfo );

	CPoint texp = m_terrainGrid->WorldToTexture( m_pointerPos );

	// Size of brush radius in pixels.
	float radius = m_brush.radius*m_pixelsPerMeter;

	//////////////////////////////////////////////////////////////////////////
	// Paint spot on selected layer.
	//////////////////////////////////////////////////////////////////////////
	CLayer *pLayer = GetSelectedLayer();
	if (!pLayer || pLayer->IsAutoGen())
		return;

	static bool bPaintLock = false;
	if (bPaintLock)
		return;

	bPaintLock = true;
	
	CImagePainter layerPainter;
	CImagePainter::SPaintBrush br;
	br.type = CImagePainter::PAINT_BRUSH;
	if (m_brush.type == ET_BRUSH_SMOOTH)
		br.type = CImagePainter::SMOOTH_BRUSH;
	br.radius = radius;
	br.hardness = m_brush.hardness;
	br.color = m_brush.value;
	if (m_brush.bErase)
		br.color = 0;

	// Paint spot on layer mask.
	CByteImage &layerMask = pLayer->GetMask();
	layerPainter.PaintBrush( layerMask,texp.x,texp.y,br );
	
	//////////////////////////////////////////////////////////////////////////
	// Update Terrain textures.
	//////////////////////////////////////////////////////////////////////////
	CPoint sector[4];
	sector[0] = m_terrainGrid->WorldToSector( m_pointerPos+Vec3(-m_brush.radius,-m_brush.radius,0) );
	sector[1] = m_terrainGrid->WorldToSector( m_pointerPos+Vec3(m_brush.radius,-m_brush.radius,0) );
	sector[2] = m_terrainGrid->WorldToSector( m_pointerPos+Vec3(m_brush.radius,m_brush.radius,0) );
	sector[3] = m_terrainGrid->WorldToSector( m_pointerPos+Vec3(-m_brush.radius,m_brush.radius,0) );

	PaintSector( sector[0],texp,pLayer );
	if (sector[1] != sector[0])
		PaintSector( sector[1],texp,pLayer );
	if (sector[2] != sector[0] && sector[2] != sector[1])
		PaintSector( sector[2],texp,pLayer );
	if (sector[3] != sector[0] && sector[3] != sector[1] && sector[3] != sector[2])
		PaintSector( sector[3],texp,pLayer );

	//////////////////////////////////////////////////////////////////////////
	// Update surface types.
	//////////////////////////////////////////////////////////////////////////
	// Build rectangle in heightmap coordinates.
	int hmapWidth = m_heightmap->GetWidth();
	int hmapHeight = m_heightmap->GetHeight();
	int unitSize = m_heightmap->GetUnitSize();
	int hradius = (m_brush.radius+2) / unitSize;
	CPoint hpos = m_heightmap->WorldToHmap(m_pointerPos);
	CRect hmaprc;
	hradius = max(hradius,1);
	hmaprc.left = max(hpos.x-hradius,0);
	hmaprc.top = max(hpos.y-hradius,0);
	hmaprc.right = min(hpos.x+hradius,hmapWidth);
	hmaprc.bottom = min(hpos.y+hradius,hmapHeight);

	// Calculate surface type for this block.
	m_heightmap->CalcSurfaceTypes( &hmaprc );
	// Update surface types at 3d engine terrain.
	m_heightmap->UpdateEngineTerrain( hmaprc.left,hmaprc.top,hmaprc.Width(),hmaprc.Height(),false,true );

	bPaintLock = false;
}

//////////////////////////////////////////////////////////////////////////
CLayer* CTerrainTexturePainter::GetSelectedLayer() const
{
	CString selLayer = s_toolPanel->GetSelectedLayer();
	CCryEditDoc *pDoc = GetIEditor()->GetDocument();
	for (int i = 0; i < pDoc->GetLayerCount(); i++)
	{
		CLayer *pLayer = pDoc->GetLayer(i);
		if (selLayer == pLayer->GetLayerName())
		{
			return pLayer;
		}
	}
	return 0;
}
