//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//  Description:
//	For every weapon type in the game, we have one CWeaponClass. It allows
//	access to the individual firemodes of the weapon (and their
//	corresponding properties).
//						
//	History:
//	- May 2003: Created by MarcoK
//
//////////////////////////////////////////////////////////////////////
#ifndef WEAPONCLASS_H__
#define WEAPONCLASS_H__

#include "ScriptObjectVector.h"
#include "FireType.h"
#include "xplayer.h"

#include <string>
#include <vector>

struct IScriptSystem;
class  CWeaponSystemEx;
class  CScriptObjectWeaponClass;

enum EWeaponFunc
{
	WeaponFunc_OnInit = 0,
	WeaponFunc_OnActivate,
	WeaponFunc_OnDeactivate,
	WeaponFunc_WeaponReady,
	WeaponFunc_OnAnimationKey,
	WeaponFunc_OnStopFiring,
	WeaponFunc_OnFireCancel,
	WeaponFunc_Reload,
	WeaponFunc_Drop,
	WeaponFunc_OnUpdate,
	WeaponFunc_OnFire,
	WeaponFunc_OnHit,
	WeaponFunc_OnEvent,
	WeaponFunc_Count
};

#define OT_ENTITY			0
#define OT_STAT_OBJ		1
#define OT_TERRAIN		2
#define OT_BOID				3
#define OT_RIGID_PARTICLE	10

/*!
* Structure that represents how object been hit by the weapon.
*/
struct SWeaponHit
{
	Vec3		pos;					//!< Excact position of hit.
	Vec3		normal;				//!< Normal of hitted surface.
	Vec3		dir;					//!< Direction of shoot.
	float		damage;				//!< Calculated damage that should be inflicted by this kind of hit.
	IEntity *target;			//!< What entity was hit.
	IEntityRender	*targetStat;	//!< What static object was hit.
	int			ipart;				//!< id of hit entity part
	int			objecttype;		//!< If static object or terrain was hit			1-statobj, 2-terrain
	IEntity *shooter;			//!< Player who shot.
	IScriptObject *weapon;//!< Weapon used to shoot.
	IEntity *projectile;	//!< Projectile that caused this hit.
	int surface_id;				//!< Physical surface
	int weapon_death_anim_id; //!< Death anim ID
	int iImpactForceMul;
	int	iImpactForceMulFinal;
	int iImpactForceMulFinalTorso;

	// Explosion related.
	float rmin;
	float rmax;
	float radius;					//!< Damage radius.
	float impulsive_pressure;


	//	void FillScriptTable( CScriptTable &tbl ) const;
};

typedef struct WeaponParams
{
	WeaponParams()
	{
		fire_activation=eHolding;
		max_recoil=1.2f;
		min_recoil=0.5f;
		fMinAccuracy=0.7f;
		fMaxAccuracy=0.5f;
		fAimImprovement=0.5;
		no_ammo=false;
		accuracy_decay_on_run=1;
		iFireModeType = -1; //unknown
	}

	bool		bAIMode;			//!< is this mode used only by the AI
	bool		bAllowHoldBreath;	//!< should the hold breath key be active for this weapon in zoom mode
//	bool		bAutoAiming;	//!< does this fire mode support automatic aiming support
	float   fAutoAimDist;		//!< if 0 - no autoaiming, otherwise - autoAim snap distance in screen space
								// works only for wevicles autoveapons
	float   fMinAccuracy;
	float   fMaxAccuracy;
	float		fAimImprovement;
	float		fAimRecoilModifier;
	float		fSprintPenalty;
	float   fReloadTime;
	float   fFireRate;
	float   fTapFireRate;
	float   fDistance;
	int     nDamage;
	float   fDamageDropPerMeters;
	int     nBulletpershot;
	int			fire_activation;

	float		fAccuracyModifierStanding;
	float		fAccuracyModifierCrouch;
	float		fAccuracyModifierProne;

	float		fRecoilModifierStanding;
	float		fRecoilModifierCrouch;
	float		fRecoilModifierProne;

	float   max_recoil;
	float   min_recoil;
	float		accuracy_decay_on_run;
	bool		no_ammo;
	float		whizz_sound_radius;

	int			iFireModeType;	//!< type of the fire mode (instant, projectile, melee)
	bool		bShootUnderwater;

	// projectile-related
	int     iBulletsPerClip;
	int			iDeathAnim;
	int			iImpactForceMul;
	int			iImpactForceMulFinal;
	int			iImpactForceMulFinalTorso;

	string	sProjectileClass;
} WeaponParams;

class CWeaponClass : public ICharInstanceSink
{
public:
	CWeaponClass(CWeaponSystemEx& rWeaponSystem);
	virtual ~CWeaponClass();

	// BEGIN ICharInstanceSink
	virtual void OnStartAnimation(const char *sAnimation);
	virtual void OnAnimationEvent(const char *sAnimation, AnimSinkEventData UserData);
	virtual void OnEndAnimation(const char *sAnimation);
	// END ICharInstanceSink

	bool	Init(const string& sName);
	void	Reset();

	const string&						GetName() const					{	return m_sName;	}
	void										SetName(const string& sName)		{	m_sName = sName;	}
	int											GetID() const						{	return m_ID;		}
	CWeaponSystemEx&				GetWeaponSystem()	const	{	return m_rWeaponSystem;	}

	// rendering related
	IStatObj*						GetObject() const			{	return m_pObject;	}
	ICryCharInstance*		GetCharacter() const	{	return m_pCharacter;	}
	IStatObj*						GetMuzzleFlash() const{	return m_pMuzzleFlash;	}
	const string&				GetBindBone() const		{	return m_sBindBone;	}
	void								SetBindBone(const string& sBindBone)	{	m_sBindBone = sBindBone;	}
	bool								LoadMuzzleFlash(const string& sGeometryName);

	// serialization
	void Read(CStream& stm);
	void Write(CStream& stm) const;

	// calls script functions of script object
	void ScriptOnInit();
	void ScriptOnActivate(IEntity *pShooter);
	void ScriptOnDeactivate(IEntity *pShooter);
	void ScriptWeaponReady(IEntity *pShooter);
	void ScriptOnStopFiring(IEntity *pShooter);
	bool ScriptOnFireCancel(IScriptObject *params);
	void ScriptReload(IEntity *pShooter);
	void ScriptDrop(IScriptObject *params);
	void ScriptOnUpdate(float fDeltaTime, IEntity *pShooter);
	bool ScriptOnFire(IScriptObject *params);
	void ScriptOnHit(IScriptObject *params);
	void ScriptOnEvent(int eventID, IScriptObject *params, bool *pRet=NULL);

	// positioning
	void SetFirstPersonWeaponPos(const Vec3 &pos, const Vec3 &angles);
	//! Find position of fire for this player.
	Vec3	GetFirePos(IEntity *pIEntity) const;

	//! Set offset of weapon for first person view.
	void SetFirstPersonOffset(const Vec3d &posOfs, const Vec3d &angOfs);
	Vec3 GetFirstPersonOffset() { return m_fpvPosOffset; };
	void MoveToFirstPersonPos(IEntity *pIEntity);

	const Vec3& GetAngles() const	{	return m_vAngles;	}
	const Vec3& GetPos() const	{	return m_vPos;	}

	IScriptObject* GetScriptObject() {	return m_soWeaponClass;	}

	bool	IsLoaded() const	{	return m_bIsLoaded;	}
	void	Unload();
	bool	Load();

	//! retrieve fire-type dependent fire rate ... 
	float GetFireRate(eFireType ft)	const;

	//! does the weapon have an AI specific fire mode
	//!
	//! @return true if there is an AI fire mode
	bool HasAIFireMode()	const;

	//! retrieve the AI specific fire mode number
	//!
	//! @return the fire mode the AI should use
	int GetAIFireMode()	const;

	//! retrieve the index of the next fire mode. This function will
	//! skip/retrieve AI specific firemodes where necessary.
	//!
	//! @return index of the next firemode
	int GetNextFireMode(int oldMode, bool isAI)	const;

	//! memory statistics
	//! @return number of bytes used
	unsigned	MemStats() const;

private:
	bool InitWeaponClassVariables();
	bool InitScripts();
	bool InitModels();
	void ProcessHitTarget(const SWeaponHit &hit);

	// the actual weapon class variables
	int									m_ID;							//!< class ID
	string							m_sScript;				//!< script file representing this weapon class
	string							m_sPickup;				//!< pickup script file representing this weapon class
	bool								m_bIsLoaded;			//!< is the class loaded

	CWeaponSystemEx&		m_rWeaponSystem;	//!< reference to the weapon system we belong to
	IScriptSystem*			m_pScriptSystem;	//!< pointer to script system
	string							m_sName;					//!< the name of the weapon class (e.g. "Machete")
	IScriptObject*			m_soWeaponClass;

	// position
	Vec3								m_vAngles;
	Vec3								m_vPos;
	Vec3								m_fpvPos;
	Vec3								m_fpvAngles;
	Vec3								m_fpvPosOffset;
	Vec3								m_fpvAngleOffset;

	// script function callback tables
	HSCRIPTFUNCTION			m_hClientFuncs[WeaponFunc_Count];
	HSCRIPTFUNCTION			m_hServerFuncs[WeaponFunc_Count];

	// rendering related
	IStatObj*						m_pObject;				//!< third person weapon model
	ICryCharInstance*		m_pCharacter;			//!< first person animated weapon
	IStatObj*						m_pMuzzleFlash;		//!< muzzle flash (used in both 3rd and 1st person)
	string							m_sBindBone;			//!< name of bone to bind object to

	// FIXME: clean this stuff up
public:
	WeaponParams *AddWeaponParams(const WeaponParams &params );
	void					GetWeaponParams(WeaponParams &params );
	bool					GetModeParams(int mode, WeaponParams &stats);
	bool					CancelFire();
	void					Update(CPlayer *pPlayer);
	int						Fire(const Vec3d &origin, const Vec3d &angles, CPlayer *pPlayer, WeaponInfo &winfo, IPhysicalEntity *pIRedirected);
	void					ProcessHit(const SWeaponHit &hit);
	void					CalculateWeaponAngles(BYTE random_seed, Vec3d* pVector, float fAccuracy);

	typedef std::vector<WeaponParams*> WeaponParamsVec;
	typedef WeaponParamsVec::iterator WeaponParamsVecItor;
	WeaponParamsVec m_vFireModes;
	int						m_nAIMode;					//!< fire mode number which the AI will use for this weapon (-1 if there is no AI mode)
	char					m_HoldingType;
	WeaponParams	m_fireParams;
	float					m_fLastUpdateTime;
	int						m_nLastMaterial;		//used to avoid playing the same material sound twice in a row

	_SmartScriptObject	m_ssoFireTable;
	_SmartScriptObject	m_ssoProcessHit;

	_SmartScriptObject	m_sso_Params_OnAnimationKey;
	_SmartScriptObject	m_sso_Params_OnActivate;
	_SmartScriptObject	m_sso_Params_OnDeactivate;

	CScriptObjectVector m_ssoHitPosVec;
	CScriptObjectVector m_ssoHitDirVec;
	CScriptObjectVector m_ssoHitNormVec;
	CScriptObjectVector m_ssoHitPt;
	CScriptObjectVector m_ssoBulletPlayerPos;
};

#endif //WEAPONCLASS_H__
