
//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XEntityVehicle.cpp
//  Description: Entity player vehicle stuff, split into another file
//								to avoid to have everything in xplayer.cpp
//
//  History:
//	- this file uses code from xplayer.cpp created by petar and kirill 
//  - October 09, 2002: this file created by Marco Corbetta
//	- October 11, 2002: major changes to boat/vehicle code by M.C.
//
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "XPlayer.h"
#include "XVehicle.h"
#include "ScriptObjectStream.h"
#include "WeaponSystemEx.h"
#include "WeaponClass.h"
#include <IEntitySystem.h>
#include <ISound.h>
#include <IAgent.h>

//for cvars
#include "Game.h"

//
//////////////////////////////////////////////////////////////////////////
void CPlayer::ProcessVehicleMovements(CXEntityProcessingCmd &ProcessingCmd)
{
	if ( ProcessingCmd.CheckAction(ACTION_RUNSPRINT) )	// we are in the vehicle - let's cycle to next position if run pressed
		m_pEntity->SendScriptEvent(ScriptEvent_CycleVehiclePos,0);
	if (m_stats.inVehicleState == PVS_DRIVER)	// driver - DRIVE!
		m_pVehicle->ProcessMovement(ProcessingCmd);
}

 
/*
//
//	calculates world position of intersection point of weapon ray with something
// 
//////////////////////////////////////////////////////////////////////////
bool CPlayer::Get3DCrosshairPosition( Vec3& pos3D, Vec3& posScr )
{
	if(m_pVehicle)
	{
		pos3D = m_pVehicle->m_vCross3D;
		if(m_pVehicle->m_bCrossOnScreen)
		{
			posScr = m_pVehicle->m_vCrossScreen;
//			posScr = m_pVehicle->m_vCrossScreenSmoothed;
		}
		else
		{
			posScr.x = -1;
			posScr.y = -1;
		}

		return m_pVehicle->m_bTargetIsLocked;
	}
	return false;
}
*/
//Attaches the player to a vehicle
//param pVehicle pointer to a vehicle to which the player should be attached to
//////////////////////////////////////////////////////////////////////////
void CPlayer::EnterVehicle( CVehicle *pVehicle, eInVehiclestate state, const char *szHelperName)
{
	// if the vehicle is not specified or we are already 
	// inside a vehicle return
	if ((!pVehicle) || (m_pVehicle))
		return;

	SwitchFlashLight(false);	// switch off flashlight

	GoStand();	// stand up - maybe was proining
	
	// [kirill] GoStand can fail coz of some collisions
	// but we need to force eyeHeight anyway
	pe_player_dimensions	dimEyes;
	dimEyes.heightEye = m_PlayerDimNormal.heightEye;
	m_pEntity->GetPhysics()->SetParams( &dimEyes );

	m_LegAngle = 0.0f;	
	m_pVehicle = pVehicle;

	// add this user to the vechicle
	m_pVehicle->AddUser( m_pEntity->GetId() );
	
	m_PrevWeaponID=-1;
	m_stats.inVehicleState = state;

	if ( (m_pGame->IsMultiplayer() && IsMyPlayer()) || (!m_pGame->IsMultiplayer() && !m_bIsAI ) )	
	{
		if (m_stats.inVehicleState==PVS_DRIVER)
		{
			m_sVehicleEyeHelper = "eye_pos";
			m_pVehicle->ResetCamera(true,m_sVehicleEyeHelper.c_str());
		}
//		else if (m_stats.inVehicleState==PVS_GUNNER)
//			m_pVehicle->ResetCamera(true,"gunner_eye_pos");
		else if (m_stats.inVehicleState==PVS_PASSENGER)
		{
//			std::string eye_String(szHelperName);
//			eye_String = eye_String+"_eye_pos";
			m_sVehicleEyeHelper = string(szHelperName)+"_eye_pos";
			m_pVehicle->ResetCamera(true,m_sVehicleEyeHelper.c_str());
//			m_pVehicle->ResetCamera(true,"passenger_eye_pos");
		}
	}
	else	// if this is MP server and not local player - keep the helper name to use for shooting
	{
		if (m_stats.inVehicleState==PVS_DRIVER)
			m_sVehicleEyeHelper = "eye_pos";
		else
			m_sVehicleEyeHelper = string(szHelperName)+"_eye_pos";
	}

	// update weapon usage
	// gunner has prioriti ocer driver
	// passangers don't use vehicle weapon
	//if (m_stats.inVehicleState==PVS_PASSENGER)
	//	return;	// when passenger - don't change the weapon

	if( m_stats.inVehicleState == PVS_DRIVER )	// when driver - can't use any weapon
	{
		bool bIsAI = m_pEntity->GetAI() && m_pEntity->GetAI()->GetType()==AIOBJECT_PLAYER;
		m_pVehicle->m_DriverID = m_pEntity->GetId();
		if(bIsAI) 
			// user is Player - he can control light 
			m_pVehicle->m_bAIDriver = false;
		else
			// user is AI - enable AutoLights
			m_pVehicle->m_bAIDriver = true;
	}

	/*if( GetGame()->GetWeaponSystemEx()->GetWeaponClassIDByName( m_pVehicle->GetWeaponName( PVS_DRIVER)) == -1
		&&
		GetGame()->GetWeaponSystemEx()->GetWeaponClassIDByName( m_pVehicle->GetWeaponName( PVS_GUNNER)) == -1)
		return;	// it's a vehicle without mounted weapon - don't change/deselect the weapon*/

	if( (m_pVehicle->GetWeaponName(m_stats.inVehicleState) == "none") ||
		GetGame()->GetWeaponSystemEx()->GetWeaponClassIDByName(m_pVehicle->GetWeaponName(m_stats.inVehicleState)) != -1 )
	{
		m_PrevWeaponID = GetSelectedWeaponId();

		//GetISystem()->GetILog()->Log("last weap id: %i\n", m_PrevWeaponID );

		SelectWeapon(-1);
		m_pVehicle->SetWeaponUser( m_pEntity->GetId() );
	}

	InitCameraTransition( PCM_ENTERINGVEHICLE );
}
	

// Detaches the player from a vehicle
//////////////////////////////////////////////////////////////////////////
void CPlayer::LeaveVehicle()
{
	//we cannot leave the vehicle if we're not inside
	if (!m_pVehicle)
		return;

	//filippo: force the player to stand up once he get out from the vehicle, 
	//because could happen the player didn't get stand up when entering the vehicle.
	GoStand();

	m_stats.crosshairOnScreen = true;
	m_sVehicleEyeHelper.clear();

	m_pVehicle->WeaponState( m_pEntity->GetId(), false );
	if (IsMyPlayer() && (m_stats.inVehicleState==PVS_DRIVER || m_stats.inVehicleState==PVS_PASSENGER))
		m_pVehicle->ResetCamera();

	InitCameraTransition( PCM_LEAVINVEHICLE );

	if(m_pVehicle->m_WeaponUser == m_pEntity->GetId())
		m_pVehicle->ReleaseWeaponUser();

	//filippo reminder: ReleaseWeaponUser call MakeWeaponAvailable that select the first weapon and not the last weapon
	//before enter into the vehicle , so below "ReleaseWeaponUser" we force to select the lastweaponid.

	// restore weapon you had before entering vehicle
	//[kirill] if this user had autoWeapon - always restore prevWeapon.
	// otherwise - only if there is prev Weapon - to save changed weapon 
	if( GetGame()->GetWeaponSystemEx()->GetWeaponClassIDByName(m_pVehicle->GetWeaponName(m_stats.inVehicleState)) != -1 )
	{
		//GetISystem()->GetILog()->Log("selecting weap id: %i\n", m_PrevWeaponID );
		SelectWeapon( m_PrevWeaponID );
	}
	else if(m_PrevWeaponID!=-1)	
		SelectWeapon( m_PrevWeaponID );



	// if it's gunner leaving the vehicle - enable driver to use autoWeapon
	if( m_stats.inVehicleState == PVS_GUNNER )
	{
		CPlayer *pDriver = m_pVehicle->GetUserInState( PVS_DRIVER );
		if( pDriver )
			m_pVehicle->SetWeaponUser( pDriver->GetEntity()->GetId() );
	}

	m_stats.inVehicleState = PVS_OUT;
	
	m_pVehicle->RemoveUser( GetEntity()->GetId() );

	// don't have vehicle anymore
	m_pVehicle = NULL;
}


void DemperAngl( Vec3d& current, const Vec3d& target, float tScale )
{
//Vec3d	v = (target - current)*tScale*dempCoeff;
Vec3d	diff = (target - current);//*tScale*dempCoeff;
Vec3d	v;

	diff.x = Snap_s180(diff.x);
	diff.y = Snap_s180(diff.y);
	diff.z = Snap_s180(diff.z);

	float	maxDelta = fabs(diff.x);
	if(maxDelta < fabs(diff.y) )
		maxDelta = fabs(diff.y);
	if(maxDelta < fabs(diff.z) )
		maxDelta = fabs(diff.z);
	tScale *= maxDelta;

	v = diff;
	v.Normalize();
	v*=tScale;
	if( fabs(diff.x)<fabs(v.x) )
		current.x = target.x;
	else
		current.x += v.x;
	if( fabs(diff.y)<fabs(v.y) )
		current.y = target.y;
	else
		current.y += v.y;
	if( fabs(diff.z)<fabs(v.z) )
		current.z = target.z;
	else
		current.z += v.z;
}


void DemperVec( Vec3d& current, const Vec3d& target, float tScale )
{
//Vec3d	v = (target - current)*tScale*dempCoeff;
Vec3d	diff = (target - current);//*tScale*dempCoeff;
Vec3d	v;

//	float	maxDelta = fabs(diff.x);
//	if(maxDelta < fabs(diff.y) )
//		maxDelta = fabs(diff.y);
//	if(maxDelta < fabs(diff.z) )
//		maxDelta = fabs(diff.z);
//	tScale *= maxDelta;

	v = diff;
	v.Normalize();
	v*=tScale;
	if( fabs(diff.x)<fabs(v.x) )
		current.x = target.x;
	else
		current.x += v.x;
	if( fabs(diff.y)<fabs(v.y) )
		current.y = target.y;
	else
		current.y += v.y;
	if( fabs(diff.z)<fabs(v.z) )
		current.z = target.z;
	else
		current.z += v.z;
}



//////////////////////////////////////////////////////////////////////////
void	CPlayer::UpdateBoatCamera()
{
	IEntityCamera *camera = m_pEntity->GetCamera();
	IEntity *car = m_pVehicle->GetEntity();
	Vec3d	pos = car->GetPos();
	Vec3d	angles = car->GetAngles();
	//static	Vec3d	prevCarAng=Vec3d(0,0,0);

	if (m_bFirstPerson)
	{
		// first person		

		// set default in case we miss the helper for first person
		Vec3d vEyePos=m_vEyePos;

		if (car)
		{
			//Vec3d vPos;

			// check if we are the driver or the passenger
			// and set the camera position accordingly
			/*if (m_stats.inVehicleState == PVS_DRIVER)			// this is a driver
				car->GetHelperPosition("eye_pos",vPos); 			
			else if (m_stats.inVehicleState == PVS_PASSENGER)	// this is a passenger
				car->GetHelperPosition("passenger_eye_pos",vPos); 			*/

//vPos = m_pVehicle->GetCamPos();
			if (m_stats.inVehicleState==PVS_DRIVER || m_stats.inVehicleState==PVS_PASSENGER)
				//vPos = m_pVehicle->GetCamPos();
				vEyePos = m_pVehicle->GetCamPos();
			else											
			{												// this is gunner at mounted weapon
//[kirill] ok - no any additional extracalculations for camera position of gunner
/*
//				m_vEyeAngles=m_pEntity->GetAngles();
				Vec3d	pos = camera->GetCamOffset();
				if(m_pVehicle->IsBoat())
					pos.z = 1.3f;

				Matrix44 matParent;
				matParent.SetIdentity();
				matParent=GetRotationZYX44(-gf_DEGTORAD*m_pVehicle->GetEntity()->GetAngles())*matParent; //NOTE: angles in radians and negated 
				pos = matParent.TransformVectorOLD( pos );

				Vec3d	posWpn = Vec3(0,.5,0);
				Matrix44 matWpn;
				matWpn.SetIdentity();
				matWpn=GetRotationZYX44(-gf_DEGTORAD*GetEntity()->GetAngles())*matWpn; //NOTE: angles in radians and negated 
				posWpn = matWpn.TransformVectorOLD( posWpn );

				camera->SetPos(m_pEntity->GetPos() + pos + posWpn);
*/
				//vPos = m_vEyePos;
				vEyePos = m_vEyePos;//filippo:smooth camera also for gunner.
				//camera->SetPos(m_vEyePos);
				//camera->SetAngles(m_vEyeAngles);
				//return;
//				car->GetHelperPosition("passenger_eye_pos",vPos); 			
			}

		//	if (vPos!=Vec3d(0,0,0))							
		//if (!IsEquivalent(vPos,Vec3d(0,0,0)))							
		//		vEyePos=vPos;
		}

//		camera->SetPos(vEyePos);
//		camera->SetAngles(m_vEyeAngles);


		switch(m_CameraMode)
		{
		case PCM_ENTERINGVEHICLE:
			UpdateCameraTransition( vEyePos );
			break;
		case PCM_INVEHICLE:
			{
				float fCurTime = m_pGame->GetSystem()->GetIPhysicalWorld()->GetPhysicsTime();
				if (m_pGame->IsMultiplayer() && m_pGame->UseFixedStep())
					m_pGame->SnapTime(fCurTime);
				float	timeScale = min(0.1f,fCurTime-m_fLastCamUpdateTime);//m_pTimer->GetFrameTime();
				m_fLastCamUpdateTime = fCurTime;
				Vec3 targetAngle = car->GetAngles();
				targetAngle.x = Snap_s180(targetAngle.x);
				targetAngle.y = Snap_s180(targetAngle.y);
				targetAngle.z = Snap_s180(targetAngle.z);

				m_vCurAngleParent.x = Snap_s180(m_vCurAngleParent.x);
				m_vCurAngleParent.y = Snap_s180(m_vCurAngleParent.y);
				m_vCurAngleParent.z = Snap_s180(m_vCurAngleParent.z);

				DemperAngl( m_vCurAngleParent, targetAngle, timeScale*m_pGame->p_CameraSmoothScale->GetFVal() );
				// do all the bound entity calculations again - to applay newely set parents angles
				Matrix44 mat=Matrix34::CreateRotationXYZ( Deg2Rad(m_vCurAngleParent),car->GetPos());
				mat=GetTransposed44(mat);	
				GetEntity()->SetParentLocale(mat);
				GetEntity()->CalculateInWorld();
				m_vEyeAngles = GetEntity()->GetAngles();

				m_vCurCamposVhcl = vEyePos;
				m_vCurAngleVhcl = m_vEyeAngles + m_walkParams.shakeAOffset;

/*
//				m_vCurCamposVhcl = vEyePos;
//				DemperVec( m_vCurCamposVhcl, vEyePos, timeScale*20.0f );	

//					DemperVec( m_vCurCamposVhcl, vEyePos, timeScale*10.0f );
				Vec3 delta = vEyePos - m_vCurCamposVhcl;
				float	fDelta = delta.len2();
				float	maxD=2.2f;

				if(fDelta>maxD*maxD )
				{
					delta.normalize();
					m_vCurCamposVhcl = vEyePos-delta*maxD;
				}
				else if(fDelta>0.0f)
				{
					fDelta = fDelta*timeScale*10.0f;
					if(fDelta > 1.0f)
						m_vCurCamposVhcl = vEyePos;
					else
						m_vCurCamposVhcl += delta*fDelta;
//					m_vCurCamposVhcl += delta*timeScale*100.0f;
//					m_vCurCamposVhcl += delta;

//					m_vCurCamposVhcl = vEyePos;
/*
				float	timeTotal = .1f/(fDelta + 1.0f);

					if( timeTotal<timeScale )
						m_vCurCamposVhcl = vEyePos;
					else
					{
						Vec3 dDelta = (delta/timeTotal)*timeScale;
						if(fabs(dDelta.x)>fabs(delta.x) || fabs(dDelta.y)>fabs(delta.y) ||fabs(dDelta.z)>fabs(delta.z))
							m_vCurCamposVhcl = vEyePos;
						else
							m_vCurCamposVhcl += dDelta;
					}
*/
/*
				}
//				m_vCurCamposVhcl = vEyePos;
				m_vCurAngleVhcl = m_vEyeAngles;


	m_walkParams.shakeLElapsedTime += m_pTimer->GetFrameTime();
	if(m_walkParams.shakeLElapsedTime < m_walkParams.shakeLTime)
	{
		float amplScale = ( 1.0f - ( m_walkParams.shakeLElapsedTime / m_walkParams.shakeLTime ) );

		m_walkParams.shakeLOffset.x = m_walkParams.shakeLAmpl.x*amplScale*Fsin(m_walkParams.shakeLElapsedTime*m_walkParams.shakeLFreq.x*6.283185307179586476925286766559f);
		m_walkParams.shakeLOffset.y = m_walkParams.shakeLAmpl.y*amplScale*Fcos(m_walkParams.shakeLElapsedTime*m_walkParams.shakeLFreq.y*6.283185307179586476925286766559f);
		m_walkParams.shakeLOffset.z = m_walkParams.shakeLAmpl.z*amplScale*Fsin(m_walkParams.shakeLElapsedTime*m_walkParams.shakeLFreq.z*6.283185307179586476925286766559f + 1.13f);

		Vec3d	shakeAAmpl=Vec3d(60, 30, 30);
		shakeAAmpl = shakeAAmpl*m_walkParams.shakeLAmpl.z;

		m_walkParams.shakeAOffset.x = shakeAAmpl.x*amplScale*Fsin(m_walkParams.shakeLElapsedTime*m_walkParams.shakeLFreq.x*6.283185307179586476925286766559f);
		m_walkParams.shakeAOffset.y = shakeAAmpl.y*amplScale*Fcos(m_walkParams.shakeLElapsedTime*m_walkParams.shakeLFreq.y*6.283185307179586476925286766559f);
		m_walkParams.shakeAOffset.z = shakeAAmpl.z*amplScale*Fsin(m_walkParams.shakeLElapsedTime*m_walkParams.shakeLFreq.z*6.283185307179586476925286766559f + 1.13f);
	}
	else
	{
		m_walkParams.shakeAOffset = Vec3d(0,0,0);
		m_walkParams.shakeLOffset = Vec3d(0,0,0);
		m_walkParams.shakeLElapsedTime = m_walkParams.shakeLTime;
	}

//				m_vCurCamposVhcl = vEyePos + m_walkParams.shakeLOffset;
				m_vCurAngleVhcl = m_vEyeAngles + m_walkParams.shakeAOffset;
*/
			}
			break;
		}
		camera->SetPos(m_vCurCamposVhcl);

		camera->SetAngles(m_vCurAngleVhcl);

//camera->SetAngles(m_vEyeAngles);

		return;
	}

	// third person mode uses one of the different cameras
	switch(m_pGame->b_camera->GetIVal())
	{
	case 0:
		{
			angles.x = 1;
			angles.y = 1;
			IPhysicalEntity *physEnt = car->GetPhysics();
			camera->SetCameraOffset(Vec3d(0,m_pGame->cl_ThirdPersonRange->GetFVal(),m_pGame->cl_ThirdPersonRange->GetFVal()));
			camera->SetCameraMode(pos,angles+m_pEntity->GetAngles(), physEnt);
			break;
		}
	case 1:
		{
			camera->SetAngles(angles+m_pEntity->GetAngles());
			pos.z += 4;
			camera->SetPos(pos);
			break;
		}
	case 2:
		{
			Vec3d camPos;
			Vec3d	cang; 
			angles=ConvertToRadAngles(angles);
			angles.z=0;
			angles.Normalize();
			camPos = pos - Vec3d(angles.y, -angles.x, 0.0f)*25;
			camPos.z = m_pGame->GetSystem()->GetI3DEngine()->GetWaterLevel(m_pEntity) + 8.0f;
			cang = pos - camPos;
			cang=ConvertVectorToCameraAngles(cang);
			camera->SetAngles(cang+m_pEntity->GetAngles());
			camera->SetPos(camPos);
			break;
		}
	case 3:
		{
			Vec3d camPos;
			Vec3d	cang; 
			angles=ConvertToRadAngles(angles);
			angles.z=0;
			angles.Normalize();
			camPos = pos - Vec3d(angles.x, angles.y, 0.0f)*25;
			camPos.z = m_pGame->GetSystem()->GetI3DEngine()->GetWaterLevel(m_pEntity) + 8.0f;
			cang = pos - camPos;
			cang=ConvertVectorToCameraAngles(cang);
			camera->SetAngles(cang+m_pEntity->GetAngles());
			camera->SetPos(camPos);
			break;
		}
	case 4:
		{
			if(!UpdateBonesPtrs( ))
				return; 

			Matrix44 m;
			m.SetIdentity();
			m=Matrix44::CreateRotationZYX(-angles*gf_DEGTORAD)*m; //NOTE: angles in radians and negated 

			angles.x = 0.0f;
			angles.y = 0.0f;
			camera->SetAngles(angles+m_pEntity->GetAngles());
			pos = m_pBoneHead->GetBonePosition();
			pos.z += .2f;
			pos.y += .3f;

			pos = m.TransformVectorOLD( pos );
			//pos = GetTransposed44(m)*( pos );
		
			pos += GetEntity()->GetPos();
			pos.z += .3f;
			camera->SetPos(pos);
			break;
		}
	}


}

//////////////////////////////////////////////////////////////////////////
void	CPlayer::InitCameraTransition( e_PCM mode, bool OnlyZtransition)
{
	m_bCameraTransitionOnlyZ = OnlyZtransition;

	switch( mode )
	{
	case	PCM_ENTERINGVEHICLE:

		m_CameraMode = PCM_ENTERINGVEHICLE;
		m_fCameraTime = m_pGame->p_CameraSmoothTime->GetFVal();
//		m_fCameraTime = 0.0f;


		// check if camera will go through something while transmitting. If yes - make 
		// transmition very fast (immidiate)
		{
		Vec3 vEyeDestPos = m_pVehicle->GetCamPos();
		if (m_stats.inVehicleState==PVS_GUNNER)	
			vEyeDestPos = CalcLeanOffset(0.0f);
		ray_hit RayHit;
		if (m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(vEyeDestPos, m_vEyePos - vEyeDestPos, 
			ent_all,0, &RayHit, 1, GetEntity()->GetPhysics()))
			m_fCameraTime = 0.1f;
		}

		//filippo:if m_bLastDeltaEyeVehicle is true means the player is changing sit position, so use the right eye position.
		if (m_bLastDeltaEyeVehicle && m_pVehicle)
		{
			m_vCurCamposVhcl = m_pVehicle->GetEntity()->GetPos() - m_vDeltaEyeVehicle;
			m_bLastDeltaEyeVehicle = false;
		}
		else
			m_vCurCamposVhcl = m_vEyePos;

		m_vCurAngleVhcl = m_vEyeAngles;

		//filippo
		if (m_pVehicle)
			m_vDeltaEyeVehicle = m_pVehicle->GetEntity()->GetPos() - m_vCurCamposVhcl;

//m_vCurCamposVhcl = GetEntity()->GetPos();

		break;
	case	PCM_LEAVINVEHICLE:
		{
			//filippo:when leave the vehicle store the delta from car pos and eye pos; because if we are changing sit position we need to know the last eye position for a smooth view transition.
			if (m_pVehicle)
			{
				m_vDeltaEyeVehicle = m_pVehicle->GetEntity()->GetPos() - m_vCurCamposVhcl;
				m_bLastDeltaEyeVehicle = true;
			}

		m_CameraMode = PCM_CASUAL;

		float velScale = m_pGame->p_CameraSmoothTime->GetFVal();
		IPhysicalEntity *icar = m_pVehicle->GetEntity()->GetPhysics();
		
		if(icar)	// is phisycalized
		{
		float timeVelCoeff=m_pGame->p_CameraSmoothVLimit->GetFVal();
			pe_status_dynamics status;
			icar->GetStatus(&status);
			velScale = status.v.len();
			if(velScale>timeVelCoeff)
				velScale = timeVelCoeff;
			velScale = m_pGame->p_CameraSmoothTime->GetFVal()*(timeVelCoeff-velScale)/timeVelCoeff;
		}
//		m_fCameraTime = m_pGame->p_CameraSmoothTime->GetFVal();
		m_fCameraTime = velScale;
		}
		break;
	case	PCM_CASUAL:
		{
		m_CameraMode = PCM_CASUAL;
	
		if(m_fCameraTime >= 0.1f)
			break;
		float velScale = m_pGame->p_CameraSmoothTime->GetFVal();
		m_vCurCamposVhcl = m_vEyePos;
		m_vCurAngleVhcl = m_vEyeAngles; 
//		m_fCameraTime = m_pGame->p_CameraSmoothTime->GetFVal();
		m_fCameraTime = velScale*.5f;
		}
		break;
	}
/*
	if( bEntering )
	{
		m_CameraMode = PCM_ENTERINGVEHICLE;
		m_fCameraTime = m_pGame->p_CameraSmoothTime->GetFVal();
		m_vCurCamposVhcl = m_vEyePos;
		m_vCurAngleVhcl = m_vEyeAngles;
	}
	else
	{
		m_CameraMode = PCM_LEAVINVEHICLE;

		float velScale = m_pGame->p_CameraSmoothTime->GetFVal();
		IPhysicalEntity *icar = m_pVehicle->GetEntity()->GetPhysics();
		
		if(icar)	// is phisycalized
		{
		float timeVelCoeff=m_pGame->p_CameraSmoothVLimit->GetFVal();
			pe_status_dynamics status;
			icar->GetStatus(&status);
			velScale = status.v.len();
			if(velScale>timeVelCoeff)
				velScale = timeVelCoeff;
			velScale = m_pGame->p_CameraSmoothTime->GetFVal()*(timeVelCoeff-velScale)/timeVelCoeff;
		}
//		m_fCameraTime = m_pGame->p_CameraSmoothTime->GetFVal();
		m_fCameraTime = velScale;
//		m_fCameraTime = .6f;
//		m_vCurCamposVhcl = m_vEyePos;
//		m_vCurAngleVhcl = m_vEyeAngles;
	}
*/
}

//////////////////////////////////////////////////////////////////////////
void	CPlayer::UpdateCameraTransition( const Vec3& vEyePos  )
{
float	timeScale = m_pTimer->GetFrameTime();
	if( timeScale>.1f )
		timeScale = .1f;

	//filippo: smooth only Z eyepos component if player is outside vehicles; 
	//this because otherwise when you crouch/standup your view (position&rotaion) would lag because the smoothing,
	//for vehicles this is OK, but not for standard player movement.
	/*bool bSmoothOnlyZ = false;
	if(m_CameraMode == PCM_CASUAL && !m_bLastDeltaEyeVehicle)
		bSmoothOnlyZ = true;*/

	//filippo
	if (m_pVehicle)
		m_vCurCamposVhcl = m_pVehicle->GetEntity()->GetPos() - m_vDeltaEyeVehicle;

	Vec3 delta = vEyePos - m_vCurCamposVhcl;
	//Vec3 deltap = delta;

	// do not smooth if it's too far
	// just set the position/angles
	if( delta.len2()>100 )
		m_fCameraTime = timeScale*.1f;

	m_vCurCamposVhcl += (delta/m_fCameraTime)*timeScale;
	//m_vCurCamposVhcl += delta*5.0f*timeScale;

	//filippo
	if (m_pVehicle)
			m_vDeltaEyeVehicle = m_pVehicle->GetEntity()->GetPos() - m_vCurCamposVhcl;

	m_vEyeAngles.x = Snap_s180(m_vEyeAngles.x);
	m_vEyeAngles.y = Snap_s180(m_vEyeAngles.y);
	m_vEyeAngles.z = Snap_s180(m_vEyeAngles.z);

	//if (!bSmoothOnlyZ)
	if (!m_bCameraTransitionOnlyZ)
	{
		m_vCurAngleVhcl.x = Snap_s180(m_vCurAngleVhcl.x);
		m_vCurAngleVhcl.y = Snap_s180(m_vCurAngleVhcl.y);
		m_vCurAngleVhcl.z = Snap_s180(m_vCurAngleVhcl.z);

		delta = (m_vEyeAngles - m_vCurAngleVhcl);//*tScale*dempCoeff;

		delta.x = Snap_s180(delta.x);
		delta.y = Snap_s180(delta.y);
		delta.z = Snap_s180(delta.z);

		m_vCurAngleVhcl += (delta/m_fCameraTime)*timeScale;
		//m_vCurAngleVhcl += delta*5.0f*timeScale;
	}
	else
	{
		//just use the player angles and the smoothed pos.Z component
		m_vCurAngleVhcl = m_vEyeAngles;
		m_vCurCamposVhcl.x = vEyePos.x;
		m_vCurCamposVhcl.y = vEyePos.y;
	}

	if(	(m_fCameraTime -= timeScale) <= 0 )
	//if(deltap.len2()<=0.001f)
	{	
		m_vCurCamposVhcl = vEyePos;
		m_vCurAngleVhcl = m_vEyeAngles; 

		if(m_CameraMode == PCM_ENTERINGVEHICLE)
		{
			m_vCurAngleParent = m_pVehicle->GetEntity()->GetAngles();
			m_CameraMode = PCM_INVEHICLE;

			//filippo
			if (m_pVehicle)
				m_vDeltaEyeVehicle = m_pVehicle->GetEntity()->GetPos() - m_vCurCamposVhcl;
		}
		else if(m_CameraMode == PCM_CASUAL)
		{
			m_CameraMode = PCM_OUTVEHICLE;

			//filippo
			m_bLastDeltaEyeVehicle = false;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void	CPlayer::UpdateAutoCenter()
{
	// it works only in vehicles
	if(!m_pVehicle)
		return;

float	timeScale = m_pTimer->GetFrameTime();
Vec3	curAngl = m_pEntity->GetAngles(1);

	m_fProcessTime += timeScale;

    curAngl.x = Snap_s180(curAngl.x);
	curAngl.y = Snap_s180(curAngl.y);
	curAngl.z = Snap_s180(curAngl.z);

	if(m_AutoCenter == 0)
	{
		if(!m_bMouseMoved)
			m_fNoChangeangleTime += timeScale;
		else
			m_fNoChangeangleTime = 0.0f;

		if(m_fNoChangeangleTime>m_pGame->p_AutoCenterDelay->GetFVal())
			StartAutoCenter( true );
		return;
	}
	if(m_bMouseMoved)
	{
		ResetAutoCenter();
		return;
	}
	float trh=.10f;
	if( curAngl.x < trh && curAngl.x > -trh &&
		curAngl.y < trh && curAngl.y > -trh &&
		(m_AutoCenter==2 && curAngl.z < trh && curAngl.z > -trh ||
		 m_AutoCenter==1 && curAngl.z > 180.0f-trh && curAngl.z < -180.0f+trh)
		)
	{
		curAngl.Set(0,0,0);
		m_pEntity->SetAngles( curAngl );
		ResetAutoCenter();
		return;
	}

	timeScale *= m_pGame->p_AutoCenterSpeed->GetFVal();
	float	velScale = m_pGame->p_AutoCenterSpeed->GetFVal();
	velScale = 100.0f - velScale;
	if(velScale<0.0f)
		velScale = 0.0f;
	else if(velScale>=100.0f)
		velScale = 99.9f;

	velScale = (70.0f + velScale*0.3f)/100.0f;


	timeScale = m_pGame->p_AutoCenterSpeed->GetFVal()/100.0f;
	// don't want to move too slow
	if( timeScale>.999f )
		timeScale = .999f;

	timeScale = velScale;


	if(m_AutoCenter == 1)	// going to z 180
	{
		curAngl.x = curAngl.x*timeScale;	
		curAngl.y = curAngl.y*timeScale;
		if(curAngl.z<0.0f)
			curAngl.z += (-180.0f-curAngl.z)*(1.0f-timeScale);
		else
			curAngl.z += (180.0f-curAngl.z)*(1.0f-timeScale);
	}
	else					// going to z 0
		curAngl = curAngl*timeScale;

	m_pEntity->SetAngles( curAngl );
}

//////////////////////////////////////////////////////////////////////////
void	CPlayer::StartAutoCenter(bool forward)
{
Vec3	curAngl = m_pEntity->GetAngles(1);

	// we don't want it to flip back and forth - so make time thrashold
	if( m_fProcessTime < .5f)
		return;

	m_fProcessTime = 0.0f;
    curAngl.x = Snap_s180(curAngl.x);
	curAngl.y = Snap_s180(curAngl.y);
	curAngl.z = Snap_s180(curAngl.z);

	if(forward)
		m_AutoCenter = 1;
	else
	{
		//[kirill] if angles are limited - only autorotate forward 
		if( m_AngleLimitHFlag )
			m_AutoCenter = 1;
		else if( fabs(curAngl.z)<90.0f )
			m_AutoCenter = 1;
		else
			m_AutoCenter = 2;
	}

	// make it flip randomly in different direstions if it's 180 turn
//	if(curAngl.z == 0.0f)
	if(curAngl.z<0.5f && curAngl.z>-0.5f)
	{
		if(rand()%100<50)
			curAngl.z += .01f;
		else
			curAngl.z -= .01f;
	}
	else if(curAngl.z > 179.5f)
	{
		if(rand()%100<50)
			curAngl.z = -179.99f;
		else
			curAngl.z -= .01f;
	}
	else if(curAngl.z < -179.5f)
	{
		if(rand()%100<50)
			curAngl.z = 179.99f;
		else
			curAngl.z += .01f;
	}
//	m_pEntity->SetAngles( curAngl );
}

//////////////////////////////////////////////////////////////////////////
void	CPlayer::ResetAutoCenter()
{
	m_AutoCenter = 0;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::InitVehicleCvars()
{
	IConsole *pConsole = m_pSystem->GetIConsole();
	
	//*
//Dumprot	 9000.4
//Dumpv		 1500.4
//Turn		12000.0
//Speedv		35000.0
//Speedturnmin	       5.0

	b_dump = pConsole->CreateVariable("b_dump","2000.4",0,"This variable is not used.");
	b_dumpRot = pConsole->CreateVariable("b_dumprot","9000.4",0,"This variable is not used.");
	b_dumpV = pConsole->CreateVariable("b_dumpv","1500.4",0,"This variable is not used.");
	b_dumpVH = pConsole->CreateVariable("b_dumpvh","10000.4",0,"This variable is not used.");
	b_stand = pConsole->CreateVariable("b_stand","10000.5",0,"This variable is not used.");
	b_turn = pConsole->CreateVariable("b_turn","12000.0",0,"This variable is not used.");
	b_tilt = pConsole->CreateVariable("b_tilt","2.0",0,"This variable is not used.");
	b_speedV = pConsole->CreateVariable("b_speedv","35000.0",0,"This variable is not used.");
	b_accelerationV = pConsole->CreateVariable("b_accelerationv","100000.0",0,"This variable is not used.");
	b_speedMinTurn = pConsole->CreateVariable("b_speedminturn","5.0",0,"This variable is not used.");
	b_float = pConsole->CreateVariable("b_float","7",0,"This variable is not used.");
	b_wscale = pConsole->CreateVariable("b_wscale","2.1",0,"This variable is not used.");
	b_wscalew = pConsole->CreateVariable("b_wscalew","2.1",0,"This variable is not used.");
	b_wmomentum = pConsole->CreateVariable("b_wmomentum","500.5",0,"This variable is not used.");
	//*/
	b_camera = pConsole->CreateVariable("b_camera","0",0,"This variable is not used.");

	p_CameraSmoothTime = pConsole->CreateVariable("p_camerasmoothtime",".6",0,"when entering/leaving vehicles.");
	p_CameraSmoothScale = pConsole->CreateVariable("p_camerasmoothscale","5",0,"when driving vehicles.");
	p_CameraSmoothVLimit = pConsole->CreateVariable("p_camerasmoothvlimit","20",0,"camera transition scale to vehicle speed when leaving moving vehicles.");

	p_LeaveVehicleImpuls = pConsole->CreateVariable("p_leavevehicleimpuls","20",0,"impilse scale to vehicle speed when leaving moving vehicles.");
	p_LeaveVehicleBrake = pConsole->CreateVariable("p_leavevehiclebrake","10",0,"speed thrashold to have breaks on when driver is out");
	p_LeaveVehicleBrakeDelay = pConsole->CreateVariable("p_leavevehiclebrakedelay","2",0,"delay before wehicle stops after driver out is out");

	p_AutoCenterDelay = pConsole->CreateVariable("p_autocenterdelay","30",0,"idle time before force autoCenter");
	p_AutoCenterSpeed = pConsole->CreateVariable("p_autocenterspeed","20",0,"speed of autoCentering - inverted (the bigger - the slower)");

	// show bboxes for static objects below helicopter
	h_drawbelow = pConsole->CreateVariable("h_drawbelow","0",0,
		"Toggles bounding boxes below helicopters.\n"
		"Usage: h_drawbelow [0/1]\n"
		"Default is 0 (off). Set 1 to display the bounding\n"
		"boxes of obstacles currently below a helicopter.");
}


//
//--------------------------------------------------------------------------------------
// draving weapon on the vehicle here - weapon is a character in slot 0
void CVehicle::OnDraw(const SRendParams & rParms)
{
  // draw animated component 

	ICryCharInstance *cmodel = m_pEntity->GetCharInterface()->GetCharacter(0);
    if (cmodel && (cmodel->GetFlags()&CS_FLAG_DRAW_MODEL))
    {
        SRendParams RenderParams = rParms;
		// we want it to recalculate the matrix
		RenderParams.pMatrix = NULL;
		RenderParams.vAngles = m_vWpnAng;
//RenderParams.vAngles.z = 90.0f;
//		RenderParams.vAngles.x = -RenderParams.vAngles.x;
//		RenderParams.vAngles.y = -RenderParams.vAngles.y;
//		RenderParams.vAngles = m_vWpnAng;
		GetEntity()->GetHelperPosition("gun",RenderParams.vPos,false);
//		RenderParams.vPos = m_pEntity->GetPos() + Vec3(0,0,3);
		cmodel->Draw(RenderParams,Vec3(zero));
    }
}

//
//----------------------------------------------------------------------------------
CPlayer*	CVehicle::GetUserInState( CPlayer::eInVehiclestate state )
{
UsersList::iterator	curUser;
	for(curUser=m_UsersList.begin(); curUser!=m_UsersList.end();	++curUser)
	{
	IEntity	*pCurUserEntity = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity( *curUser );
	CPlayer *pCurUserPlayer;
		if(	pCurUserEntity &&
			pCurUserEntity->GetContainer() && 
			pCurUserEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pCurUserPlayer) &&
			pCurUserPlayer->m_stats.inVehicleState == state)
			return pCurUserPlayer;
	}
	return NULL;
}

//
//----------------------------------------------------------------------------------
CPlayer*	CVehicle::GetWeaponUser( )
{
	IEntity	*pCurUserEntity = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity( m_WeaponUser );
	CPlayer *pCurUserPlayer;
	if(	pCurUserEntity &&
		pCurUserEntity->GetContainer() && 
		pCurUserEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pCurUserPlayer) )
		return pCurUserPlayer;
	return NULL;
}


//
//--------------------------------------------------------------------------------------
void CVehicle::SetWeaponUser(int entId)
{
CPlayer *shooter = GetWeaponUser();
	// there is a gunner - has priority on weapon usage
	if( shooter && shooter->m_stats.inVehicleState == CPlayer::PVS_GUNNER )
		return;
	ReleaseWeaponUser( true );

	m_WeaponUser = entId;
	IEntity	*pCurUserEntity = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity( m_WeaponUser );
	if(	pCurUserEntity &&
		pCurUserEntity->GetContainer() && 
		pCurUserEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&shooter) )
	{
		int wpnId = GetGame()->GetWeaponSystemEx()->GetWeaponClassIDByName( GetWeaponName( shooter->m_stats.inVehicleState ) );
		if( wpnId != -1 )
		{
//			shooter->m_PrevWeaponID=shooter->GetSelectedWeaponId();
			shooter->MakeWeaponAvailable(wpnId, true);
			shooter->SelectWeapon(wpnId);
			//[kirill]
			//this is needed to fix problem with quickLoad - the weapon was reset to 0 firemode
			//-	Mounted guns on vehicles that have 2 firemodes are reset to MG after ql even if rockets were used
			if(m_pGame->m_bIsLoadingLevelFromFile)
				shooter->SwitchFiremode(shooter->m_stats.firemode);
			else
				shooter->SwitchFiremode(0);
		}

		if( shooter->m_stats.inVehicleState == CPlayer::PVS_GUNNER )	// when driver - can't use any weapon
		{
			// set to gunner agle limits from the weapon
			shooter->SetAngleLimitBase( Vec3(0,0,180) );
			if(m_AngleLimitVFlag)
			{
				shooter->EnableAngleLimitV(1);
				shooter->SetMinAngleLimitV(m_MinVAngle);
				shooter->SetMaxAngleLimitV(m_MaxVAngle);
			}
			if(m_AngleLimitHFlag)
			{
				shooter->EnableAngleLimitH(1);
				shooter->SetMinAngleLimitH(m_MinHAngle);
				shooter->SetMaxAngleLimitH(m_MaxHAngle);
			}
		}
	}

	shooter->GetEntity()->SendScriptEvent(ScriptEvent_InVehicleAmmo,1);

}

//
//--------------------------------------------------------------------------------------
void CVehicle::ReleaseWeaponUser( bool bDeselectWeapon )
{
CPlayer *shooter;
	IEntity	*pCurUserEntity = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity( m_WeaponUser );
	if(	pCurUserEntity &&
		pCurUserEntity->GetContainer() && 
		pCurUserEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&shooter) )
	{
		m_WeaponUser = 0;
		//
		// remove the vehicle's weapon from player
		int wpnId = GetGame()->GetWeaponSystemEx()->GetWeaponClassIDByName( GetWeaponName( shooter->m_stats.inVehicleState ) );
		if( wpnId != -1 )
			shooter->MakeWeaponAvailable(wpnId, 0);		

		shooter->GetEntity()->SendScriptEvent(ScriptEvent_InVehicleAmmo,0);

		if(bDeselectWeapon)
			shooter->SelectWeapon(-1);
	}
}

//
//--------------------------------------------------------------------------------------
void CVehicle::UpdateWeaponPosAngl( )
{
//const char *pszBoneName="Bip01 Head\0";
const char *pszBoneName="Bip01 Spine1\0";
Vec3	Position;
Vec3	Center;
Vec3	bestPoint3D;
Vec3	bestPointScreen;
float	autoaimWndSize = 0.0f;
float	minDist = 0.0f;
bool bAutoAim=false;
CPlayer	*shooter = GetUserInState( CPlayer::PVS_GUNNER );
float	timeScale = m_pGame->GetSystem()->GetITimer()->GetFrameTime()*2.0f;

m_bTargetIsLocked = false;
//	m_vWpnPos = GetEntity()->GetPos()+Vec3(0,0,3.5);
GetEntity()->GetHelperPosition("gun",m_vWpnPos);

	if(shooter)
	{	
		// if there is a gunner - use he's angles
		m_vWpnAng = shooter->GetEntity()->GetAngles();
		m_bCrossOnScreen = true;
		shooter->m_stats.crosshairOnScreen = true;
		return;
	}
	else
		shooter = GetUserInState( CPlayer::PVS_DRIVER );
	if(!shooter)
	{	
	//[kirill] nobody using weapon - smoothly return it to "zero" position
	Vec3	vTargetAngl = m_pEntity->GetAngles() + Vec3(0,0,180);
		vTargetAngl.x = -vTargetAngl.x;
		vTargetAngl.y = -vTargetAngl.y;
		Vec3	vDiff = vTargetAngl - m_vWpnAng ;
		float trh=1.0f;
		if( vDiff.x<trh && vDiff.x>-trh && 
			vDiff.y<trh && vDiff.y>-trh && 
			vDiff.z<trh && vDiff.z>-trh )
			m_vWpnAng = vTargetAngl;
		else
		{
			vDiff.x = Snap_s180(vDiff.x);
			vDiff.y = Snap_s180(vDiff.y);
			vDiff.z = Snap_s180(vDiff.z);
			m_vWpnAng = m_vWpnAng + vDiff*.1f;
		}
		m_bCrossOnScreen = true;
		m_vWpnAngDelta.z = 0.0f;
		return;
	}

	//filippo: shift the weapon position up, to get a better firing position for the mounted weapon.
	//TODO?: make this shift customizable by scripts?
	Vec3 wpnPosOffset(0,0,0.3f);

	Matrix44 tm = Matrix44::CreateRotationZYX(-m_vWpnAng*gf_DEGTORAD); //NOTE: angles in radians and negated 
	wpnPosOffset = GetTransposed44(tm)*wpnPosOffset;
	m_vWpnPos += wpnPosOffset;
	//

//	shooter->m_stats.crosshairOnScreen = true;

	// check autoaim properties of the weapon
	WeaponParams wp;
	CWeaponClass* pSelectedWeapon = shooter->GetSelectedWeapon();
	// no autoaming in MP 
	if(!m_pGame->IsMultiplayer())
	if(pSelectedWeapon)
	{
		pSelectedWeapon->GetModeParams(shooter->m_stats.firemode, wp);
		bAutoAim = (wp.fAutoAimDist>0.0f);
		autoaimWndSize = wp.fAutoAimDist;
		minDist = autoaimWndSize*autoaimWndSize*2.0f;// 30000;
	}

	// we aim always in the center of the screen
	m_vCrossScreen.x = 400.0f;
	m_vCrossScreen.y = 300.0f;

	Vec3 vCrossHair3Dpos;

	//fixme - add the check if it's on screen
	m_bCrossOnScreen = true;

	//do autoaiming
	if(bAutoAim && m_bCrossOnScreen)
	{
	//
	//so, here goes autoaiming stuff - getting screen coordinates and finding closest to the center of
	//screen entity - then snapping on it
		IEntityItPtr It=m_pGame->GetSystem()->GetIEntitySystem()->GetEntityInFrustrumIterator( true );
		CCamera Cam=m_pGame->GetSystem()->GetViewCamera();
		IEntity *pEnt;
		ray_hit RayHit;
		IEntity *pLocal=m_pGame->GetMyPlayer();
		while (pEnt=It->Next())
		{
			if (pEnt==m_pEntity)
				continue;
			if (!pEnt->IsTrackable())
				continue;
			//
			//don't lock on people in this vehicle
			if ( std::find(m_UsersList.begin(), m_UsersList.end(), pEnt->GetId() ) != m_UsersList.end() )
				continue;
			CPlayer *pPlayer;
			if(	pEnt->GetContainer() && 
				pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer) &&
				!pPlayer->IsAlive() )
				continue;

			IPhysicalEntity *pPE=pEnt->GetPhysics();
			if (!pPE)
				continue;

			if (pszBoneName)	// if we want a bone instead of bbox-center lets do so...
			{
				IEntityCharacter *pIChar=pEnt->GetCharInterface();
				if (pIChar)
				{
					ICryCharInstance *cmodel=pIChar->GetCharacter(0);    
					if (cmodel)
					{
						ICryBone *pBone = cmodel->GetBoneByName(pszBoneName);
						if (pBone)
						{
							Center=pBone->GetBonePosition();

							Matrix44 m;
							m.SetIdentity();
							m=GetTranslationMat(pEnt->GetPos())*m;
							m=Matrix44::CreateRotationZYX(-pEnt->GetAngles()*gf_DEGTORAD)*m; //NOTE: angles in radians and negated 
							Center=m.TransformPointOLD(Center);
						}
					} 
				} 
				else
				{
					Center=pEnt->GetPos();
				}
			}

			Vec3 diff(Center-m_vWpnPos);
					
			float	length2=GetLengthSquared(diff);
			if(length2>250*250 || length2<3*3)
				continue;

			float	px, py, pz;
			m_pGame->GetSystem()->GetIRenderer()->ProjectToScreen(Center.x, Center.y, Center.z, &px, &py, &pz);
			Position.x=(float)px*8.0f;
			Position.y=(float)py*6.0f;
			Position.z=(float)pz;

			if ((Position.x>=m_vCrossScreen.x-autoaimWndSize) &&
					(Position.y>=m_vCrossScreen.y-autoaimWndSize) &&
					(Position.x<=m_vCrossScreen.x+autoaimWndSize) &&
					(Position.y<=m_vCrossScreen.y+autoaimWndSize) &&
					(Position.z>0.0f))
			{
				float dist = (Position.x-400)*(Position.x-400) + (Position.y-300)*(Position.y-300);
				if( dist<minDist )
				{
					Vec3 offset = diff.normalized()*2.0f;	// trace not from the weapon position - to skip windows
					if (m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(vectorf(m_vWpnPos + offset), diff, 
						ent_terrain|ent_static,0, &RayHit, 1,pPE, GetEntity()->GetPhysics()))
						continue;
					minDist = dist;
					bestPoint3D = Center;
					bestPointScreen = Position;
					m_bTargetIsLocked = true;
				}
			}		
		}
	}

	//define world positionof the target/crosshair, calculate weapon angles
	if( m_bTargetIsLocked )
		vCrossHair3Dpos = bestPoint3D;
	else
	{
		Vec3	shooterPos, shooterAng;
		if ( shooter->IsMyPlayer())
		{
			if(shooter->m_bFirstPerson)
			{
				shooterAng = shooter->m_vCurAngleVhcl;
				shooterPos = shooter->m_vCurCamposVhcl;
			}
			else
			{
				IEntityCamera *camera = shooter->GetEntity()->GetCamera();
				shooterAng = camera->GetAngles();//shooter->GetEntity()->GetAngles();
				shooterPos = camera->GetPos();//shooter->GetEntity()->GetPos();
			}
		}
		else
		{
			shooterAng = shooter->GetEntity()->GetAngles();
			GetEntity()->GetHelperPosition("eye_pos",shooterPos);
		}

		// get trace direction 
		Matrix44 tm = Matrix44::CreateRotationZYX(-shooterAng*gf_DEGTORAD); //NOTE: angles in radians and negated 
		Vec3 dir = GetTransposed44(tm)*(Vec3d(0,-1,0));
	
		Vec3 offset = dir*3.5f;	// trace not from the weapon position - to skip windows
		dir*=150;

		// find 3d postiton of crosshair
		ray_hit hits[1];
		int	hit=m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(shooterPos + offset, dir, ent_all&~ent_living, rwi_stop_at_pierceable,
																				hits,1, GetEntity()->GetPhysics(), shooter->GetEntity()->GetPhysics() );
		if(hit == 0)
			vCrossHair3Dpos = shooterPos + dir;
		else
			vCrossHair3Dpos = hits[0].pt;
	}
	Matrix33 VehicleMat = Matrix33::CreateRotationXYZ( m_pEntity->GetAngles()*gf_DEGTORAD );
  //this is the only place where we need the parent matrix
  //frist we translate the Target into the space of the Gun, 
	//second we tranform this vector with the transposed vehicle-matrix
	//from know on we treat the gun like on object where the parent has no rotation. 
 	Vec3 GunViewDirection	=	GetNormalized(VehicleMat.T()*(vCrossHair3Dpos-m_vWpnPos));

	float l = GetLength( Vec3(GunViewDirection.x, GunViewDirection.y, 0.0f ) );
	assert(l); //throw assert if length=0
	m_AngleLimitBase = Vec3(0,0,180);

	//calculate the sine&cosine and matrix for rotation around the X-axis
	float angleX = RAD2DEG(atan2_tpl(-GunViewDirection.z,l)); //angle for up-down movement
	if(m_AngleLimitVFlag)
	//check vertical limits	
	{
		float original = angleX;
		angleX = ClampAngle(	Snap_s360(m_AngleLimitBase.x + m_MinVAngle),
								Snap_s360(m_AngleLimitBase.x + m_MaxVAngle),
								Snap_s360(angleX));
		angleX = Snap_s180(angleX);
		if( fabs(original - angleX) > .05f )
			m_bCrossOnScreen = false;
	}
	angleX = DEG2RAD(angleX);


//	if (angleX>+0.60f) angleX=+0.60f; //limit up-movement of gun
//	if (angleX<-0.40f) angleX=-0.40f; //limit down-movement of gun
	Matrix33 UpDownMat=Matrix33::CreateRotationX(angleX); 

	//calculate the sine&cosine and matrix for rotation around the Z-axis
	float angleZ = RAD2DEG( atan2_tpl(GunViewDirection.x/l,-GunViewDirection.y/l));	//angle for left-right movement
	bool bClamped=false;
	if(m_AngleLimitHFlag)
	{
	//check horizontal limits
		float original = angleZ;
		angleZ = ClampAngle(	Snap_s360(m_AngleLimitBase.z + m_MinHAngle),
								Snap_s360(m_AngleLimitBase.z + m_MaxHAngle),
								Snap_s360(angleZ));
		angleZ = Snap_s180(angleZ);
		if( fabs(original - angleZ) > .05f )
		{
			m_bCrossOnScreen = false;
			bClamped = true;
		}
	}
	angleZ = DEG2RAD(angleZ);
	Matrix33 LeftRightMat=Matrix33::CreateRotationZ(angleZ);;

	//we concatenate all 3 matrices to get the final gun-matrix in world-space
	Matrix33 FinalGunMat=VehicleMat*LeftRightMat*UpDownMat;
	m_vWpnAng = RAD2DEG(Ang3::GetAnglesXYZ( FinalGunMat ));

//	shooter->m_stats.crosshairOnScreen = m_bCrossOnScreen;
	int wpnId = GetGame()->GetWeaponSystemEx()->GetWeaponClassIDByName( GetWeaponName( shooter->m_stats.inVehicleState ) );
	if( wpnId == -1 )
	{
		shooter->m_stats.crosshairOnScreen = true;
	}
	else if(!m_bCrossOnScreen)
	{
		// stop firing - stop sounds
		// stop animation
		if(shooter->m_stats.crosshairOnScreen)
		{
			CWeaponClass *pSelectedWeapon = shooter->GetSelectedWeapon();
			if(pSelectedWeapon)
			{
				pSelectedWeapon->ScriptOnStopFiring(shooter->GetEntity());
				WeaponState( shooter->GetEntity()->GetId(), false );
			}
		}
		shooter->m_stats.crosshairOnScreen  = false;
		shooter->SetWaitForFireRelease(true);
	}
	else
	{
		shooter->SetWaitForFireRelease(false);
		shooter->m_stats.crosshairOnScreen = true;
	}
	return;
}


void CVehicle::UpdateWeaponLimitRotation( Vec3& unlimitedPos, bool bClamped )
{
	if( bClamped )
	{
		if( m_vWpnAngNoSnap.z*unlimitedPos.z>=0 || fabs(unlimitedPos.z)<20.0f )
		{
			m_vWpnAngDelta.z = 0.0f;
			m_vWpnAng = m_vWpnAngNoSnap;
			return;
		}
		if(m_vWpnAngNoSnap.z >0 )
			m_vWpnAngDelta.z=1.0f;
		else
			m_vWpnAngDelta.z=-1.0f;
	}
	if( m_vWpnAngDelta.z == 0.0f )
	{
		m_vWpnAng = m_vWpnAngNoSnap;
		return;
	}

float	timeScale = m_pGame->GetSystem()->GetITimer()->GetFrameTime();
	timeScale*=730.0f;
	if( fabs(Snap_s360(m_vWpnAngNoSnap.z) - Snap_s360(m_vWpnAng.z)) < timeScale*7.0f )
	{
		m_vWpnAngDelta.z = 0.0f;
		m_vWpnAng = m_vWpnAngNoSnap;
		return;
	}

	m_vWpnAng.x = m_vWpnAngNoSnap.x;
	m_vWpnAng.y = m_vWpnAngNoSnap.y;
	if(m_vWpnAngDelta.z>0) 
		m_vWpnAng.z += timeScale;
	else
		m_vWpnAng.z -= timeScale;

return;
/*
//	m_vWpnAngNoSnap = curPos;
//	return;

	if( bClamped )
	{
		m_vWpnAngDelta.z = Snap_s360(curPos.z) - Snap_s360(m_vWpnAngNoSnap.z);
	}
	if( m_vWpnAngDelta.z == 0.0f )
	{
		m_vWpnAngNoSnap = curPos;
		return;
	}
float	timeScale = m_pGame->GetSystem()->GetITimer()->GetFrameTime();
	timeScale*=730.0f;
	if( fabs(m_vWpnAngNoSnap.z - curPos.z) < timeScale*7.0f )
	{
		m_vWpnAngDelta.z = 0.0f;
		m_vWpnAngNoSnap = curPos;
		return;
	}

	m_vWpnAngNoSnap.x = curPos.x;
	m_vWpnAngNoSnap.y = curPos.y;
	if(m_vWpnAngDelta.z>0) 
		m_vWpnAngNoSnap.z += timeScale;
	else
		m_vWpnAngNoSnap.z -= timeScale;
*/
}


bool CVehicle::AnglesToLimit( Vec3& angl )
{
	m_AngleLimitBase = Vec3(0,0,180);
	m_bCrossOnScreen = true;

	if(m_AngleLimitVFlag)
	//check vertical limits	
	{
		float original = angl.x;
		angl.x = ClampAngle(	Snap_s360(m_AngleLimitBase.x + m_MinVAngle),
								Snap_s360(m_AngleLimitBase.x + m_MaxVAngle),
								Snap_s360(angl.x));
		angl.x = Snap_s180(angl.x);
		if( fabs(original - angl.x) > .05f )
			m_bCrossOnScreen = false;

	}
	if(m_AngleLimitHFlag)
	{
	//check horizontal limits
		float original = angl.z;
		angl.z = ClampAngle(	Snap_s360(m_AngleLimitBase.z + m_MinHAngle),
								Snap_s360(m_AngleLimitBase.z + m_MaxHAngle),
								Snap_s360(angl.z));
		angl.z = Snap_s180(angl.z);
		if( fabs(original - angl.z) > .05f )
		{
			m_bCrossOnScreen = false;
			return true;
		}
	}
	return false;
}
