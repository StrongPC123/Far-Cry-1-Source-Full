
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef SCRIPTOBJECTWEAPONCLASS_H__
#define SCRIPTOBJECTWEAPONCLASS_H__

#include <IScriptSystem.h>
#include <IEntitySystem.h>
#include <_ScriptableEx.h>

class CWeaponClass;
struct WeaponParams;

class CScriptObjectFireParam :
	public _ScriptableEx<CScriptObjectFireParam>
{
public:
	CScriptObjectFireParam();
	virtual ~CScriptObjectFireParam();
	bool Create(IScriptSystem *pScriptSystem,WeaponParams *p);
	static void InitializeTemplate(IScriptSystem *pSS);
	static void ReleaseTemplate();
private:
	WeaponParams *m_pWeaponParams;
};


class CScriptObjectWeaponClass :
	public _ScriptableEx<CScriptObjectWeaponClass>,
	public IScriptObjectSink 
{
public:
	CScriptObjectWeaponClass();
	virtual ~CScriptObjectWeaponClass();

	bool Create(CXGame* pGame, CWeaponClass* pWeaponClass);

	//IScriptObjectSink
	void OnRelease()
	{
		m_pScriptThis->Clear();
		m_pScriptThis=NULL;
		delete this;
	}

	static void InitializeTemplate(IScriptSystem *pSS);
	static void ReleaseTemplate();

	int SetName(IFunctionHandler *pH);
	int SetShaderFloat(IFunctionHandler *pH);
	int SetBindBone(IFunctionHandler *pH);
	int SetAnimationKeyEvent(IFunctionHandler *pH);
	int StartAnimation(IFunctionHandler *pH);
	int ResetAnimation(IFunctionHandler *pH);
	int GetAnimationLength(IFunctionHandler *pH);
	int GetCurAnimation(IFunctionHandler *pH);
	int IsAnimationRunning(IFunctionHandler *pH);
	int GetPos(IFunctionHandler *pH);

	int GetBonePos(IFunctionHandler *pH);
	int GetInstantHit(IFunctionHandler *pH);
	int GetProjectileFiringAngle(IFunctionHandler *pH);
	int Hit(IFunctionHandler *pH);
	int SetFirstPersonWeaponPos(IFunctionHandler *pH);
	int SetHoldingType(IFunctionHandler *pH);
	int SetWeaponFireParams(IFunctionHandler *pH);

	int LoadObject(IFunctionHandler *pH);
	int AttachObjectToBone(IFunctionHandler *pH);
	int DetachObjectToBone(IFunctionHandler *pH);

	int CacheObject(IFunctionHandler *pH);

	int DrawScopeFlare(IFunctionHandler *pH);
	int CalcFlareIntensity(IFunctionHandler *pH);
public:

private:
	typedef std::vector<CScriptObjectFireParam*> FireParamsObjVec;
	typedef FireParamsObjVec::iterator FireParamsObjVecItor;
	static void GetDirection(IEntity *pEntity, Vec3& vDir);

	FireParamsObjVec m_vFireParams;

	static IScriptObject *m_pMemberBonePos;

	CWeaponClass*	m_pWeaponClass;
	CXGame*				m_pGame;
	ISystem*			m_pSystem;
};

#endif  //SCRIPTOBJECTWEAPONCLASS_H__
