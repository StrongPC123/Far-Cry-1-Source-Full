//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XSnapshot.cpp
//  Description: Snapshot manager class.
//
//  History:
//  - August 14, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XSnapshot.h"
#include "Stream.h"
#include "XSystemBase.h"
///////////////////////////////////////////////
CXSnapshot::CXSnapshot()
{
	m_pTimer=0;
	Reset();
	SetSendPerSecond(0);
	m_clientMaxBitsPerSecond = 768000; // DSL	
	m_iCarryOverBps=0;
}

///////////////////////////////////////////////
CXSnapshot::~CXSnapshot()
{
}

///////////////////////////////////////////////
void CXSnapshot::Init(CXServer *pServer,CXServerSlot *pServerSlot)
{
	m_pServer=pServer;
	m_pServerSlot=pServerSlot;
	m_pTimer=pServer->m_pTimer;
	m_pISystem=pServer->m_pISystem;
	m_pPhysicalWorld=pServer->m_pGame->GetSystem()->GetIPhysicalWorld();
	m_nMaxSnapshotBitSize=0;
	m_nLastSnapshotBitSize=0;
	m_fLastUpdate=0.0f;
}


//!reset the status of the snapshot(called every time)
void CXSnapshot::Reset()
{
	m_stmReliable.Reset();
	m_stmUnreliable.Reset();
	if(m_pTimer)
		m_fLastUpdate=m_pTimer->GetCurrTime();
}

/*!Sets the frequency of snapshot per second
	@param cSendPerSecond number of snapshots per second
*/
void CXSnapshot::SetSendPerSecond(int nSendPerSecond)
{
	m_nSendPerSecond=((nSendPerSecond<=0)?20:nSendPerSecond);
}

/*!Adds a new entity to the snapshot
	@param pEntity a pointer to the entity interface
*/
void CXSnapshot::AddEntity(IEntity *pEntity)
{
	NetEntityListItor itor=m_lstNetEntities.begin();
	EntityId id=pEntity->GetId();
	IEntity *pE;
	while(itor!=m_lstNetEntities.end())
	{
		if(pE=itor->GetEntity())
		{
			if(pE==pEntity)
			{
				//why he found it
				return;
			}
		}
		
		++itor;
	}
	m_lstNetEntities.push_back(CNetEntityInfo(m_pServerSlot,m_pTimer,pEntity));
}

/*!Removes an entity from the snapshot
	@param pEntity a pointer to the entity interface
*/
bool CXSnapshot::RemoveEntity(IEntity *pEntity)
{
	NetEntityListItor itor=m_lstNetEntities.begin();
	EntityId id=pEntity->GetId();
	IEntity *pE;
	while(itor!=m_lstNetEntities.end())
	{
		if(pE=itor->GetEntity())
		{
			if(pE==pEntity)
			{
				break;
			}
		}
		
		++itor;
	}

	if(itor!=m_lstNetEntities.end())
	{
		itor->Invalidate();
		m_lstNetEntities.erase(itor);
	}
	else
	{
		NET_TRACE("<<NET>>CXSnapshot::RemoveEntity ENTITY NOT FOUND [%d]",pEntity->GetId());
		GameWarning("CXSnapshot::RemoveEntity ENTITY NOT FOUND [%d]",pEntity->GetId());		
		return false;
	//	_asm int 3;
	}
	return true;
}

///////////////////////////////////////////////
int CXSnapshot::GetSendPerSecond() 
{ 
	int iActualSendPerSec=m_nSendPerSecond;
	int iServerMax=m_pServer->GetMaxUpdateRate();			// limited by server setting

	if(iActualSendPerSec>iServerMax)
		iActualSendPerSec=iServerMax;

	return iActualSendPerSec; 
}



///////////////////////////////////////////////
float CXSnapshot::GetLastUpdate() const
{
	return m_fLastUpdate;
}


///////////////////////////////////////////////
/*!Build and send a snapshot
	this function retreive the current serverslot entity(the entity associated to this serverslot)
	retreive his camera/position and sort all entities present into the snapshot by priority and
	write all those that fit into the snapshot.
	(The priority is determinated by distance,age,type etc...)
	@param pEntity a pointer to the entity interface
*/
void CXSnapshot::BuildAndSendSnapshot()
{
	FUNCTION_PROFILER( GetISystem(), PROFILE_GAME );

	IBitStream *pBitStream = m_pServer->m_pGame->GetIBitStream();			// compression helper

	m_fLastUpdate=m_pTimer->GetCurrTime();
	IEntity *pEntity;
	IEntity *pSlotEntity=m_pISystem->GetEntity(m_pServerSlot->GetPlayerId());
	if(!pSlotEntity)
		return;

	int iActualSendPerSec=GetSendPerSecond();

	SServerSlotBandwidthStats SSStats;

	m_pServerSlot->GetBandwidthStats(SSStats);

	int nSnapshotSize = SSStats.m_nReliableBitCount + SSStats.m_nUnreliableBitCount;		// in bit per snapshot

	Vec3 v3ViewerPos=pSlotEntity->GetPos();

	NetEntityListItor itor=m_lstNetEntities.begin();

	uint32 dwObjectSum = 0;
	uint32 dwEstimatedBps = 0;
	uint32 dwPriorityMin = 0;

	// update the priority
	while(itor!=m_lstNetEntities.end())
	{
		NetEntityListItor iTemp=itor;
		itor->Update(v3ViewerPos);
		++iTemp;
		
		if(itor->NeedUpdate())
		{
			++dwObjectSum;
			dwEstimatedBps+=itor->CalcEstimatedSize();
			if(dwPriorityMin==0)
				dwPriorityMin=itor->GetPriority();
			 else 
				dwPriorityMin=min(dwPriorityMin,itor->GetPriority());
		}

		itor=iTemp;
	}
	dwEstimatedBps*=iActualSendPerSec;

	if(!dwObjectSum)
		return;							// nothing to do

	// send the server timestamp for the physics

	m_stmUnreliable.Reset();
	m_stmUnreliable.Write(m_pServerSlot->GetCommandClientPhysTime());
	m_stmUnreliable.Write(m_pServerSlot->GetClientWorldPhysTime());
	nSnapshotSize+=m_pServerSlot->SendUnreliableMsg(XSERVERMSG_TIMESTAMP,m_stmUnreliable);
	m_stmUnreliable.Reset();

	
	unsigned int maxRateBps;		// bits per second

	{
		if(m_pServer->m_GameContext.bInternetServer)
			maxRateBps=(unsigned int)m_pServer->sv_maxrate->GetIVal();
		 else
			maxRateBps=(unsigned int)m_pServer->sv_maxrate_lan->GetIVal();

		maxRateBps=min(maxRateBps,m_clientMaxBitsPerSecond);
	}


	if(!m_pServerSlot->m_sClientString.empty())
	{
		m_stmUnreliable.Reset();
		m_stmUnreliable.WritePkd((BYTE)XSERVERMSG_CLIENTSTRING);
		m_stmUnreliable.Write(m_pServerSlot->m_sClientString.c_str());
	}
	
	itor=m_lstNetEntities.begin();

	// per second
	int iAvailableBps =  (int)maxRateBps+m_iCarryOverBps-nSnapshotSize*iActualSendPerSec;

	if(iAvailableBps<0)
		iAvailableBps=100;			// we cannot archieve that little bps because we already wasted more 

	// used as treshold to decide if a entity should be sent or not
	float fPrioritySendLevel = ((float)dwPriorityMin/(float)iActualSendPerSec) * (float)(dwEstimatedBps)/(float)iAvailableBps;

	uint32 dwObjectSent=0;

	// send the entities
	while(itor!=m_lstNetEntities.end())
	{						
		pEntity=itor->GetEntity();

		if(itor->NeedUpdate())
		{
			if(pEntity->GetNetPresence() && !pEntity->IsGarbage() && !m_pServerSlot->IsClientSideEntity(pEntity))
			{
				if(itor->GetTimeAffectedPriority()>=fPrioritySendLevel)
				{
					++dwObjectSent;
					m_stmUnreliable.Reset();	
					
					pBitStream->WriteBitStream(m_stmUnreliable,pEntity->GetId(),eEntityId);

					itor->Write(m_pServer,m_stmUnreliable);

					WRITE_COOKIE_NO(m_stmUnreliable,28);
					int nRealDeltaSize = m_pServerSlot->SendUnreliableMsg(XSERVERMSG_UPDATEENTITY,m_stmUnreliable,pEntity->GetName());

					nSnapshotSize+=nRealDeltaSize;

					m_stmUnreliable.Reset();
					itor->Reset();
				}
			}
		}
		++itor;	
	}

	if(m_pServer->sv_netstats->GetIVal()&0x4)	//log netentities sent/count
		GetISystem()->GetILog()->Log("netentities %d/%d level: %.2f (%dbps(~%dbps)/%dbps(%d)) carry:%d fac:%.2f",dwObjectSent,dwObjectSum,fPrioritySendLevel,
				dwEstimatedBps,nSnapshotSize*iActualSendPerSec,maxRateBps,iAvailableBps,m_iCarryOverBps,
				(float)(dwEstimatedBps)/(float)iAvailableBps);

	m_iCarryOverBps = maxRateBps/iActualSendPerSec - nSnapshotSize;

	// update bandwidth statistics

	m_nLastSnapshotBitSize=nSnapshotSize;
	if(m_nLastSnapshotBitSize>m_nMaxSnapshotBitSize)
		m_nMaxSnapshotBitSize=m_nLastSnapshotBitSize;
	
	Reset();

	m_pServerSlot->ResetBandwidthStats();
}


void CXSnapshot::SetClientBitsPerSecond(unsigned int rate)
{
	m_clientMaxBitsPerSecond=rate;
}