`////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   BrushTool.cpp
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Terrain Modification Tool implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BrushTool.h"

#include "..\Viewport.h"

#include "Brush.h"

#include "..\Objects\ObjectManager.h"
#include "..\Objects\BrushObject.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CBrushTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CBrushTool::CBrushTool()
{
	m_IEditor = 0;
	m_mode = BrushNoMode;
	m_bMouseCaptured = false;
	m_lastBrushBounds = BBox( Vec3(-32,-32,-32),Vec3(32,32,32) );
}

//////////////////////////////////////////////////////////////////////////
CBrushTool::~CBrushTool()
{
}

//////////////////////////////////////////////////////////////////////////
void CBrushTool::BeginEditParams( IEditor *ie,int flags )
{
	m_IEditor = ie;
}

//////////////////////////////////////////////////////////////////////////
void CBrushTool::EndEditParams()
{
}

void::CBrushTool::Display( DisplayContext &dc )
{
	CalcLastBrushSize();
}

bool CBrushTool::OnLButtonDown( CViewport *view,UINT nFlags,CPoint point )
{
	m_mode = BrushNoMode;

	m_mouseDownPos = point;

	if (nFlags & MK_SHIFT)
	{
		// Enable Create or sheer mode.
		if (GetIEditor()->GetSelection()->IsEmpty())
		{
			// Create new brush.
			m_mode = BrushCreateMode;
			view->BeginUndo();
			view->CaptureMouse();
			m_bMouseCaptured = true;
			return true;
		}
	}

	bool bMouseClickedSelected = false;
	// If clicked within selected brush. move this brush.
	ObjectHitInfo hitInfo(view,point);
	if (view->HitTest( point,hitInfo ))
	{
		if (hitInfo.object->GetType() == OBJTYPE_BRUSH && hitInfo.object->IsSelected())
		{
			bMouseClickedSelected = true;

			// Only Left buttons should be pressed (no combinations)
			if (nFlags == MK_LBUTTON)
			{
				m_mode = BrushMoveMode;
				view->BeginUndo();
				view->CaptureMouse();
				m_bMouseCaptured = true;
				return true;
			}
		}
	}

	if (nFlags & MK_SHIFT)
	{
		if (!bMouseClickedSelected)
		{
			// Clicked outside of selected objects.
			// So start streching brushes.
			//GetIEditor()->ClearSelection();

			bool bShear = (nFlags & MK_CONTROL) != 0;

			m_mode = BrushStretchMode;
			view->BeginUndo();
			view->CaptureMouse();
			m_bMouseCaptured = true;

			// Select brush drag sides.
			CSelectionGroup *selection = GetIEditor()->GetSelection();
			for (int i = 0; i < selection->GetCount(); i++)
			{
				if (!selection->GetObject(i)->IsKindOf(RUNTIME_CLASS(CBrushObject)))
					continue;
				CBrushObject *brushObj = (CBrushObject*)selection->GetObject(i);
				Vec3 raySrc,rayDir;
				view->ViewToWorldRay( point,raySrc,rayDir );
				brushObj->SelectBrushSide( raySrc,rayDir,bShear );
			}
			m_IEditor->UpdateViews(eUpdateObjects);

			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBrushTool::OnLButtonUp( CViewport *view,UINT nFlags,CPoint point )
{
	bool bResult = false;
	if (m_mode == BrushMoveMode)
	{
		view->AcceptUndo( "Move Brush" );
		bResult = true;
	}
	else if (m_mode == BrushCreateMode)
	{
		view->AcceptUndo( "Create Brush" );
		bResult = true;
	}
	else if (m_mode == BrushStretchMode)
	{
		view->AcceptUndo( "Sheer Brush(s)" );
		bResult = true;
	}
	if (m_bMouseCaptured)
	{
		view->ReleaseMouse();
	}

	m_mode = BrushNoMode;
	return bResult;
}

//////////////////////////////////////////////////////////////////////////
bool CBrushTool::OnMouseMove( CViewport *view,UINT nFlags, CPoint point )
{	
	if (m_mode == BrushMoveMode)
	{
		// Move brush.
		GetIEditor()->RestoreUndo();

		Vec3 p1 = view->MapViewToCP(m_mouseDownPos);
		Vec3 p2 = view->MapViewToCP(point);
		Vec3 v = view->GetCPVector(p1,p2);

		int coordSys = GetIEditor()->GetReferenceCoordSys();
		GetIEditor()->GetSelection()->Move( v,false, coordSys==COORDS_WORLD );
		return true;
	}
	if (m_mode == BrushCreateMode)
	{
		NewBrush( view,point );
		return true;
	}
	else if (m_mode == BrushStretchMode)
	{
		StretchBrush( view,point );
		return true;
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBrushTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	bool bProcessed = false;
	if (event == eMouseLDown)
	{
		bProcessed = OnLButtonDown( view,flags,point );
	}
	else if (event == eMouseLUp)
	{
		bProcessed = OnLButtonUp( view,flags,point );
	}
	else if (event == eMouseMove)
	{
		bProcessed = OnMouseMove( view,flags,point );
	}

	// Not processed.
	return bProcessed;
}

//////////////////////////////////////////////////////////////////////////
bool CBrushTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	if (nChar == VK_ESCAPE)
	{
		// Escape clears selection.
		GetIEditor()->ClearSelection();
		return true;
	}
	// Not processed.
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CBrushTool::OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	// Not processed.
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CBrushTool::NewBrush( CViewport *view,CPoint point )
{
	CRect rc( m_mouseDownPos,point );
	rc.NormalizeRect();

	BBox brushBox;
	brushBox.Reset();
	brushBox.Add( view->MapViewToCP( CPoint(rc.left,rc.top) ) );
	brushBox.Add( view->MapViewToCP( CPoint(rc.right,rc.bottom) ) );

	switch (view->GetType())
	{
	case ET_ViewportXY:
		brushBox.min.z = m_lastBrushBounds.min.z;
		brushBox.max.z = m_lastBrushBounds.max.z;
		break;
	case ET_ViewportXZ:
		brushBox.min.y = m_lastBrushBounds.min.y;
		brushBox.max.y = m_lastBrushBounds.max.y;
		break;
	case ET_ViewportYZ:
		brushBox.min.x = m_lastBrushBounds.min.x;
		brushBox.max.x = m_lastBrushBounds.max.x;
		break;
	default:
		brushBox.min.z = m_lastBrushBounds.min.z;
		brushBox.max.z = m_lastBrushBounds.max.z;
	}

	if (IsVectorsEqual(brushBox.min,brushBox.max))
		return;

	// If width or height or depth are zero.
	if (fabs(brushBox.min.x-brushBox.max.x) < 0.01 ||
			fabs(brushBox.min.y-brushBox.max.y) < 0.01 ||
			fabs(brushBox.min.z-brushBox.max.z) < 0.01)
		return;

	if (IsEquivalent(m_lastBrushBounds.min,brushBox.min,0) && IsEquivalent(m_lastBrushBounds.max,brushBox.max,0))
		return;

	m_lastBrushBounds = brushBox;

	Vec3 center = (brushBox.min + brushBox.max)/2.0f;
	brushBox.min -= center;
	brushBox.max -= center;

	SBrush *brush = new SBrush;
	SMapTexInfo ti;
	brush->Create( brushBox.min,brushBox.max,&ti );
	bool bSolidValid = brush->BuildSolid();

	CBrushObject *brushObj;

	CBaseObject *obj = m_IEditor->GetSelectedObject();
	if (obj && obj->IsKindOf(RUNTIME_CLASS(CBrushObject)))
	{
		brushObj = (CBrushObject*)obj;
	}
	else
	{
		m_IEditor->ClearSelection();
		brushObj = (CBrushObject*)m_IEditor->NewObject( "Brush" );
		m_IEditor->SelectObject( brushObj );
	}
	brushObj->SetPos( center );

	brushObj->SetBrush( brush );
}

//////////////////////////////////////////////////////////////////////////
void CBrushTool::StretchBrush( CViewport* view,CPoint point )
{
	Vec3 src = view->MapViewToCP(m_mouseDownPos);
	Vec3 trg = view->MapViewToCP(point);
	Vec3 delta = trg - src;

	if (IsEquivalent(delta,Vec3(0,0,0),0))
		return;

	m_mouseDownPos = point;

	Vec3 raySrc,rayDir;
	view->ViewToWorldRay( point,raySrc,rayDir );

	CSelectionGroup *selection = GetIEditor()->GetSelection();
	for (int i = 0; i < selection->GetCount(); i++)
	{
		if (!selection->GetObject(i)->IsKindOf(RUNTIME_CLASS(CBrushObject)))
			continue;
		CBrushObject *brushObj = (CBrushObject*)selection->GetObject(i);
		brushObj->MoveSelectedPoints( delta );

		//SBrush *brush = brushObj->GetBrush();
		//brush->SideSelect( 
	}
	view->Invalidate(FALSE);
}

//////////////////////////////////////////////////////////////////////////
void CBrushTool::CalcLastBrushSize()
{
	bool bSelected = false;
	BBox box;
	box.Reset();
	CSelectionGroup *sel = m_IEditor->GetSelection();
	for (int i = 0; i < sel->GetCount(); i++)
	{
		if (sel->GetObject(i)->GetType() == OBJTYPE_BRUSH)
		{
			BBox local;
			sel->GetObject(i)->GetBoundBox(local);
			box.Add( local.min );
			box.Add( local.max );
			bSelected = true;
		}
	}
	BBox empty;
	empty.Reset();
	if (bSelected)
	{
		if (!IsEquivalent(box.min,empty.min,0) && !IsEquivalent(box.max,empty.max,0))
		{
			m_lastBrushBounds = box;
		}
	}
}