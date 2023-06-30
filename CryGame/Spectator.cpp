
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <IEntitySystem.h>
#include "Game.h"
#include "spectator.h"
#include "XEntityProcessingCmd.h"
#include "XServer.h"

//////////////////////////////////////////////////////////////////////////
CSpectator::CSpectator(CXGame *pGame) :m_vAngles(0,0,0)
{
	m_pScriptObject=NULL;
	m_pGame=pGame;
	m_eiHost=0;
	m_fLastTargetSwitch=0.0f;

	m_AreaUser.SetGame( pGame );
	m_AreaUser.SetEntity( GetEntity() );

}

//////////////////////////////////////////////////////////////////////////
CSpectator::~CSpectator(void)
{
	m_pGame->m_XAreaMgr.ExitAllAreas( m_AreaUser );
}

//////////////////////////////////////////////////////////////////////////
// Initialize the spectator-container.
bool CSpectator::Init()
{
	IEntity *entity = GetEntity();

	entity->GetScriptObject()->SetValue("type", "spectator");
	// set camera
	entity->SetCamera(m_pGame->GetSystem()->GetIEntitySystem()->CreateEntityCamera());
	entity->GetCamera()->GetCamera().Init(m_pGame->GetSystem()->GetIRenderer()->GetWidth(),m_pGame->GetSystem()->GetIRenderer()->GetHeight());	
	m_pTimer=m_pGame->GetSystem()->GetITimer();
	entity->SetNeedUpdate(true);

	entity->SetNetPresence(true);			// needed to sync m_eiHost

	m_roll = 0;

	float fMass=100.0f;
	float fHeight=1.2f;
	float fEyeHeight=1.7f;
	float sphere_height=0.6f;
	float radius=0.6f;
	float fGravity=0.0f;

	entity->CreateLivingEntity(fMass,fHeight,fEyeHeight,sphere_height,radius,-1,fGravity,true,true);

	pe_player_dynamics pd;
	pd.bActive = 1;
	pd.kAirControl=1.0f;
	pd.bSwimming=1;
	entity->GetPhysics()->SetParams(&pd);

	{
		pe_params_flags pf;

		pf.flagsAND=~(lef_push_players | lef_push_objects);

		entity->GetPhysics()->SetParams(&pf);
	}

	{
		pe_params_part pp;

		pp.ipart=0;
		pp.flagsAND=0;

		entity->GetPhysics()->SetParams(&pp);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Update the camera.
void CSpectator::Update()
{
	IEntity *pEntity=GetEntity();		

	m_pGame->ConstrainToSandbox(pEntity);			// limit in the world boundaries

	if(m_pGame->m_pServer)			// detect if player was removed (not 100% accurate, might switch to different player)
	{
		bool bReset=false;

		IEntity *pHost = m_pGame->GetSystem()->GetIEntitySystem()->GetEntity(m_eiHost);

		if(pHost)
		{
			IEntityContainer *pICnt=pHost->GetContainer();

			if(pICnt)
			{
				CPlayer *pPlayer;

				if(!pICnt->QueryContainerInterface(CIT_IPLAYER,(void **)&pPlayer))
					bReset=true;
			}
			else bReset=true;
		}
		else bReset=true;

		if(bReset)
			ResetHostId();					// host entity was removed
	}

	// only a client has to set the camera
	if(m_pGame->IsClient())
	{
		IEntityCamera *pEC;
		Vec3d v3Angles;
		
		if(m_eiHost!=0)
			return;
		
		pEC=pEntity->GetCamera();
		if(pEC)
		{
			pEC->SetAngles(m_vAngles);
			pEC->SetPos(pEntity->GetPos()+Vec3(0,0,0.6f));		// 1.7m eye height
		}

		// update areas only for local spectator
		if(IsMySpectator())
		{
			m_AreaUser.SetEntity(GetEntity());
			m_pGame->m_XAreaMgr.UpdatePlayer( m_AreaUser );
		}

	}
}

//////////////////////////////////////////////////////////////////////////
EntityId CSpectator::GetHostId() const
{
	return m_eiHost;
}

//////////////////////////////////////////////////////////////////////////
void CSpectator::ResetHostId()
{
	m_eiHost=0;
}

//////////////////////////////////////////////////////////////////////////
void CSpectator::OnSetAngles( const Vec3d &ang ) {}

//////////////////////////////////////////////////////////////////////////
IScriptObject *CSpectator::GetScriptObject()
{
	return m_pScriptObject;
}

//////////////////////////////////////////////////////////////////////////
void CSpectator::SetScriptObject(IScriptObject *object)
{
	m_pScriptObject=object;
}

//////////////////////////////////////////////////////////////////////////
// Save upcast.
bool CSpectator::QueryContainerInterface(ContainerInterfaceType desired_interface, void **ppInterface )
{
	if (desired_interface == CIT_ISPECTATOR)
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
void CSpectator::GetEntityDesc( CEntityDesc &desc ) const 
{
}

//////////////////////////////////////////////////////////////////////////
// Process input.
void CSpectator::ProcessKeys(CXEntityProcessingCmd &epc)
{
	if(epc.CheckAction(ACTION_FIRE0))
	{
		if(m_pGame->m_pServer && (m_pTimer->GetCurrTime()-m_fLastTargetSwitch>1))
		{
			epc.RemoveAction(ACTION_FIRE0);
		
			m_pGame->m_pServer->m_ServerRules.OnSpectatorSwitchModeRequest(m_pEntity);
			m_fLastTargetSwitch=m_pTimer->GetCurrTime();
		}
	}

	if(m_eiHost!=0) 
		return;

	// fly-mode
	Vec3d angles=epc.GetDeltaAngles();

	m_vAngles=angles;

	m_vAngles.x = max(m_vAngles.x,-65);
  m_vAngles.x = min(m_vAngles.x,65);

	epc.SetDeltaAngles(m_vAngles);

	Vec3d pos=m_pEntity->GetPos();
	
  int strafe = 0, move = 0;
	if(epc.CheckAction(ACTION_MOVE_FORWARD))  move = 1;
	if(epc.CheckAction(ACTION_MOVE_BACKWARD)) move = -1;
	if(epc.CheckAction(ACTION_MOVE_LEFT))     strafe = -1;
	if(epc.CheckAction(ACTION_MOVE_RIGHT))    strafe = 1;
	

  Vec3d d;
	
  d.x = (float)(move*cry_cosf(DEG2RAD(m_vAngles.z-90)));
  d.y = (float)(move*cry_sinf(DEG2RAD(m_vAngles.z-90)));

  d.x *= (float)cry_cosf(DEG2RAD(-m_vAngles.x));
  d.y *= (float)cry_cosf(DEG2RAD(-m_vAngles.x));
  d.z = (float)(move*cry_sinf(DEG2RAD(-m_vAngles.x)));

  d.x += (float)(strafe*cry_cosf(DEG2RAD(m_vAngles.z-180)));
  d.y += (float)(strafe*cry_sinf(DEG2RAD(m_vAngles.z-180)));

  float curtime = m_pGame->m_pSystem->GetITimer()->GetFrameTime();
      
  if (strafe==0) 
  {
		m_roll = m_roll/(1+(float)cry_sqrtf(curtime*1000.0f)/25);
  }
  else
  {
    m_roll += strafe*curtime/-0.03f;
    float maxroll = 3.0f;
    if(m_roll>maxroll)  m_roll = (float)maxroll;
    if(m_roll<-maxroll) m_roll = (float)-maxroll;
  };

  m_vAngles.y = m_roll;
    
	// move entity by physics
	if(curtime)
	{
		pe_action_move motion;

		motion.dt=curtime;
		motion.dir=d*12.0f;			// spectator speed is 12x

		m_pEntity->GetPhysics()->Action(&motion);
	}
}

//////////////////////////////////////////////////////////////////////////
bool CSpectator::Write(CStream &stm,EntityCloneState *cs)
{
	return stm.Write(m_eiHost);
}

//////////////////////////////////////////////////////////////////////////
bool CSpectator::Read(CStream &stm)
{
	return stm.Read(m_eiHost);
}

	

//////////////////////////////////////////////////////////////////////////
bool CSpectator::IsMySpectator() const
{
	if(!m_pGame->m_pClient) return false;
	return (m_pEntity->GetId()==m_pGame->m_pClient->GetPlayerId());
}


//////////////////////////////////////////////////////////////////////////
void CSpectator::OnEntityNetworkUpdate( const EntityId &idViewerEntity, const Vec3d &v3dViewer, uint32 &inoutPriority, 
	EntityCloneState &inoutCloneState ) const
{
	bool bLocalPlayer = inoutCloneState.m_bLocalplayer;

	if(!bLocalPlayer)								// spectators are not sent to other player
		inoutPriority=0;							// no update at all
}
