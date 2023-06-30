////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   VegetationTool.cpp
//  Version:     v1.00
//  Created:     11/1/2002 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Places vegetation on terrain.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "VegetationTool.h"
#include "Viewport.h"
#include "VegetationPanel.h"
#include "Heightmap.h"
#include "VegetationMap.h"
#include "Objects\DisplayContext.h"
#include "NumberDlg.h"
#include "PanelPreview.h"
#include "Settings.h"

#include "I3dEngine.h"
#include "IPhysics.h"

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CVegetationTool,CEditTool)

float CVegetationTool::m_brushRadius = 1;
//bool CVegetationTool::m_bPlaceMode = true;

//////////////////////////////////////////////////////////////////////////
CVegetationTool::CVegetationTool()
{
	m_panelId = 0;
	m_panel = 0;
	m_panelPreview = 0;
	m_panelPreviewId = 0;

	m_pointerPos(0,0,0);

	m_vegetationMap = GetIEditor()->GetHeightmap()->GetVegetationMap();

	GetIEditor()->ClearSelection();

	m_bPaintingMode = false;
	m_bPlaceMode = true;

	m_opMode = OPMODE_NONE;

	SetStatusText( "Click to Place or Remove Vegetation" );
}

//////////////////////////////////////////////////////////////////////////
CVegetationTool::~CVegetationTool()
{
	m_pointerPos(0,0,0);
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	if (!m_panelId)
	{
		CWaitCursor wait;
		m_panel = new CVegetationPanel(this,AfxGetMainWnd());
		m_panelId = m_ie->AddRollUpPage( ROLLUP_TERRAIN,"Vegetation",m_panel );

		if (gSettings.bPreviewGeometryWindow)
		{
			m_panelPreview = new CPanelPreview(AfxGetMainWnd());
			m_panelPreviewId = m_ie->AddRollUpPage( ROLLUP_TERRAIN,"Object Preview",m_panelPreview );
			m_panel->SetPreviewPanel(m_panelPreview);
		}

		AfxGetMainWnd()->SetFocus();

		GetIEditor()->UpdateViews( eUpdateStatObj );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::EndEditParams()
{
	if (m_panelPreviewId)
	{
		m_ie->RemoveRollUpPage(ROLLUP_TERRAIN,m_panelPreviewId);
		m_panelPreviewId = 0;
		m_panelPreview = 0;
	}
	if (m_panelId)
	{
		m_ie->RemoveRollUpPage(ROLLUP_TERRAIN,m_panelId);
		m_panel = 0;
		m_panelId = 0;
	}
	GetIEditor()->SetStatusText( "Ready" );
}

//////////////////////////////////////////////////////////////////////////
// Specific mouse events handlers.
bool CVegetationTool::OnLButtonDown( CViewport *view,UINT nFlags,CPoint point )
{
	m_mouseDownPos = point;
	bool bShift = nFlags & MK_SHIFT;
	bool bCtrl = nFlags & MK_CONTROL;
	bool bAlt = (GetAsyncKeyState(VK_MENU) & (1<<15)) != 0;

	m_opMode = OPMODE_NONE;
	if (m_bPaintingMode)
	{
		m_opMode = OPMODE_PAINT;

		if (nFlags&MK_CONTROL)
      m_bPlaceMode = false;
		else
			m_bPlaceMode = true;
		
		GetIEditor()->BeginUndo();
		view->CaptureMouse();
		PaintBrush();
	}
	else
	{
		Matrix44 tm;
		tm.SetIdentity();
		tm.SetTranslationOLD( view->ViewToWorld(point) );
		view->SetConstrPlane( point,tm );

		m_opMode = OPMODE_SELECT;

		if (bShift)
		{
			PlaceThing();
			m_opMode = OPMODE_MOVE;
			GetIEditor()->BeginUndo();
		}
		else
		{
			if (!bCtrl && !bAlt)
			{
				ClearThingSelection();
			}
			// Select thing.
			if (SelectThingAtPoint(view,point))
			{
				if (bAlt)
					m_opMode = OPMODE_SCALE;
				else
					m_opMode = OPMODE_MOVE;
				GetIEditor()->BeginUndo();
			}
		}
		view->CaptureMouse();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CVegetationTool::OnLButtonUp( CViewport *view,UINT nFlags,CPoint point )
{
	if (GetIEditor()->IsUndoRecording())
	{
		if (m_opMode == OPMODE_MOVE)
			GetIEditor()->AcceptUndo( "Vegetation Move" );
		else if (m_opMode == OPMODE_SCALE)
			GetIEditor()->AcceptUndo( "Vegetation Scale" );
		else if (m_opMode == OPMODE_PAINT)
			GetIEditor()->AcceptUndo( "Vegetation Paint" );
		else
			GetIEditor()->CancelUndo();
	}
	if (m_opMode == OPMODE_SELECT)
	{
		CRect rect = view->GetSelectionRectangle();
		if (!(nFlags & MK_CONTROL))
		{
			ClearThingSelection();
		}
		if (!rect.IsRectEmpty())
			SelectThingsInRect( view,rect );
		/*
		BBox box;
		GetIEditor()->GetSelectedRegion( box );
		if (!box.IsEmpty())
		{
			ClearThingSelection();
			std::vector<CVegetationInstance*> selectedThings;
			m_vegetationMap->GetObjectInstances( box.min.x,box.min.y,box.max.x,box.max.y,selectedThings );
			for (int i = 0; i < selectedThings.size(); i++)
			{
				if (!selectedThings[i]->object->IsHidden())
				{
					SelectThing( selectedThings[i],false );
				}
			}
		}
		*/
	}
	m_opMode = OPMODE_NONE;

	view->ReleaseMouse();

	// Reset selected region.
	view->ResetSelectionRegion();
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CVegetationTool::OnMouseMove( CViewport *view,UINT nFlags,CPoint point )
{
	if (nFlags&MK_CONTROL)
		m_bPlaceMode = false;
	else
		m_bPlaceMode = true;

	if (m_opMode == OPMODE_PAINT)
	{
		if (nFlags&MK_LBUTTON)
			PaintBrush();
	}
	else if (m_opMode == OPMODE_SELECT)
	{
    // Define selection.
		view->SetSelectionRectangle( m_mouseDownPos,point );
		//CRect rc( m_cMouseDownPos,point );
	}
	else if (m_opMode == OPMODE_MOVE)
	{
		GetIEditor()->RestoreUndo();
    // Define selection.
		int axis = GetIEditor()->GetAxisConstrains();
		Vec3 p1 = view->MapViewToCP( m_mouseDownPos );
		Vec3 p2 = view->MapViewToCP( point );
		Vec3 v = view->GetCPVector(p1,p2);
		MoveSelected( v,(axis == AXIS_TERRAIN) );

		//m_mouseDownPos = point;
	}
	else if (m_opMode == OPMODE_SCALE)
	{
		GetIEditor()->RestoreUndo();
		// Define selection.
		int y = m_mouseDownPos.y - point.y;
		if (y != 0)
		{
			float fScale = 1.0f + (float)y/100.0f;
			ScaleSelected( fScale );
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CVegetationTool::MouseCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	m_pointerPos = view->ViewToWorld( point,0,true );
	m_mousePos = point;
	m_bPlaceMode = true;

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

	GetIEditor()->SetMarkerPosition( m_pointerPos );
	int unitSize = GetIEditor()->GetHeightmap()->GetUnitSize();

	if (flags & MK_CONTROL)
	{
		//swap x/y
		float slope = GetIEditor()->GetHeightmap()->GetSlope( m_pointerPos.y/unitSize,m_pointerPos.x/unitSize );
		char szNewStatusText[512];
		sprintf(szNewStatusText, "Slope: %g",slope );
		GetIEditor()->SetStatusText(szNewStatusText);
	}
	else
	{
		GetIEditor()->SetStatusText( "Shift: Place New  Ctrl: Add To Selection  Alt: Scale Selected  DEL: Delete Selected" );
	}

	m_prevMousePos = point;

	// Not processed.
	return bProcessed;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::Display( DisplayContext &dc )
{
	if (!m_bPaintingMode)
	{
		if (dc.flags & DISPLAY_2D)
			return;

		dc.SetColor( 0,1,0,1 );
		// Single object 3D display mode.
		for (int i = 0; i < m_selectedThings.size(); i++)
		{
			float radius = m_selectedThings[i]->object->GetObjectSize() * m_selectedThings[i]->scale;
			dc.DrawTerrainCircle( m_selectedThings[i]->pos,radius/2.0f,0.1f );
		}
	}
	else
	{
		// Brush paint mode.

		if (dc.flags & DISPLAY_2D)
		{
			CPoint p1 = dc.view->WorldToView(m_pointerPos);
			CPoint p2 = dc.view->WorldToView(m_pointerPos+Vec3(m_brushRadius,0,0));
			float radius = sqrtf( (p2.x-p1.x)*(p2.x-p1.x) + (p2.y-p1.y)*(p2.y-p1.y) );
			dc.SetColor( 0,1,0,1 );
			dc.DrawWireCircle2d( p1,radius,0 );
			return;
		}

		dc.SetColor( 0,1,0,1 );
		dc.DrawTerrainCircle( m_pointerPos,m_brushRadius,0.2f );

		float col[4] = { 1,1,1,0.8f };
		if (m_bPlaceMode)
			dc.renderer->DrawLabelEx( m_pointerPos+Vec3(0,0,1),1.0f,col,true,true,"Place" );
		else
			dc.renderer->DrawLabelEx( m_pointerPos+Vec3(0,0,1),1.0f,col,true,true,"Remove" );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CVegetationTool::OnKeyDown( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	bool bProcessed = false;
	if (nChar == VK_ADD)
	{
		if (m_brushRadius < 300)
			m_brushRadius += 1;
		bProcessed = true;
	}
	if (nChar == VK_SUBTRACT)
	{
		if (m_brushRadius > 1)
			m_brushRadius -= 1;
		bProcessed = true;
	}
	if (nChar == VK_DELETE)
	{
		CUndo undo( "Vegetation Delete" );
		if (!m_selectedThings.empty())
		{
			if (AfxMessageBox( "Delete Selected Instances?",MB_YESNO) == IDYES)
			{
				for (int i = 0; i < m_selectedThings.size(); i++)
				{
					m_vegetationMap->DeleteObjInstance( m_selectedThings[i] );
				}
				ClearThingSelection();
				if (m_panel)
					m_panel->UpdateAllObjectsInTree();
			}
		}
		bProcessed = true;
	}
	if (nChar == VK_CONTROL && !(nFlags&(1<<14))) // only once (no repeat).
	{
		m_bPlaceMode = true;
		m_opMode = OPMODE_NONE;
	}
	if (bProcessed && m_panel)
	{
		m_panel->m_radius.SetPos(m_brushRadius);
	}
	return bProcessed;
}

//////////////////////////////////////////////////////////////////////////
bool CVegetationTool::OnKeyUp( CViewport *view,uint nChar,uint nRepCnt,uint nFlags )
{
	if (nChar == VK_CONTROL)
	{
		m_bPlaceMode = true;
		m_opMode = OPMODE_NONE;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::GetSelectedObjects( std::vector<CVegetationObject*> &objects )
{
	objects.clear();
	for (int i = 0; i < m_vegetationMap->GetObjectCount(); i++)
	{
		if (m_vegetationMap->GetObject(i)->IsSelected())
			objects.push_back( m_vegetationMap->GetObject(i) );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::PaintBrush()
{
	GetSelectedObjects( m_selectedObjects );

	CRect rc( m_pointerPos.x-m_brushRadius,m_pointerPos.y-m_brushRadius,
						m_pointerPos.x+m_brushRadius,m_pointerPos.y+m_brushRadius );
	if (m_bPlaceMode)
	{
		for (int i = 0; i < m_selectedObjects.size(); i++)
		{
			int numInstances = m_selectedObjects[i]->GetNumInstances();
			m_vegetationMap->PaintBrush( rc,true,m_selectedObjects[i] );
			// If number of instances changed.
			if (numInstances != m_selectedObjects[i]->GetNumInstances())
				m_panel->UpdateObjectInTree( m_selectedObjects[i] );
		}
	}
	else
	{
		for (int i = 0; i < m_selectedObjects.size(); i++)
		{
			int numInstances = m_selectedObjects[i]->GetNumInstances();
			m_vegetationMap->ClearBrush( rc,true,m_selectedObjects[i] );
			// If number of instances changed.
			if (numInstances != m_selectedObjects[i]->GetNumInstances())
				m_panel->UpdateObjectInTree( m_selectedObjects[i] );
		}
	}

	BBox updateRegion;
	updateRegion.min = m_pointerPos - Vec3(m_brushRadius,m_brushRadius,m_brushRadius);
	updateRegion.max = m_pointerPos + Vec3(m_brushRadius,m_brushRadius,m_brushRadius);
	GetIEditor()->UpdateViews( eUpdateStatObj,&updateRegion );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::PlaceThing()
{
	GetSelectedObjects( m_selectedObjects );

	CUndo undo( "Vegetation Place" );
	if (!m_selectedObjects.empty())
	{
		CVegetationInstance *thing = m_vegetationMap->PlaceObjectInstance( m_pointerPos,m_selectedObjects[0] );
		// If number of instances changed.
		if (thing)
		{
			ClearThingSelection();
			SelectThing(thing);
			m_panel->UpdateObjectInTree( m_selectedObjects[0] );

			BBox updateRegion;
			updateRegion.min = m_pointerPos - Vec3(1,1,1);
			updateRegion.max = m_pointerPos + Vec3(1,1,1);
			GetIEditor()->UpdateViews( eUpdateStatObj,&updateRegion );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::Distribute()
{
	GetSelectedObjects( m_selectedObjects );
	
	for (int i = 0; i < m_selectedObjects.size(); i++)
	{
		int numInstances = m_selectedObjects[i]->GetNumInstances();

		CRect rc( 0,0,m_vegetationMap->GetSize(),m_vegetationMap->GetSize() );
		m_vegetationMap->PaintBrush( rc,false,m_selectedObjects[i] );

		if (numInstances != m_selectedObjects[i]->GetNumInstances())
			m_panel->UpdateObjectInTree( m_selectedObjects[i] );
	}
	if (!m_selectedObjects.empty())
		GetIEditor()->UpdateViews( eUpdateStatObj );

	ClearThingSelection();
}
	
//////////////////////////////////////////////////////////////////////////
void CVegetationTool::DistributeMask( const char *maskFile )
{
	GetSelectedObjects( m_selectedObjects );
	
	for (int i = 0; i < m_selectedObjects.size(); i++)
	{
		int numInstances = m_selectedObjects[i]->GetNumInstances();
		//map->PaintMask( maskFile,m_selectedObjects[i] );

		if (numInstances != m_selectedObjects[i]->GetNumInstances())
			m_panel->UpdateObjectInTree( m_selectedObjects[i] );
	}
	if (!m_selectedObjects.empty())
		GetIEditor()->UpdateViews( eUpdateStatObj );

	ClearThingSelection();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::Clear()
{
	GetSelectedObjects( m_selectedObjects );
	
	for (int i = 0; i < m_selectedObjects.size(); i++)
	{
		int numInstances = m_selectedObjects[i]->GetNumInstances();

		CRect rc( 0,0,m_vegetationMap->GetSize(),m_vegetationMap->GetSize());
		m_vegetationMap->ClearBrush( rc,false,m_selectedObjects[i] );

		if (numInstances != m_selectedObjects[i]->GetNumInstances())
			m_panel->UpdateObjectInTree( m_selectedObjects[i] );
	}
	if (!m_selectedObjects.empty())
		GetIEditor()->UpdateViews( eUpdateStatObj );

	ClearThingSelection();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::ClearMask( const char *maskFile )
{
	//GetSelectedObjects( m_selectedObjects );
	
	/*
	for (int i = 0; i < m_selectedObjects.size(); i++)
	{
		int numInstances = m_selectedObjects[i]->GetNumInstances();

		//m_vegetationMap->ClearMask( maskFile,m_selectedObjects[i] );
		if (numInstances != m_selectedObjects[i]->GetNumInstances())
			m_panel->UpdateObjectInTree( m_selectedObjects[i] );
	}
	*/
	
	m_vegetationMap->ClearMask( maskFile );

	//if (!m_selectedObjects.empty())
		GetIEditor()->UpdateViews( eUpdateStatObj );

	ClearThingSelection();
}

void CVegetationTool::HideSelectedObjects( bool bHide )
{
	GetSelectedObjects( m_selectedObjects );

	for (int i = 0; i < m_selectedObjects.size(); i++)
	{
		m_vegetationMap->HideObject( m_selectedObjects[i],bHide );
	}
	if (!m_selectedObjects.empty())
	{
		/*
		GetIEditor()->UpdateViews( eUpdateStatObj );
		CStatObjMap *vegetationMap = GetIEditor()->GetStatObjMap();
		vegetationMap->RemoveObjectsFromTerrain();
		vegetationMap->PlaceObjectsOnTerrain();
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::RemoveSelectedObjects()
{
	GetSelectedObjects( m_selectedObjects );

	GetIEditor()->BeginUndo();
	for (int i = 0; i < m_selectedObjects.size(); i++)
	{
		int numInstances = m_selectedObjects[i]->GetNumInstances();

		m_vegetationMap->RemoveObject( m_selectedObjects[i] );

		if (numInstances != m_selectedObjects[i]->GetNumInstances())
			m_panel->UpdateObjectInTree( m_selectedObjects[i] );
	}
	GetIEditor()->AcceptUndo( "Remove Brush" );

	ClearThingSelection();

	if (!m_selectedObjects.empty())
	{
		GetIEditor()->UpdateViews( eUpdateStatObj );
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::SetMode( bool paintMode )
{
	if (paintMode)
	{
		SetStatusText( "Hold Ctrl to Remove Vegetation" );
	}
	else
	{
		SetStatusText( "Push Paint button to start painting" );
	}
	ClearThingSelection();
	m_bPaintingMode = paintMode;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::ClearThingSelection()
{
	m_selectedThings.clear();
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::SelectThing( CVegetationInstance *thing,bool bSelObject )
{
	// If already selected.
	if (std::find(m_selectedThings.begin(),m_selectedThings.end(),thing) != m_selectedThings.end())
		return;

	if (thing->object->IsHidden())
		return;

	m_selectedThings.push_back(thing);
	if (m_panel && bSelObject)
	{
		m_panel->SelectObject( thing->object );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CVegetationTool::SelectThingAtPoint( CViewport *view,CPoint point,bool bNotSelect )
{
	Vec3 raySrc,rayDir;
	view->ViewToWorldRay( point,raySrc,rayDir );
	
	bool bCollideTerrain=false;
	Vec3 pos = view->ViewToWorld( point,&bCollideTerrain,true );

	IPhysicalWorld *pPhysics = GetIEditor()->GetSystem()->GetIPhysicalWorld();
	if (pPhysics)
	{
		int objTypes = ent_static;
		int flags = rwi_stop_at_pierceable|rwi_ignore_terrain_holes;
		//flags = 31;
		ray_hit hit;
		int col = pPhysics->RayWorldIntersection( raySrc,rayDir*1000.0f,objTypes,flags,&hit,1 );
		if (hit.dist > 0 && !hit.bTerrain)
		{
			pe_status_pos statusPos;
			hit.pCollider->GetStatus( &statusPos );
			pos = statusPos.pos;
		}
	}

	// Find closest thing to this point.
	CVegetationInstance *obj = m_vegetationMap->GetNearestInstance( pos,1.0f );
	if (obj)
	{
		if (!bNotSelect)
			SelectThing(obj);
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::MoveSelected( const Vec3 &offset,bool bFollowTerrain )
{
	BBox box;
	box.Reset();
	Vec3 newPos,oldPos;
	for (int i = 0; i < m_selectedThings.size(); i++)
	{
		oldPos = m_selectedThings[i]->pos;
		newPos = oldPos + offset;
		//if (bFollowTerrain)
		{
			// Make sure object keeps it height.
			//float height = oldPos.z - GetIEditor()->GetTerrainElevation( oldPos.x,oldPos.y );
			//newPos.z = GetIEditor()->GetTerrainElevation( newPos.x,newPos.y ) + height;
		}
		// Always stick to terrain.
		newPos.z = GetIEditor()->GetTerrainElevation( newPos.x,newPos.y );

		m_vegetationMap->MoveInstance( m_selectedThings[i],newPos );
		box.Add( newPos );
	}
	box.min = box.min - Vec3(1,1,1);
	box.max = box.max + Vec3(1,1,1);
	GetIEditor()->UpdateViews( eUpdateStatObj,&box );
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::ScaleSelected( float fScale )
{
	if (fScale <= 0.01f)
		return;

	if (!m_selectedThings.empty())
	{
		int numThings = m_selectedThings.size();
		for (int i = 0; i < numThings; i++)
		{
			m_vegetationMap->RecordUndo( m_selectedThings[i] );
			m_selectedThings[i]->scale *= fScale;

			// Force this object to be placed on terrain again.
			m_vegetationMap->MoveInstance(m_selectedThings[i],m_selectedThings[i]->pos);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::ScaleObjects()
{
	float fScale = 1;
	if (m_selectedThings.size() == 1)
	{
		fScale = m_selectedThings[0]->scale;
	}

	CNumberDlg dlg( AfxGetMainWnd(),fScale,"Scale Selected Object(s)" );
	if (dlg.DoModal() != IDOK)
		return;

	fScale = dlg.GetValue();
	if (fScale <= 0)
		return;

	if (!m_selectedThings.empty())
	{
		int numThings = m_selectedThings.size();
		for (int i = 0; i < numThings; i++)
		{
			m_vegetationMap->RecordUndo( m_selectedThings[i] );
			if (numThings > 1)
				m_selectedThings[i]->scale *= fScale;
			else
				m_selectedThings[i]->scale = fScale;

			// Force this object to be placed on terrain again.
			m_vegetationMap->MoveInstance(m_selectedThings[i],m_selectedThings[i]->pos);
		}
	}
	else
	{
		GetSelectedObjects( m_selectedObjects );
		for (int i = 0; i < m_selectedObjects.size(); i++)
		{
			m_vegetationMap->ScaleObjectInstances( m_selectedObjects[i],fScale );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::SelectThingsInRect( CViewport *view,CRect rect )
{
	BBox box;
	std::vector<CVegetationInstance*> things;
	m_vegetationMap->GetAllInstances( things );
	for (int i = 0; i < things.size(); i++)
	{
		Vec3 pos = things[i]->pos;
		box.min.Set( pos.x-0.1f,pos.y-0.1f,pos.z-0.1f );
		box.max.Set( pos.x+0.1f,pos.y+0.1f,pos.z+0.1f );
		if (!view->IsBoundsVisible(box))
			continue;
		CPoint pnt = view->WorldToView( things[i]->pos );
		if (rect.PtInRect(pnt))
		{
			SelectThing( things[i],false );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::Command_Activate()
{
	CEditTool *pTool = GetIEditor()->GetEditTool();
	if (pTool && pTool->IsKindOf(RUNTIME_CLASS(CVegetationTool)))
	{
		// Already active.
		return;
	}
	pTool = new CVegetationTool;
	GetIEditor()->SetEditTool( pTool );
	GetIEditor()->SelectRollUpBar( ROLLUP_TERRAIN );
	AfxGetMainWnd()->RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
}

//////////////////////////////////////////////////////////////////////////
// Class description.
//////////////////////////////////////////////////////////////////////////
class CVegetationTool_ClassDesc : public CRefCountClassDesc
{
	//! This method returns an Editor defined GUID describing the class this plugin class is associated with.
	virtual ESystemClassID SystemClassID() { return ESYSTEM_CLASS_EDITTOOL; }

	//! Return the GUID of the class created by plugin.
	virtual REFGUID ClassID() 
	{
		// {854ECA09-C572-444a-BC1F-2F7A71E1277D}
		static const GUID guid = { 0x854eca09, 0xc572, 0x444a, { 0xbc, 0x1f, 0x2f, 0x7a, 0x71, 0xe1, 0x27, 0x7d } };
		return guid;
	}

	//! This method returns the human readable name of the class.
	virtual const char* ClassName() { return "VegetationTool"; };

	//! This method returns Category of this class, Category is specifing where this plugin class fits best in
	//! create panel.
	virtual const char* Category() { return "Terrain"; };
	//////////////////////////////////////////////////////////////////////////
};

//////////////////////////////////////////////////////////////////////////
void CVegetationTool::RegisterTool( CRegistrationContext &rc )
{
	rc.pClassFactory->RegisterClass( new CVegetationTool_ClassDesc );

	rc.pCommandManager->RegisterCommand( "EditTool.VegetationTool.Activate",functor(CVegetationTool::Command_Activate) );
}