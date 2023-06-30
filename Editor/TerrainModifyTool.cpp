////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TerrainModifyTool.cpp
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain Modification Tool implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "INITGUID.H"
#include "TerrainModifyTool.h"
#include "TerrainPanel.h"
#include "Viewport.h"
#include "TerrainModifyPanel.h"
#include "Heightmap.h"
#include "Objects\DisplayContext.h"
#include "Objects\ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CTerrainModifyTool,CEditTool)

CTerrainBrush CTerrainModifyTool::m_brush[eBrushTypeLast];
BrushType			CTerrainModifyTool::m_currentBrushType = eBrushFlatten;

//////////////////////////////////////////////////////////////////////////
CTerrainModifyTool::CTerrainModifyTool()
{
	SetStatusText( _T("Modify Terrain Heightmap") );
	m_panelId = 0;
	m_panel = 0;

	m_bSmoothOverride = false;
	m_bQueryHeightMode = false;
	m_bPaintingMode = false;
	m_bLowerTerrain = false;

	m_brush[eBrushFlatten].type = eBrushFlatten;
	m_brush[eBrushSmooth].type = eBrushSmooth;
	m_brush[eBrushRiseLower].type = eBrushRiseLower;
	m_pBrush = &m_brush[m_currentBrushType];
	
	m_pointerPos(0,0,0);
	GetIEditor()->ClearSelection();

	if (m_pBrush->height < 0)
	{
		// Initialize brush height to water level.
		m_pBrush->height = GetIEditor()->GetHeightmap()->GetWaterLevel();
	}

	m_hPickCursor = AfxGetApp()->LoadCursor( IDC_POINTER_GET_HEIGHT );
	m_hPaintCursor = AfxGetApp()->LoadCursor( IDC_HAND_INTERNAL );
	m_hFlattenCursor = AfxGetApp()->LoadCursor( IDC_POINTER_FLATTEN );
	m_hSmoothCursor = AfxGetApp()->LoadCursor( IDC_POINTER_SMOOTH );
}

//////////////////////////////////////////////////////////////////////////
CTerrainModifyTool::~CTerrainModifyTool()
{
	m_pointerPos(0,0,0);

	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->CancelUndo();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	if (!m_panelId)
	{
		m_panel = new CTerrainModifyPanel(this,AfxGetMainWnd());
		m_panelId = m_ie->AddRollUpPage( ROLLUP_TERRAIN,"Modify Terrain",m_panel );
		AfxGetMainWnd()->SetFocus();

		m_panel->SetBrush(*m_pBrush);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::EndEditParams()
{
	if (m_panelId)
	{
		m_ie->RemoveRollUpPage(ROLLUP_TERRAIN,m_panelId);
		m_panel = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainModifyTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	m_bSmoothOverride = false;
	if (flags & MK_SHIFT)
	{
		m_bSmoothOverride = true;
	}
	m_bPaintingMode = false;
	m_bQueryHeightMode = false;
	m_bLowerTerrain = false;
	if (flags&MK_CONTROL)
	{
		if (m_pBrush->type != eBrushRiseLower) // Rise/Lower brush do not support picking.
			m_bQueryHeightMode = true;
		else
			m_bLowerTerrain = true;
	}

	bool bPainting = false;
	m_pointerPos = view->ViewToWorld( point,0,true );
	if ((event == eMouseLDown || (event == eMouseMove && (flags&MK_LBUTTON))) && m_bQueryHeightMode)
	{
		m_pBrush->height = GetIEditor()->GetTerrainElevation(m_pointerPos.x,m_pointerPos.y);
		UpdateUI();
	}
	else if (event == eMouseLDown)
	{
		if (!GetIEditor()->IsUndoRecording())
			GetIEditor()->BeginUndo();
		Paint();
		bPainting = true;
	}

	//if (event == eMouseMove && (flags&MK_LBUTTON) && !(flags&MK_CONTROL) && !(flags&MK_SHIFT))
	if (event == eMouseMove && (flags&MK_LBUTTON) && !m_bQueryHeightMode)
	{
		Paint();
		bPainting = true;
	}

	if (event == eMouseMDown)
	{
		// When middle mouse button down.
		m_MButtonDownPos = point;
		m_prevRadius = m_pBrush->radius;
		m_prevRadiusInside = m_pBrush->radiusInside;
		m_prevHeight = m_pBrush->height;
	}
	if (event == eMouseMove && (flags&MK_MBUTTON) && (flags&MK_SHIFT) && !bPainting)
	{
		// Change brush radius.
		CPoint p = point - m_MButtonDownPos;
		//float dist = sqrtf(p.x*p.x + p.y*p.y);
		m_pBrush->radius = m_prevRadius + p.x * 0.1f;
		m_pBrush->radiusInside = m_prevRadiusInside - p.y * 0.1f;
		if (m_pBrush->radiusInside > m_pBrush->radius)
			m_pBrush->radiusInside = m_pBrush->radius;
		UpdateUI();
	}
	if (event == eMouseMove && (flags&MK_MBUTTON) && (flags&MK_CONTROL) && !bPainting)
	{
		// Change brush radius.
		CPoint p = point - m_MButtonDownPos;
		float dist = sqrtf(p.x*p.x + p.y*p.y);
		m_pBrush->height = m_prevHeight - p.y * 0.1f;
		if (m_pBrush->height < 0)
			m_pBrush->height = 0;
		UpdateUI();
	}

	m_bPaintingMode = bPainting;

	if (!bPainting && GetIEditor()->IsUndoRecording())
	{
		GetIEditor()->AcceptUndo( "Terrain Modify" );
	}

	// Show status help.
	if (m_pBrush->type == eBrushRiseLower)
		GetIEditor()->SetStatusText( "CTRL: Inverse Height  SHIFT: Smooth  LMB: Rise/Lower/Smooth  +-: Change Brush Radius  */: Change Height" );
	else
		GetIEditor()->SetStatusText( "CTRL: Query Height  SHIFT: Smooth  LMB: Flatten/Smooth  +-: Change Brush Radius  */: Change Height" );

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::Display( DisplayContext &dc )
{
	if (dc.flags & DISPLAY_2D)
		return;

	if (m_pBrush->type != eBrushSmooth)
	{
		dc.SetColor( 0.5f,1,0.5f,1 );
		dc.DrawTerrainCircle( m_pointerPos,m_pBrush->radiusInside,0.2f );
	}
	dc.SetColor( 0,1,0,1 );
	dc.DrawTerrainCircle( m_pointerPos,m_pBrush->radius,0.2f );
	if (m_pointerPos.z < m_pBrush->height)
	{
		if (m_pBrush->type != eBrushSmooth)
		{
			Vec3 p = m_pointerPos;
			p.z = m_pBrush->height;
			dc.SetColor( 1,1,0,1 );
			if (m_pBrush->type == eBrushFlatten)
				dc.DrawTerrainCircle( p,m_pBrush->radius,m_pBrush->height-m_pointerPos.z );
			dc.DrawLine( m_pointerPos,p );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainModifyTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	bool bProcessed = false;
	if (nChar == VK_MULTIPLY)
	{
		m_pBrush->height += 1;
		bProcessed = true;
	}
	if (nChar == VK_DIVIDE)
	{
		if (m_pBrush->height > 0)
			m_pBrush->height -= 1;
		bProcessed = true;
	}
	if (nChar == VK_ADD)
	{
		m_pBrush->radius += 1;
		m_pBrush->radiusInside += 1;
		bProcessed = true;
	}
	if (nChar == VK_SUBTRACT)
	{
		if (m_pBrush->radius > 1)
			m_pBrush->radius -= 1;
		if (m_pBrush->radiusInside > 0)
			m_pBrush->radiusInside -= 1;
		bProcessed = true;
	}
	if (bProcessed)
	{
		UpdateUI();
	}
	return bProcessed;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::UpdateUI()
{
	if (m_panel)
	{
		m_panel->SetBrush(*m_pBrush);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::Paint()
{
	CHeightmap *heightmap = GetIEditor()->GetHeightmap();
	int unitSize = heightmap->GetUnitSize();

	//dc.renderer->SetMaterialColor( 1,1,0,1 );
	int tx = RoundFloatToInt(m_pointerPos.y / unitSize);
	int ty = RoundFloatToInt(m_pointerPos.x / unitSize);

	float fInsideRadius = (m_pBrush->radiusInside / unitSize);
	int tsize = (m_pBrush->radius / unitSize);
	if (tsize == 0)
		tsize = 1;
	int tsize2 = tsize*2;
	int x1 = tx - tsize;
	int y1 = ty - tsize;

	if (m_pBrush->type == eBrushFlatten && !m_bSmoothOverride)
		heightmap->DrawSpot2( tx,ty,tsize,fInsideRadius,m_pBrush->height,m_pBrush->hardness,m_pBrush->bNoise,m_pBrush->noiseFreq/10.0f,m_pBrush->noiseScale/1000.0f );
	if (m_pBrush->type == eBrushRiseLower && !m_bSmoothOverride)
	{
		float h = m_pBrush->height;
		if (m_bLowerTerrain)
			h = -h;
		heightmap->RiseLowerSpot( tx,ty,tsize,fInsideRadius,h,m_pBrush->hardness,m_pBrush->bNoise,m_pBrush->noiseFreq/10.0f,m_pBrush->noiseScale/1000.0f );
	}
	else if (m_pBrush->type == eBrushSmooth || m_bSmoothOverride)
		heightmap->SmoothSpot( tx,ty,tsize,m_pBrush->height,m_pBrush->hardness );//,m_pBrush->noiseFreq/10.0f,m_pBrush->noiseScale/10.0f );

	heightmap->UpdateEngineTerrain( x1,y1,tsize2,tsize2,true,false );

	if (m_pBrush->bRepositionObjects)
	{
		BBox box;
		box.min = m_pointerPos - Vec3(m_pBrush->radius,m_pBrush->radius,0);
		box.max = m_pointerPos + Vec3(m_pBrush->radius,m_pBrush->radius,0);
		box.min.z -= 10000;
		box.max.z += 10000;
		// Make sure objects preserve height.
		GetIEditor()->GetObjectManager()->SendEvent( EVENT_KEEP_HEIGHT,box );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainModifyTool::OnSetCursor( CViewport *vp )
{
	if (m_bQueryHeightMode)
	{
		SetCursor( m_hPickCursor );
		return true;
	}
	/*
	if (m_bPaintingMode)
	{
		SetCursor( m_hPaintCursor );
		return true;
	}
	*/
	if ((m_pBrush->type == eBrushFlatten || m_pBrush->type == eBrushRiseLower) && !m_bSmoothOverride)
	{
		SetCursor( m_hFlattenCursor );
		return true;
	}
	else if (m_pBrush->type == eBrushSmooth || m_bSmoothOverride)
	{
		SetCursor( m_hSmoothCursor );
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::SetActiveBrushType( BrushType type  )
{
	m_currentBrushType = type;
	m_pBrush = &m_brush[m_currentBrushType];
	if (m_panel)
	{
		CTerrainBrush brush;
		GetBrush( brush );
		m_panel->SetBrush( brush );
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::Command_Activate()
{
	CEditTool *pTool = GetIEditor()->GetEditTool();
	if (pTool && pTool->IsKindOf(RUNTIME_CLASS(CTerrainModifyTool)))
	{
		// Already active.
		return;
	}
	pTool = new CTerrainModifyTool;
	GetIEditor()->SetEditTool( pTool );
	GetIEditor()->SelectRollUpBar( ROLLUP_TERRAIN );
	AfxGetMainWnd()->RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::Command_Flatten()
{
	Command_Activate();
	CEditTool *pTool = GetIEditor()->GetEditTool();
	if (pTool && pTool->IsKindOf(RUNTIME_CLASS(CTerrainModifyTool)))
	{
		CTerrainModifyTool *pModTool = (CTerrainModifyTool*)pTool;
		pModTool->SetActiveBrushType(eBrushFlatten);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::Command_Smooth()
{
	Command_Activate();
	CEditTool *pTool = GetIEditor()->GetEditTool();
	if (pTool && pTool->IsKindOf(RUNTIME_CLASS(CTerrainModifyTool)))
	{
		CTerrainModifyTool *pModTool = (CTerrainModifyTool*)pTool;
		pModTool->SetActiveBrushType(eBrushSmooth);
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::Command_RiseLower()
{
	Command_Activate();
	CEditTool *pTool = GetIEditor()->GetEditTool();
	if (pTool && pTool->IsKindOf(RUNTIME_CLASS(CTerrainModifyTool)))
	{
		CTerrainModifyTool *pModTool = (CTerrainModifyTool*)pTool;
		pModTool->SetActiveBrushType(eBrushRiseLower);
	}
}

//////////////////////////////////////////////////////////////////////////
// Class description.
//////////////////////////////////////////////////////////////////////////
class CTerrainModifyTool_ClassDesc : public CRefCountClassDesc
{
	//! This method returns an Editor defined GUID describing the class this plugin class is associated with.
	virtual ESystemClassID SystemClassID() { return ESYSTEM_CLASS_EDITTOOL; }

	//! Return the GUID of the class created by plugin.
	virtual REFGUID ClassID() 
	{
		return TERRAIN_MODIFY_TOOL_GUID;
	}

	//! This method returns the human readable name of the class.
	virtual const char* ClassName() { return "TerrainModifyTool"; };

	//! This method returns Category of this class, Category is specifing where this plugin class fits best in
	//! create panel.
	virtual const char* Category() { return "Terrain"; };
	//////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
void CTerrainModifyTool::RegisterTool( CRegistrationContext &rc )
{
	rc.pClassFactory->RegisterClass( new CTerrainModifyTool_ClassDesc );
	
	rc.pCommandManager->RegisterCommand( "EditTool.TerrainModifyTool.Activate",functor(CTerrainModifyTool::Command_Activate) );
	rc.pCommandManager->RegisterCommand( "EditTool.TerrainModifyTool.Flatten",functor(CTerrainModifyTool::Command_Flatten) );
	rc.pCommandManager->RegisterCommand( "EditTool.TerrainModifyTool.Smooth",functor(CTerrainModifyTool::Command_Smooth) );
	rc.pCommandManager->RegisterCommand( "EditTool.TerrainModifyTool.RiseLower",functor(CTerrainModifyTool::Command_RiseLower) );
}
