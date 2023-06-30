
//////////////////////////////////////////////////////////////////////
//
//	Game source code (c) Crytek 2001-2003
//	
//	File: GameMisc.cpp
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
#include "ScriptObjectBoids.h"
#include "Flock.h" 
#include "WeaponClass.h"
#include "ScriptObjectRenderer.h"
#include "ScriptTimerMgr.h"

//////////////////////////////////////////////////////////////////////////
void CXGame::EnableUIOverlay(bool bEnable, bool bExclusiveInput)
{
	if (!m_bEditor)
	{
		m_bUIOverlay = bEnable;

		if ((bExclusiveInput) && (!m_bUIExclusiveInput))
		{
			m_pIActionMapManager->Disable();
			m_pSystem->GetIInput()->SetExclusiveListener(m_pUISystem);

			m_bUIExclusiveInput = 1;
		}
		else if ((!bExclusiveInput) && (m_bUIExclusiveInput))
		{
			m_pIActionMapManager->Enable();
			m_pSystem->GetIInput()->SetExclusiveListener(0);

			m_bUIExclusiveInput = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::IsUIOverlay()
{
	return m_bUIOverlay;
}

//////////////////////////////////////////////////////////////////////////
//! Check if a sound is potentially hearable (used to check if loading a dialog is needed)
bool CXGame::IsSoundPotentiallyHearable(Vec3d &SoundPos, float fClipRadius)
{
	ASSERT(m_pSystem);
	ISoundSystem *pSoundSystem=m_pSystem->GetISoundSystem();
	if (!pSoundSystem)
		return false;
	if (pSoundSystem->UsingDirectionalAttenuation())
		return true;
	CCamera &Cam=m_pSystem->GetViewCamera();
	float fDist=(Cam.GetPos()-SoundPos).GetLengthSquared();
	if (fDist<fClipRadius)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////
//! Retrieve the server-rules.
CXServerRules* CXGame::GetRules() const
{
	if (m_pServer) 
		return m_pServer->GetRules();
	// if not server.
	return 0;
}

//////////////////////////////////////////////////////////////////////
//!Force the view camera of the player
//!
//!(NOTE: this function is here because the editor needs it here)
void CXGame::SetViewAngles(const Vec3d &angles)
{
	if (m_pClient)
		m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(angles);
}

//////////////////////////////////////////////////////////////////////
//! Retrieve the local player-entity
IEntity *CXGame::GetMyPlayer()
{
	if (m_pClient)
		return m_pClient->m_pISystem->GetLocalPlayer();
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//! Selects the current UI (hud etc.)
void CXGame::SetCurrentUI(CUIHud *pUI)
{
	if (pUI!=m_pCurrentUI)  // new ui has been selected
	{
		// shutdown the old one
		if (m_pCurrentUI) 
			m_pCurrentUI->ShutDown();
		// init the new one
		m_pCurrentUI = pUI;
		m_pCurrentUI->Init(m_pScriptSystem);
	}
}

// changes in and out of third person
//////////////////////////////////////////////////////////////////////////
void CXGame::SetViewMode(bool bThirdPerson)
{
	if (GetMyPlayer())
	{
		CPlayer *pPlayer;
		if (GetMyPlayer()->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer))
		{
			// prevent player from going into third person mode when he is aiming
			if (pPlayer->m_stats.aiming)
				return;
			
			// disable third person view when not in vehicle AND not in devmode
			if(!IsDevModeEnable() && bThirdPerson && !pPlayer->GetVehicle())
				return;
			/*
			if (pPlayer->GetVehicle() && bThirdPerson)
			{
				// do not allow the player to switch to 3rd person
				// mode when driving a vehicle
				return;
			}
			*/

			pPlayer->m_bFirstPerson = !bThirdPerson;
			// don't hide player - need to call CPlayer::OnDraw always for mounted weapons
			// use pPlayer->m_bFirstPerson to not draw player
//			if (bThirdPerson)
				pPlayer->GetEntity()->DrawCharacter(0,ETY_DRAW_NORMAL);
//			else
//				pPlayer->GetEntity()->DrawCharacter(0,ETY_DRAW_NONE);

			pPlayer->SetViewMode(bThirdPerson);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::HideLocalPlayer( bool hide,bool bEditor )
{
	m_bHideLocalPlayer = hide;
	if (GetMyPlayer())
	{
		CPlayer *pPlayer;
		if (GetMyPlayer()->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**)&pPlayer))
		{
			// [Marco K] weapons are now drawn through the player Entity
			if(pPlayer->GetSelectedWeapon()){
				if(hide){
					pPlayer->GetEntity()->DrawCharacter(1, 0);
				}
				else{
					pPlayer->GetEntity()->DrawCharacter(1, CS_FLAG_DRAW_NEAR);
				}
			}

			if (!bEditor)
			{
				// Dont do this in editor.
				pPlayer->GetEntity()->EnablePhysics(!hide);
				if (!hide)
				{
					// Force player positin on physics if it was desynced.
					Vec3 pos = pPlayer->GetEntity()->GetPos();
					pPlayer->GetEntity()->SetPos(pos+Vec3(0,0,0.1f)); // Force change of position.
					pPlayer->GetEntity()->SetPos(pos);
				}
			}
			// hide actual player
			if(hide)
			{
				pPlayer->GetEntity()->DrawCharacter(0,0);
			}
			else //if (!pPlayer->IsFirstPerson())
				pPlayer->SetViewMode(!pPlayer->IsFirstPerson());
			//GetEntity()->DrawCharacter(0,1);
				
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CXGame::ReloadScripts()
{
	// includes the physical material properties (e.g. friction)
	m_XSurfaceMgr.ReloadMaterials();
}

//////////////////////////////////////////////////////////////////////////
void CXGame::OnSetVar(ICVar *pVar)
{
	IXSystem *pS=GetXSystem();
	if (pS && IsMultiplayer())
		pS->OnSetVar(pVar);
}

//////////////////////////////////////////////////////////////////////////
void CXGame::CreateExplosion(const Vec3& pos,float fDamage,float rmin,float rmax,float radius,float fImpulsivePressure,
														 float fShakeFactor,float fDeafnessRadius,float fDeafnessTime,
														 float fImpactForceMul,float fImpactForceMulFinal,float fImpactForceMulFinalTorso,
														 float rMinOcc,int nOccRes,int nOccGrow, IEntity *pShooter, int shooterSSID, IEntity *pWeapon, 
														 float fTerrainDefSize,int nTerrainDecalId, bool bScheduled)
{
	if (!bScheduled && IsMultiplayer() && UseFixedStep())
	{
		if (IsServer())
			ScheduleEvent(-1, pos,fDamage,rmin,rmax,radius,fImpulsivePressure,fShakeFactor,fDeafnessRadius,fDeafnessTime,fImpactForceMul,
				fImpactForceMulFinal,fImpactForceMulFinalTorso,rMinOcc,nOccRes,nOccGrow, pShooter,shooterSSID,pWeapon, fTerrainDefSize,nTerrainDecalId);
		return;
	}

	IPhysicalWorld *pWorld = m_pSystem->GetIPhysicalWorld();
	int iTypes = ent_rigid|ent_sleeping_rigid|ent_living;
	if (m_pSystem->GetIConsole()->GetCVar("physics_quality")->GetIVal()>=2)
		iTypes |= ent_independent;

	//simulate the first explosion only for applying damage(0 as impulse and -1 as occlusion res)
	// [Anton] the first explosion is a normal one: it builds occlusion map and applies impulse
	pWorld->SimulateExplosion(pos,pos, rmin,rmax,rmin, fImpulsivePressure, nOccRes,nOccGrow,rMinOcc, 0,0, iTypes);

	float	force = fDamage*.0173f;
	float	falloff, curDamage;
	if( force > 2.0f )
		force = 2.0f;
	m_pSystem->GetI3DEngine()->ApplyForceToEnvironment(pos,radius,force);

	IPhysicalEntity **ppList = NULL;
	int iNumEntites;
	int i;
	IEntity *pIEntity = NULL;

	vectorf vMin(pos.x - radius, pos.y - radius, pos.z - radius);
	vectorf vMax(pos.x + radius, pos.y + radius, pos.z + radius);

	_SmartScriptObject pGR(m_pScriptSystem,true);
	m_pScriptSystem->GetGlobalValue("GameRules",*pGR);

	//////////////////////////////////////////////////////////////////////////
	// Check independent entities.
	//////////////////////////////////////////////////////////////////////////
	ppList = NULL;
	iNumEntites = pWorld->GetEntitiesInBox(vMin, vMax, ppList, ent_independent|ent_allocate_list);
	for (i=0; i<iNumEntites; i++)
	{
		int physType = ppList[i]->GetiForeignData();
		if (physType == OT_BOID)
		{
			// Check if boid hit.
			CBoidObject *pBoid = (CBoidObject*)ppList[i]->GetForeignData(OT_BOID);
			if (pBoid)
			{
				string surfaceName;
				pBoid->Kill( pos,(pBoid->m_pos-pos)*10.0f,surfaceName );
			}
		}
	}
	pWorld->GetPhysUtils()->DeletePointer(ppList);
	//////////////////////////////////////////////////////////////////////////

	ppList = NULL;
	iNumEntites = pWorld->GetEntitiesInBox(vMin, vMax, ppList, (ent_all&~(ent_terrain|ent_independent))|ent_allocate_list);

	for (i=0; i<iNumEntites; i++)
	{
		pIEntity = (IEntity *) ppList[i]->GetForeignData();
		if (!pIEntity)
		{
			continue;
		}
		
		// to take big objects into account be measure the distance to the bounding sphere surface
		float fPhyEntityRad = pIEntity->GetRadiusPhys();
	
		float fDistance = (pos - pIEntity->GetPos()).Length()-fPhyEntityRad;
		
		if(fDistance<0)
			fDistance=0;			// the objects interpenetrate
		
		if (fDistance > radius)
			continue;
		falloff = 1.0f - fDistance / radius;
		curDamage = fDamage*falloff;
		Vec3 bbox[2]; pIEntity->GetBBox(bbox[0],bbox[1]);
		Vec3 dir=(bbox[0]+bbox[1])*0.5f-pos;//pIEntity->GetPos()-pos;
		dir.Normalize();
		CScriptObjectVector oDir(m_pScriptSystem),oPos(m_pScriptSystem);
		oDir=dir;	oPos=pIEntity->GetPos();

		// send DEAFENED event is needed...
		if (fDeafnessRadius>0.0f)
		{
			float fDeafenedTime=fDeafnessTime*(1.0f-fDistance/fDeafnessRadius);;
			_SmartScriptObject pObj(m_pScriptSystem);
			pObj->SetValue("fTime", fDeafenedTime);
			pIEntity->SendScriptEvent(ScriptEvent_Deafened, *pObj);
		}
		// send DAMAGE event
		_SmartScriptObject pTable(m_pScriptSystem);
		if (pWeapon)
			pTable->SetValue("weapon",pWeapon->GetScriptObject());
		if (pIEntity->GetScriptObject())
			pTable->SetValue("target",pIEntity->GetScriptObject());
		pTable->SetValue( "explosion",true );
		if (pShooter && !IsMultiplayer())								// in Multiplayer it's not save to send the shooter - the EntityId might be reused already
			pTable->SetValue("shooter",pShooter->GetScriptObject());

		if(shooterSSID!=-1)																// -1 means unknown ClientID
			pTable->SetValue("shooterSSID",shooterSSID);		// in Multiplayer this is the save way to get the shooter
		
		pTable->SetValue("dir",*oDir);
		pTable->SetValue("damage",curDamage);
		pTable->SetValue("damage_type", "normal");
		pTable->SetValue("weapon_death_anim_id", 0);
		pTable->SetValue("pos",*oPos);
		pTable->SetValue("ipart", -1);

		pTable->SetValue("impact_force_mul",fImpactForceMul*falloff);
		pTable->SetValue("impact_force_mul_final",fImpactForceMulFinal*falloff);
		pTable->SetValue("impact_force_mul_final_torso",fImpactForceMulFinalTorso*falloff);

		//_SmartScriptObject pMat(m_pScriptSystem, true);
		//if (m_pScriptSystem->GetGlobalValue("mat_flesh", pMat))
		IScriptObject *pMat  = NULL;
		pMat = m_XSurfaceMgr.GetMaterialByName("mat_flesh");
		if (pMat)
			pTable->SetValue("target_material", pMat);

		pIEntity->OnDamage(pTable);		
	}
	pWorld->GetPhysUtils()->DeletePointer(ppList);

	//
	//shake the camera from explosion
	pIEntity = GetMyPlayer();
	if( pIEntity )
	{
		CPlayer* pPlayer=NULL;
		CXClient *pClient=GetClient();
		if (pIEntity->GetContainer()) 
			pIEntity->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void**) &pPlayer);
		if(pPlayer && pClient)
		{
//		float	distCoeff = damage/(pIEntity->GetPos()-pos).Length();
//		float	distCoeff = damage*(1.0f - (pIEntity->GetPos()-pos).Length()*pClient->cl_explShakeDCoef->GetFVal());
		float	distCoeff = (1.0f - (pIEntity->GetPos()-pos).Length()*pClient->cl_explShakeDCoef);
			if( distCoeff>1.0f )
				distCoeff = 1.0f;
			if( distCoeff>0.0f )
			{
				distCoeff *= fShakeFactor;
				pPlayer->SetShakeL(	Vec3(fDamage*distCoeff*pClient->cl_explShakeAmplH, 
																 fDamage*distCoeff*pClient->cl_explShakeAmplH, 
																 fDamage*distCoeff*pClient->cl_explShakeAmplV),
													Vec3(pClient->cl_explShakeFreq, pClient->cl_explShakeFreq, pClient->cl_explShakeFreq),
													distCoeff*pClient->cl_explShakeTime);
			}
		}
	}
	//simulate the explosion for adding impulse .
	// [Anton] the 2nd explosion has nOccRes=-1, meaning it will reuse the occlusion map from the previous explosion
	// and add impulse only to the objects that were not affected by the previous explosion
	pWorld->SimulateExplosion(pos,pos, rmin, rmax, rmin, fImpulsivePressure, -1,nOccGrow,rMinOcc, 0,0,iTypes);

	if (fTerrainDefSize>0)
	{
		bool bDeform = false;

		//check if e_deformable_terrain for some reason is NULL, maybe at the init time it wasnt already created.
		if (!e_deformable_terrain)
			e_deformable_terrain = m_pSystem->GetIConsole()->GetCVar("e_deformable_terrain");

		// always make it false, when e_deformable_terrain is set to 0
		if (e_deformable_terrain)
			bDeform = e_deformable_terrain->GetIVal()!=0?true:false;

		Vec3d vHitDir = (pShooter->GetPos() - pos).GetNormalized();
		m_pSystem->GetI3DEngine()->OnExplosion(pos,vHitDir,fTerrainDefSize,nTerrainDecalId,bDeform);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CXGame::GoreOn() const
{
	return (g_Gore->GetIVal()==2);
}

//////////////////////////////////////////////////////////////////////////
IBitStream *CXGame::GetIBitStream()
{
	if(IsMultiplayer())
		return &m_BitStreamCompressed;

	return &m_BitStreamBase;
}
//////////////////////////////////////////////////////////////////////////
bool CXGame::ExecuteScript(const char *sPath,bool bForceReload)
{
	string temp=sPath;
	string::size_type n;
	n=temp.find("$GT$");
	if(n!=string::npos){
		temp.replace(n,4,g_GameType->GetString());
		if(!m_pScriptSystem->ExecuteFile(temp.c_str(),false,bForceReload))
		{
			temp=sPath;
			temp.replace(n,4,"Default");
			return m_pScriptSystem->ExecuteFile(temp.c_str(),true,bForceReload);
		}
		return true;
	}
//no special path
	return m_pScriptSystem->ExecuteFile(sPath,true);
}

//////////////////////////////////////////////////////////////////////////
void CXGame::OnCollectUserData(INT_PTR nValue,int nCookie)		//AMD Port
{
	switch(nCookie){
		case USER_DATA_SCRIPTOBJRENDERER:{
			CScriptObjectRenderer *pSS=(CScriptObjectRenderer *)nValue;
			delete pSS;
										 }
			break;
		default:
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
int CXGame::AddTimer(IScriptObject *pTable,unsigned int nStartTimer,unsigned int nTimer,IScriptObject *pUserData,bool bUpdateDuringPause)
{
	return (m_pScriptTimerMgr->AddTimer(pTable,nStartTimer,nTimer,pUserData,bUpdateDuringPause));
}
