////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   StatObj.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: StaticObject implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Entity.h"
#include "EntityPanel.h"
#include "PanelTreeBrowser.h"
#include "PropertiesPanel.h"
#include "Viewport.h"
#include "Settings.h"
#include "LineGizmo.h"
#include "Settings.h"

#include <I3DEngine.h>
#include <IAgent.h>
#include <IMovieSystem.h>
#include <IEntitySystem.h>
#include <EntityDesc.h>

#include "EntityPrototype.h"
#include "Material\MaterialManager.h"

#include "BrushObject.h"

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CEntity,CBaseObject)
IMPLEMENT_DYNCREATE(CSimpleEntity,CEntity)

int CEntity::m_rollupId = 0;
CEntityPanel* CEntity::m_panel = 0;
float CEntity::m_helperScale = 1;

namespace
{
	int s_propertiesPanelIndex = 0;
	CPropertiesPanel* s_propertiesPanel = 0;

	int s_propertiesPanelIndex2 = 0;
	CPropertiesPanel* s_propertiesPanel2 = 0;

	// Prevent OnPropertyChange to be executed when loading many properties at one time.
	static bool s_ignorePropertiesUpdate = false;

	CPanelTreeBrowser* s_treePanel = NULL;
	int s_treePanelId = 0;

	struct AutoReleaseAll
	{
		AutoReleaseAll() {};
		~AutoReleaseAll()
		{
			delete s_propertiesPanel;
			delete s_propertiesPanel2;
			delete s_treePanel;
		}
	} s_autoReleaseAll;
};

//////////////////////////////////////////////////////////////////////////
CEntity::CEntity()
{
	m_entity = 0;
	m_loadFailed = false;

	m_script = 0;

	m_visualObject = 0;

	m_box.min.Set(0,0,0);
	m_box.max.Set(0,0,0);

	m_proximityRadius = 0;
	m_innerRadius = 0;
	m_outerRadius = 0;

	m_animNode = 0;
	m_displayBBox = true;
	m_entityId = 0;
	m_visible = true;
	m_bCalcPhysics = true;
	//m_staticEntity = false;

	m_bEntityXfromValid = false;
	ZeroStruct( m_materialGUID );

	SetColor( RGB(255,255,0) );
	
	// Init Variables.
	mv_castShadows = false;
	mv_selfShadowing = false;
	mv_recvShadowMaps = true;
	mv_castShadowMaps = true;
	mv_castLightmap = false;
	mv_recvLightmap = false;
	mv_hiddenInGame = false;
	mv_ratioLOD = 100;
	mv_ratioViewDist = 100;
	mv_UpdateVisLevel = eUT_Always;
	mv_notOnLowSpec = false;

	AddVariable( mv_castShadows,"CastShadows",_T("CastShadowVolume"),functor(*this,&CEntity::OnRenderFlagsChange) );
	AddVariable( mv_selfShadowing,"SelfShadowing",functor(*this,&CEntity::OnRenderFlagsChange) );
	AddVariable( mv_castShadowMaps,"CastShadowMaps",functor(*this,&CEntity::OnRenderFlagsChange) );
	AddVariable( mv_recvShadowMaps,"RecvShadowMaps",functor(*this,&CEntity::OnRenderFlagsChange) );
	AddVariable( mv_castLightmap,"PreCalcShadows",_T("CastLightmap"),functor(*this,&CEntity::OnRenderFlagsChange) );
	AddVariable( mv_recvLightmap,"ReceiveLightmap",functor(*this,&CEntity::OnRenderFlagsChange) );
	AddVariable( mv_ratioLOD,"LodRatio",functor(*this,&CEntity::OnRenderFlagsChange) );
	AddVariable( mv_ratioViewDist,"ViewDistRatio",functor(*this,&CEntity::OnRenderFlagsChange) );
	AddVariable( mv_notOnLowSpec,"SkipOnLowSpec" );
	AddVariable( mv_hiddenInGame,"HiddenInGame" );
//  AddVariable( mv_UpdateVisLevel, "UpdateVisLevel", functor(*this,&CEntity::OnRenderFlagsChange) ); // waiting comboboxes support
	mv_ratioLOD.SetLimits( 0,255 );
	mv_ratioViewDist.SetLimits( 0,255 );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::Done()
{
	DeleteEntity();
	UnloadScript();

	if (m_trackGizmo)
	{
		RemoveGizmo( m_trackGizmo );
		m_trackGizmo = 0;
	}
	if (m_animNode)
	{
		m_animNode->UnregisterCallback(this);
		GetIEditor()->GetMovieSystem()->RemoveNode( m_animNode );
		m_animNode = 0;
	}
	ReleaseEventTargets();

	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	CBaseObject::Init( ie,prev,file );

	if (IsCreateGameObjects())
	{
		m_animNode = CreateAnimNode();
		if (m_animNode)
		{
			m_animNode->SetName( GetName() );
			m_animNode->RegisterCallback(this);
		}

		m_trackGizmo = new CTrackGizmo;
		m_trackGizmo->SetAnimNode( m_animNode );
		AddGizmo( m_trackGizmo );
	}
	
	if (prev)
	{
		CEntity *pe = (CEntity*)prev;
		m_pMaterial = pe->m_pMaterial;
		m_materialGUID = pe->m_materialGUID;
		// Clone Properties.
		if (pe->m_properties)
		{
			m_properties = CloneProperties(pe->m_properties);
		}
		if (pe->m_properties2)
		{
			m_properties2 = CloneProperties(pe->m_properties2);
		}
		// When cloning entity, do not get properties from script.
		LoadScript( pe->GetEntityClass(),false,false );
		SpawnEntity();
		UpdatePropertyPanel();
	}
	else if (!file.IsEmpty())
	{
		SetUniqName( file );
		m_entityClass = file;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
IAnimNode* CEntity::CreateAnimNode()
{
	//GUID guid = GetId();
	//int nodeId = Crc32Gen::GetCRC32( (const char*)&guid,sizeof(guid),0xffffffff );
	//int entityId = Crc32Gen::GetCRC32( GetId();
	int nodeId = GetId().Data1;
	return GetIEditor()->GetMovieSystem()->CreateNode( ANODE_ENTITY,nodeId );
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::IsSameClass( CBaseObject *obj )
{
	if (GetClassDesc() == obj->GetClassDesc())
	{
		CEntity *ent = (CEntity*)obj;
		return GetScript() == ent->GetScript();
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::ConvertFromObject( CBaseObject *object )
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetLookAt( CBaseObject *target )
{
	CBaseObject::SetLookAt(target);
	if (m_animNode)
	{
		IAnimNode *trgAnimNode = 0;
		if (target)
			trgAnimNode = target->GetAnimNode();
		m_animNode->SetTarget( trgAnimNode );
	}
}

//////////////////////////////////////////////////////////////////////////
IPhysicalEntity* CEntity::GetCollisionEntity() const
{
	// Returns physical object of entity.
	if (m_entity)
		return m_entity->GetPhysics();
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::GetBoundBox( BBox &box )
{
	box = m_box;
	box.Transform( GetWorldTM() );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::GetLocalBounds( BBox &box )
{
	box = m_box;
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::HitTestEntity( HitContext &hc,bool &bHavePhysics )
{
	bHavePhysics = true;
	IPhysicalWorld *pPhysWorld = GetIEditor()->GetSystem()->GetIPhysicalWorld();
	// Test 3D viewport.
	IPhysicalEntity *physic = 0;

	ICryCharInstance *pCharacter = m_entity->GetCharInterface()->GetCharacter(0);
	if (pCharacter)
	{	
		physic = pCharacter->GetCharacterPhysics();
		if (physic)
		{
			int type = physic->GetType();
			if (type == PE_NONE || type == PE_PARTICLE || type == PE_ROPE || type == PE_SOFT)
				physic = 0;
			else if (physic->GetStatus( &pe_status_nparts() ) == 0)
				physic = 0;
		}
		if (physic)
		{
			ray_hit hit;
			int col = pPhysWorld->RayTraceEntity( physic,hc.raySrc,hc.rayDir*10000.0f,&hit );
			if (col <= 0)
				return false;
			hc.dist = hit.dist;
			return true;
		}
	}

	physic = m_entity->GetPhysics();
	if (physic)
	{
		int type = physic->GetType();
		if (type == PE_NONE || type == PE_PARTICLE || type == PE_ROPE || type == PE_SOFT)
			physic = 0;
		else if (physic->GetStatus( &pe_status_nparts() ) == 0)
			physic = 0;
	}
	// Now if box intersected try real geometry ray test.
	if (physic)
	{
		ray_hit hit;
		int col = pPhysWorld->RayTraceEntity( physic,hc.raySrc,hc.rayDir*10000.0f,&hit );
		if (col <= 0)
			return false;
		hc.dist = hit.dist;
		return true;
	}
	else
	{
		bHavePhysics = false;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::HitTest( HitContext &hc )
{
	if (!hc.b2DViewport)
	{
		// Test 3D viewport.
		if (m_entity)
		{
			bool bHavePhysics = false;
			if (HitTestEntity( hc,bHavePhysics ))
				return true;
			if (bHavePhysics)
			{
				return false;
			}
		}
		if (m_visualObject)
		{
			Matrix44 tm = GetWorldTM();
			float sz = m_helperScale * gSettings.gizmo.helpersScale;
			tm.ScaleMatRow( Vec3(sz,sz,sz) );
			primitives::ray aray; aray.origin = hc.raySrc; aray.dir = hc.rayDir*10000.0f;

			IGeomManager *pGeomMgr = GetIEditor()->GetSystem()->GetIPhysicalWorld()->GetGeomManager();
			IGeometry *pRay = pGeomMgr->CreatePrimitive(primitives::ray::type, &aray);
			geom_world_data gwd;
			gwd.R.extract_from4x4T( tm, gwd.offset,gwd.scale);
			geom_contact *pcontacts = 0;
			int col = (m_visualObject->GetPhysGeom() && m_visualObject->GetPhysGeom()->pGeom) ? m_visualObject->GetPhysGeom()->pGeom->Intersect(pRay, &gwd,0, 0, pcontacts) : 0;
			pGeomMgr->DestroyGeometry(pRay);
			if (col > 0)
			{
				if (pcontacts)
					hc.dist = pcontacts[col-1].t;
				return true;
			}
		}
	}
	/*
	else if (m_entity)
	{
	float dist = FLT_MAX;
	bool bHaveHit = false;
	CEntityObject entobj;
	IPhysicalWorld *physWorld = GetIEditor()->GetSystem()->GetIPhysicalWorld();
	int numobj = m_entity->GetNumObjects();
	if (numobj == 0)
	{
	hc.weakHit = true;
	return true;
	}
	vector origin = hc.raySrc;
	vector dir = hc.rayDir*10000.0f;
	Matrix tm;
	GetMatrix( tm );
	tm.Transpose();
	for (int i = 0; i < numobj; i++)
	{
	m_entity->GetEntityObject(i,entobj);
	if (entobj.object && entobj.object->GetPhysGeom())
	{
	ray_hit hit;
	int col = physWorld->RayTraceGeometry( entobj.object->GetPhysGeom(),(float*)tm.m_values,origin,dir,&hit );
	if (col > 0)
	{
	dist = __min(dist,hit.dist);
	bHaveHit = true;
	}
	}
	}
	if (bHaveHit)
	{
	hc.dist = dist;
	return true;
	}
	else
	{
	return false;
	}
	}
	*/

	float hitEpsilon = hc.view->GetScreenScaleFactor( GetWorldPos() ) * 0.01f;
	float hitDist;

	float fScale = GetScale().x;
	BBox boxScaled;
	boxScaled.min = m_box.min*fScale;
	boxScaled.max = m_box.max*fScale;

	Matrix44 invertWTM = GetWorldTM();
	invertWTM.Invert44();
	//Vec3 xformedRaySrc = invertWTM*hc.raySrc;
	//Vec3 xformedRayDir = GetNormalized( invertWTM*hc.rayDir );
	
	
	Vec3 xformedRaySrc = invertWTM.TransformPointOLD(hc.raySrc);
	Vec3 xformedRayDir = invertWTM.TransformVectorOLD(hc.rayDir);
	xformedRayDir.Normalize();

	/*
	if (m_staticEntity)
	{
		// Collided with bbox of entity.
		hc.dist = GetDistance(xformedRaySrc,pntContact);
		return true;
	}
	else
	*/
	{
		Vec3 intPnt;
		// Check intersection with bbox edges.
		if (boxScaled.RayEdgeIntersection( xformedRaySrc,xformedRayDir,hitEpsilon,hitDist,intPnt ))
		{
			hc.dist = GetDistance(xformedRaySrc,intPnt);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
int CEntity::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseMove || event == eMouseLDown)
	{
		Vec3 pos;
		// Rise Entity above ground on Bounding box ammount.
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
				pos.z = GetIEditor()->GetTerrainElevation(pos.x,pos.y);
				pos.z = pos.z - m_box.min.z;
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
bool CEntity::CreateGameObject()
{
	if (!m_script)
	{
		if (!m_entityClass.IsEmpty())
			LoadScript( m_entityClass,true );
	}
	if (!m_entity)
	{
		if (!m_entityClass.IsEmpty())
			SpawnEntity();
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::LoadScript( const CString &entityClass,bool bForceReload,bool bGetScriptProperties,XmlNodeRef xmlProperties,XmlNodeRef xmlProperties2 )
{
	if (entityClass == m_entityClass && m_script != 0 && !bForceReload)
		return true;
	m_entityClass = entityClass;
	m_loadFailed = false;

	UnloadScript();

	if (!IsCreateGameObjects())
		return false;

	//HACK
	// Special case to ignore static entities.
	if (entityClass == "StaticEntity")
	{
		OnLoadFailed();
		return false;
	}

	m_script = CEntityScriptRegistry::Instance()->Find( m_entityClass );
	if (!m_script)
	{
		OnLoadFailed();
		return false;
	}

	// Load script if its not loaded yet.
	if (!m_script->IsValid())
	{
		if (!m_script->Load())
		{
			OnLoadFailed();
			return false;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Make visual editor object for this entity.
	//////////////////////////////////////////////////////////////////////////
	if (!m_script->GetVisualObject().IsEmpty())
	{
		//m_entity->LoadObject( 0,m_script->GetVisualObject(),1 );
		//m_entity->DrawObject( 0,ETY_DRAW_NORMAL );
		m_visualObject = GetIEditor()->Get3DEngine()->MakeObject( m_script->GetVisualObject() );
		if (m_visualObject)
			m_visualObject->SetShaderTemplate( EFT_USER_FIRST+1,"s_ObjectColor",NULL );
	}

	bool bUpdateUI = false;
	// Create Entity properties from Script properties..
	if (bGetScriptProperties && m_prototype == NULL && m_script->GetProperties() != NULL)
	{
		bUpdateUI = true;
		CVarBlockPtr oldProperties = m_properties;
		m_properties = CloneProperties( m_script->GetProperties() );

		if (xmlProperties)
		{
			s_ignorePropertiesUpdate = true;
			m_properties->Serialize( xmlProperties,true );
			s_ignorePropertiesUpdate = false;
		}
		else if (oldProperties)
		{
			// If we had propertied before copy thier values to new script.
			s_ignorePropertiesUpdate = true;
			m_properties->CopyValuesByName(oldProperties);
			s_ignorePropertiesUpdate = false;
		}
	}

	// Create Entity properties from Script properties..
	if (bGetScriptProperties && m_script->GetProperties2() != NULL)
	{
		bUpdateUI = true;
		CVarBlockPtr oldProperties = m_properties2;
		m_properties2 = CloneProperties( m_script->GetProperties2() );

		if (xmlProperties2)
		{
			s_ignorePropertiesUpdate = true;
			m_properties2->Serialize( xmlProperties2,true );
			s_ignorePropertiesUpdate = false;
		}
		else if (oldProperties)
		{
			// If we had propertied before copy thier values to new script.
			s_ignorePropertiesUpdate = true;
			m_properties2->CopyValuesByName(oldProperties);
			s_ignorePropertiesUpdate = false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SpawnEntity()
{
	if (!m_script)
		return;
	
	// Do not spawn second time.
	if (m_entity)
		return;

	m_loadFailed = false;

	EntityClassId ClassId = m_script->GetClsId();

	//CLogFile::FormatLine( "Loading Entity %s (%s)",(const char*)entityClass,(const char*)GetName() );

	if (m_entityId != 0)
	{
		if (GetIEditor()->GetSystem()->GetIEntitySystem()->IsIDUsed( m_entityId ))
			m_entityId = 0;
	}

	CEntityDesc ed;
	ed.ClassId = ClassId;
	ed.name = (const char*)GetName();
	ed.pos = GetWorldPos();
	ed.angles = GetAngles();
	ed.id = m_entityId;
	if (ed.id == 0)
		ed.id = -1; // Tells to Entity system to genrate new static id.
	
	IEntitySystem *pEntitySystem = GetIEditor()->GetSystem()->GetIEntitySystem();
	// Spawn Entity but not initialize it.
	m_entity = pEntitySystem->SpawnEntity( ed,false );
	if (m_entity)
	{
		m_entityId = m_entity->GetId();

		// Bind to parent.
		BindToParent();
		BindIEntityChilds();

		m_entity->Hide( !m_visible );

		//m_script->SetCurrentProperties( this );
		m_script->SetEventsTable( this );

		// Mark this entity non destroyable.
		m_entity->SetDestroyable(false);

		//////////////////////////////////////////////////////////////////////////
		// If have material, assign it to the entity.
		if (m_pMaterial)
			m_pMaterial->AssignToEntity( m_entity );
		
		// Force transformation on entity.
		XFormGameEntity();

		if (m_properties != NULL)
		{
			m_script->SetProperties( m_entity,m_properties,false );
		}
		if (m_properties2 != NULL)
		{
			m_script->SetProperties2( m_entity,m_properties2,false );
		}

		//////////////////////////////////////////////////////////////////////////
		// Now initialize entity.
		//////////////////////////////////////////////////////////////////////////
		if (!pEntitySystem->InitEntity( m_entity,ed ))
		{
			m_entity = 0;
			OnLoadFailed();
			return;
		}

		// Update render flags of entity (Must be after InitEntity).
		OnRenderFlagsChange(0);

		if (!m_physicsState.IsEmpty())
		{
			m_entity->SetPhysicsState( m_physicsState );
		}

		//////////////////////////////////////////////////////////////////////////
		// Check if needs to display bbox for this entity.
		//////////////////////////////////////////////////////////////////////////
		m_bCalcPhysics = true;
		if (m_entity->GetPhysics() != 0)
		{
			m_displayBBox = false;
			if (m_entity->GetPhysics()->GetType() == PE_SOFT)
			{
				m_bCalcPhysics = false;
				//! Ignore entity being updated from physics.
				m_entity->SetFlags(ETY_FLAG_IGNORE_PHYSICS_UPDATE);
			}
		}
		else
			m_displayBBox = true;

		//////////////////////////////////////////////////////////////////////////
		// Calculate entity bounding box.
		CalcBBox();
		
		//////////////////////////////////////////////////////////////////////////
		// Assign entity pointer to animation node.
		if (m_animNode)
			m_animNode->SetEntity(m_entity);
	}
	else
	{
		OnLoadFailed();
	}

	//?
	UpdatePropertyPanel();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::DeleteEntity()
{
	if (m_entity)
	{
		UnbindIEntity();
		m_entity->SetDestroyable(true);
		GetIEditor()->GetSystem()->GetIEntitySystem()->RemoveEntity( m_entity->GetId(),true );
	}
	m_entity = 0;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UnloadScript()
{
	if (m_entity)
	{
		DeleteEntity();
	}
	if (m_visualObject)
		GetIEditor()->Get3DEngine()->ReleaseObject(m_visualObject);
	m_visualObject = 0;
	m_script = 0;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::XFormGameEntity()
{
	if (!m_entity)
		return;

	m_entity->SetPos( GetPos(),false );
	m_entity->SetAngles( GetAngles() );
	m_entity->SetScale( GetScale().x );
	// Make sure this entity is succesfully registered in correct sectors after loading.
	m_entity->ForceRegisterInSectors();

		/*
		const Matrix &tm = GetWorldTM();
		//m_entity->SetPos( Vec3(tm[3][0],tm[3][1],tm[3][2]) );
		Quat rotate(tm);
		//m_entity->SetAngles( rotate.GetEulerAngles()*180.0f/PI );
		m_bEntityXfromValid = true;
		*/
}

//////////////////////////////////////////////////////////////////////////
void CEntity::CalcBBox()
{
	if (m_entity)
	{
		// Get Local bounding box of entity.
		m_entity->GetLocalBBox( m_box.min,m_box.max );

		if (m_box.IsEmpty())
			m_displayBBox = true;
		
		if (m_visualObject)
		{
			Vec3 minp = m_visualObject->GetBoxMin()*m_helperScale*gSettings.gizmo.helpersScale;
			Vec3 maxp = m_visualObject->GetBoxMax()*m_helperScale*gSettings.gizmo.helpersScale;
			m_box.Add( minp );
			m_box.Add( maxp );
		}
		float minSize = 0.1f;
		if (fabs(m_box.max.x-m_box.min.x)+fabs(m_box.max.y-m_box.min.y)+fabs(m_box.max.z-m_box.min.z) < minSize)
		{
			m_box.min = -Vec3( minSize,minSize,minSize );
			m_box.max = Vec3( minSize,minSize,minSize );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetName( const CString &name )
{
	if (name == GetName())
		return;

	CBaseObject::SetName( name );
	if (m_entity)
		m_entity->SetName( (const char*)GetName() );
	if (m_animNode)
	{
		m_animNode->SetName( name );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetPos( const Vec3d &pos )
{
	if (IsVectorsEqual(GetPos(),pos))
		return;

	CBaseObject::SetPos( pos );

	if (m_entity)
	{
		//m_entity->SetPos( GetWorldPos() );
		m_entity->SetPos( GetPos(),false );
		m_entity->ForceRegisterInSectors();
	}
	if (m_animNode)
	{
		m_animNode->SetPos( GetIEditor()->GetAnimation()->GetTime(),pos );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetAngles( const Vec3d &angles )
{
	if (IsVectorsEqual(GetAngles(),angles))
		return;

	CBaseObject::SetAngles( angles );

	if (m_entity)
	{
		m_entity->SetAngles( GetAngles() );
	}
	
	if (m_animNode)
	{
		m_rotate.SetRotationXYZ( angles*gf_PI/180.0f );
		m_animNode->SetRotate( GetIEditor()->GetAnimation()->GetTime(),m_rotate );
	}
}
	
void CEntity::SetScale( const Vec3d &scale )
{
	if (IsVectorsEqual(GetScale(),scale))
		return;

	CBaseObject::SetScale( scale );
	
	if (m_entity)
	{
		m_entity->SetScale( GetScale().x );
		CalcBBox();
	}
	if (m_animNode)
	{
		m_animNode->SetScale( GetIEditor()->GetAnimation()->GetTime(),scale );
	}
	/*
	if (m_entity)
		m_entity->SetScale(scale);
	*/
//	CBaseObject::SetScale( scale );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::BeginEditParams( IEditor *ie,int flags )
{
	CBaseObject::BeginEditParams( ie,flags );

	if (m_properties2 != NULL)
	{
		if (!s_propertiesPanel2)
			s_propertiesPanel2 = new CPropertiesPanel( AfxGetMainWnd() );
		else
			s_propertiesPanel2->DeleteVars();
		s_propertiesPanel2->AddVars( m_properties2 );
		if (!s_propertiesPanelIndex2)
			s_propertiesPanelIndex2 = ie->AddRollUpPage( ROLLUP_OBJECTS,CString(GetTypeName()) + " Properties2",s_propertiesPanel2 );
	}

	if (!m_prototype)
	{
		if (m_properties != NULL)
		{
			if (!s_propertiesPanel)
				s_propertiesPanel = new CPropertiesPanel( AfxGetMainWnd() );
			else
				s_propertiesPanel->DeleteVars();
			s_propertiesPanel->AddVars( m_properties );
			if (!s_propertiesPanelIndex)
				s_propertiesPanelIndex = ie->AddRollUpPage( ROLLUP_OBJECTS,CString(GetTypeName()) + " Properties",s_propertiesPanel );
		}
	}

	if (!m_panel && m_entity)
	{
		m_panel = new CEntityPanel(AfxGetMainWnd());
		m_panel->Create( CEntityPanel::IDD,AfxGetMainWnd() );
		m_rollupId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,m_entityClass,m_panel );
	}

	if (m_panel)
		m_panel->SetEntity( this );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::EndEditParams( IEditor *ie )
{
	if (m_rollupId != 0)
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,m_rollupId );
	m_rollupId = 0;
	m_panel = 0;

	if (s_propertiesPanelIndex != 0)
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,s_propertiesPanelIndex );
	s_propertiesPanelIndex = 0;
	s_propertiesPanel = 0;

	if (s_propertiesPanelIndex2 != 0)
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,s_propertiesPanelIndex2 );
	s_propertiesPanelIndex2 = 0;
	s_propertiesPanel2 = 0;

	CBaseObject::EndEditParams( GetIEditor() );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdatePropertyPanel()
{
	// If user interface opened reload properties.
	if (s_propertiesPanel && m_properties != 0)
	{
		s_propertiesPanel->DeleteVars();
		s_propertiesPanel->AddVars( m_properties );
	}

	// If user interface opened reload properties.
	if (s_propertiesPanel2 && m_properties2 != 0)
	{
		s_propertiesPanel2->DeleteVars();
		s_propertiesPanel2->AddVars( m_properties2 );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::BeginEditMultiSelParams( bool bAllOfSameType )
{
	CBaseObject::BeginEditMultiSelParams(bAllOfSameType);

	if (!bAllOfSameType)
		return;

	if (m_properties2 != NULL)
	{
		if (!s_propertiesPanel2)
			s_propertiesPanel2 = new CPropertiesPanel( AfxGetMainWnd() );
		else
			s_propertiesPanel2->DeleteVars();

		// Add all selected objects.
		CSelectionGroup *grp = GetIEditor()->GetSelection();
		for (int i = 0; i < grp->GetCount(); i++)
		{
			CEntity *ent = (CEntity*)grp->GetObject(i);
			if (ent->m_properties2)
				s_propertiesPanel2->AddVars( ent->m_properties2 );
		}
		if (!s_propertiesPanelIndex2)
			s_propertiesPanelIndex2 = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,CString(GetTypeName()) + " Properties",s_propertiesPanel2 );
	}

	if (m_properties != NULL && m_prototype == NULL)
	{
		if (!s_propertiesPanel)
			s_propertiesPanel = new CPropertiesPanel( AfxGetMainWnd() );
		else
			s_propertiesPanel->DeleteVars();

		// Add all selected objects.
		CSelectionGroup *grp = GetIEditor()->GetSelection();
		for (int i = 0; i < grp->GetCount(); i++)
		{
			CEntity *ent = (CEntity*)grp->GetObject(i);
			CVarBlock *vb = ent->m_properties;
			if (vb)
				s_propertiesPanel->AddVars( vb );
		}
		if (!s_propertiesPanelIndex)
			s_propertiesPanelIndex = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,CString(GetTypeName()) + " Properties",s_propertiesPanel );
	}
}
	
//////////////////////////////////////////////////////////////////////////
void CEntity::EndEditMultiSelParams()
{
	if (s_propertiesPanelIndex != 0)
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,s_propertiesPanelIndex );
	s_propertiesPanelIndex = 0;
	s_propertiesPanel = 0;

	if (s_propertiesPanelIndex2 != 0)
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,s_propertiesPanelIndex2 );
	s_propertiesPanelIndex2 = 0;
	s_propertiesPanel2 = 0;

	CBaseObject::EndEditMultiSelParams();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetSelected( bool bSelect )
{
	CBaseObject::SetSelected( bSelect );

	if (m_entity)
	{
		int flags = m_entity->GetRndFlags();
		if (bSelect)
			flags |= ERF_SELECTED;
		else
			flags &= ~ERF_SELECTED;
		m_entity->SetRndFlags( flags );
	}
	if (m_animNode)
		m_animNode->SetFlags( (bSelect) ? (m_animNode->GetFlags()|ANODE_FLAG_SELECTED) : (m_animNode->GetFlags()&(~ANODE_FLAG_SELECTED)) );
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnPropertyChange( IVariable *var )
{
	if (s_ignorePropertiesUpdate)
		return;

	if (m_script != 0 && m_entity != 0)
	{
		if (m_properties)
			m_script->SetProperties( m_entity,m_properties,true );
		if (m_properties2)
			m_script->SetProperties2( m_entity,m_properties2,true );
		// After change of properties bounding box of entity may change.
		CalcBBox();
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::Display( DisplayContext &dc )
{
	if (!m_entity)
		return;

	//XFormEntity();
	
	Matrix44 wtm = GetWorldTM();

	COLORREF col = GetColor();
	if (IsFrozen())
		col = dc.GetFreezeColor();

	//Vec3 scale = GetScale();

	dc.PushMatrix( wtm );

	if (IsSelected())
	{
		dc.SetSelectedColor( 0.5f );
		//dc.renderer->Draw3dBBox( GetPos()-Vec3(m_radius,m_radius,m_radius),GetPos()+Vec3(m_radius,m_radius,m_radius));
		dc.DrawWireBox( m_box.min,m_box.max );
	}
	else
	{
		if (m_displayBBox || (dc.flags & DISPLAY_2D))
		{
			dc.SetColor( col,0.3f );
			dc.DrawWireBox( m_box.min,m_box.max );
		}
	}

	/*
	// Only display solid BBox if visual object is associated with the entity.
	if (m_displayBBox && m_visualObject)
	{

		dc.renderer->EnableDepthWrites(false);
		dc.SetColor(col,0.05f);
		dc.DrawSolidBox( m_box.min,m_box.max );
		dc.renderer->EnableDepthWrites(true);
	}
	*/

	/*
	if (m_entity && m_proximityRadius >= 0)
	{
		float r = m_proximityRadius;
		dc.SetColor( 1,1,0,1 );
		dc.DrawWireBox(  GetPos()-Vec3(r,r,r),GetPos()+Vec3(r,r,r) );
	}
	*/

	// Draw radiuses if present and object selected.
	if (gSettings.viewports.bAlwaysShowRadiuses || IsSelected())
	{
		float fScale = GetScale().x; // Ignore matrix scale.
		if (fScale == 0) fScale = 1;
		if (m_innerRadius > 0)
		{
			dc.SetColor( 0,1,0,0.3f );
			dc.DrawWireSphere( Vec3(0,0,0),m_innerRadius/fScale );
		}
		if (m_outerRadius > 0)
		{
			dc.SetColor( 1,1,0,0.8f );
			dc.DrawWireSphere( Vec3(0,0,0),m_outerRadius/fScale );
		}
	}

	dc.PopMatrix();

	// Entities themself are rendered by 3DEngine.

	if (m_visualObject)
	{
		/*
		float fScale = dc.view->GetScreenScaleFactor(wtm.GetTranslation()) * 0.04f;
		wtm[0][0] *= fScale; wtm[0][1] *= fScale; wtm[0][2] *= fScale;
		wtm[1][0] *= fScale; wtm[1][1] *= fScale; wtm[1][2] *= fScale;
		wtm[2][0] *= fScale; wtm[2][1] *= fScale; wtm[2][2] *= fScale;
		*/
		Matrix44 tm(wtm);
		float sz = m_helperScale*gSettings.gizmo.helpersScale;
		tm.ScaleMatRow( Vec3(sz,sz,sz) );

		SRendParams rp;
		if (IsSelected())
			rp.vColor = Rgb2Vec(dc.GetSelectedColor());
		else
			rp.vColor = Rgb2Vec(col);
    rp.dwFObjFlags |= FOB_TRANS_MASK;
		rp.fAlpha = 1;
		rp.nDLightMask = GetIEditor()->Get3DEngine()->GetLightMaskFromPosition(wtm.GetTranslationOLD(),1.f) & 0xFFFF;
		rp.pMatrix = &tm;
		m_visualObject->Render( rp,Vec3(zero),0 );
	}

	if (IsSelected())
	{
		if (m_entity)
		{
			IAIObject *pAIObj = m_entity->GetAI();
			if (pAIObj)
				DrawAIInfo( dc,pAIObj );
		}
	}

	/*
	if ((dc.flags & DISPLAY_LINKS) && !m_eventTargets.empty())
	{
		DrawTargets(dc);
	}
	*/

	DrawDefault(dc,col);
}

//////////////////////////////////////////////////////////////////////////
void CEntity::DrawAIInfo( DisplayContext &dc,IAIObject *aiObj )
{
	assert( aiObj );
	IPuppet *pPuppet;
	if (aiObj->CanBeConvertedTo( AIOBJECT_PUPPET,(void**)&pPuppet))
	{
		AgentParameters &ap =	pPuppet->GetPuppetParameters();

		// Draw ranges.
		bool bTerrainCircle = false;
		Vec3 wp = GetWorldPos();
		float z = GetIEditor()->GetTerrainElevation( wp.x,wp.y );
		if (fabs(wp.z-z) < 5)
			bTerrainCircle = true;

		dc.SetColor( RGB(0,255,0) );
		if (bTerrainCircle)
			dc.DrawTerrainCircle( wp,ap.m_fSoundRange,0.2f );
		else
			dc.DrawCircle( wp,ap.m_fSoundRange );

		dc.SetColor( RGB(255,255,0) );
		if (bTerrainCircle)
			dc.DrawTerrainCircle( wp,ap.m_fCommRange,0.2f );
		else
			dc.DrawCircle( wp,ap.m_fCommRange );

		dc.SetColor( RGB(255,0,0) );
		if (bTerrainCircle)
			dc.DrawTerrainCircle( wp,ap.m_fSightRange,0.2f );
		else
			dc.DrawCircle( wp,ap.m_fSightRange );

		dc.SetColor( RGB(255/2,0,0) );
		if (bTerrainCircle)
			dc.DrawTerrainCircle( wp,ap.m_fSightRange/2,0.2f );
		else
			dc.DrawCircle( wp,ap.m_fSightRange/2 );

		dc.SetColor( RGB(0,0,255) );
		if (bTerrainCircle)
			dc.DrawTerrainCircle( wp,ap.m_fAttackRange,0.2f );
		else
			dc.DrawCircle( wp,ap.m_fAttackRange );

		//dc.SetColor( 0,1,0,0.3f );		
		//dc.DrawWireSphere( Vec3(0,0,0),m_innerRadius );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::DrawTargets( DisplayContext &dc )
{
	/*
	BBox box;
	for (int i = 0; i < m_eventTargets.size(); i++)
	{
		CBaseObject *target = m_eventTargets[i].target;
		if (!target)
			return;

		dc.SetColor( 0.8f,0.4f,0.4f,1 );
		GetBoundBox( box );
		Vec3 p1 = 0.5f*Vec3(box.max+box.min);
		target->GetBoundBox( box );
		Vec3 p2 = 0.5f*Vec3(box.max+box.min);

		dc.DrawLine( p1,p2 );

		Vec3 p3 = 0.5f*(p2+p1);

		if (!(dc.flags & DISPLAY_HIDENAMES))
		{
			float col[4] = { 0.8f,0.4f,0.4f,1 };
			dc.renderer->DrawLabelEx( p3+Vec3(0,0,0.3f),1.2f,col,true,true,m_eventTargets[i].event );
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CEntity::Serialize( CObjectArchive &ar )
{
	CBaseObject::Serialize( ar );
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		m_entityId = 0;
		
		m_physicsState = "";
		// Load
		CString entityClass = m_entityClass;
		m_loadFailed = false;

		if (!m_prototype)
			xmlNode->getAttr( "EntityClass",entityClass );
		xmlNode->getAttr( "EntityId",m_entityId );
		xmlNode->getAttr( "PhysicsState",m_physicsState );

		ZeroStruct(m_materialGUID);
		if (xmlNode->getAttr( "MaterialGUID",m_materialGUID ))
		{
			m_pMaterial = (CMaterial*)GetIEditor()->GetMaterialManager()->FindItem( m_materialGUID );
			if (!m_pMaterial)
			{
				CErrorRecord err;
				err.error.Format( "Material %s for Entity %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
				err.pObject = this;
				err.severity = CErrorRecord::ESEVERITY_WARNING;
				ar.ReportError(err);
				//Warning( "Material %s for Entity %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
			}
			else
			{
				if (m_pMaterial->GetParent())
					SetMaterial( m_pMaterial->GetParent() );
				m_pMaterial->SetUsed();
			}
			UpdateMaterialInfo();
		}
		else
			m_pMaterial = 0;

		// Load Event Targets.
		ReleaseEventTargets();
		XmlNodeRef eventTargets = xmlNode->findChild( "EventTargets" );
		if (eventTargets)
		{
			for (int i = 0; i < eventTargets->getChildCount(); i++)
			{
				XmlNodeRef eventTarget = eventTargets->getChild(i);
				CEntityEventTarget et;
				et.target = 0;
				GUID targetId = GUID_NULL;
				eventTarget->getAttr( "TargetId",targetId );
				eventTarget->getAttr( "Event",et.event );
				eventTarget->getAttr( "SourceEvent",et.sourceEvent );
				m_eventTargets.push_back( et );
				if (targetId != GUID_NULL)
					ar.SetResolveCallback( this,targetId,functor(*this,&CEntity::ResolveEventTarget),i );
			}
		}

		XmlNodeRef propsNode;
		XmlNodeRef props2Node = xmlNode->findChild("Properties2");
		if (!m_prototype)
		{
			propsNode = xmlNode->findChild("Properties");
		}

		bool bLoaded = LoadScript( entityClass,!ar.bUndo,true,propsNode,props2Node );
		if (ar.bUndo)
		{
			SpawnEntity();
		}
	}
	else
	{
		// Saving.
		if (!m_entityClass.IsEmpty() && m_prototype == NULL)
			xmlNode->setAttr( "EntityClass",m_entityClass );

		if (m_entityId != 0)
			xmlNode->setAttr( "EntityId",m_entityId );

		if (!GuidUtil::IsEmpty(m_materialGUID))
		{
			xmlNode->setAttr( "MaterialGUID",m_materialGUID );
		}

		if (!m_physicsState.IsEmpty())
			xmlNode->setAttr( "PhysicsState",m_physicsState );

		if (!m_prototype)
		{
			//! Save properties.
			if (m_properties)
			{
				XmlNodeRef propsNode = xmlNode->newChild("Properties");
				m_properties->Serialize( propsNode,ar.bLoading );
			}
		}

		//! Save properties.
		if (m_properties2)
		{
			XmlNodeRef propsNode = xmlNode->newChild("Properties2");
			m_properties2->Serialize( propsNode,ar.bLoading );
		}

		// Save Event Targets.
		if (!m_eventTargets.empty())
		{
			XmlNodeRef eventTargets = xmlNode->newChild( "EventTargets" );
			for (int i = 0; i < m_eventTargets.size(); i++)
			{
				CEntityEventTarget &et = m_eventTargets[i];
				GUID targetId = GUID_NULL;
				if (et.target != 0)
					targetId = et.target->GetId();

				XmlNodeRef eventTarget = eventTargets->newChild( "EventTarget" );
				eventTarget->setAttr( "TargetId",targetId );
				eventTarget->setAttr( "Event",et.event );
				eventTarget->setAttr( "SourceEvent",et.sourceEvent );
			}
		}
	}
}

XmlNodeRef CEntity::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	if (m_loadFailed)
		return 0;

	//@HACK
	//[Timur] This is the worst ever Hack! unfortunatly Have to be here because of our fabolus game programmers, sigh.
	if (m_entityClass == "DynamicLight")
	{
		// If dynamic light and casts lightmap do not export this.
		if (mv_castLightmap)
			return 0;
	}

	// Export entities to entities.ini
	XmlNodeRef objNode = CBaseObject::Export( levelPath,xmlNode );

	objNode->setTag( "Entity" );
	objNode->setAttr( "EntityClass",m_entityClass );
	objNode->setAttr( "EntityId",m_entityId );

	if (m_pMaterial)
	{
		objNode->setAttr( "Material",m_pMaterial->GetFullName() );
	}

	if (!m_physicsState.IsEmpty())
		objNode->setAttr( "PhysicsState",m_physicsState );

	objNode->setAttr( "Layer",GetLayer()->GetName() );

	// Store parent entity.
	if (GetParent())
	{
		if (GetParent()->IsKindOf( RUNTIME_CLASS(CEntity) ))
		{
			CEntity *parentEntity = (CEntity*)GetParent();
			if (parentEntity)
				objNode->setAttr( "ParentId",parentEntity->GetEntityId() );

			Vec3 pos = GetPos();
			Vec3 angles = GetAngles();
			Vec3 scale = GetScale();

			// When exporting optimize for 0,0,0 condition.
			
			// Export local coords.
			if (!IsEquivalent(pos,Vec3(0,0,0),0))
				objNode->setAttr( "Pos",pos );
			else
				objNode->delAttr( "Pos" );

			// Export local angles.
			if (!IsEquivalent(angles,Vec3(0,0,0),0))
				objNode->setAttr( "Angles",angles );
			else
				objNode->delAttr( "Angles" );

			// Export local scale.
			if (!IsEquivalent(scale,Vec3(1,1,1),0))
				objNode->setAttr( "Scale",scale );
			else
				objNode->delAttr( "Scale" );
		}
	}

	// Export Event Targets.
	if (!m_eventTargets.empty())
	{
		XmlNodeRef eventTargets = objNode->newChild( "EventTargets" );
		for (int i = 0; i < m_eventTargets.size(); i++)
		{
			CEntityEventTarget &et = m_eventTargets[i];

			int entityId = 0;
			if (et.target)
			{
				if (et.target->IsKindOf( RUNTIME_CLASS(CEntity) ))
				{
					entityId = ((CEntity*)et.target)->GetEntityId();
				}
			}

			XmlNodeRef eventTarget = eventTargets->newChild( "EventTarget" );
			//eventTarget->setAttr( "Target",obj->GetName() );
			eventTarget->setAttr( "Target",entityId );
			eventTarget->setAttr( "Event",et.event );
			eventTarget->setAttr( "SourceEvent",et.sourceEvent );
		}
	}

	//! Export properties.
	if (m_properties)
	{
		XmlNodeRef propsNode = objNode->newChild("Properties");
		m_properties->Serialize( propsNode,false );
	}
	//! Export properties.
	if (m_properties2)
	{
		XmlNodeRef propsNode = objNode->newChild("Properties2");
		m_properties2->Serialize( propsNode,false );
	}

	return objNode;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnEvent( ObjectEvent event )
{
	CBaseObject::OnEvent(event);

	switch (event)
	{
		case EVENT_INGAME:
		case EVENT_OUTOFGAME:
			if (m_entity)
			{
				if (event == EVENT_INGAME)
				{
					if (!m_bCalcPhysics)
						m_entity->ClearFlags(ETY_FLAG_IGNORE_PHYSICS_UPDATE);
					// Entity must be hidden when going to game.
					if (m_visible)
						m_entity->Hide( mv_hiddenInGame );
				}
				else if (event == EVENT_OUTOFGAME)
				{
					// Entity must be returned to editor visibility state.
					m_entity->Hide( !m_visible );
					m_entity->SetGarbageFlag(false); // If marked as garbage, unmark it.
				}
				XFormGameEntity();
				if (!m_physicsState.IsEmpty())
				{
					IPhysicalEntity *physic = m_entity->GetPhysics();
					if (physic)
					{
						const char *str = m_physicsState;
						physic->SetStateFromSnapshotTxt( const_cast<char*>(str),m_physicsState.GetLength() );
						physic->PostSetStateFromSnapshot();
					}
				}
				if (event == EVENT_OUTOFGAME)
				{
					if (!m_bCalcPhysics)
						m_entity->SetFlags(ETY_FLAG_IGNORE_PHYSICS_UPDATE);
				}
			}
			break;
		case EVENT_REFRESH:
			if (m_entity)
			{
				// force entity to be registered in terrain sectors again.

				//-- little hack to force reregistration of entities
				//<<FIXME>> when issue with registration in editor is resolved
				Vec3d pos = GetPos();
				pos.z+=1.f;
				m_entity->SetPos( pos,false );
				//----------------------------------------------------

				XFormGameEntity();
			}
			break;

		case EVENT_AFTER_LOAD:
			if (m_entity)
			{
				// Force entities to register them-self in sectors.
				// force entity to be registered in terrain sectors again.
				XFormGameEntity();
				BindToParent();
				BindIEntityChilds();
				if (m_script)
				{
					//m_script->SetCurrentProperties( this );
					m_script->SetEventsTable( this );
				}
			}
			break;

		case EVENT_UNLOAD_GEOM:
		case EVENT_UNLOAD_ENTITY:
			if (m_script)
				m_script = 0;
			if (m_entity)
			{
				UnloadScript();
			}
			break;

		case EVENT_RELOAD_ENTITY:
			GetIEditor()->GetErrorReport()->SetCurrentValidatorObject( this );
			if (m_script)
				m_script->Reload();
			Reload();
			break;

		case EVENT_RELOAD_GEOM:
			GetIEditor()->GetErrorReport()->SetCurrentValidatorObject( this );
			Reload();
			break;

		case EVENT_PHYSICS_GETSTATE:
			AcceptPhysicsState();
			break;
		case EVENT_PHYSICS_RESETSTATE:
			ResetPhysicsState();
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::Reload( bool bReloadScript )
{
	if (!m_script || bReloadScript)
		LoadScript( m_entityClass,true );
	if (m_entity)
		DeleteEntity();
	SpawnEntity();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdateVisibility( bool visible )
{
	CBaseObject::UpdateVisibility(visible);
	if (visible != m_visible)
	{
		if (!visible && m_entity)
		{
			m_entity->Hide(true);
			// Unload entity.
			//OnEvent(EVENT_UNLOAD_ENTITY);
		}
		if (visible && m_entity)
		{
			m_entity->Hide(false);
		}
	}
	m_visible = visible;
};

/*
class StaticInit
{
public:
	StaticInit()
	{
		Vec3 angles(20,11.51,112);
		Vec3 a1,a2;

		Matrix tm;
		tm.Identity();
		tm.RotateMatrix( angles );
		quaternionf qq( angles.z*PI/180.0f,angles.y*PI/180.0f,angles.x*PI/180.0f );

		float mtx[3][3];
		qq.getmatrix_buf( (float*)mtx );

		Quat q(tm);
		Quat q1;
		q1.SetEulerAngles( angles*PI/180.0f );
		Matrix tm1;
		q1.GetMatrix(tm1);
		a1 = q.GetEulerAngles() * 180.0f/PI;
		a2 = q1.GetEulerAngles() * 180.0f/PI;
	}
};
StaticInit ss;
*/

//////////////////////////////////////////////////////////////////////////
void CEntity::OnNodeAnimated()
{
	if (!m_animNode)
		return;

	Vec3 pos = m_animNode->GetPos();
	Vec3 scale = m_animNode->GetScale();
	Quat rotate = m_animNode->GetRotate();


	//m_entity->SetPos( GetPos(),false );
	//m_entity->SetAngles( GetAngles() );
	//m_entity->SetScale( GetScale().x );

	bool bModified = false;
	if (!IsVectorsEqual(pos,GetPos()))
	{
		CBaseObject::SetPos( pos );
		bModified = true;
		if (m_entity)
			m_entity->SetPos( GetPos(),false );
	}
	if (rotate.w != m_rotate.w || rotate.v.x != m_rotate.v.x || rotate.v.y != m_rotate.v.y || rotate.v.z != m_rotate.v.z)
	{
		m_rotate = rotate;
		Vec3 angles = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(m_rotate)));
		CBaseObject::SetAngles( angles );
		bModified = true;
		if (m_entity)
			m_entity->SetAngles( GetAngles() );
	}
	if (!IsVectorsEqual(scale,GetScale()))
	{
		CBaseObject::SetScale( scale );
		bModified = true;
		if (m_entity)
			m_entity->SetScale( GetScale().x );
	}
	//if (bModified)
		//XFormGameEntity();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::InvalidateTM()
{
	CBaseObject::InvalidateTM();

	// If matrix changes.
	if (m_trackGizmo)
	{
		//m_trackGizmo->SetMatrix( GetWorldTM() );
		if (GetParent())
		{
			m_trackGizmo->SetMatrix( GetParent()->GetWorldTM() );
		}
		else
		{
			Matrix44 tm;
			tm.SetIdentity();
			m_trackGizmo->SetMatrix(tm);
		}
	}

	if (m_entity && GetParent())
	{
		if (!m_entity->IsBound())
		{
			m_entity->ForceBindCalculation(true);
			m_entity->SetParentLocale( GetParent()->GetWorldTM() );
			XFormGameEntity();
		}
	}
	m_bEntityXfromValid = false;
}

//////////////////////////////////////////////////////////////////////////
//! Attach new child node.
void CEntity::AttachChild( CBaseObject* child,bool bKeepPos )
{
	CBaseObject::AttachChild( child,bKeepPos );
	if (child && child->IsKindOf(RUNTIME_CLASS(CEntity)))
	{
		((CEntity*)child)->BindToParent();
	}
}

//! Detach all childs of this node.
void CEntity::DetachAll( bool bKeepPos )
{
	//@FIXME: Unbind all childs.
	CBaseObject::DetachAll(bKeepPos);
}

// Detach this node from parent.
void CEntity::DetachThis( bool bKeepPos )
{
	UnbindIEntity();
	CBaseObject::DetachThis(bKeepPos);
}

//////////////////////////////////////////////////////////////////////////
void CEntity::BindToParent()
{
	if (!m_entity)
		return;

	CBaseObject *parent = GetParent();
	if (parent)
	{
		if (parent->IsKindOf(RUNTIME_CLASS(CEntity)))
		{
			CEntity *parentEntity = (CEntity*)parent;

			IEntity *ientParent = parentEntity->GetIEntity();
			if (ientParent)
			{
				XFormGameEntity();
				ientParent->Bind( m_entity->GetId(),0 );
				XFormGameEntity();
			}
		}
		else
		{
			m_entity->ForceBindCalculation(true);
			m_entity->SetParentLocale( GetParent()->GetWorldTM() );
			XFormGameEntity();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::BindIEntityChilds()
{
	if (!m_entity)
		return;

	int numChilds = GetChildCount();
	for (int i = 0; i < numChilds; i++)
	{
		CBaseObject *child = GetChild(i);
		if (child && child->IsKindOf(RUNTIME_CLASS(CEntity)))
		{
			IEntity *ientChild = ((CEntity*)child)->GetIEntity();
			if (ientChild)
			{
				((CEntity*)child)->XFormGameEntity();
				m_entity->Bind( ientChild->GetId(),0 );
				((CEntity*)child)->XFormGameEntity();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UnbindIEntity()
{
	if (!m_entity)
		return;

	m_entity->ForceBindCalculation(false);
	
	CBaseObject *parent = GetParent();
	if (parent && parent->IsKindOf(RUNTIME_CLASS(CEntity)))
	{
		CEntity *parentEntity = (CEntity*)parent;
		
		IEntity *ientParent = parentEntity->GetIEntity();
		if (ientParent)
		{
			//m_entity->ForceBindCalculation(false);
			ientParent->Unbind( m_entity->GetId(),0 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::PostClone( CBaseObject *pFromObject,CObjectCloneContext &ctx )
{
	CBaseObject::PostClone( pFromObject,ctx );
	
	CEntity *pFromEntity = (CEntity*)pFromObject;
	// Clone event targets.
	if (!pFromEntity->m_eventTargets.empty())
	{
		int numTargets = pFromEntity->m_eventTargets.size();
		for (int i = 0; i < numTargets; i++)
		{
			CEntityEventTarget &et = pFromEntity->m_eventTargets[i];
			CBaseObject *pClonedTarget = ctx.FindClone( et.target );
			if (!pClonedTarget)
				pClonedTarget = et.target; // If target not cloned, link to original target.

			// Add cloned event.
			AddEventTarget( pClonedTarget,et.event,et.sourceEvent,true );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::ResolveEventTarget( CBaseObject *object,unsigned int index )
{
	// Find targetid.
	assert( index >= 0 && index < m_eventTargets.size() );
	if (object)
		object->AddEventListener( functor(*this,&CEntity::OnEventTargetEvent) );
	m_eventTargets[index].target = object;
	if (!m_eventTargets.empty() && m_script != 0)
		m_script->SetEventsTable( this );

	// Make line gizmo.
	if (!m_eventTargets[index].pLineGizmo && object)
	{
		CLineGizmo *pLineGizmo = new CLineGizmo;
		pLineGizmo->SetObjects( this,object );
		pLineGizmo->SetColor( Vec3(0.8f,0.4f,0.4f),Vec3(0.8f,0.4f,0.4f) );
		pLineGizmo->SetName( m_eventTargets[index].event );
		AddGizmo( pLineGizmo );
		m_eventTargets[index].pLineGizmo = pLineGizmo;
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::ReleaseEventTargets()
{
	while (!m_eventTargets.empty())
		RemoveEventTarget( m_eventTargets.size()-1,false );
	m_eventTargets.clear();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnEventTargetEvent( CBaseObject *target,int event )
{
	// When event target is deleted.
	if (event == CBaseObject::ON_DELETE)
	{
		// Find this target in events list and remove.
		int numTargets = m_eventTargets.size();
		for (int i = 0; i < numTargets; i++)
		{
			if (m_eventTargets[i].target == target)
			{
				RemoveEventTarget( i );
				numTargets = m_eventTargets.size();
				i--;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CEntity::AddEventTarget( CBaseObject *target,const CString &event,const CString &sourceEvent,bool bUpdateScript )
{
	StoreUndo( "Add EventTarget" );
	CEntityEventTarget et;
	et.target = target;
	et.event = event;
	et.sourceEvent = sourceEvent;

	// Assign event target.
	if (et.target)
		et.target->AddEventListener( functor(*this,&CEntity::OnEventTargetEvent) );

	if (target)
	{
		// Make line gizmo.
		CLineGizmo *pLineGizmo = new CLineGizmo;
		pLineGizmo->SetObjects( this,target );
		pLineGizmo->SetColor( Vec3(0.8f,0.4f,0.4f),Vec3(0.8f,0.4f,0.4f) );
		pLineGizmo->SetName( event );
		AddGizmo( pLineGizmo );
		et.pLineGizmo = pLineGizmo;
	}

	m_eventTargets.push_back( et );

	// Update event table in script.
	if (bUpdateScript && m_script != 0)
		m_script->SetEventsTable( this );

	return m_eventTargets.size()-1;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::RemoveEventTarget( int index,bool bUpdateScript )
{
	if (index >= 0 && index < m_eventTargets.size())
	{
		StoreUndo( "Remove EventTarget" );

		if (m_eventTargets[index].pLineGizmo)
		{
			RemoveGizmo( m_eventTargets[index].pLineGizmo );
		}

		if (m_eventTargets[index].target)
			m_eventTargets[index].target->RemoveEventListener( functor(*this,&CEntity::OnEventTargetEvent) );
		m_eventTargets.erase( m_eventTargets.begin()+index );

		// Update event table in script.
		if (bUpdateScript && m_script != 0 && m_entity != 0)
			m_script->SetEventsTable( this );
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnRenderFlagsChange( IVariable *var )
{
	if (m_entity)
	{
		m_entity->SetRndFlags(ERF_CASTSHADOWVOLUME, mv_castShadows);
		m_entity->SetRndFlags(ERF_SELFSHADOW, mv_selfShadowing);;
		m_entity->SetRndFlags(ERF_CASTSHADOWMAPS, mv_castShadowMaps);
		m_entity->SetRndFlags(ERF_RECVSHADOWMAPS, mv_recvShadowMaps);
		m_entity->SetRndFlags(ERF_CASTSHADOWINTOLIGHTMAP,mv_castLightmap);
		m_entity->SetRndFlags(ERF_USELIGHTMAPS,mv_recvLightmap);
		m_entity->SetRndFlags(ERF_SELECTED,IsSelected());

		m_entity->SetLodRatio(mv_ratioLOD);
		m_entity->SetViewDistRatio(mv_ratioViewDist);
    //m_entity->SetUpdateVisLevel((EEntityUpdateVisLevel)(int)mv_UpdateVisLevel);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnRadiusChange( IVariable *var )
{
	var->Get(m_proximityRadius);
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnInnerRadiusChange( IVariable *var )
{
	var->Get(m_innerRadius);
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnOuterRadiusChange( IVariable *var )
{
	var->Get(m_outerRadius);
}

//////////////////////////////////////////////////////////////////////////
CVarBlock* CEntity::CloneProperties( CVarBlock *srcProperties )
{
	assert( srcProperties );

	CVarBlock *properties = srcProperties->Clone(true);

	//@FIXME Hack to dispay radiuses of properties.
	// wires properties from param block, to this entity internal variables.
	IVariable *var = 0;
	var = properties->FindVariable( "Radius",false );
	if (var && (var->GetType() == IVariable::FLOAT || var->GetType() == IVariable::INT))
	{
		var->Get(m_proximityRadius);
		var->AddOnSetCallback( functor(*this,&CEntity::OnRadiusChange) );
	}
	var = properties->FindVariable( "InnerRadius",false );
	if (var && (var->GetType() == IVariable::FLOAT || var->GetType() == IVariable::INT))
	{
		var->Get(m_innerRadius);
		var->AddOnSetCallback( functor(*this,&CEntity::OnInnerRadiusChange) );
	}
	var = properties->FindVariable( "OuterRadius",false );
	if (var && (var->GetType() == IVariable::FLOAT || var->GetType() == IVariable::INT))
	{
		var->Get(m_outerRadius);
		var->AddOnSetCallback( functor(*this,&CEntity::OnOuterRadiusChange) );
	}

	// Each property must have callback to our OnPropertyChange.
	properties->AddOnSetCallback( functor(*this,&CEntity::OnPropertyChange) );

	return properties;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::OnLoadFailed()
{
	m_loadFailed = true;

	CErrorRecord err;
	err.error.Format( "Entity %s Failed to Spawn (Script: %s)",(const char*)GetName(),(const char*)m_entityClass );
	err.pObject = this;
	GetIEditor()->GetErrorReport()->ReportError(err);
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetMaterial( CMaterial *mtl )
{
	StoreUndo( "Assign Material" );
	m_pMaterial = mtl;
	if (m_pMaterial)
	{
		m_pMaterial->SetUsed();
		m_materialGUID = m_pMaterial->GetGUID();
	}
	else
	{
		ZeroStruct(m_materialGUID);
	}
	UpdateMaterialInfo();
}

//////////////////////////////////////////////////////////////////////////
void CEntity::UpdateMaterialInfo()
{
	if (m_entity)
	{
		if (m_pMaterial)
			m_pMaterial->AssignToEntity( m_entity );
		else
			m_entity->SetMaterial(0);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::AcceptPhysicsState()
{
	if (m_entity)
	{
		//StoreUndo( "Accept Physics State" );
		// [Anton] - StoreUndo sends EVENT_AFTER_LOAD, which forces position and angles to editor's position and
		// angles, which are not updated with the physics value
		Vec3 pos = m_entity->GetPos();
		Vec3 angles = m_entity->GetAngles();
		SetPos( pos );
		SetAngles( angles );
		IPhysicalEntity *physic = m_entity->GetPhysics();
		if (physic && physic->GetType() == PE_ARTICULATED)
		{
			// Only get state snapshot for articulated characters.
			char str[4096*4];
			int len = physic->GetStateSnapshotTxt( str,sizeof(str) );
			if (len > sizeof(str)-1)
				len = sizeof(str)-1;
			str[len] = 0;
			m_physicsState = str;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::ResetPhysicsState()
{
	if (m_entity)
	{
		//StoreUndo( "Reset Physics State" );
		m_physicsState = "";
		Reload();
	}
}

void CEntity::SetHelperScale( float scale )
{
	bool bChanged = m_helperScale != scale;
	m_helperScale = scale;
	if (bChanged)
	{
		CalcBBox();
	}
}

//////////////////////////////////////////////////////////////////////////
//! Analyze errors for this object.
void CEntity::Validate( CErrorReport *report )
{
	CBaseObject::Validate( report );
	// Checks for invalid values in base object.
	if (!GuidUtil::IsEmpty(m_materialGUID) && m_pMaterial == NULL)
	{
		CErrorRecord err;
		err.error.Format( "Material %s for Entity %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
		err.pObject = this;
		report->ReportError(err);
		//Warning( "Material %s for Entity %s not found,",GuidUtil::ToString(m_materialGUID),(const char*)GetName() );
	}

	if (!m_entity)
	{
		CErrorRecord err;
		err.error.Format( "Entity %s Failed to Spawn (Script: %s)",(const char*)GetName(),(const char*)m_entityClass );
		err.pObject = this;
		report->ReportError(err);
		return;
	}

	int slot;
	
	// Check Entity.
	int numObj = m_entity->GetNumObjects();
	for (slot = 0; slot < numObj; slot++)
	{
		CEntityObject obj;
		if (!m_entity->GetEntityObject( slot,obj ))
			continue;

		if (obj.object != NULL && obj.object->IsDefaultObject())
		{
			//const char *filename = obj.object->GetFileName();
			// File Not found.
			CErrorRecord err;
			err.error.Format( "Geometry File in Slot %d for Entity %s not found",slot,(const char*)GetName() );
			//err.file = filename;
			err.pObject = this;
			err.flags = CErrorRecord::FLAG_NOFILE;
			report->ReportError(err);
		}
	}
	/*
	IEntityCharacter *pIChar = m_entity->GetCharInterface();
	pIChar->GetCharacter(0);
	for (slot = 0; slot < MAX_ANIMATED_MODELS; slot++)
	{
		ICryCharInstance *pCharacter = pIChar->GetCharacter(slot);
		pCharacter->IsDe
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CEntity::GatherUsedResources( CUsedResources &resources )
{
	CBaseObject::GatherUsedResources( resources );
	if (m_properties)
		m_properties->GatherUsedResources( resources );
	if (m_properties2)
		m_properties2->GatherUsedResources( resources );
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::IsSimilarObject( CBaseObject *pObject )
{
	if (pObject->GetClassDesc() == GetClassDesc() && pObject->GetRuntimeClass() == GetRuntimeClass())
	{
		CEntity *pEntity = (CEntity*)pObject;
		if (m_entityClass == pEntity->m_entityClass && 
				m_proximityRadius == pEntity->m_proximityRadius &&
				m_innerRadius == pEntity->m_innerRadius &&
				m_outerRadius == pEntity->m_outerRadius)
		{
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// CSimple Entity.
//////////////////////////////////////////////////////////////////////////
bool CSimpleEntity::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	bool bRes = false;
	if (file.IsEmpty())
	{
		bRes = CEntity::Init( ie,prev,"" );
	}
	else
	{
		bRes = CEntity::Init( ie,prev,"BasicEntity" );
		LoadScript( m_entityClass );
		SetGeometryFile( file );
	}

	return bRes;
}

void CSimpleEntity::SetGeometryFile( const CString &filename )
{
	if (m_properties)
	{
		IVariable* pModelVar = m_properties->FindVariable( "object_Model" );
		if (pModelVar)
		{
			pModelVar->Set( filename );
		}
	}
}

CString CSimpleEntity::GetGeometryFile() const
{
	CString filename;
	if (m_properties)
	{
		IVariable* pModelVar = m_properties->FindVariable( "object_Model" );
		if (pModelVar)
		{
			pModelVar->Get( filename );
		}
	}
	return filename;
}

//////////////////////////////////////////////////////////////////////////
bool CSimpleEntity::ConvertFromObject( CBaseObject *object )
{
	CBaseObject::ConvertFromObject( object );
	if (object->IsKindOf(RUNTIME_CLASS(CBrushObject)))
	{
		CBrushObject *pBrushObject = (CBrushObject*)object;

		IStatObj *prefab = pBrushObject->GetPrefabGeom();
		if (!prefab)
			return false;

		// Copy entity shadow parameters.
		int rndFlags = pBrushObject->GetRenderFlags();

		mv_castShadows = (rndFlags & ERF_CASTSHADOWVOLUME) != 0;
		mv_selfShadowing = (rndFlags & ERF_SELFSHADOW) != 0;
		mv_castShadowMaps = (rndFlags & ERF_CASTSHADOWMAPS) != 0;
    mv_recvShadowMaps = (rndFlags & ERF_RECVSHADOWMAPS) != 0;
		mv_castLightmap = (rndFlags & ERF_CASTSHADOWINTOLIGHTMAP) != 0;
		mv_recvLightmap = (rndFlags & ERF_USELIGHTMAPS) != 0;
		mv_ratioLOD = pBrushObject->GetRatioLod();
		mv_ratioViewDist = pBrushObject->GetRatioViewDist();

		LoadScript( "BasicEntity" );
		SpawnEntity();
		SetGeometryFile( prefab->GetFileName() );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CSimpleEntity::BeginEditParams( IEditor *ie,int flags )
{
	CEntity::BeginEditParams( ie,flags );

	CString filename = GetGeometryFile();

	if (gSettings.bGeometryBrowserPanel)
	{
		if (!filename.IsEmpty())
		{
			if (!s_treePanel)
			{
				s_treePanel = new CPanelTreeBrowser;
				int flags = CPanelTreeBrowser::NO_DRAGDROP|CPanelTreeBrowser::NO_PREVIEW|CPanelTreeBrowser::SELECT_ONCLICK;
				s_treePanel->Create( functor(*this,&CSimpleEntity::OnFileChange),GetClassDesc()->GetFileSpec(),AfxGetMainWnd(),flags );
			}
			if (s_treePanelId == 0)
				s_treePanelId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,_T("Geometry"),s_treePanel,false );
		}

		if (s_treePanel)
		{
			s_treePanel->SetSelectCallback( functor(*this,&CSimpleEntity::OnFileChange) );
			s_treePanel->SelectFile( filename );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CSimpleEntity::EndEditParams( IEditor *ie )
{
	if (s_treePanelId != 0)
	{
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,s_treePanelId );
		s_treePanelId = 0;
	}

	CEntity::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
void CSimpleEntity::OnFileChange( CString filename )
{
	CUndo undo("Entity Geom Modify");
	StoreUndo( "Entity Geom Modify" );
	SetGeometryFile( filename );
}

//////////////////////////////////////////////////////////////////////////
//! Analyze errors for this object.
void CSimpleEntity::Validate( CErrorReport *report )
{
	CEntity::Validate( report );
	
	// Checks if object loaded.
}

//////////////////////////////////////////////////////////////////////////
bool CSimpleEntity::IsSimilarObject( CBaseObject *pObject )
{
	if (pObject->GetClassDesc() == GetClassDesc() && pObject->GetRuntimeClass() == GetRuntimeClass())
	{
		CSimpleEntity *pEntity = (CSimpleEntity*)pObject;
		if (GetGeometryFile() == pEntity->GetGeometryFile())
		{
			return true;
		}
	}
	return false;
}