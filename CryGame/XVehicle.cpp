#include "stdafx.h"
#include "XVehicle.h"
#include <IAgent.h>

//////////////////////////////////////////////////////////////////////////
#define SGN(a) ((a < 0) ? -1 : 1) 

string	CVehicle::m_sNoWeaponName("NoWeapon");

//////////////////////////////////////////////////////////////////////////
CVehicle::CVehicle(CXGame *pGame):
m_fPedalSpeed(0.0f),
m_DriverID(0),
m_fPrevFwvSpeedLen(0.0f),
m_pHeadLight(NULL),
m_pHeadLightLeft(NULL),
m_pHeadLightRight(NULL),
m_pBreakLightLeft(NULL),
m_pBreakLightRight(NULL),
m_bHeadLightsOn(false),
m_bBreakLightsOn(false),
m_bAutoLights(false),
m_bPhysAwaike(false),
m_bBrakeOnNodriver(true),
m_fNoDriverTime(0.0f),
m_AngleLimitVFlag(false),
m_AngleLimitHFlag(false),
m_vWpnAng(0,0,180),
m_vWpnAngDelta(0,0,0),
m_bCrossOnScreen(false),
m_WeaponUser(0),
m_bForceHandBreak(false)
{
	m_pGame = pGame;
	m_pScriptObject=NULL;
	m_fPedalSpeed = 10.0f;
	m_fSteerSpeed = 25.0f; m_fv0MaxSteer = 40.0f; m_fkvMaxSteer = 0.0f;
	m_fv0SteerRelaxation = 0.0f; m_fkvSteerRelaxation = 5.0f;
	m_fMaxSteeringPedal = 0.015f; m_fPedalLimitSpeed = 10.0f;
	m_ScriptStream.Create(m_pGame->GetScriptSystem());
	m_Type = VHT_CAR;

	SwitchLights(0);

	m_btVelocity = 0;
	m_vEnginePos(0,0,0);

	//make unit so that at least it will move
//	m_fBoatTurnSpeed=1.0f;
//	m_fBoatSpeed=1.0f;

	m_fEngineHealth=100.0f;			
	m_fDeathTimer=-1;
	
	m_pGliderGravity=m_pGame->GetSystem()->GetIConsole()->GetCVar("game_GliderGravity");		
	m_pGliderBackImpulse=m_pGame->GetSystem()->GetIConsole()->GetCVar("game_GliderBackImpulse");		
	m_pGliderDamping=m_pGame->GetSystem()->GetIConsole()->GetCVar("game_GliderDamping");		
	m_pGliderStartGravity=m_pGame->GetSystem()->GetIConsole()->GetCVar("game_GliderStartGravity");		

	m_sAutoWeaponName.clear();
	m_sMountedWeaponName.clear();
//	m_sNoWeaponName = "NoWEapon";

//m_sWeaponName = "VehicleMountedMG";

m_AngleLimitVFlag = true;
m_AngleLimitHFlag = true;
//m_MinVAngle = -20;
m_MaxVAngle = 20;
m_MinHAngle = -95;
m_MaxHAngle = 95;


	m_bUpdateCamera = false;
	m_vCamStiffness[0](0,0,0); m_vCamStiffness[1](0,0,0);
	m_vCamLimits[0](1,1,1); m_vCamLimits[1](1,1,1);
	m_fCamDamping=0.8f; m_fMaxCamTimestep=0.01f; m_fCamSnapDist=0.001f; m_fCamSnapVel=0.01f;

	m_fhandbraking_value = 0;
	m_fhandbraking_value_nodriver = 0;

	m_fMaxBrakingFriction = 1.0;
	m_fMaxBrakingFrictionNoDriver = 1.0;

	m_bUsingNoDriverFriction = false;

	m_DirVelDotProduct = 0.0f;

	m_fstabilizejump = 0.0f;

	m_flastwheelrotation = 0.0f;

	m_fWaterlevelLimit = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
CVehicle::~CVehicle()
{
	SAFE_DELETE( m_pHeadLight );
	SAFE_DELETE( m_pHeadLightLeft );
	SAFE_DELETE( m_pHeadLightRight );
	SAFE_DELETE( m_pBreakLightLeft );
	SAFE_DELETE( m_pBreakLightRight );

	if(m_pScriptObject)
		m_pScriptObject->Release();
}

//////////////////////////////////////////////////////////////////////////
bool CVehicle::Init()
{
	m_bAcceleratedLastUpdate = false;
	m_bAcceleratedFlagSetLastFrame = false;
	m_bIsBreaking = false;
	m_iPrevMoveTime = 0;
	m_fSimTime = 0;

	GetEntity()->GetScriptObject()->SetValue("type", "Vehicle");
	// [kirill] need this to use character(weapon on vehicle) and m_objects (vehicle's geometry)
	// when calculating BBox
	GetEntity()->SetFlags( ETY_FLAG_CALCBBOX_USEALL );
	
	return true;
}
 
//////////////////////////////////////////////////////////////////////////
void CVehicle::Update()
{
	// [marco] remove the vehicle after 30 seconds it 
	// exploded - not in multiplayer
	float fTimeStep = m_pGame->GetSystem()->GetITimer()->GetFrameTime();
	if (m_fEngineHealth<=0)
		m_bHeadLightsOn=false;
	if (!m_pGame->IsMultiplayer())  
	{	
		if (m_fEngineHealth<=0)
		{	
			// first time set the death timer
			if (m_fDeathTimer<-0.5f) // epsilon
			{			
				float	deathTimer=m_pGame->p_deathtime->GetFVal();
				m_fDeathTimer = deathTimer;	
			}
			else
				m_fDeathTimer-=fTimeStep;

			if (m_fDeathTimer<=0.0f)
			{
				m_fDeathTimer = 0.0f;
				int nRendererFrameID = m_pGame->GetSystem()->GetIRenderer()->GetFrameID();
				if (nRendererFrameID-m_pEntity->GetDrawFrame()>150)
				{
					m_pEntity->Remove();
					return;
				}
			}
		}
	}

	UpdateLights();
	UpdateWeaponPosAngl();

	if(GetEntity()->GetPhysics())
	if(!GetEntity()->GetPhysics()->GetStatus(&pe_status_awake()))
		UpdateCamera(fTimeStep, m_pGame->IsSynchronizing());

	if ((m_Type == VHT_BOAT) )
		WakeupPhys();

	if (GetEntity()->GetPhysics())
	{
		pe_params_flags pf;
		pf.flagsOR = pef_custom_poststep | pef_fixed_damping;
		GetEntity()->GetPhysics()->SetParams(&pf);
	}
}

void CVehicle::UpdatePhysics(float fTimeStep)
{
	if(!HasDriver())
		m_fNoDriverTime += fTimeStep;

	UpdateMovementStatus();

	if ((m_Type == VHT_BOAT))
		UpdateBoat(fTimeStep);
	else if ( m_Type == VHT_PARAGLIDER )
		UpdateParaglider(fTimeStep);
	//[filippo]
	/*else if ((m_Type == VHT_CAR) && !HasDriver() && (m_bBrakeOnNodriver || m_fNoDriverTime>3.0f))
	{
		// there is no driver
		// brake the car
		IPhysicalEntity *icar = GetEntity()->GetPhysics();
		if(!icar)	// not phisycalized
		{
			m_pGame->GetSystem()->GetILog()->Log("\002 WARNING car is NOT phisycalized [ %s ]", m_pEntity->GetName());
			return;
		}
		
		pe_action_drive drive;
		drive.bHandBrake = 1;
		icar->Action(&drive);	
	}*/
	else if (m_Type == VHT_CAR)
	{
		// there is no driver
		// brake the car
		IPhysicalEntity *icar = GetEntity()->GetPhysics();
		if(!icar)	// not phisycalized
		{
			m_pGame->GetSystem()->GetILog()->Log("\002 WARNING car is NOT phisycalized [ %s ]", m_pEntity->GetName());
			return;
		}
		
		if ((!HasDriver() && (m_bBrakeOnNodriver || m_fNoDriverTime>3.0f)) || m_bForceHandBreak)
		{
			//filippo:when vehicles are without driver use a fake friction and save the right friction value.
			if (!m_bUsingNoDriverFriction)
			{
				pe_params_car pc;
				pc.maxBrakingFriction = m_fMaxBrakingFrictionNoDriver;

				//GetISystem()->GetILog()->Log("change friction to %f",pc.maxBrakingFriction);

				icar->SetParams(&pc);

				m_bUsingNoDriverFriction = true;
			}

			pe_action_drive drive;
			drive.bHandBrake = 1;
			icar->Action(&drive);

			AdditionalPhysics(icar,fTimeStep,true);
		}
		else
		{
			//if there is no driver and we still dont handbrake automatically , release the pedal
			if (!HasDriver())
			{
				pe_action_drive drive;
				drive.pedal = 0;
				icar->Action(&drive);
			}
			//filippo:when vehicles have driver use restore the right friction value.
			if (m_bUsingNoDriverFriction)
			{
				pe_params_car pc;
				pc.maxBrakingFriction = m_fMaxBrakingFriction;

				//GetISystem()->GetILog()->Log("change friction to %f",pc.maxBrakingFriction);

				icar->SetParams(&pc);

				m_bUsingNoDriverFriction = false;
			}

			AdditionalPhysics(icar,fTimeStep,false);
		}
	}
	//
	
	m_pGame->ConstrainToSandbox(GetEntity());
	UpdateCamera(fTimeStep, m_pGame->IsSynchronizing());

	if (m_pGame->m_pClient && !m_pGame->m_pServer && m_pGame->m_pClient->GetPlayerId()==m_DriverID && !m_bAIDriver)
		ProcessMovement(m_cmdLastMove);
}

// 0 - normal
// 1 - moving back
// 2 - turnLeft
// 3 - turnRight
// 4 - break
// 5 - break beckwards
// sets current movement type to be used to play animations for users (turning/breaking/movingBack)
void	CVehicle::UpdateMovementStatus()
{
	// if nobody inside - no need to update usersMovamaneStates
	if(m_UsersList.empty())
		return;

	IPhysicalEntity *pEnt=m_pEntity->GetPhysics();
	pe_status_dynamics	dyn;
	pEnt->GetStatus(&dyn);

	m_TurnState = 0;
	if(dyn.w.z > .1f)
		m_TurnState = 2;
	else if(dyn.w.z < -.1f)
		m_TurnState = 3;

	Vec3	vFwdDir = m_pEntity->GetAngles();

	Matrix44 tm;
	tm.SetIdentity();
	tm=Matrix44::CreateRotationZYX(-vFwdDir*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 
	vFwdDir = GetTransposed44(tm)*(Vec3d(0,-1,0));

	float	dot2d = vFwdDir.x*dyn.v.x + vFwdDir.y*dyn.v.y;
	float	velLen2 = dyn.v.len2();
	if( dot2d>0 && velLen2>1.0f)
		m_TurnState = 1;

	//GetISystem()->GetILog()->Log("dot2d: %f",dot2d);

	if(m_bIsBreaking && velLen2>3.0f)
	{
		if(m_TurnState == 1)
   			m_TurnState = 5;	
		else
   			m_TurnState = 4;
	}

	m_btVelocity = velLen2;

	m_DirVelDotProduct = dot2d;
}


//////////////////////////////////////////////////////////////////////////
//
//this is specific fake-physics code for the boat only		
//
void CVehicle::UpdateBoat(float fTimeStep)
{
	float	dTime = fTimeStep;//m_pGame->GetSystem()->GetITimer()->GetFrameTime();
	m_fSimTime += fTimeStep;
	if( dTime > 0.1f )
		dTime = 0.1f ;

	pe_action_awake	aa;
	aa.bAwake=0;
	
//dTime = .02f;

//	if(m_fTimeDelay > -10)
//		m_fTimeDelay -= dTime;

	if (!m_pEntity->GetPhysics())
		return;	// not physicalized
	IPhysicalEntity *pEnt=m_pEntity->GetPhysics();

	float	cTime = m_fSimTime;//m_pGame->GetSystem()->GetITimer()->GetCurrTime();
//	float	coef = m_BoatParams.m_fBtStand*dTime;
	Vec3	pos;
	pe_action_impulse am;
	am.iSource = 3; // this will mark impulse as non-scheduleable in fixed timestep multiplayer
	Matrix33 m;
	Vec3		boatUp=Vec3(0.0f,0.0f,1.0f);
	Vec3		boatFwd=Vec3(0.0f,-1.0f,0.0f);
	Vec3		vert=Vec3(0.0f,0.0f,1.0f);
	Vec3		dv;
	Vec3		waterLine;
	pe_status_dynamics	dyn;
	pe_status_pos spos;
	spos.pMtx3x3 = m.data;
	pEnt->GetStatus(&spos);
	pos = spos.pos;

	//get the engine pos, from where to apply the impulse
	GetEntity()->GetHelperPosition("waterlevel",waterLine,true);	
	waterLine = m*waterLine+pos;
//	if( m_Type == VHT_BOATDEAD )
//		waterLine.z += .4f;

	pEnt->GetStatus( &dyn );
	m_btVelocity = dyn.v.len();

//if(m_btVelocity>10)
//m_pGame->GetSystem()->GetILog()->Log("\001 boat VEL [ %.2f ]", m_btVelocity);

	am.momentum.Set(0,0,0);
	// transform into entity space
	boatUp = m*boatUp;
	boatFwd = m*boatFwd;
	dv = vert^boatUp;

	// waves
	float	waveWScale = m_pGame->b_wscalew->GetFVal();
	float	waveScale = m_pGame->b_wscale->GetFVal();
	float	waveMomentum = m_BoatParams.m_fBtWave;//m_pGame->b_wmomentum->GetFVal();
	Vec3	wave;
	wave.x = cry_cosf((pos.x+pos.y)*waveScale + cTime*waveWScale);
	wave.y = cry_sinf((pos.x+pos.y)*waveScale + cTime*waveWScale*1.21f);
	wave.z = 0.0f;

	//get the engine pos, from where to apply the impulse
	GetEntity()->GetHelperPosition("engine",m_vEnginePos,true);
	m_vEnginePos = m*m_vEnginePos+pos;
	
	float	waterLevel = (m_pGame->GetSystem()->GetI3DEngine()->GetWaterLevel( m_pEntity) - waterLine.z );//+ .6f - pos.z);

	//m_btInWater = (m_pGame->m_p3DEngine->GetWaterLevel(m_pEntity) > m_vEnginePos.z);

	bool inwatersave = m_btInWater;
	m_btInWater = (waterLevel>m_fWaterlevelLimit);

	//this is because we dont want the inwater state to switch between inwater/onground every 0.01 sec
	if (m_btInWater && !inwatersave)
		m_fWaterlevelLimit = -0.25f;
	else if (!m_btInWater && inwatersave)
		m_fWaterlevelLimit = 0.0f;

	//m_pGame->GetSystem()->GetILog()->Log("waterLevel:%.3f,limit:%.3f",waterLevel,m_fWaterlevelLimit);

	bool	bFloating = true;
//	float	coef = (1.0f + dv.len2()*5.0f)*m_BoatParams.m_fBtStand*dTime;
	float	coef;

//	if(coef>(3.0f)*m_BoatParams.m_fBtStand*dTime)
//		coef = (3.0f)*m_BoatParams.m_fBtStand*dTime;

//	waterLevel += m_btVelocity*.02f;

	//if(waterLevel>0.0f)	// is in water
	if(m_btInWater)	// is in water
	{
//		m_btInWater = true;
		// go stright
//		am.momentum.x = -dv.x*coef;
//		am.momentum.y = -dv.y*coef;
		coef = (1.0f + dv.len2()*5.0f)*m_BoatParams.m_fBtStand*dTime;

		am.momentum.x = -(dv.x+dyn.w.x*m_BoatParams.m_fBtDumpW)*coef;
		am.momentum.y = -(dv.y+dyn.w.y*m_BoatParams.m_fBtDumpW)*coef;

//am.momentum.set(0,0,0);
//*
		// add waves
		wave = vert^wave;
		am.momentum.x += wave.x*waveMomentum*dTime;
		am.momentum.y += wave.y*waveMomentum*dTime;
//*/
		// do water friction
//		Vec3	waveDump=dyn.w*m_BoatParams.m_fBtDumpW*dTime;
//		am.momentum = Vec3(am.momentum.x - waveDump.x, am.momentum.y - waveDump.y, am.momentum.z - dyn.w.z*m_BoatParams.m_fBtDumpRot*dTime);
		am.momentum.z -= dyn.w.z*m_BoatParams.m_fBtDumpRot*dTime;
///*
		// here goes speed damping - separate fwd/back and sideways
		// get forvard and side component of velocity
		Vec3	projVel(dyn.v.x, dyn.v.y, 0.0f);
		Vec3	projDir(boatFwd.x, boatFwd.y, 0.0f);
		float	projVelLen = projVel.len();
		projVel.normalize();
		projDir.normalize();
		Vec3	projSide(projDir.y, -projDir.x, 0.0f);
		float	velCos = projVel.x*projDir.x + projVel.y*projDir.y;
		float	fwdVelLen = projVelLen*velCos;
		Vec3	fwdV = projDir*fwdVelLen;
		velCos = projVel.x*projSide.x + projVel.y*projSide.y;
		Vec3	sideV = projSide*projVelLen*velCos;
		am.impulse.x = -(fwdV.x*m_BoatParams.m_fBtDumpV + sideV.x*m_BoatParams.m_fBtDumpVSide)*dTime;
		am.impulse.y = -(fwdV.y*m_BoatParams.m_fBtDumpV + sideV.y*m_BoatParams.m_fBtDumpVSide)*dTime;
//am.impulse.Set(0,0,0);
		am.impulse.z = -dyn.v.z*m_BoatParams.m_fBtDumpVH*dTime;
//*
		//
		// do tilt on speedup
		fwdVelLen = -fwdVelLen;
		float	velDiff = fwdVelLen - m_fPrevFwvSpeedLen;	
		if( fwdVelLen>0.0f )
		{
			Vec3 momentum = (-boatFwd)^Vec3(0.0f, 0.0f, m_BoatParams.m_fBtTitlSpd);
//			if(velDiff>0 && velDiff/fwdVelLen*dTime>0.0007F)
			if(velDiff>0 && velDiff/fwdVelLen>m_BoatParams.m_fBtTitlSpdA)
			{
				if(velDiff>dTime*20.0f)
					velDiff=dTime*20.0f;
				am.momentum += vectorf(momentum*velDiff);
				m_fPrevFwvTilt = velDiff;
			}
			else if( fwdVelLen>m_BoatParams.m_fBtTitlSpdMinV )
			{
			float	tilt = fwdVelLen*dTime*m_BoatParams.m_fBtTitlSpdMinVTilt;
			float	diff = m_fPrevFwvTilt - tilt;
			float	tiltSpd=.1f;
				if(diff/dTime>tiltSpd)
					tilt = m_fPrevFwvTilt - tiltSpd*dTime;
//				tilt += (m_fPrevFwvTilt - tilt)*.1f;
				am.momentum += vectorf(momentum*tilt);
				m_fPrevFwvTilt = tilt;
			}
		}
		m_fPrevFwvSpeedLen = fwdVelLen;
//*/
	}
	else								// not in water (jumping?)
	{				
		coef = (1.0f + dv.len2()*5.0f)*m_BoatParams.m_fBtStandAir*dTime;

		bFloating = false;
		{
			// go stright
			am.momentum.x = -(dv.x+dyn.w.x*m_BoatParams.m_fBtDumpW)*coef;
			am.momentum.y = -(dv.y+dyn.w.y*m_BoatParams.m_fBtDumpW)*coef;
/*
			// do air friction
			am.momentum = am.momentum - dyn.w*m_BoatParams.m_fBtDumpW*dTime*.3f;
//				dyn.w*m_pGame->b_dump->GetFVal()*dTime*.3f;
*/
		}
	}

	pEnt->Action(&am);

	pe_simulation_params sp;			
	sp.gravity.zero();
	sp.gravity.z = waterLevel*m_pGame->b_float->GetFVal();//*m_pGame->b_float->GetFVal();
//	if(!m_btInWater)
	if(!bFloating) 
	{
		//filippo: if m_BoatParams.m_fBoatGravity is 0 use the old sys
		sp.gravity.z = (m_BoatParams.m_fBoatGravity==0)?(sp.gravity.z*1.5f - 3):(m_BoatParams.m_fBoatGravity);
		//sp.gravity.z = sp.gravity.z*1.5f - 3; // LUC modification for better jumps out of water
	}
	else
		sp.gravity.z = sp.gravity.z*3.5f;
	pEnt->SetParams(&sp);

	// set water level
	//
	//don't use water/float with physics
	pe_params_buoyancy pb;
	pb.waterDensity = 0.0f;
	pEnt->SetParams(&pb);
}


//////////////////////////////////////////////////////////////////////////
void CVehicle::UpdateParaglider(float fTimeStep)
{
	float	dTime = fTimeStep;//m_pGame->GetSystem()->GetITimer()->GetFrameTime();
	if( dTime > 0.1f )
		dTime = 0.1f ;

//dTime = .02f;

	if(!HasDriver())
	{

		// do override gravity for the paraglider
		IPhysicalEntity *pEnt=m_pEntity->GetPhysics();
		pe_simulation_params sp;
		sp.gravity.zero();
		sp.gravity.z = -9.8f;
		sp.gravityFreefall=vectorf(0,0,-9.8f);
		sp.damping=m_pGliderDamping->GetFVal()*4.0f;
		sp.dampingFreefall=m_pGliderDamping->GetFVal();
		m_pEntity->GetPhysics()->SetParams(&sp);
		return;
	}

//	if(m_fTimeDelay > -10)
//		m_fTimeDelay -= dTime;


	//
	//this is specific fake-physics code for the boat only		
	//
	if (!m_pEntity->GetPhysics())
		return;	// not physicalized


	float	cTime = m_pGame->GetSystem()->GetITimer()->GetCurrTime();
	float	coef = m_BoatParams.m_fBtStand*dTime;
	Vec3	pos = m_pEntity->GetPos();
	Vec3	angle = m_pEntity->GetAngles();
	pe_action_impulse am;
	Matrix44	m;
	Vec3		boatUp=Vec3(0.0f,0.0f,1.0f);
	Vec3		boatFwd=Vec3(0.0f,-1.0f,0.0f);
	Vec3		vert=Vec3(0.0f,0.0f,1.0f);
	Vec3		dv;
	pe_status_dynamics	dyn;

	m_pEntity->GetPhysics()->GetStatus( &dyn );
	m_btVelocity = dyn.v.len();

	am.momentum.Set(0,0,0);
	// transform into entity space
	m.SetIdentity();
	angle.x = -DEG2RAD(angle.x);
	angle.y = -DEG2RAD(angle.y);
	angle.z = -DEG2RAD(angle.z);
	m.SetRotationZYX( angle );
	boatUp = m.TransformVectorOLD(boatUp);
	boatFwd = m.TransformVectorOLD(boatFwd);
	dv = vert^boatUp;

//go stright
	coef = (1.0f + dv.len2()*5.0f)*m_BoatParams.m_fBtStand*dTime;
	am.momentum.x = -dv.x*coef;
	am.momentum.y = -dv.y*coef;


	// do water friction
//	Vec3	waveDump=dyn.w*m_BoatParams.m_fBtDumpW*dTime;
//	am.momentum = Vec3(am.momentum.x - waveDump.x, am.momentum.y - waveDump.y, am.momentum.z - dyn.w.z*m_BoatParams.m_fBtDumpRot*dTime);
	am.momentum = am.momentum - dyn.w*m_BoatParams.m_fBtDumpRot*dTime;

//am.momentum.Set(0,0,0);

///*
	// here goes speed damping - separate fwd/back and sideways
	// get forvard and side component of velocity
	Vec3	projVel(dyn.v.x, dyn.v.y, 0.0f);
	Vec3	projDir(boatFwd.x, boatFwd.y, 0.0f);
	float	projVelLen = projVel.len();
	projVel.normalize();
	projDir.normalize();
	Vec3	projSide(projDir.y, -projDir.x, 0.0f);
	float	velCos = projVel.x*projDir.x + projVel.y*projDir.y;
	float	fwdVelLen = projVelLen*velCos;
	Vec3	fwdV = projDir*fwdVelLen;
	velCos = projVel.x*projSide.x + projVel.y*projSide.y;
	Vec3	sideV = projSide*projVelLen*velCos;
	am.impulse.x = -(fwdV.x*m_BoatParams.m_fBtDumpV + sideV.x*m_BoatParams.m_fBtDumpVSide)*dTime;
	am.impulse.y = -(fwdV.y*m_BoatParams.m_fBtDumpV + sideV.y*m_BoatParams.m_fBtDumpVSide)*dTime;
//am.impulse.Set(0,0,0);
	am.impulse.z = -dyn.v.z*m_BoatParams.m_fBtDumpVH*dTime;

	am.impulse.x += -(fwdV.x*m_BoatParams.m_fBtDumpV + sideV.x*m_BoatParams.m_fBtDumpVSide)*dTime;
	am.impulse.y += -(fwdV.y*m_BoatParams.m_fBtDumpV + sideV.y*m_BoatParams.m_fBtDumpVSide)*dTime;



//m_pGame->GetSystem()->GetILog()->Log("\003 GLIDER [ %.2f ] >>  %.2f ", m_btVelocity, m_pEntity->GetPos().z);

//*
	if(fabs(boatFwd.z)<.07) // just go forward
	{
		am.impulse -= boatFwd*dTime*m_BoatParams.m_fBtSpeedV;
	}
	else if(boatFwd.z>0)	// going down
	{
////		am.impulse.z -= boatFwd.z*20.0f;
		boatFwd *= 1.0f+boatFwd.z*6.0f;
		am.impulse -= boatFwd*dTime*m_BoatParams.m_fBtSpeedV;

//		projDir *= (1.0f+boatFwd.z*2.1f);
//		am.impulse -= projDir*dTime*m_BoatParams.m_fBtSpeedV;
	}
	else			// doing up
	{

		if(projVelLen>m_BoatParams.m_fPgUpSpeedThrhld)	// go up if speed is high
		{
		float upscale = projVelLen;
			if( upscale>m_BoatParams.m_fPgUpSpeedThrhld+5 )
				upscale = m_BoatParams.m_fPgUpSpeedThrhld+5;
			am.impulse.z -= boatFwd.z*m_BoatParams.m_fPgBackUpRate*upscale*dTime;
		}

/*
		if(m_btVelocity>m_BoatParams.m_fPgUpSpeedThrhld)	// go up if speed is high
		{
		float upscale = m_btVelocity;
			if( upscale>m_BoatParams.m_fPgUpSpeedThrhld+5 )
				upscale = m_BoatParams.m_fPgUpSpeedThrhld+5;
			am.impulse.z -= boatFwd.z*m_BoatParams.m_fPgBackUpRate*upscale*dTime;
		}
//		else if(m_btVelocity<7.0f)	// too slow - fall
//			am.impulse.z -= 520.0f*dTime;
//*/
		if(m_btVelocity>5.0f)	// slow down
		{
//			projDir *= (boatFwd.z*m_BoatParams.m_fPgBackUpSlowRate);
//			am.impulse += projDir*dTime*m_BoatParams.m_fBtSpeedV;
			am.impulse += boatFwd.z*dyn.v*m_BoatParams.m_fPgBackUpSlowRate*dTime;
		}
		else					// keep going
		{
			am.impulse -= projDir*dTime*m_BoatParams.m_fBtSpeedV;

			Vec3 momentum = (boatFwd)^Vec3(0.0f, 0.0f, m_BoatParams.m_fBtTitlSpd)*10.0f;
			am.momentum = vectorf(momentum*dTime);
		}
	}
//*/

    // go down
	am.impulse.z -= m_BoatParams.m_fPgDownRate*dTime;

//	am.impulse -= projDir*dTime*m_BoatParams.m_fBtSpeedV;
	m_pEntity->GetPhysics()->Action(&am);

	// do override gravity for the paraglider
	IPhysicalEntity *pEnt=m_pEntity->GetPhysics();
	pe_simulation_params sp;					
	sp.gravity.zero();
//	sp.gravity.z = m_pGliderStartGravity->GetFVal();		
//	sp.gravityFreefall=vectorf(0,0,m_pGliderGravity->GetFVal());
	sp.gravityFreefall.zero();
	sp.damping=m_pGliderDamping->GetFVal()*4.0f;
	sp.dampingFreefall=m_pGliderDamping->GetFVal();
	m_pEntity->GetPhysics()->SetParams(&sp);

	//don't use water/float with physics
	pe_params_buoyancy pb;
	pb.waterDensity = 0.0f;
	m_pEntity->GetPhysics()->SetParams(&pb);
}



//////////////////////////////////////////////////////////////////////////
// not-real-physics boat
void CVehicle::ProcessMovementBoat2(CXEntityProcessingCmd &cmd, float velScale  )
{	
bool	bCanBreak = (velScale>=0);

	if(velScale<0)
		velScale = 1;

	IPhysicalWorld *pWorld = m_pGame->GetSystem()->GetIPhysicalWorld();
	int iCurTime = pWorld->GetiPhysicsTime();
	float dt = (iCurTime - m_iPrevMoveTime)*pWorld->GetPhysVars()->timeGranularity;	
//		m_pGame->GetSystem()->GetITimer()->GetFrameTime();
	m_iPrevMoveTime = iCurTime;
	if( dt>.1f )
		dt = .1f;

//return;
	if(!m_pEntity->GetPhysics())
		return;
	IPhysicalEntity *iboat = GetEntity()->GetPhysics();

	// to prevent undisered input
	if(velScale>1.0f)
		velScale = 1.0f;
	if(velScale<.1f)
		velScale = .1f;

bool	bCanTurn	= (m_btVelocity>m_BoatParams.m_fBtSpeedTurnMin);
float	fMovementImpuls	= m_BoatParams.m_fBtSpeedV*velScale;
//float	fTurnImpuls	= m_BoatParams.m_fBtTurn*m_btVelocity*.05f;
float	fTurnImpuls	= m_BoatParams.m_fBtTurn - m_btVelocity*m_BoatParams.m_fBtTurnSpeedScale;
	
	if(fTurnImpuls < m_BoatParams.m_fBtTurnMin)
		fTurnImpuls = m_BoatParams.m_fBtTurnMin;

	if( (cmd.CheckAction(ACTION_MOVEMODE) || cmd.CheckAction(ACTION_MOVEMODE_TOGGLE)) && bCanBreak )
	{
		fTurnImpuls = m_BoatParams.m_fBtTurn*2;
		bCanTurn = true;
	}

	//get the engine pos, from where to apply the impulse
//	GetEntity()->GetHelperPosition("engine",m_vEnginePos);	

	Vec3 vWaterFlowSpeed(0,0,0);
	//if (!m_btInWater && (m_Type!=VHT_PARAGLIDER))
	//	return;
 
	// FIXME
	if (!m_btInWater)
	{
		if (m_Type!=VHT_PARAGLIDER)
			return;
	}
	else
	if (m_Type==VHT_PARAGLIDER)
		return;

	//define impulse action
	pe_status_pos spos;
	iboat->GetStatus(&spos);
	pe_action_impulse	control;
	control.iSource = 3; // this will mark impulse as non-scheduleable in fixed timestep multiplayer
	Vec3 angles = Ang3::GetAnglesXYZ(matrix3x3f(spos.q))*gf_RADTODEG;
	Vec3	turn = Vec3( 0.0f, 1.0f, 0.0f );
	Vec3 momentum;
	Vec3	tdir = angles;
	Vec3	dir;
	pe_status_dynamics	dyn;

	iboat->GetStatus( &dyn );
	//	velocity = dyn.v.len();
	//tdir = ConvertToRadAngles(tdir);
	Vec3 refdir(0,-1,0);
	Matrix44 tm;
	tm.SetIdentity();
	//tm.RotateMatrix_fix( angles );
	tm=Matrix44::CreateRotationZYX(-angles*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 
	tdir = GetTransposed44(tm)*(refdir);

	dir = -tdir;
	if (m_Type!=VHT_PARAGLIDER)	
	{
		dir.z = 0;
		dir.Normalize();
	}

	tdir = dir;
	tdir *= 10.0f;

	control.momentum = Vec3(0.0f, 0.0f, 0.0f);
	control.impulse = Vec3(0.0f, 0.0f, 0.0f);

//*/
	//drive the boat
	//dt = .02f;

/*
	if (cmd.CheckAction(ACTION_JUMP))
	{
		pe_action_reset	rst;
		iboat->Action(&rst);

		Vec3	angle = Vec3(30,21,30);
//		Vec3	angle = Vec3(40,31,30);
    m_pEntity->SetAngles(angle);
	}
*/

	if (cmd.CheckAction(ACTION_MOVE_FORWARD))
	{
		control.impulse = dir*fMovementImpuls*dt;		
	}
	else 
	if (cmd.CheckAction(ACTION_MOVE_BACKWARD))
	{
		if(bCanBreak)
			control.impulse = -dir*fMovementImpuls*dt;
		else
			control.impulse = -dir*fMovementImpuls*.3f*dt;
	}

	if (cmd.CheckAction(ACTION_MOVE_RIGHT) && bCanTurn)
	{
		control.momentum = -Vec3(	-dir.x*m_btVelocity*m_BoatParams.m_fBtTitlTurn*dt,
								-dir.y*m_btVelocity*m_BoatParams.m_fBtTitlTurn*dt,
								fTurnImpuls*dt);
	}
	else 
	if (cmd.CheckAction(ACTION_MOVE_LEFT) && bCanTurn) 
	{
		control.momentum = Vec3(	-dir.x*m_btVelocity*m_BoatParams.m_fBtTitlTurn*dt,
									-dir.y*m_btVelocity*m_BoatParams.m_fBtTitlTurn*dt,
									fTurnImpuls*dt);
	}
	if( dyn.v*dir<0.0f )		// moving backward - switch turning direction	
	{
		control.momentum.z = -control.momentum.z;
	}

	if( cmd.CheckAction(ACTION_WALK) && bCanBreak)	// do breaking
	{
		control.impulse = -dyn.v*m_BoatParams.m_fBtSpeedV*dt*.082f;
		control.impulse.z = 0;
	}
	if( cmd.CheckAction(ACTION_MOVEMODE2) && bCanBreak)	// do backoff
	{
		if(m_btVelocity>.1)
			control.impulse = -dyn.v*m_BoatParams.m_fBtSpeedV*dt*.82f;
		else
			control.impulse = -dir*m_BoatParams.m_fBtSpeedV*dt*5.0f;
		control.impulse.z = 0;
	}
	iboat->Action(&control);
}


//////////////////////////////////////////////////////////////////////////
// not-real-physics boat
void CVehicle::ProcessMovementParaglider(CXEntityProcessingCmd &cmd )
{	

//return;

	IPhysicalWorld *pWorld = m_pGame->GetSystem()->GetIPhysicalWorld();
	int iCurTime = pWorld->GetiPhysicsTime();
	float dt = (iCurTime - m_iPrevMoveTime)*pWorld->GetPhysVars()->timeGranularity;	
//		m_pGame->GetSystem()->GetITimer()->GetFrameTime();
	m_iPrevMoveTime = iCurTime;
	if( dt>.1f )
		dt = .1f;
	if(!m_pEntity->GetPhysics())
		return;

float	fMovementImpuls	= m_BoatParams.m_fBtSpeedV;
float	fTurnImpuls	= m_BoatParams.m_fBtTurn;//*m_btVelocity*.05f;
bool	bCanTurn	= (m_btVelocity>m_BoatParams.m_fBtSpeedTurnMin);

	//get the engine pos, from where to apply the impulse
//	GetEntity()->GetHelperPosition("engine",m_vEnginePos);	

	//define impulse action
	pe_action_impulse	control;
	Vec3 angles = m_pEntity->GetAngles();
	Vec3	turn = Vec3( 0.0f, 1.0f, 0.0f );
	Vec3 momentum;
	Vec3	tdir = angles;
	Vec3	dir;
	pe_status_dynamics	dyn;
	

	m_pEntity->GetPhysics()->GetStatus( &dyn );
	//	velocity = dyn.v.len();

	//tdir = ConvertToRadAngles(tdir);

	Vec3 refdir(0,-1,0);

	Matrix44 tm;
	tm.SetIdentity();
	//tm.RotateMatrix_fix( angles );
	tm=Matrix44::CreateRotationZYX(-angles*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 
	tdir = GetTransposed44(tm)*(refdir);

	dir = -tdir;
	tdir = dir;
	tdir *= 10.0f;

	control.momentum = Vec3(0.0f, 0.0f, 0.0f);
	control.impulse = Vec3(0.0f, 0.0f, 0.0f);

//*/
	//drive the glider
	IPhysicalEntity *iboat = GetEntity()->GetPhysics();

	if (cmd.CheckAction(ACTION_MOVE_FORWARD))
	{
		// go down when pressing forward
		Vec3 momentum = (-tdir)^Vec3(0.0f, 0.0f, m_BoatParams.m_fBtTitlSpd);
		control.momentum = vectorf(momentum*dt);
	}
	else 
	if (cmd.CheckAction(ACTION_MOVE_BACKWARD))
	{
		// can't fly back when using the paraglider, but rather
		// go up
		Vec3 momentum = (tdir)^Vec3(0.0f, 0.0f, m_BoatParams.m_fBtTitlSpd);
		control.momentum = vectorf(momentum*dt);
	}

	// to prevent rolling over
	bCanTurn = fabs(dyn.w.z)<.4f;

	if (cmd.CheckAction(ACTION_MOVE_RIGHT) && bCanTurn)
	{

		control.momentum = -Vec3(	-dir.x*m_btVelocity*m_BoatParams.m_fBtTitlTurn*dt,
								-dir.y*m_btVelocity*m_BoatParams.m_fBtTitlTurn*dt,
								fTurnImpuls*dt);		
	}
	else 
	if (cmd.CheckAction(ACTION_MOVE_LEFT) && bCanTurn) 
	{
		control.momentum = Vec3(	-dir.x*m_btVelocity*m_BoatParams.m_fBtTitlTurn*dt,
									-dir.y*m_btVelocity*m_BoatParams.m_fBtTitlTurn*dt,
									fTurnImpuls*dt);
	}
	iboat->Action(&control);

/*
	if( cmd.CheckAction(ACTION_WALK) )	// push it up - for debuggibg
	{
		m_pEntity->SetPos( m_pEntity->GetPos() + Vec3(0,0,10) );
	}
*/
}





//////////////////////////////////////////////////////////////////////////
void CVehicle::ProcessMovement(CXEntityProcessingCmd &cmd)
{

//	if(m_fTimeDelay > 0)	// waite for startup delay
//		return;
	m_cmdLastMove = cmd;

	if (m_Type == VHT_BOAT)
	{
		ProcessMovementBoat2( cmd );
		return;
	}
	if (m_Type == VHT_PARAGLIDER)
	{
		ProcessMovementParaglider( cmd );
		return;
	}


//	unsigned long nFlags = cmd.GetActionFlags();

	//drive the car
	IPhysicalEntity *icar = GetEntity()->GetPhysics();
	if(!icar)	// not phisycalized
	{
		m_pGame->GetSystem()->GetILog()->Log("\002 WARNING car is NOT phisycalized [ %s ]", m_pEntity->GetName());
		return;
	}


	// brakes off - will switch on if breaking
	m_bBreakLightsOn = false;

	pe_action_drive drive;
	pe_status_vehicle status;
	float maxsteer;

	icar->GetStatus(&status);

	IPhysicalWorld *pWorld = m_pGame->GetSystem()->GetIPhysicalWorld();
	int iCurTime = pWorld->GetiPhysicsTime();
	float dt = (iCurTime - m_iPrevMoveTime)*pWorld->GetPhysVars()->timeGranularity;	
//		m_pGame->GetSystem()->GetITimer()->GetFrameTime();
	m_iPrevMoveTime = iCurTime;

	if (m_bAcceleratedLastUpdate)
		m_bAcceleratedLastUpdate = false;

/*
no boost is used
	if (cmd.CheckAction(ACTION_VEHICLE_BOOST))
	{
		drive.bHandBrake = 0;
    drive.pedal = 1;
		TRACE("BOOST!!!!\n");
		if (!m_bAcceleratedFlagSetLastFrame)
		{
			m_bAcceleratedLastUpdate = true;
			m_bAcceleratedFlagSetLastFrame = true;
		}
	}
	else
*/

	m_bIsBreaking = false; //cmd.CheckAction(ACTION_JUMP);

	if (cmd.CheckAction(ACTION_MOVE_FORWARD))
	{
		if (m_DirVelDotProduct>1.0f)
		{
			m_bIsBreaking = true;
		}
		else
		{
			drive.bHandBrake = 0;
			drive.dpedal = m_fPedalSpeed*dt*min(m_fEngineHealth/50.0f,1.0f); //use damage model here, filippo:begin to use damage model when vehicle have half the health.
		}

		if (!m_bAcceleratedFlagSetLastFrame)
		{
			m_bAcceleratedLastUpdate = true;
			m_bAcceleratedFlagSetLastFrame = true;
		}
	}
	else if (cmd.CheckAction(ACTION_MOVE_BACKWARD))
	{
		//if we are trying to brake with footbrake use in any case the handbrake
		//TODO? make this a parameter of the vehicle?
		if (m_DirVelDotProduct<-1.0f)
		{
			m_bIsBreaking = true;
		}
		else
		{
			drive.bHandBrake = 0;
			drive.dpedal = -m_fPedalSpeed*dt*min(m_fEngineHealth/50.0f,1.0f); //use damage model here, filippo:begin to use damage model when vehicle have half the health.
			m_bBreakLightsOn = true;
		}
	}
	else
	{
		if(drive.pedal!=1){
		
			drive.bHandBrake = 0;
			drive.pedal = 0;
	
		m_bAcceleratedFlagSetLastFrame = false;
		}
	}

	if (cmd.CheckAction( ACTION_WALK) || !HasDriver() || cmd.CheckAction(ACTION_JUMP))
		m_bIsBreaking = true;

	/*if (nFlags & ACTIONFLAG_JUMP)
	{
		m_bIsBreaking = !m_bIsBreaking;
		cmd.RemoveActionFlags(ACTIONFLAG_JUMP);
	}*/
	

	drive.steer = status.steer;
	float vel = status.vel.len();
	maxsteer = m_fv0MaxSteer + m_fkvMaxSteer*vel;
	//maxsteer = 40.0f;
	//if (maxsteer<40) maxsteer=40;
	maxsteer *= (float)(gf_PI)/180.0f;
	float maxpedal = 1.0f;
	if (status.vel.len2()>sqr(m_fPedalLimitSpeed))
		maxpedal -= fabs_tpl(status.steer)/maxsteer*(1.0f-m_fMaxSteeringPedal);
	if (status.pedal>maxpedal)
		drive.pedal = maxpedal;
	else if (status.pedal<-maxpedal)
		drive.pedal = -maxpedal;

	// rotate the steering wheel (if any)
	int nSlot=m_pEntity->GetSteeringWheelSlot();
	if (nSlot>=0)
	{
		// set the correct position 
		// since the steering wheel is at pos. 0,0,0
		Vec3 vCurrAngles;
		m_pEntity->GetObjectAngles(nSlot,vCurrAngles);
		vCurrAngles.y=drive.steer*180;

		float deltaa = vCurrAngles.y-m_flastwheelrotation;

		//if the delta is above 180 deg snap it to the correct angles, otherwise could be a crazy rotating wheel in few cases.
		if (fabs(deltaa)<180)
		{
			m_flastwheelrotation += deltaa*min(dt*5.0f,1.0f);//5 degree at sec
			vCurrAngles.y = m_flastwheelrotation;
		}
		else
			m_flastwheelrotation = vCurrAngles.y;

		Vec3 vRotPointObjSpace;
		m_pEntity->GetHelperPosition("steering_pivot",vRotPointObjSpace,true);
		m_pEntity->SetObjectPos(nSlot,vRotPointObjSpace);
		//m_pEntity->SetObjectAngles(nSlot,vCurrAngles);
		m_pEntity->SetObjectAngles(nSlot,vCurrAngles);
	}

	/*float	curSteerSpeed = m_fSteerSpeed*(1.0f+sqrt_tpl(fabs_tpl(status.steer)/maxsteer)*m_fSteerSpeedVelSCale);// - vel*m_fSteerSpeedVelSCale;
	if(curSteerSpeed<m_fSteerSpeedMin)
		curSteerSpeed = m_fSteerSpeedMin;*/

	//get the normalized speed delta, between 0-30
	float speeddelta = vel;
	if (speeddelta > 30.0f) speeddelta = 30.0f;
	speeddelta /= 30.0f;

	//use a steerspeed between min and max velocity values
	float steerdelta = m_fSteerSpeed - m_fSteerSpeedMin;
	float	curSteerSpeed = m_fSteerSpeedMin + steerdelta * speeddelta;
	//the same per the steerscale
	float steerscaledelta = m_fsteerspeed_scale - m_fsteerspeed_scale_min;
	float sensitivitybyspeed = m_fsteerspeed_scale_min + steerscaledelta * speeddelta;

	//GetISystem()->GetILog()->Log("steerSpeed:%f,steer:%f",curSteerSpeed*sensitivitybyspeed,drive.steer);	

	//the second condition because we want steering to go from left to right quickly, so in this case we use steerrelaxation.
	if (cmd.CheckAction(ACTION_MOVE_RIGHT) && drive.steer>-0.1f)
	{
		drive.steer += curSteerSpeed*((float)(gf_PI)/180.0f)*dt*sensitivitybyspeed;
		if (drive.steer > maxsteer)
			drive.steer = maxsteer;
	} 
	//the second condition because we want steering to go from left to right quickly, so in this case we use steerrelaxation.
	else if (cmd.CheckAction(ACTION_MOVE_LEFT) && drive.steer<0.1f) 
	{
		drive.steer -= curSteerSpeed*((float)(gf_PI)/180.0f)*dt*sensitivitybyspeed;
		if (drive.steer < -maxsteer)
			drive.steer = -maxsteer;
	}
	else //if (/*status.vel.len()*/vel>0.01f && status.steer!=0)
	{
	
		/*if (status.steer > 0)
		{
			//drive.steer -= curSteerSpeed*((float)(gf_PI)/180.0f)*dt*.5f;
			//if(drive.steer<0)
			//	drive.steer = 0;
			drive.dsteer = -dt*(m_fv0SteerRelaxation+m_fkvSteerRelaxation*vel)*((float)(gf_PI)/180.0f);
		}
		else
		{
			//drive.steer += curSteerSpeed*((float)(gf_PI)/180.0f)*dt*.5f;
			//if(drive.steer>0)
			//	drive.steer = 0;
			drive.dsteer = dt*(m_fv0SteerRelaxation+m_fkvSteerRelaxation*vel)*((float)(gf_PI)/180.0f);
		}

		if (status.steer*drive.dsteer<0 && fabs_tpl(status.steer)<fabs_tpl(drive.dsteer))
			drive.steer = drive.dsteer = 0;*/

		//get the delta necessary to put the steer in the center position, and change the steer in smooth way.
		float deltaa = 0-drive.steer;
		drive.steer += deltaa*min(dt*m_fv0SteerRelaxation*gf_PI/180.0f,1.0f);
	}

	pe_params_car pc;
	pc.axleFriction = m_fAxleFriction;
	icar->SetParams(&pc);


	if (m_bIsBreaking)
	{
		drive.bHandBrake = 1;	// handbrake brake always
		m_bBreakLightsOn = true;
/*
		if (vel<m_fBrakeVelThreshold)
			drive.bHandBrake = 1;
		else
		{
			pc.axleFriction = m_fBrakeAxleFriction;
			icar->SetParams(&pc);
		}
*/
	}

//drive.bHandBrake = 1;


//GetGame()->GetSystem()->GetILog()->Log("\001 %.3f", drive.dpedal);

	icar->Action(&drive);	
}

//////////////////////////////////////////////////////////////////////////
void CVehicle::GetWheelStatus(int nWheel, pe_status_wheel *pStatus)
{
	if (!pStatus)
		return;
	IPhysicalEntity *icar = GetEntity()->GetPhysics();
	if (!icar)
		return;
	pStatus->iWheel=nWheel;
	icar->GetStatus(pStatus);
}

//////////////////////////////////////////////////////////////////////////
void CVehicle::SetDrivingParams(float pedalspeed,float steerspeed,float v0maxsteer,float kvmaxsteer,float v0steerrelax,float kvsteerrelax,
																float brake_vel_threshold,float brake_axle_friction,
																float steerspeedVelScale, float steerspeedMin,
																float maxSteeringPedal, float pedalLimitSpeed, 
																float fhandbrakingvalue,float fmaxbrakingfrictionnodriver,
																float fhandbrakingvaluenodriver,float fstabilizejump,
																float fsteerspeedscale, float fsteerspeedscalemin)
{
	m_fPedalSpeed = pedalspeed; m_fSteerSpeed = steerspeed; m_fv0MaxSteer = v0maxsteer; m_fkvMaxSteer = kvmaxsteer;
	m_fv0SteerRelaxation = v0steerrelax; m_fkvSteerRelaxation = kvsteerrelax;
	m_fBrakeVelThreshold = brake_vel_threshold; m_fBrakeAxleFriction = brake_axle_friction;
	m_fSteerSpeedVelSCale = steerspeedVelScale;
	m_fSteerSpeedMin = steerspeedMin;
	m_fMaxSteeringPedal = maxSteeringPedal;
	m_fPedalLimitSpeed = pedalLimitSpeed;
	m_fhandbraking_value = fhandbrakingvalue;
	m_fhandbraking_value_nodriver = fhandbrakingvaluenodriver;
	m_fstabilizejump = fstabilizejump;
	m_fMaxBrakingFrictionNoDriver = 1.0;
	m_fsteerspeed_scale = fsteerspeedscale;
	m_fsteerspeed_scale_min = fsteerspeedscalemin;

	IPhysicalEntity *icar = NULL;

	if (GetEntity())
		icar = GetEntity()->GetPhysics();

	if (icar)
	{
		pe_params_car pc;
		icar->GetParams(&pc);
		m_fAxleFriction = pc.axleFriction;

		//
		m_fMaxBrakingFriction = pc.maxBrakingFriction;

		if (fmaxbrakingfrictionnodriver<=0)// if there is no fmaxbrakingfrictionnodriver specified use the same friction in any case
			m_fMaxBrakingFrictionNoDriver = m_fMaxBrakingFriction;
		else
			m_fMaxBrakingFrictionNoDriver = fmaxbrakingfrictionnodriver;

		//and at first time set the car to nodriver friction.
		pc.maxBrakingFriction = m_fMaxBrakingFrictionNoDriver;
		//GetISystem()->GetILog()->Log("%s change friction to %f (first time)",m_pEntity->GetName(),pc.maxBrakingFriction);
		icar->SetParams(&pc);

		m_bUsingNoDriverFriction = true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool CVehicle::QueryContainerInterface(ContainerInterfaceType desired_interface, void **ppInterface )
{
	if (desired_interface == CIT_IVEHICLE)
	{
		*ppInterface = (void *) this;
		return true;
	}
	else
	{
		*ppInterface = 0;
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
bool CVehicle::Write(CStream& stm,EntityCloneState *cs)
{
	if(m_pEntity->IsHidden())
		return true;

	if (!cs || cs->m_bOffSync)
	{
		unsigned char ucHealth = (int)(m_fEngineHealth*2+0.5f);
		stm.Write(ucHealth);
		if (m_pGame->UseFixedStep() && (m_Type == VHT_BOAT))
		{
			stm.Write(true);
			stm.Write(m_fPrevFwvTilt);
			stm.Write(m_fPrevFwvSpeedLen);
			stm.Write(m_fSimTime);
		}
		else
			stm.Write(false);

		if (cs && !cs->m_bLocalplayer)
		{
			stm.Write(true);
			stm.Write(m_cmdLastMove.CheckAction(ACTION_MOVE_FORWARD));
			stm.Write(m_cmdLastMove.CheckAction(ACTION_MOVE_BACKWARD));
			stm.Write(m_cmdLastMove.CheckAction(ACTION_MOVE_LEFT));
			stm.Write(m_cmdLastMove.CheckAction(ACTION_MOVE_RIGHT));
			stm.Write(m_cmdLastMove.CheckAction(ACTION_WALK));
			stm.Write(m_cmdLastMove.CheckAction(ACTION_JUMP));
		}
		else
			stm.Write(false);
	}

	IScriptSystem *pScScriptSystem=m_pGame->GetScriptSystem();
	//CScriptObjectStream stmObj;
	//stmObj.Create(pScScriptSystem,&stm);
	m_ScriptStream.Attach(&stm);
	pScScriptSystem->BeginCall(GetEntity()->GetEntityClassName(),"OnWrite");
	pScScriptSystem->PushFuncParam(GetEntity()->GetScriptObject());
	pScScriptSystem->PushFuncParam(m_ScriptStream.GetScriptObject());
	pScScriptSystem->EndCall();

	stm.Write(m_bHeadLightsOn);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CVehicle::Read(CStream& stm)
{

	if(m_pEntity->IsHidden())
		return true;

	IPhysicalEntity *icar = GetEntity()->GetPhysics();
	pe_params_flags pf;
	icar->GetParams(&pf);
	if (!(pf.flags & pef_checksum_received))
	{
		unsigned char ucHealth;
		stm.Read(ucHealth);
		m_fEngineHealth =  ucHealth*0.5f;
		bool bnz; stm.Read(bnz);
		if (bnz)
		{
			stm.Read(m_fPrevFwvTilt);
			stm.Read(m_fPrevFwvSpeedLen);
			stm.Read(m_fSimTime);
		}
		if ((m_Type == VHT_BOAT)) 
		{
			pe_status_dynamics sd;
			m_pEntity->GetPhysics()->GetStatus( &sd );
			m_btVelocity = sd.v.len();
		}
		
		stm.Read(bnz);
		if (bnz)
		{
			m_cmdLastMove.Reset();
			stm.Read(bnz); if (bnz)
				m_cmdLastMove.AddAction(ACTION_MOVE_FORWARD);
			stm.Read(bnz); if (bnz)
				m_cmdLastMove.AddAction(ACTION_MOVE_BACKWARD);
			stm.Read(bnz); if (bnz)
				m_cmdLastMove.AddAction(ACTION_MOVE_LEFT);
			stm.Read(bnz); if (bnz)
				m_cmdLastMove.AddAction(ACTION_MOVE_RIGHT);
			stm.Read(bnz); if (bnz)
				m_cmdLastMove.AddAction(ACTION_WALK);
			stm.Read(bnz); if (bnz)
				m_cmdLastMove.AddAction(ACTION_JUMP);
		}

		UpdateCamera(0,true);
	}

	IScriptSystem *pScScriptSystem=m_pGame->GetScriptSystem();
	/*CScriptObjectStream stmObj;
	stmObj.Create(pScScriptSystem,&stm);*/
	m_ScriptStream.Attach(&stm);
	pScScriptSystem->BeginCall(GetEntity()->GetEntityClassName(),"OnRead");
	pScScriptSystem->PushFuncParam(GetEntity()->GetScriptObject());
	pScScriptSystem->PushFuncParam(m_ScriptStream.GetScriptObject());
	pScScriptSystem->EndCall();

	if(icar && m_pGame && !m_pGame->IsMultiplayer())	// not phisycalized
	{	// [Anton] - don't put vehicles on brake during network synchronization!

		if ((m_Type == VHT_BOAT) || m_Type == VHT_PARAGLIDER)
		{
			pe_action_set_velocity control;
//			pe_action_impulse control;
			icar->Action(&control);
		}
		else
		{
			pe_action_drive drive;
			drive.pedal = 0;
			drive.bHandBrake = 1;
			icar->Action(&drive);
		}
	}

	stm.Read( m_bHeadLightsOn );

	return true;
}


void CVehicle::SetEngineHealth(float fEngineHealth, bool bScheduled)
{
	if (m_pGame->IsMultiplayer() && m_pGame->UseFixedStep() && !bScheduled)
	{
		if (m_pGame->IsServer())
			m_pGame->ScheduleEvent(-1, m_pEntity,fEngineHealth);
		return;
	}
	m_fEngineHealth = fEngineHealth;
}

//////////////////////////////////////////////////////////////////////////
//void CVehicle::SetWaterVehicleParameters(float fBSpeed,float fBTurn)
void CVehicle::SetWaterVehicleParameters(	WATER_VEHICLE_PARAMS &wvpar, bool bFlyingVehicle)
{
	m_BoatParams = wvpar;

	if (bFlyingVehicle)
		m_Type = VHT_PARAGLIDER;
	else
		m_Type = VHT_BOAT;
}


//////////////////////////////////////////////////////////////////////////
bool	CVehicle::HasDriver()
{
	return (m_DriverID!=0);
}

//////////////////////////////////////////////////////////////////////////
void	CVehicle::WakeupPhys( )
{
//	if (!m_bPhysAwaike && GetEntity()->GetPhysics())
	// wake up always when visible or used - otherwise some wierd stuff happens - boats don't move
	if (GetEntity()->GetPhysics())
	{
		pe_action_awake aa;
		aa.bAwake = 1;
		GetEntity()->GetPhysics()->Action(&aa);
		m_bPhysAwaike = true;
	}
}


//////////////////////////////////////////////////////////////////////////
void	CVehicle::AddUser(int entId)
{
	WakeupPhys( );
	m_UsersList.push_back(entId);
}


//////////////////////////////////////////////////////////////////////////
void	CVehicle::RemoveUser( int entId )
{
UsersList::iterator	itr = std::find(m_UsersList.begin(), m_UsersList.end(), entId );
	if( itr != m_UsersList.end() )
		m_UsersList.erase( itr );
}

void CVehicle::PreloadInstanceResources(Vec3d vPrevPortalPos, float fPrevPortalDistance, float fTime)
{
#pragma message( "Warning: Preloading of render resources is not implemented in " __FUNCTION__ )
	// for all objects what will be used for rendering call 
	// ICryCharInstance::PreloadResources or
	// IStatObj::PreloadResources
	// Ask Vlad for details
}

//
//--------------------------------------------------------------------------------------
void	CVehicle::InitHeadLight( const char* sImg, const char* sShader )
{
	SAFE_DELETE( m_pHeadLight );
	//ReleaseLight( m_pHeadLight );
	//m_pHeadLight = 0;

	if(sImg && sImg[0])		// no texture - no light!!!
	{
		m_pHeadLight = new CDLight();
		m_pHeadLight->m_fAnimSpeed = 0;
		int nFlags2 = FT2_FORCECUBEMAP;
//    if (bUseAsCube)
//	    nFlags2 |= FT2_REPLICATETOALLSIDES;
//    if (fAnimSpeed)
//      nFlags2 |= FT2_CHECKFORALLSEQUENCES;
		m_pHeadLight->m_pLightImage = m_pGame->GetSystem()->GetIRenderer()->EF_LoadTexture(sImg, 0, nFlags2, eTT_Cubemap);
		m_pHeadLight->m_Flags = DLF_PROJECT | DLF_IGNORE_OWNER;

		if(sShader && sShader[0])
			m_pHeadLight->m_pShader = m_pGame->GetSystem()->GetIRenderer()->EF_LoadShader((char*)sShader, eSH_World);
	}

}

//
//--------------------------------------------------------------------------------------
void	CVehicle::InitFakeLight( CDLight** pLight, const char* sShader )
{
	if (*pLight)
		delete *pLight;

	if(sShader && sShader[0])		// no shader - no light!!!
	{
	*pLight = new CDLight();
		(*pLight)->m_Flags = DLF_POINT|DLF_FAKE | DLF_IGNORE_OWNER;
		(*pLight)->m_pShader = m_pGame->GetSystem()->GetIRenderer()->EF_LoadShader((char*)sShader, eSH_World);
	}
}

//
//--------------------------------------------------------------------------------------
void	CVehicle::UpdateLights( )
{
	if(m_fEngineHealth<1)	// no lights when destroyed
		return;

//	float	totalLightScale = 1.0f - m_pGame->GetSystem( )->GetI3DEngine()->GetAmbientLightAmountForEntity(m_pEntity)*2.0f;

	if(m_bAutoLights && m_bAIDriver)
	{
		if(!HasDriver() )
			return;
//		float	totalLightScale = m_pGame->GetSystem( )->GetI3DEngine()->GetAmbientLightAmountForEntity(m_pEntity);
//		if( totalLightScale>.6f )
//			return;
		m_bHeadLightsOn = true;
	}
	//
	// adding the light
//	if(0)
	if(m_bHeadLightsOn)
	{
		if(m_pHeadLight)
		{
			m_pHeadLight->m_fLightFrustumAngle = 45;
			m_pHeadLight->m_fRadius = 50;

			m_pEntity->GetHelperPosition(m_HeadLightHelper.c_str(), m_pHeadLight->m_Origin );
			m_pHeadLight->m_ProjAngles = Vec3d(m_pEntity->GetAngles().y,m_pEntity->GetAngles().x,m_pEntity->GetAngles().z+90);
			
		//	m_pHeadLight->m_Color = CFColor(totalLightScale,totalLightScale,totalLightScale, 1.0f);
		//	m_pHeadLight->m_SpecColor = CFColor(totalLightScale,totalLightScale,totalLightScale);

			m_pHeadLight->m_Color = CFColor(1.f,1.f,1.f, 1.0f);
			m_pHeadLight->m_SpecColor = CFColor(1.f,1.f,1.f);

	//m_pHeadLight->m_pShader = NULL;

			m_pHeadLight->m_Flags = DLF_PROJECT | DLF_LIGHTSOURCE | DLF_IGNORE_OWNER;
			m_pGame->GetSystem()->GetI3DEngine()->AddDynamicLightSource(*m_pHeadLight, GetEntity());
		}

		// flares
		UpdateFakeLight(m_pHeadLightLeft, m_HeadLightHelperLeft.c_str());
		UpdateFakeLight(m_pHeadLightRight, m_HeadLightHelperRight.c_str());
	}

	if( !HasDriver() )
		return;
	//
	// adding flares on backlights if breaking
	if(m_bBreakLightsOn)
	{
		UpdateFakeLight(m_pBreakLightLeft, m_BackLightHelperLeft.c_str());
		UpdateFakeLight(m_pBreakLightRight, m_BackLightHelperRight.c_str());
	}
}

//
//--------------------------------------------------------------------------------------
void	CVehicle::UpdateFakeLight( CDLight* light, const char* sHelper )
{

	if( light == NULL ) return;
	light->m_fLightFrustumAngle = 30;
	light->m_fRadius = 5;

	m_pEntity->GetHelperPosition(sHelper, light->m_Origin);
	light->m_ProjAngles = Vec3d(m_pEntity->GetAngles().y,m_pEntity->GetAngles().x,m_pEntity->GetAngles().z+90);
	
//	m_pHeadLight->m_Color = CFColor(totalLightScale,totalLightScale,totalLightScale, 1.0f);
//	m_pHeadLight->m_SpecColor = CFColor(totalLightScale,totalLightScale,totalLightScale);

	light->m_Color = CFColor(1.f,1.f,1.f, 1.0f);
	light->m_SpecColor = CFColor(1.f,1.f,1.f);

	light->m_Flags = DLF_FAKE | DLF_LIGHTSOURCE | DLF_PROJECT | DLF_IGNORE_OWNER;

//	m_pHeadLight->m_Flags = DLF_PROJECT | DLF_LIGHTSOURCE;
	m_pGame->GetSystem()->GetI3DEngine()->AddDynamicLightSource(*light, GetEntity(), light->m_nEntityLightId);


}


//
//--------------------------------------------------------------------------------------
void	CVehicle::SwitchLights( int light )
{
	if(light == 1)
		m_bHeadLightsOn = true;
	else if(light == 0)
		m_bHeadLightsOn = false;
	else
		m_bHeadLightsOn = !m_bHeadLightsOn;
}

//
//--------------------------------------------------------------------------------------

float CVehicle::GetLightRadius()
{
	if(m_pHeadLight && m_bHeadLightsOn)
		return m_pHeadLight->m_fRadius;
	
	return 0;
}

//
//--------------------------------------------------------------------------------------
void CVehicle::GetFirePosAngles(Vec3d& firePos, Vec3d& fireAngles)
{
//	if(m_sWeaponName.empty())
//		return;

//	firePos = GetEntity()->GetPos()+Vec3(0,0,3.5);
//	GetEntity()->GetHelperPosition("turret",firePos);	

	firePos = m_vWpnPos;
	fireAngles = m_vWpnAng;
	return;
}


//
//--------------------------------------------------------------------------------------
const string& CVehicle::GetWeaponName( CPlayer::eInVehiclestate state )
{
	switch(state)
	{
	case CPlayer::PVS_DRIVER:
		return m_sAutoWeaponName;
	case CPlayer::PVS_GUNNER:
		return m_sMountedWeaponName;
	}
	return m_sNoWeaponName;
}



//
//--------------------------------------------------------------------------------------

void CVehicle::SetCameraParams(Vec3 vCamStiffness[2],Vec3 vCamLimits[2],float fCamDamping,float fMaxCamTimestep,
															 float fCamSnapDist,float fCamSnapVel)
{
	m_vCamStiffness[0] = vCamStiffness[0];
	m_vCamStiffness[1] = vCamStiffness[1];
	m_vCamLimits[0] = vCamLimits[0];
	m_vCamLimits[1] = vCamLimits[1];
  m_fCamDamping = fCamDamping;
	m_fMaxCamTimestep = fMaxCamTimestep;
	m_fCamSnapDist = fCamSnapDist;
	m_fCamSnapVel = fCamSnapVel;
}


void CVehicle::ResetCamera(bool bUpdateCamera,const char *pHelperName)
{
	if (bUpdateCamera && GetEntity()->GetPhysics())
	{
		pe_status_pos sp;
		pe_status_dynamics sd;
		GetEntity()->GetPhysics()->GetStatus(&sp);
		GetEntity()->GetPhysics()->GetStatus(&sd);
		GetEntity()->GetHelperPosition(pHelperName,m_vCamHelperPos,true);	
		m_vPrevCamTarget = m_vCamPos = sp.pos + sp.q*m_vCamHelperPos;
		m_vCamVel = sd.v + (sd.w^m_vCamPos-sd.centerOfMass);
		m_bUpdateCamera = true;
	}
	else
		m_bUpdateCamera = false;
}


void CVehicle::UpdateCamera(float fTimeStep, bool bSynchronizing)
{
	if (m_bUpdateCamera)
	{
		pe_status_pos sp;
		GetEntity()->GetPhysics()->GetStatus(&sp);
		Vec3 posTarget,posTargetNew,posDelta,posDeltaLoc,posDeltaLocNorm,velTarget;
		Matrix33 R = Matrix33(sp.q);
		posTargetNew = sp.pos + R*m_vCamHelperPos;

		if (!bSynchronizing)
		{
			if (fTimeStep==0 || (velTarget = (posTargetNew-m_vPrevCamTarget)/fTimeStep).len2()>sqr(200) || 
					m_vCamStiffness[0].len2()+m_vCamStiffness[1].len2()==0)
			{
				m_vCamPos = posTargetNew;
				pe_status_dynamics sd;
				GetEntity()->GetPhysics()->GetStatus(&sd);
				m_vCamVel = sd.v + (sd.w^m_vCamPos-sd.centerOfMass);
			}
			else
			{
				float ks,fStep;
				int i,j;

				posTarget = m_vPrevCamTarget;
				do 
				{
					fStep = min(fTimeStep,m_fMaxCamTimestep);
					posTarget += velTarget*fStep;
					m_vCamPos += m_vCamVel*fStep;

					if ((posDelta = m_vCamPos-posTarget).len2()>sqr(m_fCamSnapDist))
					{
						posDeltaLocNorm = (posDeltaLoc=posDelta*R).GetNormalized();
						for(i=0,ks=0; i<3; i++)
						{
							j = isneg(posDeltaLoc[i])^1;
							if (m_vCamStiffness[j][i]==0)
								m_vCamPos -= R.GetColumn(i)*posDeltaLoc[i];
							else if (fabs_tpl(posDeltaLoc[i])>m_vCamLimits[j][i])
								m_vCamPos -= R.GetColumn(i)*(posDeltaLoc[i]+m_vCamLimits[j][i]*(j*2-1));
							ks += sqr(posDeltaLocNorm[i])*m_vCamStiffness[j][i];
						}
						//m_vCamVel -= posDelta*((ks + 2.0f*sqrt_tpl(ks)*m_fCamDamping*((posDelta*(m_vCamVel-velTarget))/posDelta.len2()))*fStep);
						m_vCamVel -= posDelta*(ks*fStep);
						m_vCamVel += (velTarget-m_vCamVel)*(m_fCamDamping*fStep);
					}
					else if ((m_vCamVel-velTarget).len2()<sqr(m_fCamSnapVel))
					{
						m_vCamPos = posTarget;
						m_vCamVel = velTarget;
					}
				}	while ((fTimeStep-=fStep)>0.0001f);
			}
			m_vPrevCamTarget = posTargetNew;
		}
		else
		{
			m_vCamPos += (posTargetNew-m_vPrevCamTarget);
			m_vPrevCamTarget = posTargetNew;
		}
	}
}

//
//--------------------------------------------------------------------------------------
void CVehicle::WeaponState(int userId, bool shooting, int fireMode)
{
CPlayer* theShooter = GetUserInState( CPlayer::PVS_PASSENGER);
	
	// it's a passenger - has it's own weapon, don't affect vehicle's weapon
	if( theShooter )	
		return;

	if(shooting)
	{
		// use appropriate fire animation, depending on fire mode (MG/rockets)
		if(fireMode == 0)
			m_pEntity->StartAnimation(0, "default" );
	}
	else
	{
		theShooter = GetUserInState( CPlayer::PVS_GUNNER );
		if(theShooter)
			if(theShooter->GetEntity()->GetId()!=userId)	
				// there is still gunner in - don't stop ani
				return;
		ICryCharInstance *pCharacter = m_pEntity->GetCharInterface()->GetCharacter(0);
		if(pCharacter)
			pCharacter->StopAnimation(0);
	}
}

//[filippo]
void CVehicle::AdditionalPhysics(IPhysicalEntity *pcar,float fdelta,bool bforcebreaking)
{
	if (pcar==NULL || m_fEngineHealth<=0)
		return;

	pe_status_vehicle vstatus;
	pe_status_dynamics dstatus;

	pcar->GetStatus(&vstatus);
	pcar->GetStatus(&dstatus);

	float fvelmod = dstatus.v.len();

	if (vstatus.bWheelContact>1)
	{
		float handbrake;
		pe_action_set_velocity action;

		if (m_bUsingNoDriverFriction)
			handbrake = m_fhandbraking_value_nodriver;
		else
			handbrake = m_fhandbraking_value;

		if ((m_bIsBreaking||bforcebreaking||vstatus.bHandBrake) && fvelmod>0 && handbrake>0 
				&& vstatus.nActiveColliders==0) // [Anton] don't force unaccounted for excessive damping when contacting with other
																				// objects, for ex. platform in the boat puzzle
		{
			//GetISystem()->GetILog()->Log("braking");

			action.v[0] = dstatus.v[0];
			action.v[1] = dstatus.v[1];
			action.v[2] = dstatus.v[2];
						
			float fnewvel = fvelmod - handbrake*fdelta;
			
			if (fnewvel<=0)
				fnewvel = 0;
			
			action.v = action.v / fvelmod * fnewvel;

			pcar->Action(&action);
		}

		/*action.w[0] = dstatus.w[0]*0.01f;
		action.w[1] = dstatus.w[1]*0.01f;
		action.w[2] = dstatus.w[2];

		pcar->Action(&action);*/
	}
	else if (m_fstabilizejump>0)//flying? try to stabilize the rolling	
	//if (m_fstabilizejump>0)//flying? try to stabilize the rolling
	{
		Matrix44 tm;
		tm.SetIdentity();
		tm = Matrix44::CreateRotationZYX(-m_pEntity->GetAngles()*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 
			
		Vec3 vUpDir = GetTransposed44(tm)*Vec3d(0,0,1);
		Vec3 vFwdDir = GetTransposed44(tm)*Vec3d(0,-1,0);

		//GetISystem()->GetILog()->Log("a:%.1f,%.1f,%.1f",m_pEntity->GetAngles().x,m_pEntity->GetAngles().y,m_pEntity->GetAngles().z);

		if (vUpDir.z > 0.5f) 
		{
			Vec3 vStabilizedangles = m_pEntity->GetAngles();
			vStabilizedangles.y = 0;

			tm.SetIdentity();
			tm = Matrix44::CreateRotationZYX(-vStabilizedangles*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 
			Vec3 vert = GetTransposed44(tm)*Vec3d(0,0,1);
				
			pe_action_impulse am;
			
			//Vec3 vert(vFwdDir.x,vFwdDir.y,1.0f);

			float coef = m_fstabilizejump*fdelta;

			/*if (vstatus.bWheelContact>1)
			{
				coef *= 2.0f;
			}*/
			
			//vert.Normalize();

			Vec3 dv = vert^vUpDir;

			coef *= 1.0f + dv.len2();

			am.momentum.x = -(dv.x+dstatus.w.x)*coef;
			am.momentum.y = -(dv.y+dstatus.w.y)*coef;
			am.momentum.z = 0;

			//GetISystem()->GetILog()->Log("momentum: %.1f %.1f %.1f", am.momentum.x, am.momentum.y, am.momentum.z);

			pcar->Action(&am);
		}

		//GetISystem()->GetILog()->Log("w:%.1f,%.1f,%.1f , fwd:%.1f,%.1f,%.1f",dstatus.w.x,dstatus.w.y,dstatus.w.z,vFwdDir.x,vFwdDir.y,vFwdDir.z);
	}
}


void CVehicle::SaveAIState(CStream & stm, CScriptObjectStream & scriptStream)
{

	IAIObject *pObject = m_pEntity->GetAI();
	if (pObject)		
		pObject->Save(stm);

	IScriptSystem *pScriptSystem = m_pGame->GetSystem()->GetIScriptSystem();
	HSCRIPTFUNCTION	saveOverallFunction=NULL;
	if( m_pEntity->GetScriptObject() && m_pEntity->GetScriptObject()->GetValue("OnSaveOverall", saveOverallFunction) )
	{
		pScriptSystem->BeginCall(saveOverallFunction);
		pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
		pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
		pScriptSystem->EndCall();
	}

}

void CVehicle::LoadAIState(CStream & stm, CScriptObjectStream & scriptStream)
{

	IAIObject *pObject = m_pEntity->GetAI();
	if (pObject)
		pObject->Load(stm);

	IScriptSystem *pScriptSystem = m_pGame->GetSystem()->GetIScriptSystem();
	HSCRIPTFUNCTION	saveOverallFunction=NULL;
	if( m_pEntity->GetScriptObject() && m_pEntity->GetScriptObject()->GetValue("OnLoadOverall", saveOverallFunction) )
	{
		pScriptSystem->BeginCall(saveOverallFunction);
		pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
		pScriptSystem->PushFuncParam(scriptStream.GetScriptObject());
		pScriptSystem->EndCall();
	}
}





void CVehicle::OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority, 
	EntityCloneState &inoutCloneState) const
{
	IEntity *pLocPlayerEnt = GetISystem()->GetIEntitySystem()->GetEntity(idViewerEntity);

	if(pLocPlayerEnt)		// if there is a local player
	{
		IEntityContainer *pCnt = pLocPlayerEnt->GetContainer();

		if(pCnt)
		{
			CPlayer *pLocPlayer;

			// if this vehicle has the player as passanger
			if(pCnt->QueryContainerInterface(CIT_IPLAYER,(void **)&pLocPlayer))		// and it's a player
			{
				CVehicle *pVehiclepLocPlayer = pLocPlayer->GetVehicle();

				if(pVehiclepLocPlayer && pVehiclepLocPlayer->GetEntity()->GetId()==m_pEntity->GetId())	// that is inside this vehicle
					inoutPriority += 350000; // boost the vehicle the player rides in
			}
		}
	}
}

