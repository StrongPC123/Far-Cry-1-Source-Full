
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// XPuppetProxy.h: interface for the CXPuppetProxy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XPUPPETPROXY_H__79A09EA6_D523_43B3_804F_DA16597D8AB6__INCLUDED_)
#define AFX_XPUPPETPROXY_H__79A09EA6_D523_43B3_804F_DA16597D8AB6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <IAgent.h>
#include <string>

#include "aihandler.h"


struct IPuppet;
struct IScriptSystem;
class CPlayer;
class CXGame;

#define HEAD_TORSO_MAX 0.8f
#define TORSO_BODY_MAX 0.6f

#define OBJECTBEHAVIOUR_HELICOPTER	30
#define OBJECTBEHAVIOUR_AIRPLANE  	50

#define JUMP_TAKEOFF	0xffff
#define JUMP_LAND		0xff00

typedef struct StartupAnimFlags
{
	bool bMoving;
	bool bMovePending;

	StartupAnimFlags()
	{
		bMoving = false;
		bMovePending = false;
	}
} StartupAnimFlags;

class CXPuppetProxy : public IPuppetProxy, public ICharInstanceSink
{
	CAIHandler	m_AIHandler;
	StartupAnimFlags	m_SAF;
// move this to CPlayer
//	bool m_bIsFish;

	IPhysicalEntity *m_pCharacterPhysics;

	IEntity *m_pEntity;
	CPlayer *m_pPlayer;

	bool m_bFireOverride;
	bool m_bWasSwimming;
	bool m_bAllowedToMove;

	Vec3d	 m_vBodyDir;
	Vec3d	 m_vTorsoDir;
	Vec3d	 m_vHeadDir;
	Vec3d	 m_vPreviousTargetPos;

	int		m_nWeaponBoneID;
	int		m_nLastSelectedJumpAnim;

	bool m_bInJump;

	_HScriptFunction	m_hBehaviourFunc;
	_HScriptFunction	m_hMotorFunc;
	_HScriptFunction	m_hSignalFunc;

	string	m_strRootBone;	

	IScriptSystem *m_pScriptSystem;
	bool	m_bDead;

	float m_fForwardSpeed;
	float m_fBackwardSpeed;

	float m_fMovementRestrictionDuration;

	float m_fLastJumpDuration;
	int	  m_nJumpDirection;
	Vec3d m_vJumpVector;

	pe_player_dimensions	m_dimStandingPuppet;
	pe_player_dimensions	m_dimCrouchingPuppet;
	pe_player_dimensions	m_dimProningPuppet;

	pe_player_dimensions	m_dimCurrentDimensions;
	int	m_nLastBodyPos;

	SOBJECTSTATE	m_LastObjectState;
	CXGame * m_pGame;

	_SmartScriptObject pSignalTable;
	_SmartScriptObject pJumpTable;
	
	void *m_pAISystem;//!< point to AI system , needed to check if AI is inside indoor or not.

/*	char	m_MutantJumpStage;
	Vec3d	m_vMJVelocity;
	float	m_MJAniLength;
	float	m_MJTime;
	float	m_MJTimeTotal;
	float	m_MJTimeFly;
	float	m_MJVertA;
	float	m_MJVertV;
	float	m_MJVertPos;
	char	m_MJAnimation;
	*/
	
public:
	
	void SetFireOverride() { m_bFireOverride = true;}
	void SetPuppetDimensions(float height, float eye_height, float sphere_height, float radius);
	void SetSpeeds(float fwd, float bkw);
	void SetRootBone(const char *pRootBone);
	CXPuppetProxy(IEntity *pEntity,IScriptSystem *pScriptSystem, CXGame *pGame);
	virtual ~CXPuppetProxy();

	int Update(SOBJECTSTATE *state);
	void Release() { delete this; }

//	bool	IsBound() { return m_pEntity->IsBound(); }

	IPhysicalEntity* GetPhysics() 
	{ 
		return m_pEntity->GetPhysics();
	}
	IEntity*	GetEntity() 
	{ 
		return m_pEntity;
	}

	void OnStartAnimation(const char *sAnimation){}
	void OnAnimationEvent(const char *sAnimation,AnimSinkEventData UserData);
	void OnEndAnimation(const char *sAnimation);


	void SetSignalFunc(HSCRIPTFUNCTION pFunc);
	void SetBehaviourFunc(HSCRIPTFUNCTION pFunc);
	void SetMotorFunc(HSCRIPTFUNCTION pFunc);

	bool QueryProxy(unsigned char type, void **pProxy);

	void GetDimensions(int bodypos, float &eye_height, float &height);

	bool CheckStatus(unsigned char status);

protected:

	void UpdateMind(SOBJECTSTATE *state);
	int UpdateMotor(SOBJECTSTATE *state);

//	void	MutantJumpTurn( const Vec3d& target );
//	bool	MutantJumpInit( const Vec3d& target );
//	bool	MutantJumpProceed(  );

public:
	
	void Reset(void);
	void SendSignal(int signalID, const char * szText,IEntity *pSender);
	void SendAuxSignal(int signalID, const char * szText);
	bool CustomUpdate(Vec3d & vPos, Vec3d & vAngles);
	void ApplyHealth(float fHealth);



	void DebugDraw(struct IRenderer * pRenderer);
	void ApplyMovement(pe_action_move * move_params, SOBJECTSTATE *state);
	void MovementControl(bool bEnableMovement, float fDuration) { m_bAllowedToMove = bEnableMovement;
																  m_fMovementRestrictionDuration = fDuration;}
	void Save(CStream &str);
	void Load(CStream &str);
	void Load_PATCH_1(CStream &str);
};

#endif // !defined(AFX_XPUPPETPROXY_H__79A09EA6_D523_43B3_804F_DA16597D8AB6__INCLUDED_)
