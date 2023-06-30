////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TerrainMoveTool.cpp
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain Modification Tool implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TerrainMoveTool.h"
#include "Viewport.h"
#include "Heightmap.h"
#include "Objects\ObjectManager.h"

#include "I3DEngine.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CTerrainMoveTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CTerrainMoveTool::CTerrainMoveTool()
{
	m_pointerPos(0,0,0);
	m_archive = 0;
}

//////////////////////////////////////////////////////////////////////////
CTerrainMoveTool::~CTerrainMoveTool()
{
	m_pointerPos(0,0,0);
	if (m_archive)
		delete m_archive;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainMoveTool::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;

	if (!m_archive)
	{
		m_archive = new CXmlArchive("Root");
		// Save area to archive.
		BBox srcBox;
		GetIEditor()->GetSelectedRegion( srcBox );

		// Move terrain heightmap block.
		CPoint hmapSrcMin,hmapSrcMax;
		hmapSrcMin = GetIEditor()->GetHeightmap()->WorldToHmap(srcBox.min);
		hmapSrcMax = GetIEditor()->GetHeightmap()->WorldToHmap(srcBox.max);
		m_srcRect.SetRect( hmapSrcMin,hmapSrcMax );

		GetIEditor()->GetHeightmap()->ExportBlock( m_srcRect,*m_archive );
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainMoveTool::EndEditParams()
{
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainMoveTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	m_pointerPos = view->ViewToWorld( point,0,true );

	BBox box;
	GetIEditor()->GetSelectedRegion(box);

	if (event == eMouseLDown)
	{
		// Move terrain area.
		Move();
		// Close tool.
		GetIEditor()->SetEditTool(0);
	}
	else
	{
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainMoveTool::Display( DisplayContext &dc )
{
	BBox box;
	GetIEditor()->GetSelectedRegion(box);

	Vec3 p1 = GetIEditor()->GetHeightmap()->HmapToWorld( CPoint(m_srcRect.left,m_srcRect.top) );
	Vec3 p2 = GetIEditor()->GetHeightmap()->HmapToWorld( CPoint(m_srcRect.right,m_srcRect.bottom) );

	Vec3 ofs = m_pointerPos - p1;
	p1 += ofs;
	p2 += ofs;

	dc.SetColor( RGB(0,0,255) );
	dc.DrawTerrainRect( p1.x,p1.y,p2.x,p2.y,0.2f );
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainMoveTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	bool bProcessed = false;
	return bProcessed;
}

bool CTerrainMoveTool::OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainMoveTool::Move()
{
	// Move terrain area.
	CUndo undo( "Copy Area" );
	CWaitCursor wait;

	CHeightmap *pHeightamp = GetIEditor()->GetHeightmap();
	assert( pHeightamp );

	// Switch archive to loading.
	m_archive->bLoading = true;

	// Move terrain heightmap block.
	CPoint hmapPos;
	hmapPos = pHeightamp->WorldToHmap(m_pointerPos);

	CPoint offset = pHeightamp->ImportBlock( *m_archive,hmapPos );
	Vec3 moveOffset = pHeightamp->HmapToWorld(offset);

	// Load selection from archive.
	XmlNodeRef objRoot = m_archive->root->findChild("Objects");
	if (objRoot)
	{
		GetIEditor()->ClearSelection();
		CObjectArchive ar( GetIEditor()->GetObjectManager(),objRoot,true );
		GetIEditor()->GetObjectManager()->LoadObjects( ar,false );
	}

	// Move all objects.
	GetIEditor()->GetSelection()->Move( moveOffset,false,true );
}

//////////////////////////////////////////////////////////////////////////
void CTerrainMoveTool::SetArchive( CXmlArchive *ar )
{
	if (m_archive)
		delete m_archive;
	m_archive = ar;

	int x1,y1,x2,y2;
	// Load rect size our of archive.
	m_archive->root->getAttr( "X1",x1 );
	m_archive->root->getAttr( "Y1",y1 );
	m_archive->root->getAttr( "X2",x2 );
	m_archive->root->getAttr( "Y2",y2 );

	m_srcRect.SetRect( x1,y1,x2,y2 );
}