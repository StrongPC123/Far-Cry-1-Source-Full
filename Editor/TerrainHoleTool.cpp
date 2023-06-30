////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TerrainHoleTool.cpp
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain Modification Tool implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TerrainHoleTool.h"
#include "Viewport.h"
#include "TerrainHolePanel.h"
#include "Heightmap.h"
#include "Objects\DisplayContext.h"

#include "I3DEngine.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CTerrainHoleTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CTerrainHoleTool::CTerrainHoleTool()
{
	m_panelId = 0;
	m_panel = 0;
	
	m_pointerPos(0,0,0);

	m_bMakeHole = true;
	m_brushRadius = 1;
	GetIEditor()->ClearSelection();
}

//////////////////////////////////////////////////////////////////////////
CTerrainHoleTool::~CTerrainHoleTool()
{
	m_pointerPos(0,0,0);
	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->CancelUndo();
}

//////////////////////////////////////////////////////////////////////////
void CTerrainHoleTool::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	if (!m_panelId)
	{
		m_panel = new CTerrainHolePanel(this,AfxGetMainWnd());
		m_panelId = m_ie->AddRollUpPage( ROLLUP_TERRAIN,"Modify Terrain",m_panel );
		AfxGetMainWnd()->SetFocus();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainHoleTool::EndEditParams()
{
	if (m_panelId)
	{
		m_ie->RemoveRollUpPage(ROLLUP_TERRAIN,m_panelId);
		m_panel = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainHoleTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	m_pointerPos = view->ViewToWorld( point,0,true );
	if (event == eMouseLDown || (event == eMouseMove && (flags&MK_LBUTTON)))
	{
		/*
		if (flags&MK_CONTROL)
		{
			bool bMakeHole = m_bMakeHole;
			m_bMakeHole = false;
			Modify();
			m_bMakeHole = bMakeHole;
		}
		else
		{
			Modify();
		}
		*/
		if (!GetIEditor()->IsUndoRecording())
			GetIEditor()->BeginUndo();
		Modify();
	}
	else
	{
		if (GetIEditor()->IsUndoRecording())
			GetIEditor()->AcceptUndo( "Terrain Hole" );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainHoleTool::Display( DisplayContext &dc )
{
	if (dc.flags & DISPLAY_2D)
		return;

	CHeightmap *heightmap = m_ie->GetHeightmap();
	int unitSize = heightmap->GetUnitSize();

	dc.renderer->SetMaterialColor( 0,1,0,1 );
	dc.DrawTerrainCircle( m_pointerPos,m_brushRadius,0.2f );

	float fx1 = (m_pointerPos.y - m_brushRadius)/unitSize;
	float fy1 = (m_pointerPos.x - m_brushRadius)/unitSize;
	float fx2 = (m_pointerPos.y + m_brushRadius)/unitSize;
	float fy2 = (m_pointerPos.x + m_brushRadius)/unitSize;


	int x1 = MAX(fx1,0);
	int y1 = MAX(fy1,0);
	int x2 = MIN(fx2,heightmap->GetWidth()-1);
	int y2 = MIN(fy2,heightmap->GetHeight()-1);

	if (m_bMakeHole)
		dc.renderer->SetMaterialColor( 1,0,0,1 );
	else
		dc.renderer->SetMaterialColor( 0,0,1,1 );
	Vec3 p1,p2,p3,p4;
	// Make hole.
	for (int x = x1; x <= x2; x++)
	{
		for (int y = y1; y <= y2; y++)
		{
			p1.x = y*unitSize;
			p1.y = x*unitSize;
			
			p2.x = y*unitSize+unitSize;
			p2.y = x*unitSize;
			
			p3.x = y*unitSize+unitSize;
			p3.y = x*unitSize+unitSize;
			
			p4.x = y*unitSize;
			p4.y = x*unitSize+unitSize;

			p1.z = dc.engine->GetTerrainElevation(p1.x,p1.y)+0.2f;
			p2.z = dc.engine->GetTerrainElevation(p2.x,p2.y)+0.2f;
			p3.z = dc.engine->GetTerrainElevation(p3.x,p3.y)+0.2f;
			p4.z = dc.engine->GetTerrainElevation(p4.x,p4.y)+0.2f;
			dc.DrawLine( p1,p2 );
			dc.DrawLine( p2,p3 );
			dc.DrawLine( p3,p4 );
			dc.DrawLine( p1,p4 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainHoleTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	bool bProcessed = false;
	if (nChar == VK_ADD)
	{
		if (m_brushRadius < 100)
			m_brushRadius += 0.5f;
		bProcessed = true;
	}
	if (nChar == VK_SUBTRACT)
	{
		if (m_brushRadius > 0.5f)
			m_brushRadius -= 0.5f;
		bProcessed = true;
	}
	if (nChar == VK_CONTROL && !(nFlags&(1<<14))) // only once (no repeat).
	{
		m_bMakeHole = !m_bMakeHole;
		m_panel->SetMakeHole(m_bMakeHole);
	}
	if (bProcessed && m_panel)
	{
		m_panel->m_radius.SetPos(m_brushRadius);
	}
	return bProcessed;
}

bool CTerrainHoleTool::OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	if (nChar == VK_CONTROL)
	{
		m_bMakeHole = !m_bMakeHole;
		m_panel->SetMakeHole(m_bMakeHole);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainHoleTool::Modify()
{
	CHeightmap *heightmap = m_ie->GetHeightmap();
	int unitSize = heightmap->GetUnitSize();

	float fx1 = (m_pointerPos.y - m_brushRadius)/unitSize;
	float fy1 = (m_pointerPos.x - m_brushRadius)/unitSize;
	float fx2 = (m_pointerPos.y + m_brushRadius)/unitSize;
	float fy2 = (m_pointerPos.x + m_brushRadius)/unitSize;

	int x1 = MAX(fx1,0);
	int y1 = MAX(fy1,0);
	int x2 = MIN(fx2,heightmap->GetWidth()-1);
	int y2 = MIN(fy2,heightmap->GetHeight()-1);

	heightmap->MakeHole( x1,y1,x2-x1,y2-y1,m_bMakeHole );
}
