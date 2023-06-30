////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   flock.cpp
//  Version:     v1.00
//  Created:     5/4/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Flock.h"
#include "BugsFlock.h"
#include "WeaponClass.h"

#include <float.h>
#include <limits.h>
#include <IScriptSystem.h>
#include <ScriptObjectVector.h>

#include <CryCharAnimationParams.h>

#define BIRDS_PHYSICS_DENSITY 200
#define BIRDS_PHYSICS_INWATER_DENSITY 800
#define FISH_PHYSICS_DENSITY 800

#define MAX_BIRDS_DISTANCE 300

#define MAX_SPEED 15
#define MIN_SPEED 2.5f

#define MAX_ATTRACT_DISTANCE 20
#define MIN_ATTRACT_DISTANCE 5

#define MAX_FLIGHT_HEIGHT 40
#define MIN_FLIGHT_HEIGHT 5

#define MAX_REST_TIME 5
//#define LANDING_SPEED 1.0f
#define LANDING_FORCE 2.0f

#define MAX_FLIGHT_TIME 30
#define MIN_FLIGHT_TIME 10

#define SCARE_DISTANCE 10

#define ALIGNMENT_FACTOR 1.0f
#define COHESION_FACTOR 1.0f
#define SEPARATION_FACTOR 10.0f

#define ORIGIN_ATTRACT_FACTOR 0.1f
#define DESIRED_HEIGHT_FACTOR 0.4f
#define AVOID_LAND_FACTOR 10.0f

#define MAX_ANIMATION_SPEED 1.7f

//! Return random value in [-1,1] range.
inline float frand()
{
	return ((float)rand()*2.0f / RAND_MAX) - 1.0f;
}

struct MathUtil
{
	//! the Gaussian a.k.a the "bell curve", is a good function to 
	//! model fields of influence.  You get a nice round peak and falls off
	//! to zero with distance.
	//! As space_metric is basically how wide the standard deviation of the the bell curve gets.
	//! space_metric_r = 1/(space_metric * space_metric)
	static float CalcGaussianWeight( const Vec3 &v1,const Vec3 &v2, float space_metric_r)
	{ 
		Vec3 d = v2 - v1;
		return cry_expf(-(d.x*d.x*space_metric_r))*cry_expf(-(d.y*d.y*space_metric_r))*cry_expf(-(d.z*d.z*space_metric_r));
	}

	//////////////////////////////////////////////////////////////////////////
	static Matrix44 Vector2Matrix( const Vec3 &dir,const Vec3 &up,float rollAngle=0 )
	{
		Matrix44 M;
		// LookAt transform.
		Vec3 xAxis,yAxis,zAxis;
		Vec3 upVector = -up;

		yAxis = GetNormalized(dir);

		//if (zAxis.x == 0.0 && zAxis.z == 0)	up.Set( -zAxis.y,0,0 );	else up.Set( 0,1.0f,0 );

		xAxis = GetNormalized((upVector.Cross(yAxis)));
		zAxis = GetNormalized(xAxis.Cross(yAxis));

		// OpenGL kind of matrix.
		M[0][0] = xAxis.x;
		M[1][0] = yAxis.x;
		M[2][0] = zAxis.x;
		M[3][0] = 0;

		M[0][1] = xAxis.y;
		M[1][1] = yAxis.y;
		M[2][1] = zAxis.y;
		M[3][1] = 0;

		M[0][2] = xAxis.z;
		M[1][2] = yAxis.z;
		M[2][2] = zAxis.z;
		M[3][2] = 0;

		M[0][3] = 0;
		M[1][3] = 0;
		M[2][3] = 0;
		M[3][3] = 1;

		if (rollAngle != 0)
		{
			Matrix44 RollMtx;
			RollMtx.SetIdentity();

			float s = cry_sinf(rollAngle);
			float c = cry_cosf(rollAngle);

			RollMtx[0][0] = c; RollMtx[2][0] = -s;;
			RollMtx[0][2] = s; RollMtx[2][2] = c;

			// Matrix multiply.
			M = RollMtx * M;
		}

		return M;
	}
};


//////////////////////////////////////////////////////////////////////////
CBoidObject::CBoidObject( SBoidContext &bc )
{
	m_flock = 0;
	m_pPhysics = NULL;
	m_heading(1,0,0);
	m_accel(0,0,0);
	m_speed = 0;
	m_object = 0;
	m_banking = 0;
	m_alignHorizontally = 0;
	// flags
	m_dead = false;
	m_dying = false;
	m_physicsControlled = false;
	m_inwater = false;
	m_nodraw = false;

	m_speed = bc.MinSpeed + (frand()+1)/2.0f*(bc.MaxSpeed - bc.MinSpeed);
	m_heading = GetNormalized( Vec3(frand(),frand(),0) );
}

//////////////////////////////////////////////////////////////////////////
CBoidObject::~CBoidObject()
{
	if (m_pPhysics)
		m_pPhysics->GetWorld()->DestroyPhysicalEntity(m_pPhysics);
}

//////////////////////////////////////////////////////////////////////////
void CBoidObject::CalcMatrix( Matrix44 &mtx )
{
	if (m_physicsControlled && m_pPhysics)
	{
		pe_status_pos ppos;
		mtx.SetIdentity();
		ppos.pMtx4x4T = mtx.data;
		m_pPhysics->GetStatus(&ppos);
		/*
		mtx = GetTransposed44( Matrix44(ppos.q) );
		mtx.SetTranslationOLD( ppos.pos );
		*/
		return;
	}
	Vec3 up = Vec3(0,0,1);
	Vec3 dir = -m_heading;
	dir.z *= (1.0f-m_alignHorizontally);
	mtx =  MathUtil::Vector2Matrix( dir,up,m_banking*0.5f );
	mtx.SetTranslationOLD( m_pos );
}

//////////////////////////////////////////////////////////////////////////
void CBoidObject::Render( SRendParams &rp,CCamera &cam,SBoidContext &bc )
{
	if (m_nodraw)
		return;
	if (m_object)
	{
		// Cull boid.
		if (!cam.IsSphereVisibleFast( Sphere(m_pos,bc.fBoidRadius*bc.boidScale) ))
			return;

		Matrix44 mtx;
		CalcMatrix( mtx );

		Matrix44 ms;
		ms.SetIdentity();
		ms=Matrix33::CreateScale( Vec3(bc.boidScale,bc.boidScale,bc.boidScale) )*ms;

		mtx = ms * mtx;

		//m_object->Update();
		rp.pMatrix = &mtx;

		m_object->Update();
		m_object->Draw( rp, mtx.GetTranslationOLD() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidObject::CalcMovement( float dt,SBoidContext &bc,bool banking )
{
	// Calc movement with current velocity.
	if (m_speed > bc.MaxSpeed)
		m_speed = bc.MaxSpeed;
	if (m_speed < bc.MinSpeed)
		m_speed = bc.MinSpeed;

	Vec3 prevAccel;

	if (banking)
	{
		if (m_accel.x != 0 && m_accel.y != 0 && m_accel.z != 0)
			prevAccel = GetNormalized(m_accel);
		else
			banking = false;
	}

	Vec3 velocity = m_heading*m_speed;
	m_pos = m_pos + velocity*dt;
	velocity = velocity + m_accel*dt;
	m_speed = velocity.Length();
	if (fabs(m_speed) > 0.0001f)
		m_heading = velocity * (1.0f/m_speed); // Normilized velocity vector is our heading.

	if (banking)
	{
		Vec3 sideDir = m_heading.Cross(Vec3(0,0,1));
		m_banking = prevAccel.Dot(GetNormalized(sideDir));
	}

	if (m_object)
	{
		float animSpeed = m_speed / (bc.MaxSpeed - bc.MinSpeed + 0.1f) * bc.MaxAnimationSpeed;
		m_object->SetAnimationSpeed( animSpeed );
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidObject::CalcFlockBehavior( SBoidContext &bc,Vec3 &vAlignment,Vec3 &vCohesion,Vec3 &vSeparation )
{
	// Vector of sight between boids.
	Vec3 sight;

	float maxAttractDist2 = bc.MaxAttractDistance*bc.MaxAttractDistance;

	vSeparation(0,0,0);
	vAlignment(0,0,0);
	vCohesion(0,0,0);

	Vec3 v;

	// Avarage alignment and speed.
	Vec3 avgAlignment(0,0,0);
	Vec3 avgNeighborsCenter(0,0,0);
	int numMates = 0;

	int numBoids = m_flock->GetBoidsCount();
	for (int i = 0; i < numBoids; i++)
	{
		CBoidObject *boid = m_flock->GetBoid(i);
		if (boid == this) // skip myself.
			continue;

		// Check if this boid is in our range of sight.
		float dist2 = GetLengthSquared((boid->m_pos - m_pos));
		if (dist2 > maxAttractDist2)
			continue;

		// If this neighbour is in our field of view.
		// Calc distance between two boids.
		v = boid->m_pos - m_pos;
		float distance = v.Length();
		// Normilize direction vector between boids.
		Vec3 sight = v * (1.0f/distance);
		if (m_heading.Dot(sight) < bc.cosFovAngle)
			continue;

		numMates++;

		// Alignment with boid direction.
		avgAlignment += boid->m_heading * boid->m_speed;

		// Calculate avarage center of all neightbour boids.
		avgNeighborsCenter += boid->m_pos;

		// Distraction from other boids.
		if (distance < bc.MinAttractDistance)
		{
			// Boid too close, distract from him.
			float w = (1.0f - distance/bc.MinAttractDistance);
			float weight = w*w;
			vSeparation -= sight*weight * bc.factorSeparation;
		}
		/*
		else
		{
			// Attracted to boid.
			float w = (distance-bc.MinAttractDistance)/(bc.MaxAttractDistance-bc.MinAttractDistance);
			separationAccel = attractWeight*w*w;
		}
		*/
	}
	if (numMates > 0)
	{
		avgAlignment = avgAlignment * (1.0f/numMates);
		//float avgSpeed = avgAlignment.Length();
		vAlignment = avgAlignment;

		// Attraction to mates.
		avgNeighborsCenter = avgNeighborsCenter * (1.0f/numMates);
		Vec3 cohesionDir = avgNeighborsCenter - m_pos;
		float distance = cohesionDir.Length();
		cohesionDir = cohesionDir * (1.0f/distance);
		float w = (distance - bc.MinAttractDistance)/(bc.MaxAttractDistance - bc.MinAttractDistance);
		vCohesion = cohesionDir*w*w;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidObject::Physicalize( SBoidContext &bc )
{
	pe_params_particle ppart;
	ppart.gravity = vectorf(0,0,0);
	ppart.flags = particle_traceable | particle_no_roll | pef_never_affect_triggers;
	ppart.mass = bc.fBoidMass;
	ppart.size = bc.fBoidRadius;
	ppart.thickness = 0.1f;
	m_pPhysics =  bc.physics->CreatePhysicalEntity(PE_PARTICLE,&ppart,this,OT_BOID);
}

//////////////////////////////////////////////////////////////////////////
void CBoidObject::CreateRigidBox( SBoidContext &bc,const Vec3 &boxSize,float density )
{
	if (m_pPhysics)
	{
		bc.physics->DestroyPhysicalEntity(m_pPhysics);
		m_pPhysics = 0;
	}

	Matrix44 mtx;
	CalcMatrix( mtx );

	pe_params_pos bodypos;
	bodypos.pos = m_pos;
	bodypos.q = Quat( GetTransposed44(mtx) );
	m_pPhysics =  bc.physics->CreatePhysicalEntity(PE_RIGID,&bodypos,this,OT_BOID);

	pe_params_flags pf;
	pf.flagsOR = pef_never_affect_triggers;
	m_pPhysics->SetParams(&pf);

	primitives::box geomBox;
	geomBox.Basis.SetIdentity();
	geomBox.center.Set(0,0,0);
	geomBox.size = boxSize;
	geomBox.bOriented = 0;
	IGeometry *pGeom = bc.physics->GetGeomManager()->CreatePrimitive( primitives::box::type,&geomBox );
	phys_geometry *physGeom = bc.physics->GetGeomManager()->RegisterGeometry( pGeom );

	pe_geomparams partpos;
	partpos.pos.Set(0,0,0);
	partpos.density = density; // some fish density.
	partpos.surface_idx = 255; // default.

	m_pPhysics->AddGeometry( physGeom,&partpos,0 );
	bc.physics->GetGeomManager()->UnregisterGeometry( physGeom );

	pe_simulation_params symparams;
	symparams.damping = 0.3f;
	symparams.dampingFreefall = 0.3f;
	m_pPhysics->SetParams(&symparams);

	pe_params_buoyancy pb;
	pb.waterDensity = 1000.0f;
	pb.waterDamping = 1;
	pb.waterResistance = 1000;
	pb.waterPlane.n.Set(0,0,1);
	//pb.waterPlane.origin.set(0,0,m_pISystem->GetI3DEngine()->GetWaterLevel(&m_center));
	pb.waterPlane.origin.Set( 0,0,bc.waterLevel );
	m_pPhysics->SetParams(&pb);
}

//////////////////////////////////////////////////////////////////////////
void CBoidObject::CreateArticulatedCharacter( SBoidContext &bc,const Vec3 &size,float density )
{
	if (m_pPhysics)
	{
		bc.physics->DestroyPhysicalEntity(m_pPhysics);
		m_pPhysics = 0;
	}

	Matrix44 mtx;
	CalcMatrix( mtx );

	pe_params_pos bodypos;
	bodypos.pos = m_pos;
	bodypos.q = Quat( GetTransposed44(mtx) );
	bodypos.iSimClass = 2;
	m_pPhysics =  bc.physics->CreatePhysicalEntity(PE_ARTICULATED,&bodypos,this,OT_BOID);

	//m_pPhysics =  m_object->RelinquishCharacterPhysics();

	pe_params_flags pf;
	pf.flagsOR = pef_never_affect_triggers;
	m_pPhysics->SetParams(&pf);
	pe_params_articulated_body pab;
	pab.bGrounded = 0;
	pab.bCheckCollisions = 1;
	pab.bCollisionResp = 1;
	m_pPhysics->SetParams(&pab);

	m_object->BuildPhysicalEntity( m_pPhysics,bc.fBoidMass,255,1,0 );

	pe_simulation_params symparams;
	symparams.damping = 0.3f;
	symparams.dampingFreefall = 0.3f;
	m_pPhysics->SetParams(&symparams);

	pe_params_buoyancy pb;
	pb.waterDensity = 1000.0f;
	pb.waterDamping = 1;
	pb.waterResistance = 1000;
	pb.waterPlane.n.Set(0,0,1);
	//pb.waterPlane.origin.set(0,0,m_pISystem->GetI3DEngine()->GetWaterLevel(&m_center));
	pb.waterPlane.origin.Set( 0,0,bc.waterLevel );
	m_pPhysics->SetParams(&pb);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CBoidBird::CBoidBird( SBoidContext &bc )
: CBoidObject( bc )
{
	m_flightTime = 0;
	m_lastThinkTime = 0;
	m_maxFlightTime = 0;

	m_landing = false;
	m_takingoff = true;
	m_onGround = false;

	m_maxFlightTime = MIN_FLIGHT_TIME + (frand()+1)/2*(MAX_FLIGHT_TIME-MIN_FLIGHT_TIME);
	m_desiredHeigh = bc.MaxHeight + (frand()+1)/2*(bc.MaxHeight - bc.MinHeight);
	m_birdOriginPos = bc.flockPos;
}

CBoidBird::~CBoidBird()
{
}

void CBoidBird::OnFlockMove( SBoidContext &bc )
{
	m_birdOriginPos = bc.flockPos;
}

//////////////////////////////////////////////////////////////////////////
void CBoidBird::Update( float dt,SBoidContext &bc )
{
	if (m_physicsControlled)
	{
		if (m_pPhysics)
		{
			pe_status_pos ppos;
			m_pPhysics->GetStatus(&ppos);
			m_pos = ppos.pos;
			Vec3 pos = m_pos;
			// When hitting water surface, increase physics desnity.
			if (!m_inwater && m_pos.z+bc.fBoidRadius <= bc.engine->GetWaterLevel( &pos ))
			{
				m_inwater = true;
				pe_simulation_params sym;
				sym.density = BIRDS_PHYSICS_INWATER_DENSITY;
				m_pPhysics->SetParams( &sym );
			}
			bool bAwake = m_pPhysics->GetStatus(&pe_status_awake()) != 0;
			if (bAwake && m_pPhysics->GetType() == PE_ARTICULATED)
			{
				m_object->SynchronizeWithPhysicalEntity(m_pPhysics);
			}
			if (!m_inwater && !bAwake)
			//if (m_pPhysics->GetStatus(&pe_status_awake()))
			{
				// Resting. disable physics.
				//m_physicsControlled = false;
				//bc.physics->DestroyPhysicalEntity( m_pPhysics );
				//m_pPhysics = 0;
				m_dead = true;
			}
		}
		return;
	}
	if (m_dead)
		return;

	m_lastThinkTime += dt;

	if (bc.waterLevel > bc.terrainZ)
		bc.terrainZ = bc.waterLevel;

	//if (m_lastThinkTime) 
	{
	//	/*
		if (bc.followPlayer)
		{
			if (GetSquaredDistance(m_pos,bc.playerPos) > MAX_BIRDS_DISTANCE*MAX_BIRDS_DISTANCE)
			{
				float z = bc.MinHeight + (frand()+1)/2.0f*(bc.MaxHeight - bc.MinHeight);
				m_pos = bc.playerPos + Vec3(frand()*MAX_BIRDS_DISTANCE,frand()*MAX_BIRDS_DISTANCE,z );
				m_speed = bc.MinSpeed + ((frand()+1)/2.0f) / (bc.MaxSpeed - bc.MinSpeed);
				m_heading = GetNormalized(Vec3(frand(),frand(),0));
			}
		}
//		*/


		if (!m_onGround)
		{
			Think(bc);

			// Calc movement with current velocity.
			CalcMovement( dt,bc,true );
			m_accel.Set(0,0,0);
		}

		// Check if landed.
		if ((m_landing || m_dying) && !m_onGround)
		{
			float LandEpsilon = 0.5f;
			if (m_pos.z - bc.terrainZ < LandEpsilon)
			{
				m_onGround = true;
				m_landing = false;
				m_flightTime = 0;
				if (m_object)
					m_object->StartAnimation( "landing", CryCharAnimationParams() );

				// Check if landed on water.
				if (m_pos.z-bc.waterLevel < LandEpsilon+0.1f && !m_dying)
				{
					//! From water immidiatly take off.
					//! Gives fishing effect. 
					TakeOff(bc);
				}
			}
		}

		if (m_takingoff)
		{
			if (m_pos.z - bc.terrainZ > bc.MinHeight)
			{
				m_takingoff = false;
			}
		}

		if (!bc.noLanding)
		{
			if (!m_onGround)
			{
				m_flightTime += dt;
				if (m_flightTime > m_maxFlightTime /*&& (m_pos.z - bc.terrainZ) < bc.MinHeight*2*/)
				{
					// Wants to land.
					if (!m_landing)
					{
						// Only now wants to land.
						//m_object->StartAnimation( "takeoff" );
					}
					m_landing = true;
				}
			}
			else
			{
				m_flightTime += dt;
				if (m_flightTime > MAX_REST_TIME || GetDistance(m_pos,bc.playerPos) < SCARE_DISTANCE)
				{
					// Take-off.
					TakeOff(bc);
				}
			}
		}
	}

	if (m_pPhysics)
	{
		pe_params_pos ppos;
		ppos.pos = vectorf(m_pos);
		m_pPhysics->SetParams(&ppos);
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidBird::TakeOff( SBoidContext &bc )
{
	// Take-off.
	m_flightTime = 0;
	m_landing = false;
	m_onGround = false;
	m_maxFlightTime = MIN_FLIGHT_TIME + (frand()+1)/2*(MAX_FLIGHT_TIME-MIN_FLIGHT_TIME);
	//m_desiredHeigh = bc.MinHeight + (frand()+1)/2*(MAX_FLIGHT_HEIGHT - bc.MinHeight);
	m_desiredHeigh = bc.flockPos.z;
	m_takingoff = true;
	m_heading.z = 0;
	
	if (m_object)
		m_object->StartAnimation( "fly_loop" , CryCharAnimationParams());
}

//////////////////////////////////////////////////////////////////////////
void CBoidBird::Think( SBoidContext &bc )
{
	Vec3 flockHeading(0,0,0);
	float flockSpeed = 0;

	m_accel(0,0,0);
	float height = m_pos.z - bc.terrainZ;
		
	// Free will.
	// Continue accelerating in same dir untill target speed reached.
	// Try to maintain avarage speed of (maxspeed+minspeed)/2
	float targetSpeed = (bc.MaxSpeed + bc.MinSpeed)/2;
	m_accel -= m_heading*(m_speed-targetSpeed)*0.5f;

	// Desired height.
	float dh = m_desiredHeigh - m_pos.z;

	// Gaussian weight.
	m_accel.z = cry_expf(-(dh*dh)/(3*3)) * bc.factorKeepHeight;
	//m_accel.z = dh * DESIRED_HEIGHT_FACTOR;

		//exp(-(d.x*d.x*space_metric_r))
	//m_accel.z = (m_desiredHeigh+bc.terrainZ - m_pos.z) * DESIRED_HEIGHT_FACTOR;

	
	if (bc.factorAlignment != 0)
	{
		//CalcCohesion();
		Vec3 alignmentAccel;
		Vec3 cohesionAccel;
		Vec3 separationAccel;
		CalcFlockBehavior(bc,alignmentAccel,cohesionAccel,separationAccel);

		//! Adjust for allignment,
		//m_accel += alignmentAccel.Normalized()*ALIGNMENT_FACTOR;
		m_accel += alignmentAccel*bc.factorAlignment;
		m_accel += cohesionAccel*bc.factorCohesion;
		m_accel += separationAccel;
	}

	// Avoid land.
	if (height < bc.MinHeight && !m_landing)
	{
		float v = (1.0f - height/bc.MinHeight);
		m_accel += Vec3(0,0,v*v)*bc.factorAvoidLand;
	}
	else
	// Avoid max height.
	if (height > bc.MaxHeight)
	{
		float v = (height - bc.MaxHeight)*0.1f;
		m_accel += Vec3(0,0,-v);
	}
	else
	{
		// Allways try to accelerate in direction oposite to current in Z axis.
		m_accel.z = -m_heading.z * 2.0f;
	}

	// Attract to origin point.
	if (bc.followPlayer)
	{
		m_accel += (bc.playerPos - m_pos) * bc.factorAttractToOrigin;
	}
	else
	{
		m_accel += (m_birdOriginPos - m_pos) * bc.factorAttractToOrigin;
	}

	if (rand()%80 == 1)
	{
		m_birdOriginPos = Vec3(bc.flockPos.x+frand()*bc.MaxAttractDistance,bc.flockPos.y+frand()*bc.MaxAttractDistance,bc.flockPos.z );
	}

	// Avoid collision with Terrain and Static objects.
	float fCollisionAvoidanceWeight = 1.0f;
	float fCollisionDistance = 30;

	m_alignHorizontally = 0;
	// Wants to land.
	if (m_landing)
	{
		// Go down.
		m_accel.z = -LANDING_FORCE;
		fCollisionDistance = 4.0f;

		if (m_pos.z - bc.terrainZ < bc.MinHeight)
		{
			// Landing procedure.
			// Slow down bird, and align horizontally.
			float l = (bc.MinHeight - (m_pos.z - bc.terrainZ))/bc.MinHeight;
			//m_accel = m_accel.Normalized()*l;
			m_accel += -m_heading*m_speed*l;
			//m_speed = m_speed*(1.2f - l);
			m_alignHorizontally = l;
		}
	}

	if (bc.avoidObstacles)
	{
		// Avoid obstacles & terrain.
		IPhysicalWorld *physWorld = bc.physics;

		vectorf vPos = m_pos;
		vectorf vDir = m_heading*fCollisionDistance;
		int objTypes = ent_terrain|ent_static;
		int flags = rwi_stop_at_pierceable|rwi_ignore_terrain_holes;
		ray_hit hit;
		int col = physWorld->RayWorldIntersection( vPos,vDir,objTypes,flags,&hit,1 );
		if (col != 0 && hit.dist > 0)
		{
			//GetIEditor()->GetRenderer()->SetMaterialColor( 1,0,0,1 );
			//GetIEditor()->GetRenderer()->DrawLine( m_pos,m_pos+m_heading*hit.dist );

			if (m_landing)
			{
				// Decrease speed of bird.
				m_accel = -m_heading*(fCollisionDistance - hit.dist)/fCollisionDistance;
			}
			else
			{
				// Turn right/up.
				Vec3 normal = hit.n;
				//if (m_banking > 0) sideDir = -sideDir;
				//m_accel -= m_heading*(fCollisionDistance - hit.dist)*fCollisionAvoidanceWeight;
				//m_accel += (sideDir*2.0f+Vec3(0,0,1))*(fCollisionDistance - hit.dist)*fCollisionAvoidanceWeight;
				float w = (1.0f - hit.dist/fCollisionDistance);
				//Vec3 R = (m_heading - (2.0f*normal)).Normalized();
				m_accel += normal*w*w*bc.factorAvoidLand * fCollisionAvoidanceWeight;
			}
		}
	}

	if (m_landing)
	{
		//GetIEditor()->GetRenderer()->SetMaterialColor( 1,0,1,1 );
		//GetIEditor()->GetRenderer()->DrawLine( m_pos,m_pos+Vec3(0,0,-10) );
	}

	// Limits birds to above water and land.
	if (m_pos.z < bc.terrainZ-0.2f)
	{
		m_pos.z = bc.terrainZ-0.2f;
	}
	if (m_pos.z < bc.waterLevel-0.2f)
	{
		m_pos.z = bc.waterLevel-0.2f;
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidBird::Kill( const Vec3 &hitPoint,const Vec3 &force,string &surfaceName )
{
	surfaceName = "mat_feathers";

	if (!m_dead)
	{
		if (m_pPhysics)
			m_pPhysics->GetWorld()->DestroyPhysicalEntity(m_pPhysics);
		m_pPhysics = 0;
	}

	if (CFlockManager::m_e_flocks_hunt == 0)
	{
		m_physicsControlled = false;
		m_dead = true;
		m_nodraw = true;
		// No physics at all.
		return;
	}

	SBoidContext bc;
	m_flock->GetBoidSettings(bc);

	Vec3 impulse = force;
	if (impulse.GetLength() > 100.0f)
	{
		impulse.Normalize();
		impulse *= 100.0f;
	}

	if (!m_physicsControlled)
	{
		if (!m_object)
			return;
		Vec3 mins,maxs;
		m_object->GetBBox( mins,maxs );
		Vec3 size = ((maxs - mins)/2.2f)*bc.boidScale;
		//CreateRigidBox( bc,size,BIRDS_PHYSICS_DENSITY );
		CreateArticulatedCharacter( bc,size,BIRDS_PHYSICS_DENSITY );
		impulse += m_heading * (size.x*size.y*size.z)*BIRDS_PHYSICS_DENSITY;
		m_physicsControlled = true;

		// Small impulse.
		// Apply force on this body.
		pe_action_impulse theAction;
		Vec3 someforce;
		someforce = impulse;
		someforce.Normalize();
		theAction.impulse = someforce;
		theAction.ipart = 0;
		theAction.iApplyTime = 0;
		m_pPhysics->Action(&theAction);
	}

	if (m_physicsControlled)
	{
		// Apply force on this body.
		pe_action_impulse theAction;
		theAction.impulse = impulse;
		theAction.point = hitPoint;
		theAction.iApplyTime = 0;
		m_pPhysics->Action(&theAction);
	}

	if (m_object && !m_dying && !m_dead)
	{
		m_object->ResetAnimations();
		//m_object->StartAnimation( "death",CryCharAnimationParams() );
	}

	m_dead = true;
}

//////////////////////////////////////////////////////////////////////////
CBoidFish::CBoidFish( SBoidContext &bc )
: CBoidObject( bc )
{
	m_dead = 0;
	m_dying = 0;
	m_pOnSpawnBubbleFunc = NULL;
}

CBoidFish::~CBoidFish()
{
	if (m_pOnSpawnBubbleFunc)
	{
		IScriptSystem *pScriptSystem = m_flock->m_flockMgr->GetSystem()->GetIScriptSystem();
		if (pScriptSystem)
			pScriptSystem->ReleaseFunc( m_pOnSpawnBubbleFunc );
	}
}

void CBoidFish::Update( float dt,SBoidContext &bc )
{
	if (m_physicsControlled)
	{
		if (m_pPhysics)
		{
			// If fish is dead, get it position from physics.
			pe_status_pos ppos;
			m_pPhysics->GetStatus(&ppos);
			m_pos = ppos.pos;
		}
	}
	if (m_dead)
		return;
	if (m_dying)
	{
		// If fish is dying it floats up to the water surface, and die there.
		//UpdateDying(dt,bc);
		m_dyingTime += dt;
		if (m_dyingTime > 60)
		{
			m_dead = true;
			m_dying = false;
			if (m_object)
				m_object->ResetAnimations();
		}
		return;
	}


	//////////////////////////////////////////////////////////////////////////
	if (bc.followPlayer)
	{
		if (GetSquaredDistance(m_pos,bc.playerPos) > MAX_BIRDS_DISTANCE*MAX_BIRDS_DISTANCE)
		{
			float z = bc.MinHeight + (frand()+1)/2.0f*(bc.MaxHeight - bc.MinHeight);
			m_pos = bc.playerPos + Vec3(frand()*MAX_BIRDS_DISTANCE,frand()*MAX_BIRDS_DISTANCE,z );
			m_speed = bc.MinSpeed + ((frand()+1)/2.0f) / (bc.MaxSpeed - bc.MinSpeed);
			m_heading = GetNormalized(Vec3(frand(),frand(),0));
		}
	}

	float height = m_pos.z - bc.terrainZ;

	// Continue accelerating in same dir untill target speed reached.
	// Try to maintain avarage speed of (maxspeed+minspeed)/2
	float targetSpeed = (bc.MaxSpeed + bc.MinSpeed)/2;
	m_accel -= m_heading*(m_speed-targetSpeed)*0.5f;

	/*
	//m_lastThinkTime += dt;
	if (rand() % 10 == 1)
	{
		bc.flockPos = bc.flockPos + Vec3(frand()*30,frand()*30,0 );
		//m_accel.x = frand()*120;
		//m_accel.y = frand()*120;
	}
	*/
	
	if (bc.factorAlignment != 0)
	{
		Vec3 alignmentAccel;
		Vec3 cohesionAccel;
		Vec3 separationAccel;
		CalcFlockBehavior(bc,alignmentAccel,cohesionAccel,separationAccel);

		m_accel += alignmentAccel*bc.factorAlignment;
		m_accel += cohesionAccel*bc.factorCohesion;
		m_accel += separationAccel;
	}

	// Avoid water.
	if (m_pos.z > bc.waterLevel-1)
	{
		float h = bc.waterLevel - m_pos.z;
		float v = (1.0f - h);
		m_accel.z += (-v*v)*bc.factorAvoidLand;
	}
	else
	// Avoid land.
	if (height < bc.MinHeight)
	{
		float v = (1.0f - height/bc.MinHeight);
		m_accel.z += (v*v)*bc.factorAvoidLand;
	}
	else
	{
		// Allways try to accelerate in direction oposite to current in Z axis.
		m_accel.z = -m_heading.z * 2.0f;
	}

	// Attract to origin point.
	if (bc.followPlayer)
	{
		m_accel += (bc.playerPos - m_pos) * bc.factorAttractToOrigin;
	}
	else
	{
		//float originDistance2 = GetLengthSquared(bc.flockPos-m_pos);
		//if (originDistance2 > bc.MaxAttractDistance*bc.MaxAttractDistance)
		{
			float randomRadius = bc.MaxAttractDistance * 0.5f;
			Vec3 origin(bc.flockPos.x+frand()*randomRadius,bc.flockPos.y+frand()*randomRadius,bc.flockPos.z );
			m_accel += (origin - m_pos) * bc.factorAttractToOrigin;
		}

		//m_accel += (bc.flockPos - m_pos) * bc.factorAttractToOrigin;
		/*
		float originDistance2 = GetLengthSquared(bc.flockPos-m_pos);
		if (originDistance2 > bc.MaxAttractDistance*bc.MaxAttractDistance)
		{
			Vec3 origin(bc.flockPos.x+frand(),bc.flockPos.y+frand(),bc.flockPos.z );
			m_accel += (origin - m_pos) * bc.factorAttractToOrigin;
		}
		*/
	}

	// Avoid collision with Terrain and Static objects.
	float fCollisionAvoidanceWeight = 1.0f;
	float fCollisionDistance = 10;

	if (bc.avoidObstacles)
	{
		// Avoid obstacles & terrain.
		IPhysicalWorld *physWorld = bc.physics;

		vectorf vPos = m_pos;
		vectorf vDir = m_heading*fCollisionDistance;
		int objTypes = ent_terrain|ent_static;
		int flags = rwi_stop_at_pierceable|rwi_ignore_terrain_holes;
		ray_hit hit;
		int col = physWorld->RayWorldIntersection( vPos,vDir,objTypes,flags,&hit,1 );
		if (col != 0 && hit.dist > 0)
		{
			// Turn from collided surface.
			Vec3 normal = hit.n;
			float w = (1.0f - hit.dist/fCollisionDistance);
			//Vec3 R = (m_heading - (2.0f*normal)).Normalized();
			m_accel += normal*w*bc.factorAvoidLand * fCollisionAvoidanceWeight;
		}
	}

	if (rand()%40 == 1)
	{
		// Aplying random horizontal force.
		Vec3 v(frand(),frand(),frand()/2);
		//v = v.Cross(m_heading);
		m_accel += v*30;

		// Spawn bubble.
		SpawnBubble( m_pos,bc );
	}

	//////////////////////////////////////////////////////////////////////////
	// Player must scare fishes off.
	//////////////////////////////////////////////////////////////////////////
	float playerDist = GetDistance(m_pos,bc.playerPos);
	if (playerDist < SCARE_DISTANCE)
	{
		Vec3 retreatDir = m_pos - bc.playerPos;
		retreatDir.Normalize();
		float scareFactor = (1.0f - playerDist/SCARE_DISTANCE);
		m_accel.x += retreatDir.x*scareFactor*bc.factorAvoidLand;
		m_accel.y += retreatDir.y*scareFactor*bc.factorAvoidLand;
	}

	//////////////////////////////////////////////////////////////////////////
	// Calc movement.
	CalcMovement( dt,bc,false );
	m_accel.Set(0,0,0);

	// Limits fishes to under water and above terrain.
	if (m_pos.z > bc.waterLevel-0.2f)
	{
		m_pos.z = bc.waterLevel-0.2f;
	}
	else if (m_pos.z < bc.terrainZ+0.2f && bc.terrainZ < bc.waterLevel)
	{
		m_pos.z = bc.terrainZ+0.2f;
	}

	// Update physics position.
	if (m_pPhysics)
	{
		pe_params_pos ppos;
		ppos.pos = vectorf(m_pos);
		m_pPhysics->SetParams(&ppos);
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidFish::SpawnBubble( const Vec3 &pos,SBoidContext &bc )
{
	if (!bc.entity)
		return;

	IScriptObject *pScriptObject = bc.entity->GetScriptObject();
	if (!pScriptObject)
		return;

	IScriptSystem *pScriptSystem = m_flock->m_flockMgr->GetSystem()->GetIScriptSystem();

	if (!m_pOnSpawnBubbleFunc)
	{
		pScriptObject->GetValue( "OnSpawnBubble",m_pOnSpawnBubbleFunc );
	}

	if (m_pOnSpawnBubbleFunc)
	{
		if (!vec_Bubble)
		{
			vec_Bubble.Create(pScriptSystem);
		}
		vec_Bubble = pos;
		pScriptSystem->BeginCall( m_pOnSpawnBubbleFunc );
		pScriptSystem->PushFuncParam( pScriptObject );
		pScriptSystem->PushFuncParam( *vec_Bubble );
		pScriptSystem->EndCall();
	}
}

//////////////////////////////////////////////////////////////////////////
void CBoidFish::Kill( const Vec3 &hitPoint,const Vec3 &force,string &surfaceName )
{
	SBoidContext bc;
	m_flock->GetBoidSettings(bc);

	surfaceName = "mat_fish";

	// Can`t Kill fish in MP game.
	IGame *pGame = GetISystem()->GetIGame();
	if (pGame && pGame->GetModuleState( EGameMultiplayer ) )
		return;

	float boxSize = bc.fBoidRadius/2;
	float mass = ((boxSize/4)*boxSize*boxSize)*FISH_PHYSICS_DENSITY; // box volume * density
	Vec3 impulse = force * mass * 0.1f;

	if (!m_physicsControlled)
	{
		CreateRigidBox( bc,Vec3(boxSize/4,boxSize,boxSize),FISH_PHYSICS_DENSITY );
		//impulse += m_heading*m_speed*mass;
		m_physicsControlled = true;
	}

	if (m_physicsControlled)
	{
		// Apply force on this body.
		pe_action_impulse theAction;
		theAction.impulse = impulse;
		theAction.point = hitPoint;
		theAction.iApplyTime = 0;
		m_pPhysics->Action(&theAction);
	}

	if (m_object && !m_dead && !m_dying)
	{
		//m_object->ResetAnimations();
		//m_object->StartAnimation( "death" );
		m_object->SetAnimationSpeed( 1 );
	}

	m_dying = true;
	m_dyingTime = 0;
}

//////////////////////////////////////////////////////////////////////////
CFlock::CFlock( int id,CFlockManager *mgr )
{
	m_flockMgr = mgr;
	m_id = id;
	m_bEnabled = true;
	m_pEntity = NULL;
	m_updateFrameID = 0;
	m_percentEnabled = 100;

	m_type = EFLOCK_BIRDS;

	m_bounds.min = Vec3(-1,-1,-1);
	m_bounds.max = Vec3(1,1,1);

	GetDefaultBoidsContext(m_bc);

	m_bc.engine = m_flockMgr->GetSystem()->GetI3DEngine();
	m_bc.physics = m_flockMgr->GetSystem()->GetIPhysicalWorld();
	m_bc.waterLevel = m_bc.engine->GetWaterLevel();
	m_bc.fBoidMass = 1;
	m_bc.fBoidRadius = 1;
}

//////////////////////////////////////////////////////////////////////////
void CFlock::SetName( const char *name )
{
	strcpy( m_name,name );
}

//////////////////////////////////////////////////////////////////////////
void CFlock::GetDefaultBoidsContext( SBoidContext &bc )
{
	ZeroStruct(bc);
	bc.MinHeight = MIN_FLIGHT_HEIGHT;
	bc.MaxHeight = MAX_FLIGHT_HEIGHT;

	bc.MaxAttractDistance = MAX_ATTRACT_DISTANCE;
	bc.MinAttractDistance = MIN_ATTRACT_DISTANCE;

	bc.MaxSpeed = MAX_SPEED;
	bc.MinSpeed = MIN_SPEED;

	bc.factorAlignment = ALIGNMENT_FACTOR;
	bc.factorCohesion = COHESION_FACTOR;
	bc.factorSeparation = SEPARATION_FACTOR;

	bc.factorAttractToOrigin = ORIGIN_ATTRACT_FACTOR;
	bc.factorKeepHeight = DESIRED_HEIGHT_FACTOR;
	bc.factorAvoidLand = AVOID_LAND_FACTOR;

	bc.MaxAnimationSpeed = MAX_ANIMATION_SPEED;

	bc.followPlayer = false;
	bc.avoidObstacles = true;

	bc.cosFovAngle = cry_cosf(100.0f*3.1415f/180.0f);
	bc.maxVisibleDistance = 300;

	bc.boidScale = 5;
}

CFlock::~CFlock()
{
	ClearBoids();
	if (m_pEntity)
		m_pEntity->SetContainer(0);
}

//////////////////////////////////////////////////////////////////////////
void CFlock::ClearBoids()
{
	I3DEngine *engine = m_flockMgr->GetSystem()->GetI3DEngine();
	for (Boids::iterator it = m_boids.begin(); it != m_boids.end(); ++it)
	{
		CBoidObject* boid = *it;
		if (boid->m_object)
		{
			engine->RemoveCharacter(boid->m_object);
		}
		delete boid;
	}
	m_boids.clear();
}

//////////////////////////////////////////////////////////////////////////
void CFlock::AddBoid( CBoidObject *boid )
{
	boid->m_flock = this;
	m_boids.push_back(boid);
}

//////////////////////////////////////////////////////////////////////////
bool CFlock::IsFlockActive() const
{
	if (!m_bEnabled)
		return false;
	
	if (m_percentEnabled <= 0)
		return false;

	if (m_bc.followPlayer)
		return true;

	float d = m_bc.maxVisibleDistance;
	if (d*d < GetSquaredDistance(m_flockMgr->GetSystem()->GetViewCamera().GetPos(),m_origin))
	{
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CFlock::SetEnabled( bool bEnabled )
{
	if (m_bEnabled != bEnabled)
	{
		m_bEnabled = bEnabled;
		if (m_pEntity)
		{
			if (m_bEnabled)
			{
				m_pEntity->SetUpdateVisLevel( eUT_PotVisible );
				m_pEntity->SetUpdateRadius( GetMaxVisibilityDistance() );
			}
			else
				m_pEntity->SetUpdateVisLevel( eUT_Never );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFlock::SetPercentEnabled( int percent )
{
	if (percent < 0)
		percent = 0;
	if (percent > 100)
		percent = 100;
	
	m_percentEnabled = percent;
}

//////////////////////////////////////////////////////////////////////////
void CFlock::SetEntity( IEntity* entity )
{
	assert( entity );
	m_pEntity = entity;
	m_pEntity->SetContainer( this );
	m_pEntity->SetNeedUpdate(true);
	m_pEntity->SetUpdateVisLevel( eUT_PotVisible );
	m_pEntity->SetUpdateRadius( GetMaxVisibilityDistance() );
	m_pEntity->InitEntityRenderState();
	m_pEntity->SetRadius(1);
};

//////////////////////////////////////////////////////////////////////////
void CFlock::Update()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	if (!IsFlockActive())
		return;

	if (!m_flockMgr->IsFlocksEnabled())
		return;

	float dt = m_flockMgr->GetSystem()->GetITimer()->GetFrameTime();
	// Make sure delta time is limited.
	if (dt > 1.0f)
		dt = 0.01f;
	if (dt > 0.1f)
		dt = 0.1f;
	/*
	for (Boids::iterator it = m_boids.begin(); it != m_boids.end(); ++it)
	{
		CBoidObject *boid = *it;
		boid->Think();
	}
	*/
	//m_bc.playerPos = m_flockMgr->GetPlayerPos();
	m_bc.playerPos = m_flockMgr->GetSystem()->GetViewCamera().GetPos(); // Player position is position of camera.
	m_bc.flockPos = m_origin;
	m_bc.waterLevel = m_bc.engine->GetWaterLevel( &m_origin );

	m_bounds.min = Vec3(FLT_MAX,FLT_MAX,FLT_MAX);
	m_bounds.max = Vec3(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	int numBoids = m_boids.size();
	if (m_percentEnabled < 100)
	{
		numBoids = (m_percentEnabled*numBoids)/100;
	}

	int num = 0;
	for (Boids::iterator it = m_boids.begin(); it != m_boids.end(); ++it)
	{
		if (num++ > numBoids)
			break;

		CBoidObject* boid = *it;

		m_bc.terrainZ = m_bc.engine->GetTerrainElevation(boid->m_pos.x,boid->m_pos.y);
		boid->Update(dt,m_bc);

		// Update bounding box of flock.
		m_bounds.min.x = __min( m_bounds.min.x,boid->m_pos.x-m_bc.fBoidRadius );
		m_bounds.min.y = __min( m_bounds.min.y,boid->m_pos.y-m_bc.fBoidRadius );
		m_bounds.min.z = __min( m_bounds.min.z,boid->m_pos.z-m_bc.fBoidRadius );
		m_bounds.max.x = __max( m_bounds.max.x,boid->m_pos.x+m_bc.fBoidRadius );
		m_bounds.max.y = __max( m_bounds.max.y,boid->m_pos.y+m_bc.fBoidRadius );
		m_bounds.max.z = __max( m_bounds.max.z,boid->m_pos.z+m_bc.fBoidRadius );
	}
	if (m_pEntity && !m_boids.empty())
	{
		// Entity Bbox must be local.
		AABB box;
		box.min = m_bounds.min - m_origin;
		box.max = m_bounds.max - m_origin;
		m_pEntity->SetBBox( box.min,box.max );
	}
	m_updateFrameID = m_flockMgr->GetSystem()->GetIRenderer()->GetFrameID();	
	//m_flockMgr->GetSystem()->GetILog()->Log( "Birds Update" );
}

//////////////////////////////////////////////////////////////////////////
void CFlock::OnDraw(const SRendParams & EntDrawParams)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	if (!m_flockMgr->IsFlocksEnabled())
		return;

	// Only draw birds flock on the same frame id, as update call.
	int frameId = m_flockMgr->GetSystem()->GetIRenderer()->GetFrameID();
	if (abs(frameId-m_updateFrameID) > 2)
		return;

	SRendParams rp( EntDrawParams );
	CCamera &cam = m_flockMgr->GetSystem()->GetViewCamera();
	// Check if flock bounding box is visible.
	if (!cam.IsAABBVisibleFast( AABB(m_bounds.min,m_bounds.max) ))
		return;

	int numBoids = m_boids.size();
	if (m_percentEnabled < 100)
	{
		numBoids = (m_percentEnabled*numBoids)/100;
	}

	int num = 0;
	for (Boids::iterator it = m_boids.begin(); it != m_boids.end(); ++it)
	{
		if (num++ > numBoids)
			break;

		CBoidObject *boid = *it;
		boid->Render(rp,cam,m_bc);
	}
	//m_flockMgr->GetSystem()->GetILog()->Log( "Birds Draw" );
}

//////////////////////////////////////////////////////////////////////////
void CFlock::SetBoidSettings( SBoidContext &bc )
{
	m_bc = bc;
	if (m_bc.MinHeight == 0)
		m_bc.MinHeight = 0.01f;

	if (m_pEntity)
		m_pEntity->SetUpdateRadius( GetMaxVisibilityDistance() );
}

//////////////////////////////////////////////////////////////////////////
void CFlock::SetPos( const Vec3& pos )
{
	Vec3 ofs = pos - m_origin;
	m_origin = pos;
	m_bc.flockPos = m_origin;
	for (Boids::iterator it = m_boids.begin(); it != m_boids.end(); ++it)
	{
		CBoidObject *boid = *it;
		boid->m_pos += ofs;
		boid->OnFlockMove( m_bc );
	}
	// Update bounding box of flock entity.
	if (m_pEntity)
	{
		//float s = m_bc.MaxAttractDistance;
		float s = 1;
		//m_pEntity->SetBBox( pos-Vec3(s,s,s),pos+Vec3(s,s,s) );
		//m_pEntity->ForceRegisterInSectors();
	}
}

//////////////////////////////////////////////////////////////////////////
bool CFlock::RayTest( Vec3 &raySrc,Vec3 &rayTrg,SFlockHit &hit )
{
//	Vec3 v;
	Vec3 p1,p2;
	// Check all boids.
	for (unsigned int i = 0; i < m_boids.size(); i++)
	{
		CBoidObject *boid = m_boids[i];
		if ( Intersect::Lineseg_Sphere( raySrc,rayTrg,  boid->m_pos,m_bc.boidScale,  p1,p2 ) > 0)
		{
			hit.object = boid;
			hit.dist = (raySrc - p1).Length();
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Flock/Birds flocks.
//////////////////////////////////////////////////////////////////////////
void CBirdsFlock::CreateBoids( SBoidsCreateContext &ctx )
{
	ClearBoids();
	
	string model;
	if (!ctx.models.empty())
		model = ctx.models[0];

	// Different boids.
	for (int i = 0; i < ctx.boidsCount; i++)
	{
		CBoidObject *boid = new CBoidBird( m_bc );
		float radius = m_bc.fSpawnRadius;
		boid->m_pos = m_origin + Vec3(radius*frand(),radius*frand(),frand()*radius);
		boid->m_heading = GetNormalized( Vec3(frand(),frand(),0) );
		boid->m_object = m_flockMgr->GetSystem()->GetI3DEngine()->MakeCharacter( model.c_str() );
		if (boid->m_object)
		{
			boid->m_object->StartAnimation( "fly_loop" , CryCharAnimationParams());
			Vec3 mins,maxs;
			boid->m_object->GetBBox( mins,maxs );
			m_bc.fBoidRadius = (GetLength(maxs-mins)/2.0f) * m_bc.boidScale;
			boid->Physicalize(m_bc);
		}
		AddBoid(boid);
	}
}

//////////////////////////////////////////////////////////////////////////
void CFishFlock::CreateBoids( SBoidsCreateContext &ctx )
{
	ClearBoids();
	
	string model;
	if (!ctx.models.empty())
		model = ctx.models[0];

	// Different boids.
	for (int i = 0; i < ctx.boidsCount; i++)
	{
		CBoidObject *boid = new CBoidFish( m_bc );
		float radius = m_bc.MaxAttractDistance*2;
		radius = 2.0f;
		boid->m_pos = m_origin + Vec3(radius*frand(),radius*frand(),frand()*10);

		float terrainZ = m_bc.engine->GetTerrainElevation(boid->m_pos.x,boid->m_pos.y);
		if (boid->m_pos.z <= terrainZ)
			boid->m_pos.z = terrainZ + 0.01f;
		if (boid->m_pos.z > m_bc.waterLevel)
			boid->m_pos.z = m_bc.waterLevel-1;

		boid->m_speed = m_bc.MinSpeed + (frand()+1)/2.0f*(m_bc.MaxSpeed - m_bc.MinSpeed);
		boid->m_heading = GetNormalized( Vec3(frand(),frand(),0) );
		boid->m_object = m_flockMgr->GetSystem()->GetI3DEngine()->MakeCharacter( model.c_str() );
		if (boid->m_object)
		{
			boid->m_object->StartAnimation( "swim_loop", CryCharAnimationParams() );
			Vec3 mins,maxs;
			boid->m_object->GetBBox( mins,maxs );
			m_bc.fBoidRadius = GetLength(maxs-mins) * m_bc.boidScale;
			boid->Physicalize(m_bc);
		}
		AddBoid(boid);
	}
}


//////////////////////////////////////////////////////////////////////////
// CFlockManager update.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void CFlockManager::Update( float dt,Vec3 &playerPos )
{
	m_playerPos = playerPos;
}

//////////////////////////////////////////////////////////////////////////
void CFlockManager::Render()
{
}

int CFlockManager::m_e_flocks = 1;
int CFlockManager::m_e_flocks_hunt = 0;

//////////////////////////////////////////////////////////////////////////
CFlockManager::CFlockManager( ISystem *system )
{
	// Create one flock.
	m_system = system;
	m_lastFlockId = 1;
	//m_object = system->GetI3DEngine()->MakeCharacter( "Objects\\Other\\Seagull\\Seagull.cgf" );
	//m_object = system->GetI3DEngine()->MakeObject( "Objects\\Other\\Seagull\\Seagull.cgf" );
	system->GetIConsole()->Register( "e_flocks",&m_e_flocks,1,VF_DUMPTODISK,"Enable Flocks (Birds/Fishes)" );
	system->GetIConsole()->Register( "e_flocks_hunt",&m_e_flocks_hunt,0,0,"Birds will fall down..." );
}

//////////////////////////////////////////////////////////////////////////
CFlockManager::~CFlockManager()
{
	ClearFlocks();
}

//////////////////////////////////////////////////////////////////////////
CFlock* CFlockManager::CreateFlock( EFlockType type )
{
	CFlock *flock = NULL;

	m_lastFlockId++;

	switch (type)
	{
	case EFLOCK_BIRDS:
		flock = new CBirdsFlock( m_lastFlockId,this );
		break;
	case EFLOCK_FISH:
		flock = new CFishFlock( m_lastFlockId,this );
		break;
	case EFLOCK_BUGS:
		flock = new CBugsFlock( m_lastFlockId,this );
		break;
	}

	if (!flock)
		return 0;

	flock->m_type = type;

	m_flocks.push_back(flock);
	return flock;
}

//////////////////////////////////////////////////////////////////////////
void CFlockManager::RemoveFlock( CFlock *flock )
{
	for (Flocks::iterator it = m_flocks.begin(); it != m_flocks.end(); ++it)
	{
		if (flock == *it)
		{
			delete flock;
			m_flocks.erase( it );
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CFlockManager::ClearFlocks()
{
	for (Flocks::iterator it = m_flocks.begin(); it != m_flocks.end(); ++it)
	{
		delete *it;
	}
	m_flocks.clear();
}

//////////////////////////////////////////////////////////////////////////
bool CFlockManager::IsFlockVisible( CFlock *flock )
{
	if (flock->IsFollowPlayer())
		return true;

	float d = flock->GetMaxVisibilityDistance();
	if (d*d < GetSquaredDistance(m_playerPos,flock->GetPos()))
	{
		return false;
	}
	if (flock->GetEntity())
	{
		// Check if flock entity is potentially visible.
		//if (!m_system->GetI3DEngine()->IsPotentiallyVisible( flock->GetEntity(),0 ))
			//return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CFlockManager::RayTest( Vec3 &raySrc,Vec3 &rayTrg,SFlockHit &hit,bool onlyVisible )
{
	for (Flocks::iterator it = m_flocks.begin(); it != m_flocks.end(); ++it)
	{
		CFlock *flock = *it;

		// Can only hit visible flocks.
		if (onlyVisible && !IsFlockVisible(flock))
			continue;

		if (flock->RayTest( raySrc,rayTrg,hit ))
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
CFlock* CFlockManager::GetFlock( int id )
{
	//@FIXME: Linear search!! later replace with map.
	for (Flocks::iterator it = m_flocks.begin(); it != m_flocks.end(); ++it)
	{
		CFlock *flock = *it;
		if (flock->GetId() == id)
			return flock;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
CFlock* CFlockManager::FindFlock( const char *sFlockName )
{
	//@FIXME: Linear search!!
	for (Flocks::iterator it = m_flocks.begin(); it != m_flocks.end(); ++it)
	{
		CFlock *flock = *it;
		if (stricmp(sFlockName,flock->GetName()) == 0)
			return flock;
	}
	return 0;
}

void CFlock::PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime)
{
	for (int i = 0; i < GetBoidsCount(); i++)
	{
		CBoidObject * pBoid = GetBoid(i);
		float fDistance = fPrevPortalDistance + vPrevPortalPos.GetDistance((m_bounds.min+m_bounds.min)*0.5f);
		pBoid->m_object->PreloadResources(fDistance, fTime, 0);
	}
}