// Puppet.cpp: implementation of the CPuppet class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Puppet.h"
#include <algorithm>
#include "GoalOp.h"
#include "Graph.h"
#include "AIPlayer.h"

#include <IConsole.h>
#include <IPhysics.h>
//<<FIXME>> REMOVE LATER
#include <ISystem.h>
#include <ILog.h>
#include <ITimer.h>
#include "Cry_Math.h"
#include <Cry_Camera.h>
#include "VertexList.h"
#include <stream.h>

#ifdef LINUX
#	include <platform.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////://////////////////

CPuppet::CPuppet()
{
	
	m_nSampleFrequency = GetAISystem()->m_cvSampleFrequency->GetIVal();
	m_bTargetDodge = false;
	m_vLastTargetVector = Vec3d(0,0,0);
	m_fSuppressFiring = 0;
	m_bOnceWithinAttackRange = false;
	m_fMotionAddition = 0.f;
	m_bAccurateDirectionFire = false;
	m_bLastHideResult = false;
	m_vLastHidePoint = Vec3d(0,0,0);
	m_pDEBUGLastHideSpot = 0;
	m_fAIMTime = 0;
	m_bHaveLiveTarget = false;
	m_bInFire = false;
	m_bMeasureAll = false;
	m_pCurrentGoalPipe = 0;
	m_pAttentionTarget = 0;
	m_pPrevAttentionTarget = 0;
	m_bBlocked = false;
	m_bAllowedToFire = false;
	m_bRunning = false;
	m_bRightStrafe = false;
	m_bLeftStrafe = false;
	m_nBodyPos = 0;
	m_bEnabled = true;
	m_bVisible = false;
	m_fUrgency = 10.f;
	m_bUpdateInternal = true;
	m_pProxy = 0;
	m_pLastOpResult = 0;
	m_fRespawnTime = 2; // seconds
	m_vDEBUG_VECTOR(0,0,0);
	m_bDEBUG_Unstuck=false;
	m_fAccuracySupressor =0.f;
	m_bLooseAttention = false;
	m_pLooseAttentionTarget = 0;
	m_pFormation = 0;
	m_pMyObstacle = 0;
	m_vActiveGoals.reserve(20);
	m_bSmartFire = true;
	m_fLastUpdateTime = 0;
	
}

CPuppet::~CPuppet()
{

	if (!m_mapWanders.empty())
	{
		GoalMap::iterator gi;
		for (gi=m_mapWanders.begin();gi!=m_mapWanders.end();gi++)
		{
			CGoalPipe *pPipe = gi->second;
			delete pPipe;
		}
		m_mapWanders.clear();
	}

	if (!m_mapIdles.empty())
	{
		GoalMap::iterator gi;
		for (gi=m_mapIdles.begin();gi!=m_mapIdles.end();gi++)
		{
			CGoalPipe *pPipe = gi->second;
			delete pPipe;
		}
		m_mapIdles.clear();
	}

	if (!m_mapAttacks.empty())
	{
		GoalMap::iterator gi;
		for (gi=m_mapAttacks.begin();gi!=m_mapAttacks.end();gi++)
		{
			CGoalPipe *pPipe = gi->second;
			delete pPipe;
		}
		m_mapAttacks.clear();
	}

	if (!m_mapRetreats.empty())
	{
		GoalMap::iterator gi;
		for (gi=m_mapRetreats.begin();gi!=m_mapRetreats.end();gi++)
		{
			CGoalPipe *pPipe = gi->second;
			delete pPipe;
		}
		m_mapRetreats.clear();
	}

	if (m_pProxy)
	{
		m_pProxy->Release();
		m_pProxy = 0;
	}

	if (m_pCurrentGoalPipe)
		ResetCurrentPipe();

}


void CPuppet::ParseParameters(const AIObjectParameters &params)
{
	 // do custom parse on the parameters
	m_Parameters  = params.m_sParamStruct;
	m_fEyeHeight = params.fEyeHeight;
	m_State.fHealth = m_Parameters.m_fMaxHealth;
	m_fDEBUG_MaxHealth = m_State.fHealth;

	
	if(m_Parameters.m_fHorizontalFov <0 || m_Parameters.m_fHorizontalFov > 180 )	// see all around
		m_fHorizontalFOVrad = -1.0f;
	else
		m_fHorizontalFOVrad = (float) (cos((m_Parameters.m_fHorizontalFov/2.f) * (3.14/180)));

	IPuppetProxy *pProxy;
	if (params.pProxy->QueryProxy(AIPROXY_PUPPET, (void **) &pProxy))
		m_pProxy = pProxy;
	else
		m_pProxy = NULL;

	//m_Parameters.m_fSpeciesHostility = 2.f;
	//m_Parameters.m_fGroupHostility = 0.f;

	m_Parameters.m_fPersistence = (1.f-m_Parameters.m_fPersistence) * 0.1f;
	
	if (m_Parameters.m_fPersistence == 0)
		m_Parameters.m_fPersistence = 0.01f;

	if (m_Parameters.m_nGroup>=0)
		m_pAISystem->AddToGroup(this,m_Parameters.m_nGroup);

	if (m_Parameters.m_nSpecies)
		m_pAISystem->AddToSpecies(this,m_Parameters.m_nSpecies);
}


void CPuppet::Update()
{
	FUNCTION_PROFILER(GetAISystem()->m_pSystem, PROFILE_AI);

	if (!m_bDryUpdate)
	{	
			float fCurrentTime = m_pAISystem->m_pSystem->GetITimer()->GetCurrTime();
			if (m_fLastUpdateTime>0)
				m_fTimePassed = fCurrentTime - m_fLastUpdateTime;
			else
				m_fTimePassed = 0;
			m_fLastUpdateTime = fCurrentTime;
			
			m_State.Reset();

			// change state here----------------------------------------



			GetStateFromActiveGoals(m_State);
			
			// affect puppet parameters here----------------------------------------
			if (m_bCanReceiveSignals)
				UpdatePuppetInternalState();				


			// Avoid movable stuff
			CrowdControl();


		//	if (!m_pAtteNntionTarget)
		//		m_State.bReevaluate = true;
			CheckTargetLateralMovement();

	}


	//--------------------------------------------------------
	// Orient towards the attention target always
	Navigate(m_pAttentionTarget);

	//--------------------------------------------------------
	// manipulate the fire flag
	if (m_pAttentionTarget) 
	{
    	m_State.nTargetType = m_pAttentionTarget->GetType();
		if ((m_State.nTargetType == AIOBJECT_PLAYER) && m_pAISystem->m_bCollectingAllowed )
			m_fEngageTime+=m_pAISystem->m_pSystem->GetITimer()->GetFrameTime();
		m_State.bTargetEnabled = m_pAttentionTarget->m_bEnabled;
	}

	//<<FIXME>> QUICK E3 INERTIA FIX - make correct later
	if (m_bMoving)
		m_State.bCloseContact = true;
	else
		m_State.bCloseContact = false;

	FireCommand();
	
	if (m_pAISystem->m_cvUpdateProxy->GetIVal())
	{
		FRAME_PROFILER("AIProxyUpdate",GetAISystem()->m_pSystem,PROFILE_AI);
		m_pProxy->Update(&m_State);
		m_State.bReevaluate = false;	// always give a chance for the reevaluation to reach the proxy
											// before setting it to false
	}

	if (m_pFormation)
		m_pFormation->Update( (CAIObject *) this);

	if (!m_lstBindings.empty())
		UpdateHierarchy();

	if (m_Parameters.m_bAwareOfPlayerTargeting)
		CheckPlayerTargeting();
}


void CPuppet::QuickVisibility()
{
	if (m_mapVisibleAgents.empty())
		return;

	VisibilityMap copy = m_mapVisibleAgents;
	VisibilityMap::iterator vi = copy.begin(),viend = copy.end();
	for (;vi!=viend;++vi)
		AddToVisibleList(vi->first,true);

}

void CPuppet::AddToVisibleList(CAIObject *pAIObject, bool bForce, float fAdditionalMultiplier)
{
	if (m_pAISystem->m_cvIgnorePlayer->GetIVal())
	{
		if (pAIObject->GetType() == AIOBJECT_PLAYER)
			return;
	} 


	VisibilityMap::iterator vi = m_mapVisibleAgents.find(pAIObject);
	MemoryMap::iterator mi = m_mapMemory.find(pAIObject);
	bool bSpecial = false;
	CPuppet *pPuppetTarget=0;
	if (pAIObject->CanBeConvertedTo(AIOBJECT_CPUPPET,(void**)&pPuppetTarget))
		bSpecial = pPuppetTarget->GetParameters().m_bSpecial;

	if ((pAIObject->GetType()==AIOBJECT_PLAYER) || bSpecial)
	if (vi== m_mapVisibleAgents.end() && (mi==m_mapMemory.end()))
	{
		// diminish visibility at far distance
		Vec3d targetpos = pAIObject->GetPos();
		float distance = (targetpos - m_vPosition).GetLength();
		if (distance < (m_Parameters.m_fSightRange))
		{

			float dist_coeff = 1 - (distance / (m_Parameters.m_fSightRange));
			float chance = dist_coeff;

			chance = (chance*chance+0.1f) * m_pAISystem->m_cvSOM_Speed->GetFVal() * fAdditionalMultiplier; 

			float	eyeH = pAIObject->GetEyeHeight();

			if ( distance > (m_Parameters.m_fSightRange/2.f))
			{
				if (!pAIObject->m_bMoving)
//					return;
					chance*=0.5f;
			}
				

				// impair the chance further if target has low profile
				if (eyeH < 1.1f)
					chance*=0.5f;
				else 
				if (eyeH < 0.6f)
					chance*=0.4f;

		//	}
			if (distance<5.f)
				chance+=5.f;

			DevaluedMap::iterator di;
			di = m_mapPotentialTargets.find(pAIObject);
			if (di != m_mapPotentialTargets.end())
			{
				di->second+=chance;
				if (di->second > 10.f)
				{
					ObjectObjectMap::iterator ooi = m_mapInterestingDummies.find(pAIObject);
					if (ooi!=m_mapInterestingDummies.end())
						m_pAISystem->RemoveDummyObject(ooi->second);
					m_mapPotentialTargets.erase(di);
				}
				else 
				{
					if (di->second > 5.f)
					{
						ObjectObjectMap::iterator ooi = m_mapInterestingDummies.find(pAIObject);
						if (ooi==m_mapInterestingDummies.end())
						{
							// create new interest dummy
							CAIObject *pDummy = m_pAISystem->CreateDummyObject();
							pDummy->SetPos(pAIObject->GetPos());
							m_mapInterestingDummies.insert(ObjectObjectMap::iterator::value_type(pAIObject,pDummy));
						}
					}
					return;
				}
			}
			else
			{
				m_mapPotentialTargets.insert(DevaluedMap::iterator::value_type(pAIObject,chance));
				return;
			}
		}
	}



	
	
	if (vi!=m_mapVisibleAgents.end())
	{
		// seen last frame, update time
		if (!bForce)
		{
			// we have clear sight
			//(vi->second).fExposureTime += m_pAISystem->m_pSystem->GetITimer()->GetFrameTime();
			(vi->second).fExposureTime += m_fTimePassed;
			(vi->second).bFrameTag = true;
			m_fAccuracySupressor = 0.f;
		}
		else
		{
			// we are seing trough soft cover
			if ((vi->second).fExpirationTime < 1.f)
			{
				//(vi->second).fExpirationTime+=m_pAISystem->m_pSystem->GetITimer()->GetFrameTime();
				(vi->second).fExpirationTime+=m_fTimePassed;
				(vi->second).bFrameTag = true;
			}
			m_fAccuracySupressor = 1.f;
		}
	}
	else if (!bForce)
	{
		VisionSD newrecord;
		if (mi!=m_mapMemory.end())
		{
			VisionSD data;
			AssessThreat(pAIObject, data);
			newrecord.fExposureTime = (mi->second).fIntensity/(data.fThreatIndex*40);
		//	if (m_pAttentionTarget==(mi->second).pDummyRepresentation)
		//		SetAttentionTarget(0);
			m_pAISystem->RemoveDummyObject((mi->second).pDummyRepresentation); // remove dummy of this object from agent memory
			m_mapMemory.erase(mi); // no need to keep in memory, we see it again...
		}
		else
		{
			VisionSD data;
			AssessThreat(pAIObject, data);
			newrecord.fExposureTime = data.fThreatIndex*10.f;
			//newrecord.fExposureTime = 0;
		}
		newrecord.bFrameTag = true;
		m_mapVisibleAgents.insert(VisibilityMap::iterator::value_type(pAIObject,newrecord));
	}
	
} 

bool CPuppet::CanBeConvertedTo(unsigned short type, void **pConverted)
{
	if (type == AIOBJECT_PIPEUSER)
	{
		*pConverted = (IPipeUser *) this;
		return true;
	}

	if (type == AIOBJECT_CPIPEUSER)
	{
		*pConverted = (CPipeUser *) this;
		return true;
	}

	if (type == AIOBJECT_PUPPET)
	{
		*pConverted = (IPuppet *) this;
		return true;
	}

	if (type == AIOBJECT_CPUPPET)
	{
		*pConverted = (CPuppet *) this;
		return true;
	}

	*pConverted = 0;
	return false;
}


void CPuppet::UpdatePuppetInternalState()
{
	FUNCTION_PROFILER(GetAISystem()->m_pSystem, PROFILE_AI);
	float fMaxThreat=0, fMaxInterest=0;
	bool bSoundThreat = false;
	bool bSoundInterest = false;
	bool bVisualThreat = false;
	CAIObject *pThreat = 0, *pInterest  =0;

	// assess threat for visible objects
	VisibilityMap::iterator vi; 
	for (vi=m_mapVisibleAgents.begin();vi!=m_mapVisibleAgents.end();)
	{
		if (!(vi->second).bFrameTag)
		{
			VisibilityMap::iterator todelete = vi;
			if ((vi->first)->m_bEnabled)
				Remember(vi->first, vi->second);
			vi++;
			m_mapVisibleAgents.erase(todelete);
		}
		else
		{
			CAIPlayer *pPlayer = 0;
			if ((vi->first)->CanBeConvertedTo(AIOBJECT_PLAYER,(void**) &pPlayer))
			{
				if (m_nObjectType==AIOBJECT_PUPPET && m_Parameters.m_bPerceivePlayer)
					pPlayer->RegisterPerception(120.f);
			}
			(vi->second).bFrameTag = false;
			AssessThreat(vi->first, vi->second);
			if  (((vi->second).fThreatIndex > fMaxThreat) && (m_mapDevaluedPoints.find(vi->first) == m_mapDevaluedPoints.end()))
			{
				fMaxThreat = (vi->second).fThreatIndex;
				pThreat = vi->first;
				bVisualThreat = true;
			}
			if (((vi->second).fInterestIndex > fMaxInterest) && (m_mapDevaluedPoints.find(vi->first) == m_mapDevaluedPoints.end()))
			{
				fMaxInterest = (vi->second).fInterestIndex;
				pInterest = vi->first;
			}
			vi++;
		}
	}


	// check threat values for things that you remember
	MemoryMap::iterator mi;
	for (mi=m_mapMemory.begin();mi!=m_mapMemory.end();)
	{
		//(mi->second).fIntensity-=0.01f;
		CAIPlayer *pPlayer = 0;
		if ((mi->first)->CanBeConvertedTo(AIOBJECT_PLAYER,(void**) &pPlayer))
		{
			if (m_nObjectType == AIOBJECT_PUPPET && m_Parameters.m_bPerceivePlayer)
				pPlayer->RegisterPerception(100.f);
		}

		(mi->second).fIntensity-=m_Parameters.m_fPersistence;
		if ( (mi->second).fThreatIndex > 0) 
			//(mi->second).fThreatIndex-=0.05f;	// if you dont see him, you get less and less afraid :)
			(mi->second).fThreatIndex-=m_fTimePassed;	// if you dont see him, you get less and less afraid :)


		if ((mi->second).fIntensity < 0)
		{
			MemoryMap::iterator todelete = mi;
			m_pAISystem->RemoveDummyObject((mi->second).pDummyRepresentation);
			mi++;
			m_mapMemory.erase(todelete);
		}
		else
		{
			if (m_mapDevaluedPoints.find(mi->first)== m_mapDevaluedPoints.end())
			{
				if ((mi->second).fThreatIndex > fMaxThreat && !bVisualThreat)
				{
					fMaxThreat = (mi->second).fThreatIndex;		
					pThreat = (mi->second).pDummyRepresentation;
					m_State.bMemory = true;				
				}
			}
/*			if ((data.fInterestIndex > fMaxInterest) && (m_mapDevaluedPoints.find(mi->first) == m_mapDevaluedPoints.end()))
			{
				fMaxInterest = data.fInterestIndex;
				pInterest = mi->first;
			}
*/
			mi++;
		}
	}


	// check threat value for things you have heard since last update
	AudibilityMap::iterator ai;
	for (ai=m_mapSoundEvents.begin();ai!=m_mapSoundEvents.end();)
	{
		//(ai->second).fTimeout-=0.005f;
		if ((ai->second).fThreatIndex > 0) 
			//(ai->second).fThreatIndex -= 0.004f;
			(ai->second).fThreatIndex -= m_fTimePassed;


		if ((ai->second).fInterestIndex > 0)
			(ai->second).fTimeout-=m_Parameters.m_fPersistence;
		else
			(ai->second).fTimeout-=0.001f;


		if ( (ai->second).fTimeout < 0 )
		{
			// lets remove this sound event
			AudibilityMap::iterator todelete = ai;
			m_pAISystem->RemoveDummyObject((ai->second).pDummyRepresentation);
			ai++;
			m_mapSoundEvents.erase(todelete);
		}
		else
		{
			if (m_mapDevaluedPoints.find((ai->second).pOwner)==m_mapDevaluedPoints.end())
			{
				if ( (ai->second).fThreatIndex > fMaxThreat && !bVisualThreat)
				{
					fMaxThreat = (ai->second).fThreatIndex;
					pThreat = (ai->second).pDummyRepresentation;
					bSoundThreat = true;
				}

				if ( (ai->second).fInterestIndex > fMaxInterest )
				{
					fMaxInterest = (ai->second).fInterestIndex;
					pInterest = (ai->second).pDummyRepresentation;
					bSoundInterest = true;
				}
			}

			ai++;
		}
	}

	ObjectObjectMap::iterator ooi;
	if (!m_mapInterestingDummies.empty() && (fMaxInterest < 0.01f) )
	{
		for(ooi=m_mapInterestingDummies.begin();ooi!=m_mapInterestingDummies.end();ooi++)
		{
			CAIObject *pDummy = ooi->second;
			float dist = (m_vPosition-pDummy->GetPos()).GetLength();
			fMaxInterest = 0.5f * (1 - (dist/m_Parameters.m_fSightRange));
			pInterest = pDummy;
		}
		if (fMaxInterest<0.01f)
			SetAttentionTarget(0);
	}

	
	//-----------------------------------------------------------------------

	if (m_bUpdateInternal)
	{

		if (m_State.bMemory)
		{
			if (fMaxThreat > 0)
			{
				if ((pThreat!= m_pAttentionTarget) && (pThreat->m_bEnabled))
				{
					m_bHaveLiveTarget = false;
					SetAttentionTarget(pThreat);
					m_State.bReevaluate = true;
					m_State.bSound = bSoundThreat;
					m_State.fThreat = fMaxThreat;
					m_State.fInterest = 0;
				}

			}
			else 
				if ((pInterest != m_pAttentionTarget) && (fMaxInterest > 0.f))
			{
				// go into investigate
					if (pInterest->m_bEnabled)
					{
						SetAttentionTarget(pInterest);
						m_State.bReevaluate = true;
						m_State.bSound = bSoundInterest;
						m_State.fThreat = 0;
						m_State.fInterest = fMaxInterest;
					}
			}
				
		}
		else
		{
			// there is nothing from memory
			if (fMaxThreat > 0)
			{
				if ((pThreat!= m_pAttentionTarget) && (pThreat->m_bEnabled))
				{
					// go into attack
					m_bHaveLiveTarget = !bSoundThreat;
					SetAttentionTarget(pThreat);
					//m_pAISystem->PropagateThreatToTeam(pThreat,this);
					m_State.bReevaluate = true;
					m_State.bSound = bSoundThreat;
					m_State.fThreat = fMaxThreat;
					m_State.fInterest = 0;
				}

			}
			else 
				if ((pInterest != m_pAttentionTarget) && (fMaxInterest > 0.f))
			{
				// go into investigate
					if (pInterest->m_bEnabled)
					{
						SetAttentionTarget(pInterest);
						m_State.bReevaluate = true;
						m_State.bSound = bSoundInterest;
						m_State.fThreat = 0;
						m_State.fInterest = fMaxInterest;
						//ResetCurrentPipe();
					}
			}
		}

	}


	// update devaluated points
	DevaluedMap::iterator di;
	for (di=m_mapDevaluedPoints.begin();di!=m_mapDevaluedPoints.end();)
	{
		(di->second) -= 0.05f;
		if ( (di->second) < 0 )
		{
			DevaluedMap::iterator todelete;
			todelete = di;
			di++;
			m_mapDevaluedPoints.erase(todelete);
		}
		else
			di++;
	}


	// now update possible targets	
	for (di=m_mapPotentialTargets.begin();di!=m_mapPotentialTargets.end();)
	{
	
		(di->second) -=0.1f;	
	
		if (m_nObjectType == AIOBJECT_PUPPET)
		{
			CAIPlayer *pPlayer;
			if (m_Parameters.m_bPerceivePlayer)
			{
				if ( (di->first)->CanBeConvertedTo(AIOBJECT_PLAYER,(void**)&pPlayer) )
					pPlayer->RegisterPerception(di->second);
			}
		}



		if ( (di->second) < 0)
		{
			
			ObjectObjectMap::iterator ooi = m_mapInterestingDummies.find(di->first);
			if ( ooi != m_mapInterestingDummies.end())
				m_pAISystem->RemoveDummyObject(ooi->second);

			
			DevaluedMap::iterator todelete;
			todelete = di;
			di++;
			m_mapPotentialTargets.erase(todelete);
		}
		else
			di++;
	}


//	if (m_bUpdateInternal)
//	{
		// prepare report for the proxy -------------------------------------------
		if (m_pAttentionTarget && m_pAttentionTarget->m_bEnabled)
		{
			m_State.bHaveTarget = true;
		}
		else
		{
			m_State.bHaveTarget = false;
		}
///	}

	
//	m_State.bCloseContact = m_bCloseContact;	
	
}

void CPuppet::Event(unsigned short eType, SAIEVENT *pEvent)
{
	switch (eType)
	{
		case AIEVENT_DROPBEACON:
			CreateFormation("beacon");
		break;
		case AIEVENT_MOVEMENT_CONTROL:
			if (pEvent->nDeltaHealth==1)
				m_bMovementSupressed = false;
			else
				m_bMovementSupressed = true;
		break;
		case AIEVENT_ONBODYSENSOR:
			if (pEvent->fInterest > m_fSuppressFiring)
				m_fSuppressFiring = pEvent->fInterest;
		break;
		case AIEVENT_CLEAR:
			{
				m_vActiveGoals.clear();
				GetAISystem()->FreeFormationPoint(m_Parameters.m_nGroup,this);
				SetAttentionTarget(0);
				m_bBlocked = false;
				m_bUpdateInternal = true;
				m_bCanReceiveSignals = true;
			}
		break;
		case AIEVENT_CLOAK:
			m_bCloaked = true;
		break;
		case AIEVENT_UNCLOAK:
			m_bCloaked = false;
		break;
		case AIEVENT_REJECT:
			RestoreAttentionTarget( );
		break;
		case AIEVENT_DISABLE:
			m_bEnabled = false;
		break;
		case AIEVENT_ENABLE:
			m_bEnabled = true;
		break;
		case AIEVENT_SLEEP:
			m_bEnabled = false;
			m_bSleeping = true;	
		break;
		case AIEVENT_WAKEUP:
			if (m_bSleeping)
			{
				m_bEnabled = true;
				m_bSleeping = false;
			}
		break;
		case AIEVENT_ONVISUALSTIMULUS:
				HandleVisualStimulus(pEvent);
			break;
		case AIEVENT_ONPATHDECISION:
				HandlePathDecision(pEvent);
			break;
		case AIEVENT_ONSOUNDEVENT:
				HandleSoundEvent(pEvent);
			break;
		case AIEVENT_AGENTDIED:
			m_bEnabled = false;
			m_bSleeping = false;
			m_pAISystem->RemoveFromGroup(m_Parameters.m_nGroup, this);
			ResetCurrentPipe();
			if(m_pProxy)
				m_pProxy->Reset();
			m_pAISystem->ReleaseFormationPoint(this);
			m_pAISystem->CancelAnyPathsFor(this);
			m_pReservedNavPoint = 0;
			if (m_pFormation)
				m_pAISystem->ReleaseFormation(m_Parameters.m_nGroup);
			m_pAISystem->RemoveObjectFromAllOfType(AIOBJECT_PUPPET,this);
			m_pAISystem->RemoveObjectFromAllOfType(AIOBJECT_VEHICLE,this);
			m_pAISystem->RemoveObjectFromAllOfType(AIOBJECT_ATTRIBUTE,this);

			if (pEvent->nDeltaHealth && m_Parameters.m_bPerceivePlayer)
				m_pAISystem->GetAutoBalanceInterface()->RegisterEnemyLifetime(m_fEngageTime);
			break;
	}
		
}


void CPuppet::HandleVisualStimulus(SAIEVENT *pEvent)
{
	if (m_bCanReceiveSignals)
		AddToVisibleList((CAIObject *) pEvent->pSeen, pEvent->bFuzzySight,pEvent->fThreat);
}



bool CPuppet::PointVisible(const Vec3d &pos)
{
	
	Vec3d direction = pos - m_vPosition;

	// lets see if it is outside of its vision range
	if (direction.x > m_Parameters.m_fSightRange)
		return false;

	if (direction.y > m_Parameters.m_fSightRange)
		return false;
	
	if (direction.GetLength() > m_Parameters.m_fSightRange)
		return false; 

	Vec3d myorievector = m_vOrientation;


	myorievector=ConvertToRadAngles(myorievector);

	if(GetType() == AIOBJECT_VEHICLE)	// vehicle models are fliped on z axis
		myorievector = -myorievector;

	direction.Normalize();

	if(m_fHorizontalFOVrad<0.0f)	// hor fov is not restricted
		return true;

	float fdot = direction.Dot(myorievector);
	
	if ( fdot <  0 )
		return false; // its behind him 

	if (fdot < m_fHorizontalFOVrad)
		return false;	// its outside of his FOV
	
	return true;
}

void CPuppet::Navigate(CAIObject *pTarget)
{
	FUNCTION_PROFILER(GetAISystem()->m_pSystem, PROFILE_AI);
	if (m_bLooseAttention)
	{
		if (m_pLooseAttentionTarget)
			pTarget = m_pLooseAttentionTarget;
		else
			return;
	}

 	Vec3d vDir, vAngles, vTargetPos;
	float fTime = m_pAISystem->m_pSystem->GetITimer()->GetFrameTime();
	int TargetType = AIOBJECT_NONE;

	if (!m_bDirectionalNavigation && pTarget)
	{
		Matrix44 mat;
		mat.SetIdentity();
		Vec3d fwd(0.f,-1.f,0.f);
		// follow this attention target
		vAngles = GetAngles();
//		Ang3 puppetAngles;
//		puppetAngles=vAngles;
		Ang3 puppetAngles=vAngles;

		vTargetPos = pTarget->GetPos();
		m_State.vTargetPos = vTargetPos;
		vDir = vTargetPos - m_vPosition;
		float distance = vDir.GetLength();
		if (m_pAttentionTarget)
			m_State.nTargetType = m_pAttentionTarget->GetType();
		m_State.fDistanceFromTarget = distance;

		
	
		vDir.Normalize();
		mat=Matrix44::CreateRotationZYX(-vAngles*gf_DEGTORAD )*mat; //NOTE: anges in radians and negated 
		vAngles = GetTransposed44(mat)*(fwd) + mat.GetTranslationOLD();

		
		TargetType = pTarget->GetType();
		float zcross =  vDir.x * vAngles.y - vDir.y * vAngles.x;
		m_State.fValue =  (-zcross) * m_Parameters.m_fResponsiveness * fTime;
		if (zcross > 0.f)
			m_State.turnright = true;
		else 
			m_State.turnleft = true;

		if (fabs(zcross)<0.2f)
			m_bAccurateDirectionFire = true;
		else
			m_bAccurateDirectionFire = false;

		assert (-1e+9 < m_State.fValue && m_State.fValue < 1e+9);

		
		if ( (TargetType == AIOBJECT_PLAYER) || (TargetType == AIOBJECT_PUPPET) || m_bLooseAttention ) 
		//if ((TargetType != AIOBJECT_DUMMY) && (TargetType != AIOBJECT_HIDEPOINT) && (TargetType != AIOBJECT_WAYPOINT))
		{
			Vec3d vertCorrection = vDir;
		//	mat.Identity();
			//mat.RotateMatrix(vertCorrection);
			//vertCorrection = mat.TransformPoint(fwd);
			Ang3	ang;
			ang=ConvertVectorToCameraAnglesSnap180( vDir );
			vertCorrection=ConvertVectorToCameraAngles(vertCorrection);

			if (vertCorrection.x > 90.f)
				vertCorrection.x = 90.f;
			if (vertCorrection.x < -90.f)
				vertCorrection.x = 360+vertCorrection.x;
	
//			m_State.fValueAux = -(puppetAngles.x - vertCorrection.x)*0.1f;

		//float DifferenceX( const Ang3d_tpl& ang2 ) {return Snap180( x- ang2.x );}
		//	m_State.fValueAux = ang.DifferenceX( puppetAngles )*0.1f;
			m_State.fValueAux = Snap_s180(ang.x-puppetAngles.x )* fTime*5.f;//0.1f;
			assert(-1e+9 < m_State.fValueAux && m_State.fValueAux < 1e+9);
		}

	}
	else if (m_bDirectionalNavigation)
	{
		Matrix44 mat;
		mat.SetIdentity();
		Vec3d fwd(0.f,-1.f,0.f);

		vDir = m_State.vMoveDir;
		vAngles = GetAngles();

		mat=Matrix44::CreateRotationZYX(-vAngles*gf_DEGTORAD )*mat; //NOTE: anges in radians and negated 
		vAngles = GetTransposed44(mat)*(fwd) + mat.GetTranslationOLD();
		
		float zcross =  vDir.x * vAngles.y - vDir.y * vAngles.x;
		m_State.fValue =  (-zcross) * m_Parameters.m_fResponsiveness * fTime;
		if (zcross > 0.f)
			m_State.turnright = true;
		else 
			m_State.turnleft = true;

	}

	if (!pTarget || ((TargetType != AIOBJECT_PLAYER) && (TargetType != AIOBJECT_PUPPET) && !m_bLooseAttention))
	{
		Ang3 puppetAngles=GetAngles();
		if (m_State.vMoveDir.len2()>0.001f)
			vDir = m_State.vMoveDir;
		else
			vDir.Set(0,-1,0);
		Vec3d vertCorrection = vDir;
		//	mat.Identity();
		//mat.RotateMatrix(vertCorrection);
		//vertCorrection = mat.TransformPoint(fwd);
		Ang3	ang;
		ang=ConvertVectorToCameraAnglesSnap180( vDir );
		vertCorrection=ConvertVectorToCameraAngles(vertCorrection);

		if (vertCorrection.x > 90.f)
			vertCorrection.x = 90.f;
		if (vertCorrection.x < -90.f)
			vertCorrection.x = 360+vertCorrection.x;

		//			m_State.fValueAux = -(puppetAngles.x - vertCorrection.x)*0.1f;

		//float DifferenceX( const Ang3d_tpl& ang2 ) {return Snap180( x- ang2.x );}
		//	m_State.fValueAux = ang.DifferenceX( puppetAngles )*0.1f;
		m_State.fValueAux = Snap_s180(ang.x-puppetAngles.x )*0.1f;

	}
}

void CPuppet::Remember(CAIObject *pObject, VisionSD &data)
{

	if ((pObject == m_pAttentionTarget) && m_bUpdateInternal)
	{
			SetAttentionTarget(0);
		//	ResetCurrentPipe();
	}
	

	MemoryMap::iterator mi;
	mi = m_mapMemory.find(pObject);
	if (mi!= m_mapMemory.end())
	{
		// hmm, i seem to remember this one from before
		// reinforce memory
		(mi->second).fIntensity += data.fExposureTime*(data.fThreatIndex*20.f);
		(mi->second).vLastKnownPosition = pObject->GetPos();
	}
	else
	{
		// create new memory item
		MemoryRecord newrecord;
	//	newrecord.fIntensity = 10.f + data.fExposureTime*(data.fThreatIndex*20.f);//*data.fIndex; bigger threats should be remembered longer
		newrecord.fIntensity = /*data.fExposureTime**/(data.fThreatIndex*20.f);//*data.fIndex; bigger threats should be remembered longer
		if (newrecord.fIntensity > 10)
			newrecord.fIntensity = 10;
		newrecord.fThreatIndex = data.fThreatIndex;
		newrecord.vLastKnownPosition = pObject->GetPos();
		if (pObject->GetEyeHeight())
			newrecord.vLastKnownPosition.z -=pObject->GetEyeHeight();
		newrecord.pDummyRepresentation = m_pAISystem->CreateDummyObject();
		newrecord.pDummyRepresentation->SetPos(newrecord.vLastKnownPosition);
		newrecord.pDummyRepresentation->SetName(pObject->GetName());
		m_mapMemory.insert(MemoryMap::iterator::value_type(pObject,newrecord));
	}
}



void CPuppet::AssessThreat(CAIObject *pObject, VisionSD &data)
{

	// find multiplier first
	float fMultiplier = 1.f;
	MapMultipliers::iterator mi; 
	mi = m_pAISystem->m_mapMultipliers.find(pObject->GetType());
	if (mi!=m_pAISystem->m_mapMultipliers.end())
	{
		fMultiplier = mi->second;
    data.fThreatIndex = fMultiplier; 
		data.fInterestIndex = fMultiplier; 
	}
	else
	{
		data.fThreatIndex = 0; 
		data.fInterestIndex = 0; 
	}


	switch(pObject->GetType())
	{
		case AIOBJECT_PLAYER:
					// special processing for player					
					CAIPlayer *pPlayer;
					if (pObject->CanBeConvertedTo(AIOBJECT_PLAYER,(void**) &pPlayer))
					{
						data.fThreatIndex = CalculateThreat(pPlayer->m_Parameters)*fMultiplier;
						data.fInterestIndex = CalculateInterest(pPlayer->m_Parameters)*fMultiplier;
					}
					
			break;

		case AIOBJECT_PUPPET:			
			{
				CPuppet *pPuppet;
				if (pObject->CanBeConvertedTo(AIOBJECT_CPUPPET,(void**) &pPuppet))
				{
					// now access parameters
					
					data.fInterestIndex = CalculateInterest(pPuppet->m_Parameters);
					data.fThreatIndex = CalculateThreat(pPuppet->m_Parameters);

					// is it bigger or smaller than me
					if (m_fEyeHeight < pPuppet->m_fEyeHeight)
					{
						data.fThreatIndex*=1.5f;
					}

					// if it is shooting, double his threat value
					if (pPuppet->m_State.fire)
						data.fThreatIndex*=2.f;

					// if this species has a special modifier, use that as well
					MapMultipliers::iterator spi=m_pAISystem->m_mapSpeciesThreatMultipliers.find(pPuppet->m_Parameters.m_nSpecies); 
					if (spi!=m_pAISystem->m_mapSpeciesThreatMultipliers.end())
						data.fThreatIndex*=spi->second;
				}
			}
			break;

	}

	// scale the threat or the interest based on distance from puppet
	Vec3d hispos=pObject->GetPos()-m_vPosition;

	float coeff = (2 - (hispos.GetLength() / (m_Parameters.m_fSightRange/2.f)));
	data.fThreatIndex *= coeff;
	data.fInterestIndex *= coeff;
}


void CPuppet::RequestPathTo(const Vec3d &pos)
{
	m_nPathDecision=PATHFINDER_STILLTRACING;
	Vec3d myPos = m_vPosition;
	if (m_nObjectType == AIOBJECT_PUPPET)
		myPos.z-=m_fEyeHeight;
	m_pAISystem->TracePath(myPos,pos,this);
}

void CPuppet::HandlePathDecision(SAIEVENT *pEvent)
{
	if (pEvent->bPathFound)
	{
		m_nPathDecision = PATHFINDER_PATHFOUND;
		// lets get it from the ai system
		m_lstPath.clear();
		m_lstPath.insert(m_lstPath.begin(),m_pAISystem->GetGraph()->m_lstPath.begin(),m_pAISystem->GetGraph()->m_lstPath.end());
		m_lstPath.push_back(pEvent->vPosition);
	}
	else
	{
		m_nPathDecision = PATHFINDER_NOPATH;
	} 
}

void CPuppet::Devalue(CAIObject *pObject, bool bDevaluePuppets)
{
	DevaluedMap::iterator di;
	VisibilityMap::iterator vi;

	float fAmount=20.f;

	if ((vi = m_mapVisibleAgents.find(pObject)) != m_mapVisibleAgents.end())
		fAmount += (vi->second).fExposureTime;

	unsigned short type = pObject->GetType();

	if ((type == AIOBJECT_PUPPET) && !bDevaluePuppets)
		return;

	if (type == AIOBJECT_PLAYER)
		return;


	// if this was a sound target, means we are just deleting it from the audibility map
	if (type == AIOBJECT_DUMMY)
	{
		// probably a sound event. lets delete it from the audibility map
		AudibilityMap::iterator ai;
		for (ai=m_mapSoundEvents.begin();ai!=m_mapSoundEvents.end();ai++)
		{
			if ((ai->second).pDummyRepresentation == pObject)
			{
				m_mapSoundEvents.erase(ai);
				break;
			}
		}
	}

	if (pObject == m_pAttentionTarget)
		SetAttentionTarget(0);

	if ((di = m_mapDevaluedPoints.find(pObject)) == m_mapDevaluedPoints.end())
		m_mapDevaluedPoints.insert(DevaluedMap::iterator::value_type(pObject,fAmount));
	
}

void CPuppet::OnObjectRemoved(CAIObject *pObject)
{
	// remove it from the visible list
	VisibilityMap::iterator vi;

	if ( (vi=m_mapVisibleAgents.find(pObject)) != m_mapVisibleAgents.end() )
	{
		m_mapVisibleAgents.erase(vi);
		if (m_Parameters.m_bSpecial)
		{
			if (m_mapVisibleAgents.empty())
				SetSignal(-20,"RESUME_SPECIAL_BEHAVIOUR");
			else
				SetSignal(-20,"CANNOT_RESUME_SPECIAL_BEHAVIOUR");
		}
	}

	// from memory
	MemoryMap::iterator mi;

	if ( (mi=m_mapMemory.find(pObject)) != m_mapMemory.end() )	
	{
		m_mapMemory.erase(mi);
		if (m_Parameters.m_bSpecial)
		{
			if (m_mapMemory.empty())
				SetSignal(-20,"RESUME_SPECIAL_BEHAVIOUR");
			else
				SetSignal(-20,"CANNOT_RESUME_SPECIAL_BEHAVIOUR");
		}

	}

	// lets remove all sound events that were generated by this object

	if (!m_mapSoundEvents.empty())
	{
		AudibilityMap::iterator ai;
		for (ai=m_mapSoundEvents.begin();ai!=m_mapSoundEvents.end();)
		{
			if ((ai->second).pOwner == pObject)
			{
				AudibilityMap::iterator todelete = ai;
				m_pAISystem->RemoveDummyObject((todelete->second).pDummyRepresentation);
				ai++;
				m_mapSoundEvents.erase(todelete);
				if (m_Parameters.m_bSpecial)
				{
					if (m_mapSoundEvents.empty())
						SetSignal(-20,"RESUME_SPECIAL_BEHAVIOUR");
					else
						SetSignal(-20,"CANNOT_RESUME_SPECIAL_BEHAVIOUR");
				}
			}
			else
				ai++;
		}
	}
	


	DevaluedMap::iterator di;

	if ( (di=m_mapDevaluedPoints.find(pObject)) != m_mapDevaluedPoints.end() )
		m_mapDevaluedPoints.erase(di);

	// from potential targets
	if ( (di=m_mapPotentialTargets.find(pObject)) != m_mapPotentialTargets.end() )
		m_mapPotentialTargets.erase(di);

	if (pObject == m_pAttentionTarget)
	{
		SetAttentionTarget(0);
		//ResetCurrentPipe();
	}

	if (pObject == m_pPrevAttentionTarget)
	{
		m_pPrevAttentionTarget = 0;
		//ResetCurrentPipe();
	}

	RemoveFromGoalPipe(pObject);


	if (pObject == m_pLastOpResult && pObject->GetType()!=AIOBJECT_HIDEPOINT)
		SetLastOpResult(0);
	else if (pObject->GetType()==AIOBJECT_HIDEPOINT)
		m_pLastOpResult = 0;

	// if its a dummy, remove it from the interesting dummies
	ObjectObjectMap::iterator ooi;
	if (pObject->GetType()==AIOBJECT_DUMMY)
	{
		if (!m_mapInterestingDummies.empty())
		{
			for (ooi=m_mapInterestingDummies.begin();ooi!=m_mapInterestingDummies.end();)
			{
				if ((ooi->second) == pObject)
				{
					m_mapInterestingDummies.erase(ooi);
					break;
				}
				ooi++;
			}
		}
	}
	else
	{
		// maybe the real rep of the interesting dummies got removed??
		ooi=m_mapInterestingDummies.find(pObject);
		if (ooi!=m_mapInterestingDummies.end())
			m_mapInterestingDummies.erase(ooi);
	}

	if (pObject == m_pReservedNavPoint)
		m_pReservedNavPoint = 0;

	if (!m_State.vSignals.empty())
	{
		std::vector<AISIGNAL>::iterator si=m_State.vSignals.begin();
		for (;si!=m_State.vSignals.end();)
		{
			if ((*si).pSender && (*si).pSender == pObject->GetAssociation())
				si=m_State.vSignals.erase(si);
			else
				++si;
		}
	}
//	if(  )

}

void CPuppet::Forget(CAIObject *pDummyObject)
{
	
	if (m_mapMemory.empty())
		return;

	MemoryMap::iterator mi;
	for (mi=m_mapMemory.begin();mi!=m_mapMemory.end();mi++)
	{
		MemoryRecord mr = mi->second;
		if (mr.pDummyRepresentation == pDummyObject)
		{
				m_mapMemory.erase(mi);
				m_State.bMemory = false; 
				break;
		}
	}
	
}


void CPuppet::HandleSoundEvent(SAIEVENT *pEvent)
{
	// just push a dummy for every sound event now
	AudibilityMap::iterator si;

	if (m_pAttentionTarget)
	{
		int TargetType = m_pAttentionTarget->GetType();
		// PETAR: This is never needed...
		if ( /*TargetType == AIOBJECT_PLAYER ||*/ TargetType == AIOBJECT_VEHICLE )
			return;

//		if (!m_mapMemory.empty())
//			return;
	}

	// try to find whether this sound was heard before
	if ( (si = m_mapSoundEvents.find(pEvent->nDeltaHealth)) != m_mapSoundEvents.end() )
	{
		// we hear it again... Make it more interesting and update its position
		SoundSD &data = (si->second);
		data.pDummyRepresentation->SetPos(pEvent->vPosition);
		data.vPosition = pEvent->vPosition;
		data.fInterestIndex = pEvent->fInterest;
		data.fThreatIndex = pEvent->fThreat;
		data.fTimeout +=(pEvent->fThreat) + (pEvent->fInterest * 0.5f);
		if ( data.fTimeout > 40.f) 
			data.fTimeout = 40.f;
	}
	else
	{ 
		// create new
			// we only put dummy objects in the SoundEvents map,
		SoundSD data;
		data.fThreatIndex = pEvent->fThreat;
		data.fInterestIndex = pEvent->fInterest;
		data.vPosition = pEvent->vPosition;
		data.pOwner = (CAIObject*)pEvent->pSeen;
		data.fTimeout = (pEvent->fThreat)* 10.f + (pEvent->fInterest * 0.5f) ;
		if (data.pOwner->GetType()==AIOBJECT_PLAYER)
			data.fThreatIndex*=2.f;
		if (data.fTimeout < 5.f) 
			data.fTimeout = 5.f;
		if (data.fTimeout > 40.f)
			data.fTimeout = 40.f;

		data.pDummyRepresentation = m_pAISystem->CreateDummyObject();
		data.pDummyRepresentation->SetPos(data.vPosition);
		data.pDummyRepresentation->SetName("SOUND DUMMY");
		m_mapSoundEvents.insert(AudibilityMap::iterator::value_type(pEvent->nDeltaHealth, data));
	}

	
}

bool CPuppet::PointAudible(const Vec3d &pos, float fRadius)
{
	if (m_Parameters.m_fSoundRange < 0.00001)
		return false;

	float center_dist = (pos - m_vPosition).GetLength();

	if (m_pAISystem->m_cvPercentSound->GetIVal())
	{
		fRadius *= m_Parameters.m_fSoundRange/100;

		if (center_dist < fRadius)
			return true;

		return false;

	}
	else
	{
		// calculate approx intesection area of two circles
		float touch_dist = fRadius + m_Parameters.m_fSoundRange;

		if (center_dist > touch_dist)
			return false;	// no intersection

		float affectedRadius = m_Parameters.m_fSoundRange;
		// check for aplicable supressors
		// go trough all the sound supressors and reduse radius if in effective radius of sepressor 
		// [PETAR] Try to find nearest supressor that includes this puppet in its radius
		m_pAISystem->SupressSoundEvent(m_vPosition,affectedRadius);
		
/*		CAIObject *pSupressor = (CAIObject*)m_pAISystem->GetNearestObjectOfType(m_vPosition,AIOBJECT_SNDSUPRESSOR);
		//AIObjects::iterator spr;
		//if ( m_pAISystem->GetFirstObject( AIOBJECT_SNDSUPRESSOR, spr ) )
		if ( pSupressor )
		{
//			while (spr->first == AIOBJECT_SNDSUPRESSOR)
//			{
				pSupressor->Supress(m_vPosition,affectedRadius);
//				(spr->second)->Supress(m_vPosition, affectedRadius);
//				spr++;
//			}
		}
*/
		if ( affectedRadius == 0.0f )
			return false;	// can't hear anythig in supressed area

		if (center_dist > affectedRadius + fRadius)
			return false;	// no intersection

		return true;
	}
}




// calculates threat based on input parameters and this puppet's parameters
float CPuppet::CalculateThreat(const AgentParameters & params)
{
	float fRetValue = 0;

	if (m_Parameters.m_nSpecies != params.m_nSpecies)
		fRetValue+= m_Parameters.m_fSpeciesHostility;

	if (m_Parameters.m_nGroup != params.m_nGroup)
		fRetValue+= m_Parameters.m_fGroupHostility;

	
		
	return fRetValue;
}

// calculates interest value of the target with the given parameters
float CPuppet::CalculateInterest(const AgentParameters & params)
{
	float fRetValue = 0;
	
	if (m_Parameters.m_nSpecies != params.m_nSpecies)
		fRetValue+=0.3f;
//	else
//		fRetValue+=0.1f;

	//if (m_Parameters.m_nGroup != params.m_nGroup)
	//	fRetValue+=0.1f;
//	else
	//	fRetValue+=0.3f;

	return fRetValue;
}



// Steers the puppet outdoors and makes it avoid the immediate obstacles
void CPuppet::Steer(const Vec3d & vTargetPos, GraphNode * pNode)
{

	IAIObject *obstVehicle=GetAISystem()->GetNearestObjectOfType( GetPos(), AIOBJECT_VEHICLE, 10 );

	if(!obstVehicle)	// nothing to steer away from
		return;

	Vec3d	vObstDir = obstVehicle->GetPos() - GetPos();
	Vec3d	vCurrDir = m_State.vMoveDir;
//	Vec3d	vTargetPos;
//	if(m_pAttentionTarget)
//		vTargetPos = m_pAttentionTarget->GetPos();
//	else
//		vTargetPos = m_State.vTargetPos;
	Vec3d	vObsTargetDir = vTargetPos - obstVehicle->GetPos();
	vCurrDir.z=0;
	vObstDir.z=0;
	vObsTargetDir.z=0;
	float	fObstDist = vObstDir.GetLength();
	float	fObstRadius=7.0f;



	vObstDir.normalize();
	float fdot = vObstDir.Dot(vCurrDir);
	float zcross = vObstDir.x*vCurrDir.y - vObstDir.y*vCurrDir.x;
	if (fdot > 0)
	{
		// only influence movement if puppet on any kind of collision course
		Matrix44 m;
		m.SetIdentity();
		if (zcross > 0) 
			//m.RotateMatrix_fix(Vec3d(0,0,90));
			m=Matrix44::CreateRotationZYX(-Vec3d(0,0,90)*gf_DEGTORAD )*m; //NOTE: anges in radians and negated 
		else
			//m.RotateMatrix_fix(Vec3d(0,0,-90));
			m=Matrix44::CreateRotationZYX(Vec3d(0,0,90)*gf_DEGTORAD )*m; //NOTE: anges in radians and negated 

			//POINT_CHANGED_BY_IVO
			//Vec3d correction = m.TransformPoint(guypos);
		Vec3d correction = GetTransposed44(m)*(vCurrDir) + m.GetTranslationOLD();
			
		//m_vDEBUG_VECTOR = vtx;
		if (fObstDist < fObstRadius)
		{
			vObsTargetDir.normalize();

			float	fObstTargetDot = vObstDir.Dot(vObsTargetDir);

			float amount = 1.f - (fObstDist/fObstRadius);


			if( fObstTargetDot>-.35f )
			{
				m_State.vMoveDir += correction*5.5f*amount;
			}
			else
			{
//				float amount = 1.f - (fObstDist/fObstRadius);
				correction*=amount; // scale correction
//				m_State.vMoveDir +=correction;
			}
			m_State.vMoveDir.normalize();
		}
	}
	return;

}

void CPuppet::CreateFormation(const char * szName)
{
	

 	if (!szName)
	{
		if (m_pFormation)
			m_pAISystem->ReleaseFormation(m_Parameters.m_nGroup);
		m_pFormation = 0;
		return;
	}

	// special handling for beacons :) It will create a formation point where the current target of the
	// puppet is.
	if (!stricmp(szName,"beacon"))
	{
		if (m_pAttentionTarget)
			m_pAISystem->UpdateBeacon(m_Parameters.m_nGroup,m_pAttentionTarget->GetPos(), m_pAttentionTarget);
		else if (m_pLastOpResult)
			m_pAISystem->UpdateBeacon(m_Parameters.m_nGroup,m_pLastOpResult->GetPos(),m_pLastOpResult);

	}
	else
	{
		if (m_pFormation)
			m_pAISystem->ReleaseFormation(m_Parameters.m_nGroup);
		m_pFormation = 0;
		m_pFormation = m_pAISystem->CreateFormation(m_Parameters.m_nGroup,szName);
	}

}

void CPuppet::Reset(void)
{
		SetAttentionTarget(0);
	ResetCurrentPipe();
	m_lstPath.clear();
	SetLastOpResult(0);

	m_mapVisibleAgents.clear();
	m_mapMemory.clear();
	m_mapSoundEvents.clear();
	m_mapDevaluedPoints.clear();
	m_mapPotentialTargets.clear();
	m_pLastNode = 0;

	if (m_pFormation)
		m_pAISystem->ReleaseFormation(m_Parameters.m_nGroup);

	m_pFormation = 0;
}


void CPuppet::ReleaseFormation(void)
{
	delete m_pFormation;
	m_pFormation = 0;
	m_pAISystem->ReleaseFormation(m_Parameters.m_nGroup);
}

Vec3d CPuppet::GetIndoorHidePoint(int nMethod, float fSearchDistance, bool bSameOk)
{
	float maxValue = -1;
	float maxDot = -1;
	CGraph *pGraph = m_pAISystem->GetGraph();

	Vec3d retPoint=m_vPosition;
	

	int rnd=0;
	if (nMethod == HM_RANDOM)
	{
		if (!pGraph->m_lstSelected.empty())
			rnd = int(rand() % pGraph->m_lstSelected.size());
	}

	ListObstacles::iterator li;
	for (li=pGraph->m_lstSelected.begin();li!=pGraph->m_lstSelected.end();li++,rnd--)
	{
		ObstacleData od = (*li);
		Vec3d pos = (*li).vPos;
		Vec3d dir = (*li).vDir;
		float fDot = -1;
		if (m_pAttentionTarget) {
			//fDot = ( (m_pAttentionTarget->GetPos()-pos  ).Normalized()).Dot(dir.Normalized());
			fDot =  GetNormalized((m_pAttentionTarget->GetPos()-pos) ).Dot(GetNormalized(dir));
		}
		

		switch (nMethod)
		{
		case HM_RANDOM:
			{
				if (!rnd)
				{
					retPoint = pos;
					maxValue = 2000;
				}
			}
			break;
		case HM_NEAREST:
			{
				float val = fSearchDistance - (pos-m_vPosition).GetLength();
				if (val > maxValue && !Compromising(od,true) && (fDot > maxDot))
				{
						maxValue = val;
						maxDot = fDot;
						retPoint = pos;
				}
			}
			break;
		case HM_NEAREST_TO_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float val = (pos-m_pAttentionTarget->GetPos()).GetLength();
				//float val = 2000 - (pos-m_pAttentionTarget->GetPos()).GetLength();
				if (one.Dot(two)>0 /*&& two.GetLength()>5.f*/)
				{
					if (val > maxValue && !Compromising(od,true) /*&& (fDot > maxDot)*/)
					{
							maxValue = val;
							maxDot = fDot;
							retPoint = pos;
					}
				}
			}
			break;
		case HM_FARTHEST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float val = ( GetLengthSquared(pos-m_pAttentionTarget->GetPos()) );
				if (one.Dot(two)<0)
				{
					if (val > maxValue && !Compromising(od,true) && (fDot > maxDot))
					{
						maxValue = val;
						maxDot = fDot;
						retPoint = pos;
					}
				}
			}
			break;
		case HM_LEFTMOST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float zcross = one.x*two.y - one.y*two.x;

				zcross = 2000+zcross;
				if (zcross > maxValue && !Compromising(od,true) && (fDot > maxDot))
				{
					maxValue = zcross;
					maxDot = fDot;
					retPoint = pos;
				}
			}
			break;
		case HM_FRONTLEFTMOST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float zcross = one.x*two.y - one.y*two.x;
				one.Normalize();
				two.Normalize();

				float f = one.Dot(two);

				if (f>0.2)
				{
					zcross = 2000+zcross;
					if (zcross > maxValue && !Compromising(od,true) && (fDot > maxDot))
					{
						maxValue = zcross;
						maxDot = fDot;
						retPoint = pos;
					}
				}

			}
			break;
		case HM_RIGHTMOST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float zcross = one.x*two.y - one.y*two.x;

				zcross = 2000-zcross;
				if (zcross > maxValue && !Compromising(od,true) && (fDot > maxDot))
				{
					maxValue = zcross;
					maxDot = fDot;
					retPoint = pos;
				}
			}
			break;
		case HM_FRONTRIGHTMOST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float zcross = one.x*two.y - one.y*two.x;
				one.Normalize();
				two.Normalize();

				float f = one.Dot(two);

				if (f>0.2)
				{
					zcross = 2000-zcross;
					if (zcross > maxValue && !Compromising(od,true) && (fDot > maxDot))
					{
						maxValue = zcross;
						maxDot = fDot;
						retPoint = pos;
					}
				}
			}
			break;
		case HM_NEAREST_TO_LASTOPRESULT:
		case HM_NEAREST_TO_LASTOPRESULT_NOSAME:
			if (m_pLastOpResult)
			{

				Vec3d one = m_pLastOpResult->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float val = 2000 - (pos-m_pLastOpResult->GetPos()).GetLength();
				if (one.Dot(two)>0)
				{
					if (val > maxValue && !Compromising(od,true))
					{
						maxValue = val;
						retPoint = pos;
					}
				}
			}
			break;
		case HM_FARTHEST_FROM_LASTOPRESULT:
			if (m_pLastOpResult)
			{
				float val = ( GetLengthSquared(pos-m_pLastOpResult->GetPos()) );
				if (val > maxValue && !Compromising(od,true))
				{
					maxValue = val;
					retPoint = pos;
				}
			}


			break;
			// add more here as needed
		}

	}



	if (maxValue < 0)
	{
		// no hiding points - not with your specified filter or range
		// generate signal that there is no Hiding place 
		SetSignal(-2,"");
		m_bLastHideResult = false;
		return m_vPosition;
	}
	else if ((nMethod != HM_NEAREST) && (nMethod != HM_RANDOM) && (nMethod != HM_NEAREST_TO_LASTOPRESULT_NOSAME))
	{
		//if (m_vLastHidePoint == retPoint)
		if (IsEquivalent(m_vLastHidePoint,retPoint) && !bSameOk)
		{
			SetSignal(-2,"");
			m_bLastHideResult = false;
			return m_vPosition;
		}
	}

	m_vLastHidePoint = retPoint;
	m_bLastHideResult = true;
	return retPoint;
	
}

// decides whether to fire or not
void CPuppet::FireCommand(void)
{
	
	FUNCTION_PROFILER(GetAISystem()->m_pSystem, PROFILE_AI);
	if (m_bAllowedToFire)
		m_State.aimLook = true;
	else
		m_State.aimLook = false;


	m_fCos = m_Parameters.m_fAccuracy + m_fAccuracySupressor + m_fMotionAddition;

	float frameTime = GetAISystem()->m_pSystem->GetITimer()->GetFrameTime();
	if (GetAISystem()->m_cvLateralMovementAccuracyDecay->GetIVal() && m_pAttentionTarget && m_bTargetDodge)
	{
			if (m_fMotionAddition<m_Parameters.m_fAccuracy)
				m_fMotionAddition+=frameTime*0.1f;
	}
	else
	{
		if (m_fMotionAddition>0)
	//		m_fMotionAddition-=frameTime*0.1f;
		m_fMotionAddition = 0;
	}






	if (m_fSuppressFiring>0.001f)
	{
		m_fSuppressFiring-=GetAISystem()->m_pSystem->GetITimer()->GetFrameTime();
		return;
	}


	if (!m_bAccurateDirectionFire)
		return;

	Vec3d vAngles, vTargetPos;

	if (m_pAttentionTarget)
	{


		if (m_pAttentionTarget->GetType()>AIOBJECT_PLAYER)
			return;

		if (m_bSmartFire)
		{
			int nType = m_pAttentionTarget->GetType();
			if ( (nType==AIOBJECT_DUMMY) || 
				(nType==AIOBJECT_HIDEPOINT) || 
				(nType==AIOBJECT_WAYPOINT) )
			{
				if (GetAISystem()->m_cvTargetMovingAccuracyMultiplier->GetIVal())
				{
					if (m_fMotionAddition<0)
						m_fMotionAddition = 0;
						//m_fMotionAddition+=frameTime*0.1f;
				}
				return;
			}
		}



			if (GetAISystem()->m_cvTargetMovingAccuracyMultiplier->GetIVal() && m_pAttentionTarget && !m_bTargetDodge)
			{
					// increase accurracy
					if (m_Parameters.m_fAccuracy + m_fMotionAddition>0)
						m_fMotionAddition-=frameTime*0.05f;
			}
			else
			{
				if (m_fMotionAddition<0)
					m_fMotionAddition = 0;
				//	m_fMotionAddition+=frameTime*0.1f;
			}



		m_State.vFireDir	=	GetNormalized(m_pAttentionTarget->GetPos() - m_vPosition);


		// follow this attention target
		vAngles = GetAngles();
		vTargetPos = m_pAttentionTarget->GetPos();
		vTargetPos.z -= m_pAttentionTarget->GetEyeHeight()*0.2f;
		m_State.vFireDir = vTargetPos - m_vPosition;
		float distance = (vTargetPos - m_vPosition).GetLength();
		float fAccuracy = m_Parameters.m_fAccuracy + m_fAccuracySupressor + m_fMotionAddition;


		//<<FIXME>> implement multipliers eiter as puppet properties or as
		// global variables
//		if (m_pAttentionTarget->m_bMoving)
//			fAccuracy += m_pAISystem->m_cvTargetMovingAccuracyMultiplier->GetFVal();

	//	if (m_bMoving)
	//		fAccuracy += m_pAISystem->m_cvRunAccuracyMultiplier->GetFVal();
		//-------------------------------------------------------------------

		if (fAccuracy>1.f) 
			fAccuracy=1.f;

		// check if within fire range
		if (distance < m_Parameters.m_fAttackRange)
		{
			if (m_bAllowedToFire)
			{

				float probability = ((float) (rand() % 50) + 50.f) / 100.f;
				float ratio = distance / m_Parameters.m_fAttackRange;		

				  
				if (m_Parameters.m_fAggression>=0.f)
				{
 					if (!m_bInFire)
					{
						if (m_fAIMTime < ratio)
						{
							m_fAIMTime+=m_fTimePassed*0.1f;
							return;
						}
						else
						{
							m_bInFire = true;
							m_fAIMTime = (1.f-ratio)*probability*(m_Parameters.m_fAggression);
						}
					}
					else
					{
						if (m_fAIMTime > 0)
							m_fAIMTime-=m_fTimePassed*0.1f;
							//m_fAIMTime-=0.01f;
						else
						{
							m_bInFire = false;
							m_fAIMTime = -probability*m_Parameters.m_fAggression;
						}
					}
				}
				
				

				if (m_pAttentionTarget->m_bEnabled && m_pAISystem->NoFriendsInWay(this, vTargetPos-m_vPosition))
				{

						m_State.fire = true;
						

						if ( (probability < fAccuracy)  && (m_pAttentionTarget->GetType() == AIOBJECT_PLAYER) && distance>10.f)
						{

							int building;
							IVisArea *pArea;
							
							if (!m_pAISystem->CheckInside(vTargetPos,building,pArea))
							//if (pTargetNode->nBuildingID < 0) 
							{
								GraphNode *pTargetNode = m_pAISystem->GetGraph()->GetEnclosing(vTargetPos,m_pAttentionTarget->m_pLastNode);
								m_pAttentionTarget->m_pLastNode = pTargetNode;
								Vec3d targetang = m_pAttentionTarget->GetAngles();

								Matrix44 m;
								m.SetIdentity();
							//	m.RotateMatrix_fix(targetang);
								m=Matrix44::CreateRotationZYX(-targetang*gf_DEGTORAD )*m; //NOTE: anges in radians and negated 
							//POINT_CHANGED_BY_IVO
							//targetang = m.TransformPoint(Vec3d(0,-1,0));
								targetang = GetTransposed44(m) * Vec3d(0,-1,0);

								// find the obstacle that is infront of your target
								float maxdot = -1.f;
								Vec3d vUpdTargetPos = vTargetPos;
								ObstacleIndexVector::iterator oi;
								for (oi=pTargetNode->vertex.begin();oi!=pTargetNode->vertex.end();oi++)
								{
										Vec3d obstacle_pos = m_pAISystem->m_VertexList.GetVertex((*oi)).vPos;
										obstacle_pos= obstacle_pos - vTargetPos;
										float fdot = targetang.Dot(GetNormalized(obstacle_pos));

										if (obstacle_pos.GetLength() > 6.f)
											fdot = -1.f;
										if (fdot>maxdot)
										{
											maxdot = fdot;
											vUpdTargetPos = m_pAISystem->m_VertexList.GetVertex((*oi)).vPos;
										}
								}

								// check whether the direction is ok
								Vec3d accurateshot = GetNormalized(vTargetPos-m_vPosition);
								if (accurateshot.Dot( GetNormalized(vUpdTargetPos-m_vPosition)) > 0.9f)
										vTargetPos = vUpdTargetPos;
							
								if (maxdot < 0.4f)
								{
									targetang.z = 0;
									Vec3d targpos = m_pAttentionTarget->GetPos();
									targpos.z -= m_pAttentionTarget->GetEyeHeight();
									vTargetPos = targpos + targetang*4.f;
									m_vDEBUG_VECTOR = vTargetPos;
								}
							}
						}


						
						vTargetPos.x += ( ((float) (rand() % 200)) / 100.f  - 1.f ) * fAccuracy;
						vTargetPos.y += ( ((float) (rand() % 200)) / 100.f  - 1.f ) * fAccuracy;
						vTargetPos.z += ( ((float) (rand() % 200)) / 100.f  - 1.f ) * fAccuracy;


						m_State.vFireDir = vTargetPos - m_vPosition;
						
				}
			}
		}
		else
		{
			if (m_bAllowedToFire && !m_bOnceWithinAttackRange && m_pAttentionTarget->GetType()!=AIOBJECT_DUMMY)
			{
				m_bOnceWithinAttackRange = true;
				IGoalPipe *pPipe = m_pAISystem->CreateGoalPipe("always_approach_to_attack_range");
				GoalParameters params;
				params.fValue = m_Parameters.m_fAttackRange;
				pPipe->PushGoal(AIOP_APPROACH,true,params);
				InsertSubPipe(0,"always_approach_to_attack_range",0);
			}

		}
	m_State.vFireDir.Normalize();
	}


}

void CPuppet::AddToMemory(CAIObject * pObject)
{
	VisionSD data;
	AssessThreat(pObject,data);
	data.fExposureTime = 10.f;
	Remember(pObject,data);
}

// finds hide point in graph based on specified search method
Vec3d CPuppet::FindHidePoint(float fSearchDistance, int nMethod, bool bIndoors, bool bSameOk)
{
	// first get all hide points within the search distance
	CGraph *pGraph=0;
	if (bIndoors)
		pGraph = m_pAISystem->GetGraph();
	else
		pGraph = m_pAISystem->GetHideGraph();
	if (!pGraph)
	{
			// no hiding points - NO HIDE GRAPH
			// generate signal that there is no Hiding place 
			SetSignal(-2,"");
			m_bLastHideResult = false;
			return m_vPosition;
	}

	if (nMethod < HM_NEAREST_TO_LASTOPRESULT)
		pGraph->SelectNodesInSphere(m_vPosition,fSearchDistance,m_pLastNode);
	else
	{
		if (m_pLastOpResult)
			pGraph->SelectNodesInSphere(m_pLastOpResult->GetPos(),fSearchDistance,m_pLastOpResult->m_pLastNode);
		else
		{
			// no hiding points - NO LAST OP RESULT
			// generate signal that there is no Hiding place 
			SetSignal(-2,"");
			m_bLastHideResult = false;
			return m_vPosition;
		}
	}

	if (pGraph->m_lstSelected.empty())
	{
		// no hiding points - Nothing within this radius
		// generate signal that there is no Hiding place 
		SetSignal(-2,"");
		m_bLastHideResult = false;
		return m_vPosition;
	}

	float maxValue = -1;
	float maxValueNearest = -1;
	
	Vec3d retPoint=m_vPosition;
	Vec3d retPointNearest=m_vPosition;
	void *pPhysicalEntity = 0;

	int rnd=0;
	if (nMethod == HM_RANDOM)
	{
		if (!pGraph->m_lstSelected.empty())
			rnd = int(rand() % pGraph->m_lstSelected.size());
	}

	if (bIndoors)
		retPoint = GetIndoorHidePoint(nMethod,fSearchDistance,bSameOk);
	else
		retPoint = GetOutdoorHidePoint(nMethod,fSearchDistance,bSameOk);

	return retPoint;


}

// Evaluates whether the chosen navigation point will expose us too much to the target
bool CPuppet::Compromising(const ObstacleData &od, bool bIndoor)
{
	Vec3d vTarget = od.vPos;
	
	if (!m_pAttentionTarget) 
		return false;

	Vec3d one,two;
	
	
	if (!GetAISystem()->NoSameHidingPlace(this,vTarget))
		return true;


	if (!bIndoor)
	{
		one = m_pAttentionTarget->GetPos() - m_vPosition;
		two = vTarget - m_vPosition;
		float f = one.Dot(two);

		if (f < 0)
			return false;

		if (GetLengthSquared(two) > GetLengthSquared(one)) 
			return true;
	}
	else
	{
		one = m_pAttentionTarget->GetPos() - od.vPos;
		Vec3d vLookAt = od.vDir;
		one.Normalize();
		float f = one.Dot(vLookAt);

		if (f < 0.5f)
			return true;
	}

	return false;
}



// returns true if puppet visible
bool CPuppet::Sees(CPuppet * pObject)
{
//	if (m_mapVisibleAgents.find(pObject) == m_mapVisibleAgents.end())
	//	return false;

	Vec3d otherpos = pObject->GetPos();
	

	if (PointVisible(otherpos))
	{
		float fdist = (m_vPosition-otherpos).GetLength();
		if (fdist < ( m_Parameters.m_fAttackRange/2 ) )	//<<FIXME> HACK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			return true;
		else
			return false;
	}
	else
		return false;
	
}

void CPuppet::SetParameters(AgentParameters & sParams)
{
	if (sParams.m_nGroup != m_Parameters.m_nGroup)
	{
		m_pAISystem->RemoveFromGroup(m_Parameters.m_nGroup,this);
		if (m_pFormation)
			m_pAISystem->ReleaseFormation(m_Parameters.m_nGroup);

		m_pAISystem->AddToGroup(this,sParams.m_nGroup);
		CAIObject *pBeacon = m_pAISystem->GetBeacon(m_Parameters.m_nGroup);
		if (pBeacon)
			m_pAISystem->UpdateBeacon(sParams.m_nGroup,pBeacon->GetPos());
	}
	//m_bUpdateInternal = !sParams.m_bIgnoreTargets;
	if (sParams.m_bIgnoreTargets)
		m_bCanReceiveSignals = false;
	else
		m_bCanReceiveSignals = true;

	//sParams.m_bIgnoreTargets = false;
	m_Parameters = sParams;

	if(m_Parameters.m_fHorizontalFov <0 || m_Parameters.m_fHorizontalFov > 180 )	// see all around
		m_fHorizontalFOVrad = -1.0f;
	else
		m_fHorizontalFOVrad = (float) (cos((m_Parameters.m_fHorizontalFov/2.f) * (3.14/180)));

	// temp solution
	if (m_Parameters.m_fResponsiveness<60.f)
		m_Parameters.m_fResponsiveness*=400.f/7.5f;

}



Vec3d CPuppet::GetOutdoorHidePoint(int nMethod, float fSearchDistance, bool bSameOk)
{

	float maxValue = -1;
	CGraph *pGraph = m_pAISystem->GetHideGraph();

	Vec3d retPoint=m_vPosition;
	
	int rnd=0;
	if (nMethod == HM_RANDOM)
	{
		if (!pGraph->m_lstSelected.empty())
			rnd = int(rand() % pGraph->m_lstSelected.size());
	}

	ListObstacles::iterator li;
	for (li=pGraph->m_lstSelected.begin();li!=pGraph->m_lstSelected.end();li++,rnd--)
	{
		ObstacleData od = (*li);
		Vec3d pos = (*li).vPos;
		Vec3d dir = (*li).vDir;

		switch (nMethod)
		{
		case HM_RANDOM:
			{
				if (!rnd)
				{
					retPoint = pos;
					maxValue = 2000;
				}
			}
			break;
		case HM_NEAREST:
			{
//				if (m_pAttentionTarget)
//				{
					Vec3d angles = ConvertToRadAngles(m_vOrientation);
					Vec3d att_pos = m_vPosition+10.f*angles;
					Vec3d one = m_vPosition-att_pos;
					Vec3d two = pos-att_pos;

					if (one.Dot(two)>0)
					{
						float val = fSearchDistance - (pos-m_vPosition).GetLength();
						if (val > maxValue && !Compromising(od,false))
						{
							maxValue = val;
							retPoint = pos;
						}
					}
//				}
			}
			break;
		case HM_NEAREST_TO_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d att_pos = m_pAttentionTarget->GetPos();
				//Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				//Vec3d	two = pos-m_vPosition;
				Vec3d one = m_vPosition-att_pos;
				Vec3d two = pos-att_pos;


				float val = 2000 - GetLengthSquared(pos-att_pos);
				if (one.Dot(two)>0)
				{
					if (val > maxValue && !Compromising(od,false))
					{
						maxValue = val;
						retPoint = pos;
					}
				}
			}
			break;
		case HM_FARTHEST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float val = GetLengthSquared( (pos-m_pAttentionTarget->GetPos()) );
				if (one.Dot(two)<0)
				{
					if (val > maxValue && !Compromising(od,false))
					{
						maxValue = val;
						retPoint = pos;
					}
				}
			}
			break;
		case HM_LEFTMOST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float zcross = one.x*two.y - one.y*two.x;

				zcross = 2000+zcross;
				if (zcross > maxValue && !Compromising(od,false))
				{
					maxValue = zcross;
					retPoint = pos;
				}
			}
			break;
		case HM_FRONTLEFTMOST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float zcross = one.x*two.y - one.y*two.x;
				one.Normalize();
				two.Normalize();

				float f = one.Dot(two);

				if (f>0.3f)
				{
					zcross = 2000+zcross;
					if (zcross > maxValue && !Compromising(od,false) && !IsEquivalent(od.vPos,m_vLastHidePoint))
					{
						maxValue = zcross;
						retPoint = pos;
					}
				}

			}
			break;
		case HM_RIGHTMOST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float zcross = one.x*two.y - one.y*two.x;

				zcross = 2000-zcross;
				if (zcross > maxValue && !Compromising(od,false))
				{
					maxValue = zcross;
					retPoint = pos;
				}
			}
			break;
		case HM_FRONTRIGHTMOST_FROM_TARGET:
			if (m_pAttentionTarget)
			{
				Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				Vec3d	two = pos-m_vPosition;

				float zcross = one.x*two.y - one.y*two.x;
				one.Normalize();
				two.Normalize();

				float f = one.Dot(two);

				if (f>0.3f)
				{
					zcross = 2000-zcross;
					if (zcross > maxValue && !Compromising(od,false) && !IsEquivalent(od.vPos,m_vLastHidePoint))
					{
						maxValue = zcross;
						retPoint = pos;
					}
				}
			}
			break;
		case HM_NEAREST_TO_LASTOPRESULT:
		case HM_NEAREST_TO_LASTOPRESULT_NOSAME:
			if (m_pLastOpResult)
			{
				Vec3d att_pos = m_pLastOpResult->GetPos();
				//Vec3d one = m_pAttentionTarget->GetPos()-m_vPosition;
				//Vec3d	two = pos-m_vPosition;
				Vec3d one = m_vPosition-att_pos;
				//Vec3d two = pos-att_pos;

				float val = 2000 - GetLengthSquared(pos-att_pos);
///				if (one.Dot(two)>0)
				{
					if (val > maxValue && !Compromising(od,false))
					{
						maxValue = val;
						retPoint = pos;
					}
				}
			}
			break;
		case HM_FARTHEST_FROM_LASTOPRESULT:
			if (m_pLastOpResult)
			{
				float val = GetLengthSquared((pos-m_pLastOpResult->GetPos()));
				if (val > maxValue && !Compromising(od,false))
				{
					maxValue = val;
					retPoint = pos;
				}
			}


			break;
			// add more here as needed
		}
	}



	if (maxValue < 0)
	{
		// no hiding points - not with your specified filter or range
		// generate signal that there is no Hiding place 
		SetSignal(-2,"");
		m_bLastHideResult = false;
		return m_vPosition;
	}
	else if ((nMethod != HM_NEAREST) && (nMethod != HM_RANDOM) && (nMethod != HM_NEAREST_TO_LASTOPRESULT_NOSAME))
	{
		//if (m_vLastHidePoint == retPoint)
		if (IsEquivalent(m_vLastHidePoint,retPoint) && !bSameOk)
		{
			SetSignal(-2,"");
			m_bLastHideResult = false;
			return m_vPosition;
		}
	}

	m_vLastHidePoint = retPoint;
	m_bLastHideResult = true;
	return retPoint;
}


void CPuppet::CrowdControl(void)
{
	float fPersonalSpaceRadius = m_fEyeHeight;
	Vec3d radius_vector(fPersonalSpaceRadius,fPersonalSpaceRadius,fPersonalSpaceRadius);

	IPhysicalEntity **pSleepingRigids;
	IPhysicalWorld *pWorld = GetAISystem()->GetPhysicalWorld();

	int nr = pWorld->GetEntitiesInBox(m_vPosition-radius_vector, m_vPosition+radius_vector,pSleepingRigids, ent_rigid | ent_sleeping_rigid);
	if (nr)
	{
		for (int i=0;i<nr;i++)
		{
			IPhysicalEntity *pRigid = pSleepingRigids[i];
			pe_status_pos ppos;
			pRigid->GetStatus(&ppos);

			
			ppos.pos.z = m_vPosition.z;
			Vec3d guypos = ppos.pos-m_vPosition;
			float poslength = guypos.GetLength();
			guypos.Normalize();


			float fdot = guypos.Dot(m_State.vMoveDir);
			float zcross = guypos.x*m_State.vMoveDir.y - guypos.y*m_State.vMoveDir.x;
			if (fdot > 0)
			{
				// only influence movement if puppet on any kind of collision course
				Matrix44 m;
				m.SetIdentity();
				if (zcross > 0) 
					m=Matrix44::CreateRotationZYX(-Vec3d(0,0,90)*gf_DEGTORAD )*m; //NOTE: anges in radians and negated 
				else
					m=Matrix44::CreateRotationZYX(-Vec3d(0,0,90)*gf_DEGTORAD )*m; //NOTE: anges in radians and negated 

				Vec3d correction = GetTransposed44(m)*(guypos) + m.GetTranslationOLD();

				if (poslength < m_fEyeHeight)
				{
					float amount = 1.f - (poslength/m_fEyeHeight);
					correction*=amount; // scale correction
					m_State.vMoveDir +=correction;
					m_bDirectionalNavigation = false;
				}
			}
		}
	}

}

void CPuppet::CheckPlayerTargeting(void)
{
	CAIObject *pPlayer = m_pAISystem->GetPlayer();

	if (!pPlayer)
		return;

	VisibilityMap::iterator vi = m_mapVisibleAgents.find(pPlayer);
	if (vi==m_mapVisibleAgents.end())
		return;


	Vec3d lookDir = ConvertToRadAngles(pPlayer->GetAngles());
	Vec3d relPos = GetNormalized(m_vPosition - pPlayer->GetPos());

	float fdot = lookDir.Dot(relPos);
	if (fdot>0.98f)
		SetSignal(1,"OnPlayerAiming");
	else if (fdot<-0.9f)
		SetSignal(1,"OnPlayerLookingAway");

}

void CPuppet::RemoveFromGoalPipe(CAIObject* pObject)
{
	if (m_pCurrentGoalPipe)
	{
		if (m_pCurrentGoalPipe->IsInSubpipe())
		{
			CGoalPipe *pPipe = m_pCurrentGoalPipe;
			do
			{
				if (pPipe->m_pArgument == pObject)
					pPipe->m_pArgument = 0;
			}
			while (pPipe = pPipe->GetSubpipe());
		}
		else
		{
			if (m_pCurrentGoalPipe->m_pArgument == pObject)
				m_pCurrentGoalPipe->m_pArgument = 0;
		}
	}
}

void CPuppet::CheckTargetLateralMovement()
{
	if (!m_pAttentionTarget)
		return;

	if (m_nSampleFrequency--)
		return;

	m_nSampleFrequency=GetAISystem()->m_cvSampleFrequency->GetIVal();
	
	if (m_vLastTargetVector.Length()<0.001)
	{
		m_vLastTargetVector = GetNormalized(m_pAttentionTarget->GetPos()-m_vPosition);
	}

	Vec3d currTargetVector = GetNormalized(m_pAttentionTarget->GetPos()-m_vPosition);
	float fDot = m_vLastTargetVector.Dot(currTargetVector);
	if (fDot<0.92)
		m_bTargetDodge = true;
	else
		m_bTargetDodge = false;

	m_vLastTargetVector = currTargetVector;

}

CAIObject * CPuppet::GetMemoryOwner(CAIObject * pMemoryRepresentation)
{
	if (!m_mapMemory.empty())
	{
		MemoryMap::iterator mi = m_mapMemory.begin(),miend = m_mapMemory.end();
		for (;mi!=miend;++mi)
		{
			if ( (mi->second).pDummyRepresentation == pMemoryRepresentation )
				return mi->first;
		}
	}
	return NULL;
}


void CPuppet::Save(CStream &stm)
{
	// save bodypos
	stm.Write((int)m_State.bodystate);
	stm.Write(m_bEnabled);

	if (!m_bEnabled)
	{
		stm.Write((int)0); // for mem targets
		stm.Write((int)0); // for sound targets
		stm.Write((int)0); // for devalued targets
		stm.Write((int)0); // for executing pipe
		stm.Write((int)0); // for lastop
		if (m_pProxy)
		{
			m_pProxy->Save(stm);
		}
		stm.Write(m_bUpdateInternal);
		stm.Write(m_bCanReceiveSignals);
		return;
	}


	// serialize memory targets
	if (!m_mapMemory.empty())
	{
		stm.Write((int)m_mapMemory.size());
		MemoryMap::iterator mi = m_mapMemory.begin(),miend = m_mapMemory.end();
		for (;mi!=miend;++mi)
		{
			CAIObject *pMemoryOf = mi->first;
			MemoryRecord mr = mi->second;
			stm.Write(pMemoryOf->GetName());
			stm.Write(mr.fIntensity);
			stm.Write(mr.fThreatIndex);
			stm.Write(mr.vLastKnownPosition);
		}
	}
	else
		stm.Write((int)0);


	// serialize sound events
	if (!m_mapSoundEvents.empty())
	{
		stm.Write((int)m_mapSoundEvents.size());
		AudibilityMap::iterator ai=m_mapSoundEvents.begin(),aiend=m_mapSoundEvents.end();
		for (;ai!=aiend;++ai)
		{
			SoundSD data = ai->second;
			stm.Write(data.pOwner->GetName());
			stm.Write(data.fInterestIndex);
			stm.Write(data.fThreatIndex);
			stm.Write(data.fTimeout);
			stm.Write(data.vPosition);
			stm.Write(ai->first);
		}
	}
	else
		stm.Write((int)0);

	// serialize devalued
	if (!m_mapDevaluedPoints.empty())
	{
		stm.Write((int)m_mapDevaluedPoints.size());
		DevaluedMap::iterator di=m_mapDevaluedPoints.begin(),diend=m_mapDevaluedPoints.end();
		for (;di!=diend;++di)
		{
			stm.Write((di->first)->GetName());
			stm.Write((float) di->second);
		}
	}
	else
		stm.Write((int)0);

	// serialize executing pipe
	if (m_pCurrentGoalPipe)
	{
		stm.Write((int)1);
		stm.Write(m_pCurrentGoalPipe->m_sName.c_str());
		stm.Write(m_pCurrentGoalPipe->GetPosition());
		if (m_pCurrentGoalPipe->m_pArgument)
		{
			stm.Write((int)1);
			stm.Write(m_pCurrentGoalPipe->m_pArgument->GetName());
		}
		else
			stm.Write((int)0);
//[kirill]
//		stm.Write(m_bBlocked);
		CGoalPipe *pPipe = m_pCurrentGoalPipe;
		
		while (pPipe->IsInSubpipe())
		{
			pPipe=pPipe->GetSubpipe();
			stm.Write((int)1);
			stm.Write(pPipe->m_sName.c_str());
			stm.Write(pPipe->GetPosition());
			if (pPipe->m_pArgument)
			{
				stm.Write((int)1);
				stm.Write(pPipe->m_pArgument->GetName());
			}
			else
				stm.Write((int)0);
			
		}
		stm.Write((int)0);
	}
	else
		stm.Write((int)0);

	// serialize last op target
	if (m_pLastOpResult)
	{
		stm.Write((int)1);
		stm.Write(m_pLastOpResult->GetName());
	}
	else
		stm.Write((int)0);

	if (m_pProxy)
		m_pProxy->Save(stm);

	stm.Write(m_bUpdateInternal);
	stm.Write(m_bCanReceiveSignals);
}

void CPuppet::Load(CStream &stm)
{
	stm.Read(m_State.bodystate);
	stm.Read(m_bEnabled);

	// load memory targets
	m_mapMemory.clear();
	int nNumMemories=0;
	stm.Read(nNumMemories);
	if (nNumMemories)
	{
		// load memory targets
		int i=0;
		while (i<nNumMemories)
		{
			char szName[255];
			stm.Read(szName,255);
			CAIObject *pObject = (CAIObject*)m_pAISystem->GetAIObjectByName(0,szName);
			
			MemoryRecord mr;
			stm.Read(mr.fIntensity);
			stm.Read(mr.fThreatIndex);
			stm.Read(mr.vLastKnownPosition);

			if (mr.fThreatIndex<0) 
				// it must be at least above 0
				mr.fThreatIndex=0.1f;
			
			if (pObject)
			{
				mr.pDummyRepresentation = m_pAISystem->CreateDummyObject();
				mr.pDummyRepresentation->SetPos(mr.vLastKnownPosition);
				mr.pDummyRepresentation->SetName(pObject->GetName());
				m_mapMemory.insert(MemoryMap::iterator::value_type(pObject,mr));
			}
			i++;
		}
	}

	// load sound events
	int nNumSoundEvents = 0;
	stm.Read(nNumSoundEvents);
	if (nNumSoundEvents)
	{
		int i=0;
		while (i<nNumSoundEvents)
		{
			char szName[255];
			stm.Read(szName,255);
			CAIObject *pObject = (CAIObject*)m_pAISystem->GetAIObjectByName(0,szName);
			
			SoundSD data;
			stm.Read(data.fInterestIndex);
			stm.Read(data.fThreatIndex);
			stm.Read(data.fTimeout);
			stm.Read(data.vPosition);
			int nSoundID;
			stm.Read(nSoundID);

			if (data.fThreatIndex<0)
				data.fThreatIndex = 0.1f;

			if (pObject)
			{
				data.pOwner  = pObject;
				data.pDummyRepresentation = m_pAISystem->CreateDummyObject();
				data.pDummyRepresentation->SetPos(data.vPosition);
				data.pDummyRepresentation->SetName("SOUND DUMMY");
				m_mapSoundEvents.insert(AudibilityMap::iterator::value_type(nSoundID, data));
			}

			i++;
		}
	}

	// load devalued targets
	int nNumDev = 0;
	stm.Read(nNumDev);
	if (nNumDev)
	{
		int i=0;
		while (i<nNumDev)
		{
			char szName[255];
			stm.Read(szName,255);
			CAIObject *pObject = (CAIObject*)m_pAISystem->GetAIObjectByName(0,szName);

			float fValue;
			stm.Read(fValue);

			if (pObject)
			{
				m_mapDevaluedPoints.insert(DevaluedMap::iterator::value_type(pObject, fValue));
			}

			i++;
		}
	}



	int nExecutingPipe=0;
	stm.Read(nExecutingPipe);
	if (nExecutingPipe)
	{
		char sName[255];
		int nPosition;

		stm.Read(sName,255);
		stm.Read(nPosition);
		

		CAIObject *pArgument = 0;
		int nHasArgument;
		stm.Read(nHasArgument);
		if (nHasArgument)
		{
			char szArgName[255];
			stm.Read(szArgName,255);
			pArgument = m_pAISystem->GetAIObjectByName(szArgName);
		}

//[kirill]
//		stm.Read(m_bBlocked);
		// select the same pipe
		SelectPipe(0,sName,pArgument);
		

		int nSubpipes=0;
		stm.Read(nSubpipes);

		if (!nSubpipes && nPosition>0)
			nPosition--;
		
		m_pCurrentGoalPipe->SetPosition(nPosition);

		while (nSubpipes)
		{
			stm.Read(sName,255);
			stm.Read(nPosition);
			stm.Read(nHasArgument);
			pArgument = 0;
			if (nHasArgument)
			{
				char szArgName[255];
				stm.Read(szArgName,255);
				pArgument = m_pAISystem->GetAIObjectByName(szArgName);
			}
		
			InsertSubPipe(0,sName,pArgument);
			CGoalPipe *pBottomMostPipe = m_pCurrentGoalPipe;
			while (pBottomMostPipe->IsInSubpipe())
				pBottomMostPipe=pBottomMostPipe->GetSubpipe();

			pBottomMostPipe->SetPosition(nPosition);
			stm.Read(nSubpipes);
		}
	}

	// last op result
	// serialize last op target
	int nHasLastOpResult;
	stm.Read(nHasLastOpResult);
	if (nHasLastOpResult)
	{
		char sName[255];
		stm.Read(sName,255);
		CAIObject *pLOP = m_pAISystem->GetAIObjectByName(sName);
		SetLastOpResult(pLOP);
	}

	if (m_pProxy)
		m_pProxy->Load(stm);

	stm.Read(m_bUpdateInternal);
	stm.Read(m_bCanReceiveSignals);


	UpdatePuppetInternalState();
}

void CPuppet::Load_PATCH_1(CStream &stm)
{
	// load memory targets
	m_mapMemory.clear();
	int nNumMemories=0;
	stm.Read(nNumMemories);
	if (nNumMemories)
	{
		// load memory targets
		int i=0;
		while (i<nNumMemories)
		{
			char szName[255];
			stm.Read(szName,255);
			CAIObject *pObject = (CAIObject*)m_pAISystem->GetAIObjectByName(0,szName);

			MemoryRecord mr;
			stm.Read(mr.fIntensity);
			stm.Read(mr.fThreatIndex);
			stm.Read(mr.vLastKnownPosition);

			if (mr.fThreatIndex<0) 
				// it must be at least above 0
				mr.fThreatIndex=0.1f;

			if (pObject)
			{
				mr.pDummyRepresentation = m_pAISystem->CreateDummyObject();
				mr.pDummyRepresentation->SetPos(mr.vLastKnownPosition);
				mr.pDummyRepresentation->SetName(pObject->GetName());
				m_mapMemory.insert(MemoryMap::iterator::value_type(pObject,mr));
			}
			i++;
		}
	}

	// load sound events
	int nNumSoundEvents = 0;
	stm.Read(nNumSoundEvents);
	if (nNumSoundEvents)
	{
		int i=0;
		while (i<nNumSoundEvents)
		{
			char szName[255];
			stm.Read(szName,255);
			CAIObject *pObject = (CAIObject*)m_pAISystem->GetAIObjectByName(0,szName);

			SoundSD data;
			stm.Read(data.fInterestIndex);
			stm.Read(data.fThreatIndex);
			stm.Read(data.fTimeout);
			stm.Read(data.vPosition);
			int nSoundID;
			stm.Read(nSoundID);

			if (data.fThreatIndex<0)
				data.fThreatIndex = 0.1f;

			if (pObject)
			{
				data.pOwner  = pObject;
				data.pDummyRepresentation = m_pAISystem->CreateDummyObject();
				data.pDummyRepresentation->SetPos(data.vPosition);
				data.pDummyRepresentation->SetName("SOUND DUMMY");
				m_mapSoundEvents.insert(AudibilityMap::iterator::value_type(nSoundID, data));
			}

			i++;
		}
	}

	int nExecutingPipe=0;
	stm.Read(nExecutingPipe);
	if (nExecutingPipe)
	{
		char sName[255];
		int nPosition;

		stm.Read(sName,255);
		stm.Read(nPosition);


		CAIObject *pArgument = 0;
		int nHasArgument;
		stm.Read(nHasArgument);
		if (nHasArgument)
		{
			char szArgName[255];
			stm.Read(szArgName,255);
			pArgument = m_pAISystem->GetAIObjectByName(szArgName);
		}
		stm.Read(m_bBlocked);

		// select the same pipe
		SelectPipe(0,sName,pArgument);
		m_pCurrentGoalPipe->SetPosition(nPosition);

		int nSubpipes=0;
		stm.Read(nSubpipes);
		while (nSubpipes)
		{
			stm.Read(sName,255);
			stm.Read(nPosition);
			stm.Read(nHasArgument);
			pArgument = 0;
			if (nHasArgument)
			{
				char szArgName[255];
				stm.Read(szArgName,255);
				pArgument = m_pAISystem->GetAIObjectByName(szArgName);
			}

			InsertSubPipe(0,sName,pArgument);
			CGoalPipe *pBottomMostPipe = m_pCurrentGoalPipe;
			while (pBottomMostPipe->IsInSubpipe())
				pBottomMostPipe=pBottomMostPipe->GetSubpipe();

			pBottomMostPipe->SetPosition(nPosition);
			stm.Read(nSubpipes);
		}
	}

	// last op result
	// serialize last op target
	int nHasLastOpResult;
	stm.Read(nHasLastOpResult);
	if (nHasLastOpResult)
	{
		char sName[255];
		stm.Read(sName,255);
		CAIObject *pLOP = m_pAISystem->GetAIObjectByName(sName);
		SetLastOpResult(pLOP);
	}

	if (m_pProxy)
		m_pProxy->Load_PATCH_1(stm);
	UpdatePuppetInternalState();
}