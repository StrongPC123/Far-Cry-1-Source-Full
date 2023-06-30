////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TextureTool.cpp
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain Modification Tool implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TextureTool.h"
#include "Viewport.h"
#include "TexturePanel.h"
#include "Objects\DisplayContext.h"

#include <I3DEngine.h>
//#include <I3DIndoorEngine.h>

// Access internals of 3d engine.
#include "MeshIdx.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CTextureTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CTextureTool::CTextureTool()
{
	m_panelId = 0;
	m_panel = 0;
	
	m_pointerPos(0,0,0);

	m_bRemoveSelection = false;
	m_brushRadius = 1;
	GetIEditor()->ClearSelection();
}

//////////////////////////////////////////////////////////////////////////
CTextureTool::~CTextureTool()
{
}

//////////////////////////////////////////////////////////////////////////
void CTextureTool::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	if (!m_panelId)
	{
		m_panel = new CTexturePanel(this,AfxGetMainWnd());
		m_panelId = m_ie->AddRollUpPage( ROLLUP_TERRAIN,"Texture Properties",m_panel );
		AfxGetMainWnd()->SetFocus();
	}
}

//////////////////////////////////////////////////////////////////////////
void CTextureTool::EndEditParams()
{
	if (m_panelId)
	{
		m_ie->RemoveRollUpPage(ROLLUP_TERRAIN,m_panelId);
		m_panel = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTextureTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	m_pointerPos = view->ViewToWorld( point );
	if (event == eMouseLDown/* || (event == eMouseMove && (flags&MK_LBUTTON))*/)
	{
		Vec3 raySrc,rayDir;
		view->ViewToWorldRay( point,raySrc,rayDir );
		
		bool removeSelection = false;
		PickSelection( raySrc,rayDir );
		
		if (flags&MK_CONTROL)
		{
			removeSelection = true;
		}
		PickSelection( raySrc,rayDir,removeSelection );
	}
	else
	{
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTextureTool::Display( DisplayContext &dc )
{
	if (dc.flags & DISPLAY_2D)
		return;

	dc.SetColor( 1,0,0,0.2f );

	Vec3 v[3];
	int i;
	// Render selection.
	for (i = 0; i < m_selection.size(); i++)
	{
		SelectedElem &elem = m_selection[i];
		dc.DrawTri( elem.v[0],elem.v[1],elem.v[2] );
	}

	dc.SetColor( 1,1,1,0.8f );
	for (i = 0; i < m_selection.size(); i++)
	{
		SelectedElem &elem = m_selection[i];
		dc.DrawLine( elem.v[0],elem.v[1] );
		dc.DrawLine( elem.v[1],elem.v[2] );
		dc.DrawLine( elem.v[0],elem.v[2] );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTextureTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
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
		//m_bRemoveSelection = !m_bRemoveSelection;
		//m_panel->SetMakeHole(m_bMakeHole);
	}
	if (bProcessed && m_panel)
	{
		//m_panel->m_radius.SetPos(m_brushRadius);
	}
	return bProcessed;
}

bool CTextureTool::OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	if (nChar == VK_CONTROL)
	{
		//m_bMakeHole = !m_bMakeHole;
		//m_panel->SetMakeHole(m_bMakeHole);
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CTextureTool::Modify()
{
	
}

//////////////////////////////////////////////////////////////////////////
void CTextureTool::PickSelection( Vec3 &rayOrg,Vec3 &rayDir,bool removeSelection )
{
	/*
	IIndoorBase *indoor = GetIEditor()->Get3DEngine()->GetBuildingManager();

	IndoorRayIntInfo hit;
	Vec3 rayTrg = rayOrg + rayDir*1000.0f;

	m_raySrc = rayOrg;
	m_rayTrg = rayTrg;

	return;
	ASSERT(0);
	//please use the physics code for ray-intersection
	//if (!indoor->RayIntersection( rayOrg,rayTrg,hit ))
	//return;

	if (removeSelection)
	{
		int index = GetSelection( hit.pObj,hit.nFaceIndex );
		if (index >= 0)
			m_selection.erase( m_selection.begin()+index );
	}
	else
	{	
		// Check if this face already selected.
		int index = GetSelection( hit.pObj,hit.nFaceIndex );
		if (index >= 0)
			return;

		SelectedElem elem;
		elem.face = hit.nFaceIndex;
		elem.pos = indoor->GetPosition( hit.nBuildId );
		elem.object = hit.pObj;
		
		CIndexedMesh *mesh = elem.object->GetTriData();
		assert( elem.face >= 0 && elem.face < mesh->m_nFaceCount );
		CObjFace *face = &mesh->m_pFaces[elem.face];
		//face->
		elem.v[0] = mesh->m_pVerts[face->v[0]] + elem.pos + face->m_Plane.n*0.01f;
		elem.v[1] = mesh->m_pVerts[face->v[1]] + elem.pos + face->m_Plane.n*0.01f;
		elem.v[2] = mesh->m_pVerts[face->v[2]] + elem.pos + face->m_Plane.n*0.01f;
		
		m_selection.push_back( elem );
	}
	*/
}

int	CTextureTool::GetSelection( IStatObj *obj,int faceIndex ) const
{
	for (int i = 0; i < m_selection.size(); i++)
	{
		const SelectedElem &elem = m_selection[i];
		if (elem.object == obj && elem.face == faceIndex)
		{
			return i;
		}
	}
	return -1;
}