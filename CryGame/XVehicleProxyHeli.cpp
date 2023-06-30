#include "stdafx.h"
#include "xvehicleproxy.h"
#include "XVehicle.h"
//#include <IAgent.h>
#include <IAISystem.h>





//
//--------------------------------------------------------------------------------------------------
// isn't it obvious
// state->bodystate 1			- helicopter droping people, 
//												ignore objects below, only terrain is used for altettude calculations
//												use target altitude for reference - NOT terrain
// state->bodystate 2			- add deviation - move erratically when under fire
// state->bodystate 3			- attacking
// state->bodystate 4			- special path - sticking to the tagpoint - ignore flightAltitude and obstacles
// state->fStickDist<1.0f	-	must be attacking - stuck to target
void CXVehicleProxy::MoveLikeAHelicopter(SOBJECTSTATE * state)
{
//MoveLikeAHelicopter2(state);
//return;

bool	bAttacking = (state->bodystate == 3);//state->fStickDist>1.0f);			// attacking

		Vec3d angles = m_pEntity->GetAngles();

		Vec3d	movement;

		float tScale = m_pGame->GetSystem()->GetITimer()->GetFrameTime();
		if(tScale>.1f)		//to prevent stuff when debuggin
			tScale = .1f;

		movement(0,0,0);

		if(state->fValueAux<0.0f)	// is landed - don't move
		{
			Vec3d curPos = m_pEntity->GetPos();
//			movement.z=UpdateAltitude( m_Movement, 2, tScale, 0);
//			m_pEntity->SetAngles(m_Direction,false);	
//		m_pEntity->SetAngles(angles,false);	

			float	desiredAlt = m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation( curPos.x, curPos.y );
			float	waterAltitude = m_pGame->GetSystem()->GetI3DEngine()->GetWaterLevel(m_pEntity);

			if( waterAltitude > desiredAlt )
				desiredAlt = waterAltitude;
			desiredAlt -= state->fValueAux;

			movement.z = desiredAlt - curPos.z;

			DemperVect(m_Movement, movement, tScale*11.1f);

			pe_action_move motion;
			// motion.dir.set(0,0,0);
			motion.dir = m_Movement;
			if(m_pEntity->GetPhysics())
				m_pEntity->GetPhysics()->Action(&motion);

			angles.x = 0;
			angles.y = 0;

			DemperVect(m_Direction, angles, tScale*10.3f);

			m_pEntity->SetAngles(m_Direction,false);	

			return;
		}




		Vec3d dir = angles;
		dir=ConvertToRadAngles(dir);

		// handle turning
		if (state->turnleft || state->turnright)
		{
				angles.z+=state->fValue;	
		}

		if( state->dodge )
		{
			movement = state->vMoveDir*m_fForwardSpeed;
		}
		else
		{
			movement = state->vTargetPos - m_pEntity->GetPos();
			movement.z = 0.0f;				//	don't care about target Z position - fly on desired altitude


			if(state->fStickDist>0)
			{
//				movement = state->vTargetPos - (m_pEntity->GetPos() + m_Movement*5);
//				movement.z = 0.0f;				//	don't care about target Z position - fly on desired altitude

				float deltaDist = movement.Length() - state->fStickDist;

				if( deltaDist < -.1f )
				{
					movement.Normalize();
					movement *= -m_fForwardSpeed;

					if( deltaDist > -m_fForwardSpeed )
						movement *= (-deltaDist/m_fForwardSpeed);
				}
				else if( deltaDist > .1f )
				{
					movement.Normalize();
					movement *= m_fForwardSpeed;
					if( deltaDist < m_fForwardSpeed )
						movement *= (deltaDist/m_fForwardSpeed);
				}
				else
					movement(0,0,0);

/*
				float curM = m_Movement.len2();
				float newM = movement.len2();
				if( curM>newM*2.0f )
				{

//					movement = -m_Movement + movement;

					Vec3d curNrm = m_Movement.normalized();
					Vec3d newNrm = movement.normalized();
				
					if( curNrm.Dot(newNrm) > .5 || curNrm.Dot(newNrm) < -.5 )
						movement = -m_Movement*(sqrtf(curM) - sqrtf(newM));
				}
*/
			}
			else
			if(state->vMoveDir.Length())
			{
				movement.Normalize();
				movement*=m_fForwardSpeed;
/*
				if(movement.Length()*3 > m_fForwardSpeed*3)
				{
					movement.Normalize();
					movement*=m_fForwardSpeed;
				}
				else
					movement *= .33f;
	//			else
*/
			}
			else
				movement(0,0,0);

			//
			//	add strafe
			if(state->left || state->right)
			{
				Vec3d ang = m_pEntity->GetAngles();
				ang=ConvertToRadAngles(ang);
				Vec3d leftdir = state->vTargetPos - m_pEntity->GetPos();
				leftdir.Normalize();
	//			Vec3d leftdir = ang;
				Matrix44 mat;
				mat.SetIdentity();
				if (state->left)
				{
					//mat.RotateMatrix_fix(Vec3d(0,0,90));
					mat=Matrix44::CreateRotationZYX(-Vec3d(0,0,90)*gf_DEGTORAD)*mat; //NOTE: angles in radians and negated 
				}
				else
				{
					//mat.RotateMatrix_fix(Vec3d(0,0,-90));
					mat=Matrix44::CreateRotationZYX(-Vec3d(0,0,-90)*gf_DEGTORAD)*mat; //NOTE: angles in radians and negated 
				}
				leftdir = mat.TransformPointOLD(leftdir);
				movement += leftdir*m_fBackwardSpeed;
			}

			float	absMinAlt = m_MinAltitude;
			if(bAttacking)			// attacking
				absMinAlt = state->vTargetPos.z + state->fValueAux;
			else if( state->bodystate==1 )	// dropping people
				absMinAlt = state->vTargetPos.z + state->fValueAux;
			else if( state->bodystate==4 )	// special - stick to tagPoint
			{
				absMinAlt = state->vTargetPos.z;
				state->fValueAux = 0;
			}

			// if state->bodystate==1 - ignore objects below, only terrain is used for altettude calculations
			float	deltaA=UpdateAltitude( m_Movement, state->fValueAux, absMinAlt,
//						(state->fStickDist>1.0f||state->bodystate==1 ? state->vTargetPos.z : m_MinAltitude),	// if attacking or dropping - don't go lower tham target
						(state->bodystate!=1)&&(state->bodystate!=4));


			m_ReverseTime += m_pGame->GetSystem()->GetITimer()->GetFrameTime();
			if( m_ReverseTime>4.1f)		// if last change was long ago
			{
				float fwdVal = HelyEvaluatePosition(m_pEntity->GetPos()+m_Movement*3.5);
				if(m_pEntity->GetPos().z + 5 < fwdVal)		// if high obstacles-terrain in direstion of movement
				{
					// - try to change strafing dir when attacking
					SendSignal( 0, "CHANGESTRAFE", m_pEntity );
					m_ReverseTime = 0.0f;
				}
			}
			movement.z = deltaA;
			if( state->bodystate == 2 )	//	add deviation - move erratically
			{
				UpdateDeviation( state->vTargetPos, tScale );
				movement += m_Deviation;
			}
		}

		if(state->bodystate!=1 && state->bodystate!=4)
			movement += HelyAvoidCollision()*40.0f;

		angles.x = 0;
		angles.y = 0;

		// generate random floating illusion
//		if( state->bodystate == 1 )
		{
			m_fAngle+=tScale;
		}



		// Add random floating illusion
		Vec3d	hoverPos, hoverAngle;
		UpdateHover( hoverPos, hoverAngle, m_fAngle );
		// scale it down with speed
		movement += hoverPos;//*(1.0f-m_fThrust/m_fForwardSpeed);
		angles += hoverAngle;//*(1.0f-m_fThrust/m_fForwardSpeed);

//movement.z = 0;
		Vec3d tmpS=movement;
		Vec3d tmpT=m_Movement;

		// if there is high m_fForwardSpeed make less smoothing to allow hely to respond faster
		if(m_fForwardSpeed>35)
		{
			DemperVect(m_Movement, movement, tScale*m_fForwardSpeed*10.0f);
		}
		else
		{
			if(bAttacking)			// attacking
				DemperVect(m_Movement, movement, tScale*m_fForwardSpeed*.5f);	// low smoothing
			else
				if(state->fStickDist>0)
					DemperVect(m_Movement, movement, tScale*25.1f);
				else
					DemperVect(m_Movement, movement, tScale*m_fForwardSpeed*0.5f);
		}
//m_Movement =  movement;

		if( tmpS.z>20 )	// alarm - need to go up immediately to awoid ground/object collision
			DemperVect(tmpT, tmpS, tScale*25.0f);
		else
		{
			if( m_Movement.z>7 )
				DemperVect(tmpT, tmpS, tScale*50.5f);
//				m_Movement.z = 7 + (m_Movement.z-7)*.3;
/*
			if( tmpS.z<0 )	// going down - smooth it more
				DemperVect(tmpT, tmpS, tScale*5.5f);
			else			// doing up - not much smoothing, to avoid getting into terrain/obstacles
				DemperVect(tmpT, tmpS, tScale*5.1f);
*/
			DemperVect(tmpT, tmpS, tScale*5.5f);
			if( state->bodystate==1 && tmpT.z<-5.0f )	// when dropping people - limit for descending speed
	//				tmpT.z = -2;
					tmpT.z /= 5.0f;
		}

		m_Movement.z = tmpT.z;

//		m_Movement = movement;

		//
		//	tilt in direction of movement
		Vec3d	horizontMovment = m_Movement;
		horizontMovment.z = 0.0f;
		float	tilt = GetLengthSquared(horizontMovment);
		if( tilt )
		{
		Vec3d	tVector;
//			tilt = 20.0f*tilt/(m_fForwardSpeed*m_fForwardSpeed);
			tilt = (2.0f*m_fForwardSpeed)*tilt/(m_fForwardSpeed*m_fForwardSpeed);
			if( tilt>30 )
				tilt = 30;
			tVector( tilt, 0.0f, 0.0f );
			Vec3d vMoveNorm = m_Movement;
			vMoveNorm=ConvertVectorToCameraAngles(vMoveNorm);

	//		vMoveNorm.Snap360();
			Vec3d	ang = m_pEntity->GetAngles();
//			ang.Snap360();
			Matrix44 mat;
			mat.SetIdentity();
			mat=Matrix44::CreateRotationZYX(-Vec3d(0,0,vMoveNorm.z-m_pEntity->GetAngles().z)*gf_DEGTORAD)*mat; //NOTE: angles in radians and negated 
			tVector = GetTransposed44(mat)*(tVector);

			angles += tVector;
		}
		//
		//	add tilt if rotating/turning
		if(state->vMoveDir.Length())
		{
			Vec3d vMoveNorm = state->vMoveDir;
			float zcross = dir.x * vMoveNorm.y - dir.y * vMoveNorm.x;
			zcross *=30.f; 
			if ((m_fTilt < 30.f) && (m_fTilt > -30.f))
					m_fTilt += (zcross-m_fTilt)*0.03f;
		}
		else 
		{
			if (fabs(m_fTilt) > 0) 
				m_fTilt-=m_fTilt*0.051f;
		}

		angles.y += m_fTilt;


		tmpT = m_Direction;
		tmpS = angles;
		DemperVect(m_Direction, angles, tScale*7.3f);
		DemperVect(tmpT, tmpS, tScale*65.5f);
		m_Direction.z = tmpT.z;

//		DemperVect(m_Direction, angles, tScale*7.3f);



//m_Direction.Set(0,0,0);

		m_pEntity->SetAngles(m_Direction,false);	
//		m_pEntity->SetAngles(angles,false);	

		pe_action_move motion;
		// motion.dir.set(0,0,0);
		motion.dir = m_Movement;

		if(m_pEntity->GetPhysics())
			m_pEntity->GetPhysics()->Action(&motion);
}


//
//--------------------------------------------------------------------------------------------------
// generate random floating illusion
void CXVehicleProxy::UpdateHover(  Vec3d& posOffset, Vec3d& angOffset,	const float t )
{
	float	floatingAmpl = 2.0f;
	float	scale = .4f;

	float c1 = floatingAmpl*cry_sinf(m_fAngle*.24f)*cry_cosf(1.6f*m_fAngle);
	float c2 = floatingAmpl*cry_cosf(m_fAngle*.6f)*cry_sinf(1.2f*m_fAngle);
	angOffset.x = c1*floatingAmpl;
	angOffset.y = c2*floatingAmpl*3.4f;
	angOffset.z = (c1+c2)*floatingAmpl*0.0f;
	posOffset.x = ((c1+c2)/2.f)*0.01f;
	posOffset.y = c2*0.01f;
	posOffset.z = c1*0.0012f;
	posOffset *= 50.0f;
	angOffset *= .5f;
}


//
//--------------------------------------------------------------------------------------------------
//	
//	desiredAltitude - alr above terrain
//	absMinAlt - never go below
float CXVehicleProxy::UpdateAltitude( const Vec3d& moveDir, float desiredAltitude, float absMinAlt, 
																		 bool useObjects )
{
Vec3d curPos = m_pEntity->GetPos();
float	allowedObjectDist = 7.0f;
float	objectsDist=-1000, fwdObjectDist=-1000;
float	minPossibleAlt=0;	// never go below this
Vec3d	fwdPos = m_pEntity->GetPos() + moveDir*5.0f;
float	terrainAltitude = -1000;

	if(useObjects)		// get terrain/ objects below in box
	{
		terrainAltitude = HelyGetTerrainElevation( curPos, fwdPos, 15 );
		objectsDist = GetObjectsBelowHeight( curPos, allowedObjectDist, .6f );
		fwdObjectDist = GetObjectsBelowHeight( fwdPos, allowedObjectDist, .6f );
		if(objectsDist < fwdObjectDist)
			objectsDist = (objectsDist + fwdObjectDist)*.5f;
	}
	else				// get only terrain below in small box
	{
		terrainAltitude = HelyGetTerrainElevation( curPos, curPos, 1 );
	}

float	waterAltitude = m_pGame->GetSystem()->GetI3DEngine()->GetWaterLevel(m_pEntity);
	
	if( terrainAltitude < waterAltitude )
		terrainAltitude = waterAltitude;

	if(terrainAltitude<objectsDist)
		minPossibleAlt = objectsDist;
	else
		minPossibleAlt = terrainAltitude;

	desiredAltitude = desiredAltitude + terrainAltitude;

	// don't go too close to objects below
	if( desiredAltitude<objectsDist+6.1f )
		desiredAltitude = objectsDist+6.1f;
	// don't go lower desired absolute flight altitude
	if( desiredAltitude<absMinAlt )
		desiredAltitude = absMinAlt;

//float	deltaAltitude = desiredAltitude - (curPos.z - terrainAltitude);
float	deltaAltitude = desiredAltitude - curPos.z;

float	scale = 1.0f;

	
	if( deltaAltitude > 1.0f )
		scale = 1.0f;
	else if( deltaAltitude > 0.0f )
		scale *= scale;

	// if moving down - make it smooth
	if(deltaAltitude>-15 && deltaAltitude<0)
		scale = (1.0f+deltaAltitude/15.0f);

	// cup the force 
	if(deltaAltitude>7.0f)		
		deltaAltitude = 7.0f;
	else if(deltaAltitude<-7.0f)
		deltaAltitude = -7.0f;

	if( (curPos.z - 5) < minPossibleAlt) // if in ground or objects - froce go up
	{
		if(m_Movement.z < 0 ) // if moving down
			m_Movement.z *= .5f;	// slow it
		scale = 1.0f;
		float	upInput = -((curPos.z - 5) - minPossibleAlt);	// deeper in something - more up force
		if( upInput<10 )
			deltaAltitude = 17 + 5.0f*upInput/10.0f;
		else
			deltaAltitude = 17 + upInput*5.0f;
		if(deltaAltitude>30)
			deltaAltitude = 30;
	}
	return 	deltaAltitude*scale;
}


//--------------------------------------------------------------------------------------------------
//
//returns height of the hiest static entity object in self bbox extended down by depth
float CXVehicleProxy::GetObjectsBelowHeight( const Vec3d& pos, const float depth, const float scale )
{
Vec3d bbox[2];
float	theHeight = -1000;
float	selfButtom;

	m_pEntity->GetBBox( bbox[0], bbox[1] );

	selfButtom = bbox[0].z;
	bbox[0].z -= depth; 

//	vectorf vMin = bbox[0];
//	vectorf vMax = bbox[1];

	vectorf vMin = pos + (bbox[0]-m_pEntity->GetPos())*scale;
	vectorf vMax = pos + (bbox[1]-m_pEntity->GetPos())*scale;


	IPhysicalEntity **ppList = NULL;
	int iNumEntites;
	// object types - bitmask 0-terrain 1-static, 2-sleeping, 3-physical, 4-living
	iNumEntites = m_pGame->GetSystem()->GetIPhysicalWorld()->GetEntitiesInBox(vMin, vMax, ppList,  1);

	for(int i=0; i<iNumEntites; i++)
	{
	pe_status_pos status;
		// Get bbox (status) from physics engine
		if(ppList[i]->GetType()==PE_LIVING)
			continue;
		ppList[i]->GetStatus(&status);

		if(m_pGame->h_drawbelow->GetIVal() != 0 )
			m_pGame->GetSystem()->GetIRenderer()->Draw3dBBox( status.BBox[0] + status.pos, status.BBox[1] + status.pos);

		status.BBox[1].z += status.pos.z; 
		if(theHeight<status.BBox[1].z)
			theHeight = status.BBox[1].z;
	}

//return 10;
	return theHeight;
//	return selfButtom - theHeight;
}


//
//--------------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------------
void CXVehicleProxy::UpdateDeviation( const Vec3d& targetPos, const float tScale )
{
float	speed=12.3f;
	m_DeviationTime -= tScale;


//	m_Deviation.Set(0,0,0);
//return;


	if(m_DeviationTime<0)
	{

		{
			Vec3d ang = m_pEntity->GetAngles();
			ang=ConvertToRadAngles(ang);
			Vec3d leftdir = targetPos - m_pEntity->GetPos();
			leftdir.Normalize();
//			Vec3d leftdir = ang;
			Matrix44 mat;
			mat.SetIdentity();
			if (rand()%100<50)
			{
				//mat.RotateMatrix_fix(Vec3d(0,0,90));
				mat=Matrix44::CreateRotationZYX(-Vec3d(0,0,90)*gf_DEGTORAD)*mat; //NOTE: angles in radians and negated 
			}
			else
			{
				//mat.RotateMatrix_fix(Vec3d(0,0,-90));
				mat=Matrix44::CreateRotationZYX(-Vec3d(0,0,-90)*gf_DEGTORAD)*mat; //NOTE: angles in radians and negated 
			}
			leftdir = mat.TransformPointOLD(leftdir);
			m_Deviation = leftdir*m_fBackwardSpeed;
		}

/*
		m_Deviation.x = rand()/(float)RAND_MAX*speed;
		m_Deviation.y = rand()/(float)RAND_MAX*speed;
		m_Deviation.z = rand()/(float)RAND_MAX*speed*.3;
		m_DeviationTime = rand()/(float)RAND_MAX*5.0f;
*/
		m_DeviationTime = rand()/(float)RAND_MAX*5.0f;
//		if(m_DeviationTime<0)
//			m_DeviationTime -= 3.0f;
//		else
//			m_DeviationTime += 3.0f;

	}

	
}

//
//--------------------------------------------------------------------------------------------------
void CXVehicleProxy::DemperVect( Vec3d& current, const Vec3d& target, const float tScale )
{
//Vec3d	v = (target - current)*tScale*dempCoeff;
Vec3d	v = (target - current);//*tScale*dempCoeff;
v.Normalize();
v*=tScale;
float	tmp;
		
	tmp = current.x + v.x;
	if( (tmp>target.x && tmp>current.x) || (tmp<target.x && tmp<current.x) )
		current.x = target.x;
	else
		current.x = tmp;
	tmp = current.y + v.y;
	if( (tmp>target.y && tmp>current.y) || (tmp<target.y && tmp<current.y) )
		current.y = target.y;
	else
		current.y = tmp;
	tmp = current.z + v.z;
	if( (tmp>target.z && tmp>current.z) || (tmp<target.z && tmp<current.z) )
		current.z = target.z;
	else
		current.z = tmp;
}


//
//--------------------------------------------------------------------------------------------------
// avoiding rockets helicopter
Vec3d CXVehicleProxy::UpdateThreatHeli( void* threat )
{
IEntity* theThreatingEntity = (IEntity*)threat;
float	safeHeight = 8.0f;

	if( m_Movement.z<0 )
		safeHeight -= m_Movement.z;

	if(!theThreatingEntity)
		return Vec3d(0,0,0);
// Create a new status object.  The fields are initialized for us
		pe_status_dynamics status;
	// Get a pointer to the physics engine
	IPhysicalEntity *physEnt = theThreatingEntity->GetPhysics();
	// Get new player status from physics engine
	if (physEnt && physEnt->GetStatus(&status))
	{
		// Get our current velocity, default will be (0,0,0)
		Vec3d vel = (Vec3d)status.v;
		Vec3d dir = m_pEntity->GetPos() - theThreatingEntity->GetPos();

		if(vel.x == 0 && vel.y == 0 && vel.z == 0)
			return Vec3d(0,0,0);

//		dirN.Normalize();
//		vel.Normalize();

		float fdot = dir.Dot(vel);
		
		if ( fdot <  0 )
		{
			// its behind him 
			return Vec3d(0,0,0);
		}
		
		float t = ( dir.x*vel.x + dir.y*vel.y + dir.z*vel.z )/( vel.x*vel.x + vel.y*vel.y + vel.z*vel.z );
		Vec3d x = vel*t;

		dir = dir - x;

		if( dir.x == 0 && dir.y == 0 && dir.z == 0 )	// rocket goes exactly at self pos (probably newer will happen) Jus move somewhere
			dir.x = 1;
		else
		if( GetLengthSquared(dir) > 120 )	// too far away
			return Vec3d(0,0,0);


		Vec3d curMovementDirHor = m_Movement;
		Vec3d dirHor = dir;
		curMovementDirHor.z = 0;
		curMovementDirHor.Normalize();
		dirHor.z = 0;
		dirHor.Normalize();
		dirHor += curMovementDirHor;
		if(GetLengthSquared(dirHor)<1)	// prefer to move verticaly - correction direction is too different from current movement direction
			dir.z *= 50.55f;

		dir.Normalize();

		if(dir.z < .4f && CanGoDown(safeHeight))	// tend to go down - duck
		{
			if( fabs(dir.z) < .3f )
				dir.z = -.3f;
			else
				dir.z -= .25f;		
			dir.z *= 1.55f;
			dir.Normalize();
		}

		if( dir.z < 0.0f )	// check to avoid going in ground/trees/whatever
		{
			if(!CanGoDown(safeHeight))
			{
				dir.z = .0f;		
				dir.Normalize();
			}
		}
		return dir;

//		return Vec3d(1,0,0);
	}
	else
	{
		return Vec3d(0,0,0);
	}
}


//
//--------------------------------------------------------------------------------------------------
bool CXVehicleProxy::CanGoDown( const float deltaHeight )
{
float depth = GetObjectsBelowHeight( m_pEntity->GetPos(), deltaHeight );
Vec3d curPos = m_pEntity->GetPos();
float	terrainAltitude = HelyGetTerrainElevation(curPos, curPos, 2);
//m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation( curPos.x, curPos.y );
	
	return curPos.z-terrainAltitude > deltaHeight && curPos.z-depth > deltaHeight;
}


//
//--------------------------------------------------------------------------------------------------
float CXVehicleProxy::HelyEvaluatePosition( const Vec3d& pos )
{
float	value = HelyGetTerrainElevation( pos, pos, 3 );
float	objects = GetObjectsBelowHeight( pos, 7, .6f );

	if( objects>value )
		return objects;
	else
		return value;
}

//
//--------------------------------------------------------------------------------------------------
float CXVehicleProxy::HelyGetTerrainElevation( const Vec3d& pos, const Vec3d& fwdPos, float boxSize )
{
float	terrainAltitude = m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation( pos.x, pos.y );
float	fwdTerrainAltitude = m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation( fwdPos.x, fwdPos.y );
	if( terrainAltitude < fwdTerrainAltitude )
		terrainAltitude = (fwdTerrainAltitude + terrainAltitude)*.5f;
	fwdTerrainAltitude = m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation( pos.x+boxSize, pos.y+boxSize );
	if( terrainAltitude < fwdTerrainAltitude )
		terrainAltitude = fwdTerrainAltitude;
	fwdTerrainAltitude = m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation( pos.x+boxSize, pos.y-boxSize );
	if( terrainAltitude < fwdTerrainAltitude )
		terrainAltitude = fwdTerrainAltitude;
	fwdTerrainAltitude = m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation( pos.x-boxSize, pos.y+boxSize );
	if( terrainAltitude < fwdTerrainAltitude )
		terrainAltitude = fwdTerrainAltitude;
	fwdTerrainAltitude = m_pGame->GetSystem()->GetI3DEngine()->GetTerrainElevation( pos.x-boxSize, pos.y-boxSize );
	if( terrainAltitude < fwdTerrainAltitude )
		terrainAltitude = fwdTerrainAltitude;

	return terrainAltitude;
}



//
//--------------------------------------------------------------------------------------------------
bool CXVehicleProxy::IsTargetVisible( const Vec3d& selfPos, const Vec3d& targetPos )
{

//return true;

// trace reay to target, skiping self
	ray_hit hit;
	if (! m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(selfPos,targetPos-selfPos,
		ent_terrain|ent_static|ent_sleeping_rigid, 
		rwi_stop_at_pierceable,&hit,1,m_pEntity->GetPhysics()))
		return true;
	return false;
}

//
//--------------------------------------------------------------------------------------------------
Vec3d CXVehicleProxy::HeliAttackAdvance( SOBJECTSTATE &state )
{

//return m_pEntity->GetPos();


Vec3d targetPos = state.vTargetPos;
Vec3d curPos = m_pEntity->GetPos();
float curValue = HelyEvaluatePosition( curPos );

Vec3d curTargetDir = targetPos - curPos;
	curTargetDir.z=0;
	curTargetDir.normalize();

Vec3d candidatePos;
float candidateValue;	
Vec3d bestPos = curPos;
float bestValue;
float dist = 35;
float fAlt = state.fValueAux;

	if(!IsTargetVisible(curPos, targetPos+Vec3d( 0,0,1 )))
		curValue += 200;
	bestValue = curValue;

	if(state.fStickDist>0)
		dist = state.fStickDist;

	// check point behind target
	candidatePos = targetPos + curTargetDir*dist;
	candidateValue = HelyEvaluatePosition(candidatePos);
	if( candidateValue<bestValue )
	{
		if(!IsTargetVisible(candidatePos+Vec3d( 0,0,fAlt ), targetPos+Vec3d( 0,0,1 )))
			candidateValue += 200;
		if( candidateValue<bestValue )
		{
			bestPos = candidatePos;
			bestValue = candidateValue;
		}
	}

	// check point to left
	candidatePos = targetPos + Vec3d(curTargetDir.y, -curTargetDir.x, 0)*dist;
	candidateValue = HelyEvaluatePosition(candidatePos);
	if( candidateValue<bestValue )
	{
		if(!IsTargetVisible(candidatePos+Vec3d( 0,0,fAlt ), targetPos+Vec3d( 0,0,1 )))
			candidateValue += 200;
		if( candidateValue<bestValue )
		{
			bestPos = candidatePos;
			bestValue = candidateValue;
		}
	}

	// check point to right
	candidatePos = targetPos + Vec3d(-curTargetDir.y, curTargetDir.x, 0)*dist;
	candidateValue = HelyEvaluatePosition(candidatePos);
	if( candidateValue<bestValue )
	{
		if(!IsTargetVisible(candidatePos+Vec3d( 0,0,fAlt ), targetPos+Vec3d( 0,0,1 )))
			candidateValue += 200;
		if( candidateValue<bestValue )
		{
			bestPos = candidatePos;
			bestValue = candidateValue;
		}
	}

	//check some random points around target
	for( int i=0; i<5; i++ )
	{
		float	dir=(float)(rand()%360);
		candidatePos = targetPos + Vec3d(cry_cosf( DEG2RAD(dir) ), cry_sinf( DEG2RAD(dir) ), 0)*dist;
		candidateValue = HelyEvaluatePosition(candidatePos);
		if( candidateValue<bestValue )
		{
			if(!IsTargetVisible(candidatePos+Vec3d( 0,0,fAlt ), targetPos+Vec3d( 0,0,1 )))
				candidateValue += 200;
			if( candidateValue<bestValue )
			{
				bestPos = candidatePos;
				bestValue = candidateValue;
			}
		}
	}

	return bestPos;
}


//
//--------------------------------------------------------------------------------------------------
Vec3d CXVehicleProxy::HelyAvoidCollision( float distance )
{
//IAISystem
//IAIObject *obstVehicle = m_pGame->GetSystem()->GetAISystem()->GetNearestObjectOfType( m_pEntity->GetPos(), AIOBJECT_VEHICLE, avoidDist );
IAIObject *obstVehicle = m_pGame->GetSystem()->GetAISystem()->GetNearestObjectOfType( m_pEntity->GetPos(), AIOBJECT_VEHICLE, distance, m_pEntity->GetAI());
Vec3d	correction(0,0,0);

	if( obstVehicle )
	{
		correction = m_pEntity->GetPos() - obstVehicle->GetPos();
		float scale = correction.len();

		if(scale>distance)
		{
			return correction;
		}

		correction.normalize();
		correction *= (distance - scale)/distance;
	}
	
	return correction;
}
//
//--------------------------------------------------------------------------------------------------



//
//--------------------------------------------------------------------------------------------------
Vec3d CXVehicleProxy::BoatAvoidCollision( float range )
{
//IAISystem
//IAIObject *obstVehicle = m_pGame->GetSystem()->GetAISystem()->GetNearestObjectOfType( m_pEntity->GetPos(), AIOBJECT_VEHICLE, avoidDist );
IAIObject *obstVehicle = m_pGame->GetSystem()->GetAISystem()->GetNearestObjectOfType( m_pEntity->GetPos(), AIOBJECT_VEHICLE, range, m_pEntity->GetAI());
Vec3d	correction(0,0,0);
float	crossLimit = 0.0f;

	if( obstVehicle )
	{

	IUnknownProxy*	pProxy = obstVehicle->GetProxy();
	CXVehicleProxy*	pVProxy=NULL;	

		if(!pProxy->QueryProxy(AIPROXY_VEHICLE, (void**)&pVProxy))
			return correction;
		// it's helicopter or
		// it's player's boat/car
		// 
		if(!pVProxy->m_pVehicle || !pVProxy->m_pVehicle->m_bAIDriver)
		{
			crossLimit = .5f;
//			return correction;
//			return CarAvoidCollision( range );
		}

		IUnknownProxy *obsProxy = obstVehicle->GetProxy();
		IPhysicalEntity* phys=obsProxy->GetPhysics();

		pe_status_dynamics status;
		phys->GetStatus( &status );
		Vec3d	colliderVel = status.v;
		GetPhysics()->GetStatus( &status );
		Vec3d	selfVel = status.v;
		Vec3d	diff = obstVehicle->GetPos() - m_pEntity->GetPos();
		Vec3d	relVel = colliderVel - selfVel;

		relVel.z=0;
		diff.z=0;

		float scaleVel = relVel.len2()*.05f;

		if(scaleVel<.1f)
			scaleVel = .1f;

		float	distance = diff.len();
		relVel.normalize();
		diff.normalize();

		float	dotZ = diff.x*relVel.x + diff.y*relVel.y;

		if(dotZ>0.0f)
			return correction;

		selfVel.z=0;
		selfVel.normalize();

//		float crossZ = diff.x*relVel.y - diff.y*relVel.x;
		float crossZ = diff.x*selfVel.y - diff.y*selfVel.x;

		if( crossZ < -crossLimit )
			correction = Vec3d( selfVel.y, -selfVel.x, 0.0f );
		else if( crossZ > crossLimit )
			correction = Vec3d( -selfVel.y, selfVel.x, 0.0f );


		float scaleAng = 1.0f - dotZ;
		float scaleDist = 10.0f*(range - distance)/range;
		correction *= scaleAng*scaleDist*scaleVel;
	}
	
	return correction;

}



//
//--------------------------------------------------------------------------------------------------
Vec3d CXVehicleProxy::CarAvoidCollision( float range )
{
//IAISystem
//IAIObject *obstVehicle = m_pGame->GetSystem()->GetAISystem()->GetNearestObjectOfType( m_pEntity->GetPos(), AIOBJECT_VEHICLE, avoidDist );
IAIObject *obstVehicle = m_pGame->GetSystem()->GetAISystem()->GetNearestObjectOfType( m_pEntity->GetPos(), AIOBJECT_VEHICLE, range, m_pEntity->GetAI());
Vec3d	correction(0,0,0);


	if( obstVehicle )
	{
		IUnknownProxy *obsProxy = obstVehicle->GetProxy();
		IPhysicalEntity* phys=obsProxy->GetPhysics();

		pe_status_dynamics status;
		phys->GetStatus( &status );
		Vec3d	colliderVel = status.v;
		GetPhysics()->GetStatus( &status );
		Vec3d	selfVel = status.v;
		Vec3d	diff = obstVehicle->GetPos() - m_pEntity->GetPos();
		Vec3d	relVel = colliderVel - selfVel;

		relVel.z=0;
		diff.z=0;

		float scaleVel = relVel.len2()*.05f;

		if(scaleVel<.1f)
			scaleVel = .1f;

		float	distance = diff.len();
		relVel.normalize();
		diff.normalize();

		float	dotZ = diff.x*relVel.x + diff.y*relVel.y;

		if(dotZ>-0.1f)	// moving away from obstacel -- do nothing
			return correction;

		selfVel.z=0;
		selfVel.normalize();

//		float crossZ = diff.x*relVel.y - diff.y*relVel.x;
		float crossZ = diff.x*selfVel.y - diff.y*selfVel.x;

/*
		if( crossZ<0 )
			correction = Vec3d( diff.y, -diff.x, 0.0f );
		else
			correction = Vec3d( -diff.y, diff.x, 0.0f );
*/
		if( crossZ<0 )
			correction = Vec3d( selfVel.y, -selfVel.x, 0.0f );
		else
			correction = Vec3d( -selfVel.y, selfVel.x, 0.0f );


		float scaleAng = 1.0f - dotZ;
		float scaleDist = 10.0f*(range - distance)/range;
		

		correction *= scaleAng*scaleDist*scaleVel;
/*

		IUnknownProxy *obsProxy = obstVehicle->GetProxy();
		IPhysicalEntity* obstPhys=obsProxy->GetPhysics();

		pe_status_dynamics status;
		obstPhys->GetStatus( &status );

		Vec3d	obsVel = status.v;

		obstPhys = GetPhysics();
		obstPhys->GetStatus( &status );
		Vec3d	selfVel = status.v;

		selfVel.z = 0.0f;


		Vec3d relVel = obsVel - selfVel;
		Vec3d diff = obstVehicle->GetPos() - m_pEntity->GetPos();
		relVel.z = 0.0f;
		diff.z = 0.0f;
		float velScale = relVel.len2()*.01f;
		float distance = diff.len();	

		selfVel.normalize();
		relVel.normalize();
		diff.normalize();

		float dirScale = (-relVel.x*diff.x-relVel.y*diff.y) + 1.0f;
		float distScale = (range - distance)/range;


		correction = -diff;


//		selfVel.normalize();
//		float crossZ = (relVel.x*diff.y - relVel.y*diff.x);
		float crossZ = (selfVel.x*diff.y - selfVel.y*diff.x);
		if( crossZ<0 )
		{
			correction = Vec3d(-selfVel.y, selfVel.x, 0);
		}
		else
		{
			correction = Vec3d(selfVel.y, -selfVel.x, 0);
		}

		correction *= velScale*dirScale*distScale;
*/
	}
	
	return correction;

}

void CXVehicleProxy::Load(CStream &stm)
{
	int nPresent=0;

	//check curr behaviour
	stm.Read(nPresent);
	if (nPresent)
	{
		char str[255];
		stm.Read(str,255);
		m_AIHandler.SetBehaviour(str);
	}

	// check prev
	stm.Read(nPresent);
	if (nPresent)
	{
		char str[255];
		stm.Read(str,255);
	}
}

void CXVehicleProxy::Save(CStream &stm)
{
	// save the current & previous behaviours
	const char *szString;
	if (m_AIHandler.m_pBehavior)
	{
		stm.Write((int)1); // we have a current behaviour
		m_AIHandler.m_pBehavior->GetValue("Name",szString);
		stm.Write(szString);
	}
	else
		stm.Write((int)0);

	if (m_AIHandler.m_pPreviousBehavior)
	{
		stm.Write((int)1);
		m_AIHandler.m_pPreviousBehavior->GetValue("Name",szString);
		stm.Write(szString);
	}
	else
		stm.Write((int)0);
}

//
//--------------------------------------------------------------------------------------------------
