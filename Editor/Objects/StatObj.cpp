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
#include "StatObj.h"
#include "..\StatObjPanel.h"
#include "selectiongroup.h"

#include "..\Viewport.h"
#include "..\AnimationContext.h"

#include <I3DEngine.h>
#include <IEntitySystem.h>

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
// Do not replace CBaseObject to CEntity, we want in Our hierarchy static objects be not derived from entity.
IMPLEMENT_DYNCREATE(CStaticObject,CEntity)

int CStaticObject::m_rollupId = 0;
CStatObjPanel* CStaticObject::m_panel = 0;

//////////////////////////////////////////////////////////////////////////
CStaticObject::CStaticObject()
{
	//m_staticEntity = true;
	m_loadFailed = false;
	m_bCharacter = false;
	
	mv_rigidBody = false;
	mv_hidable = false;
	mv_density = 1;
	mv_mass = 1;

	mv_animationLoop = false;
	mv_animationSpeed = 1;

	AddVariable( mv_hidable,"Hidable",functor(*this,OnRigidBodyChange) );
	AddVariable( mv_rigidBody,"RigidBody",functor(*this,OnRigidBodyChange) );
	AddVariable( mv_mass,"Mass",functor(*this,OnMassChange) );
	AddVariable( mv_density,"Density",functor(*this,OnMassChange) );
	AddVariable( mv_animation,"Animation",functor(*this,OnAnimationChange) );
	AddVariable( mv_animationLoop,"AnimLoop",functor(*this,OnAnimationChange) );
	AddVariable( mv_animationSpeed,"AnimSpeed",functor(*this,OnAnimationChange) );
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::Done()
{
	UnloadObject();
	CEntity::Done();
}

void CStaticObject::UnloadObject()
{
	UnloadEntity();
}

//////////////////////////////////////////////////////////////////////////
bool CStaticObject::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	//CBaseObject::Init( ie,prev,file );

	SetColor( RGB(0,255,255) );

	CEntity::Init( ie,prev,"" );

	if (prev)
	{
		CStaticObject* po = (CStaticObject*)prev;
		LoadObject( po->GetObjectName(),true );
	}
	else if (!file.IsEmpty())
	{
		LoadObject( file,true );
		char filename[1024];
		_splitpath( file,0,0,filename,0 );
		SetUniqName( filename );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::SetScale( const Vec3d &scale )
{
	CBaseObject::SetScale( scale );
	
	IEntity *entity =	GetIEntity();
	if (entity)
	{
		entity->SetScale( GetScale().x );
		CalcBBox();
	}
	if (m_animNode)
	{
		m_animNode->SetScale( GetIEditor()->GetAnimation()->GetTime(),scale );
	}
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::LoadObject( const CString &file,bool bForceReload )
{
	// Ignore loading if object name is same as before.
	if (file == m_objectName && !m_loadFailed && !bForceReload)
		return;

	m_objectName = file;

	if (!IsCreateGameObjects())
		return;
	
	CLogFile::FormatLine( "Loading Static Object %s",(const char*)file );

	UnloadObject();

	m_loadFailed = false;

	// Spawn new entity.
	LoadEntity( "StaticEntity",true );
	// Load this static object to the entity.
	IEntity *entity =	GetIEntity();
	if (entity)
	{
		if (entity->GetCharInterface()->LoadCharacter( 0,file ))
		{
			entity->DrawCharacter(0,ETY_DRAW_NORMAL);
			int flags = entity->GetCharInterface()->GetCharacter(0)->GetFlags();
			entity->GetCharInterface()->GetCharacter(0)->SetFlags(flags|CS_FLAG_UPDATE);
			// Character loaded.
			m_bCharacter = true;
		}
		else
		{
			// Object loaded.
			entity->LoadObject( 0,m_objectName,1 );
			entity->DrawObject( 0,ETY_DRAW_NORMAL );
			m_bCharacter = false;
		}
	//	CEntityObject the_object;
	//	if (entity->GetEntityObject(0,the_object)) 
	//	{
	//		the_object.object->SetHideability(m_hidable);
	//	}

		entity->SetNetPresence(false);
		entity->SetScale( GetScale().x );

		if (mv_rigidBody) 
		{
			//entity->SetNeedUpdate(true);
			entity->CreateRigidBody(PE_RIGID, mv_density,mv_mass,0 );
		}
		else
		{
			//entity->SetNeedUpdate(false);
			entity->CreateStaticEntity( mv_mass,0 );
		}
		
		if (m_bCharacter)
		{
			OnAnimationChange(0);
			entity->GetCharInterface()->PhysicalizeCharacter(0,mv_mass,0 );
		}

		CalcBBox();
				// new stuff - PETAR
		IPhysicalEntity *pPhysics = entity->GetPhysics();
		if (pPhysics)
		{
			if (mv_rigidBody)
			{
				pe_params_pos params;
				params.iSimClass = 1; // sleep.
				pPhysics->SetParams( &params );
			}

			pe_params_foreign_data pfn;
			pPhysics->GetParams(&pfn);
			if (mv_hidable)
			{
					pfn.iForeignFlags|=PFF_HIDABLE;
			}
			else
			{
					pfn.iForeignFlags&=~PFF_HIDABLE;
			}
			pPhysics->SetParams(&pfn);
		}
		//----------------------------------

	}
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::ReloadObject()
{
	LoadObject( m_objectName,true );
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::BeginEditParams( IEditor *ie,int flags )
{
	CBaseObject::BeginEditParams( ie,flags );

	if (!m_panel)
	{
		m_panel = new CStatObjPanel(AfxGetMainWnd());
		m_rollupId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,"StaticObject Parameters",m_panel );
	}
	if (m_panel)
	{
		m_panel->SetObject( this );
	}
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::EndEditParams( IEditor *ie )
{
	if (m_rollupId != 0)
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,m_rollupId );

	m_rollupId = 0;
	m_panel = 0;

	CBaseObject::EndEditParams( ie );
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::BeginEditMultiSelParams( bool bAllOfSameType )
{
	CBaseObject::BeginEditMultiSelParams(bAllOfSameType);
	
	if (bAllOfSameType)
	{
		m_panel = new CStatObjPanel(AfxGetMainWnd());
		m_rollupId = GetIEditor()->AddRollUpPage( ROLLUP_OBJECTS,"StaticObject Parameters",m_panel );
		m_panel->SetMultiSelect( true );
	}
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::EndEditMultiSelParams()
{
	if (m_rollupId)
		GetIEditor()->RemoveRollUpPage( ROLLUP_OBJECTS,m_rollupId );
	m_rollupId = 0;
	m_panel = 0;

	CBaseObject::EndEditMultiSelParams();
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::Display( DisplayContext &dc )
{
	if (!m_entity)
		return;

	COLORREF col = GetColor();
	if (IsSelected())
	{
		//float scl = GetScale().x;

		dc.PushMatrix( GetWorldTM() );

		dc.SetSelectedColor();
		dc.DrawWireBox( m_box.min,m_box.max );

		dc.PopMatrix();
	}
	else if (dc.flags & DISPLAY_2D)
	{
		dc.PushMatrix( GetWorldTM() );
		dc.SetColor( GetColor() );
		dc.DrawWireBox( m_box.min,m_box.max );
		dc.PopMatrix();
	}
	DrawDefault(dc,col);
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::Serialize( CObjectArchive &ar )
{
	CBaseObject::Serialize( ar );
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		// Loading.
		m_loadFailed = false;

		xmlNode->getAttr( "EntityId",m_entityId );
		xmlNode->getAttr( "ObjectName",m_objectName );
		LoadObject( m_objectName,true );

		if (ar.bUndo && m_panel && m_panel->GetObject() == this)
			m_panel->UpdateObject();
	}
	else
	{
		// Saving.
		if (m_entityId != 0)
			xmlNode->setAttr( "EntityId",m_entityId );
		
		if (!m_objectName.IsEmpty())
			xmlNode->setAttr( "ObjectName",m_objectName );
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CStaticObject::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	// Ignore exporting.
	XmlNodeRef objNode = CEntity::Export( levelPath,xmlNode );
	objNode->setTag( "StaticEntity" );

	if (m_bCharacter)
	{
		// Character.
		objNode->setAttr( "CharacterName",m_objectName );
	}
	else
	{
		objNode->setAttr( "ObjectName",m_objectName );
	}

	return objNode;
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::OnEvent( ObjectEvent event )
{
	switch (event)
	{
		case EVENT_UNLOAD_ENTITY:
		case EVENT_RELOAD_ENTITY:
			// For static objects ignore unloading of entity.
			return;
		
		case EVENT_RELOAD_GEOM:
			ReloadObject();
			break;
		case EVENT_UNLOAD_GEOM:
			UnloadObject();
			break;

		case EVENT_OUTOFGAME:
			// After exiting game reset simulation class to sleeping.
			if (m_entity)
			{
				IPhysicalEntity *pPhysics = m_entity->GetPhysics();
				if (pPhysics)
				{
					if (mv_rigidBody)
					{
						pe_params_pos params;
						params.iSimClass = 1; // sleep.
						pPhysics->SetParams( &params );
					}
				}
			}
			break;
	}

	CEntity::OnEvent(event);
	return;
/*
	CBaseObject::OnEvent(event);
	
	if (!m_loadFailed && !m_objectName.IsEmpty())
	{
		switch (event)
		{
		case EVENT_INGAME:
			if (!IsHidden())
				GetIEditor()->Get3DEngine()->AddStaticObject( m_objectName,GetPos(),GetScale().x );
			break;
		case EVENT_OUTOFGAME:
			if (!IsHidden())
				GetIEditor()->Get3DEngine()->RemoveStaticObject( m_objectName,GetPos() );
			break;
		case EVENT_REFRESH:
			// When refreshing make sure object is removed.
			GetIEditor()->Get3DEngine()->RemoveStaticObject( m_objectName,GetPos() );
			break;
		case EVENT_RELOAD_TEXTURES:
			// When refreshing make sure object is removed.
			if (m_object)
				m_object->Refresh(FRO_SHADERS|FRO_TEXTURES);
			break;
		case EVENT_RELOAD_GEOM:
			// When refreshing make sure object is removed.
			ReloadObject();
			break;
		case EVENT_UNLOAD_GEOM:
			// When refreshing make sure object is removed.
			ReleaseObject();
			break;
		}
	}
	*/
}

void CStaticObject::SetRigidBody( bool enable )
{
	StoreUndo( "RigidBody" );
	mv_rigidBody = enable;
};

//////////////////////////////////////////////////////////////////////////
void CStaticObject::SetHidable( bool hidable )
{
	StoreUndo( "Hidable" );

	mv_hidable = hidable;
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::SetMass( float mass )
{
	StoreUndo( "Mass" );
	mv_mass = mass;
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::OnRigidBodyChange( IVariable *var )
{
	// Reload object.
	if (m_entity)
		LoadObject( m_objectName,true );
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::OnMassChange( IVariable *var )
{
	// Reload object.
	if (m_entity && mv_rigidBody)
	{
		IPhysicalEntity *phys = m_entity->GetPhysics();
		pe_params_pos param;
		//param.s
		//m_physic->SetParams(&temp);
		//	phys->SetParams( LoadObject( m_objectName,true );
	}
}

//////////////////////////////////////////////////////////////////////////
void CStaticObject::OnAnimationChange( IVariable *var )
{
	if (m_entity && m_bCharacter)
	{
		ICryCharInstance *character = m_entity->GetCharInterface()->GetCharacter(0);
		if (character)
		{
			CString anim = mv_animation;
			if (anim.IsEmpty())
				anim = "Default";

			ICryAnimationSet *animSet = character->GetModel()->GetAnimationSet();
			if (animSet)
			{
				animSet->SetLoop( animSet->Find(anim),mv_animationLoop );
			}
			character->StartAnimation( anim );
			character->SetAnimationSpeed(mv_animationSpeed);
		}
	}
}