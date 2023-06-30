
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameCallBacks.cpp
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
#include "XVehicleSystem.h"
#include "XVehicle.h"

///////////////////////////////////////////////////////////////////////////
/////////////////////////// physics callbacks /////////////////////////////

//////////////////////////////////////////////////////////////////////////
int CXGame::CreatePhysicalEntity(void *pForeignData,int iForeignData,int iForeignFlags)
{
	switch (iForeignData&0x0F) 
	{
		case 0: return ((IEntity*)pForeignData)->CreatePhysicalEntityCallback(iForeignFlags); // CEntity
		case 1: ((IEntityRender*)pForeignData)->Physicalize(true); return 1; // CBrush
		case 2: return m_pSystem->GetI3DEngine()->PhysicalizeStaticObject(pForeignData,iForeignData,iForeignFlags); // CStatObjInst
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
int CXGame::DestroyPhysicalEntity(IPhysicalEntity *pent)
{
	pe_params_foreign_data pfd;
	pent->GetParams(&pfd);

	switch (pfd.iForeignData&0x0F) 
	{
		case 0: return ((IEntityRender*)pfd.pForeignData)->DestroyPhysicalEntityCallback(pent); // CEntity
		case 1: ((IEntityRender*)pfd.pForeignData)->DestroyPhysicalEntityCallback(pent); return 1; // CBrush
		case 2: return 2; // CStatObjInst
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
const char *CXGame::GetForeignName(void *pForeignData,int iForeignData,int iForeignFlags)
{
	if (pForeignData)
		switch (iForeignData) 
		{
			case 0: return ((IEntity*)pForeignData)->GetName();
			case 1: return "Brush/StatObj";
		}
	return "Orphan Entity";
}

//////////////////////////////////////////////////////////////////////////
void CXGame::OnBBoxOverlap(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, 
													 IPhysicalEntity *pCollider, void *pColliderForeignData,int iColliderForeignData)
{
	if (iForeignData==0 && iColliderForeignData==0)
	{
		IEntity *pEntity = (IEntity*)pForeignData;
		IEntity *pColliderEntity = (IEntity*)pColliderForeignData;
		pEntity->OnPhysicsBBoxOverlap( pColliderEntity );
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::OnCollision(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, coll_history_item *pCollision)
{
}

//////////////////////////////////////////////////////////////////////////
void CXGame::OnStateChange(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, int iOldSimClass,int iNewSimClass)
{
	// If foregin data is entity.
	if (iForeignData==0)
	{
		IEntity *pEntity = ((IEntity*)pForeignData);
		pEntity->OnPhysicsStateChange( iNewSimClass,iOldSimClass );
	}
}

//////////////////////////////////////////////////////////////////////////
int CXGame::OnImpulse(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, pe_action_impulse *action)
{
	if (iForeignData==0 && IsMultiplayer() && UseFixedStep() && ((IEntity*)pForeignData)->GetNetPresence())
	{
		if (IsServer())
			ScheduleEvent(-1,pEntity,action);
		return 0;
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////
void CXGame::OnPostStep(IPhysicalEntity *pEntity, void *pForeignData,int iForeignData, float fTimeInterval)
{
	IEntityContainer *pCnt;
	CVehicle *pVehicle;
	CPlayer *pPlayer;

	if (iForeignData==0 && pForeignData)
	{
		if (((IEntity*)pForeignData)->GetUpdateVisLevel()==eUT_PhysicsPostStep)
		{
			SEntityUpdateContext ctx;
			ctx.nFrameID = m_pSystem->GetIRenderer() ? m_pSystem->GetIRenderer()->GetFrameID() : 0;
			ctx.pCamera = &m_pSystem->GetViewCamera();
			ctx.fCurrTime = m_pSystem->GetITimer()->GetCurrTime();
			ctx.fFrameTime = fTimeInterval;
			ctx.bProfileToLog = false;
			ctx.numVisibleEntities = 0;
			ctx.numUpdatedEntities = 0;

			((IEntity*)pForeignData)->Update(ctx);
		}
		else if (pCnt=((IEntity*)pForeignData)->GetContainer())
		{
			if (pCnt->QueryContainerInterface(CIT_IVEHICLE, (void**)&pVehicle))
				pVehicle->UpdatePhysics(fTimeInterval);
			else if (pCnt->QueryContainerInterface(CIT_IPLAYER, (void**)&pPlayer))
				pPlayer->UpdatePhysics(fTimeInterval);
		}
	}
}
