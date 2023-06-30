
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// ScriptObjectPlayer.h: interface for the CScriptObjectPlayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTPLAYER_H__29A77DC8_C5A4_487C_943B_FF56EA8E01EE__INCLUDED_)
#define AFX_SCRIPTOBJECTPLAYER_H__29A77DC8_C5A4_487C_943B_FF56EA8E01EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>
#include <ScriptObjectVector.h>

class CPlayer;

/*! In this class are all player-related script-functions implemented.

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/

enum SOP_MEMBER_LUA_TABLES {
	SOP_MEMBER_TV_HELPER,
	SOP_MEMBER_SHAKE_AXIS,

	SOP_MEMBER_LAST
};

class CScriptObjectPlayer :
public _ScriptableEx<CScriptObjectPlayer>,
public IScriptObjectSink
{
public:
	CScriptObjectPlayer();
	virtual ~CScriptObjectPlayer();
	bool Create(IScriptSystem *pScriptSystem);
	void SetPlayer(CPlayer *pPlayer);
//IScriptObjectSink
	void OnRelease()
	{
		m_pScriptThis=NULL;
		delete this;
	}
	static void InitializeTemplate(IScriptSystem *pSS);
	static void ReleaseTemplate();
public:
	int RedirectInputTo(IFunctionHandler *pH);
	int DeselectWeapon(IFunctionHandler *pH);
	CPlayer * GetPlayer();
	int GetWeaponInfo(IFunctionHandler *pH);
	int GetWeaponsSlots(IFunctionHandler *pH);
	int CalculateAccuracyFactor(IFunctionHandler *pH);
	int WaitForFireRelease(IFunctionHandler *pH);
	//int SetWeaponInfo(IFunctionHandler *pH);
	int SetCurrWeapon(IFunctionHandler *pH);
	int SetSwayAmp(IFunctionHandler *pH);
	int SetSwayFreq(IFunctionHandler *pH);
	int GetCurrWeapon(IFunctionHandler *pH);
	int GetViewIntersection(IFunctionHandler *pH);
	int SetGravity(IFunctionHandler *pH);
	
	int SetAngleLimit(IFunctionHandler *pH);
	int SetAngleLimitH(IFunctionHandler *pH);
	int SetAngleLimitV(IFunctionHandler *pH);
	int GetAngleLimitH(IFunctionHandler *pH);
	int GetAngleLimitV(IFunctionHandler *pH);

	int SetAngleLimitBase(IFunctionHandler *pH);
	int SetMinAngleLimitV(IFunctionHandler *pH);
	int SetMaxAngleLimitV(IFunctionHandler *pH);
	int EnableAngleLimitV(IFunctionHandler *pH);
	int SetMinAngleLimitH(IFunctionHandler *pH);
	int SetMaxAngleLimitH(IFunctionHandler *pH);
	int EnableAngleLimitH(IFunctionHandler *pH);

	int SetName(IFunctionHandler *pH);
	int GetName(IFunctionHandler *pH);

  int MakeWeaponAvailable(IFunctionHandler *pH);


	int InitWeapons(IFunctionHandler *pH);
	int GetCurrWeaponId(IFunctionHandler *pH);
	int CalcDmgShakeAxis(IFunctionHandler *pH);
	int ShakeCamera(IFunctionHandler *pH);
	int SetCameraOffset(IFunctionHandler *pH);
	int GetCameraOffset(IFunctionHandler *pH);
//	int Die(IFunctionHandler *pH);
	int StartDie(IFunctionHandler *pH);
	int HasCollided(IFunctionHandler *pH);

	int SetDimNormal(IFunctionHandler *pH);
	int SetDimCrouch(IFunctionHandler *pH);
	int SetDimProne(IFunctionHandler *pH);

	int GetBoneHitZone(IFunctionHandler *pH);
	int GetArmDamage(IFunctionHandler *pH);
	int GetLegDamage(IFunctionHandler *pH);

	int SetMoveParams(IFunctionHandler *pH);
	int HolsterGun(IFunctionHandler *pH);
	int HoldGun(IFunctionHandler *pH);

	int GetColor(IFunctionHandler *pH);
	/*
	int AllignOnSurface(IFunctionHandler *pH);
	int SetAngleLimitBaseOnEnviroment(IFunctionHandler *pH);
	int SetAngleLimitBaseOnVertical(IFunctionHandler *pH);
	int SetRunSpeed(IFunctionHandler *pH);
	int SetWalkSpeed(IFunctionHandler *pH);
	int SetCrouchSpeed(IFunctionHandler *pH);
	int SetProneSpeed(IFunctionHandler *pH);
	int SetJumpForce(IFunctionHandler *pH);
	int SetLean(IFunctionHandler *pH);
	int SetCameraBob(IFunctionHandler *pH);
	int SetWeaponBob(IFunctionHandler *pH);*/

	int SetDynamicsProperties(IFunctionHandler *pH);

	int SetSmoothInput(IFunctionHandler *pH);

	int GetTreadedOnMaterial(IFunctionHandler *pH);
	int GetTouchedMaterial(IFunctionHandler *pH);

	int	GetTPVHelper(IFunctionHandler *pH);
	int	GetHelperPos(IFunctionHandler *pH);
	
	int	GetTargetScreenPos(IFunctionHandler *pH);
	int	GetTargetTime(IFunctionHandler *pH);
	
	int ShakeCameraL(IFunctionHandler *pH);

	int SelectFirstWeapon(IFunctionHandler *pH);

	int GetCurVehicle(IFunctionHandler *pH);

//	int SetAnimationRefSpeed(IFunctionHandler *pH);
	int	SetAnimationRefSpeedRun(IFunctionHandler *pH);
	int	SetAnimationRefSpeedWalkRelaxed(IFunctionHandler *pH);
	int	SetAnimationRefSpeedWalk(IFunctionHandler *pH);
	int	SetAnimationRefSpeedXRun(IFunctionHandler *pH);
	int	SetAnimationRefSpeedXWalk(IFunctionHandler *pH);
	int	SetAnimationRefSpeedCrouch(IFunctionHandler *pH);
	int DrawThirdPersonWeapon(IFunctionHandler *pH);

	int SetDimOverride(IFunctionHandler *pH);

	int CounterAdd(IFunctionHandler *pH);
	int CounterIncrement(IFunctionHandler *pH);
	int CounterGetValue(IFunctionHandler *pH);
	int CounterSetValue(IFunctionHandler *pH);
	int CounterSetEvent(IFunctionHandler *pH);
	int GetCharacterAngles(IFunctionHandler *pH);

	int SwitchFlashLight(IFunctionHandler *pH);
	int GiveFlashLight(IFunctionHandler *pH);
	int GiveBinoculars(IFunctionHandler *pH);
	int IsSwimming(IFunctionHandler *pH);
	int GetBlindScreenPos(IFunctionHandler *pH);

//
//-----------------------------------------------------------------------------------------------
//	for debug/test purposes
	int StartFire(IFunctionHandler *pH);
	int ClearFire(IFunctionHandler *pH);

	int PlaySound(IFunctionHandler *pH);

	int GetFirePosAngles(IFunctionHandler *pH);
	int SetPivot(IFunctionHandler *pH);
	int	SetHeatVisionValues(IFunctionHandler *pH);
	int	SetBlendTime(IFunctionHandler *pH);		// sets blend time for particular animation

	// makes some player stats persistent
	int SavePlayerElements(IFunctionHandler *pH);
	int LoadPlayerElements(IFunctionHandler *pH);

private:
	CPlayer *m_pPlayer;
	static IScriptObject *m_pTempObj;
	static IScriptObject *m_pTempAng;
	static IScriptObject *m_pWeaponSlots;
	static IScriptObject *m_pTempBloodObj;
	static IScriptObject *m_pBlindScreenPos;

	CScriptObjectVector m_pCameraOffset;
	CScriptObjectVector m_pGetColor;
	_SmartScriptObject m_pWeaponInfo;

	// member script objects (preallocated)
	static IScriptObject* m_memberSO[SOP_MEMBER_LAST];
	void SetMemberVector( SOP_MEMBER_LUA_TABLES member,const Vec3 &vec );

	int		m_LastTouchedMaterialID;

	float m_fSpeedRun;
	float m_fSpeedWalk;
	float m_fSpeedCrouch;
	float m_fSpeedProne;

public:
	int SelectNextWeapon(IFunctionHandler * pH);
	int SetAISpeedMult(IFunctionHandler *pH);
	int InitDynamicLight(IFunctionHandler *pH);

	int InitStaminaTable(IFunctionHandler *pH);
	int GetProjectedBloodPos(IFunctionHandler *pH);
	IEntityRender * GetIEntityRender(const struct pe_params_foreign_data & fd);
	int UseLadder(IFunctionHandler *pH);
	int GetCrosshairState(IFunctionHandler *pH);
	int ResetCamera(IFunctionHandler *pH);
	int ResetRotateHead(IFunctionHandler *pH);
	int CanStand(IFunctionHandler *pH);
};

#endif // !defined(AFX_SCRIPTOBJECTPLAYER_H__29A77DC8_C5A4_487C_943B_FF56EA8E01EE__INCLUDED_)
