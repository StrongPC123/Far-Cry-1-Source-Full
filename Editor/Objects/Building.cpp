////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Building.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: StaticObject implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Building.h"
#include "Entity.h"

#include "..\BuildingPanel.h"
#include "..\Viewport.h"

#include <I3DEngine.h>
#include "IPhysics.h"
#include <I3DIndoorEngine.h>

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CBuilding,CGroup)

int CBuilding::m_rollupId;
CBuildingPanel* CBuilding::m_panel = 0;

bool CBuilding::m_portals = true;

//////////////////////////////////////////////////////////////////////////
CBuilding::CBuilding()
{
	// Initial area size.
	m_bbox.min=Vec3d(-1,-1,-1);
	m_bbox.max=Vec3d(1,1,1);

	m_buildingId = -1;
	m_bAlwaysDrawBox = false;
	m_wireFrame = false;

	// Building is initially open.
	Open();
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::Done()
{
	if (ValidBuildingId(m_buildingId))
	{
		// Delete indoor building.
		GetBuildMgr()->DeleteBuilding( m_buildingId );
		m_buildingId = 0;
	}
	UnbindAllHelpers();
	m_helpers.clear();

	CGroup::Done();
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::ReleaseBuilding()
{
	if (ValidBuildingId(m_buildingId))
	{
		// Delete indoor building.
		GetBuildMgr()->DeleteBuilding( m_buildingId );
	}
	m_buildingId = -1;
}

//////////////////////////////////////////////////////////////////////////
bool CBuilding::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	m_ie = ie;

	SetColor( RGB(255,255,255) );

	CGroup::Init( ie,prev,file );
	if (prev)
	{
		LoadBuilding( ((CBuilding*)prev)->GetObjectName() );
	}
	else if (!file.IsEmpty())
	{
		LoadBuilding( file );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::SetPos( const Vec3d &pos )
{
	CGroup::SetPos(pos);
	if (ValidBuildingId(m_buildingId))
	{
		// Move indoor building.
		GetBuildMgr()->SetBuildingPos( GetWorldPos(),m_buildingId );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::SetAngles( const Vec3d &angles )
{
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::SetScale( const Vec3d &scale )
{
}

///////////////////////////////////////////////////////////////////
void CBuilding::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	CGroup::BeginEditParams( ie,flags );

	if (!m_panel)
	{
		m_panel = new CBuildingPanel(0);
		m_panel->Create( CBuildingPanel::IDD,AfxGetMainWnd() );
		m_rollupId = ie->AddRollUpPage( ROLLUP_OBJECTS,"Building Parameters",m_panel );
	}

	m_panel->SetBuilding( this );
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::EndEditParams( IEditor *ie )
{
	if (m_rollupId != 0)
		ie->RemoveRollUpPage( ROLLUP_OBJECTS,m_rollupId );
	m_rollupId = 0;
	m_panel = 0;

	CGroup::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::GetBoundBox( BBox &box )
{
	CGroup::GetBoundBox( box );
	/*
	box = m_box;
	box.min += GetPos();
	box.max += GetPos();
	*/
}

//////////////////////////////////////////////////////////////////////////
bool CBuilding::HitTest( HitContext &hc )
{
	bool bHit = CGroup::HitTestChilds( hc );
	if (bHit)
		return true;

	Vec3 p;

	float tr = hc.distanceTollerance/2;
	BBox box;
	box.min = GetWorldPos() + m_bbox.min - Vec3(tr,tr,tr);
	box.max = GetWorldPos() + m_bbox.max + Vec3(tr,tr,tr);
	if (box.IsIntersectRay(hc.raySrc,hc.rayDir,p ))
	{
		//hc.dist = Vec3(hc.raySrc - p).Length();

		if (ValidBuildingId(m_buildingId))
		{
			//! If camera source is inside building dont select it.
			if (GetBuildMgr()->CheckInside( hc.raySrc,m_buildingId))
				return false;

			//Vec3d src = hc.raySrc;
			//Vec3d trg = hc.raySrc + hc.rayDir*10000.0f;
//			IndoorRayIntInfo hit;
			//ASSERT(0);
			//return (false);
			//please use the physics code for ray-inters.
			//if (!GetBuildMgr()->RayIntersection( src,trg,hit,m_buildingId ))
			//	return false;

			vectorf vPos(hc.raySrc);
			vectorf vDir(hc.rayDir);
			int objTypes = ent_static;
			int flags = rwi_stop_at_pierceable|rwi_ignore_terrain_holes;
			ray_hit hit;
			int col = GetIEditor()->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection( vPos,vDir,objTypes,flags,&hit,1 );
			if (col > 0)
			{
				IStatObj *pStatObj = (IStatObj*)hit.pCollider->GetForeignData();
				if (GetBuildMgr()->GetBuildingIdFromObj( pStatObj ) == m_buildingId)
				{
					hc.dist = hit.dist;
					return true;
				}
			}
			//hc.geometryHit = true;
		}
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::Display( DisplayContext &dc )
{
	CGroup::Display( dc );

	/*
	// Create buildling if not done already.
	DrawDefault(dc,GetColor());

	if (IsSelected())
	{
		float t = m_ie->GetSystem()->GetITimer()->GetCurrTime();
		float r1 = (float)fabs(sin(t*8.0f));
//			float r2 = cos(t*3);
		dc.renderer->SetMaterialColor( 1,0,r1,0.5f );
		//dc.renderer->DrawBall( GetPos(),1.3f );

		dc.renderer->PushMatrix();
		dc.renderer->TranslateMatrix( GetPos() );
		dc.renderer->RotateMatrix( GetAngles() );
		dc.renderer->Draw3dBBox( m_bbox.min,m_bbox.max );
		dc.renderer->PopMatrix();
	}
	*/
	
	Vec3 origin = GetWorldPos();
	// Display helpers.
	dc.SetColor( 0,1,1,0.3f );
	for (int i = 0; i < m_helpers.size(); i++)
	{
		if (m_helpers[i].object == 0)
		{
			dc.renderer->DrawBall( origin+m_helpers[i].pos,0.3f );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::OnEvent( ObjectEvent event )
{
	CGroup::OnEvent(event);

	switch (event)
	{
		case EVENT_INGAME:
			GetBuildMgr()->EnableVisibility( true );
		break;
		
		case EVENT_OUTOFGAME:
			GetBuildMgr()->EnableVisibility( m_portals );
		break;

		case EVENT_RELOAD_GEOM:
			LoadBuilding( m_objectName,true );
			break;

		case EVENT_UNLOAD_GEOM:
			ReleaseBuilding();
			break;
	}
};

//////////////////////////////////////////////////////////////////////////
void CBuilding::Serialize( CObjectArchive &ar )
{
	CGroup::Serialize( ar );
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		bool wireFrame = false;
		xmlNode->getAttr( "Wireframe",wireFrame );
		//xmlNode->getAttr( "Portals",portals );
		SetWireframe( wireFrame );

		for (int i = 0; i < xmlNode->getChildCount(); i++)
		{
			XmlNodeRef cgf = xmlNode->getChild(i);
			CString name;
			if (cgf->isTag("Cgf") && cgf->getAttr("Name",name))
			{
				LoadBuilding( name );
				// Only one object.
				break;
			}
		}

		// Load information about objects attached to helpers.
		SerializeHelpers( ar );
	}
	else
	{
		// Saving.
		xmlNode->setAttr( "Wireframe",m_wireFrame );
		//xmlNode->setAttr( "Portals",m_portals );
		// Only one object.
		XmlNodeRef cgf = xmlNode->newChild("Cgf");
		cgf->setAttr( "Name",m_objectName );

		// Serialize helpers that have attached objects.
		SerializeHelpers( ar );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::SerializeHelpers( CObjectArchive &ar )
{
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		// Load information about objects attached to helpers.
		XmlNodeRef helpersNode = xmlNode->findChild( "Helperes" );
		if (helpersNode)
		{
			for (int i = 0; i < helpersNode->getChildCount(); i++)
			{
				XmlNodeRef helperNode = helpersNode->getChild(i);
				CString helperName;
				GUID objectId = GUID_NULL;
				helperNode->getAttr( "Id",objectId );
				helperNode->getAttr( "Name",helperName );
				for (int j = 0; j < m_helpers.size(); j++)
				{
					if (stricmp(m_helpers[j].name,helperName)==0)
					{
						ar.SetResolveCallback( objectId,functor(*this,ResolveHelper),j );
					}
				}
			}
		}
	}
	else
	{
		XmlNodeRef helpersNode = xmlNode->newChild( "Helperes" );

		// Serialize helpers that have attached objects.
		for (int i = 0; i < m_helpers.size(); i++)
		{
			ObjectHelper &helper = m_helpers[i];
			if (helper.object != 0)
			{
				XmlNodeRef helperNode = helpersNode->newChild( "Helper" );
				helperNode->setAttr( "Id",helper.object->GetId() );
				helperNode->setAttr( "Name",helper.name );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::ResolveHelper( CBaseObject *object,uint helperIndex )
{
	if (helperIndex >= 0 && helperIndex < m_helpers.size())
	{
		BindToHelper( helperIndex,object );
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CBuilding::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	XmlNodeRef node = CGroup::Export( levelPath,xmlNode );
	node->setTag( "Building" );
	node->setAttr( "Geometry",m_objectName );
	/*
	//for (int i = 0; i < m_objectNames.size(); i++)
	{
		XmlNodeRef cgf = node->newChild("Cgf");
		cgf->setAttr( "Name",m_objectName );
	}
	*/
	return node;
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::LoadBuilding( const CString &object,bool bForceReload )
{
	if (stricmp(object,m_objectName)==0 && ValidBuildingId(m_buildingId) && !bForceReload)
	{
		return;
	}
	m_objectName = object;

	if (!IsCreateGameObjects())
		return;

	ReleaseBuilding();
	m_sectorHidden.clear();

	if (m_objectName.IsEmpty())
		return;

	IndoorBaseInterface IndInterface;
	ISystem *pISystem = GetIEditor()->GetSystem();
	IndInterface.m_pSystem = pISystem;
	IndInterface.m_p3dEngine = pISystem->GetI3DEngine(); //(I3DEngine *)pISystem->GetIProcess();
	IndInterface.m_pLog = pISystem->GetILog();
	IndInterface.m_pRenderer = pISystem->GetIRenderer();
	IndInterface.m_pConsole = pISystem->GetIConsole();

	IIndoorBase *buildManager = GetBuildMgr();

	if (ValidBuildingId(m_buildingId))
		buildManager->DeleteBuilding( m_buildingId );

	m_buildingId = buildManager->CreateBuilding( IndInterface );
	if (!ValidBuildingId(m_buildingId))
		return;
	 
	int numSectors = buildManager->AddBuilding( m_objectName,m_buildingId );

	buildManager->SetBuildingPos( GetWorldPos(),m_buildingId );

	m_sectorHidden.resize(numSectors);

	// Save info about objects to helpers attachment.
	XmlNodeRef helpers = new CXmlNode( "Helpers" );
	CObjectArchive saveAr( GetObjectManager(),helpers,false );
	SerializeHelpers( saveAr );
	UnbindAllHelpers();
	m_helpers.clear();

	const char *helperName = 0;
	int i = 0;
	do {
		Vec3 pos;
		Matrix HelperMat;
		helperName = buildManager->GetHelper( i,m_buildingId,pos,&HelperMat );
		if (helperName)
		{
			ObjectHelper eh;
			eh.object = 0;
			eh.pos = pos;
//			eh.angles = rot.GetEulerAngles();

			assert(0); // not tested !!!
			AffineParts affineParts;
			affineParts.SpectralDecompose(HelperMat);
			eh.angles = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(affineParts.rot)));

			eh.name = CString(helperName).SpanExcluding( "/" );
			if (strchr(helperName,'/') != 0)
			{
				eh.className = CString(helperName).Mid( eh.name.GetLength()+1 );
			}
			m_helpers.push_back(eh);
		}
		i++;
	} while (helperName != NULL);

	// Load info about objects to helpers attachment.
	CObjectArchive loadAr( GetObjectManager(),helpers,true );
	SerializeHelpers( loadAr );

	GetBuildMgr()->EnableVisibility( m_portals );

	CalcBoundBox();
}
	
//////////////////////////////////////////////////////////////////////////
void CBuilding::CalcBoundBox()
{
	// Bounding box of building is an intersection of all objects bounding boxes
	m_bbox.Reset();

	if (ValidBuildingId(m_buildingId))
	{
		GetBuildMgr()->GetBBox( m_bbox.min,m_bbox.max,m_buildingId );
	}
	else
	{
		m_bbox.min=Vec3d(-1,-1,-1);
		m_bbox.max=Vec3d(1,1,1);
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::SpawnEntities()
{
	CEntityClassDesc entClassDesc;

	// Spawn entities using Class Name given in helper.
	for (int i = 0; i < m_helpers.size(); i++)
	{
		ObjectHelper &eh = m_helpers[i];
		if (eh.object == 0)
		{
			//
			CEntity *entity = (CEntity*)GetIEditor()->NewObject( entClassDesc.ClassName(),eh.className );
			if (!entity)
			{
				CLogFile::FormatLine( "Building: Entity Spawn failed, Unknown class: %s",(const char*)eh.className );
				continue;
			}
			entity->SetPos( GetWorldPos()+eh.pos );
			entity->SetAngles( eh.angles );
			eh.object = entity;
			AttachChild( entity );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::HideSector( int index,bool bHide )
{
	if (ValidBuildingId(m_buildingId))
	{
		if (index >= 0 && index < GetNumSectors())
		{
			m_sectorHidden[index] = bHide;
			GetBuildMgr()->HideSector( bHide,m_buildingId,index );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CBuilding::IsSectorHidden( int index ) const
{
	if (index >= 0 && index < GetNumSectors())
		return m_sectorHidden[index];
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::SetWireframe( bool bEnable )
{
	StoreUndo( "Set Wireframe" );
	m_wireFrame = bEnable;

	GetBuildMgr()->EnableWireframe( bEnable );
}
	
//////////////////////////////////////////////////////////////////////////
bool CBuilding::IsWireframe() const
{
	return m_wireFrame;
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::BindToHelper( int helperIndex,CBaseObject *obj )
{
	assert( helperIndex >= 0 && helperIndex < m_helpers.size() );

	if (obj->IsChildOf(this) || obj == m_helpers[helperIndex].object)
		return;

	StoreUndo( "Bind Helper" );

	obj->AddEventListener( functor(*this,OnHelperEvent) );
	m_helpers[helperIndex].object = obj;
	obj->SetPos( GetWorldPos() + m_helpers[helperIndex].pos );
	//obj->SetAngles( m_helpers[helperIndex].angles );
	
	AttachChild( obj );
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::UnbindHelper( int helperIndex )
{
	assert( helperIndex >= 0 && helperIndex < m_helpers.size() );

	if (m_helpers[helperIndex].object)
	{
		StoreUndo( "Unbind Helper" );

		m_helpers[helperIndex].object->RemoveEventListener( functor(*this,&CBuilding::OnHelperEvent) );

		RemoveChild( m_helpers[helperIndex].object );
		m_helpers[helperIndex].object = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::UnbindAllHelpers()
{
	for (int i = 0; i < m_helpers.size(); i++)
	{
		if (m_helpers[i].object)
		{
			UnbindHelper(i);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::OnHelperEvent( CBaseObject *object,int event )
{
	if (event == CBaseObject::ON_DELETE)
	{
		// Find helper to unbind.
		for (int i = 0; i < m_helpers.size(); i++)
		{
			if (m_helpers[i].object == object)
			{
				UnbindHelper(i);
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IIndoorBase* CBuilding::GetBuildMgr()
{
	return GetIEditor()->Get3DEngine()->GetBuildingManager();
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::UpdateVisibility( bool visible )
{
	CGroup::UpdateVisibility(visible);
	if (ValidBuildingId(m_buildingId))
		GetBuildMgr()->HideBuilding( !visible,m_buildingId );
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::InvalidateTM()
{
	CGroup::InvalidateTM();
	// Move window to new world position.
	if (ValidBuildingId(m_buildingId))
	{
		// Move indoor building.
		GetBuildMgr()->SetBuildingPos( GetWorldPos(),m_buildingId );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBuilding::SetPortals( bool enable )
{
	m_portals = enable;
	if (ValidBuildingId(m_buildingId))
		GetBuildMgr()->EnableVisibility(enable);
}