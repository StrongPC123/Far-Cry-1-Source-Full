////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   bugsflock.cpp
//  Version:     v1.00
//  Created:     11/4/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BugsFlock.h"

#include <CryCharAnimationParams.h>

#define BUGS_SCARE_DISTANCE 3.0f

enum EBugsFlockBehaviors
{
	EBUGS_BUG,
	EBUGS_DRAGONFLY,
	EBUGS_FROG,
};

//! Return random value in [-1,1] range.
inline float frand()
{
	return ((float)rand()*2.0f / RAND_MAX) - 1.0f;
}

//////////////////////////////////////////////////////////////////////////
CBugsFlock::CBugsFlock( int id,CFlockManager *flockMgr )
:	CFlock( id,flockMgr )
{
}

//////////////////////////////////////////////////////////////////////////
CBugsFlock::~CBugsFlock()
{
	ReleaseObjects();
}

void CBugsFlock::ReleaseObjects()
{
	for (unsigned int i = 0; i < m_objects.size(); i++)
	{
		if (m_objects[i])
			m_bc.engine->ReleaseObject( m_objects[i] );
	}
	m_objects.clear();
}

//////////////////////////////////////////////////////////////////////////
void CBugsFlock::CreateBoids( SBoidsCreateContext &ctx )
{
	int i;

	ClearBoids();
	ReleaseObjects();

	AABB bbox;
	bbox.min.Set(0,0,0);
	bbox.max.Set(0,0,0);
	for (i = 0; i < (int)ctx.models.size(); i++)
	{
		IStatObj *pObj = m_bc.engine->MakeObject( ctx.models[i].c_str() );
		if (pObj)
		{
			bbox.Add( pObj->GetBoxMin() );
			bbox.Add( pObj->GetBoxMax() );
			m_objects.push_back( pObj );
		}
	}

	int numObj = m_objects.size();

	// Different boids.
	for (i = 0; i < ctx.boidsCount; i++)
	{
		CBoidBug *boid = new CBoidBug( m_bc );
		float radius = m_bc.MaxAttractDistance;
		boid->m_pos = m_origin + Vec3(radius*frand(),radius*frand(),
			m_bc.MinHeight+(m_bc.MaxHeight-m_bc.MinHeight)*frand() );

		boid->m_heading = GetNormalized( Vec3(frand(),frand(),0) );
		boid->m_speed = m_bc.MinSpeed + (m_bc.MaxSpeed-m_bc.MinSpeed)*frand();

		//boid->CalcRandomTarget( GetPos(),m_bc );
		if (!ctx.characterModel.empty())
		{
			if (numObj == 0 || (i % (numObj+1) == 0))
			{
				boid->m_object = m_flockMgr->GetSystem()->GetIAnimationSystem()->MakeCharacter( ctx.characterModel.c_str() );
				if (boid->m_object)
				{
					Vec3 mins,maxs;
					boid->m_object->GetBBox(mins,maxs);
					bbox.Add( mins );
					bbox.Add( maxs );
					if (!ctx.animation.empty())
					{
						// Play animation in loop.
						boid->m_object->StartAnimation( ctx.animation.c_str(), CryCharAnimationParams() );
						ICryAnimationSet *animSet = boid->m_object->GetModel()->GetAnimationSet();
						if (animSet)
						{
							animSet->SetLoop( animSet->Find(ctx.animation.c_str()),true );
						}
					}
				}
			}
		}
	
		if (numObj > 0)
			boid->m_objectId = (i % numObj);
		else
			boid->m_objectId = 0;

		//boid->m_p = m_flockMgr->GetSystem()->GetI3DEngine()->MakeCharacter( model.c_str() );
		/*
		if (boid->m_object)
		{
			boid->Physicalize(m_bc);
		}
		*/
		AddBoid(boid);
	}
	m_bc.fBoidRadius = GetLength(bbox.max - bbox.min) * m_bc.boidScale;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CBoidBug::CBoidBug( SBoidContext &bc )
: CBoidObject( bc )
{
	m_objectId = 0;
	m_onGround = 0;
}

void CBoidBug::UpdateBugsBehavior( float dt,SBoidContext &bc )
{
	if ((rand() % 10) == 0)
	{
		// Randomally modify heading vector.
		m_heading.x += frand()*0.2f*bc.factorAlignment; // Used as random movement.
		m_heading.y += frand()*0.2f*bc.factorAlignment;
		m_heading.z += frand()*0.1f*bc.factorAlignment;
		m_heading = GetNormalized(m_heading);
		if (bc.behavior == EBUGS_DRAGONFLY)
			m_speed = bc.MinSpeed + (bc.MaxSpeed - bc.MinSpeed)*frand();
	}

	// Avoid player.
	Vec3 fromPlayerVec = Vec3( m_pos.x-bc.playerPos.x, m_pos.y-bc.playerPos.y, 0 );
	if (GetLengthSquared(fromPlayerVec) < BUGS_SCARE_DISTANCE*BUGS_SCARE_DISTANCE) // 2 meters.
	{
		float d = (BUGS_SCARE_DISTANCE - fromPlayerVec.Length());
		m_accel += 5.0f * fromPlayerVec * d;
	}

	// Maintain average speed.
	float targetSpeed = (bc.MaxSpeed + bc.MinSpeed)/2;
	m_accel -= m_heading*(m_speed-targetSpeed)*0.5f;

	//m_accel = (m_targetPos - m_pos)*bc.factorAttractToOrigin;

	if (m_pos.z < bc.terrainZ+bc.MinHeight)
	{
		m_accel.z = (bc.terrainZ+bc.MinHeight-m_pos.z)*bc.factorAttractToOrigin;
	}
	else if (m_pos.z > bc.terrainZ+bc.MaxHeight)
	{
		m_accel.z = -(m_pos.z-bc.terrainZ+bc.MinHeight)*bc.factorAttractToOrigin;
	}
	else
	{
		// Allways try to accelerate in direction oposite to current in Z axis.
		m_accel.z += -m_heading.z * 0.2f;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidBug::UpdateDragonflyBehavior( float dt,SBoidContext &bc )
{
	UpdateBugsBehavior( dt,bc );	
}

//////////////////////////////////////////////////////////////////////////
void CBoidBug::UpdateFrogsBehavior( float dt,SBoidContext &bc )
{
	if (m_onGround)
	{
		if (((rand() % 100) == 1) ||
				(GetLengthSquared(Vec3(bc.playerPos.x-m_pos.x,bc.playerPos.y-m_pos.y,0)) < BUGS_SCARE_DISTANCE*BUGS_SCARE_DISTANCE))
		{
			// Sacred by player or random jump.
			m_onGround = false;
			m_heading = m_pos - bc.playerPos;
			if (m_heading != Vec3(0,0,0))
				m_heading = GetNormalized(m_heading);
			else
				m_heading = GetNormalized(Vec3(frand(),frand(),frand()));
			m_heading.z = 0.2f + (frand()+1.0f)*0.5f;
			m_heading += Vec3(frand()*0.3f,frand()*0.3f,0 );
			if (m_heading != Vec3(0,0,0))
				m_heading = GetNormalized(m_heading);
			else
				m_heading = GetNormalized(Vec3(frand(),frand(),frand()));
			m_speed = bc.MaxSpeed;
		}
	}

	bc.terrainZ = bc.engine->GetTerrainElevation(m_pos.x,m_pos.y);

	float range = bc.MaxAttractDistance;

	Vec3 origin = bc.flockPos;

	if (bc.followPlayer)
	{
		origin = bc.playerPos;
	}

	// Keep in range.
	if (bc.followPlayer)
	{
		if (m_pos.x < origin.x - range)
			m_pos.x = origin.x + range;
		if (m_pos.y < origin.y - range)
			m_pos.y = origin.y + range;
		if (m_pos.x > origin.x + range)
			m_pos.x = origin.x - range;
		if (m_pos.y > origin.y + range)
			m_pos.y = origin.y - range;
	}
	else
	{
		/*
		if (bc.behavior == EBUGS_BUG || bc.behavior == EBUGS_DRAGONFLY)
		{
			if (m_pos.x < origin.x-range)
				m_accel = (origin - m_pos)*bc.factorAttractToOrigin;
			if (m_pos.y < origin.y-range)
				m_accel = (origin - m_pos)*bc.factorAttractToOrigin;
			if (m_pos.x > origin.x+range)
				m_accel = (origin - m_pos)*bc.factorAttractToOrigin;
			if (m_pos.y > origin.y+range)
				m_accel = (origin - m_pos)*bc.factorAttractToOrigin;
		}
		*/
	}
	m_accel.Set( 0,0,-10 );

	bool bBanking = m_object != 0;
	CalcMovement( dt,bc,bBanking );

	if (m_pos.z < bc.terrainZ+0.1f)
	{
		// Land.
		m_pos.z = bc.terrainZ+0.1f;
		m_onGround = true;
		m_speed = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidBug::Update( float dt,SBoidContext &bc )
{
	if (bc.behavior == EBUGS_FROG)
	{
		UpdateFrogsBehavior( dt,bc );
		return;
	}

	if (m_onGround)
	{
		if (GetLengthSquared(Vec3(bc.playerPos.x-m_pos.x,bc.playerPos.y-m_pos.y,0)) < BUGS_SCARE_DISTANCE*BUGS_SCARE_DISTANCE)
		{
			// Sacred by player, fast takeoff.
			m_onGround = false;
			m_heading = GetNormalized(m_pos - bc.playerPos);
			m_heading.z = 0.3f;
			m_heading = GetNormalized(m_heading);
			m_speed = 1;
		}
		else if ((rand() % 50) == 0)
		{
			// take off.
			m_onGround = false;
			m_heading.z = 0.2f;
			m_heading = GetNormalized(m_heading);
		}
		return;
	}
	// Keep in range.
	bc.terrainZ = bc.engine->GetTerrainElevation(m_pos.x,m_pos.y);

	float range = bc.MaxAttractDistance;

	Vec3 origin = bc.flockPos;

	if (bc.followPlayer)
	{
		origin = bc.playerPos;
	}
	
	if (bc.followPlayer)
	{
		if (m_pos.x < origin.x - range)
			m_pos.x = origin.x + range;
		if (m_pos.y < origin.y - range)
			m_pos.y = origin.y + range;
		if (m_pos.x > origin.x + range)
			m_pos.x = origin.x - range;
		if (m_pos.y > origin.y + range)
			m_pos.y = origin.y - range;
	}
	else
	{
		if (bc.behavior == EBUGS_BUG || bc.behavior == EBUGS_DRAGONFLY)
		{
			if (m_pos.x < origin.x-range)
				m_accel = (origin - m_pos)*bc.factorAttractToOrigin;
			if (m_pos.y < origin.y-range)
				m_accel = (origin - m_pos)*bc.factorAttractToOrigin;
			if (m_pos.x > origin.x+range)
				m_accel = (origin - m_pos)*bc.factorAttractToOrigin;
			if (m_pos.y > origin.y+range)
				m_accel = (origin - m_pos)*bc.factorAttractToOrigin;
		}
	}

	if (bc.behavior == EBUGS_BUG)
	{
		UpdateBugsBehavior( dt,bc );
	}
	else 	if (bc.behavior == EBUGS_DRAGONFLY)
	{
		UpdateDragonflyBehavior( dt,bc );
	}
	else 	if (bc.behavior == EBUGS_FROG)
	{
		UpdateFrogsBehavior( dt,bc );
	}
	else
	{
		UpdateBugsBehavior( dt,bc );
	}

	bool bBanking = m_object != 0;
	CalcMovement( dt,bc,bBanking );

	if (m_pos.z < bc.terrainZ+0.1f)
	{
		// Land.
		m_pos.z = bc.terrainZ+0.1f;
		if (!bc.noLanding && (rand()%10) == 0)
		{
			m_onGround = true;
			m_speed = 0;
			m_accel.Set(0,0,0);
		}
	}

	if (m_pos.z < bc.waterLevel)
		m_pos.z = bc.waterLevel;
}

//////////////////////////////////////////////////////////////////////////
void CBoidBug::Render( SRendParams &rp,CCamera &cam,SBoidContext &bc )
{
	// Cull boid.
	if (!cam.IsSphereVisibleFast( Sphere(m_pos,bc.fBoidRadius*bc.boidScale) ))
		return;

	Vec3 oldheading;
	if (bc.behavior == EBUGS_FROG)
	{
		// Frogs/grasshopers do not change z orientation.
		oldheading = m_heading;
		if (fabsf(m_heading.x > 0.0001f) || fabsf(m_heading.y > 0.0001f))
			m_heading = GetNormalized( Vec3(m_heading.x,m_heading.y,0 ) );
		else
		{
			m_heading = GetNormalized(Vec3(frand(),frand(),frand()));
			oldheading = m_heading;
		}
	}

	Matrix44 mtx;
	CalcMatrix( mtx );
	mtx.ScaleMatRow( Vec3(bc.boidScale,bc.boidScale,bc.boidScale) );

	if (bc.behavior == EBUGS_FROG)
	{
		m_heading = oldheading;
	}

	rp.pMatrix = &mtx;

	if (m_object)
	{
		m_object->Update();
		m_object->Draw( rp, Vec3(zero) );
	}
	else
	{
		//pStatObj
		CBugsFlock *flock = (CBugsFlock*)m_flock;
		int numo = flock->m_objects.size();
		if (numo == 0)
			return;
		IStatObj *pObj = flock->m_objects[m_objectId % numo];
		if (!pObj)
			return;

		pObj->Render( rp, Vec3(zero),0);
	}
}

void CBugsFlock::PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime)
{
	for (unsigned int i = 0; i < m_objects.size(); i++)
	{
		IStatObj * pStatObj = m_objects[i];
		float fDistance = fPrevPortalDistance + vPrevPortalPos.GetDistance((m_bounds.min+m_bounds.min)*0.5f);
		pStatObj->PreloadResources(fDistance, fTime, 0);
	}
}