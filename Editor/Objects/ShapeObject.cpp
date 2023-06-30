////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ShapeObject.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CShapeObject implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ShapeObject.h"
#include "..\ShapePanel.h"
#include "..\Viewport.h"
#include "Util\Triangulate.h"

#include <I3DEngine.h>
#include <IAISystem.h>
#include <IGame.h>
#include <IMarkers.h>

#include <vector>
#include "IEntitySystem.h"

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CShapeObject,CBaseObject)
IMPLEMENT_DYNCREATE(CAIForbiddenAreaObject,CShapeObject)
IMPLEMENT_DYNCREATE(CAIPathObject,CShapeObject)
IMPLEMENT_DYNCREATE(CAINavigationModifierObject,CShapeObject)
IMPLEMENT_DYNCREATE(CAIOcclusionPlaneObject,CShapeObject)

#define RAY_DISTANCE 100000.0f

//////////////////////////////////////////////////////////////////////////
int CShapeObject::m_rollupId = 0;
CShapePanel* CShapeObject::m_panel = 0;

//////////////////////////////////////////////////////////////////////////
CShapeObject::CShapeObject()
{
	m_bForce2D = false;
	mv_closed = false;
	mv_areaId = 0;
	mv_groupId = 0;
	mv_width = 0;
	mv_height = 0;
	mv_displayFilled = false;
	
	m_bbox.min = m_bbox.max = Vec3(0,0,0);
	m_selectedPoint = -1;
	m_lowestHeight = 0;
	m_IArea = 0;
	m_bIgnoreGameUpdate = true;
	m_bAreaModified = true;
	m_bDisplayFilledWhenSelected = false;

	AddVariable( mv_width,"Width",functor(*this,&CShapeObject::OnShapeChange) );
	AddVariable( mv_height,"Height",functor(*this,&CShapeObject::OnShapeChange) );
	AddVariable( mv_areaId,"AreaId",functor(*this,&CShapeObject::OnShapeChange) );
	AddVariable( mv_groupId,"GroupId",functor(*this,&CShapeObject::OnShapeChange) );
	AddVariable( mv_closed,"Closed",functor(*this,&CShapeObject::OnShapeChange) );
	AddVariable( mv_displayFilled,"DisplayFilled" );

	SetColor( Vec2Rgb(Vec3(0,0.8f,1)) );
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::Done()
{
	m_entities.Clear();
	m_points.clear();
	UpdateGameArea(true);
	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CShapeObject::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	m_ie = ie;

	m_bIgnoreGameUpdate = true;

	bool res = CBaseObject::Init( ie,prev,file );

	if (prev)
	{
		m_points = ((CShapeObject*)prev)->m_points;
		m_bIgnoreGameUpdate = false;
		CalcBBox();
	}

	m_bIgnoreGameUpdate = false;

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool CShapeObject::CreateGameObject()
{
	return true;
};

//////////////////////////////////////////////////////////////////////////
void CShapeObject::SetName( const CString &name )
{
	CBaseObject::SetName( name );
	m_bAreaModified = true;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::InvalidateTM()
{
	CBaseObject::InvalidateTM();
	m_bAreaModified = true;
//	CalcBBox();

	if (!IsOnlyUpdateOnUnselect() && !m_bIgnoreGameUpdate)
		UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::GetBoundBox( BBox &box )
{
	box = m_bbox;
	box.Transform( GetWorldTM() );
	float s = 1.0f;
	box.min -= Vec3(s,s,s);
	box.max += Vec3(s,s,s);
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::GetLocalBounds( BBox &box )
{
	box = m_bbox;
}

//////////////////////////////////////////////////////////////////////////
bool CShapeObject::HitTest( HitContext &hc )
{
	// First check if ray intersect our bounding box.
	float tr = hc.distanceTollerance/2 + SHAPE_CLOSE_DISTANCE;

	// Find intersection of line with zero Z plane.
	float minDist = FLT_MAX;
	Vec3 intPnt;
	//GetNearestEdge( hc.raySrc,hc.rayDir,p1,p2,minDist,intPnt );

	bool bWasIntersection = false;
	Vec3 ip;
	Vec3 rayLineP1 = hc.raySrc;
	Vec3 rayLineP2 = hc.raySrc + hc.rayDir*RAY_DISTANCE;
	const Matrix44 &wtm = GetWorldTM();

	for (int i = 0; i < m_points.size(); i++)
	{
		int j = (i < m_points.size()-1) ? i+1 : 0;

		if (!mv_closed && j == 0 && i != 0)
			continue;

		Vec3 pi = wtm.TransformPointOLD(m_points[i]);
		Vec3 pj = wtm.TransformPointOLD(m_points[j]);

		float d = 0;
		if (RayToLineDistance( rayLineP1,rayLineP2,pi,pj,d,ip ))
		{
			if (d < minDist)
			{
				bWasIntersection = true;
				minDist = d;
				intPnt = ip;
			}
		}

		if (mv_height > 0)
		{
			if (RayToLineDistance( rayLineP1,rayLineP2,pi+Vec3(0,0,mv_height),pj+Vec3(0,0,mv_height),d,intPnt ))
			{
				if (d < minDist)
				{
					bWasIntersection = true;
					minDist = d;
					intPnt = ip;
				}
			}
			if (RayToLineDistance( rayLineP1,rayLineP2,pi,pi+Vec3(0,0,mv_height),d,intPnt ))
			{
				if (d < minDist)
				{
					bWasIntersection = true;
					minDist = d;
					intPnt = ip;
				}
			}
		}
	}

	float fShapeCloseDistance = SHAPE_CLOSE_DISTANCE*hc.view->GetScreenScaleFactor(intPnt) * 0.01f;

	if (bWasIntersection && minDist < fShapeCloseDistance+hc.distanceTollerance)
	{
		hc.dist = GetDistance(hc.raySrc,intPnt);
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	CBaseObject::BeginEditParams( ie,flags );

	if (!m_panel)
	{
		m_panel = new CShapePanel;
		m_rollupId = ie->AddRollUpPage( ROLLUP_OBJECTS,"Shape Parameters",m_panel );
	}
	if (m_panel)
		m_panel->SetShape( this );
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::EndEditParams( IEditor *ie )
{
	if (m_rollupId != 0)
	{
		ie->RemoveRollUpPage( ROLLUP_OBJECTS,m_rollupId );
		CalcBBox();
	}
	m_rollupId = 0;
	m_panel = 0;

	CBaseObject::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
int CShapeObject::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseMove || event == eMouseLDown || event == eMouseLDblClick)
	{
		Vec3 pos = view->MapViewToCP(point);

		bool firstTime = false;
		if (m_points.size() < 2)
		{
			SetPos( pos );
		}

		pos.z += SHAPE_Z_OFFSET;
		
		if (m_points.size() == 0)
		{
			InsertPoint( -1,Vec3(0,0,0) );
			firstTime = true;
		}
		else
		{
			SetPoint( m_points.size()-1,pos - GetWorldPos() );
		}

		if (event == eMouseLDblClick)
		{
			if (m_points.size() > 3)
			{
				m_points.pop_back(); // Remove last uneeded point.
				EndCreation();
				return MOUSECREATE_OK;
			}
			else
				return MOUSECREATE_ABORT;
		}

		if (event == eMouseLDown)
		{
			Vec3 vlen = Vec3(pos.x,pos.y,0) - Vec3(GetWorldPos().x,GetWorldPos().y,0);
			if (m_points.size() > 2 && vlen.Length() < SHAPE_CLOSE_DISTANCE)
			{
				EndCreation();
				return MOUSECREATE_OK;
			}
			if (GetPointCount() >= GetMaxPoints())
			{
				//m_points.pop_back(); // Remove last uneeded point.
				EndCreation();
				return MOUSECREATE_OK;
			}

			InsertPoint( -1,pos-GetWorldPos() );
		}
		return MOUSECREATE_CONTINUE;
	}
	return CBaseObject::MouseCreateCallback( view,event,point,flags );
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::EndCreation()
{
	SetClosed(true);
	if (m_panel)
		m_panel->SetShape(this);
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::Display( DisplayContext &dc )
{
	//dc.renderer->EnableDepthTest(false);
	
	const Matrix44 &wtm = GetWorldTM();
	COLORREF col;

	float fPointSize = 0.5f;

	bool bPointSelected = false;
	if (m_selectedPoint >= 0 && m_selectedPoint < m_points.size())
	{
		bPointSelected = true;
	}

	if (m_points.size() > 1)
	{
		if ((IsSelected() || IsHighlighted()) && !bPointSelected)
		{
			col = dc.GetSelectedColor();
			dc.SetColor( col );
		}
		else
		{
			if (IsFrozen())
				dc.SetFreezeColor();
			else
				dc.SetColor( GetColor() );
			col = GetColor();
		}

		for (int i = 0; i < m_points.size(); i++)
		{
			int j = (i < m_points.size()-1) ? i+1 : 0;

			Vec3 p0 = wtm.TransformPointOLD(m_points[i]);
			//dc.renderer->DrawBall( p0,0.8f );

			float fScale = fPointSize*dc.view->GetScreenScaleFactor(p0) * 0.01f;
			Vec3 sz(fScale,fScale,fScale);
			dc.DrawWireBox( p0-sz,p0+sz );

			if (!mv_closed && j == 0 && i != 0)
				continue;
			
			Vec3 p1 = wtm.TransformPointOLD(m_points[j]);
			dc.DrawLine( p0,p1 );
			//DrawTerrainLine( dc,pos+m_points[i],pos+m_points[j] );
			
			if (mv_height != 0)
			{
				BBox box = m_bbox;
				box.Transform( GetWorldTM() );
				m_lowestHeight = box.min.z;
				// Draw second capping shape from above.
				float z = m_lowestHeight + mv_height;
				dc.DrawLine( p0,Vec3(p0.x,p0.y,z) );
				dc.DrawLine( Vec3(p0.x,p0.y,z),Vec3(p1.x,p1.y,z) );
			}
		}

		// Draw selected point.
		if (bPointSelected)
		{
			dc.SetSelectedColor( 0.5f );

			Vec3 selPos = wtm.TransformPointOLD(m_points[m_selectedPoint]);
			//dc.renderer->DrawBall( selPos,1.0f );
			float fScale = fPointSize*dc.view->GetScreenScaleFactor(selPos) * 0.01f;
			Vec3 sz(fScale,fScale,fScale);
			dc.DrawWireBox( selPos-sz,selPos+sz );

			DrawAxis( dc,m_points[m_selectedPoint],0.15f );
		}
	}

	if (!m_entities.IsEmpty())
	{
		Vec3 vcol = Rgb2Vec(col);
		int num = m_entities.GetCount();
		for (int i = 0; i < num; i++)
		{
			CBaseObject *obj = m_entities.Get(i);
			if (!obj)
				continue;
			int p1,p2;
			float dist;
			Vec3 intPnt;
			GetNearestEdge( obj->GetWorldPos(),p1,p2,dist,intPnt );
			dc.DrawLine( intPnt,obj->GetWorldPos(),CFColor(vcol.x,vcol.y,vcol.z,0.7f),CFColor(1,1,1,0.7f) );
		}
	}

	if (mv_closed && !IsFrozen())
	{
		if (mv_displayFilled || (IsSelected() && m_bDisplayFilledWhenSelected) || IsHighlighted())
		{
			if (IsHighlighted())
				dc.SetColor( GetColor(),0.1f );
			else
				dc.SetColor( GetColor(),0.3f );
			static std::vector<Vec3> tris;
			tris.resize(0);
			tris.reserve( m_points.size()*3 );
			if (CTriangulate::Process( m_points,tris ))
			{
				for (int i = 0; i < tris.size(); i += 3)
				{
					dc.DrawTri( wtm.TransformPointOLD(tris[i]),wtm.TransformPointOLD(tris[i+1]),wtm.TransformPointOLD(tris[i+2]) );
				}
			}
		}
	}
	
	//dc.renderer->EnableDepthTest(true);

	DrawDefault(dc,GetColor());
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::DrawTerrainLine( DisplayContext &dc,const Vec3 &p1,const Vec3 &p2 )
{
	float len = (p2-p1).Length();
	int steps = len/2;
	if (steps <= 1)
	{
		dc.DrawLine( p1,p2 );
		return;
	}
	Vec3 pos1 = p1;
	Vec3 pos2 = p1;
	for (int i = 0; i < steps-1; i++)
	{
		pos2 = p1 + (1.0f/i)*(p2-p1);
		pos2.z = dc.engine->GetTerrainElevation(pos2.x,pos2.y);
		dc.SetColor( i*2,0,0,1 );
		dc.DrawLine( pos1,pos2 );
		pos1 = pos2;
	}
	//dc.DrawLine( pos2,p2 );
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::Serialize( CObjectArchive &ar )
{
	m_bIgnoreGameUpdate = true;
	CBaseObject::Serialize( ar );
	m_bIgnoreGameUpdate = false;

	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		m_bAreaModified = true;
		m_bIgnoreGameUpdate = true;
		m_entities.Clear();

		GUID entityId;
		if (xmlNode->getAttr( "EntityId",entityId ))
		{
			// For Backward compatability.
			//m_entities.push_back(entityId);
			ar.SetResolveCallback( this,entityId,functor(m_entities,&CSafeObjectsArray::Add) );
		}

		// Load Entities.
		m_points.clear();
		XmlNodeRef points = xmlNode->findChild( "Points" );
		if (points)
		{
			for (int i = 0; i < points->getChildCount(); i++)
			{
				XmlNodeRef pnt = points->getChild(i);
				Vec3 pos;
				pnt->getAttr( "Pos",pos );
				m_points.push_back(pos);
			}
		}
		XmlNodeRef entities = xmlNode->findChild( "Entities" );
		if (entities)
		{
			for (int i = 0; i < entities->getChildCount(); i++)
			{
				XmlNodeRef ent = entities->getChild(i);
				GUID entityId;
				if (ent->getAttr( "Id",entityId ))
				{
					//m_entities.push_back(id);
					ar.SetResolveCallback( this,entityId,functor(m_entities,&CSafeObjectsArray::Add) );
				}
			}
		}

		if (ar.bUndo)
		{
			// Update game area only in undo mode.
			m_bIgnoreGameUpdate = false;
		}
		CalcBBox();
		UpdateGameArea();

		// Update UI.
		if (m_panel && m_panel->GetShape() == this)
			m_panel->SetShape(this);
	}
	else
	{
		// Saving.
		// Save Points.
		if (!m_points.empty())
		{
			XmlNodeRef points = xmlNode->newChild( "Points" );
			for (int i = 0; i < m_points.size(); i++)
			{
				XmlNodeRef pnt = points->newChild( "Point" );
				pnt->setAttr( "Pos",m_points[i] );
			}
		}
		// Save Entities.
		if (!m_entities.IsEmpty())
		{
			XmlNodeRef nodes = xmlNode->newChild( "Entities" );
			for (int i = 0; i < m_entities.GetCount(); i++)
			{
				XmlNodeRef entNode = nodes->newChild( "Entity" );
				if (m_entities.Get(i))
					entNode->setAttr( "Id",m_entities.Get(i)->GetId() );
			}
		}
	}
	m_bIgnoreGameUpdate = false;
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CShapeObject::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	XmlNodeRef objNode = CBaseObject::Export( levelPath,xmlNode );

	// Export position in world space.
	Matrix44 wtmNoScale = GetWorldTM();
	wtmNoScale.NoScale();

	//QUAT_CHANGED_BY_IVO
	//Quat q(wtmNoScale);
	//Quat q = CovertMatToQuat<float>( GetTransposed44(wtmNoScale) );
	Quat q = Quat( GetTransposed44(wtmNoScale) );

	Vec3 worldPos = wtmNoScale.GetTranslationOLD();
	Vec3 worldAngles = Ang3::GetAnglesXYZ(Matrix33(q)) * 180.0f/gf_PI;

	//if (worldPos != Vec3(0,0,0))
	if (!IsEquivalent(worldPos,Vec3(0,0,0)))
		objNode->setAttr( "Pos",worldPos );
		
	//if (worldAngles != Vec3(0,0,0))
	if (!IsEquivalent(worldAngles,Vec3(0,0,0),VEC_EPSILON))
		objNode->setAttr( "Angles",worldAngles );

	const Matrix44 &wtm = GetWorldTM();
	// Export Points
	if (!m_points.empty())
	{
		XmlNodeRef points = objNode->newChild( "Points" );
		for (int i = 0; i < m_points.size(); i++)
		{
			XmlNodeRef pnt = points->newChild( "Point" );
			pnt->setAttr( "Pos",wtm.TransformPointOLD(m_points[i]) );
		}
	}

	// Export Entities.
	if (!m_entities.IsEmpty())
	{
		XmlNodeRef nodes = objNode->newChild( "Entities" );
		for (int i = 0; i < m_entities.GetCount(); i++)
		{
			XmlNodeRef entNode = nodes->newChild( "Entity" );
			CBaseObject *obj = m_entities.Get(i);
			CString name;
			if (obj)
				name = obj->GetName();
			entNode->setAttr( "Name",name );
		}
	}

	return objNode;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::CalcBBox()
{
	if (m_points.empty())
		return;

	// Reposition shape, so that shape object position is in the middle of the shape.
	Vec3 center = m_points[0];
	if (center.x != 0 || center.y != 0 || center.z != 0)
	{
		// May not work correctly if shape is transformed.
		for (int i = 0; i < m_points.size(); i++)
		{
			m_points[i] -= center;
		}
		const Matrix44 &ltm = GetLocalTM();
		SetPos( GetPos() + ltm.TransformVectorOLD(center) );
	}

	// First point must always be 0,0,0.
	m_bbox.Reset();
	for (int i = 0; i < m_points.size(); i++)
	{
		m_bbox.Add( m_points[i] );
	}
	if (m_bbox.min.x > m_bbox.max.x)
	{
		m_bbox.min = m_bbox.max = Vec3(0,0,0);
	}
	BBox box = m_bbox;
	box.Transform( GetWorldTM() );
	m_lowestHeight = box.min.z;

	m_bbox.max.z = max( m_bbox.max.z,mv_height );
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::SetSelected( bool bSelect )
{
	bool bWasSelected = IsSelected();
	CBaseObject::SetSelected( bSelect );
	if (!bSelect && bWasSelected)
	{
		// When unselecting shape, update area in game.
		if (m_bAreaModified)
			UpdateGameArea();
		m_bAreaModified = false;
	}
}

//////////////////////////////////////////////////////////////////////////
int CShapeObject::InsertPoint( int index,const Vec3 &point )
{
	if (GetPointCount() >= GetMaxPoints())
		return GetPointCount()-1;
	int newindex;
	StoreUndo( "Insert Point" );

	m_bAreaModified = true;

	if (index < 0 || index >= m_points.size())
	{
		m_points.push_back( point );
		newindex = m_points.size()-1;
	}
	else
	{
		m_points.insert( m_points.begin()+index,point );
		newindex = index;
	}
	SetPoint( newindex,point );
	CalcBBox();
	return newindex;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::RemovePoint( int index )
{
	if ((index >= 0 || index < m_points.size()) && m_points.size() > 3)
	{
		m_bAreaModified = true;
		StoreUndo( "Remove Point" );
		m_points.erase( m_points.begin()+index );
		CalcBBox();
	}
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::PostClone( CBaseObject *pFromObject,CObjectCloneContext &ctx )
{
	CBaseObject::PostClone( pFromObject,ctx );

	CShapeObject *pFromShape = (CShapeObject*)pFromObject;
	// Clone event targets.
	if (!pFromShape->m_entities.IsEmpty())
	{
		int numEntities = pFromShape->m_entities.GetCount();
		for (int i = 0; i < numEntities; i++)
		{
			CBaseObject *pTarget = pFromShape->m_entities.Get(i);
			CBaseObject *pClonedTarget = ctx.FindClone( pTarget );
			if (!pClonedTarget)
				pClonedTarget = pTarget; // If target not cloned, link to original target.

			// Add cloned event.
			if (pClonedTarget)
				AddEntity( pClonedTarget );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::AddEntity( CBaseObject *entity )
{
	assert( entity );

	m_bAreaModified = true;

	StoreUndo( "Add Entity" );
	m_entities.Add( entity );
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::RemoveEntity( int index )
{
	assert( index >= 0 && index < m_entities.GetCount() );
	StoreUndo( "Remove Entity" );

	m_bAreaModified = true;

	if (index < m_entities.GetCount())
		m_entities.Remove( m_entities.Get(index) );
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CShapeObject::GetEntity( int index )
{
	assert( index >= 0 && index < m_entities.GetCount() );
	return m_entities.Get(index);
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::SetClosed( bool bClosed )
{
	StoreUndo( "Set Closed" );
	mv_closed = bClosed;

	m_bAreaModified = true;

	if (m_IArea && !mv_closed)
	{
		GetIEditor()->GetGame()->GetAreaManager()->DeleteArea( m_IArea );
		m_IArea = 0;
	}
	if (!m_IArea && mv_closed)
	{
		UpdateGameArea();
	}
}

//////////////////////////////////////////////////////////////////////////
int CShapeObject::GetAreaId()
{
	return mv_areaId;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::SelectPoint( int index )
{
	m_selectedPoint = index;
}

void CShapeObject::SetPoint( int index,const Vec3 &pos )
{
	Vec3 p = pos;
	if (m_bForce2D)
	{
		if (!m_points.empty())
			p.z = m_points[0].z; // Keep original Z.
	}
	if (index >= 0 && index < m_points.size())
	{
		m_points[index] = p;
	}
	m_bAreaModified = true;
	if (!IsOnlyUpdateOnUnselect())
		UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
int CShapeObject::GetNearestPoint( const Vec3 &raySrc,const Vec3 &rayDir,float &distance )
{
	int index = -1;
	float minDist = FLT_MAX;
	Vec3 rayLineP1 = raySrc;
	Vec3 rayLineP2 = raySrc + rayDir*RAY_DISTANCE;
	Vec3 intPnt;
	const Matrix44 &wtm = GetWorldTM();
	for (int i = 0; i < m_points.size(); i++)
	{
		float d = PointToLineDistance( rayLineP1,rayLineP2,wtm.TransformPointOLD(m_points[i]),intPnt );
		if (d < minDist)
		{
			minDist = d;
			index = i;
		}
	}
	distance = minDist;
	return index;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::GetNearestEdge( const Vec3 &pos,int &p1,int &p2,float &distance,Vec3 &intersectPoint )
{
	p1 = -1;
	p2 = -1;
	float minDist = FLT_MAX;
	Vec3 intPnt;
	
	const Matrix44 &wtm = GetWorldTM();

	for (int i = 0; i < m_points.size(); i++)
	{
		int j = (i < m_points.size()-1) ? i+1 : 0;

		if (!mv_closed && j == 0 && i != 0)
			continue;

		float d = PointToLineDistance( wtm.TransformPointOLD(m_points[i]),wtm.TransformPointOLD(m_points[j]),pos,intPnt );
		if (d < minDist)
		{
			minDist = d;
			p1 = i;
			p2 = j;
			intersectPoint = intPnt;
		}
	}
	distance = minDist;
}

//////////////////////////////////////////////////////////////////////////
bool CShapeObject::RayToLineDistance( const Vec3 &rayLineP1,const Vec3 &rayLineP2,const Vec3 &pi,const Vec3 &pj,float &distance,Vec3 &intPnt )
{
	Vec3 pa,pb;
	float ua,ub;
	if (!LineLineIntersect( pi,pj, rayLineP1,rayLineP2, pa,pb,ua,ub ))
		return false;

	// If before ray origin.
	if (ub < 0)
		return false;

	float d = 0;
	if (ua < 0)
		d = PointToLineDistance( rayLineP1,rayLineP2,pi,intPnt );
	else if (ua > 1)
		d = PointToLineDistance( rayLineP1,rayLineP2,pj,intPnt );
	else
	{
		intPnt = rayLineP1 + ub*(rayLineP2-rayLineP1);
		d = (pb-pa).Length();
	}
	distance = d;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::GetNearestEdge( const Vec3 &raySrc,const Vec3 &rayDir,int &p1,int &p2,float &distance,Vec3 &intersectPoint )
{
	p1 = -1;
	p2 = -1;
	float minDist = FLT_MAX;
	Vec3 intPnt;
	Vec3 rayLineP1 = raySrc;
	Vec3 rayLineP2 = raySrc + rayDir*RAY_DISTANCE;

	const Matrix44 &wtm = GetWorldTM();

	for (int i = 0; i < m_points.size(); i++)
	{
		int j = (i < m_points.size()-1) ? i+1 : 0;

		if (!mv_closed && j == 0 && i != 0)
			continue;

		Vec3 pi = wtm.TransformPointOLD(m_points[i]);
		Vec3 pj = wtm.TransformPointOLD(m_points[j]);

		float d = 0;
		if (!RayToLineDistance( rayLineP1,rayLineP2,pi,pj,d,intPnt ))
			continue;
		
		if (d < minDist)
		{
			minDist = d;
			p1 = i;
			p2 = j;
			intersectPoint = intPnt;
		}
	}
	distance = minDist;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::UpdateGameArea( bool bRemove )
{
	if (m_bIgnoreGameUpdate)
		return;
	
	if (!IsCreateGameObjects())
		return;

	if (bRemove)
	{
		if (m_IArea)
		{
			GetIEditor()->GetGame()->GetAreaManager()->DeleteArea( m_IArea );
			m_IArea = 0;
		}
		return;
	}

	if (m_IArea && !mv_closed)
	{
		GetIEditor()->GetGame()->GetAreaManager()->DeleteArea( m_IArea );
		m_IArea = 0;
	}

	if (!mv_closed)
		return;

	const Matrix44 &wtm = GetWorldTM();

	BBox box = m_bbox;
	box.Transform(wtm);
	m_lowestHeight = box.min.z;

	std::vector<Vec3> points;
	if (GetPointCount() > 0)
	{
		points.resize(GetPointCount());
		for (int i = 0; i < GetPointCount(); i++)
		{
			points[i] = wtm.TransformPointOLD( GetPoint(i) );
		}
	}

	if (m_IArea)
	{
		m_IArea->SetPoints( &points[0],points.size() );
	}
	else if (GetPointCount() > 2)
	{
		std::vector<string>	entitiesName;
		entitiesName.clear();

		m_IArea = GetIEditor()->GetGame()->GetAreaManager()->CreateArea( &points[0], points.size(), entitiesName, mv_areaId, mv_groupId, mv_width );
	}

	if (m_IArea)
	{
		m_IArea->SetID(mv_areaId);
		m_IArea->SetProximity(mv_width);
		m_IArea->SetVSize(mv_height);
		m_IArea->SetVOrigin(m_lowestHeight);
		m_IArea->SetGroup(mv_groupId);

		m_IArea->ClearEntities();
		for (int i = 0; i < GetEntityCount(); i++)
		{
			CBaseObject *obj = GetEntity(i);
			if (obj)
				m_IArea->AddEntity( obj->GetName() );
		}
	}
	m_bAreaModified = false;
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::OnShapeChange( IVariable *var )
{
	m_bAreaModified = true;
	if (!m_bIgnoreGameUpdate)
		UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CShapeObject::OnEvent( ObjectEvent event )
{
	if (event == EVENT_AFTER_LOAD)
	{
		// After loading Update game structure.
		UpdateGameArea();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CShapeObject::HitTestRect( HitContext &hc )
{
	BBox box;
	// Retrieve world space bound box.
	GetBoundBox( box );

	// Project all edges to viewport.
	const Matrix44 &wtm = GetWorldTM();
	for (int i = 0; i < m_points.size(); i++)
	{
		int j = (i < m_points.size()-1) ? i+1 : 0;
		if (!mv_closed && j == 0 && i != 0)
			continue;

		Vec3 p0 = wtm.TransformPointOLD(m_points[i]);
		Vec3 p1 = wtm.TransformPointOLD(m_points[j]);

		CPoint pnt0 = hc.view->WorldToView( p0 );
		CPoint pnt1 = hc.view->WorldToView( p1 );

		// See if pnt0 to pnt1 line intersects with rectangle.
		// check see if one point is inside rect and other outside, or both inside.
		bool in0 = hc.rect.PtInRect(pnt0);
		bool in1 = hc.rect.PtInRect(pnt0);
		if ((in0 && in1) || (in0 && !in1) || (!in0 && in1))
		{
			hc.object = this;
			return true;
		}
	}

	return false;

/*

	// transform all 8 vertices into world space
	CPoint p[8] = 
	{ 
		hc.view->WorldToView(Vec3d(box.min.x,box.min.y,box.min.z)),
			hc.view->WorldToView(Vec3d(box.min.x,box.max.y,box.min.z)),
			hc.view->WorldToView(Vec3d(box.max.x,box.min.y,box.min.z)),
			hc.view->WorldToView(Vec3d(box.max.x,box.max.y,box.min.z)),
			hc.view->WorldToView(Vec3d(box.min.x,box.min.y,box.max.z)),
			hc.view->WorldToView(Vec3d(box.min.x,box.max.y,box.max.z)),
			hc.view->WorldToView(Vec3d(box.max.x,box.min.y,box.max.z)),
			hc.view->WorldToView(Vec3d(box.max.x,box.max.y,box.max.z))
	};

	CRect objrc,temprc;

	objrc.left = 10000;
	objrc.top = 10000;
	objrc.right = -10000;
	objrc.bottom = -10000;
	// find new min/max values
	for(int i=0; i<8; i++)
	{
		objrc.left = min(objrc.left,p[i].x);
		objrc.right = max(objrc.right,p[i].x);
		objrc.top = min(objrc.top,p[i].y);
		objrc.bottom = max(objrc.bottom,p[i].y);
	}
	if (objrc.IsRectEmpty())
	{
		// Make objrc at least of size 1.
		objrc.bottom += 1;
		objrc.right += 1;
	}
	if (temprc.IntersectRect( objrc,hc.rect ))
	{
		hc.object = this;
		return true;
	}
	return false;
	*/
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CAIForbiddenAreaObject
//////////////////////////////////////////////////////////////////////////
CAIForbiddenAreaObject::CAIForbiddenAreaObject()
{
	m_bDisplayFilledWhenSelected = true;
	SetColor( RGB(255,0,0) );
}

//////////////////////////////////////////////////////////////////////////
void CAIForbiddenAreaObject::UpdateGameArea( bool bRemove )
{
	if (m_bIgnoreGameUpdate)
		return;
	if (!IsCreateGameObjects())
		return;

	IAISystem *ai = GetIEditor()->GetSystem()->GetAISystem();
	if (ai)
	{
		ai->DeletePath( GetName() );
		ai->CreatePath( GetName() , AREATYPE_FORBIDDEN);
			

		if (bRemove)
			return;

		const Matrix44 &wtm = GetWorldTM();
		for (int i = 0; i < GetPointCount(); i++)
		{
			ai->AddPointToPath( wtm.TransformPointOLD(GetPoint(i)),GetName(),AREATYPE_FORBIDDEN );
		}
	}
	m_bAreaModified = false;
}

//////////////////////////////////////////////////////////////////////////
// CAIPathObject.
//////////////////////////////////////////////////////////////////////////
CAIPathObject::CAIPathObject()
{
	SetColor( RGB(180,180,180) );
}

void CAIPathObject::UpdateGameArea( bool bRemove )
{
	if (m_bIgnoreGameUpdate)
		return;
	if (!IsCreateGameObjects())
		return;

	IAISystem *ai = GetIEditor()->GetSystem()->GetAISystem();
	if (ai)
	{
		ai->DeletePath( GetName() );
		ai->CreatePath( GetName() );

		if (bRemove)
			return;

		const Matrix44 &wtm = GetWorldTM();
		for (int i = 0; i < GetPointCount(); i++)
		{
			ai->AddPointToPath( wtm.TransformPointOLD(GetPoint(i)),GetName() );
		}
	}
	m_bAreaModified = false;
}

//////////////////////////////////////////////////////////////////////////
// CAINavigationModifierObject
//////////////////////////////////////////////////////////////////////////
CAINavigationModifierObject::CAINavigationModifierObject()
{
	m_bDisplayFilledWhenSelected = true;
	SetColor( RGB(24,128,231) );
}

void CAINavigationModifierObject::UpdateGameArea( bool bRemove )
{
	if (m_bIgnoreGameUpdate)
		return;
	if (!IsCreateGameObjects())
		return;

	IAISystem *ai = GetIEditor()->GetSystem()->GetAISystem();
	if (ai)
	{
		ai->DeletePath( GetName() );
		ai->CreatePath( GetName(),AREATYPE_NAVIGATIONMODIFIER,GetHeight());

		if (bRemove)
			return;

		const Matrix44 &wtm = GetWorldTM();
		for (int i = 0; i < GetPointCount(); i++)
		{
			ai->AddPointToPath( wtm.TransformPointOLD(GetPoint(i)),GetName(),AREATYPE_NAVIGATIONMODIFIER );
		}
	}
	m_bAreaModified = false;
}

//////////////////////////////////////////////////////////////////////////
// CAIOclussionPlaneObject
//////////////////////////////////////////////////////////////////////////
CAIOcclusionPlaneObject::CAIOcclusionPlaneObject()
{
	m_bDisplayFilledWhenSelected = true;
	SetColor( RGB(24,90,231) );
	m_bForce2D = true;
	mv_closed = true;
	mv_displayFilled = true;
}

void CAIOcclusionPlaneObject::UpdateGameArea( bool bRemove )
{
	if (m_bIgnoreGameUpdate)
		return;
	if (!IsCreateGameObjects())
		return;

	IAISystem *ai = GetIEditor()->GetSystem()->GetAISystem();
	if (ai)
	{
		ai->DeletePath( GetName() );
		ai->CreatePath( GetName(),AREATYPE_OCCLUSION_PLANE,GetHeight());

		if (bRemove)
			return;

		const Matrix44 &wtm = GetWorldTM();
		for (int i = 0; i < GetPointCount(); i++)
		{
			ai->AddPointToPath( wtm.TransformPointOLD(GetPoint(i)),GetName(),AREATYPE_OCCLUSION_PLANE );
		}
	}
	m_bAreaModified = false;
}