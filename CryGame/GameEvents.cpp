
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameEvents.cpp
//  
//	History:
//	-October	31,2003: created
//	
//////////////////////////////////////////////////////////////////////

#include "stdafx.h" 

#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"
#include "XClient.h"
#include "UIHud.h"
#include "XPlayer.h"
#include "PlayerSystem.h"
#include "XServer.h"
#include "WeaponSystemEx.h"
#include "ScriptObjectGame.h"
#include "ScriptObjectInput.h"
#include <IEntitySystem.h>

#include "UISystem.h"
#include "ScriptObjectUI.h"

#include "XVehicle.h"

//////////////////////////////////////////////////////////////////////////////////
void EventPlayerCmd::Write(CStream &stm,int iPhysicalTime, IBitStream *pBitStream )
{
	int iCmdTime = cmd.GetPhysicalTime();
  cmd.SetPhysicalTime(iPhysicalTime);
	pBitStream->WriteBitStream(stm,idEntity,eEntityId);
	cmd.Write(stm,pBitStream,false);
	cmd.SetPhysicalTime(iCmdTime);		
}

//////////////////////////////////////////////////////////////////////////
void EventPlayerCmd::Read(CStream &stm, int &iPhysicalTime, IBitStream *pBitStream )
{
	pBitStream->ReadBitStream(stm,idEntity,eEntityId);
	cmd.Read(stm,pBitStream);
	iPhysicalTime = cmd.GetPhysicalTime();
}

//////////////////////////////////////////////////////////////////////////
void EventPlayerCmd::Execute(CXGame *pGame)
{
	IEntitySystem *pES = pGame->GetSystem()->GetIEntitySystem();
	IEntity *pEnt;
	CPlayer *pPlayer;
	if (pES && (pEnt = pES->GetEntity(idEntity)) && pEnt->GetContainer() && 
			pEnt->GetContainer()->QueryContainerInterface(CIT_IPLAYER, (void**)&pPlayer))
		pPlayer->ProcessMovements(cmd,true);
}

//////////////////////////////////////////////////////////////////////////
void EventExplosion::Write(CStream &stm,int iPhysicalTime, IBitStream *pBitStream )
{
	stm.WritePacked((unsigned int)iPhysicalTime);
	stm.Write(pos);
	stm.Write(damage);
	stm.Write(rmin);
	stm.Write(rmax);
	stm.Write(radius);
	stm.Write(impulsivePressure);
	stm.Write(shakeFactor);
	stm.Write(deafnessRadius);
	stm.Write(deafnessTime);
	stm.Write(iImpactForceMul);
	stm.Write(iImpactForceMulFinal);
	stm.Write(iImpactForceMulFinalTorso);
	stm.Write(rminOcc);
	stm.Write(nOccRes);
	stm.Write(nGrow);
	pBitStream->WriteBitStream(stm,idShooter,eEntityId);
	pBitStream->WriteBitStream(stm,idWeapon,eEntityId);
	stm.Write(terrainDefSize);
	stm.Write(nTerrainDecalId);

	if(nShooterSSID==-1)
		stm.Write((uint8)255);			// used if ClientID is unknwon
	 else
		stm.Write((uint8)nShooterSSID);
}

//////////////////////////////////////////////////////////////////////////
void EventExplosion::Read(CStream &stm,int &iPhysicalTime, IBitStream *pBitStream )
{
	stm.ReadPacked((unsigned int &)iPhysicalTime);
	stm.Read(pos);
	stm.Read(damage);
	stm.Read(rmin);
	stm.Read(rmax);
	stm.Read(radius);
	stm.Read(impulsivePressure);
	stm.Read(shakeFactor);
	stm.Read(deafnessRadius);
	stm.Read(deafnessTime);
	stm.Read(iImpactForceMul);
	stm.Read(iImpactForceMulFinal);
	stm.Read(iImpactForceMulFinalTorso);
	stm.Read(rminOcc);
	stm.Read(nOccRes);
	stm.Read(nGrow);
	pBitStream->ReadBitStream(stm,idShooter,eEntityId);
	pBitStream->ReadBitStream(stm,idWeapon,eEntityId);
	stm.Read(terrainDefSize);
	stm.Read(nTerrainDecalId);

	uint8 ucId;
	stm.Read(ucId);
	if(ucId!=255)
		nShooterSSID=(int32)ucId;			// used if ClientID is unknwon
	 else
		nShooterSSID=-1;
}

//////////////////////////////////////////////////////////////////////////
void EventExplosion::Execute(CXGame *pGame)
{
	IEntitySystem *pES = pGame->GetSystem()->GetIEntitySystem();
	IEntity *pShooter=pES->GetEntity(idShooter), *pWeapon=pES->GetEntity(idWeapon);

	pGame->CreateExplosion(pos,damage,rmin,rmax,radius,impulsivePressure,shakeFactor*(1.0f/254),
		deafnessRadius*(20.0f/254),deafnessTime*0.25f,iImpactForceMul,iImpactForceMulFinal,iImpactForceMulFinalTorso,
		rminOcc*(1.0f/254),nOccRes,nGrow,pShooter,nShooterSSID,pWeapon,terrainDefSize*(20.0f/254),nTerrainDecalId, true);
}

//////////////////////////////////////////////////////////////////////////
void EventPhysImpulse::Write(CStream &stm,int iPhysicalTime, IBitStream *pBitStream )
{
	stm.WritePacked((unsigned int)iPhysicalTime);
	stm.Write(idPhysEnt);
	stm.Write(impulse);
	stm.Write(bHasPt);
	if (bHasPt)
		stm.Write(pt);
	stm.Write(bHasMomentum);
	if (bHasMomentum)
		stm.Write(momentum);
}

//////////////////////////////////////////////////////////////////////////
void EventPhysImpulse::Read(CStream &stm,int &iPhysicalTime, IBitStream *pBitStream )
{
	stm.ReadPacked((unsigned int &)iPhysicalTime);
	stm.Read(idPhysEnt);
	stm.Read(impulse);
	stm.Read(bHasPt);
	if (bHasPt)
		stm.Read(pt);
	stm.Read(bHasMomentum);
	if (bHasMomentum)
		stm.Read(momentum);
}

//////////////////////////////////////////////////////////////////////////
void EventPhysImpulse::Execute(CXGame *pGame)
{
	IPhysicalEntity *pPhysEnt = pGame->GetSystem()->GetIPhysicalWorld()->GetPhysicalEntityById(idPhysEnt);
	if (pPhysEnt)
	{
		pe_action_impulse ai;
		ai.impulse = impulse;
		if (bHasPt)
			ai.point = pt;
		if (bHasMomentum)
			ai.momentum = momentum;
		ai.iSource = 3;
		pPhysEnt->Action(&ai);
	}
}

//////////////////////////////////////////////////////////////////////////
void EventVehicleDamage::Write(CStream &stm,int iPhysicalTime, IBitStream *pBitStream )
{
	stm.WritePacked((unsigned int)iPhysicalTime);
	pBitStream->WriteBitStream(stm,idVehicle,eEntityId);
	stm.Write(damage);
}

//////////////////////////////////////////////////////////////////////////
void EventVehicleDamage::Read(CStream &stm,int &iPhysicalTime, IBitStream *pBitStream )
{
	stm.ReadPacked((unsigned int &)iPhysicalTime);
	pBitStream->ReadBitStream(stm,idVehicle,eEntityId);
	stm.Read(damage);
}

//////////////////////////////////////////////////////////////////////////
void EventVehicleDamage::Execute(CXGame *pGame)
{
	IEntitySystem *pES = pGame->GetSystem()->GetIEntitySystem();
	IEntity *pEnt;
	CVehicle *pVehicle;
	if (pES && (pEnt = pES->GetEntity(idVehicle)) && pEnt->GetContainer() && 
			pEnt->GetContainer()->QueryContainerInterface(CIT_IVEHICLE, (void**)&pVehicle))
		pVehicle->SetEngineHealth(damage*0.5f,true);
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::HasScheduledEvents()
{
	return m_lstEvents.size()>0;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::ScheduleEvent(int iPhysTime, BaseEvent *pEvent)
{
	if (iPhysTime<0)
	{
		CXServer *pServer=GetServer();

		iPhysTime = SnapTime(m_pSystem->GetIPhysicalWorld()->GetiPhysicsTime());

		if(pServer)
			iPhysTime += pServer->GetSchedulingDelay();
		 else
			m_pLog->LogError("ScheduleEvent pServer is 0"); 
	}

	int nSize=m_lstEvents.size(),iMiddle,iBound[2]={ 0,nSize };
	if (iBound[1]>0) do {
		iMiddle = (iBound[0]+iBound[1])>>1;
		iBound[isneg(iPhysTime-m_lstEvents[iMiddle].iPhysTime)] = iMiddle;
	} while(iBound[1]>iBound[0]+1);
	for(; iBound[1]<nSize && m_lstEvents[iBound[1]].iPhysTime==iPhysTime; iBound[1]++);
	if (iBound[0]<nSize && m_lstEvents[iBound[0]].iPhysTime>iPhysTime)
		iBound[1] = iBound[0];

	GameEvent event;
	event.iPhysTime = iPhysTime;
	event.idx = m_iLastCmdIdx++;
	(event.pEvent = pEvent)->nRefCount = 1;
	m_lstEvents.insert(m_lstEvents.begin()+iBound[1], event);
}

//////////////////////////////////////////////////////////////////////////
void CXGame::ScheduleEvent(IEntity *pEnt, CXEntityProcessingCmd &cmd)
{
	EventPlayerCmd *pEvent = new EventPlayerCmd;
	pEvent->idEntity = pEnt->GetId();
	pEvent->cmd = cmd;
	pEvent->cmd.ResetTimeSlices();
	ScheduleEvent(cmd.GetPhysicalTime(), pEvent);
}

//////////////////////////////////////////////////////////////////////////
void CXGame::ScheduleEvent(int iPhysTime, const Vec3& pos,float fDamage,float rmin,float rmax,float radius,float fImpulsivePressure,
													 float fShakeFactor,float fDeafnessRadius,float fDeafnessTime,
													 float fImpactForceMul,float fImpactForceMulFinal,float fImpactForceMulFinalTorso,
													 float rMinOcc,int nOccRes,int nOccGrow, IEntity *pShooter, int shooterSSID, IEntity *pWeapon, float fTerrainDefSize,int nTerrainDecalId)
{
	EventExplosion *pEvent = new EventExplosion;
	pEvent->pos = pos;
	pEvent->damage = fDamage;
	pEvent->rmin = rmin;
	pEvent->rmax = rmax;
	pEvent->radius = radius;
	pEvent->impulsivePressure = fImpulsivePressure;
	pEvent->shakeFactor = (int)(fShakeFactor*254.0f+0.5f);
	pEvent->deafnessRadius = (int)(fDeafnessRadius*(254.0f/20)+0.5f);
	pEvent->deafnessTime = (int)(fDeafnessTime*4+0.5f);
	pEvent->iImpactForceMul = (int)(fImpactForceMul+0.5f);
	pEvent->iImpactForceMulFinal = (int)(fImpactForceMulFinal+0.5f);
	pEvent->iImpactForceMulFinalTorso = (int)(fImpactForceMulFinalTorso+0.5f);
	pEvent->rminOcc = (int)(rMinOcc*254.0f+0.5f);
	pEvent->nOccRes = nOccRes;
	pEvent->nGrow = nOccGrow;
	pEvent->idShooter = pShooter ? pShooter->GetId() : -1;
	pEvent->idWeapon = pWeapon ? pWeapon->GetId() : -1;
	pEvent->terrainDefSize = (int)(fTerrainDefSize*(254.0f/20)+0.5f);
	pEvent->nTerrainDecalId = nTerrainDecalId;
	pEvent->nShooterSSID=shooterSSID;
	ScheduleEvent(iPhysTime,pEvent);
}

//////////////////////////////////////////////////////////////////////////
void CXGame::ScheduleEvent(int iPhysTime, IEntity *pVehicle,float fDamage)
{
	EventVehicleDamage *pEvent = new EventVehicleDamage;
	pEvent->idVehicle = pVehicle->GetId();
	pEvent->damage = (int)(fDamage*2+0.5f);
	ScheduleEvent(iPhysTime, pEvent);
}

////
void CXGame::ScheduleEvent(int iPhysTime, IPhysicalEntity *pPhysEnt,pe_action_impulse *pai)
{
	EventPhysImpulse *pEvent = new EventPhysImpulse;
	pEvent->idPhysEnt = m_pSystem->GetIPhysicalWorld()->GetPhysicalEntityId(pPhysEnt);
	pEvent->impulse = pai->impulse;
	if (pEvent->bHasPt = !is_unused(pai->point))
		pEvent->pt = pai->point;
	if (pEvent->bHasMomentum = !is_unused(pai->momentum))
		pEvent->momentum = pai->momentum;
	ScheduleEvent(iPhysTime, pEvent);
}

//////////////////////////////////////////////////////////////////////////
void CXGame::ExecuteScheduledEvents()
{
	int iCurTime=SnapTime(m_pSystem->GetIPhysicalWorld()->GetiPhysicsTime());
	size_t i, sz = m_lstEvents.size();
	for(i=0; i<sz && m_lstEvents[i].iPhysTime <= iCurTime; ++i)
	{
		if (m_lstEvents[i].iPhysTime<iCurTime)
			m_pLog->Log("Event missed the schedule by %.4f", 
			(iCurTime-m_lstEvents[i].iPhysTime)*m_pSystem->GetIPhysicalWorld()->GetPhysVars()->timeGranularity);
		m_lstEvents[i].pEvent->Execute(this);
	}

	if (i)
		m_lstEvents.erase(m_lstEvents.begin(),m_lstEvents.begin()+i);
}

//////////////////////////////////////////////////////////////////////////
void CXGame::WriteScheduledEvents(CStream &stm, int &iLastEventWritten, int iTimeDelta)
{
	IBitStream *pBitStream=GetIBitStream();

	unsigned int i,sz;
	int iLastEventCur = -1;
	for(i=sz=0; i<m_lstEvents.size(); i++)
		sz += m_lstEvents[i].idx>iLastEventWritten;
	stm.Write((short)sz);

	if (sz>0)
	{
		for(i=0; i<m_lstEvents.size(); i++)
		if (m_lstEvents[i].idx>iLastEventWritten)
		{
			stm.WriteNumberInBits(m_lstEvents[i].pEvent->GetType(),2);
			m_lstEvents[i].pEvent->Write(stm,m_lstEvents[i].iPhysTime+iTimeDelta,pBitStream);
			iLastEventCur = max(iLastEventCur, m_lstEvents[i].idx);
		}
		iLastEventWritten = iLastEventCur;
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::ReadScheduledEvents(CStream &stm)
{
	IBitStream *pBitStream=GetIBitStream();
	short nEvents;

	stm.Read(nEvents);
	for(int i=0; i<nEvents; i++)
	{
		int iType=0; stm.ReadNumberInBits(iType,2);
		BaseEvent *pEvent;
		switch (iType)
		{
			case EVENT_MOVECMD: pEvent = new EventPlayerCmd; break;
			case EVENT_EXPLOSION: pEvent = new EventExplosion; break;
			case EVENT_IMPULSE: pEvent = new EventPhysImpulse; break;
			case EVENT_VEHDAMAGE: pEvent = new EventVehicleDamage;
		}
		int iPhysTime;
		pEvent->Read(stm,iPhysTime,pBitStream);
		ScheduleEvent(iPhysTime,pEvent);
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::AdvanceReceivedEntities(int iPhysicalWorldTime)
{
	m_bSynchronizing = true;

	IPhysicalWorld *pWorld = GetSystem()->GetIPhysicalWorld();
	//m_pLog->LogToConsole("client time step %.3f",(pWorld->GetiPhysicsTime()-m_iPhysicalWorldTime)*pWorld->GetPhysVars()->timeGranularity);
	float timeGran=pWorld->GetPhysVars()->timeGranularity, fixedStep=g_MP_fixed_timestep->GetFVal(), fStep;
	int iPrevTime=pWorld->GetiPhysicsTime(), iStep=GetiFixedStep();

	if (iPrevTime>iPhysicalWorldTime)
	{
		if (UseFixedStep())
		{
			pWorld->SetiPhysicsTime(SnapTime(iPhysicalWorldTime));
			int i = SnapTime(iPrevTime)-SnapTime(iPhysicalWorldTime);
			if (i>iStep*20)
			{
				m_pLog->LogToConsole("Client catch-up step is too big (%.4f)",i*timeGran);
				i = iStep*20;
			}
			for(; i>0; i-=iStep)
			{
				//m_pGame->ExecuteScheduledEvents();
				pWorld->TimeStep(fixedStep, ent_rigid|ent_flagged_only);
			}
			pWorld->SetiPhysicsTime(iPhysicalWorldTime);
			fStep = min(fixedStep*20, (iPrevTime-iPhysicalWorldTime)*timeGran);
			do {
				pWorld->TimeStep(min(fStep,0.1f), ent_living|ent_independent|ent_flagged_only);
			} while	((fStep-=0.1f)>0.0001f);
		}
		else
		{
			fStep = min(0.3f, (pWorld->GetiPhysicsTime()-iPhysicalWorldTime)*timeGran);
			do {
				pWorld->TimeStep(min(fStep,0.1f), ent_all|ent_flagged_only);
			} while ((fStep-=0.1f)>0.0001f);
		}
		pWorld->SetiPhysicsTime(iPrevTime);
	}
	else
		pWorld->SetiPhysicsTime(iPhysicalWorldTime);

	m_bSynchronizing = false;
}
