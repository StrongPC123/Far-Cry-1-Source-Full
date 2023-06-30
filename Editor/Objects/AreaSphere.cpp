////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   AreaSphere.cpp
//  Version:     v1.00
//  Created:     25/10/2002 by Lennert.
//  Compilers:   Visual C++ 6.0
//  Description: CAreaSphere implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AreaSphere.h"
#include "ObjectManager.h"
#include "..\Viewport.h"
#include <IEntitySystem.h>
#include <IGame.h>
#include <IMarkers.h>

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CAreaSphere,CBaseObject)

int CAreaSphere::m_nRollupId=0;
CPickEntitiesPanel* CAreaSphere::m_pPanel=NULL;

//////////////////////////////////////////////////////////////////////////
CAreaSphere::CAreaSphere()
{
	m_IArea=NULL;
	m_areaId = -1;
	m_edgeWidth=0;
	m_radius = 3;
	mv_groupId = 0;

	AddVariable( m_areaId,"AreaId",functor(*this,&CAreaSphere::OnAreaChange) );
	AddVariable( m_edgeWidth,"FadeInZone",functor(*this,&CAreaSphere::OnSizeChange) );
	AddVariable( m_radius,"Radius",functor(*this,&CAreaSphere::OnSizeChange) );
	AddVariable( mv_groupId,"GroupId",functor(*this,&CAreaSphere::OnAreaChange) );
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::Done()
{
	if (m_IArea)
	{
		GetIEditor()->GetGame()->GetAreaManager()->DeleteArea( m_IArea );
		m_IArea=NULL;
	}
	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CAreaSphere::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	bool res = CBaseObject::Init( ie,prev,file );
	SetColor( RGB(0,0,255) );

	return res;
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::GetBoundBox( BBox &box )
{
	box.min=Vec3d(-m_radius, -m_radius, -m_radius);
	box.max=Vec3d(m_radius, m_radius, m_radius);
	box.Transform( GetWorldTM() );
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::GetLocalBounds( BBox &box )
{
	box.min = Vec3d(-m_radius, -m_radius, -m_radius);
	box.max = Vec3d(m_radius, m_radius, m_radius);
}

void CAreaSphere::SetAngles( const Vec3d &angles )
{
	// Ignore angles on CAreaBox.
}
	
void CAreaSphere::SetScale( const Vec3d &scale )
{
	// Ignore scale on CAreaBox.
}

//////////////////////////////////////////////////////////////////////////
bool CAreaSphere::HitTest( HitContext &hc )
{
	Vec3 origin = GetWorldPos();
	Vec3 w = origin - hc.raySrc;
	w = hc.rayDir.Cross( w );
	float d = w.Length();

	Matrix44 invertWTM = GetWorldTM();
	Vec3 worldPos = invertWTM.GetTranslationOLD();
	float epsilonDist = hc.view->GetScreenScaleFactor( worldPos ) * 0.01f;
	if ((d < m_radius + epsilonDist) &&
			(d > m_radius - epsilonDist))
	{
		hc.dist = GetDistance(hc.raySrc,origin);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::BeginEditParams( IEditor *ie,int flags )
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
void CAreaSphere::EndEditParams( IEditor *ie )
{
	if (m_nRollupId)
		ie->RemoveRollUpPage( ROLLUP_OBJECTS, m_nRollupId);
	m_nRollupId = 0;
	m_pPanel = NULL;
	CBaseObject::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::OnAreaChange(IVariable *pVar)
{
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::OnSizeChange(IVariable *pVar)
{
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::Display( DisplayContext &dc )
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
	

	const Matrix44 &tm = GetWorldTM();
	Vec3 pos = GetWorldPos();	
	
	bool bFrozen = IsFrozen();
	
	if (bFrozen)
		dc.SetFreezeColor();
	//////////////////////////////////////////////////////////////////////////
	if (!bFrozen)
		dc.SetColor( solidColor,alpha );

	if (IsSelected())
	{
		dc.renderer->SetCullMode( R_CULL_DISABLE );
		int rstate = dc.ClearStateFlag( GS_DEPTHWRITE );
		dc.DrawBall( pos, m_radius );
		dc.SetState( rstate );
		dc.renderer->SetCullMode( R_CULL_BACK );
	}

	if (!bFrozen)
		dc.SetColor( wireColor,1 );
	dc.DrawWireSphere( pos, m_radius );
	if (m_edgeWidth)
		dc.DrawWireSphere( pos, m_radius-m_edgeWidth );
	//////////////////////////////////////////////////////////////////////////

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
void CAreaSphere::Serialize( CObjectArchive &ar )
{
	CBaseObject::Serialize( ar );
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		m_entities.clear();

		BBox box;
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
	}
	else
	{
		// Saving.
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
XmlNodeRef CAreaSphere::Export( const CString &levelPath,XmlNodeRef &xmlNode )
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
void CAreaSphere::SetAreaId(int nAreaId)
{
	m_areaId=nAreaId;
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
int CAreaSphere::GetAreaId()
{
	return m_areaId;
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::SetRadius(float fRadius)
{
	m_radius=fRadius;
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
float CAreaSphere::GetRadius()
{
	return m_radius;
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::AddEntity( CBaseObject *pEntity )
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
void CAreaSphere::RemoveEntity( int index )
{
	assert( index >= 0 && index < m_entities.size() );
	StoreUndo( "Remove Entity" );

	m_entities.erase( m_entities.begin()+index );
	UpdateGameArea();
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CAreaSphere::GetEntity( int index )
{
	assert( index >= 0 && index < m_entities.size() );
	return GetIEditor()->GetObjectManager()->FindObject(m_entities[index]);
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::OnEvent( ObjectEvent event )
{
	if (event == EVENT_AFTER_LOAD)
	{
		// After loading Update game structure.
		UpdateGameArea();
	}
}

//////////////////////////////////////////////////////////////////////////
void CAreaSphere::UpdateGameArea()
{
	if (!IsCreateGameObjects())
		return;

	Vec3 Pos=GetWorldPos();

	if (m_IArea)
	{
		m_IArea->SetCenter(Pos);
		m_IArea->SetRadius(m_radius);
		m_IArea->SetProximity( m_edgeWidth );
		m_IArea->SetID(m_areaId);
		m_IArea->SetGroup(mv_groupId);
	}
	else
	{
		std::vector<string>	entitiesName;
		entitiesName.clear();
//areaFix
//		m_IArea = GetIEditor()->GetGame()->CreateArea(Pos, m_radius, entitiesName, m_areaId, m_edgeWidth );
		m_IArea = GetIEditor()->GetGame()->GetAreaManager()->CreateArea(Pos, m_radius, entitiesName, m_areaId, -1, m_edgeWidth );
		if (m_IArea)
		{
			m_IArea->SetCenter(Pos);
			m_IArea->SetRadius(m_radius);
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