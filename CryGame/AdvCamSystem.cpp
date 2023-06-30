
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <IEntitySystem.h>
#include "Game.h"
#include "AdvCamSystem.h"										// CAdvCamSystem
#include "XPlayer.h"												// CPlayer
#include "Cry_Math.h"											// TMatrix_tpl
#include "XEntityProcessingCmd.h"						// CXEntityProcessingCmd for ProcessKeys

CAdvCamSystem::CAdvCamSystem(CXGame *pGame)
{
	m_pScriptObject=NULL;
	m_pGame=pGame;
	m_eiPlayerA=INVALID_WID;
	m_eiPlayerB=INVALID_WID;
}

//////////////////////////////////////////////////////////////////////////
// Initialize the AdvCamSystem-container.
bool CAdvCamSystem::Init()
{
	IEntity *entity = GetEntity();
	entity->GetScriptObject()->SetValue("type", "advcamsystem");
	// set camera
	entity->SetCamera(m_pGame->GetSystem()->GetIEntitySystem()->CreateEntityCamera());
	entity->GetCamera()->GetCamera().Init(m_pGame->GetSystem()->GetIRenderer()->GetWidth(),m_pGame->GetSystem()->GetIRenderer()->GetHeight());
	entity->SetNeedUpdate(true);

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Update the camera.
void CAdvCamSystem::Update( void )
{
	//the server doesn't need to update AdvCamSystem
	if(!m_pGame->IsClient())
		return;

	Vec3 vSrcPos, vDstPos;
	Vec3 vSrcPos2, vDstPos2;

	if(m_pScriptObject)
	{
		_SmartScriptObject sm(m_pGame->GetScriptSystem(),true);
		_SmartScriptObject current(m_pGame->GetScriptSystem(),true);
		_SmartScriptObject future(m_pGame->GetScriptSystem(),true);

		// get the current state
		int currentState = 0;
		int futureState = 0;
		float blendFactor = 0;
		m_pEntity->GetScriptObject()->GetValue("currentState", currentState);
		m_pEntity->GetScriptObject()->GetValue("futureState", futureState);
		m_pEntity->GetScriptObject()->GetValue("blendFactor", blendFactor);
		m_pEntity->GetScriptObject()->GetValue("states", sm);
		sm->GetAt(currentState, current);
		sm->GetAt(futureState, future);

		CalcCameraVectors(current, vSrcPos, vDstPos);
		CalcCameraVectors(future, vSrcPos2, vDstPos2);

		vSrcPos = (1.0f - blendFactor) * vSrcPos + blendFactor * vSrcPos2;
		vDstPos = (1.0f - blendFactor) * vDstPos + blendFactor * vDstPos2;

		float currentRadius;
		float futureRadius;

		current->GetValue("max_radius", currentRadius);
		future->GetValue("max_radius", futureRadius);
		m_fMaxRadius = (1.0f - blendFactor) * currentRadius + blendFactor * futureRadius;
		current->GetValue("min_radius", currentRadius);
		future->GetValue("min_radius", futureRadius);
		m_fMinRadius = (1.0f - blendFactor) * currentRadius + blendFactor * futureRadius;
	}



	Vec3 vDir=vDstPos-vSrcPos;

	if (vDir.Length() < m_fMinRadius)
	{
		vDir.Normalize();
		vSrcPos = vDstPos - vDir * m_fMinRadius;
	}
	Ang3 vAngles=ConvertVectorToCameraAnglesSnap180(vDir);

	m_pEntity->SetAngles(vAngles);
	m_pEntity->SetPos(vSrcPos);


	IEntity				*pEntity	=GetEntity();
	IEntityCamera *pEC			=pEntity->GetCamera();
	Vec3					v3Angles	=pEntity->GetAngles();

	if(pEC)
	{
			pEC->SetAngles(v3Angles);

			pEC->SetPos(pEntity->GetPos());		// swing up and down - just for testing
	}
}

void CAdvCamSystem::OnSetAngles( const Vec3d &ang )
{
	/*if(m_pGame->m_pClient->GetPlayerID()==GetEntity()->GetId() || m_pGame->m_pClient->m_bLocalHost)
		m_pGame->m_pClient->m_PlayerProcessingCmd.SetDeltaAngles(ang);*/

}


IScriptObject *CAdvCamSystem::GetScriptObject()
{
	return m_pScriptObject;
}

void CAdvCamSystem::SetScriptObject(IScriptObject *object)
{
	m_pScriptObject=object;
}


//////////////////////////////////////////////////////////////////////////
// Save upcast.
bool CAdvCamSystem::QueryContainerInterface(ContainerInterfaceType desired_interface, void **ppInterface )
{

	if (desired_interface == CIT_IADVCAMSYSTEM)
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

void CAdvCamSystem::GetEntityDesc( CEntityDesc &desc ) const
{

}


//////////////////////////////////////////////////////////////////////////
// Process input.
void CAdvCamSystem::ProcessKeys( CXEntityProcessingCmd &epc )
{
	if(m_eiPlayerA && m_eiPlayerB)
	{
		// get entities
		IEntitySystem *pEntitySystem=m_pGame->m_pSystem->GetIEntitySystem();	assert(pEntitySystem);
		IEntity *pEntityA = pEntitySystem->GetEntity(m_eiPlayerA);						assert(pEntityA);if(!pEntityA)return;
		IEntity *pEntityB = pEntitySystem->GetEntity(m_eiPlayerB);						assert(pEntityB);if(!pEntityB)return;
		
		// get player containers of the entities
		CPlayer *pPlayerA = NULL;
		pEntityA->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayerA);		assert(pPlayerA);
		CPlayer *pPlayerB = NULL;
		pEntityB->GetContainer()->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayerB);		assert(pPlayerB);

		// determine which direction we are moving in
		// build camera matrix
		Vec3 vCamAngles;
		Matrix33 matCamera;
		vCamAngles	= m_pEntity->GetAngles(); 
		matCamera.SetRotationAA(gf_DEGTORAD*vCamAngles.z, Vec3(0,0,1));
		Matrix33 matPlayerA;
		vCamAngles	= pEntityA->GetAngles(); 
		matPlayerA.SetRotationAA(gf_DEGTORAD*vCamAngles.z, Vec3(0,0,1));
		Matrix33 matPlayerB;
		vCamAngles	= pEntityB->GetAngles(); 
		matPlayerB.SetRotationAA(gf_DEGTORAD*vCamAngles.z, Vec3(0,0,1));

		// we have to make a copy, because the function is called twice per frame. We modify the actions in the epc, so this
		// would cause side effects the second time running through the function.
		CXEntityProcessingCmd epcCopyA = epc;
		CXEntityProcessingCmd epcCopyB = epc;

		_SmartScriptObject sm(m_pGame->GetScriptSystem(),true);
		_SmartScriptObject current(m_pGame->GetScriptSystem(),true);
		int currentState = 0;
		float blendFactor = 0;
		m_pEntity->GetScriptObject()->GetValue("currentState", currentState);
		m_pEntity->GetScriptObject()->GetValue("states", sm);
		sm->GetAt(currentState, current);

		bool linked_a;

		current->GetValue("linked_a", linked_a);

		Vec3	vMoveDirA;
		
		if (linked_a)
			vMoveDirA = CalcPlayerMoveDirection(matCamera, 0);
		else
			vMoveDirA = CalcPlayerMoveDirection(matCamera, 0);

		Vec3	vMoveDirB = CalcPlayerMoveDirection(matCamera, 1);

		// vector from player A to player B
		Vec3 vToB = pEntityB->GetPos()-pEntityA->GetPos();

		// if player a is outside the follow radius, we move into the direction of player b
		float fDistance = vToB.Length();
		if (fDistance > m_fMaxRadius)
		{
			float weight = (fDistance - m_fMaxRadius)/10.0f;
			//vToB.Normalize();

			if (GetLengthSquared(vMoveDirB) > 0.01)
			{
				vMoveDirA += (weight)*vToB;
			}
			else if (GetLengthSquared(vMoveDirA) > 0.01)
			{
				vMoveDirB += -(weight)*vToB;
			}
		}

		// get input system
		ISystem *pSystem = m_pGame->GetSystem();
		IInput *pInput = pSystem->GetIInput();

		if (pInput->JoyButtonPressed(7))
		{
			bool isBlending;
			m_pEntity->GetScriptObject()->GetValue("isBlending", isBlending);
			if (!isBlending)
			{
				int currentState;
				m_pEntity->GetScriptObject()->GetValue("currentState", currentState);

				if (currentState == 8)
				{
					m_pEntity->GetScriptObject()->SetValue("isBlending", true);
					m_pEntity->GetScriptObject()->SetValue("futureState", 9);
				}
			}
		}
		if (pInput->JoyButtonPressed(6))
		{
			bool isBlending;
			m_pEntity->GetScriptObject()->GetValue("isBlending", isBlending);
			if (!isBlending)
			{
				int currentState;
				m_pEntity->GetScriptObject()->GetValue("currentState", currentState);

				if (currentState == 8)
				{
					m_pEntity->GetScriptObject()->SetValue("isBlending", true);
					m_pEntity->GetScriptObject()->SetValue("futureState", 10);
				}
			}
		}
		if (!pInput->JoyButtonPressed(6) && !pInput->JoyButtonPressed(7))
		{
			bool isBlending;
			m_pEntity->GetScriptObject()->GetValue("isBlending", isBlending);
			if (!isBlending)
			{
				int currentState;
				m_pEntity->GetScriptObject()->GetValue("currentState", currentState);

				if (currentState != 8)
				{
					m_pEntity->GetScriptObject()->SetValue("isBlending", true);
					m_pEntity->GetScriptObject()->SetValue("futureState", 8);
				}
			}
		}

		// hack begin
		m_pGame->m_pClient->SetPlayerID(pPlayerA->GetEntity()->GetId());
		if (GetLengthSquared(vMoveDirA) > 1.0f)
			vMoveDirA.Normalize();

		m_pGame->m_pSystem->GetIScriptSystem()->SetGlobalValue("p_speed_run", vMoveDirA.Length()*5.0f);
		// hack end

		SetMoveDirection(*pPlayerA, *pEntityA, epcCopyA, vMoveDirA, linked_a);

		// hack begin
		m_pGame->m_pClient->SetPlayerID(pPlayerB->GetEntity()->GetId());
		if (GetLengthSquared(vMoveDirB) > 1.0f)
			vMoveDirB.Normalize();

		m_pGame->m_pSystem->GetIScriptSystem()->SetGlobalValue("p_speed_run", vMoveDirB.Length()*5.0f);
		// hack end

		SetMoveDirection(*pPlayerB, *pEntityB, epcCopyB, vMoveDirB, true);

		// hack
		m_pGame->m_pClient->SetPlayerID(GetEntity()->GetId());
	}
}

bool CAdvCamSystem::Write(CStream &stm,EntityCloneState *cs)
{
	if(!stm.Write(m_eiPlayerA)) return false;
	if(!stm.Write(m_eiPlayerB)) return false;

	return true;
}

bool CAdvCamSystem::Read(CStream &stm)
{
	if(!stm.Read(m_eiPlayerA)) return false;
	if(!stm.Read(m_eiPlayerB)) return false;

	return true;
}

void CAdvCamSystem::SetMinRadius(float radius)
{
	m_fMinRadius = radius;
}

void CAdvCamSystem::SetMaxRadius(float radius)
{
	m_fMaxRadius = radius;
}

Vec3 CAdvCamSystem::CalcPlayerMoveDirection(const Matrix33 &matCamera, unsigned int joyID) const
{
	Vec3 vMoveDir;

	vMoveDir.Set(0,0,0);

	// get input system
	ISystem *pSystem = m_pGame->GetSystem();
	assert(pSystem); if (!pSystem) return vMoveDir;
	IInput *pInput = pSystem->GetIInput();
	assert(pInput); if (!pInput) return vMoveDir;

	Vec3 vControllerDir = (pInput)->JoyGetAnalog1Dir(joyID);

	vMoveDir.x = vControllerDir.y * matCamera(0, 1) - vControllerDir.x * matCamera(0,0);
	vMoveDir.y = vControllerDir.y * matCamera(1, 1) - vControllerDir.x * matCamera(1,0);

	return vMoveDir;
}

void CAdvCamSystem::CalcCameraVectors(_SmartScriptObject &scriptObject, Vec3& vSrcPos, Vec3& vDstPos)
{
	CScriptObjectVector oVec(m_pGame->GetScriptSystem(),true);

	Vec3 src_a_offset(0, 0, 0);
	float	src_origin = 0.0f;
	Vec3	src_b_offset(0, 0, 0);
	Vec3	src_ab_offset_m(0, 0, 0);
	Vec3	src_ab_offset_fac(0, 0, 0);

	Vec3	dst_a_offset(0, 0, 0);
	float	dst_origin = 1.0f;
	Vec3	dst_b_offset(0, 0, 0);
	Vec3	dst_ab_offset_m(0, 0, 0);
	Vec3	dst_ab_offset_fac(0, 0, 0);

	#define GETVAL(Name)	{ scriptObject->GetValue(#Name,Name); }
	#define GETVEC(Name)	{ if(scriptObject->GetValue(#Name,*oVec))Name=oVec.Get(); }

	GETVEC(src_a_offset)
	GETVAL(src_origin)
	GETVEC(src_b_offset)
	GETVEC(src_ab_offset_m)
	GETVEC(src_ab_offset_fac)

	GETVEC(dst_a_offset)
	GETVAL(dst_origin)
	GETVEC(dst_b_offset)
	GETVEC(dst_ab_offset_m)
	GETVEC(dst_ab_offset_fac)

	vSrcPos.Set(0, 0, 0);
	vDstPos.Set(0, 0, 0);

	Vec3 vA(0,0,0),vB(0,0,0);
	Matrix33 baseA_m, baseB_m, baseAB_m, baseAB_fac;

	baseA_m.SetIdentity(); baseB_m.SetIdentity(); baseAB_m.SetIdentity(); baseAB_fac.SetIdentity();

	if(m_eiPlayerA)
	{
		IEntitySystem *pEntitySystem=m_pGame->m_pSystem->GetIEntitySystem();								assert(pEntitySystem);
		IEntity *pEntity=pEntitySystem->GetEntity(m_eiPlayerA);															assert(pEntity);if(!pEntity)return;

		vA=pEntity->GetPos();

		Ang3 vAnglesA=pEntity->GetAngles();

		baseA_m.SetRotationAA(gf_DEGTORAD*vAnglesA.z,Vec3(0,0,1));
	}

	if(m_eiPlayerB)
	{
		IEntitySystem *pEntitySystem=m_pGame->m_pSystem->GetIEntitySystem();								assert(pEntitySystem);
		IEntity *pEntity=pEntitySystem->GetEntity(m_eiPlayerB);															assert(pEntity);if(!pEntity)return;

		vB=pEntity->GetPos();

		Ang3 vAnglesB=pEntity->GetAngles();

		baseB_m.SetRotationAA(gf_DEGTORAD*vAnglesB.z,Vec3(0,0,1));
	}

	{
		Vec3 vA2B=vB-vA;
		Vec3 vCross=vA2B^Vec3(0.0f,0.0f,1.0f);

		baseAB_fac(0,0)=vCross.x;
		baseAB_fac(1,0)=vCross.y;
		baseAB_fac(2,0)=vCross.z;
		baseAB_fac(0,1)=vA2B.x;
		baseAB_fac(1,1)=vA2B.y;
		baseAB_fac(2,1)=vA2B.z;
		baseAB_fac(0,2)=0.0f;
		baseAB_fac(1,2)=0.0f;
		baseAB_fac(2,2)=1.0f;
	}

	{
		Vec3 vA2B=vB-vA;		vA2B.Normalize();
		Vec3 vCross=vA2B^Vec3(0.0f,0.0f,1.0f);

		baseAB_m(0,0)=vCross.x;
		baseAB_m(1,0)=vCross.y;
		baseAB_m(2,0)=vCross.z;
		baseAB_m(0,1)=vA2B.x;
		baseAB_m(1,1)=vA2B.y;
		baseAB_m(2,1)=vA2B.z;
		baseAB_m(0,2)=0.0f;
		baseAB_m(1,2)=0.0f;
		baseAB_m(2,2)=1.0f;
	}

	vSrcPos+= vB*src_origin+vA*(1.0f-src_origin);
	vSrcPos+= baseA_m*src_a_offset;
	vSrcPos+= baseB_m*src_b_offset;
	vSrcPos+= baseAB_m*src_ab_offset_m;
	vSrcPos+= baseAB_fac*src_ab_offset_fac;

	vDstPos += vB*dst_origin+vA*(1.0f-dst_origin);
	vDstPos += baseA_m*dst_a_offset;
	vDstPos += baseB_m*dst_b_offset;
	vDstPos += baseAB_m*dst_ab_offset_m;
	vDstPos += baseAB_fac*dst_ab_offset_fac;
}

void CAdvCamSystem::SetMoveDirection(CPlayer &player, const IEntity &entity, CXEntityProcessingCmd &epc, const Vec3 &vMoveDir, bool linked) const
{
	// we always move forward, so clear the processing command
	epc.Reset();

	if (linked)
	{
		if (GetLengthSquared(vMoveDir) > 0.01f)
		{
			Ang3 vAngles;
			epc.AddAction(ACTION_MOVE_FORWARD);
			vAngles=ConvertVectorToCameraAnglesSnap180(vMoveDir);
			epc.SetDeltaAngles(Vec3(0, 0, vAngles.z));
		}
		else
		{
			epc.SetDeltaAngles(entity.GetAngles());
		}
	}
	else
	{
		// get direction from other controller
		ISystem *pSystem = m_pGame->GetSystem();
		IInput *pInput = pSystem->GetIInput();

		Vec3 vMoveDir2 = pInput->JoyGetAnalog1Dir(0);
		if (GetLengthSquared(vMoveDir2) > 0.01f)
		{
			if (vMoveDir2.y < -0.1f)
				epc.AddAction(ACTION_MOVE_FORWARD);
			else if (vMoveDir2.y > 0.1f)
				epc.AddAction(ACTION_MOVE_BACKWARD);
		}
		Vec3 vControllerDir = pInput->JoyGetAnalog2Dir(0);
		Ang3 vAngles = entity.GetAngles();

		if (fabs(vMoveDir2.x) > 0.1)
			vAngles.z -= vMoveDir2.x*4.0f;

		epc.SetDeltaAngles(Vec3(0, 0, vAngles.z));
	}

	player.ProcessCmd(0, epc);		// first parameter is not used
	player.ProcessAngles(epc);
}