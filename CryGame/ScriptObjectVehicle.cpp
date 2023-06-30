
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptObjectVehicle.cpp: implementation of the CScriptObjectVehicle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XPlayer.h"
#include "ScriptObjectVehicle.h"
#include "xvehicle.h"
#include "ScriptObjectVector.h"
#include "IAgent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

_DECLARE_SCRIPTABLEEX(CScriptObjectVehicle)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptObjectVehicle::CScriptObjectVehicle()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CScriptObjectVehicle::~CScriptObjectVehicle()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Initializes the script-object and makes it available for the scripts.
		@param pScriptSystem Pointer to the ScriptSystem-interface
		@param pEntitySystem Pointer to the EntitySystem-interface
*/
bool CScriptObjectVehicle::Create(IScriptSystem *pScriptSystem, IEntitySystem *pEntitySystem)
{
	m_pEntitySystem = pEntitySystem;
	Init(pScriptSystem,this);
	m_soContactPtVec.Create(pScriptSystem);
	m_soSlipVelVec.Create(pScriptSystem);
	m_pScriptThis->RegisterParent(this);

	m_GetVehicleStatus.Create( pScriptSystem );
	m_GetWheelStatus.Create( pScriptSystem );

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CScriptObjectVehicle::InitializeTemplate(IScriptSystem *pSS)
{
	_ScriptableEx<CScriptObjectVehicle>::InitializeTemplate(pSS);
	REG_FUNC(CScriptObjectVehicle,SetUser);
	REG_FUNC(CScriptObjectVehicle,ReleaseUser);
	//REG_FUNC(CScriptObjectVehicle,SetDriver);
	//REG_FUNC(CScriptObjectVehicle,ReleaseDriver);
	REG_FUNC(CScriptObjectVehicle,GetWheelStatus);
	REG_FUNC(CScriptObjectVehicle,GetVehicleVelocity);
	REG_FUNC(CScriptObjectVehicle,HasAccelerated);
	REG_FUNC(CScriptObjectVehicle,IsBreaking);
	REG_FUNC(CScriptObjectVehicle,WakeUp);	
	REG_FUNC(CScriptObjectVehicle,SetDrivingParameters);	
	REG_FUNC(CScriptObjectVehicle,SetCameraParameters);	
	REG_FUNC(CScriptObjectVehicle,SetWaterVehicleParameters);	
	REG_FUNC(CScriptObjectVehicle,SetVehicleEngineHealth);	
	//REG_FUNC(CScriptObjectVehicle,Explode);	
	REG_FUNC(CScriptObjectVehicle,SetGravity);	
	REG_FUNC(CScriptObjectVehicle,GetVertDeviation);	
	REG_FUNC(CScriptObjectVehicle,InitLights);
	REG_FUNC(CScriptObjectVehicle,EnableLights);
	REG_FUNC(CScriptObjectVehicle,SetWeaponLimits);
	REG_FUNC(CScriptObjectVehicle,SetWeaponName);
//	REG_FUNC(CScriptObjectVehicle,ShakePassengers);
	REG_FUNC(CScriptObjectVehicle,GetVehicleStatus);
	REG_FUNC(CScriptObjectVehicle,AnimateUsers);
	REG_FUNC(CScriptObjectVehicle,HandBreak);

	REG_FUNC(CScriptObjectVehicle,AnimateMountedWeapon);

	AllowPropertiesMapping(pSS);
	RegisterProperty( "velocity",PROPERTY_TYPE_FLOAT,offsetof(CVehicle,m_btVelocity));
	RegisterProperty( "inwater",PROPERTY_TYPE_BOOL,offsetof(CVehicle,m_btInWater));
	RegisterProperty( "turnState",PROPERTY_TYPE_INT,offsetof(CVehicle,m_TurnState));


	RegisterProperty( "engineHealthReadOnly",PROPERTY_TYPE_FLOAT,offsetof(CVehicle,m_fEngineHealth));	// this should not be written ( not to
																										// 	break MP synchronization

}

//////////////////////////////////////////////////////////////////////////
void CScriptObjectVehicle::ReleaseTemplate( )
{
	_ScriptableEx<CScriptObjectVehicle>::ReleaseTemplate();
}

/*! Attaches a vehicle-container.
		@param pVehicle pointer to the vehicle-container which should be attached
*/
//////////////////////////////////////////////////////////////////////////
void CScriptObjectVehicle::SetVehicle(CVehicle *pVehicle)
{
	m_pVehicle = pVehicle;

	if(!EnablePropertiesMapping(pVehicle ))
	{
		CryError( "<CryGame> (CScriptObjectVehicle::SetVehicle) failed" );
		return;
	}
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Retrieves the attached vehicle-container.
		@return pointer to the attached vehicle-container
*/
CVehicle *CScriptObjectVehicle::GetVehicle()
{
	return m_pVehicle;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CScriptObjectVehicle::SetVehicleEngineHealth(IFunctionHandler *pH)
{
	ASSERT(pH->GetParamCount()>=1);
	
	float fEngineHealth;
	pH->GetParam(1,fEngineHealth);	
	m_pVehicle->SetEngineHealth(fEngineHealth);
	
	return pH->EndFunction(); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! Attaches a player to the vehicle, (so he is able to drive it IF bDriving).
		@param nPlayerId id of the player which should be attached (int)
		@param bDriving  if 1-set driver, if 0-passenger  

*/
int CScriptObjectVehicle::SetUser(IFunctionHandler *pH)
{
//	ASSERT(pH->GetParamCount()>=3);
	CHECK_PARAMETERS(5);

	int parCount = pH->GetParamCount();



	//FIXME
	ILog *pLog=m_pVehicle->GetGame()->GetSystem()->GetILog();

	//get the playerID
	int nPlayerid;
	pH->GetParam(1,nPlayerid);	

	//get the helper
	const char *szHelperName;
		if (!pH->GetParam(2,szHelperName))	
		szHelperName=NULL;	

	//check if an animation name is specified
	const char * szAnimName;
	if (!pH->GetParam(3,szAnimName))
		szAnimName=NULL;

	// check if we're setting this player as driver (can be a passenger also)
	CPlayer::eInVehiclestate vehicleState = CPlayer::PVS_PASSENGER;
//	int	vehicleState = CPlayer::PVS_PASSENSER;
	if (parCount>3)
		pH->GetParam(4,(int&)vehicleState);

	int iSlot=0;
	pH->GetParam(5,iSlot);

	// if no helper is specified, nobody knows where to place the player
	if (!szHelperName)
	{
		pLog->Log("/003 CScriptObjectVehicle: Helper not specified");
		return pH->EndFunction();
	}

	IEntity		*pEntity = m_pEntitySystem->GetEntity(nPlayerid);
	//CXServer	*pSrv=m_pVehicle->GetGame()->GetServer();

	//if (pSrv)	//	do server stuff
	{
		//bind the player to this vehicle
		m_pVehicle->GetEntity()->Bind(nPlayerid,iSlot);
		//pSrv->BindEntity(m_pVehicle->GetEntity()->GetId(),nPlayerid,iSlot);	[Anton] - Entity->Bind already called the server

		//make the player facing the correct angles
		Vec3 hPos = Vec3(0,0,180);
		pEntity->SetAngles(hPos);

		// move the player to the correct position
		m_pVehicle->GetEntity()->GetHelperPosition(szHelperName, hPos, true);
		pEntity->SetPos( hPos, false );

		//stop all animations
		pEntity->ResetAnimations(0);
	}

	//start to play the "jump in vehicle" animation, if any
//	if (szAnimName)
//		pEntity->StartAnimation(0, szAnimName, 0, .15f);

	CPlayer *pPlayer=NULL;
	if (pEntity->GetContainer()) 
		pEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);

	if (!pPlayer)
	{
		pLog->Log("/004 CScriptObjectVehicle::SetUser trying to set nonplayer as user");
		return pH->EndFunction();
	}

	// do not le him using the weapon once he's inside
	// pPlayer->DeselectWeapon();

	// Disable normal animation control so that it doesnt overide us
	pPlayer->m_AnimationSystemEnabled = 0;

	// let the player enter the vechicle as passenger or driver
	pPlayer->EnterVehicle(m_pVehicle, vehicleState, szHelperName);

	// if hes the driver, lets switch the control keys to the vechile keys
	// the key map is changed from script

	//	deactivate the physics for the player
	pEntity->ActivatePhysics( false );

	//CLIENT side code	
	//play enter animation (if any)
	pEntity->ResetAnimations(0);
	if (szAnimName)
		pEntity->StartAnimation(0,szAnimName, 0,.15f);

	IAIObject *pObj=pEntity->GetAI();

	return pH->EndFunction(); 
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectVehicle::ReleaseUser(IFunctionHandler *pH)
{
//	CHECK_PARAMETERS(3);
	ASSERT(pH->GetParamCount()>=3);

	int playerid;
	pH->GetParam(1,playerid);

	const char *szHelperName;
	if (!pH->GetParam(2,szHelperName))
		szHelperName=NULL;

	const char *szAnimName;
	if (!pH->GetParam(3,szAnimName))
		szAnimName=NULL;

	float	angle;
	if (!pH->GetParam(4,angle))
		angle=0;

	int iSlot=0;
	pH->GetParam(5,iSlot);

  IEntity *pEntity = m_pEntitySystem->GetEntity(playerid);

	if(!pEntity)
		return pH->EndFunction();

	{
		//do server side , if server present

		//unbind the player on the server
		m_pVehicle->GetEntity()->Unbind(playerid,iSlot);
		//pSrv->UnbindEntity(m_pVehicle->GetEntity()->GetId(),playerid,iSlot); [Anton] entity already called the server

		Vec3 vPos(0,0,0);
		if (szHelperName)
		{
			m_pVehicle->GetEntity()->GetHelperPosition(szHelperName, vPos, false);
		}

		if ( (!szHelperName) || IsEquivalent(vPos,Vec3(0,0,0)) )
		{
			// if not helper is specified or not found, lets try to drop the player close
			// to the vehicle
			vPos=m_pVehicle->GetEntity()->GetPos();
			vPos.z+=1; //move it 1 meter up to try to avoid collision problems
		}
		//[kirill] animation is reset in script, just to be sure
		pEntity->ResetAnimations(0);

		pEntity->SetPos(vPos);
		// [Anton] this code seems useless
		/*IPhysicalEntity *iphys = m_pVehicle->GetEntity()->GetPhysics();
		if (iphys)
		{
			pe_status_dynamics	dyn;
			iphys->GetStatus( &dyn );
			pEntity->AddImpulse(0, vPos, dyn.v*50 );
		}*/
	}

	//play leaving animation, if any
	
	CPlayer *pPlayer=NULL;
	if(pEntity->GetContainer()) pEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);

	if(!pPlayer)
	{
		m_pVehicle->GetGame()->GetSystem()->GetILog()->Log("/004 CScriptObjectVehicle::SetUser trying to set nonplayer as user");
		return pH->EndFunction();
	}
//	enable normal animation control so that it doesnt overide us
	pPlayer->m_AnimationSystemEnabled = 1;

	if (pPlayer->IsAlive())
	{
		if (szAnimName)
			pEntity->StartAnimation(0, szAnimName, 0, .0f);
	}


	if(m_pVehicle->m_DriverID == playerid)
	{
		IPhysicalEntity *pEnt=m_pVehicle->GetEntity()->GetPhysics();
		pe_status_dynamics	dyn;
		pEnt->GetStatus( &dyn );
		//no vel trheshold to brake the vehicle once exit, it feel better.
		float velTrh = 0;//m_pVehicle->GetGame()->p_LeaveVehicleBrake->GetFVal();
		velTrh *= velTrh;
		if(pPlayer->IsAI() )
			m_pVehicle->m_bBrakeOnNodriver = true;
		else
			m_pVehicle->m_bBrakeOnNodriver = (dyn.v.len2()<velTrh);
		m_pVehicle->m_fNoDriverTime = 0.0f;
		m_pVehicle->m_DriverID = 0;
	}

	pPlayer->LeaveVehicle();

//	if (m_pVehicle->GetGame()->m_pIActionMapManager)
//		m_pVehicle->GetGame()->m_pIActionMapManager->SetActionMap("default");

//	activate the physics for the player
	pEntity->ActivatePhysics( true );

	if(pPlayer->IsMyPlayer() && pPlayer->IsAlive())	// user is killed and being released from vehicle - don't set camera to first person
		m_pVehicle->GetGame()->SetViewMode(false);	//	set first person view mode

	return pH->EndFunction();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Retrieves the current status of a certain wheel of the attached vehicle.
		@param nWheel id of the wheel to retrieve the status from (int)
		@return status table (bContact (bool), ptContact (vec), vel (float), dir (vec))
*/
int CScriptObjectVehicle::GetWheelStatus(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	//_SmartScriptObject pObj(m_pScriptSystem);
	int nWheel;
	pe_status_wheel Status;
	pH->GetParam(1,nWheel);
	m_pVehicle->GetWheelStatus(nWheel, &Status);
	//CScriptObjectVector ContactPtVec(m_pScriptSystem);
	//CScriptObjectVector SlipVelVec(m_pScriptSystem);
	m_soContactPtVec=(Vec3)Status.ptContact;
	m_soSlipVelVec=(Vec3)Status.velSlip;
	float fSlipVel=Status.velSlip.len();
	m_GetWheelStatus->SetValue( "bContact",Status.bContact );
	m_GetWheelStatus->SetValue( "ptContact", m_soContactPtVec);
	m_GetWheelStatus->SetValue( "vel", fSlipVel);
	m_GetWheelStatus->SetValue( "dir", m_soSlipVelVec);
	m_GetWheelStatus->SetValue( "compression", Status.suspLen/Status.suspLenFull);
	//filippo: could be useful to get on what type of surface the wheel is contact
	m_GetWheelStatus->SetValue( "surfaceIndex", Status.contactSurfaceIdx);
	return pH->EndFunction(m_GetWheelStatus);
}

/*! Retrieves the vehicles velocity
		@return velocity of the vehicle (float)
*/
//////////////////////////////////////////////////////////////////////////
int CScriptObjectVehicle::GetVehicleVelocity(IFunctionHandler *pH)
{
	//_SmartScriptObject pObj(m_pScriptSystem);
	//pe_status_vehicle peStatus;

	pe_status_dynamics peStatus;
	
	CHECK_PARAMETERS(0);

	IEntity *pEntity=m_pVehicle->GetEntity();
	IPhysicalEntity *pPEnt=pEntity->GetPhysics();
	if(pPEnt)
		pPEnt->GetStatus(&peStatus);
	
	float fVelocity=peStatus.v.len(); //.vel.len();
	
	return pH->EndFunction(fVelocity);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectVehicle::GetVehicleStatus(IFunctionHandler *pH)
{
#if defined(LINUX)
	IEntity *pEntity = NULL;
	if(m_pVehicle)
		pEntity=m_pVehicle->GetEntity();
	else
		return pH->EndFunction();
	IPhysicalEntity *pPEnt = NULL;
	if(pEntity)
		pPEnt=pEntity->GetPhysics();
	else
		return pH->EndFunction();
#else
	IEntity *pEntity=m_pVehicle->GetEntity();
	IPhysicalEntity *pPEnt=pEntity->GetPhysics();
#endif
	pe_status_vehicle peStatus;

	if (pPEnt)
	{
		pPEnt->GetStatus(&peStatus);

		m_GetVehicleStatus->BeginSetGetChain();
		m_GetVehicleStatus->SetValueChain( "vel",peStatus.vel.len());
		m_GetVehicleStatus->SetValueChain( "wheelcontact", peStatus.bWheelContact);
		m_GetVehicleStatus->SetValueChain( "engineRPM", peStatus.engineRPM);
		m_GetVehicleStatus->SetValueChain( "gear", peStatus.iCurGear-1);
		m_GetVehicleStatus->SetValueChain( "clutch", peStatus.clutch);
		//we want to know if lights are on/off
		m_GetVehicleStatus->SetValueChain( "headlights", (m_pVehicle->m_bHeadLightsOn?1:0));

		m_GetVehicleStatus->EndSetGetChain();

		return pH->EndFunction(m_GetVehicleStatus);
	}	

	return pH->EndFunction();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CScriptObjectVehicle::WakeUp(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0);

	IEntity *pEntity=m_pVehicle->GetEntity();
	IPhysicalEntity *pPEnt=pEntity->GetPhysics();
	if(pPEnt)
	{
		pe_params_pos	params;
		params.type = pe_params_pos::type_id;
		params.iSimClass = 2;
		pPEnt->SetParams(&params);
	}
	
	return pH->EndFunction( );
}

/*! Retrieves if the player started to accelerate the vehicle this frame
		@return state of acceleration
*/
int CScriptObjectVehicle::HasAccelerated(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0)
	return pH->EndFunction(m_pVehicle->HasAccelerated());
}

/*! Retrieves if the vehicle is breaking
		@return state of acceleration
*/
int CScriptObjectVehicle::IsBreaking(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(0)
	return pH->EndFunction(m_pVehicle->IsBreaking());
}

/*!
*/
int CScriptObjectVehicle::HandBreak(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1)
	int bBreak=0;

	pH->GetParam(1, bBreak);	
	m_pVehicle->m_bForceHandBreak = (bBreak!=0);

	return pH->EndFunctionNull();
}

int CScriptObjectVehicle::SetDrivingParameters(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem,true);
	pH->GetParam(1,*pTable);
	
	float pedal_speed=10.0f,steer_speed=25.0f,max_steer_v0=40.0f,max_steer_kv=0.0f,steer_relaxation_v0=0.0f,steer_relaxation_kv=5.0f,
		brake_vel_threshold=1000,brake_axle_friction=1000, max_steering_pedal=0.15f,pedal_limit_speed=10.0f;
	float	steer_speed_valScale = 0;
	float	steer_speed_min = 5;
	//[filippo]
	float fhandbrakevalue = 0.0f;
	float fmaxbrakingfrictionnodriver = 0.0f;
	float fhandbrakevaluenodriver = 0.0f;
	float fstabilizejump = 0.0f;
	float fsteerspeedscale = 1.0f;
	float fsteerspeedscalemin = 2.0f;
	//

	pTable->GetValue("pedal_speed",pedal_speed);
	pTable->GetValue("steer_speed",steer_speed);
	pTable->GetValue("steer_speed_valScale",steer_speed_valScale);
	pTable->GetValue("steer_speed_min",steer_speed_min);
	pTable->GetValue("max_steer_v0",max_steer_v0);
	pTable->GetValue("max_steer_kv",max_steer_kv);
	pTable->GetValue("steer_relaxation_v0",steer_relaxation_v0);
	pTable->GetValue("steer_relaxation_kv",steer_relaxation_kv);	 
	pTable->GetValue("brake_vel_threshold",brake_vel_threshold);
	pTable->GetValue("brake_axle_friction",brake_axle_friction);
	pTable->GetValue("max_steering_pedal",max_steering_pedal);
	pTable->GetValue("pedal_limit_speed",pedal_limit_speed);
	//[filippo]
	pTable->GetValue("handbraking_value",fhandbrakevalue);
	pTable->GetValue("max_braking_friction_nodriver",fmaxbrakingfrictionnodriver);
	pTable->GetValue("handbraking_value_nodriver",fhandbrakevaluenodriver);
	pTable->GetValue("stabilize_jump",fstabilizejump);
	pTable->GetValue("steer_speed_scale",fsteerspeedscale);
	pTable->GetValue("steer_speed_scale_min",fsteerspeedscalemin);
	//

	if (m_pVehicle)
		m_pVehicle->SetDrivingParams(pedal_speed, steer_speed, max_steer_v0,max_steer_kv, steer_relaxation_v0,steer_relaxation_kv,
			brake_vel_threshold,brake_axle_friction, steer_speed_valScale, steer_speed_min, max_steering_pedal,pedal_limit_speed,
			fhandbrakevalue,fmaxbrakingfrictionnodriver,fhandbrakevaluenodriver,fstabilizejump,fsteerspeedscale,fsteerspeedscalemin);

	return pH->EndFunction(m_pVehicle!=0);
}


int CScriptObjectVehicle::SetCameraParameters(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	_SmartScriptObject pTable(m_pScriptSystem,true);
	CScriptObjectVector oVec(m_pScriptSystem,true);
	pH->GetParam(1,*pTable);

	Vec3 vCamStiffness[2]={ Vec3(0,0,0),Vec3(0,0,0) },vCamLimits[2]={ Vec3(1,1,1),Vec3(1,1,1) };
	float fCamDamping=0.8f,fMaxCamTimestep=0.01f,fCamSnapDist=0.001f,fCamSnapVel=0.01f;
	
	if (pTable->GetValue("cam_stifness_negative",*oVec))
		vCamStiffness[0] = oVec.Get();
	if (pTable->GetValue("cam_stifness_positive",*oVec))
		vCamStiffness[1] = oVec.Get();
	if (pTable->GetValue("cam_limits_negative",*oVec))
		vCamLimits[0] = oVec.Get();
	if (pTable->GetValue("cam_limits_positive",*oVec))
		vCamLimits[1] = oVec.Get();
	pTable->GetValue("cam_damping",fCamDamping);
	pTable->GetValue("cam_max_timestep",fMaxCamTimestep);
	pTable->GetValue("cam_snap_dist",fCamSnapDist);
	pTable->GetValue("cam_snap_vel",fCamSnapVel);

	if (m_pVehicle)
		m_pVehicle->SetCameraParams(vCamStiffness,vCamLimits,fCamDamping,fMaxCamTimestep,fCamSnapDist,fCamSnapVel);

	return pH->EndFunction(m_pVehicle!=0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CScriptObjectVehicle::SetWaterVehicleParameters(IFunctionHandler *pH)
{

	ASSERT(pH->GetParamCount()<=1);
//	CHECK_PARAMETERS(1);

	if(pH->GetParamCount()==1)
	{
		_SmartScriptObject pTable(m_pScriptSystem,true);
		pH->GetParam(1,*pTable);
		WATER_VEHICLE_PARAMS wvPar;

		bool	bFlyingVehicle=false;
		pTable->GetValue("Damprot",		wvPar.m_fBtDumpRot);
		pTable->GetValue("Dampv",		wvPar.m_fBtDumpV);
		pTable->GetValue("Dampvs",		wvPar.m_fBtDumpVSide);
		pTable->GetValue("Dampvh",		wvPar.m_fBtDumpVH);
		pTable->GetValue("Dampw",		wvPar.m_fBtDumpW);
		pTable->GetValue("Turn",		wvPar.m_fBtTurn);
		wvPar.m_fBtTurnMin = wvPar.m_fBtTurn;
		pTable->GetValue("TurnMin",		wvPar.m_fBtTurnMin);
		pTable->GetValue("TurnVelScale",wvPar.m_fBtTurnSpeedScale);
		pTable->GetValue("Speedv",		wvPar.m_fBtSpeedV);
		pTable->GetValue("Speedturnmin",wvPar.m_fBtSpeedTurnMin);
		pTable->GetValue("Stand",		wvPar.m_fBtStand);
		pTable->GetValue("WaveM",		wvPar.m_fBtWave);
		pTable->GetValue("TiltTurn",	wvPar.m_fBtTitlTurn);

		pTable->GetValue("TiltSpd",		wvPar.m_fBtTitlSpd);
		pTable->GetValue("TiltSpdA",	wvPar.m_fBtTitlSpdA);
		pTable->GetValue("TiltSpdMinV",	wvPar.m_fBtTitlSpdMinV);
		pTable->GetValue("TiltSpdMinVTilt",		wvPar.m_fBtTitlSpdMinVTilt);


		pTable->GetValue("DownRate",	wvPar.m_fPgDownRate);
		pTable->GetValue("BackUpRate",	wvPar.m_fPgBackUpRate);
		pTable->GetValue("BackUpSlowDwnRate",	wvPar.m_fPgBackUpSlowRate);
		pTable->GetValue("UpSpeedThrhld",	wvPar.m_fPgUpSpeedThrhld);


		pTable->GetValue("Flying",	bFlyingVehicle);

		pTable->GetValue("CameraDist",	wvPar.m_fCameraDist);

		wvPar.m_fBoatGravity = 0;//set a default gravity, now zero means to use the old system.
		pTable->GetValue("gravity",	wvPar.m_fBoatGravity);

		wvPar.m_fBtStandAir = 0;
		pTable->GetValue("StandInAir",		wvPar.m_fBtStandAir);
		//if no value for StandInAir is specified use the same value of Stand.
		if (wvPar.m_fBtStandAir==0)
			wvPar.m_fBtStandAir = wvPar.m_fBtStand;


		if (m_pVehicle)
			m_pVehicle->SetWaterVehicleParameters(	wvPar, bFlyingVehicle );
	}
	else
	{
		if (m_pVehicle)
			m_pVehicle->SetType(VHT_BOATDEAD);
	}

	return pH->EndFunction(m_pVehicle!=0);
}

//////////////////////////////////////////////////////////////////////////
int CScriptObjectVehicle::SetGravity(IFunctionHandler *pH)
{	
	return (pH->EndFunction());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CScriptObjectVehicle::GetVertDeviation(IFunctionHandler *pH)
{
	Vec3 upDir(0,0,1);

	Matrix44 tm;
	tm.SetIdentity();
	tm=Matrix44::CreateRotationZYX(-m_pVehicle->GetEntity()->GetAngles()*gf_DEGTORAD)*tm; //NOTE: angles in radians and negated 

	upDir = GetTransposed44(tm)*upDir;

	Vec3 cross = upDir.Cross( Vec3(0,0,1) );

	float	res = cross.len();
	return pH->EndFunction(res);

}

//
//----------------------------------------------------------------------------------------------------------------
// helper, texture, headLeftHelper, headRightHelper, shader, backLeftHelper, backRightHelper, shader
int CScriptObjectVehicle::InitLights(IFunctionHandler *pH)
{
//	ASSERT(pH->GetParamCount()>=3);
	CHECK_PARAMETERS(8);
	const char *szName;

	if (!pH->GetParam(1,szName))	
		szName=NULL;	
	m_pVehicle->m_HeadLightHelper = szName;

	if (!pH->GetParam(2,szName))	
		szName=NULL;	
	m_pVehicle->InitHeadLight(szName, NULL);//"LightBeam/0");

	if (!pH->GetParam(3,szName))	
		szName=NULL;	
	m_pVehicle->m_HeadLightHelperLeft = szName;
	if (!pH->GetParam(4,szName))	
		szName=NULL;	
	m_pVehicle->m_HeadLightHelperRight	 = szName;


	if (!pH->GetParam(5,szName))	
		szName=NULL;	
	m_pVehicle->InitFakeLight(&m_pVehicle->m_pHeadLightLeft, szName);
	m_pVehicle->InitFakeLight(&m_pVehicle->m_pHeadLightRight, szName);

	if (!pH->GetParam(6,szName))	
		szName=NULL;	
	m_pVehicle->m_BackLightHelperLeft = szName;
	if (!pH->GetParam(7,szName))	
		szName=NULL;	
	m_pVehicle->m_BackLightHelperRight = szName;

	if (!pH->GetParam(8,szName))	
		szName=NULL;	
	m_pVehicle->InitFakeLight(&m_pVehicle->m_pBreakLightLeft, szName);
	m_pVehicle->InitFakeLight(&m_pVehicle->m_pBreakLightRight, szName);


	if(m_pVehicle->m_pHeadLight)
		m_pVehicle->m_pHeadLight->m_nEntityLightId = 0;
	if(m_pVehicle->m_pHeadLightLeft)
		m_pVehicle->m_pHeadLightLeft->m_nEntityLightId = 1;
	if(m_pVehicle->m_pHeadLightRight)
		m_pVehicle->m_pHeadLightRight->m_nEntityLightId = 2;
	if(m_pVehicle->m_pBreakLightLeft)
		m_pVehicle->m_pBreakLightLeft->m_nEntityLightId = 3;
	if(m_pVehicle->m_pBreakLightRight)
		m_pVehicle->m_pBreakLightRight->m_nEntityLightId = 4;

	return pH->EndFunction(); 
}


int CScriptObjectVehicle::EnableLights(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	
	int lights=0;
	pH->GetParam(1,lights);	
	m_pVehicle->m_bAutoLights = (lights!=0);

	return pH->EndFunction(); 
}

int CScriptObjectVehicle::SetWeaponLimits(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(4);

	pH->GetParam(1, m_pVehicle->m_MinVAngle);	
	pH->GetParam(2, m_pVehicle->m_MaxVAngle);
	pH->GetParam(3, m_pVehicle->m_MinHAngle);	
	pH->GetParam(4, m_pVehicle->m_MaxHAngle);
	m_pVehicle->m_AngleLimitVFlag = (m_pVehicle->m_MinVAngle!=m_pVehicle->m_MaxVAngle);
	m_pVehicle->m_AngleLimitHFlag = (m_pVehicle->m_MinHAngle!=m_pVehicle->m_MaxHAngle);

	return pH->EndFunction(); 
}

int CScriptObjectVehicle::SetWeaponName(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(2);
	const char *wpnName;
	pH->GetParam(1, wpnName);
	m_pVehicle->m_sAutoWeaponName = wpnName;
	pH->GetParam(2, wpnName);
	m_pVehicle->m_sMountedWeaponName = wpnName;
	return pH->EndFunction(); 
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CScriptObjectVehicle::AnimateUsers(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);
	int	aniId;
	pH->GetParam(1, aniId);	
	CVehicle::UsersList::iterator usrIt=m_pVehicle->m_UsersList.begin();
	for(; usrIt!=m_pVehicle->m_UsersList.end(); ++usrIt)
	{
		IEntity *pUsr = m_pEntitySystem->GetEntity( *usrIt );
		if( pUsr )
			pUsr->SendScriptEvent(ScriptEvent_InVehicleAnimation, aniId);
	}
	return pH->EndFunction(); 
}

int CScriptObjectVehicle::AnimateMountedWeapon(IFunctionHandler *pH)
{
	CHECK_PARAMETERS(1);

	const char *sanim_name=NULL;

	pH->GetParam(1, sanim_name);	
	
	if (sanim_name && m_pVehicle && m_pVehicle->m_pEntity)
	{
		//GetISystem()->GetILog()->Log(sanim_name);
		m_pVehicle->m_pEntity->StartAnimation(0, sanim_name, 0, 0 );
	}

	return pH->EndFunction(); 
}
