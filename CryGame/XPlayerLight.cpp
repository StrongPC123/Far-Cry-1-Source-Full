  
//////////////////////////////////////////////////////////////////////
//
//  Game Source Code
//
//  File: XPlayerLight.cpp
//  Description: Entity player class.
//					flashlight funtionality
//
//  History:
//	- apr 17,2003: Created by Kirill - splitting Xplayer.cpp in multiple files
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "XPlayer.h"

#include <IAgent.h>

// <<FIXME>> look above
#include "I3DEngine.h"

// remove this
#include <IAISystem.h>




//
//-----------------------------------------------------------------


Vec3d	CPlayer::GetFLightPos( )
{
	
	if(IsMyPlayer())
	{
		Vec3d pos;
		Vec3d ang;
		GetFirePosAngles(pos, ang);

		ang = ConvertToRadAngles(ang);

		return pos - ang;
		
	}
	if(m_pGame->pl_head->GetIVal()==0)
	{
		ICryCharInstance *pChar = m_pEntity->GetCharInterface()->GetCharacter(0);
		if(pChar)
		{
			ICryBone * bone = pChar->GetBoneByName("weapon_bone"); // find bone in the list of bones;
			if(bone)
			{
				Matrix44 mat;
				mat.SetIdentity();
				Vec3d rot = m_pEntity->GetAngles();
				rot.x=0;
				//mat.RotateMatrix_fix(angles);
				mat=Matrix44::CreateRotationZYX(-gf_DEGTORAD*rot)*mat; //NOTE: angles in radians and negated 
				return m_pEntity->GetPos() + mat.TransformPointOLD(bone->GetBonePosition()) - Vec3d(0,0,0.3f);
			}
		}
	}
	else
	{

		Vec3d pos;
		Vec3d ang;
		GetFirePosAngles(pos, ang);

		return pos + Vec3d(0.f,0.f,.3f);

/*
		if(m_pBoneHead)
		{
	//		light->m_Origin = m_pEntity->GetPos() + m_pBoneHead->GetBonePosition();
			Matrix44 mat;
			mat.Identity();
			Vec3d rot = m_pEntity->GetAngles();
			rot.x=0;
			//mat.RotateMatrix_fix(angles);
			mat=GetRotationZYX44(-gf_DEGTORAD*rot)*mat; //NOTE: angles in radians and negated 
			return m_pEntity->GetPos() + mat.TransformPoint(m_pBoneHead->GetBonePosition());
		}
*/
	}
	return m_pEntity->GetPos() + Vec3d(0,0,2);
}



//
//-----------------------------------------------------------------
void	CPlayer::ProceedFLight( )
{
	if(!m_bLightOn)
		return;

	if( !m_pDynLight )
		return;

	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	float	totalLightScale = 1.0f - m_pGame->GetSystem( )->GetI3DEngine()->GetAmbientLightAmountForEntity(m_pEntity)*2.0f;

	// for local player make it always bright
	if( IsMyPlayer() )
		totalLightScale = 1.0f;

	if(totalLightScale<.2f)
		totalLightScale = .2f;

	//
	// fixme - this causes crach in destructor
	m_pDynLight->m_Flags = DLF_PROJECT;
	if(IsMyPlayer())
		m_pDynLight->m_fLightFrustumAngle = 30;
	else
		m_pDynLight->m_fLightFrustumAngle = m_pGame->p_lightfrustum->GetFVal();	// for other players - more light
	m_pDynLight->m_fRadius = m_pGame->p_lightrange->GetFVal();

	m_pDynLight->m_Origin = GetFLightPos();
//	light->m_ProjAngles = m_pEntity->GetAngles();
	m_pDynLight->m_ProjAngles = Vec3d(m_pEntity->GetAngles().y,m_pEntity->GetAngles().x,m_pEntity->GetAngles().z-90);
	
	m_pDynLight->m_Color = CFColor(totalLightScale,totalLightScale,totalLightScale, 1.0f);
	m_pDynLight->m_SpecColor = CFColor(totalLightScale,totalLightScale,totalLightScale);

//	m_pDynLight->m_Color = CFColor(.45f,.45f,.45f, 1.0f);
//	m_pDynLight->m_SpecColor = CFColor(.45f,.45f,.45f);

	m_pDynLight->m_Flags	 |= (DLF_LIGHTSOURCE | DLF_SPECULAR_ONLY_FOR_HIGHSPEC);


/*
	light->m_pShader = 0;
	if (!light->m_pLightImage)
	{
		//Light.m_pLightImage = m_IndInterface.m_pRenderer->EF_LoadTexture("FlashLightCube", 0, FT2_FORCECUBEMAP, eTT_Cubemap);
		light->m_pLightImage = m_pGame->GetSystem()->GetIRenderer()->EF_LoadTexture("Textures/Lights/Light_testgrid", 0, FT2_FORCECUBEMAP, eTT_Cubemap);
	}
*/

//	m_pGame->GetSystem()->GetIRenderer()->EF_UpdateDLight(light);
	m_pGame->GetSystem()->GetI3DEngine()->AddDynamicLightSource(*m_pDynLight, GetEntity());

	if (m_pLightTarget)
	{
		ray_hit hit;
		Vec3d dir = ConvertToRadAngles(m_pEntity->GetAngles());
		dir*=20;
		int colliders = m_pGame->GetSystem()->GetIPhysicalWorld()->RayWorldIntersection(GetFLightPos(),dir,ent_all,0,&hit,1,m_pEntity->GetPhysics());
		if (colliders)
		{
			hit.pt.z +=float (rand()%100/1000);
			Vec3d newPos = hit.pt -m_pEntity->GetPos();
			// move it about 10 cm away from the point of collision
			float fDIST = newPos.Length();
			newPos *= 1.f - 0.1f/fDIST;
			m_pLightTarget->SetPos(m_pEntity->GetPos()+newPos);
		}
	}
}


//
//
void	CPlayer::UpdateLightBlinding()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	//return;
	if (m_pGame->m_PlayersWithLighs.empty() && 
			((IsMyPlayer()&&m_vBlindingList.empty()) ||	(m_bIsAI&&!m_stats.bIsBlinded)) )
		return;

	ListOfPlayers::iterator	pl = m_pGame->m_PlayersWithLighs.begin();

	Vec3d	tmp;
	Vec3d	thisForward;
	float	totalLightScale = m_pGame->GetSystem( )->GetI3DEngine()->GetAmbientLightAmountForEntity(m_pEntity);

	totalLightScale*=3.0f;


	if(totalLightScale>1.0f)
		totalLightScale=1.0f;
	//GetLightAmountForEntity( m_pEntity );

	//if( m_bIsAI )
	//m_pGame->GetSystem()->GetILog()->Log("\003 lScale %.3f", totalLightScale );


	GetFirePosAngles(tmp, thisForward);
	thisForward=ConvertToRadAngles(thisForward);
	thisForward.normalize();

	m_stats.curBlindingValue = 0.0f;

	for( ; pl!=m_pGame->m_PlayersWithLighs.end(); pl++ )
	{
		if( (*pl)==this )
			continue;
		CPlayer	*curPlayer = (*pl);
		Vec3d blindingPos = curPlayer->GetFLightPos();
		Vec3d	direction = blindingPos - m_vEyePos;
		Vec3d	curForward;
		float	blindingValue=0.0f;
		float	dist2 = 0;
		curPlayer->GetFirePosAngles(tmp, curForward);
		curForward=ConvertToRadAngles(curForward);
		curForward.normalize();
		dist2 = direction.len2();
		direction.normalize();

		blindingValue = direction*thisForward;

		if(blindingValue<0.0f)
			continue;

		direction = -direction;

		//dist2 = (direction*curForward);
		blindingValue = blindingValue*(direction*curForward);
		blindingValue *= blindingValue;
		blindingValue *= blindingValue;
		blindingValue *= blindingValue;
		//		blindingValue = blindingValue*blindingValue*blindingValue;

		dist2 = m_pGame->pl_dist->GetFVal()/dist2;

		if(dist2 > 1.0f)
			dist2 = cry_sqrtf(dist2);
		blindingValue *= dist2;

		blindingValue *= m_pGame->pl_intensity->GetFVal();

		blindingValue *= (1.0f - totalLightScale);

		if(blindingValue<.2f)
			continue;

		ray_hit hit;
		IPhysicalEntity *physic = curPlayer->m_pEntity->GetPhysics();
		if( m_pGame->GetSystem()->GetIPhysicalWorld()->
			RayWorldIntersection(blindingPos,m_vEyePos-blindingPos,ent_terrain | ent_static, 
			rwi_stop_at_pierceable,&hit,1, physic))
			continue;

		if(IsMyPlayer())
		{
			Vec3d	scrPos;
			GetGame()->GetSystem()->GetIRenderer()->ProjectToScreen( blindingPos.x, blindingPos.y, blindingPos.z,
				&scrPos.x,&scrPos.y,&scrPos.z);

			scrPos.z = blindingValue;
			scrPos.x *= 8.0f;
			scrPos.y *= 6.0f;
			//			m_vBlindingPosList.push_back(scrPos);
			BlindingList::iterator curB = m_vBlindingList.find(curPlayer);
			if( curB == m_vBlindingList.end() )
			{
				m_vBlindingList[curPlayer] = scrPos;
			}
			else
			{
				(curB->second).x = scrPos.x;
				(curB->second).y = scrPos.y;
				if((curB->second).z < scrPos.z)
					(curB->second).z = scrPos.z;
			}
		}

		if(m_bIsAI && m_stats.curBlindingValue < blindingValue)
		{
			m_stats.curBlindingValue = blindingValue;
		}
	}

	float fade = m_pTimer->GetFrameTime()*m_pGame->pl_fadescale->GetFVal();
	BlindingList::iterator curB = m_vBlindingList.begin();
	for( ; curB != m_vBlindingList.end(); )
	{
		(curB->second).z -= fade;
		if((curB->second).z<=0.0f)			// remove
		{
			BlindingList::iterator nextB = curB;
			nextB++;
			m_vBlindingList.erase( curB );
			curB = nextB;
			continue;
		}
		curB++; 
	}

	m_LastUsed = m_vBlindingList.begin();

	if( m_bIsAI )
	{
		IAIObject *pObject = m_pEntity->GetAI();
		IPuppet *pPuppet=0;
		if (pObject->CanBeConvertedTo(AIOBJECT_PUPPET,(void**)&pPuppet))
		{
			if(m_stats.curBlindingValue > 1.6f)
			{
				if(!m_stats.bIsBlinded)
				{
					pObject->SetSignal(0, "SHARED_BLINDED");
					m_stats.bIsBlinded = true;
				}
			}
			else 
			{	
				if(m_stats.bIsBlinded)
				{
					pObject->SetSignal(0, "SHARED_UNBLINDED");
					m_stats.bIsBlinded = false;
				}
			}
		}
	}

	//if(m_stats.curBlindingValue > .1f)
	//m_pGame->GetSystem()->GetILog()->Log("\003 blind %.3f", m_stats.curBlindingValue );
}


//
//
void	CPlayer::SwitchFlashLight( bool on )
{
	// only perform switch, if the player actually has a flashlight
	// or if switching off
	if (!m_stats.has_flashlight && !m_bLightOn)
		return;

	if(m_bLightOn == on)
		return;

	FUNCTION_PROFILER( GetISystem(),PROFILE_GAME );

	m_bLightOn = on;
	m_pEntity->SendScriptEvent(	ScriptEvent_FlashLightSwitch , (int)(m_bLightOn));
	if( m_bLightOn )
		m_pGame->m_PlayersWithLighs.push_back( this );
	else
	{
		ListOfPlayers::iterator	self = std::find(m_pGame->m_PlayersWithLighs.begin(), m_pGame->m_PlayersWithLighs.end(), this);
		if(self!=m_pGame->m_PlayersWithLighs.end())
			m_pGame->m_PlayersWithLighs.erase(self);
	}

	if (on && !m_bIsAI)
	{
		m_pLightTarget = m_pGame->GetSystem()->GetAISystem()->CreateAIObject(AIOBJECT_ATTRIBUTE,0);
		if (m_pLightTarget)
		{
			m_pLightTarget->Bind(m_pEntity->GetAI());
			m_pLightTarget->SetPos(m_pEntity->GetPos());
		}
	}
	else
	{	
		if (m_pLightTarget)
		{
			m_pGame->GetSystem()->GetAISystem()->RemoveObject(m_pLightTarget);
			m_pLightTarget = 0;
		}
	}
}

//
//--------------------------------------------------------------------------------------
bool	CPlayer::InitLight( const char* sImg, const char* sShader )
{
	if (m_pDynLight)
		delete m_pDynLight;
	m_pDynLight = new CDLight();

	if(sImg && sImg[0])
	{
    m_pDynLight->m_fAnimSpeed = 0;
    int nFlags2 = FT2_FORCECUBEMAP;
//    if (bUseAsCube)
//	    nFlags2 |= FT2_REPLICATETOALLSIDES;
//    if (fAnimSpeed)
//      nFlags2 |= FT2_CHECKFORALLSEQUENCES;
		m_pDynLight->m_pLightImage = m_pGame->GetSystem()->GetIRenderer()->EF_LoadTexture(sImg, 0, nFlags2, eTT_Cubemap);
		m_pDynLight->m_Flags = DLF_PROJECT;
	}
	else
		m_pDynLight->m_Flags = DLF_POINT;

	if(sShader && sShader[0])
		m_pDynLight->m_pShader = m_pGame->GetSystem()->GetIRenderer()->EF_LoadShader((char*)sShader, eSH_World);

	return true;
}

void	CPlayer::GiveFlashLight(bool val)
{
	if (m_stats.has_flashlight == val)
		return;

	m_stats.has_flashlight = val;
	
	// make sure we turn off the flashlight, if we take it away
	if (val == false && m_bLightOn)
	{
		SwitchFlashLight(false);
	}
}

float CPlayer::GetLightRadius()
{
	if(m_pDynLight && m_bLightOn)
		return m_pDynLight->m_fRadius;

	return 0;
}

//GetLightAmountForEntity

//
//-----------------------------------------------------------------
