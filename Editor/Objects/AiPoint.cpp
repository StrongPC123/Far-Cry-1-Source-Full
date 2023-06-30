////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   AIPoint.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CAIPoint implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AIPoint.h"
#include "ErrorReport.h"

#include "..\AIPointPanel.h"
#include <IAgent.h>

#include "..\Viewport.h"
#include "Settings.h"

#include <I3DEngine.h>
//#include <I3DIndoorEngine.h>
#include <IAISystem.h>

#define AIWAYPOINT_RADIUS 0.3f
#define DIR_VECTOR Vec3(0,-1,0)

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CAIPoint,CBaseObject)

int CAIPoint::m_rollupId = 0;
CAIPointPanel* CAIPoint::m_panel = 0;

float CAIPoint::m_helperScale = 1;

//////////////////////////////////////////////////////////////////////////
CAIPoint::CAIPoint()
{
	m_aiNode = 0;
	m_bIndoorEntrance = false;
	m_bIndoors = false;
	m_aiType = EAIPOINT_WAYPOINT;

	m_bLinksValid = false;
	m_bIgnoreUpdateLinks = false;
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::Done()
{
	m_bLinksValid = false;
	if (m_aiNode)
	{
		IGraph *aiGraph = GetIEditor()->GetSystem()->GetAISystem()->GetNodeGraph();
		if (aiGraph)
		{
			aiGraph->Disconnect(m_aiNode,false);
			if (m_bIndoorEntrance)
			{
				aiGraph->RemoveEntrance( m_aiNode->nBuildingID,m_aiNode );
			}
		}
		m_aiNode = 0;
	}

	while (!m_links.empty())
	{
		CAIPoint *obj = GetLink(0);
		if (obj)
			RemoveLink( obj );
		else
			m_links.erase(m_links.begin());
	}

	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CAIPoint::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	m_bLinksValid = false;

	SetColor( RGB(0,128,255) );
	bool res = CBaseObject::Init( ie,prev,file );
	
	if (IsCreateGameObjects())
		if (m_aiType != EAIPOINT_HIDE)
		{
			// Create AI graph node in game.
			IGraph *aiGraph = GetIEditor()->GetSystem()->GetAISystem()->GetNodeGraph();
			if (aiGraph)
				m_aiNode = aiGraph->CreateNewNode();
		}

	if (prev)
	{
		CAIPoint *pOriginal = (CAIPoint *)prev;
		SetAIType(pOriginal->m_aiType);
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
CString CAIPoint::GetTypeDescription() const
{
	if (m_aiType == EAIPOINT_HIDE)
	{
		return CString(GetClassDesc()->ClassName()) + "(Hide)";
	}
	else if (m_aiType == EAIPOINT_ENTRY)
	{
		return CString(GetClassDesc()->ClassName()) + "(Entry)";
	}
	else if (m_aiType == EAIPOINT_EXIT)
	{
		return CString(GetClassDesc()->ClassName()) + "(Exit)";
	}
	return GetClassDesc()->ClassName();
};

//////////////////////////////////////////////////////////////////////////
float CAIPoint::GetRadius()
{
	return AIWAYPOINT_RADIUS*m_helperScale * gSettings.gizmo.helpersScale;
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::SetPos( const Vec3d &pos )
{
	Vec3d oldpos = GetWorldPos();
	CBaseObject::SetPos( pos );

	/*
	if (m_aiNode)
	{
    m_aiNode->data.m_pos = pos;

		m_buildingId = -1;
	
		m_bIndoors = false;
		IAISystem *pAISystem = GetIEditor()->GetSystem()->GetAISystem();
		int buildingId;IVisArea *pArea;
		if (pAISystem && pAISystem->CheckInside(pos,buildingId,pArea))
		{
			m_aiNode->nBuildingID = buildingId;
			m_aiNode->pArea = pArea;

			if (m_buildingId != buildingId)
				m_bLinksValid = false;
			m_buildingId = buildingId;
			m_pArea= pArea;
			m_bIndoors = true;
		}
	
	}
	else
	{
		m_bIndoors = false;
		IAISystem *pAISystem = GetIEditor()->GetSystem()->GetAISystem();	
		if (!pAISystem)
			return;
		IGraph *pGraph = pAISystem->GetNodeGraph();
		if (!pGraph)
			return;
		int buildingId;IVisArea *pArea;
		if (pAISystem->CheckInside(pos,buildingId,pArea))
		{
			if (m_buildingId != buildingId)
				m_bLinksValid = false;

			m_buildingId = buildingId;
			m_pArea= pArea;
			m_bIndoors = true;
		}

		// for hide points
		int numLinks = m_links.size();
		for (int i = 0; i < numLinks; i++)
		{
			// go trough all the links in the waypoint
			CAIPoint *pnt = GetLink(i);
			if (!pnt || !pnt->m_aiNode)
				continue;

			Matrix44 m;
			m.SetIdentity();

			//m.RotateMatrix_fix(GetAngles());
			m=GetRotationZYX44(-GetAngles()*gf_DEGTORAD)*m; //NOTE: angles in radians and negated 

			//CHANGED_BY_IVO
			//Vec3 dir = m.TransformVector(DIR_VECTOR);
			Vec3 dir = GetTransposed44(m) * (DIR_VECTOR);

			pGraph->RemoveHidePoint(pnt->m_aiNode,oldpos,dir);
			pGraph->AddHidePoint(pnt->m_aiNode,pos,dir);
			// and update the point there


		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::SetAngles( const Vec3d &angles )
{
	//if (m_aiType == EAIPOINT_HIDE)
	{
		CBaseObject::SetAngles(angles);
	}
}

void CAIPoint::SetScale( const Vec3d &angles )
{
}

//////////////////////////////////////////////////////////////////////////
bool CAIPoint::HitTest( HitContext &hc )
{
	Vec3 origin = GetWorldPos();
	float radius = GetRadius();

	Vec3 w = origin - hc.raySrc;
	w = hc.rayDir.Cross( w );
	float d = w.GetLengthSquared();

	if (d < radius*radius + hc.distanceTollerance)
	{
		Vec3 i0;
		if (Intersect::Ray_SphereFirst( Ray(hc.raySrc,hc.rayDir),Sphere(origin,radius),i0))
		{
			hc.dist = GetDistance(hc.raySrc,i0);
			return true;
		}
		hc.dist = GetDistance(hc.raySrc,origin);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::GetBoundBox( BBox &box )
{
	Vec3 pos = GetWorldPos();
	float r = GetRadius();
	box.min = pos - Vec3(r,r,r);
	box.max = pos + Vec3(r,r,r);
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::GetLocalBounds( BBox &box )
{
	float r = GetRadius();
	box.min = -Vec3(r,r,r);
	box.max = Vec3(r,r,r);
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::BeginEditParams( IEditor *ie,int flags )
{
	CBaseObject::BeginEditParams( ie,flags );

	// Unselect all links.
	for (int i = 0; i < GetLinkCount(); i++)
		SelectLink(i,false);

	if (!m_panel)
	{
		m_panel = new CAIPointPanel;
		m_rollupId = ie->AddRollUpPage( ROLLUP_OBJECTS,"AIPoint Parameters",m_panel );
	}
	if (m_panel)
		m_panel->SetObject( this );
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::EndEditParams( IEditor *ie )
{
	if (m_panel)
	{
		ie->RemoveRollUpPage( ROLLUP_OBJECTS,m_rollupId );
	}
	m_rollupId = 0;
	m_panel = 0;

	// Unselect all links.
	for (int i = 0; i < GetLinkCount(); i++)
		SelectLink(i,false);

	CBaseObject::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
int CAIPoint::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseMove || event == eMouseLDown)
	{
		Vec3 pos;
		if (GetIEditor()->GetAxisConstrains() != AXIS_TERRAIN)
		{
			pos = view->MapViewToCP(point);
		}
		else
		{
			// Snap to terrain.
			bool hitTerrain;
			pos = view->ViewToWorld( point,&hitTerrain );
			if (hitTerrain)
			{
				pos.z = GetIEditor()->GetTerrainElevation(pos.x,pos.y) + GetRadius();
			}
			pos = view->SnapToGrid(pos);
		}
		SetPos( pos );
		if (event == eMouseLDown)
			return MOUSECREATE_OK;
		return MOUSECREATE_CONTINUE;
	}
	return CBaseObject::MouseCreateCallback( view,event,point,flags );
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::Display( DisplayContext &dc )
{
	COLORREF color = GetColor();
	COLORREF clrSelectedLink = GetColor();

	if (!m_bIndoors)
	{
		// Draw invalid node.
		color = RGB(255,0,0);
	}
	Vec3 wp = GetWorldPos();
	if (m_aiType == EAIPOINT_HIDE)
	{
		color = RGB(0,255,0);	//<<FIXME>> Timur solve this in a better way.
		if (IsSelected())
			dc.SetSelectedColor();
		else
			dc.SetColor( color,1 );

		//Vec3 arrowTrg = GetWorldTM().TransformPoint(0.5f*Vec3d(0,-1,0));
		//dc.DrawArrow(wp,arrowTrg);
		Matrix44 tm = GetWorldTM();
		float sz = m_helperScale * gSettings.gizmo.helpersScale;
		tm.ScaleMatRow( Vec3(sz,sz,sz) );
		dc.RenderObject( STATOBJECT_HIDEPOINT,tm );
	}
	else if (m_aiType == EAIPOINT_ENTRY || m_aiType == EAIPOINT_EXIT)
	{
		if (IsSelected())
			dc.SetSelectedColor();
		else
			dc.SetColor( color,1 );
		Matrix44 tm = GetWorldTM();
		float sz = m_helperScale * gSettings.gizmo.helpersScale;
		tm.ScaleMatRow( Vec3(sz,sz,sz) );
		dc.RenderObject( STATOBJECT_ENTRANCE,GetWorldTM() );
	}
	else
	{
		dc.SetColor( color,1 );
		dc.DrawBall( wp,GetRadius() );
		if (IsSelected())
		{
			dc.SetSelectedColor( 0.3f );
			dc.renderer->DrawBall( wp,GetRadius()+0.1f );
		}
	}

	int numLinks = m_links.size();
	for (int i = 0; i < numLinks; i++)
	{
		CAIPoint *pnt = GetLink(i);
		if (!pnt)
			continue;

		if (m_links[i].selected)
		{
			dc.SetSelectedColor();
			dc.DrawLine( wp+Vec3(0,0,0.05f),pnt->GetWorldPos()+Vec3(0,0,0.05f) );
		}
		else
		{
			if (pnt->m_aiType == EAIPOINT_HIDE)
				color = RGB(0,255,0);
			dc.SetColor( color );
			dc.DrawLine( wp,pnt->GetWorldPos() );
		}
	}

	DrawDefault( dc );
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::Serialize( CObjectArchive &ar )
{
	m_bIgnoreUpdateLinks = true;
	CBaseObject::Serialize( ar );
	m_bIgnoreUpdateLinks = false;

	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		m_bIgnoreUpdateLinks = true;
		int aiType = 0;
		xmlNode->getAttr( "AIType",aiType );
		
		// Load Links.
		m_links.clear();
		XmlNodeRef links = xmlNode->findChild( "Links" );
		if (links)
		{
			m_bLinksValid = false;
			for (int i = 0; i < links->getChildCount(); i++)
			{
				XmlNodeRef pnt = links->getChild(i);
				Link link;
				link.object = 0;
				pnt->getAttr( "Id",link.id );
				m_links.push_back(link);
			}
		}
		SetAIType((EAIPointType)aiType);
		m_bIgnoreUpdateLinks = false;
		if (ar.bUndo)
			UpdateLinks();
	}
	else
	{
		xmlNode->setAttr( "AIType",(int)m_aiType );
		// Save Links.
		if (!m_links.empty())
		{
			XmlNodeRef links = xmlNode->newChild( "Links" );
			for (int i = 0; i < m_links.size(); i++)
			{
				XmlNodeRef pnt = links->newChild( "Link" );
				CAIPoint* link = GetLink(i);
				if (!link)
					continue;
				pnt->setAttr( "Id",link->GetId() );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CAIPoint::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	/*
	XmlNodeRef objNode = CBaseObject::Export( levelPath,xmlNode );
	objNode->setAttr( "Id",GetId() );
	
	// Save Links.
	if (!m_links.empty())
	{
		XmlNodeRef links = xmlNode->newChild( "Links" );
		for (int i = 0; i < m_links.size(); i++)
		{
			XmlNodeRef pnt = links->newChild( "Link" );
			pnt->setAttr( "Id",m_links[i].id );
		}
	}

	return CBaseObject::Export( levelPath,xmlNode );
	*/
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::OnEvent( ObjectEvent event )
{
	switch (event)
	{
		//case EVENT_AFTER_LOAD: // After all objects loaded.
		case EVENT_REFRESH: // when refreshing level.
			// Recalculate indoors.
			SetPos( GetPos() );
			// After loading reconnect all links.
			UpdateLinks();
			break;
		case EVENT_CLEAR_AIGRAPH:
			m_aiNode = 0;
			break;
		case EVENT_MISSION_CHANGE:
			{
				// After mission have been changed, AI graph invalidates all pointers, create them again.
				// Create AI graph node in game.
				IGraph *aiGraph = GetIEditor()->GetSystem()->GetAISystem()->GetNodeGraph();
				if (aiGraph)
				{
					m_bLinksValid = false;
					if (m_aiType != EAIPOINT_HIDE)
					{
						m_aiNode = aiGraph->CreateNewNode();
					}
					UpdateLinks();
				}
			}
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
CAIPoint* CAIPoint::GetLink( int index )
{
	assert( index >= 0 && index < m_links.size() );
	if (!m_links[index].object)
	{
		CBaseObject *obj = FindObject(m_links[index].id);
		if (obj && obj->IsKindOf(RUNTIME_CLASS(CAIPoint)))
		{
			((CAIPoint*)obj)->AddLink( this,false );
			m_links[index].object = (CAIPoint*)obj;
		}
	}
	return m_links[index].object;
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::AddLink( CAIPoint* obj,bool bNeighbour )
{
	if (obj == this)
		return;

	GUID id = obj->GetId();
	for (int i = 0; i < m_links.size(); i++)
	{
		if (m_links[i].object == obj || m_links[i].id == id)
			return;
	}

	if (m_aiType == EAIPOINT_HIDE)
	{
		if (obj->m_aiType == EAIPOINT_HIDE)
		{
			Warning( _T("Cannot connect hide spots") );
			return;
		}
	}

	Link link;
	link.object = obj;
	link.id = id;
	m_links.push_back( link );
	m_bLinksValid = false;


	if (!bNeighbour)
	{
		obj->AddLink( this,true );
		UpdateLinks();
	}
	else if (m_aiType == EAIPOINT_HIDE)
	{
		UpdateLinks();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::RemoveLink( CAIPoint* obj,bool bNeighbour )
{
	if (obj == this || obj == 0)
		return;

	if (m_aiType == EAIPOINT_HIDE && obj->m_aiNode)
	{
		IAISystem *aiSystem = GetIEditor()->GetSystem()->GetAISystem();
		IGraph *aiGraph = aiSystem->GetNodeGraph();
		if (aiGraph)
		{
			aiGraph->RemoveHidePoint(obj->m_aiNode,m_currHidePos,m_currHideDir );
		}
	}
	
	int index = -1;
	GUID id = obj->GetId();
	for (int i = 0; i < m_links.size(); i++)
	{
		if (m_links[i].object == obj || m_links[i].id == id)
		{
			index = i;
			break;
		}
	}
	if (index < 0)
		return;

	m_links.erase( m_links.begin() + index );
	m_bLinksValid = false;

	if (!bNeighbour)
	{
		obj->RemoveLink( this,true );
		UpdateLinks();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::UpdateLinks()
{
	if (m_bLinksValid || m_bIgnoreUpdateLinks)
		return;

	IAISystem *aiSystem = GetIEditor()->GetSystem()->GetAISystem();
	if (!aiSystem)
		return;
	IGraph *aiGraph = aiSystem->GetNodeGraph();

	if (!aiGraph)
			return;

	m_bIndoors = false;

	if (m_aiType != EAIPOINT_HIDE)
	{
		if (!m_aiNode)
			return;

		IVisArea *pArea = NULL;
		int nBuildingId = -1;
		Vec3 wp = GetWorldPos();
		if (aiSystem->CheckInside(wp,nBuildingId,pArea))
		{
			m_bIndoors = true;
		}

		aiGraph->Disconnect(m_aiNode,false);

		if (m_bIndoorEntrance)
		{
			aiGraph->RemoveEntrance( m_aiNode->nBuildingID,m_aiNode );
			m_bIndoorEntrance = false;
		}

		m_aiNode->nBuildingID = nBuildingId;
		m_aiNode->pArea = m_pArea;
		m_aiNode->data.m_pos = wp;

		if (m_aiType == EAIPOINT_ENTRY && m_aiNode->nBuildingID >= 0)
		{
			aiGraph->AddIndoorEntrance( m_aiNode->nBuildingID,m_aiNode );
			m_bIndoorEntrance = true;
		}
		else if (m_aiType == EAIPOINT_EXIT && m_aiNode->nBuildingID >= 0)
		{
			aiGraph->AddIndoorEntrance( m_aiNode->nBuildingID,m_aiNode,true);
			m_bIndoorEntrance = true;
		}

		int numLinks = GetLinkCount();
		for (int i = 0; i < numLinks; i++)
		{
			CAIPoint *lnk = GetLink(i);
			if (!lnk)
				continue;

			if (!lnk->m_aiNode)
			{
				if (lnk->m_aiType == EAIPOINT_HIDE)
				{
					Vec3 wp = lnk->GetWorldPos();
					Matrix44 m = lnk->GetWorldTM();
					m.NoScale();
					Vec3 dir = m.TransformVectorOLD(DIR_VECTOR);
					aiGraph->AddHidePoint(m_aiNode,wp,dir );
				}
			}
			else
			{
				aiGraph->Connect(m_aiNode,lnk->m_aiNode);
			}
		}
	}
	else
	{
		Vec3 wp = GetWorldPos();
		Matrix44 m = GetWorldTM();
		m.NoScale();
		Vec3 dir = m.TransformVectorOLD(DIR_VECTOR);

		int numLinks = GetLinkCount();
		for (int i = 0; i < numLinks; i++)
		{
			CAIPoint *lnk = GetLink(i);
			if (!lnk || !lnk->m_aiNode)
				continue;

			aiGraph->RemoveHidePoint( lnk->m_aiNode,m_currHidePos,m_currHideDir );
			aiGraph->AddHidePoint(lnk->m_aiNode,wp,dir );
		}
		m_currHidePos = wp;
		m_currHideDir = dir;
	}
	m_bLinksValid = true;
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::SetAIType( EAIPointType type )
{
	if (type == m_aiType)
		return;
	StoreUndo( "Set AIPoint Type" );
	EAIPointType oldType = m_aiType;
	m_aiType = type;
	if (m_aiType == EAIPOINT_HIDE)
	{	
		IAISystem *aiSystem = GetIEditor()->GetSystem()->GetAISystem();
		IGraph *aiGraph = aiSystem->GetNodeGraph();
		if (m_aiNode && aiGraph)
		{
			//aiGraph->DeleteNode(m_aiNode);
			aiGraph->Disconnect(m_aiNode);
			m_aiNode = 0;
		}
	}
	else if (!m_aiNode)
	{
		IAISystem *aiSystem = GetIEditor()->GetSystem()->GetAISystem();
		IGraph *aiGraph = aiSystem->GetNodeGraph();
		if (aiGraph)
			m_aiNode = aiGraph->CreateNewNode();
	}

	m_bLinksValid = false;

	UpdateLinks();
}

//////////////////////////////////////////////////////////////////////////
//! Invalidates cached transformation matrix.
void CAIPoint::InvalidateTM()
{
	CBaseObject::InvalidateTM();

	m_bLinksValid = false;
	UpdateLinks();
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::StartPick()
{
	if (m_panel)
		m_panel->StartPick();
}

//////////////////////////////////////////////////////////////////////////
void CAIPoint::Validate( CErrorReport *report )
{
	CBaseObject::Validate( report );
	if (!m_bIndoors && m_aiType != EAIPOINT_HIDE)
	{
		CErrorRecord err;
		err.error.Format( "AI Point is not valid %s (Must be indoors)",(const char*)GetName() );
		err.pObject = this;
		err.severity = CErrorRecord::ESEVERITY_WARNING;
		err.flags = CErrorRecord::FLAG_AI;
		report->ReportError(err);
	}
}