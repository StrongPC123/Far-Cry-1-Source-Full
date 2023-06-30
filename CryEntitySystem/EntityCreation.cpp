
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Entity.cpp
//  Description: Misc Entity functions
//
//	History:
//	-Feb 14,2001:Oirignally created by Marco Corbetta
//  -: modified by Everyone
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "EntitySystem.h"
#include <string.h>


#include <Cry_Math.h>
#include <IProcess.h>
#include <ISystem.h>
#include <ISound.h>

#include <IRenderer.h>
#include "ILog.h"
#include	<I3DEngine.h>
#include <IPhysics.h>
#include <IAISystem.h>
#include <IGame.h>
#include <ILipSync.h>
#include <MeshIdx.h> // needed to get access to vertex color list to determine attached vertices for cloth
#include <CryCharAnimationParams.h>
#include <CryCharMorphParams.h>

#include "LipSync.h"

#if defined(_DEBUG) && !defined(LINUX)
static char THIS_FILE[] = __FILE__;
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, THIS_FILE, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

#define CHECK_CHARACTER_SLOT(func) \
	if ((pos < 0) || (pos > MAX_ANIMATED_MODELS))\
	{\
		m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,0,"<CryEntitySystem> Invalid slot number for method (%s)",func );\
		return;\
	}

#define CHECK_CHARACTER_SLOT_0(func) \
	if ((pos < 0) || (pos > MAX_ANIMATED_MODELS))\
	{\
		m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,0,"<CryEntitySystem> Invalid slot number for method (%s)",func );\
		return 0;\
	}


//load an object at a specified position with a spcified scale
//////////////////////////////////////////////////////////////////////
bool CEntity::LoadObject( unsigned int slot,const char *fileName,float scale, const char *geomName)
{	
	// [marco] empty filename should not be a warning nor an
	// assertion since it is completely possible in the editor
	// or in the archetype to specifiy an empty filename for an
	// object
	//[Timur] Yes it must be a warning, Script must not allow loading of empty filenames this is Designers bug
	//	otherwise if it is by design that object is optional script should check it and not call LoadObject
	//  there was alot of bugs on the maps where designers forgot to specify object while they were must to.
	//assert( fileName );
	if ((slot>ENTITY_MAX_OBJECTS) || !fileName || (strlen(fileName)==0))
	{
		m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,0,
			"CEntity::LoadObject called with Empty filename, Entity: %s",m_name.c_str() );
		return false;
	}

	if (slot < m_objects.size())
	{
		IStatObj *prev = m_objects[slot].object;
		if (prev != 0)
		{
			// Check if previous object is exactly the same.
			// Ignore duplicate loading of objects.
			if (prev->IsSameObject( fileName,geomName))
			{
				return true;
			}

			// Release old object.
			m_pISystem->GetI3DEngine()->ReleaseObject(prev);
		}
	}
	
	IStatObj * cobj;
	if(geomName)
	{
			cobj = m_pISystem->GetI3DEngine()->MakeObject(fileName,geomName);	
	}
	else
		cobj = m_pISystem->GetI3DEngine()->MakeObject(fileName);	

	if (!cobj)
	{
		if(slot < m_objects.size())
			m_objects[slot].object=0;
		return (false);
	}

	if (slot >= m_objects.size())
		m_objects.resize( slot+1 );
	
	m_objects[slot].object=cobj;
	m_objects[slot].pos = Vec3d(0,0,0);

	m_pISystem->GetI3DEngine()->FreeEntityRenderState(this);
	InitEntityRenderState();

	// leave this call... it will cause the geometry to be properly registered
	CalcWholeBBox(); 

	return (true);
}



bool CEntity::GetObjectPos(unsigned int slot,Vec3d &pos)
{
	if(m_objects.size()>slot)
	{
		pos=m_objects[slot].pos;
		return true;
	}
	return false;
}

bool CEntity::SetObjectPos(unsigned int slot,const Vec3d &pos)
{
	if(m_objects.size()>slot)
	{
		m_objects[slot].pos=pos;
		
		IntToIntMap::iterator ii = m_mapSlotToPhysicalPartID.find(slot),iiend = m_mapSlotToPhysicalPartID.end();
		while (ii!=iiend)
		{
			if ((ii->first)!=slot)
				break;

			pe_params_part temp;
			temp.partid = (ii->second);
			temp.pos = pos*m_fScale;
			if (m_physic)
				m_physic->SetParams(&temp);

			++ii;
		}
	

		m_bRecalcBBox = true;
		return true;
	}
	
	return false;
}

bool CEntity::GetObjectAngles(unsigned int slot,Vec3d &ang)
{
	if(m_objects.size()>slot)
	{
		
		ang=m_objects[slot].angles;
		return true;
	}
	return false;
}

bool CEntity::SetObjectAngles(unsigned int slot,const Vec3d &ang)
{
	if(m_objects.size()>slot)
	{
		m_objects[slot].angles=ang;

		IntToIntMap::iterator ii = m_mapSlotToPhysicalPartID.find(slot),iiend = m_mapSlotToPhysicalPartID.end();
		while (ii!=iiend)
		{
			if ((ii->first)!=slot)
				break;

			pe_params_part temp;
			temp.partid = (ii->second);
			temp.q=GetRotationAA(ang.z*(gf_PI/180.0f), vectorf(0, 0, 1))*GetRotationAA(ang.y*(gf_PI/180.0f), vectorf(0, 1, 0))*
						GetRotationAA(ang.x*(gf_PI/180.0f), vectorf(1, 0, 0)); 
			if (m_physic)
				m_physic->SetParams(&temp);

			++ii;
		}
		return true;
	}
	return false;
}

//draw a specified entity's object
//////////////////////////////////////////////////////////////////////
void CEntity::DrawObject( unsigned int slot,int mode)
{
	if (slot >= m_objects.size())
		return;
	
	if (m_objects[slot].object)
	{
		if (mode==ETY_DRAW_NORMAL)
			m_objects[slot].flags |= ETY_OBJ_INFO_DRAW;
		else if (mode==ETY_DRAW_NEAR)
			m_objects[slot].flags  |= ETY_OBJ_INFO_DRAW_NEAR;
		else
			m_objects[slot].flags &= ~ETY_OBJ_INFO_DRAW;
	}
}

void CEntity::DrawObject(int mode)
{
	std::vector<CEntityObject>:: iterator it;
	int i = 0;

	for (it=m_objects.begin(); it!=m_objects.end(); it++) 
		DrawObject(i++,mode);
}

#include "IStatObj.h"

//calc the bbox for the entity
//////////////////////////////////////////////////////////////////////
void CEntity::CalcWholeBBox()
{
	// if this is a no geometry entity, do not calculate bbox (it will be probably set)
	if (m_objects.empty() && !m_pCryCharInstance[PLAYER_MODEL_IDX]) 
  {
    // [vlad] particle system need to be registered for visibility detection
    // maybe this is wrong place to handle such entities
    UnregisterInSector();
	
		if (IsMinBB(m_vBoxMax) || IsMaxBB(m_vBoxMin))
		{
			// [marco] check if the bounding box hasn't been set from outside by an
			// entity without geometry; in that case
			m_vBoxMin.Set(0,0,0);
			m_vBoxMax.Set(0,0,0);
		}
    RegisterInSector();
		
		if (m_bTrackColliders)
			CreatePhysicsBBox();

    return;
  }

  // [vlad] we need to reregister entity if bbox changes
  UnregisterInSector();

	if (m_physic && m_physic->GetType()==PE_SOFT)
	{
		pe_params_bbox pbb;
		m_physic->GetParams(&pbb);
		m_vBoxMin = pbb.BBox[0]-m_center;
		m_vBoxMax = pbb.BBox[1]-m_center;
	}
	else if (!m_bForceBBox)
	{
	
		std::vector<CEntityObject>::iterator it;

		m_vBoxMin=SetMaxBB();
		m_vBoxMax=SetMinBB();

		m_fRadius = 0;

		Vec3	rotation = m_angles;
		if( m_flags&ETY_FLAG_CALCBBOX_ZROTATE )
			rotation.x = rotation.y = 0.0f;

		//OPTIMISED_BY_IVO  
		Matrix44 matEntity=Matrix34::CreateRotationXYZ( Deg2Rad(rotation),m_center);
		matEntity=GetTransposed44(matEntity);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

		// [marco] in case we have a character, do not calculate the bbox using the
		// attached objects but only the character geometry
		// [kirill] except when the ETY_FLAG_CALCBBOX_USEALL flag is set - then use objects and character
		// for bbox calculations
		if ( !m_pCryCharInstance[PLAYER_MODEL_IDX] || (m_flags&ETY_FLAG_CALCBBOX_USEALL) )
 		for (it=m_objects.begin(); it<m_objects.end(); it++)
		{
			CEntityObject eo = (*it);
			IStatObj *co = (eo).object;

			if (!co)
				continue;

			//OPTIMISED_BY_IVO  
			Matrix44 matObject=Matrix34::CreateRotationXYZ( Deg2Rad(eo.angles),eo.pos);
			matObject=GetTransposed44(matObject);	//[ivo] TODO: remove this after E3 and use Matrix34 instead of Matrix44

			matObject = matObject * matEntity;

			// get object space bbox
  		Vec3d mins,maxs;
			mins = co->GetBoxMin()*m_fScale;
			maxs = co->GetBoxMax()*m_fScale;

			// extend this entity's bounding box, by transformed bounding box of object
			AABB aabb(mins, maxs);
			aabb.Transform(matObject);
			m_vBoxMin.CheckMin(aabb.min);
			m_vBoxMax.CheckMax(aabb.max);
		} 

		//fixme
		//[kirill] - it's bad, has to be changed - vehicles have character with angels/position different from 
		//	entity angles/position
		//	has to be stored somewere or retrived from entity
		//	for now - just enlarge BBox up - make sure the weapon on top of vehicle fits
		if ( m_flags&ETY_FLAG_CALCBBOX_USEALL )
		{
			m_vBoxMax.z += 1.5f;
		}

		if (m_pCryCharInstance[PLAYER_MODEL_IDX])
		// [marco] if this check is enabled , nothing will work
		// because the radius for 1st person mode will never be set
		//if (m_pCryCharInstance[PLAYER_MODEL_IDX]->GetFlags() & CS_FLAG_DRAW_MODEL)
		{
  		Vec3d mins,maxs;
			m_pCryCharInstance[PLAYER_MODEL_IDX]->GetBBox(mins,maxs);
			mins*=m_fScale;
			maxs*=m_fScale;

			{ 
				// re-make matrix ignoring x and y components of angles since it is used by bones system directly
				Matrix44 matEntity;

				if( m_bIsBound )
				{
				// take parent's matrix
					matEntity = m_matParentMatrix;
					Vec3d offs = matEntity.TransformVectorOLD(m_realcenter);
					matEntity.AddTranslationOLD( offs );
					Matrix44 rot;
//					rot.SetRotationXYZ44( DEG2RAD(Vec3d(0,0,180)), Vec3d(0,0,0));
//					rot.SetRotationXYZ44( DEG2RAD(-m_realangles), Vec3d(0,0,0));
					rot.SetRotationZYX( DEG2RAD(-m_realangles), Vec3d(0,0,0));
					matEntity = rot*matEntity;
				}
				else
				{
//					matEntity = Matrix34::GetRotationXYZ34( Deg2Rad(Vec3d(0,0,m_angles.z)),m_center);
					matEntity = Matrix34::CreateRotationXYZ( Deg2Rad(rotation),m_center);
					matEntity=GetTransposed44(matEntity);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44
				}

				// extend this entity's bounding box, by transformed bounding box of character
				AABB aabb(mins, maxs);
				aabb.Transform(matEntity);
				m_vBoxMin.CheckMin(aabb.min);
				m_vBoxMax.CheckMax(aabb.max);
			}
		}
		m_fRadius = (m_vBoxMax-m_vBoxMin).Length()*0.5f;

		m_vBoxMin-=m_center;
		m_vBoxMax-=m_center;
	
	}
	else
	{
		// [marco] orient the forced bbox

		//OPTIMISED_BY_IVO  
		Matrix44 matEntity=Matrix34::CreateRotationXYZ( Deg2Rad(m_angles),m_center);
		matEntity=GetTransposed44(matEntity);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44

  	Vec3d mins=m_vForceBBoxMin;mins*=m_fScale;
		Vec3d maxs=m_vForceBBoxMax;maxs*=m_fScale;		

		m_vBoxMin=SetMaxBB();
		m_vBoxMax=SetMinBB();
		
		// extend this entity's bounding box, by transformed bounding box of character
		AABB aabb(mins, maxs);
		aabb.Transform(matEntity);
		m_vBoxMin.CheckMin(aabb.min);
		m_vBoxMax.CheckMax(aabb.max);

		m_fRadius = (m_vBoxMax-m_vBoxMin).Length()*0.5f;

		m_vBoxMin-=m_center;
		m_vBoxMax-=m_center;
	}

	// [marco] set physics callback, if not set by script
	// from outside (see beginning of this function)
	if (m_bTrackColliders)
		CreatePhysicsBBox();

  RegisterInSector();
}

//////////////////////////////////////////////////////////////////////////
//Timur[8/6/2002]
namespace
{
	void TransformBBox( const Matrix44 &tm,Vec3d &min,Vec3d &max )
	{
		Vec3d m = tm.TransformPointOLD( min );
		Vec3d vx = Vec3d(tm[0][0],tm[0][1],tm[0][2])*(max.x-min.x);
		Vec3d vy = Vec3d(tm[1][0],tm[1][1],tm[1][2])*(max.y-min.y);
		Vec3d vz = Vec3d(tm[2][0],tm[2][1],tm[2][2])*(max.z-min.z);
		min = m;
		max = m;
		if (vx.x < 0) min.x += vx.x; else max.x += vx.x;
		if (vx.y < 0) min.y += vx.y; else max.y += vx.y;
		if (vx.z < 0) min.z += vx.z; else max.z += vx.z;

		if (vy.x < 0) min.x += vy.x; else max.x += vy.x;
		if (vy.y < 0) min.y += vy.y; else max.y += vy.y;
		if (vy.z < 0) min.z += vy.z; else max.z += vy.z;

		if (vz.x < 0) min.x += vz.x; else max.x += vz.x;
		if (vz.y < 0) min.y += vz.y; else max.y += vz.y;
		if (vz.z < 0) min.z += vz.z; else max.z += vz.z;
	}
};

//////////////////////////////////////////////////////////////////////////
//Timur[8/6/2002]
void CEntity::GetLocalBBox( Vec3d &min,Vec3d &max )
{
	// if this is a no geometry entity, do not calculate bbox (it will be probably set)
	if (m_objects.empty() && !m_pCryCharInstance[PLAYER_MODEL_IDX]) 
  {
		min = m_vBoxMin;
		max = m_vBoxMax;
    return;
  }

	if (m_bForceBBox) 
	{
		min = m_vBoxMin;
		max = m_vBoxMax;
    return;
  }

	std::vector<CEntityObject>::iterator it;

	min=SetMaxBB();
	max=SetMinBB();

 	for (it=m_objects.begin(); it<m_objects.end(); it++)
	{
		CEntityObject eo = (*it);
		IStatObj *co = (eo).object;

		if (!co)
			continue;



    // make matrix to transform bbox from object space into into world space
    //Matrix44 matObject;
		//matObject.Identity();
		//matObject=GetTranslationMat(eo.pos)*matObject;
		//matObject=GetRotationZYX44(-gf_DEGTORAD*eo.angles)*matObject; //NOTE: angles in radians and negated 

		//OPTIMISED_BY_IVO  
		Matrix44 matObject=Matrix34::CreateRotationXYZ( Deg2Rad(eo.angles),eo.pos);
		matObject=GetTransposed44(matObject);	//TODO: remove this after E3 and use Matrix34 instead of Matrix44



    // get object space bbox
  	Vec3d mins,maxs;
		mins = co->GetBoxMin();
		maxs = co->GetBoxMax();

		TransformBBox( matObject,mins,maxs );
		min.CheckMin( mins );
		max.CheckMax( maxs );
	} 

  if (m_pCryCharInstance[PLAYER_MODEL_IDX])
  {
  	Vec3d mins,maxs;
    m_pCryCharInstance[PLAYER_MODEL_IDX]->GetBBox(mins,maxs);

		min.CheckMin( mins );
		max.CheckMax( maxs );
  }
}

//rigid body creation
//////////////////////////////////////////////////////////////////////
bool CEntity::CreateRigidBody(pe_type type, float density,float mass,int surface_id, Vec3d* pInitialVelocity, int slot, bool bPermanent)
{
	std::vector<CEntityObject>::iterator it;
	int i;

	if (!m_pEntitySystem->m_pOnDemandPhysics->GetIVal())
		bPermanent = true;

	if (!bPermanent) 
	{
		if (m_physic)	
		{
			m_iPhysType = PHYS_NONE;
			m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
			m_physic = 0;
		}
		m_PhysData.RigidBody.type = type;
		m_PhysData.RigidBody.mass = mass;
		m_PhysData.RigidBody.density = density;
		m_PhysData.RigidBody.surface_idx = surface_id;
		m_PhysData.RigidBody.slot = slot;
		if (!m_physPlaceholder)
		{
			pe_params_bbox pbb;
			pbb.BBox[0] = m_vBoxMin+m_center;
			pbb.BBox[1] = m_vBoxMax+m_center;
			m_physPlaceholder = m_pISystem->GetIPhysicalWorld()->CreatePhysicalPlaceholder(type,&pbb,(IEntity*)this,0,m_nID);
			pe_params_foreign_data pfd;
			pfd.pForeignData = (IEntity*)this;
			pfd.iForeignData = 0;
			pfd.iForeignFlags = 0;
			m_physPlaceholder->SetParams(&pfd);
		}
		pe_params_pos pp;
		pp.iSimClass = 2;
		m_physPlaceholder->SetParams(&pp);
		m_iPhysType = PHYS_RIGID;
		if (!pInitialVelocity)
			return true;
	}

	m_flags|=ETY_FLAG_CALC_PHYSICS;
	m_flags|=ETY_FLAG_CLIENT_ONLY;
	m_flags|=ETY_FLAG_RIGIDBODY;

	pe_params_pos bodypos;
	bodypos.pos = m_center;
	bodypos.q = GetRotationAA(m_angles.z*(gf_PI/180.0f),vectorf(0,0,1))*GetRotationAA(m_angles.y*(gf_PI/180.0f),vectorf(0,1,0))*
		GetRotationAA(m_angles.x*(gf_PI/180.0f),vectorf(1,0,0));

	if (m_physic)
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
	m_physic = m_pISystem->GetIPhysicalWorld()->CreatePhysicalEntity(type,5.0f,&bodypos,(IEntity*)this,0,m_nID,m_physPlaceholder);

	i = 0;
	if(slot<0)
	{
		for(it=m_objects.begin(); it!=m_objects.end(); it++)
		{
			if ((* it).object)
			{
				pe_geomparams partpos;
				partpos.pos = (* it).pos /*- m_center*/;
				//partpos.q = quaternionf(vectorf((* it).angles*(gf_PI/180.0f)));
				partpos.q.SetRotationXYZ( vectorf((* it).angles*(gf_PI/180.0f)) );

				partpos.density = density;
				partpos.mass = mass;
				if (surface_id>=0)
					partpos.surface_idx = surface_id;
				partpos.scale = m_fScale;
				partpos.flags = geom_collides | geom_floats;
				m_physic->AddGeometry((* it).object->GetPhysGeom(), &partpos);

				if ((* it).object->GetPhysGeom(1))
				{
					partpos.density = partpos.mass = 0;
					partpos.flags = geom_colltype_ray;
					partpos.flagsCollider = 0;
					m_physic->AddGeometry((* it).object->GetPhysGeom(1), &partpos);		
				}
			}
			i++;
		}
	}
	else	
		if(m_objects[slot].object)
		{
			pe_geomparams partpos;
			partpos.pos = m_objects[slot].pos /*- m_center*/;
			partpos.q = GetRotationAA(m_objects[slot].angles.z*(gf_PI/180.0f),vectorf(0,0,1))*
				GetRotationAA(m_objects[slot].angles.y*(gf_PI/180.0f),vectorf(0,1,0)) * GetRotationAA(m_objects[slot].angles.x*(gf_PI/180.0f),vectorf(1,0,0));
			if (density!=0)	partpos.density = density;
			else partpos.mass = mass;
			if (surface_id>=0)
				partpos.surface_idx = surface_id;
			partpos.scale = m_fScale;
			m_physic->AddGeometry(m_objects[slot].object->GetPhysGeom(), &partpos, slot);
		}


	if(pInitialVelocity)
	{
		// Apply the initial velocity to the object
		pe_action_impulse ai;
		ai.impulse = *pInitialVelocity*mass;
		ai.iApplyTime = 0;
		m_physic->Action(&ai);
	}

	pe_params_buoyancy pb;
	pb.waterDensity = 1000.0f;
	pb.waterPlane.n.Set(0,0,1);
	//pb.waterPlane.origin.set(0,0,m_pISystem->GetI3DEngine()->GetWaterLevel(&m_center));
	pb.waterPlane.origin.Set(0,0,m_pISystem->GetI3DEngine()->GetWaterLevel(this));
	m_physic->SetParams(&pb);

	pe_params_flags pf;
	pf.flagsOR = pef_monitor_state_changes;
	m_physic->SetParams(&pf);

	m_awakeCounter = 4;
	// Timur[19/11/2003] This should not be here // SetNeedUpdate( true );

	return true;
}


bool CEntity::CreateLivingEntity(float mass, float height, float eye_height, float sphere_height, float radius,int nSurfaceID, float fGravity,float fAirControl, bool collide)
{

	IPhysicalWorld *pWorld = m_pISystem->GetIPhysicalWorld();
	DestroyPhysics();
	m_physicEnabled = true;

	pe_player_dimensions dim;
	pe_player_dynamics dyn;

	dim.heightEye = eye_height;
	dim.heightPivot  = 0;
	dim.heightCollider = sphere_height;
	dim.sizeCollider.Set(radius,radius,radius);

	dyn.gravity = fGravity;
	dyn.kAirControl = fAirControl;
	dyn.kInertia = 0;
	dyn.mass = mass;
	dyn.surface_idx=nSurfaceID;
	pe_params_pos parpos;

	parpos.pos = m_center;
	parpos.q = GetRotationAA(DEG2RAD(m_angles.z), vectorf(0,0,1));

	//if (m_physic)
	//	pWorld->DestroyPhysicalEntity(m_physic);
	m_physic = pWorld->CreatePhysicalEntity(PE_LIVING,&parpos,this,0,m_nID);
	m_physic->SetParams(&dim);
	m_physic->SetParams(&dyn);

	for (unsigned int i=0;i<m_objects.size();i++)
	{
		IStatObj * pObj = m_objects[i].object;

		pe_geomparams params;
		params.surface_idx = 0;//surface_idx;
		if( collide )
			params.flags = geom_collides;
		else
			params.flags = 0;//geom_proxy;
		params.scale = m_fScale;

    if(pObj && pObj->GetPhysGeom())
		  m_physic->AddGeometry(pObj->GetPhysGeom(), &params, i);		
	}

	m_flags |= ETY_FLAG_CALC_PHYSICS;

	// clear RigidBody flag.
	m_flags &= ~ETY_FLAG_RIGIDBODY;

	return true;
	
}

//load a model for this entity
//////////////////////////////////////////////////////////////////////
bool CEntity::LoadCharacter( int pos,const char *filename )
{
	CHECK_CHARACTER_SLOT_0( "CEntity::LoadCharacter" );
//  m_pISystem->GetI3DEngine()->RemoveCharacter(m_pCryCharInstance[pos]);

	ICryCharInstance *prevChar = m_pCryCharInstance[pos];
	if (prevChar)
	{
		if (prevChar->IsModelFileEqual(filename))
		{
			// Trying to load already load character, then silently ignore it.
			prevChar->ResetAnimations();
			// [Anton] it is essential that when a character is loaded (and later physicalized) that
			// both cases (i.e. model loading and model reusing) act the same and create character in the same default
			// position. ResetAnimations is not enough to achieve that.
			prevChar->StartAnimation("default",0.0f);
			prevChar->Update();
			return true;
		}
	}

	//@FIXME put range check here.
	m_pHeadBone = 0;
	ReleaseLipSyncInterface();	// we release lipsync before we destroy the character...
  m_pCryCharInstance[pos]=NULL;       

//  CLog::Log("Loading character: %s ...", filename);

	// First Create new character.
  m_pCryCharInstance[pos] = m_pISystem->GetIAnimationSystem()->MakeCharacter(filename);
	// After that delete previous chracter.
	if (prevChar)
		m_pISystem->GetIAnimationSystem()->RemoveCharacter(prevChar);
//	m_pCryCharInstance[pos]->SetCharInstanceSink(this);
	if(m_pCryCharInstance[pos])
  {
		m_bUpdateCharacters = true;
		if(m_nMaxCharNum>=pos)
			m_nMaxCharNum=pos+1;

	  m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() | CS_FLAG_DRAW_MODEL);
	  m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() | CS_FLAG_UPDATE);

		  
		m_pHeadBone = m_pCryCharInstance[0]->GetBoneByName("Bip01 Head");
  }
	
	m_pISystem->GetI3DEngine()->FreeEntityRenderState(this);
	InitEntityRenderState();

	// leave this call... it will cause the geometry to be properly registered
	CalcWholeBBox();

  return m_pCryCharInstance[pos]!=0;
}

bool CEntity::PhysicalizeCharacter(int pos, float mass,int surface_idx,float stiffness_scale, bool bInstant)
{
	CHECK_CHARACTER_SLOT_0( "CEntity::PhysicalizeCharacter" );

	if (!m_pCryCharInstance[pos])// || !m_physic)
		return false;
	surface_idx = -1; // [Anton] force to "ignore" value, since now surface idx from geometry should always be used

	// This is for testing only (at least now): to find out if anybody starts animations after the character dies
	if (m_pCryCharInstance[pos])
		m_pCryCharInstance[pos]->EnableStartAnimation(true);

	if (!m_physic || m_physic->GetType()!=PE_LIVING)
	{
		if (m_physPlaceholder)
		{	// if the entity is physicalized for on-demand usage (via placeholder), rephysicalize it as permanent
			m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physPlaceholder);
			m_physPlaceholder = 0;
			CreatePhysicalEntityCallback(0);
		}
		bInstant = true;
	}
	if (!m_pEntitySystem->m_pOnDemandPhysics->GetIVal())
		bInstant = true;

	if (m_physic && m_physic->GetType()==PE_LIVING)
	{
		pe_params_part pp;
		pp.ipart = 0;
		pp.flagsAND = ~geom_colltype_ray;
		pp.bRecalcBBox = 0;
		m_physic->SetParams(&pp);
	}

	if (!bInstant) 
	{
		m_pCryCharInstance[pos]->DestroyCharacterPhysics();
		m_charPhysData[pos].mass = mass;
		m_charPhysData[pos].surface_idx = surface_idx;
		m_charPhysData[pos].stiffness_scale = stiffness_scale;
		if (!m_pCharPhysPlaceholders[pos])
		{
			pe_params_foreign_data pfd;
			pfd.pForeignData = this;
			pfd.iForeignData = 0;
			pfd.iForeignFlags = (pos+8)<<12;
			m_pCharPhysPlaceholders[pos] = m_pISystem->GetIPhysicalWorld()->CreatePhysicalPlaceholder(PE_ARTICULATED,&pfd);
		}
		m_pCryCharInstance[pos]->SetCharacterPhysParams(mass,surface_idx);
		return true;
	}
	IPhysicalEntity *physic = GetPhysics();

	if (!physic || physic->GetType()==PE_LIVING)
	{
		int nAux = m_pCryCharInstance[pos]->CreateAuxilaryPhysics(
			m_pCryCharInstance[pos]->CreateCharacterPhysics(physic, mass,surface_idx,stiffness_scale));
		if (!physic) 
		{
			pe_params_pos pp;
			pp.pos = m_center;
			pp.q.SetRotationXYZ(m_angles*(gf_PI/180.0f));
			pe_params_foreign_data pfd;
			pfd.pForeignData = this;
			pfd.iForeignData = 0;
			pfd.iForeignFlags = 0;
			for (nAux--;nAux>=0;nAux--)
			{
				IPhysicalEntity *paux = m_pCryCharInstance[pos]->GetCharacterPhysics(nAux);
				if (paux)
				{
					paux->SetParams(&pp);
					paux->SetParams(&pfd);
				}
			}
		}
	}
	else
	{
		m_pCryCharInstance[pos]->BuildPhysicalEntity(physic, mass,surface_idx,stiffness_scale);
		m_pCryCharInstance[pos]->CreateAuxilaryPhysics(physic);
		SetPhysAngles(m_angles);
	}
	
	return true;
}

void CEntity::KillCharacter(int pos)
{
	CHECK_CHARACTER_SLOT( "CEntity::KillCharacter" );

	if (!m_pCryCharInstance[pos])
		return;

	// This is for testing only (at least now): to find out if anybody starts animations after the character dies
	ICryCharInstance* pChar = m_pCryCharInstance[pos];
	if (pChar)
	{
		pChar->EnableStartAnimation(false);
		pChar->FreezeAllMorphs();
		int nMorph = pChar->GetModel()->GetAnimationSet()->FindMorphTarget("#rndexpr_closed_eyes");
		if (nMorph >= 0)
		{
			CryCharMorphParams Params;
			Params.fLength = 30758400.0f; // enough for 1 year
			Params.nFlags = Params.FLAGS_NO_BLENDOUT;
			pChar->StartMorph(nMorph, Params);
		}
		// to avoid the character breathing after death, stop all animations
		pChar->ResetAnimations();
	}

	if (!m_physic || m_physic->GetType()==PE_LIVING) 
	{
		pe_status_dynamics sd;
		pe_status_pos sp;
		sd.v.Set(0,0,0);
		if (m_physic)
		{
			m_physic->GetStatus(&sd);
			m_physic->GetStatus(&sp);
		}
		else
			sp.q.SetRotationXYZ( m_angles*(gf_PI/180.0f) );
		DestroyPhysics();
		m_physicEnabled = true;
		m_flags |= ETY_FLAG_CALC_PHYSICS;
		/*pe_status_dynamics sd;
		if (m_physic) 
		{
			m_physic->GetStatus(&sd);
			m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
		}*/

		m_physic = m_pCryCharInstance[pos]->RelinquishCharacterPhysics();
		if (m_physic) 
		{
			m_pISystem->GetIPhysicalWorld()->SetPhysicalEntityId(m_physic,m_nID);

			pe_params_pos pp;
			pp.pos = m_center;
			//pp.q = quaternionf(m_angles*(gf_PI/180.0f));
			pp.q = sp.q; //.SetRotationXYZ( m_angles*(gf_PI/180.0f) );
			m_physic->SetParams(&pp);

			/*pe_params_articulated_body pab;
			pab.v = sd.v;
			pab.w = sd.w;
			m_physic->SetParams(&pab);*/

			pe_params_foreign_data pfd;
			pfd.iForeignData = 0;
			pfd.pForeignData = this;
			m_physic->SetParams(&pfd);

			pe_params_buoyancy pb;
			pb.waterDensity = 1000.0f;
			pb.waterPlane.n.Set(0,0,1);
			pb.waterPlane.origin.Set(0,0,m_pISystem->GetI3DEngine()->GetWaterLevel(this));
			m_physic->SetParams(&pb);

			pe_params_flags pf;
			pf.flagsAND = ~pef_pushable_by_players;
			pf.flagsOR = pef_monitor_state_changes|pef_always_notify_on_deletion;
			m_physic->SetParams(&pf);

			pe_params_part ppart;
			ppart.flagsAND = ~geom_colltype_player;
			ppart.ipart = -1;
			do { ++ppart.ipart; } while(m_physic->SetParams(&ppart));

			if (sd.mass<500)
			{
				pe_action_set_velocity asv;
				asv.v = sd.v;
				m_physic->Action(&asv);
			}
			m_bIsADeadBody = 1;
			m_awakeCounter = 4;
			// Timur[19/11/2003] This should not be here //SetNeedUpdate( true );
		}

		if (m_pCharPhysPlaceholders[pos])
		{
			m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_pCharPhysPlaceholders[pos]);
			m_pCharPhysPlaceholders[pos] = 0;
		}
	}
}

ILipSync* CEntity::GetLipSyncInterface()
{
	if (!m_pLipSync)
	{
		assert(m_pISystem);
		assert(m_pISystem->GetIGame());

		m_pLipSync = new CLipSync;
		if ( m_pLipSync!=NULL && !m_pLipSync->Init(m_pISystem, this))
		{
			ReleaseLipSyncInterface();
			return NULL;
		}
	}
	return m_pLipSync;
}

void CEntity::ReleaseLipSyncInterface()
{
	if (m_pLipSync)
	{
		m_pLipSync->SetCallbackSink(NULL);
		m_pLipSync->Release();
		m_pLipSync=NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
void CEntity::SetCharacter( int pos,ICryCharInstance *character )
{
	assert(pos>=0 && pos<MAX_ANIMATED_MODELS);
	
	if(pos>=0 && pos<MAX_ANIMATED_MODELS)
		m_pCryCharInstance[pos] = character;
}


//////////////////////////////////////////////////////////////////////////
ICryCharInstance* CEntity::GetCharacter( int pos )
{
	CHECK_CHARACTER_SLOT_0( "CEntity::GetCharacter" );

	if(pos>=0 && pos<MAX_ANIMATED_MODELS)
		return m_pCryCharInstance[pos];
		
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::DrawCharacter( int pos,int mode )
{
	CHECK_CHARACTER_SLOT( "CEntity::DrawCharacter" );

	if (!m_pCryCharInstance[pos])
		return;

	if (mode == ETY_DRAW_NORMAL )
	{
		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() | CS_FLAG_DRAW_MODEL);
		NeedsUpdateCharacter( pos, true);
//		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() | CS_FLAG_UPDATE);
	}
	else if(mode == ETY_DRAW_NEAR )
	{
		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() | CS_FLAG_DRAW_MODEL);
		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() | CS_FLAG_DRAW_NEAR);
		NeedsUpdateCharacter( pos, true);
//		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() | CS_FLAG_UPDATE);
	}
	else
	{
		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() & ~CS_FLAG_DRAW_MODEL);
		NeedsUpdateCharacter( pos, false);
//		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() & ~CS_FLAG_UPDATE);
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntity::NeedsUpdateCharacter( int pos, bool updt )
{
	CHECK_CHARACTER_SLOT( "CEntity::NeedsUpdateCharacter" );

	if (!m_pCryCharInstance[pos])
		return;
	if( updt )
		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() | CS_FLAG_UPDATE);
	else
		m_pCryCharInstance[pos]->SetFlags(m_pCryCharInstance[pos]->GetFlags() & ~CS_FLAG_UPDATE);
}


//
////////////////////////////////////////////////////////////////////////
float CEntity::GetAnimationLength( const char *animation )
{
int pos = 0;
  if(m_pCryCharInstance[pos])
  {
		return	m_pCryCharInstance[pos]->GetModel()->GetAnimationSet()->GetLength(animation);
	}
	return	0.0f;
}

//scale animation speed
////////////////////////////////////////////////////////////////////////
void CEntity::SetAnimationSpeed( const float scale )
{
int pos = 0;
  if(m_pCryCharInstance[pos])
  {
		m_pCryCharInstance[pos]->SetAnimationSpeed( scale );
	}
}


//start a character animation
////////////////////////////////////////////////////////////////////////
bool CEntity::StartAnimation( int pos,const char *animation, int iLayerID, float fBlendTime, bool bStartWithLayer0Phase )
{
	CHECK_CHARACTER_SLOT_0( "CEntity::StartAnimation" );

    if(m_pCryCharInstance[pos])
	{
		if( animation )
		{
			CryCharAnimationParams ccap;
			ccap.fBlendInTime = ccap.fBlendOutTime = fBlendTime;
			ccap.nLayerID = iLayerID;
			ccap.nFlags = bStartWithLayer0Phase ? ccap.FLAGS_SYNCHRONIZE_WITH_LAYER_0: 0 ;
			return m_pCryCharInstance[pos]->StartAnimation(animation,ccap);
//			[PETAR] Historic call for reference:
//			return m_pCryCharInstance[pos]->StartAnimation2(animation, fBlendTime, fBlendTime, iLayerID, 
//																									true, bStartWithLayer0Phase);
		}
		else
			return m_pCryCharInstance[pos]->StopAnimation( iLayerID );
	}

  return false;
}

//force uapdate character
////////////////////////////////////////////////////////////////////////
void CEntity::ForceCharacterUpdate( int pos)
{
	if ((pos < 0) || (pos > MAX_ANIMATED_MODELS))
		return ;

	if(m_pCryCharInstance[pos])
		m_pCryCharInstance[pos]->ForceUpdate();
}


//! Return current animation ( Return -1 if animations stoped )
int CEntity::GetCurrentAnimation(int pos, int iLayerID)
{
	if ((pos < 0) || (pos > MAX_ANIMATED_MODELS))\
	{
		m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,0,"<CryEntitySystem> Invalid slot number for method CEntity::GetCurrentAnimation" );
		return -1;
	}

	if(!m_pCryCharInstance[pos])
		return(-1);		// this should never be

	return m_pCryCharInstance[pos]->GetCurrentAnimation( iLayerID );
}


//check if animation present
////////////////////////////////////////////////////////////////////////
bool CEntity::IsAnimationPresent( int pos,const char * szAnimation )
{
	CHECK_CHARACTER_SLOT_0( "CEntity::IsAnimationPresent" );

	//@FIXME put range check here.
	// the Find funciton in the animation set returns -1 if it didn't find the animation
	// with such a name, and a non-negative number if it did
	return m_pCryCharInstance[pos]
		&& m_pCryCharInstance[pos]->GetModel()->GetAnimationSet()->Find(szAnimation) >= 0;
}

//////////////////////////////////////////////////////////////////////////
void CEntity::ResetAnimations( int pos )
{
	CHECK_CHARACTER_SLOT( "CEntity::ResetAnimation" );

	if (m_pCryCharInstance[pos])
    m_pCryCharInstance[pos]->ResetAnimations();
	
}

//////////////////////////////////////////////////////////////////////////
void CEntity::SetDefaultIdleAnimation( int pos,const char * szAnimation )
{
	CHECK_CHARACTER_SLOT( "CEntity::ResetAnimation" );

	if (m_pCryCharInstance[pos])
		m_pCryCharInstance[pos]->SetDefaultIdleAnimation(pos, szAnimation?szAnimation:"" );
	
}


//////////////////////////////////////////////////////////////////////////
bool CEntity::LoadBoat(const char *objfile, float mass, int surfaceID)
{
int res = LoadObject(0,objfile,0,"boat");
	if(res)
	{
		if(LoadObject(1,objfile,0,"boat_hull"))
		{
//			m_pEntity->CreateRigidBody(PE_RIGID,0,fMass,nSurfaceID,NULL,1);
			CreateRigidBody(PE_RIGID,0,0,surfaceID,NULL,1);
			DrawObject(1, ETY_DRAW_NONE);

			if(LoadObject(2,objfile,0,"mass"))
			{
				pe_geomparams pparts;
				pparts.mass = mass;
				pparts.flags = 0;
				m_physic->AddGeometry(m_objects[2].object->GetPhysGeom(), &pparts, -1);
				DrawObject(2, ETY_DRAW_NONE);
				return true;
			}
			m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,0,"Can not load boat %s - no \"mass\" object", objfile);
			return false;
		}
		m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,0,"Can not load boat %s - no \"boat_hull\" object", objfile);
		return false;
	}
	m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,0,"Can not load boat %s - no \"boat\" object", objfile);
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::LoadVehicle(const char *objfile, pe_cargeomparams *pparts, pe_params_car *params,bool bDestroy)
{
	if (bDestroy)
	{
		// destroying this vehicle entity
		m_pISystem->GetILog()->Log("Destroying Vehicle: %s", objfile);
		// remove the objects
		m_objects.clear();
	}
	else
		// Load a vehilce for this entity
		m_pISystem->GetILog()->Log("Loading Vehicle: %s", objfile);
	
	EntityCameraParam sCamParam;
	DestroyPhysics();
	m_physicEnabled = true;

  m_flags|=ETY_FLAG_CALC_PHYSICS;
  //m_flags|=ETY_FLAG_CLIENT_ONLY;

	// clear RigidBody flag.
	m_flags &= ~ETY_FLAG_RIGIDBODY;

	//	m_center.Set(0, 0, 0);

	pe_params_pos bodypos; 
	bodypos.pos = m_center;
	//note: order of angles is flipped!
	//bodypos.q = quaternionf(m_angles.z*(PI/180.0f),m_angles.y*(PI/180.0f),m_angles.x*(PI/180.0f));
	bodypos.q.SetRotationXYZ( m_angles*(gf_PI/180.0f) );

	//if (m_physic)
	//	m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
	m_physic = m_pISystem->GetIPhysicalWorld()->CreatePhysicalEntity(PE_WHEELEDVEHICLE,&bodypos,(IEntity*)this,0,m_nID);

	I3DEngine *p3DEngine = m_pISystem->GetI3DEngine();

	unsigned int idx=0,widx=0,sidx;
	int bHasProxies=0,ihullvis=-1;
	IStatObj *cobj,*cobj_proxy;
	char geom_name[64];
	pe_params_pos hullpos;
	bool bReusePrevious = true;

	do 
	{
		sprintf(geom_name, "hull%d", idx+1);
		if (m_objects.size()>idx && m_objects[idx].object)
		{
			if (m_objects[idx].object->IsSameObject(objfile,geom_name))
				cobj = m_objects[idx].object;
			else if (bReusePrevious && idx>0)
				break;
			else
				p3DEngine->ReleaseObject(m_objects[idx].object);
		}
    else if (bReusePrevious && idx>0)
			break;
		else 
		{
			bReusePrevious = false;
			if (!(cobj = p3DEngine->MakeObject(objfile,geom_name)))
				break;
		}
			
		if (ihullvis<0 && pparts[idx].flags)
			ihullvis = idx;

		// the steering wheel is called "hull3" 
		// and therefore stored in the slot number 3...
		if (idx==2)		
			m_nSteeringWheelSlot=idx;

		sprintf(geom_name, "hullproxy%d", idx+1);
		if (bReusePrevious)
			cobj_proxy = m_auxObjects.size()>idx ? m_auxObjects[idx]:0;
		else
		{
			if (m_auxObjects.size()>idx && m_auxObjects[idx])
				p3DEngine->ReleaseObject(m_auxObjects[idx]);
			cobj_proxy = p3DEngine->MakeObject(objfile,geom_name);
		}

		if (idx >= m_objects.size())
			m_objects.resize( idx+1 );
		m_objects[idx].object = cobj;
		m_objects[idx].pos = Vec3(0,0,0); //m_center;

		if (cobj_proxy)
		{
			if (m_auxObjects.size()<=idx)
				m_auxObjects.resize( idx+1 );
			m_auxObjects[idx] = cobj_proxy;
		}

		int flags = pparts[idx].flags;
		if (pparts[idx].flags!=2) 
		{
			if (pparts[idx].flags==1) 
			{
				pparts[idx].flags = (geom_collides&~geom_colltype4) | geom_floats; // don't collide with destroyable objects
				if (bHasProxies && !cobj_proxy)
					pparts[idx].flags = 0;
			} 
			else
				pparts[idx].flagsCollider = 0;// |= geom_no_raytrace;

			m_physic->AddGeometry(cobj->GetPhysGeom(), pparts+idx, idx);

			if (cobj_proxy) 
			{
				pparts[idx].flags |= geom_proxy;
				m_physic->AddGeometry(cobj_proxy->GetPhysGeom(), pparts+idx, idx);
				bHasProxies = 1;
			}
		}
		pparts[idx].flags = flags;

		//p3DEngine->ReleaseObject(cobj_proxy);
		//if (m_objects[idx].object != cobj)
		//	p3DEngine->ReleaseObject(cobj);

		idx++;
	} while(true);

	if (idx==0)
	{
		m_pISystem->Warning( VALIDATOR_MODULE_ENTITYSYSTEM,VALIDATOR_WARNING,0,objfile,"CEntity::LoadVehicle: %s, %s not found",objfile,"hull1");
		return false;
	}

  Vec3d dummy_pos;
  IStatObj * pStatObj = m_objects[0].object;//p3DEngine->MakeObject((char*)objfile,"hull1");

	do 
	{
		sprintf(geom_name, "wheel%d_upper", widx+1);
		//if((dummy_pos = pStatObj->GetHelperPos(geom_name)) == Vec3d(0,0,0)) 
		if( IsEquivalent((dummy_pos=pStatObj->GetHelperPos(geom_name)),Vec3d(0,0,0)) ) 
		{
			sprintf(geom_name, "wheel_%d_upper", widx+1);
			//if((dummy_pos = pStatObj->GetHelperPos(geom_name)) == Vec3d(0,0,0))
			if(IsEquivalent((dummy_pos=pStatObj->GetHelperPos(geom_name)),Vec3d(0,0,0)))
				break;
		}
		pparts[idx+widx].pivot = vectorf(&dummy_pos.x); 
		sprintf(geom_name, "wheel%d_lower", widx+1);
	//	if ((dummy_pos = pStatObj->GetHelperPos(geom_name)) == Vec3d(0,0,0)) 
		if ( IsEquivalent((dummy_pos=pStatObj->GetHelperPos(geom_name)),Vec3d(0,0,0)) ) 
		{
			sprintf(geom_name, "wheel_%d_lower", widx+1);
			dummy_pos = pStatObj->GetHelperPos(geom_name);
		}
		pparts[idx+widx].lenInitial = pparts[idx+widx].pivot.z-dummy_pos.z;
		(pparts[idx+widx].pivot = dummy_pos).z -= pparts[idx+widx].lenInitial;
		widx++;
	} while (!(bDestroy && widx>2));
	
	widx=0;
	do 
	{
		sprintf(geom_name, "wheel%d", widx+1);
		if (m_objects.size()>idx+widx && m_objects[idx+widx].object)
		{
			if (bReusePrevious)
			{
				if (!strncmp(m_objects[idx+widx].object->GetGeoName(),"wheel",5))
					cobj = m_objects[idx+widx].object;
				else
					break;
			}
			else
				p3DEngine->ReleaseObject(m_objects[idx+widx].object);
		}
		else if (bReusePrevious && widx>0)
			break;
		else if (!(cobj = p3DEngine->MakeObject(objfile,geom_name))) 
		{
			sprintf(geom_name, "wheel_%d", widx+1);
			if (!(cobj = p3DEngine->MakeObject(objfile,geom_name)))
				break;
		}

		m_physic->AddGeometry(cobj->GetPhysGeom(), pparts+idx+widx, idx+widx);			

		if (idx+widx >= m_objects.size())
			m_objects.resize( idx+widx+1 );

		m_objects[idx+widx].object = cobj;
		m_objects[idx+widx].pos = Vec3d(0,0,0); // m_center;
		
		widx++;
	} while (!(bDestroy && widx>2));

	sidx=0;
	int iHelper,nLen,i,iPart[2];
	Vec3 vPos[2];
	const char *pName;

	do 
	{
		sprintf(geom_name, "susp%d", sidx+1);
		if (m_objects.size()>idx+widx+sidx && m_objects[idx+widx+sidx].object)
		{
			if (bReusePrevious)
			{
				if (m_objects[idx+widx+sidx].object->IsSameObject(objfile,geom_name))
					cobj = m_objects[idx+widx+sidx].object;
				else
					break;
			}
			else
				p3DEngine->ReleaseObject(m_objects[idx+widx+sidx].object);
		}
		else if (bReusePrevious)
			break;
		else if (!(cobj = p3DEngine->MakeObject(objfile,geom_name)))
			break;

		nLen = strlen(geom_name);
		for(iHelper=i=0; i<2 && (pName=pStatObj->GetHelperById(iHelper,vPos[i])); iHelper++) 
		if (!strncmp(pName,geom_name,nLen) && (int)strlen(pName)>nLen)
		{
			if (!strncmp(pName+nLen,"_hull",5))
				iPart[i] = ihullvis;
			else if (!strncmp(pName+nLen,"_wheel",6))
				iPart[i] = idx+atoi(pName+nLen+6)-1;
			else if (!strncmp(pName+nLen,"_susp",5))
				iPart[i] = idx+widx+atoi(pName+nLen+5)-1;
			else
				continue;
			i++;
		}
		if (i<2)
			continue;
		if (iPart[0]>iPart[1])
		{
			i=iPart[0]; iPart[0]=iPart[1]; iPart[1]=i;
			Vec3 t=vPos[0]; vPos[0]=vPos[1]; vPos[1]=t;
		}

		if (idx+widx+sidx >= m_objects.size())
			m_objects.resize( idx+widx+sidx+1 );

		m_objects[idx+widx+sidx].object = cobj;
		m_objects[idx+widx+sidx].flags = ETY_OBJ_IS_A_LINK;
		m_objects[idx+widx+sidx].ipart0 = iPart[0];
		m_objects[idx+widx+sidx].ipart1 = iPart[1];
		m_objects[idx+widx+sidx].link_start0 = vPos[0];
		m_objects[idx+widx+sidx].link_end0 = vPos[1];
		sidx++;
	} while (!(bDestroy && sidx>2));


	m_physic->SetParams(params);

	pe_params_buoyancy pb;
	pb.waterDensity = 1000.0f;
	pb.waterPlane.n.Set(0,0,1);
	pb.waterPlane.origin.Set(0,0,p3DEngine->GetWaterLevel());
	m_physic->SetParams(&pb);

	if (!bDestroy)
	{
		if (HaveCamera())
		{

			GetCamera()->GetParameters(&sCamParam);

			//if ((dummy_pos = pStatObj->GetHelperPos("driver_sit_pos")) != Vec3d(0,0,0))
			if (!IsEquivalent((dummy_pos=pStatObj->GetHelperPos("driver_sit_pos")),Vec3d(0,0,0)))
				sCamParam.m_1pcam_butt_pos = dummy_pos;

			//if ((dummy_pos = pStatObj->GetHelperPos("eye_pos")) != Vec3d(0,0,0))
			if (!IsEquivalent((dummy_pos=pStatObj->GetHelperPos("eye_pos")),Vec3d(0,0,0)))
				sCamParam.m_1pcam_eye_pos = dummy_pos;
			
			sCamParam.m_cam_kstiffness=50;
			sCamParam.m_cam_kdamping=(float)(2*cry_sqrtf(sCamParam.m_cam_kstiffness));
			sCamParam.m_cam_dir(0,-3,2); sCamParam.m_cam_dir.Normalize();
			sCamParam.m_cam_dist = 4.5;
			sCamParam.m_cur_cam_dist = 0;
			sCamParam.m_cur_cam_dangle = 0;
			sCamParam.m_cam_angle_flags = 1;
			sCamParam.m_cam_angle_kstiffness=6;
			sCamParam.m_cam_angle_kdamping=(float)(2*cry_sqrtf(sCamParam.m_cam_angle_kstiffness));

			GetCamera()->SetParameters(&sCamParam);
		}
		else		
		{
			// Set camera params.
			SetCamera( new CEntityCamera );
		}
	}

  //p3DEngine->ReleaseObject(pStatObj);
	DrawObject(ETY_DRAW_NORMAL);
	for(idx--;(int)idx>=0;idx--)
		if (pparts[idx].flags==0)
			DrawObject(idx,ETY_DRAW_NONE);

	p3DEngine->FreeEntityRenderState(this);
	InitEntityRenderState();

	// leave this call... it will cause the geometry to be properly registered
	CalcWholeBBox(); 

  return (true);
}

//////////////////////////////////////////////////////////////////////////////////
bool CEntity::CreateParticleEntity(float size,float mass, Vec3d velocity, float acc_thrust,float k_air_resistance,
								   float acc_lift,float gravity, int surface_idx,bool bSingleContact)
{
	//UnregisterInSector();
	DestroyPhysics();
	m_physicEnabled = true;

	m_flags|=ETY_FLAG_CALC_PHYSICS;
  m_flags|=ETY_FLAG_CLIENT_ONLY;
	// clear RigidBody flag.
	m_flags &= ~ETY_FLAG_RIGIDBODY;
	
	pe_params_pos partpos;
	partpos.pos = m_center;
	partpos.q.SetRotationXYZ( m_angles*(gf_PI/180.0f) );

	/*if (m_physic && m_physic->GetType()!=PE_PARTICLE) {
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
		m_physic = 0;
	}*/
	if (m_physic==0) 
		m_physic = m_pISystem->GetIPhysicalWorld()->CreatePhysicalEntity(PE_PARTICLE,&partpos,(IEntity*)this,0,m_nID);

	pe_params_particle params;
	
	params.size = size;
	params.mass = mass;
	params.velocity = velocity.Length();
	params.heading = velocity/params.velocity;
	params.accThrust = acc_thrust;
	params.kAirResistance = k_air_resistance;
	params.accLift = acc_lift;
	params.gravity.Set(0,0,gravity);
	params.surface_idx = surface_idx;
	params.flags = bSingleContact ? (particle_single_contact | particle_constant_orientation) : particle_constant_orientation;
	m_physic->SetParams(&params);
	
	
	return true;
}

//////////////////////////////////////////////////////////////////////////////////
bool CEntity::CreateStaticEntity(float mass, int surface_idx, int slotToUse, bool bPermanent)
{
	if (!m_pEntitySystem->m_pOnDemandPhysics->GetIVal())
		bPermanent = true;

	if (!bPermanent) 
	{
		if (m_physic)	
		{
			m_iPhysType = PHYS_NONE;
			m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
			m_physic = 0;
		}
		m_PhysData.Static.surface_idx = surface_idx;
		m_PhysData.Static.slot = slotToUse;
		if (!m_physPlaceholder)
		{
			pe_params_bbox pbb;
			pbb.BBox[0] = m_vBoxMin+m_center;
			pbb.BBox[1] = m_vBoxMax+m_center;
			m_physPlaceholder = m_pISystem->GetIPhysicalWorld()->CreatePhysicalPlaceholder(PE_STATIC,&pbb,(IEntity*)this,0,m_nID);
			pe_params_foreign_data pfd;
			pfd.pForeignData = this;
			pfd.iForeignData = 0;
			pfd.iForeignFlags = 0;
			m_physPlaceholder->SetParams(&pfd);
		}
		pe_params_pos pp;
		pp.iSimClass = 0;
		m_physPlaceholder->SetParams(&pp);
		m_iPhysType = PHYS_STATIC;
		return true;
	}

	//UnregisterInSector();

	m_flags|=ETY_FLAG_CALC_PHYSICS;
  m_flags|=ETY_FLAG_CLIENT_ONLY;
	// clear RigidBody flag.
	m_flags &= ~ETY_FLAG_RIGIDBODY;

	pe_params_pos partpos;
	partpos.pos = m_center;

	if (m_physic)
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
	m_physic = m_pISystem->GetIPhysicalWorld()->CreatePhysicalEntity(PE_STATIC,5.0f,&partpos,(IEntity*)this,0,m_nID);

	pe_params_pos temp;
	temp.pos = m_center;
	//temp.q = quaternionf(vectorf(m_angles*(gf_PI/180.0f)));
	temp.q.SetRotationXYZ( vectorf(m_angles*(gf_PI/180.0f)) );
	m_physic->SetParams(&temp);

	unsigned int firstObjIdx;
	unsigned int lastObjIdx;
	if(slotToUse<0)
	{
		firstObjIdx = 0;
		lastObjIdx = m_objects.size();
	}
	else
	{
		firstObjIdx = slotToUse;
		lastObjIdx = slotToUse+1;
		if(firstObjIdx>=m_objects.size())
			return false;
	}

//	for (unsigned int i=0;i<m_objects.size();i++)
	for (unsigned int i=firstObjIdx; i<lastObjIdx; i++)
	{
		IStatObj *pObj = m_objects[i].object;

		if (!pObj)
			continue;

		pe_geomparams params;
		if (surface_idx>=0)
			params.surface_idx = surface_idx;
		else if (surface_idx==-2)
			params.flags &= ~geom_colltype3;
		
		params.scale = m_fScale;
//	m_physic->SetParams((pe_params*)&params);

    if(pObj->GetPhysGeom())
		{
			//params.flags = geom_collides;
		  int partid = m_physic->AddGeometry(pObj->GetPhysGeom(), &params);		
		  m_mapSlotToPhysicalPartID.insert(IntToIntMap::iterator::value_type(i,partid));
		}

		// nType 0 is physical geometry, 1 - obstruct geometry. 
		// Obstruct geometry should not be used for collisions 
		// (geom_collides flag should not be set ). 
		// It could be that an object has only mat_obstruct geometry. 
		// GetPhysGeom(0) will return 0 - nothing to collide. 
		// GetPhysGeom(1) will return obstruct geometry, 
		// it should be used for AI visibility check and newer for collisions. 
		// the flag geom_collides should not be present in obstruct geometry physicalization. 		
		if (pObj->GetPhysGeom(1))
		{
			params.flags = geom_colltype_ray;
			params.flagsCollider = 0;
		  int partid = m_physic->AddGeometry(pObj->GetPhysGeom(1), &params);
		  m_mapSlotToPhysicalPartID.insert(IntToIntMap::iterator::value_type(i,partid));
		}
	}
	

	return true;
}


//////////////////////////////////////////////////////////////////////////////////
bool CEntity::CreateSoftEntity(float mass,float density, bool bCloth, IPhysicalEntity *pAttachTo,int iAttachToPart)
{
	if (m_physic)
		m_pISystem->GetIPhysicalWorld()->DestroyPhysicalEntity(m_physic);
	m_physic = 0;
	if (!m_pEntitySystem->m_pEnableCloth->GetIVal())
		return false;

	m_flags|=ETY_FLAG_CALC_PHYSICS;
  m_flags|=ETY_FLAG_CLIENT_ONLY;
	// clear RigidBody flag.
	m_flags &= ~ETY_FLAG_RIGIDBODY;

	pe_params_pos pp;
	pp.pos = m_center;
	pp.q.SetRotationXYZ( vectorf(m_angles*(gf_PI/180.0f)) );

	m_physic = m_pISystem->GetIPhysicalWorld()->CreatePhysicalEntity(PE_SOFT,5.0f,&pp,(IEntity*)this,0,m_nID);

	if (bCloth)
	{
		pe_params_flags pf;
		pf.flagsOR = se_skip_longest_edges;
		m_physic->SetParams(&pf);
	}

	if (m_objects.size()>0)
	{
		IStatObj * pObj = m_objects[0].object;

		pe_geomparams params;
		params.flags = 0;//geom_proxy;
		params.scale = m_fScale;
		params.mass = mass;
		params.density = density;

    if(pObj && pObj->GetPhysGeom())
		{
		  m_physic->AddGeometry(pObj->GetPhysGeom(), &params, 0);

			CLeafBuffer *pLB = pObj->GetLeafBuffer();
			strided_pointer<color4b> pColors(0);
			if ((pColors.data=(color4b*)pLB->GetColorPtr(pColors.iStride)) && pLB->m_arrVtxMap)
			{
				pe_action_attach_points aap;
				aap.nPoints = 0;
				aap.piVtx = new int[pLB->m_SecVertCount];

				for(int i=0;i<pLB->m_SecVertCount;i++) if (pColors[i].r==0)
					aap.piVtx[aap.nPoints++] = pLB->m_arrVtxMap[i];
				if (aap.nPoints)
				{
					aap.pEntity = pAttachTo;
					if (iAttachToPart>=0)
						aap.partid = iAttachToPart;
					m_physic->Action(&aap);
				}
				delete[] aap.piVtx;
			}
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CEntity::RegisterInAISystem(unsigned short type, const AIObjectParameters &params)
{

	IAISystem *pSystem = m_pISystem->GetAISystem();
	if (pSystem)
	{
		if (m_pAIObject)
		{
			pSystem->RemoveObject(m_pAIObject);
			m_pAIObject = 0;
			m_bUpdateAI = false;
		}

		if (type == 0)
			return true;

		if (m_pAIObject = pSystem->CreateAIObject(type, this))
		{
			m_bUpdateAI = true;
			m_pAIObject->ParseParameters(params);
			UpdateAIObject(false);
			return true;
		}
	}

	return false;
}


void CEntity::SetDamage(const int dmg)
{

/*
	for(int dIdx=0; dIdx<m_DmgTable.size(); dIdx++)
	{
int t0=m_DmgTable[dIdx].m_array[0];
int t1=m_DmgTable[dIdx].m_array[1];
		if(m_DmgTable[dIdx].m_array[1]>0)
		{
			DrawObject(	m_DmgTable[dIdx].m_array[0], ETY_DRAW_NONE);
			DrawObject(	m_DmgTable[dIdx].m_array[1], ETY_DRAW_NORMAL);
		}
	}		
*/
//	DrawObject(	1, ETY_DRAW_NONE);	
//	DrawObject(	11, ETY_DRAW_NORMAL);
	
}