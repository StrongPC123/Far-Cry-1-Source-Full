////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ObjectCloneTool.cpp
//  Version:     v1.00
//  Created:     18/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ObjectCloneTool.h"
#include "ObjectTypeBrowser.h"
#include "PanelTreeBrowser.h"
#include "Viewport.h"
#include "Objects\ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CObjectCloneTool,CEditTool)

//////////////////////////////////////////////////////////////////////////
CObjectCloneTool::CObjectCloneTool()
{
	m_bSetConstrPlane = true;

	GetIEditor()->SuperBeginUndo();
	
	GetIEditor()->BeginUndo();
	SetStatusText( "Left click to clone object" );
	m_selection = 0;
	if (!GetIEditor()->GetSelection()->IsEmpty())
	{
		CWaitCursor wait;
		CloneSelection();
		m_selection = GetIEditor()->GetSelection();
	}
	GetIEditor()->AcceptUndo( "Clone" );
	GetIEditor()->BeginUndo();
}

//////////////////////////////////////////////////////////////////////////
CObjectCloneTool::~CObjectCloneTool()
{
	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->SuperCancelUndo();
}

//////////////////////////////////////////////////////////////////////////
void CObjectCloneTool::CloneSelection()
{
	CSelectionGroup selObjects;
	CSelectionGroup sel;

	CSelectionGroup *currSelection = GetIEditor()->GetSelection();

	currSelection->Clone( selObjects );

	GetIEditor()->ClearSelection();
	for (int i = 0; i < selObjects.GetCount(); i++)
	{
		GetIEditor()->SelectObject( selObjects.GetObject(i) );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectCloneTool::SetConstrPlane( CViewport *view,CPoint point )
{
	Matrix44 originTM;
	originTM.SetIdentity();
	CSelectionGroup *selection = GetIEditor()->GetSelection();
	if (selection->GetCount() == 1)
	{
		originTM = selection->GetObject(0)->GetWorldTM();
	}
	else if (selection->GetCount() > 1)
	{
		originTM = selection->GetObject(0)->GetWorldTM();
		Vec3 center = view->SnapToGrid( originTM.GetTranslationOLD() );
		originTM.SetTranslationOLD( center );
	}
	view->SetConstrPlane( point,originTM );
}

//static Vec3 gP1,gP2;
//////////////////////////////////////////////////////////////////////////
void CObjectCloneTool::Display( DisplayContext &dc )
{
	//dc.SetColor( 1,1,0,1 );
	//dc.DrawBall( gP1,1.1f );
	//dc.DrawBall( gP2,1.1f );
}

//////////////////////////////////////////////////////////////////////////
bool CObjectCloneTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (m_selection)
	{
		// Set construction plane origin to selection origin.
		if (m_bSetConstrPlane)
		{
			SetConstrPlane( view,point );
			m_bSetConstrPlane = false;
		}

		if (event == eMouseLDown)
		{
			// Accept group.
			Accept();
			return true;
		}
		if (event == eMouseMove)
		{
			// Move selection.
			CSelectionGroup *selection = GetIEditor()->GetSelection();
			if (selection != m_selection)
			{
				Abort();
			}
			else if (!selection->IsEmpty())
			{
				GetIEditor()->RestoreUndo();

				Vec3 v;
				bool followTerrain = false;

				CSelectionGroup *pSelection = GetIEditor()->GetSelection();
				Vec3 selectionCenter = view->SnapToGrid( pSelection->GetCenter() );

				int axis = GetIEditor()->GetAxisConstrains();
				if (axis == AXIS_TERRAIN)
				{
					bool hitTerrain;
					v = view->ViewToWorld( point,&hitTerrain ) - selectionCenter;
					if (axis == AXIS_TERRAIN)
					{
						v = view->SnapToGrid(v);
						if (hitTerrain)
						{
							followTerrain = true;
							v.z = 0;
						}
					}
				}
				else
				{
					Vec3 p1 = selectionCenter;
					Vec3 p2 = view->MapViewToCP( point );
					if (p2.IsZero())
						return true;

					v = view->GetCPVector(p1,p2);
					// Snap v offset to grid if its enabled.
					view->SnapToGrid( v );
				}

				GetIEditor()->GetSelection()->Move( v,followTerrain,GetIEditor()->GetReferenceCoordSys()==COORDS_WORLD );
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CObjectCloneTool::Abort()
{
	// Abort
	GetIEditor()->SetEditTool(0);
}

//////////////////////////////////////////////////////////////////////////
void CObjectCloneTool::Accept()
{
	if (GetIEditor()->IsUndoRecording())
		GetIEditor()->SuperAcceptUndo( "Clone" );

	GetIEditor()->SetEditTool(0);
}

//////////////////////////////////////////////////////////////////////////
void CObjectCloneTool::BeginEditParams( IEditor *ie,int flags )
{
}

//////////////////////////////////////////////////////////////////////////
void CObjectCloneTool::EndEditParams()
{
}

//////////////////////////////////////////////////////////////////////////
bool CObjectCloneTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{ 
	if (nChar == VK_ESCAPE)
	{
		Abort();
	}
	return false; 
}