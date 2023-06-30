////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   AreaBox.cpp
//  Version:     v1.00
//  Created:     22/10/2002 by Lennert.
//  Compilers:   Visual C++ 6.0
//  Description: CAreaBox implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AreaBox.h"
#include "ObjectManager.h"
#include "..\Viewport.h"
#include <IEntitySystem.h>
#include <IGame.h>
#include <IMarkers.h>

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CAreaBox,CBaseObject)

int CAreaBox::m_nRollupId=0;
CPickEntitiesPanel* CAreaBox::m_pPanel=NULL;

//////////////////////////////////////////////////////////////////////////
CAreaBox::CAreaBox()
{
	m_IArea=NULL;
	m_areaId = -1;
	m_edgeWidth=0;
	mv_width = 4;
	mv_length = 4;
	mv_height = 1;
	mv_groupId = 0;

	m_box.min=Vec3(-mv_width/2, -mv_length/2, 0);
	m_box.max=Vec3(mv_width/2, mv_length/2, mv_height);

	AddVariable( m_areaId,"AreaId",functor(*this,&CAreaBox::OnAreaChange) );
	AddVariable( m_edgeWidth,"FadeInZone",functor(*this,&CAreaBox::OnSizeChange) );
	AddVariable( mv_width,"Width",functor(*this,&CAreaBox::OnSizeChange) );
	AddVariable( mv_length,"Length",functor(*this,&CAreaBox::OnSizeChange) );
	AddVariable( mv_height,"Height",functor(*this,&CAreaBox::OnSizeChange) );
	AddVariable( mv_groupId,"GroupId",functor(*this,&CAreaBox::OnAreaChange) );
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::Done()
{
	if (m_IArea)
	{
		GetIEditor()->GetGame()->GetAreaManager()->DeleteArea( m_IArea );
		m_IArea=NULL;
	}
	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CAreaBox::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	bool res = CBaseObject::Init( ie,prev,file );
	SetColor( RGB(0,0,255) );
	return res;
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::GetBoundBox( BBox &box )
{
	box = m_box;
	box.Transform( GetWorldTM() );
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::GetLocalBounds( BBox &box )
{
	box = m_box;
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::SetScale( const Vec3d &scale )
{
	// Ignore scale on CAreaBox.
}

//////////////////////////////////////////////////////////////////////////
bool CAreaBox::HitTest( HitContext &hc )
{
	Vec3 p;
	/*BBox box;
	box = m_box;
	box.Transform( GetWorldTM() );
	float tr = hc.distanceTollerance/2;
	box.min -= Vec3(tr,tr,tr);
	box.max += Vec3(tr,tr,tr);
	if (box.IsIntersectRay(hc.raySrc,hc.rayDir,p ))
	{
		hc.dist = Vec3(hc.raySrc - p).Length();
		return true;
	}*/
	Matrix44 invertWTM = GetWorldTM();
	Vec3 worldPos = invertWTM.GetTranslationOLD();
	invertWTM.Invert44();

	Vec3 xformedRaySrc = invertWTM.TransformPointOLD(hc.raySrc);
  //CHANGED_BY_IVO
	Vec3 xformedRayDir = GetNormalized( invertWTM.TransformVectorOLD(hc.rayDir) );

	float epsilonDist = hc.view->GetScreenScaleFactor( worldPos ) * 0.01f;
	float hitDist;

	float tr = hc.distanceTollerance/2 + 1;
	BBox box;
	box.min = m_box.min - Vec3(tr+epsilonDist,tr+epsilonDist,tr+epsilonDist);
	box.max = m_box.max + Vec3(tr+epsilonDist,tr+epsilonDist,tr+epsilonDist);
	if (box.IsIntersectRay( xformedRaySrc,xformedRayDir,p ))
	{
		if (m_box.RayEdgeIntersection( xformedRaySrc,xformedRayDir,epsilonDist,hitDist,p ))
		{
			hc.dist = GetDistance(xformedRaySrc,p);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::BeginEditParams( IEditor *ie,int flags )
{
	CBaseObject::BeginEditParams( ie,flags );
	if (!m_pPanel)
	{
		m_pPanel = new CPickEntitiesPanel;
		m_pPanel->Create( CPickEntitiesPanel::IDD,AfxGetMainWnd() );
		m_nRollupId = ie->AddRollUpPage( ROLLUP_OBJECTS,"Attached Entities",m_pPanel );
	}
	if (m_pPanel)
		m_pPanel->SetOwner(this);
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::EndEditParams( IEditor *ie )
{
	if (m_nRollupId)
		ie->RemoveRollUpPage( ROLLUP_OBJECTS, m_nRollupId);
	m_nRollupId = 0;
	m_pPanel = NULL;
	CBaseObject::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::OnAreaChange(IVariable *pVar)
{
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::OnSizeChange(IVariable *pVar)
{
	Vec3 size( 0,0,0 );
	size.x = mv_width;
	size.y = mv_length;
	size.z = mv_height;

	m_box.min = -size/2;
	m_box.max = size/2;
	// Make volume position bottom of bounding box.
	m_box.min.z = 0;
	m_box.max.z = size.z;
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::Display( DisplayContext &dc )
{
	COLORREF wireColor,solidColor;
	float wireOffset = 0;
	float alpha = 0.3f;
	if (IsSelected())
	{
		wireColor = dc.GetSelectedColor();
		solidColor = GetColor();
		wireOffset = -0.1f;
	}
	else
	{
		wireColor = GetColor();
		solidColor = GetColor();
	}
	
	//dc.renderer->SetCullMode( R_CULL_DISABLE );

	dc.PushMatrix( GetWorldTM() );
	BBox box=m_box;
	
	bool bFrozen = IsFrozen();
	
	if (bFrozen)
		dc.SetFreezeColor();
	//////////////////////////////////////////////////////////////////////////
	if (!bFrozen)
		dc.SetColor( solidColor,alpha );

	if (IsSelected())
	{
		int rstate = dc.ClearStateFlag( GS_DEPTHWRITE );
		dc.renderer->SetCullMode( R_CULL_DISABLE );
		dc.DrawSolidBox( box.min,box.max );
	  dc.SetState( rstate );
		dc.renderer->SetCullMode( R_CULL_BACK );
	}

	if (!bFrozen)
		dc.SetColor( wireColor,1 );
	dc.DrawWireBox( box.min,box.max,wireOffset );
	if (m_edgeWidth)
	{
		float fFadeScale=m_edgeWidth/200.0f;
		BBox InnerBox=box;
		Vec3 EdgeDist=InnerBox.max-InnerBox.min;
		InnerBox.min.x+=EdgeDist.x*fFadeScale; InnerBox.max.x-=EdgeDist.x*fFadeScale;
		InnerBox.min.y+=EdgeDist.y*fFadeScale; InnerBox.max.y-=EdgeDist.y*fFadeScale;
		InnerBox.min.z+=EdgeDist.z*fFadeScale; InnerBox.max.z-=EdgeDist.z*fFadeScale;
		dc.DrawWireBox( InnerBox.min,InnerBox.max );
	}
	//////////////////////////////////////////////////////////////////////////

	dc.PopMatrix();

	if (!m_entities.empty())
	{
		Vec3 vcol = Vec3(1, 1, 1);
		for (int i = 0; i < m_entities.size(); i++)
		{
			CBaseObject *obj = GetEntity(i);
			if (!obj)
				continue;
			dc.DrawLine(GetWorldPos(), obj->GetWorldPos(), CFColor(vcol.x,vcol.y,vcol.z,0.7f), CFColor(1,1,1,0.7f) );
		}
	}

	DrawDefault(dc);
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::InvalidateTM()
{
	CBaseObject::InvalidateTM();
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::Serialize( CObjectArchive &ar )
{
	CBaseObject::Serialize( ar );
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		m_entities.clear();


		BBox box;
		xmlNode->getAttr( "BoxMin",box.min );
		xmlNode->getAttr( "BoxMax",box.max );

		// Load Entities.
		XmlNodeRef entities = xmlNode->findChild( "Entities" );
		if (entities)
		{
			for (int i = 0; i < entities->getChildCount(); i++)
			{
				XmlNodeRef ent = entities->getChild(i);
				GUID entityId;
				ent->getAttr( "Id",entityId );
				m_entities.push_back(entityId);
			}
		}
		SetAreaId( m_areaId );
		SetBox(box);
	}
	else
	{
		// Saving.
//		xmlNode->setAttr( "AreaId",m_areaId );
		xmlNode->setAttr( "BoxMin",m_box.min );
		xmlNode->setAttr( "BoxMax",m_box.max );
		// Save Entities.
		if (!m_entities.empty())
		{
			XmlNodeRef nodes = xmlNode->newChild( "Entities" );
			for (int i = 0; i < m_entities.size(); i++)
			{
				XmlNodeRef entNode = nodes->newChild( "Entity" );
				entNode->setAttr( "Id",m_entities[i] );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CAreaBox::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	XmlNodeRef objNode = CBaseObject::Export( levelPath,xmlNode );
	Matrix44 wtm = GetWorldTM();
//	objNode->setAttr( "AreaId",m_areaId );
//	objNode->setAttr( "EdgeWidth",m_edgeWidth );
	// Export Entities.
	if (!m_entities.empty())
	{
		XmlNodeRef nodes = objNode->newChild( "Entities" );
		for (int i = 0; i < m_entities.size(); i++)
		{
			XmlNodeRef entNode = nodes->newChild( "Entity" );
			CBaseObject *obj = FindObject(m_entities[i]);
			CString name;
			if (obj)
				name = obj->GetName();
			entNode->setAttr( "Name",name );
		}
	}
	return objNode;
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::SetAreaId(int nAreaId)
{
	m_areaId=nAreaId;
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
int CAreaBox::GetAreaId()
{
	return m_areaId;
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::SetBox(BBox box)
{
	m_box=box;
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
BBox CAreaBox::GetBox()
{
	return m_box;
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::AddEntity( CBaseObject *pEntity )
{
	assert( pEntity );
	// Check if this entity already binded.
	if (std::find(m_entities.begin(),m_entities.end(),pEntity->GetId()) != m_entities.end())
		return;

	StoreUndo( "Add Entity" );
	m_entities.push_back(pEntity->GetId());
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::RemoveEntity( int index )
{
	assert( index >= 0 && index < m_entities.size() );
	StoreUndo( "Remove Entity" );

	m_entities.erase( m_entities.begin()+index );
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CAreaBox::GetEntity( int index )
{
	assert( index >= 0 && index < m_entities.size() );
	return GetIEditor()->GetObjectManager()->FindObject(m_entities[index]);
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::OnEvent( ObjectEvent event )
{
	if (event == EVENT_AFTER_LOAD)
	{
		// After loading Update game structure.
		UpdateGameArea();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAreaBox::UpdateGameArea()
{
	if (!IsCreateGameObjects())
		return;

//	AffineParts affineParts;
//	affineParts.SpectralDecompose(GetWorldTM());
//	Vec3 angles = RAD2DEG(affineParts.rot.GetEulerAngles());

	BBox box = m_box;
//	box.min+=affineParts.pos;
//	box.max+=affineParts.pos;

	if (m_IArea)
	{
		m_IArea->SetMin(box.min);
		m_IArea->SetMax(box.max);
		m_IArea->SetTM(GetWorldTM());
		m_IArea->SetProximity( m_edgeWidth );
		m_IArea->SetID(m_areaId);
		m_IArea->SetGroup(mv_groupId);
	}
	else
	{
		std::vector<string>	entitiesName;
		entitiesName.clear();
//areaFix
//		m_IArea = GetIEditor()->GetGame()->CreateArea(box.min, box.max, GetWorldTM(), entitiesName, m_areaId, m_edgeWidth );
		m_IArea = GetIEditor()->GetGame()->GetAreaManager()->CreateArea(box.min, box.max, GetWorldTM(), entitiesName, m_areaId, -1, m_edgeWidth );
		if (m_IArea)
		{
			m_IArea->SetMin(box.min);
			m_IArea->SetMax(box.max);
			m_IArea->SetTM(GetWorldTM());
			m_IArea->SetProximity( m_edgeWidth );
			m_IArea->SetID(m_areaId);
			m_IArea->SetGroup(mv_groupId);
		}
	}

	if (m_IArea)
	{
		m_IArea->ClearEntities();
		for (int i = 0; i < GetEntityCount(); i++)
		{
			CBaseObject *obj = GetEntity(i);
			if (obj)
				m_IArea->AddEntity( obj->GetName() );
		}
	}
}