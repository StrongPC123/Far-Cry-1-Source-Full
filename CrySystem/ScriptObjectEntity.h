// ScriptObjectEntity.h: interface for the CScriptObjectEntity class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCRIPTOBJECTENTITY_H__870B7388_021A_46C0_84B4_734EEB08DE7D__INCLUDED_)
#define AFX_SCRIPTOBJECTENTITY_H__870B7388_021A_46C0_84B4_734EEB08DE7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <IScriptSystem.h>
#include <_ScriptableEx.h>

#define PHYSICPARAM_PARTICLE				0x00000001
#define PHYSICPARAM_VEHICLE					0x00000002
#define PHYSICPARAM_PLAYERDYN				0x00000003
#define PHYSICPARAM_PLAYERDIM				0x00000004
#define PHYSICPARAM_SIMULATION			0x00000005
#define PHYSICPARAM_ARTICULATED			0x00000006
#define PHYSICPARAM_JOINT						0x00000007
#define PHYSICPARAM_ROPE						0x00000008
#define PHYSICPARAM_BUOYANCY				0x00000009
#define PHYSICPARAM_CONSTRAINT			0x0000000A
#define PHYSICPARAM_REMOVE_CONSTRAINT 0x00000B
#define PHYSICPARAM_FLAGS						0x0000000C
#define PHYSICPARAM_WHEEL						0x0000000D
#define PHYSICPARAM_SOFTBODY				0x0000000E
#define PHYSICPARAM_VELOCITY				0x0000000F
#define PHYSICPARAM_PART_FLAGS			0x00000010

#define AIPARAM_SIGHTRANGE		1
#define AIPARAM_ATTACKRANGE		2
#define AIPARAM_ACCURACY		3
#define AIPARAM_AGGRESION		4
#define AIPARAM_GROUPID			5
#define AIPARAM_SOUNDRANGE		6
#define AIPARAM_FOV				7
#define AIPARAM_COMMRANGE		8
#define AIPARAM_FWDSPEED		9
#define AIPARAM_RESPONSIVENESS	10
#define AIPARAM_SPECIES			11

struct IEntity;
struct ISystem;
class CScriptObjectVector;
#include <IPhysics.h>

#define USE_MEMBER_POS
#define ENTITYPROP_CASTSHADOWS		0x00000001
#define ENTITYPROP_DONOTCHECKVIS	0x00000002

enum SOE_MEMBER_LUA_TABLES {
	SOE_MEMBER_HELPER_POS,
	SOE_MEMBER_BONE_POS,
	SOE_MEMBER_BONE_DIR,
	SOE_MEMBER_OBJ_VEL,

	SOE_MEMBER_LAST
};

/*! In this class are all entity-related script-functions implemented in order tos expose all functionalities provided by an entity.

	IMPLEMENTATIONS NOTES:
	These function will never be called from C-Code. They're script-exclusive.
*/
class CScriptObjectEntity :
public _ScriptableEx<CScriptObjectEntity>,
public IScriptObjectSink
{
public:
	CScriptObjectEntity();
	virtual ~CScriptObjectEntity();
	bool Create(IScriptSystem *pScriptSystem, ISystem *pSystem);
	void SetEntity(IEntity *pEntity);
	void SetContainer(IScriptObject *pContainer);
//IScriptObjectSink
	void OnRelease()
	{
		m_pScriptThis->Clear();
		m_pScriptThis=NULL;
		delete this;
	}
	static void InitializeTemplate(IScriptSystem *pSS);
	static void ReleaseTemplate();
public:
	int SelectPipe(IFunctionHandler *pH);
	int EnableUpdate(IFunctionHandler *pH);
	int SetUpdateIfPotentiallyVisible(IFunctionHandler *pH);
	int SetUpdateType(IFunctionHandler *pH);
	int SetBBox(IFunctionHandler *pH);
	int GetBBox(IFunctionHandler *pH);
	int GetLocalBBox(IFunctionHandler *pH);
	int SetRadius(IFunctionHandler *pH);
	int SetUpdateRadius(IFunctionHandler *pH);
	int GetUpdateRadius(IFunctionHandler *pH);
	int LoadBreakable(IFunctionHandler *pH);
	int BreakEntity(IFunctionHandler *pH);
	int TriggerEvent(IFunctionHandler *pH);
	int GetHelperPos(IFunctionHandler *pH);
	int GetBonePos(IFunctionHandler *pH);
	int GetBoneDir(IFunctionHandler *pH);
	int GetBoneNameFromTable(IFunctionHandler *pH);
	int LoadVehicle(IFunctionHandler *pH);
	//int SetDamage(IFunctionHandler *pH);
	int CreateParticleEntity(IFunctionHandler *pH);
	int GetPos(IFunctionHandler *pH);
	int GetCenterOfMassPos(IFunctionHandler *pH);
	//int GetWorldPos(IFunctionHandler *pH);
	int SetPos(IFunctionHandler *pH);
	int SetName(IFunctionHandler *pH);
	int GetName(IFunctionHandler *pH);
	int SetAIName(IFunctionHandler *pH);
	int GetAIName(IFunctionHandler *pH);
	int PhysicalizeCharacter(IFunctionHandler *pH);
	int LoadObject(IFunctionHandler *pH);
	int LoadObjectPiece(IFunctionHandler *pH);
	int GetObjectPos(IFunctionHandler *pH);
	int SetObjectPos(IFunctionHandler *pH);
	int GetObjectAngles(IFunctionHandler *pH);
	int SetObjectAngles(IFunctionHandler *pH);
	int DrawObject(IFunctionHandler *pH);
	int CreateParticlePhys(IFunctionHandler *pH);
	int CreateLivingEntity(IFunctionHandler *pH);
	int CreateRigidBody(IFunctionHandler *pH);
	int CreateArticulatedBody(IFunctionHandler *pH);
	int CreateRigidBodyPiece(IFunctionHandler *pH);
	int CreateSoftEntity(IFunctionHandler *pH);
	int ResetPhysics(IFunctionHandler *pH);
	int AwakePhysics(IFunctionHandler *pH);
	int AwakeCharacterPhysics(IFunctionHandler *pH);
	int CreateStaticEntity(IFunctionHandler *pH);
	int SetAngles(IFunctionHandler *pH);
	int GetAngles(IFunctionHandler *pH);
	//int GetAnglesReal(IFunctionHandler *pH);	
	int Bind(IFunctionHandler *pH);
	int Unbind(IFunctionHandler *pH);
	int IsBound(IFunctionHandler *pH);
	int NetPresent(IFunctionHandler *pH);
	int RenderShadow(IFunctionHandler *pH);
	int SetRegisterInSectors(IFunctionHandler *pH);
	int SetPhysicParams(IFunctionHandler *pH);
	int SetCharacterPhysicParams(IFunctionHandler *pH);
	int GetParticleCollisionStatus(IFunctionHandler *pH);
	int GetObjectStatus(IFunctionHandler *pH);
	int SetObjectStatus(IFunctionHandler *pH);
	int GetDirectionVector(IFunctionHandler *pH);
	int IsAnimationRunning(IFunctionHandler *pH);
	int AddImpulse(IFunctionHandler *pH);
	int AddImpulseObj(IFunctionHandler *pH);
	int IsPointWithinRadius(IFunctionHandler *pH);
//	int RegisterWithAI(IFunctionHandler *pH);
	int GetDistanceFromPoint(IFunctionHandler *pH);
	int DestroyPhysics(IFunctionHandler *pH);
	int EnablePhysics(IFunctionHandler *pH);
	int SetSecondShader(IFunctionHandler *pH);
	int SetShader(IFunctionHandler *pH);
	int GetShader(IFunctionHandler *pH);
	int GetCameraPosition(IFunctionHandler *pH);
	int SetShaderFloat(IFunctionHandler *pH);
	int SetColor(IFunctionHandler *pH);
	int SetStatObjScale(IFunctionHandler *pH);
	int EnableSave(IFunctionHandler *pH);

	int PlaySound(IFunctionHandler *pH);

	int KillCharacter(IFunctionHandler *pH);
	int DrawCharacter(IFunctionHandler *pH);
	int LoadCharacter(IFunctionHandler *pH);
	int StartAnimation(IFunctionHandler *pH);
	int ResetAnimation(IFunctionHandler *pH);
	int SetAnimationEvent(IFunctionHandler *pH);
	int SetAnimationKeyEvent(IFunctionHandler *pH);
	int DisableAnimationEvent(IFunctionHandler *pH);
	int SetAnimationSpeed(IFunctionHandler *pH);
	int SetAnimationTime(IFunctionHandler *pH);
	int GetAnimationTime(IFunctionHandler *pH);
	int GetCurAnimation(IFunctionHandler *pH);
	int GetAnimationLength(IFunctionHandler *pH);
	int ReleaseLipSync(IFunctionHandler *pH);
	int DoRandomExpressions(IFunctionHandler *pH);
	int DoExpression(IFunctionHandler *pH);
	int SayDialog(IFunctionHandler *pH);
	int StopDialog(IFunctionHandler *pH);
	int SetTimer(IFunctionHandler *pH);
	int KillTimer(IFunctionHandler *pH);
	int SetScriptUpdateRate(IFunctionHandler *pH);
	// State managment.
	int GotoState(IFunctionHandler *pH);
	int IsInState(IFunctionHandler *pH);
	int GetState(IFunctionHandler *pH);
	int RegisterState(IFunctionHandler *pH);
	int	ApplyForceToEnvironment(IFunctionHandler *pH);

  int	IsVisible(IFunctionHandler *pH);
	//
	int GetTouchedSurfaceID(IFunctionHandler *pH);
	//
	//retrieves point of collision for rigid body
	int GetTouchedPoint(IFunctionHandler *pH);

	int AttachToBone(IFunctionHandler *pH);
	int AttachObjectToBone(IFunctionHandler *pH);
	int DetachObjectToBone(IFunctionHandler *pH);
//	int TranslatePartIdToDeadBody(IFunctionHandler *pH);

	int InitDynamicLight(IFunctionHandler *pH);
	int AddDynamicLight(IFunctionHandler *pH);
	int AddDynamicLight2(IFunctionHandler *pH);
	int	RemoveLight(IFunctionHandler *pH);

	//
	//	proseeding humming rockets
	int DoHam(IFunctionHandler *pH);
	int ResetHam(IFunctionHandler *pH);

	int LoadBoat(IFunctionHandler *pH);

//	int GetBuildingId(IFunctionHandler *pH);
//	int GetSectorId(IFunctionHandler *pH);
	int Damage(IFunctionHandler *pH);
//	int UpdateInSector(IFunctionHandler *pH);
	int GetCameraAngles(IFunctionHandler *pH);
	int CreateParticleEmitter(IFunctionHandler *pH);
	int CreateParticleEmitterEffect(IFunctionHandler *pH);
	int DeleteParticleEmitter(IFunctionHandler *pH);
	int GetEntitiesInContact(IFunctionHandler *pH);
	int IsAffectedByExplosion(IFunctionHandler *pH);
	int SetMaterial(IFunctionHandler *pH);
	int GetMaterial(IFunctionHandler *pH);
	int TrackColliders(IFunctionHandler *pH);
	int CheckCollisions(IFunctionHandler *pH);
	int AwakeEnvironment(IFunctionHandler *pH);

	int GetViewDistRatio(IFunctionHandler *pH);
	int SetViewDistRatio(IFunctionHandler *pH);
	int SetViewDistUnlimited(IFunctionHandler *pH);
	int SetStateClientside(IFunctionHandler *pH);

private: // -------------------------------------------------------------------------------

	int SetEntityPhysicParams(IPhysicalEntity *pe, IFunctionHandler *pH,int iOffs=0, ICryCharInstance *pIChar=0);
	int CreateRigidOrArticulatedBody(pe_type type, IFunctionHandler *pH);

	void SetMemberVector( SOE_MEMBER_LUA_TABLES member,const Vec3 &vec );

	IEntity *m_pEntity;
	IEntitySystem *m_pEntitySystem;
	ISystem *m_pISystem;
	ISoundSystem *m_pSoundSystem;
	int m_nCurrSoundId;
#ifdef USE_MEMBER_POS
	static IScriptObject *m_pObjectPos;
	static IScriptObject *m_pObjectAngles;
	static IScriptObject *m_pCameraPosition;
	static IScriptObject *m_pGenVector;
#endif

	// member script objects (preallocated)
	static IScriptObject* m_memberSO[SOE_MEMBER_LAST];

	// copy of function from ScriptObjectParticle
	bool ReadParticleTable(IScriptObject *pITable, struct ParticleParams &sParamOut);

//	pe_params_particle m_RocketParticlePar;		// quick fix for hamming rockets
	//float		m_ControlTime;
	//float		m_ControlTimeLimit;
	//Vec3d		m_Control;
//	char m_tonno[256];
public:
	// Enable/ disable various entity features
	int EnableProp(IFunctionHandler * pH);
	int InsertSubpipe(IFunctionHandler * pH);
	int ChangeAIParameter(IFunctionHandler * pH);
	int SetAICustomFloat(IFunctionHandler *pH);
	int ActivatePhysics(IFunctionHandler *pH);
	int SetHandsIKTarget(IFunctionHandler *pH);
	int SetDefaultIdleAnimations(IFunctionHandler *pH);
	int GetVelocity(IFunctionHandler *pH);

	int ApplyImpulseToEnvironment(IFunctionHandler * pH);
	int RemoveDecals(IFunctionHandler * pH);
	int SwitchLight(IFunctionHandler * pH);
	int ForceCharacterUpdate(IFunctionHandler * pH);
	int Hide(IFunctionHandler * pH);
	int NoExplosionCollision(IFunctionHandler * pH);

};

#endif // !defined(AFX_SCRIPTOBJECTENTITY_H__870B7388_021A_46C0_84B4_734EEB08DE7D__INCLUDED_)

