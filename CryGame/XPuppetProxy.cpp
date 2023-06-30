
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// XPuppetProxy.cpp: implementation of the CXPuppetProxy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XPuppetProxy.h"
#include <IScriptSystem.h>
#include "Game.h"
#include "XPlayer.h"
#include "XVehicle.h"
#include "WeaponClass.h"
#include "ScriptObjectVector.h"
#include "XEntityProcessingCmd.h"
#include <CryCharAnimationParams.h>
#include <float.h>

#include <IAISystem.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXPuppetProxy::CXPuppetProxy(IEntity *pEntity,IScriptSystem *pScriptSystem, CXGame *pGame):pSignalTable(pScriptSystem),pJumpTable(pScriptSystem)
{
	m_fMovementRestrictionDuration = -1;
	m_bAllowedToMove = false;
	m_nWeaponBoneID = -1;
	m_bWasSwimming = false;
	m_bFireOverride = false;
	m_bInJump = false;
	m_pScriptSystem = pScriptSystem;
	m_pEntity = pEntity;
	m_nLastBodyPos = 0;
	m_vPreviousTargetPos = pEntity->GetPos();

	if (m_pEntity->GetContainer())
	{
		if (!m_pEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &m_pPlayer))
			m_pPlayer = 0;
	}
	else
		m_pPlayer = 0;

//	m_pPlayer->SetEntity(pEntity);
	m_bDead = false;
	m_fBackwardSpeed = 0;
	m_fForwardSpeed = 0;
	m_vHeadDir = pEntity->GetAngles();
	m_vHeadDir=ConvertToRadAngles(m_vHeadDir);
	m_vTorsoDir = m_vHeadDir;
	m_vBodyDir = m_vHeadDir;
	m_pGame = pGame;

	m_AIHandler.Init( m_pGame, m_pEntity, m_pGame->GetSystem()->GetILog() );
	m_pEntity->GetScriptObject()->GetValue("JumpSelectionTable",pJumpTable);

	bool bFish = false;
	m_pEntity->GetScriptObject()->GetValue("IsFish",bFish);
	m_pPlayer->m_bIsFish = bFish;

	m_pCharacterPhysics = 0;
	ICryCharInstance *pInstance = m_pEntity->GetCharInterface()->GetCharacter(0);
	if (pInstance)
	{
		if (pInstance->GetModel())
		{
			m_nWeaponBoneID = pInstance->GetModel()->GetBoneByName("Bip01 R Forearm");
		}
	}

	m_pAISystem = NULL;
}

CXPuppetProxy::~CXPuppetProxy()
{
	//delete m_pPlayer;
	ICryCharInstance *pInstance = m_pEntity->GetCharInterface()->GetCharacter(0);
	if (pInstance)
		pInstance->RemoveAnimationEventSink(this);
}

int CXPuppetProxy::Update(SOBJECTSTATE *state)
{
	FUNCTION_PROFILER( m_pGame->GetSystem(),PROFILE_AI );

	if (m_pPlayer)
	{
		if (!m_pPlayer->IsAlive())
			return 0;
	}

	m_pScriptSystem->BeginCall("BasicAI","DoChatter");
	m_pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
	m_pScriptSystem->EndCall();

	// new piece that deals with stopping firing when the enemy is shot
	/*if (m_nWeaponBoneID>=0)
	{  
		FRAME_PROFILER( "AIShotReaction",m_pGame->GetSystem(),PROFILE_AI );
		pe_status_joint pjs;
		pjs.idChildBody = m_nWeaponBoneID;
		ICryCharInstance *pInstance = m_pEntity->GetCharInterface()->GetCharacter(0);
		if (pInstance)
		{
			m_pCharacterPhysics = pInstance->GetCharacterPhysics();
			if (m_pCharacterPhysics)
			{
				m_pCharacterPhysics->GetStatus(&pjs);
				if (pjs.q.Length() > 0.001f)
				{
					SAIEVENT event;
					event.fInterest = 0.2f;
					m_pEntity->GetAI()->Event(AIEVENT_ONBODYSENSOR,&event);
				}
			}
		}
	}
	*/

	Vec3d angles = m_pEntity->GetAngles(1);
    
	if(!m_pEntity->IsBound() || m_pPlayer->GetRedirected())	// move only if not bound 
//	if(1)
	{



		if ((-1e+9 < state->fValue && state->fValue < 1e+9))
		{
			// handle turning
			if (state->turnleft || state->turnright)
			{
				angles.z+=state->fValue;
			}
		}
		else
			state->fValue = 0;

		if ((-1e+9 < state->fValueAux && state->fValueAux < 1e+9))
		{
			angles.x+=state->fValueAux;
		}
		else
			state->fValueAux = 0;
		

		CXEntityProcessingCmd tempCommand;
		tempCommand.SetDeltaAngles(angles);

		//--------------------
		if (m_pPlayer)
				m_pPlayer->ProcessAngles(tempCommand);
		else
			m_pEntity->SetAngles(angles,true,false);


		if (state->left || state->right)// && state->bHaveTarget)
		{
			Vec3d ang = m_pEntity->GetAngles();
			ang=ConvertToRadAngles(ang);	

			//Vec3d leftdir = state->vTargetPos - m_pEntity->GetPos();
			Vec3d leftdir = ang;
			Matrix44 mat;
			mat.SetIdentity();
			if (state->left)
			{
				//mat.RotateMatrix(Vec3d(0,0,90));
		    mat=Matrix44::CreateRotationZYX(-Vec3d(0,0,+90)*gf_DEGTORAD)*mat; //NOTE: angles in radians and negated 
			}
			else
			{
				//mat.RotateMatrix(Vec3d(0,0,-90));
				mat=Matrix44::CreateRotationZYX(-Vec3d(0,0,-90)*gf_DEGTORAD)*mat; //NOTE: angles in radians and negated 
			}
			leftdir = mat.TransformPointOLD(leftdir);
			state->vMoveDir+=leftdir;
		}

		pe_action_move motion;
		// motion.dir.set(0,0,0);
		if (state->vMoveDir.Length() > 0)
		{
				motion.dir = GetNormalized(state->vMoveDir);
				//motion.dir.z = 0;
				//motion.dir.normalize();
		}
		else
			motion.dir.Set(0,0,0);

		if (state->jump)
		{
			FRAME_PROFILER( "AIJumpProcessing",m_pGame->GetSystem(),PROFILE_AI );
			if (m_pPlayer)
				m_pPlayer->m_stats.jumping = true;

			m_vJumpVector = state->vJumpDirection;
			m_fLastJumpDuration = state->fJumpDuration;
			m_nJumpDirection = (int)(state->nJumpDirection);

			_SmartScriptObject pDesiredJumpType(m_pScriptSystem,true);
			if (pJumpTable->GetAt(m_nJumpDirection,pDesiredJumpType))
			{
				_SmartScriptObject pDesiredJump(m_pScriptSystem,true);
				int cnt = pDesiredJumpType->Count();
				if (cnt>1)
					cnt = (rand() % (cnt)) + 1;
				m_nLastSelectedJumpAnim = cnt;
                if (pDesiredJumpType->GetAt(cnt,pDesiredJump))
				{
					const char *pAnimName;
					int nTakeoffFrame, nLandFrame;
					pDesiredJump->GetAt(1,pAnimName);
					pDesiredJump->GetAt(2,nTakeoffFrame);
					pDesiredJump->GetAt(3,nLandFrame);
					ICryCharInstance *pCharacter = m_pEntity->GetCharInterface()->GetCharacter(0);
					if (pCharacter)
					{
						// set up callbacks to jump and land correctly
						m_pGame->GetSystem()->GetILog()->Log("\001 [AIWARNING] Entity %s set a callback for animation %s (char %p)",m_pEntity->GetName(), pAnimName, pCharacter);
						pCharacter->AddAnimationEventSink(pAnimName,this);
						pCharacter->AddAnimationEvent(pAnimName,nTakeoffFrame,(void*)JUMP_TAKEOFF);
						pCharacter->AddAnimationEvent(pAnimName,nLandFrame,(void*)JUMP_LAND);
						m_pEntity->StartAnimation(0,pAnimName,3,0.1f);
						
						SAIEVENT sae;
						sae.fInterest = m_fLastJumpDuration;
						sae.fInterest += m_pEntity->GetAnimationLength(pAnimName) - ((nLandFrame-nTakeoffFrame)*0.03f);
						m_pEntity->GetAI()->Event(AIEVENT_ONBODYSENSOR,&sae);
						
						MovementControl(false, sae.fInterest);

					}
					else
						m_pGame->GetSystem()->GetILog()->Log("\003 [AIWARNING] Entity %s tried to jump but has no animated character!!",m_pEntity->GetName());

				}
			}
			else
				m_pGame->GetSystem()->GetILog()->Log("\003 [AIWARNING] No jumps specified for a certain direction for entity %s",m_pEntity->GetName());
			state->jump = false;
		}
		else
		{
			m_AIHandler.SetCurrentBehaviourVariable("fLastJumpDuration",state->fJumpDuration);
			if (m_pPlayer)
				m_pPlayer->m_stats.jumping = false;

			motion.dir *= m_fForwardSpeed;
			


			if( m_pPlayer->m_stats.bIsLimping )
			{
				motion.dir*=.73f;
			}
			else
				switch (state->bodystate)
			{
				case BODYPOS_STAND:
					if (state->run)
						motion.dir*=m_pPlayer->m_RunSpeedScale;
					//					motion.dir*=m_pGame->p_ai_runspeedmult->GetFVal();
					//					motion.dir*=1.5f;
					break;
				case BODYPOS_CROUCH:
					//				motion.dir*=0.75f;
					//				motion.dir*=m_pGame->p_ai_crouchspeedmult->GetFVal();
					motion.dir*=m_pPlayer->m_CrouchSpeedScale;
					break;
				case BODYPOS_PRONE:
					//				motion.dir*=0.5f;
					//				motion.dir*=m_pGame->p_ai_pronespeedmult->GetFVal();
					motion.dir*=m_pPlayer->m_ProneSpeedScale;
					break;
				case BODYPOS_STEALTH:
					if (state->run)
						//					motion.dir*=1.5f;
						//					motion.dir*=m_pGame->p_ai_xrunspeedmult->GetFVal();
						motion.dir*=m_pPlayer->m_XRunSpeedScale;
					else
						//					motion.dir*=0.4f;
						//					motion.dir*=m_pGame->p_ai_xwalkspeedmult->GetFVal();
						motion.dir*=m_pPlayer->m_XWalkSpeedScale;
					break;
				case BODYPOS_RELAX:
					if (state->run)
						//					motion.dir*=1.5f;
						//					motion.dir*=m_pGame->p_ai_rrunspeedmult->GetFVal();
						motion.dir*=m_pPlayer->m_RRunSpeedScale;
					else
						//					motion.dir*=0.4f;
						//					motion.dir*=m_pGame->p_ai_rwalkspeedmult->GetFVal();
						motion.dir*=m_pPlayer->m_RWalkSpeedScale;
					break;

			}

		}

		pe_player_dynamics pdyn;
		m_pEntity->GetPhysics()->GetParams(&pdyn);

		if (m_pPlayer)
		{

			// handle inertia
			// actual intertia change is done in CPlayer::UpdatePhysics
			// same for AirControl/bSwimming 
			if (((state->bCloseContact || m_bInJump) && !m_pPlayer->IsSwimming()) || m_pPlayer->m_bIsFish )
				m_pPlayer->m_bInertiaOverride = true;
			else
				m_pPlayer->m_bInertiaOverride = false;
/*
			// handle inertia
			if ((state->bCloseContact || m_bInJump) && !m_pPlayer->IsSwimming() )
				pdyn.kInertia = 0;
			else
			{
				if (m_bIsFish)
					pdyn.kInertia = 0;
				else
					pdyn.kInertia = 4;
			}
		
//			if (m_pPlayer->m_fGravityOverride!=1E10 || m_pPlayer->IsSwimming())
			if (m_pPlayer->m_stats.onLadder || m_pPlayer->IsSwimming())
			{
				pdyn.kAirControl = 1;
				pdyn.bSwimming = 1;
			}
			else
			{
				pdyn.kAirControl = 0;
				pdyn.bSwimming =0 ;
			}
*/
//			if (m_pPlayer->m_fGravityOverride!=1E10 || m_pPlayer->IsSwimming())
			if (m_pPlayer->IsSwimming())
			{
				// keep puppet on surface
				if(	m_pPlayer->m_stats.fInWater>.4f)
					motion.dir.z = 4.0f;

//					motion.dir.z = .20f;
				// when swimming underwater and goung up by press JUMP 
				// if too close to surface - don't aplpy much UP impulse 
				// to prevent jumping out of water
				if(	m_pPlayer->m_stats.fInWater < m_pPlayer->m_PlayerDimSwim.heightEye)
					motion.dir.z *= m_pPlayer->m_stats.fKWater*.02f;	// if close to surface - don't go up too fast - not to jump out of water


				if (!m_bWasSwimming )
				{
					if (!m_pPlayer->m_bIsFish)
					{
						// player entered water
						m_pEntity->GetAI()->SetSignal(1,"START_SWIMMING");
						m_bWasSwimming = true;
					}
				}
					

			}
			else if (m_bWasSwimming)
			{
				if (!m_pPlayer->m_bIsFish)
				{
					// player left water
					m_pEntity->GetAI()->SetSignal(-1,"STOP_SWIMMING");
					m_bWasSwimming = false;
				}
			}
		}

		//smooth AI movement, if AI is indoor is possible to use different acceleration values (if m_pPlayer->m_input_accel_indoor>0.0f).
		if (m_pPlayer)
		{
			bool indoor = false;
			float accel = m_pPlayer->m_input_accel;
			float decel = m_pPlayer->m_input_stop_accel;

			//if is the first time, get AISystem pointer.
			if (!m_pAISystem)
				m_pAISystem = m_pGame->GetSystem()->GetAISystem();

			int building;
			IVisArea *pArea;
							
			if (m_pAISystem && ((IAISystem *)m_pAISystem)->CheckInside(m_pEntity->GetPos(),building,pArea))
			{
				indoor = true;

				if (m_pPlayer->m_input_accel_indoor>0.0f)
				{
					accel = m_pPlayer->m_input_accel_indoor;
					decel = m_pPlayer->m_input_stop_accel_indoor;
					//m_pGame->GetSystem()->GetILog()->Log("%s indoor(%.1f,%.1f)",m_pEntity->GetName(),accel,decel);
				}
			}

			//force AI to use smaller acceleraion near target only when outdoor, 
			//indoors have fDistanceFromTarget pretty small all the times and this cause AI to smooth to much, missing doors.
			if (state->fDistanceFromTarget>0 && !indoor) 
				accel = min(state->fDistanceFromTarget,accel);
	
			m_pPlayer->DampInputVector(motion.dir,accel,decel,true,false);
		}

		// move entity
		ApplyMovement(&motion,state);
		

//		this is done in CPlayer::UpdatePhysics now
//		m_pEntity->GetPhysics()->SetParams(&pdyn);
		m_pPlayer->m_vDEBUGAIFIREANGLES = m_pEntity->GetAngles();

		// handle fire of weapon
		int temp;
		if (m_pPlayer && !m_pEntity->GetScriptObject()->GetValue("NEVER_FIRE",temp))
		{
			FRAME_PROFILER( "AIWeaponsFire",m_pGame->GetSystem(),PROFILE_AI );
			m_pPlayer->m_aimLook = state->aimLook;
			
			Vec3d fireangles = state->vFireDir;

			Vec3d mvmtDir = state->vTargetPos - m_vPreviousTargetPos;
			if (mvmtDir.len2() > 20.f)
			{
				fireangles = state->vTargetPos + GetNormalized(mvmtDir)*4.5f;
				fireangles -= m_pEntity->GetPos();
			}

			fireangles = ConvertVectorToCameraAngles(fireangles);
			m_pPlayer->m_vDEBUGAIFIREANGLES = fireangles;

			if (m_bFireOverride)
			{
				m_pPlayer->m_stats.firing=true;
				m_pPlayer->m_aimLook = true;
				m_bFireOverride = false;
			}
			else
			{
				m_pPlayer->m_aimLook = state->aimLook;
				if (state->fire)
				{
					m_pPlayer->m_stats.firing=true;
					//m_pPlayer->m_aimLook = true;
					m_pPlayer->HoldWeapon();
				}
				else
					m_pPlayer->m_stats.firing=false;
			}

		}
		else
			m_pPlayer->m_stats.firing = false;

//comment it
//m_pPlayer->m_aimLook = true;

		//Account for body position-----------------------------
		if (m_nLastBodyPos != state->bodystate)
		{
			FRAME_PROFILER( "AISwitchingStances",m_pGame->GetSystem(),PROFILE_AI );
			switch (state->bodystate)
			{
				case 1:
					m_dimCurrentDimensions = m_dimCrouchingPuppet;
					if (m_pPlayer)
						m_pPlayer->GoCrouch();
					break;

				case 2:
					m_dimCurrentDimensions = m_dimProningPuppet;
					if (m_pPlayer)
						m_pPlayer->GoProne();
					break;

				case 0:
					m_dimCurrentDimensions = m_dimStandingPuppet;
					if (m_pPlayer)
						m_pPlayer->GoStand();
					break;

				case 5:
					m_dimCurrentDimensions = m_dimStandingPuppet;
					if (m_pPlayer)
						m_pPlayer->GoStealth();
					break;

				case 4:
					m_dimCurrentDimensions = m_dimProningPuppet;
					break;

				case 3:
					m_dimCurrentDimensions = m_dimStandingPuppet;
					if (m_pPlayer)
						m_pPlayer->GoRelaxed();
					break;
			}
		
			//<<FIXME>> PUppets that will not be players????
			//m_pEntity->GetPhysics()->SetParams(&m_dimCurrentDimensions);
			m_nLastBodyPos = state->bodystate;
		}
	}
	else
	{
		if (m_pPlayer)
		{

			angles = m_pEntity->GetAngles( 1 );
	// handle turning
			if (state->turnleft || state->turnright)
				angles.z+=state->fValue;	
	
			angles.x+=state->fValueAux;	

			CXEntityProcessingCmd tempCommand;
			tempCommand.SetDeltaAngles(angles);
	//--------------------
			m_pPlayer->ProcessAngles(tempCommand);

//			m_pPlayer->m_aimLook = state->aimLook;
			if (state->fire)
			{
				Vec3d fireangles = state->vFireDir;
				fireangles=ConvertVectorToCameraAngles(fireangles);
				m_pPlayer->m_vDEBUGAIFIREANGLES =fireangles;
				//m_pPlayer->SetFiring(true);
				m_pPlayer->m_stats.firing=true;
				m_pPlayer->m_aimLook = true;
//				bAiming = true;
//				m_pPlayer->GoAim( );

//				m_pPlayer->m_stats.firing=true;
			}
			else
			{
				m_pPlayer->m_stats.firing=false;
			}
		}
	}


	//-------------------------------------------



	if (!(m_LastObjectState == *state))
	{
		m_LastObjectState = *state;
		UpdateMotor(state);
	}

	bool bSkipNextUpdate = false;
	{
		FRAME_PROFILER( "AISignalProcessing",m_pGame->GetSystem(),PROFILE_AI );
	while (!state->vSignals.empty())
	{
		AISIGNAL sstruct = state->vSignals.back();
		state->vSignals.pop_back();
		int signal = sstruct.nSignal;
		const char *szText = &sstruct.strText[0];
		IEntity* pSender= (IEntity*) sstruct.pSender;
		
		switch (signal)
		{
			case -10:
				// throw a grenate
				if (m_pPlayer)
				{
					Vec3d firepos,fireangles;
					firepos = m_pPlayer->GetEntity()->GetPos();
					firepos.z+=2.f;
					fireangles = m_pPlayer->GetEntity()->GetAngles();
					m_pPlayer->m_fGrenadeTimer=3;
					m_pPlayer->FireGrenade(firepos,fireangles,m_pPlayer->GetEntity());
				}
				//state->pSignalSender=0;
				break;
			case -20:
				bSkipNextUpdate = true;
				break;
		}

		SendSignal(signal,szText,pSender);		
		//state->nSignal = 0;
		//state->szSignalText = 0;
	}
	}

	if (state->nAuxSignal)
	{
		FRAME_PROFILER( "AIReadibilityProcessing",m_pGame->GetSystem(),PROFILE_AI );
		SendAuxSignal(state->nAuxSignal,state->szAuxSignalText.c_str());
		state->nAuxSignal = 0;
	}


	// send SYSTEM GAME EVENTS LAST!!! They are most important.
	if (state->bReevaluate && !bSkipNextUpdate)
		UpdateMind(state);

    
	if (state->bHaveTarget)
		m_vPreviousTargetPos = state->vTargetPos;
	
	return 0;
}


bool CXPuppetProxy::QueryProxy(unsigned char type, void **pProxy)
{
	if (type == AIPROXY_PUPPET)
	{
		*pProxy = (void *) this;
		return true;
	}
	else
		return false;
}

void CXPuppetProxy::SetRootBone(const char *pRootBone)
{
	m_strRootBone = pRootBone;
}

void CXPuppetProxy::SetSpeeds(float fwd, float bkw)
{
	m_fForwardSpeed = fwd;
	m_fBackwardSpeed = bkw;
	if(m_pPlayer)
		m_pPlayer->SetWalkSpeed( m_fForwardSpeed );
}



void CXPuppetProxy::SetPuppetDimensions(float height, float eye_height, float sphere_height, float radius)
{
	m_dimStandingPuppet.heightEye = eye_height;
	m_dimStandingPuppet.heightCollider = sphere_height;
	m_dimStandingPuppet.sizeCollider.Set(0.6f,0.6f,radius);

	m_dimCrouchingPuppet.heightEye = eye_height * 0.5f;
	m_dimCrouchingPuppet.heightCollider = sphere_height * 0.5f;
	m_dimCrouchingPuppet.sizeCollider.Set(0.6f, 0.6f, radius*0.5f);

	m_dimProningPuppet.heightEye = eye_height *0.2f;
	m_dimProningPuppet.heightCollider = sphere_height * 0.2f;
	m_dimProningPuppet.sizeCollider.Set(0.7f, 0.6f, radius*0.2f);
}


void CXPuppetProxy::GetDimensions(int bodypos, float &eye_height, float &height)
{
	if (bodypos)
	{
		if (bodypos == 1)
		{
			eye_height = m_dimCrouchingPuppet.heightEye;
      height = m_dimCrouchingPuppet.heightEye+0.05f;
		}
		else
		{
			eye_height = m_dimProningPuppet.heightEye;
      height = m_dimProningPuppet.heightEye+0.05f;
		}
	}
	else 
	{
		eye_height = m_dimStandingPuppet.heightEye;
		height = m_dimStandingPuppet.heightEye+0.05f;
	}
}
																	

int CXPuppetProxy::UpdateMotor(SOBJECTSTATE *state)
{
	// create table to pass to motor script
/*	_SmartScriptObject pTable(m_pScriptSystem);
	pTable->SetValue("turning",(state->turnleft || state->turnright));
	pTable->SetValue("forward",state->forward);
	pTable->SetValue("back",state->back);
	pTable->SetValue("left",state->left);
	pTable->SetValue("right",state->right);
	pTable->SetValue("bodypos",state->bodystate);
	pTable->SetValue("run",state->run);
	pTable->SetValue("signal",state->nSignal);
	pTable->SetValue("health",state->fHealth);
	pTable->SetValue("moving",(state->back || state->forward || state->left || state->right));
	

	// call script for cool animations
	m_pScriptSystem->BeginCall(m_pEntity->GetEntityClassName(),m_strMotorFuncName.c_str());
	m_pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
	m_pScriptSystem->PushFuncParam(*pTable);
	m_pScriptSystem->EndCall();*/
	return 0;
}



void CXPuppetProxy::UpdateMind(SOBJECTSTATE *state)
{

	m_AIHandler.AIMind( state );
	state->bReevaluate = false;
	return;


	bool bScriptControl = false;

	if (!state->bHaveTarget)
		int a=5;

	_SmartScriptObject pTable(m_pScriptSystem);
	pTable->SetValue("haveTarget",state->bHaveTarget);
	pTable->SetValue("fInterest",state->fInterest);
	pTable->SetValue("fThreat",state->fThreat);
	pTable->SetValue("fDistance",state->fDistanceFromTarget);
	pTable->SetValue("nType",state->nTargetType);
//	pTable->SetValue("nSignal",state->nSignal);
	pTable->SetValue("bMemory",state->bMemory);
	pTable->SetValue("bSound",state->bSound);
	pTable->SetValue("bTakingDamage",state->bTakingDamage);
	pTable->SetValue("bTargetEnabled",state->bTargetEnabled);
	
	
	m_pScriptSystem->BeginCall( m_hBehaviourFunc );
//	m_pScriptSystem->BeginCall(m_pEntity->GetEntityClassName(),m_strBehaviourFuncName.c_str());
	m_pScriptSystem->PushFuncParam(m_pEntity->GetScriptObject());
	m_pScriptSystem->PushFuncParam(*pTable);
	m_pScriptSystem->EndCall(bScriptControl);

	state->bReevaluate = false;
	
}



void CXPuppetProxy::Reset(void)
{
	if (m_pPlayer)
			m_pPlayer->m_stats.FiringType = eNotFiring;
}


void CXPuppetProxy::SendSignal(int signalID, const char * szText, IEntity *pSender)
{
	m_pEntity->SetNeedUpdate( true );
	m_AIHandler.AISignal( signalID, szText, pSender );
}

void CXPuppetProxy::SendAuxSignal(int signalID, const char * szText)
{
	m_AIHandler.DoReadibilityPack(szText);
}

void CXPuppetProxy::SetSignalFunc(HSCRIPTFUNCTION pFunc)
{
	m_hSignalFunc.Init(m_pScriptSystem,pFunc);
}

void CXPuppetProxy::SetBehaviourFunc(HSCRIPTFUNCTION pFunc)
{
	m_hBehaviourFunc.Init(m_pScriptSystem,pFunc);
	
}

void CXPuppetProxy::SetMotorFunc(HSCRIPTFUNCTION pFunc)
{
	m_hMotorFunc.Init(m_pScriptSystem,pFunc);
}



//--------------------------------------------------------------------------------------------

bool CXPuppetProxy::CustomUpdate(Vec3d & vPos, Vec3d & vAngles)
{
	if (m_pPlayer)
	{
		if (_isnan(m_pPlayer->m_vCharacterAngles.x) || _isnan(m_pPlayer->m_vCharacterAngles.y) || _isnan(m_pPlayer->m_vCharacterAngles.z))
			GameWarning("m_vCharacterAngles for entity %s are NaN",m_pEntity->GetName());
		else
			vAngles = m_pPlayer->m_vCharacterAngles;
	}
	return false;
}


void CXPuppetProxy::DebugDraw(struct IRenderer * pRenderer)
{
	pRenderer->TextToScreenColor(50,66,0.3f,0.3f,0.3f,1.f,"- Proxy Information --");

	const char *szCurrentBehaviour=0;
	const char *szPreviousBehaviour=0;
	const char *szFirstBehaviour=0;

	if (m_AIHandler.m_pBehavior)
		m_AIHandler.m_pBehavior->GetValue("Name",szCurrentBehaviour);

	if (m_AIHandler.m_pPreviousBehavior)
		m_AIHandler.m_pPreviousBehavior->GetValue("Name",szPreviousBehaviour);
	
	if (!m_AIHandler.m_FirstBehaviorName.empty())
		szFirstBehaviour = m_AIHandler.m_FirstBehaviorName.c_str();

	

	pe_player_dynamics pdyn;
	m_pEntity->GetPhysics()->GetParams(&pdyn);
	pRenderer->TextToScreen(50,70,"GRAVITY IN PHYSICS :%.3f  AirControl: %.3f",pdyn.gravity,pdyn.kAirControl);

	if (m_pPlayer)
	{
		if (m_pPlayer->m_weaponPositionState == 1)
			pRenderer->TextToScreen(50,72,"WEAPON HOLSTERED");
		else if (m_pPlayer->m_weaponPositionState == 2)
			pRenderer->TextToScreen(50,72,"HOLDING WEAPON");
		else
			pRenderer->TextToScreen(50,72,"WEAPON POS UNDEFINED");
	}

	pe_status_living pliv;
	m_pEntity->GetPhysics()->GetStatus(&pliv);
	if (IsEquivalent(pliv.vel,pliv.velRequested,0.1f))
		pRenderer->SetMaterialColor(1,1,1,1);
	else
		pRenderer->SetMaterialColor(1,0,0,1);
	pRenderer->TextToScreen(40,66,"VEL_REQUESTED:(%.2f,%.2f,%.2f)  ACTUAL_VEL:(%.2f,%.2f,%.2f)",
		pliv.velRequested.x,pliv.velRequested.y,pliv.velRequested.z,pliv.vel.x,pliv.vel.y,pliv.vel.z);
	

	

	pRenderer->TextToScreen(50,74,"BEHAVIOUR: %s",szCurrentBehaviour);
	pRenderer->TextToScreen(50,76," PREVIOUS BEHAVIOUR: %s",szPreviousBehaviour);
	pRenderer->TextToScreen(50,78," DESIGNER ASSIGNED BEHAVIOUR: %s",szFirstBehaviour);


	ICryCharInstance *pInstance = m_pEntity->GetCharInterface()->GetCharacter(0);
	if (pInstance)
	{
		for (int i=0;i<5;i++)
		{
			int nId=-1;
			if ((nId = pInstance->GetCurrentAnimation(i))>=0)
			{
				pRenderer->TextToScreen(50.f,80.f+2*i," LAYER %d: ANIM: %s",i,pInstance->GetModel()->GetAnimationSet()->GetName(nId));
			}
		}

		pRenderer->TextToScreen(50,68,"Current animation scale %.3f",pInstance->GetAnimationSpeed());
	}

}


void CXPuppetProxy::OnAnimationEvent(const char *sAnimation,AnimSinkEventData UserData)
{
	// only used for synchronizing animation jumps
	if (UserData.p == (void*)JUMP_TAKEOFF)
	{
		m_bInJump = true;
		pe_action_move motion;
		motion.dir = m_vJumpVector;
		motion.iJump = 1;
		m_pEntity->GetPhysics()->Action(&motion);
		m_pGame->GetSystem()->GetILog()->Log("\003 [AIWARNING] -------%s--------------- NOW APPLYING JUMP ACTION!! --------------------------------",sAnimation);

		_SmartScriptObject pDesiredJumpType(m_pScriptSystem,true);
		if (pJumpTable->GetAt(m_nJumpDirection,pDesiredJumpType))
		{
			_SmartScriptObject pDesiredJump(m_pScriptSystem,true);
			if (pDesiredJumpType->GetAt(m_nLastSelectedJumpAnim,pDesiredJump))
			{
				int nTakeoffFrame, nLandFrame;
				pDesiredJump->GetAt(2,nTakeoffFrame);
				pDesiredJump->GetAt(3,nLandFrame);
					
				float in_air_duration = (float)(nLandFrame-nTakeoffFrame) * (1.f/30.f);
						
				m_pPlayer->m_AnimationSystemEnabled = 0;
				m_pEntity->SetAnimationSpeed(in_air_duration/m_fLastJumpDuration);
				
			}
		}
		
		//0 means takeoff
		m_pEntity->SendScriptEvent( ScriptEvent_Jump,0 );
	}
	else if (UserData.p == (void*)JUMP_LAND)
	{
		m_pGame->GetSystem()->GetILog()->Log("\003 [AIWARNING] -------%s--------------- LANDED!! --------------------------------",sAnimation);
		m_pEntity->SetAnimationSpeed(1.f);
		m_pPlayer->m_AnimationSystemEnabled = 1;

		pe_action_move motion;
		motion.dir.Set(0,0,0);
		motion.iJump = 0;
		m_pEntity->GetPhysics()->Action(&motion);

		//1 means landing
		m_pEntity->SendScriptEvent( ScriptEvent_Jump,1 );
	}
	else
		m_pEntity->OnAnimationEvent(sAnimation,UserData);

}

void CXPuppetProxy::OnEndAnimation(const char *sAnimation)
{

	{
		ICryCharInstance *pCharacter = m_pEntity->GetCharInterface()->GetCharacter(0);
		if (pCharacter)
		{
			m_pGame->GetSystem()->GetILog()->Log("\003 [AIWARNING] ---------------------- %s animation ended ------------------------------",sAnimation);
			m_bInJump = false;
			//m_pGame->GetSystem()->GetILog()->Log("\001 [AIWARNING] Entity %s removed a callback for animation %s (char %p)",m_pEntity->GetName(), sAnimation, pCharacter);
			pCharacter->RemoveAnimationEvent(sAnimation,13,(void*)JUMP_TAKEOFF);
			pCharacter->RemoveAnimationEvent(sAnimation,29,(void*)JUMP_LAND);
			pCharacter->RemoveAnimationEventSink(sAnimation,this);
			m_pEntity->SetAnimationSpeed(1.f);
			m_pPlayer->m_AnimationSystemEnabled = 1;
		}
	}
	
}

bool CXPuppetProxy::CheckStatus(unsigned char status) 
{ 
	switch( status ){
		case AIPROXYSTATUS_INVEHICLE:
			if( m_pPlayer->GetVehicle() )
				return true;
			return false;
	}
	return false; 
}

void CXPuppetProxy::ApplyHealth(float fHealth)
{
	if (m_pPlayer)
	{
		if (fHealth > 1.f)
			m_pPlayer->m_stats.health = (int)fHealth;
	}
}
void CXPuppetProxy::ApplyMovement(pe_action_move * move_params, SOBJECTSTATE *state)
{
	FUNCTION_PROFILER(m_pGame->GetSystem(),PROFILE_AI );

	if (!m_pGame->cv_game_AllowAIMovement->GetIVal())
		m_fMovementRestrictionDuration = 1.f;	// AI is not allowed to move

	if (m_fMovementRestrictionDuration>0)
	{
		m_fMovementRestrictionDuration-=m_pGame->GetSystem()->GetITimer()->GetFrameTime();
		SAIEVENT event;
		event.nDeltaHealth=0;
		m_pEntity->GetAI()->Event(AIEVENT_MOVEMENT_CONTROL,&event);
		move_params->dir.Set(0,0,0);
		move_params->iJump = 0;
		m_pEntity->GetPhysics()->Action(move_params);
		return;

	}

	if (!m_pGame->ai_num_of_bots->GetIVal())
	{
		m_pEntity->GetPhysics()->Action(move_params);
		return;
	}

	

	if (m_SAF.bMoving)
	{
		// puppet is already moving
		m_pEntity->GetPhysics()->Action(move_params);
	}
	else
	{
		// puppet is still stationary
		if (!m_SAF.bMovePending)
		{
			if (state->vMoveDir.Length() > 0)
			{
				m_SAF.bMovePending = true;
				Vec3d lookdir;
				if (state->bHaveTarget)
					lookdir = state->vTargetPos - m_pEntity->GetAI()->GetPos();
				else
					lookdir = state->vMoveDir;
 
				ICryCharInstance *pInstance = m_pEntity->GetCharInterface()->GetCharacter(0);
				if (pInstance)
				{
					CryCharAnimationParams ccap;
					ccap.nLayerID = 0;
					ccap.fBlendInTime=0.1f;
					ccap.fBlendOutTime=0.1f;
					ccap.nFlags = ccap.FLAGS_ALIGNED;

					char animName[64] = "s";
					if (state->bodystate!=BODYPOS_RELAX)
						animName[0] = (state->bodystate==BODYPOS_STAND)?'a':'x';
					strcat(animName, (state->run)?"run":"walk");


					Vec3d angles = ConvertToRadAngles(m_pEntity->GetAI()->GetAngles());
					//if (lookdir.Length()>4.f)
					{
						float fDot = GetNormalized(lookdir).Dot(angles);
 
						if (fDot>0.5f)
						{
							strcat(animName,"fwd_start");
						}
						else if (fDot<-0.5f)
						{
							// moving generally back, use back start anim
							if (state->bHaveTarget)
								strcat(animName,"_turnaround_start");
//								pInstance->StartAnimation("srun_turnaround_start",ccap);
							else
								strcat(animName,"back_start");
								//pInstance->StartAnimation("srunback_start",ccap);
						}
						else
						{
							float zcross = lookdir.x*angles.y - lookdir.y*angles.x;
							if (zcross<0.f)
								strcat(animName,"left_start");
								//pInstance->StartAnimation("srunleft_start",ccap);
							else
								strcat(animName,"right_start");
								//pInstance->StartAnimation("srunright_start",ccap);
								
						}

						pInstance->StartAnimation(animName,ccap);
					}

				} 
			}
		}


		SAIEVENT event;
		event.nDeltaHealth=m_bAllowedToMove?1:0;
		m_pEntity->GetAI()->Event(AIEVENT_MOVEMENT_CONTROL,&event);

		if (m_bAllowedToMove)
			m_pEntity->GetPhysics()->Action(move_params);

	}

	pe_status_living pdyn;
	m_pEntity->GetPhysics()->GetStatus(&pdyn);
	Vec3d velocity = pdyn.vel;
	velocity.z = 0;
	if (velocity.Length()>0.0001f) 
		m_SAF.bMoving = true;
	else
	{
		// don't allow puppet to move
		m_SAF.bMoving = false;
		SAIEVENT event;
		event.nDeltaHealth=0;
		m_pEntity->GetAI()->Event(AIEVENT_MOVEMENT_CONTROL,&event);
	}

	if (state->vMoveDir.Length()<0.0001f)
		m_SAF.bMovePending = false;
}


void CXPuppetProxy::Load(CStream &stm)
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
	int vehicleId;
	stm.Read(vehicleId);
	if(vehicleId)
	{
		IEntity *pVehicleEnt = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(vehicleId);
		if(pVehicleEnt)
		{
			m_pEntity->GetScriptObject()->SetValue("theVehicle", pVehicleEnt->GetScriptObject());
			m_pGame->GetSystem()->GetILog()->Log(" the vehicle is %s",pVehicleEnt->GetName());
		}
	}
}


//
//
void CXPuppetProxy::Load_PATCH_1(CStream &stm)
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


void CXPuppetProxy::Save(CStream &stm)
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
	_SmartScriptObject pVehicle(m_pScriptSystem,true);
	if( m_pEntity->GetScriptObject()->GetValue("theVehicle", pVehicle))
	{
		int id;
		pVehicle->GetValue("id", id);
		stm.Write(id);
		m_pGame->GetSystem()->GetILog()->Log(" Writing vehicle id %d",id);
	}
	else
		stm.Write((int)0);

}