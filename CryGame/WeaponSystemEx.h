//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//  Description: Manages weapon classes defined in Lua-Script.
//
//	History:
//	- May 2003: Created by MarcoK
//
//////////////////////////////////////////////////////////////////////
#ifndef WEAPONSYSTEMEX_H__
#define WEAPONSYSTEMEX_H__

#include <vector>
#include <string>

struct	IScriptSystem;
class		CWeaponClass;
class		CXGame;

// all available firemodes. registered as lua-constants with identical name in Init()
enum EFireModeType
{
	FireMode_Instant			= 0x00000001,
	FireMode_Projectile		= 0x00000002,
	FireMode_Melee				= 0x00000003,
	FireMode_EngineerTool	= 0x00000004,
};


class CProjectileClass;

class CWeaponSystemEx
{
public:
	typedef std::vector<CWeaponClass*> TWeaponClasses;
	typedef std::vector<CProjectileClass*> TProjectileClasses;
	typedef std::vector<IStatObj*> TObjectVector;

public:
	CWeaponSystemEx();
	virtual ~CWeaponSystemEx();

	void AddProjectileClass( int classid );

	//!	Add a certain weapon class to the weapon system. The class ID will be the
	//! index in the m_vWeaponClasses vector. The function ensures, that the 
	//! weapon class is unique by comparing the names of each class.
	//!
	//!	@param	weaponClass the weapon class to add
	//!	@return	true on success, false if the class already exists or the loading failed
	bool AddWeaponClass(CWeaponClass* weaponClass);

	//! Initialize the weapon system. This will ALWAYS(!) reset the weapon system ... even if
	//! the call fails for some reason (script can't be loaded, no script system)
	//!
	//! @param	pGame pointer to the game this weapon system is part of
	//! @param	bRaiseError raise an error when executing the script
	//! @return true on success, false otherwise
	bool Init(CXGame* pGame, bool bRaiseError = true);
	void Reset();

	//! Execute a weapon script with the correct game-type and with the correct error raising
	//! behavior. This is usually called by the WeaponClass.
	//!
	//! @param	sScriptName name of the script to execute under the Weapons directory of the current gametype
	//! @return	true on success, false otherwise
	bool ExecuteScript(const string& sScriptName);

	//! Unload a weapon script with the correct game-type and with the correct error raising
	//! behavior. This is usually called by the WeaponClass.
	//!
	//! @param	sScriptName name of the script to unload under the Weapons directory of the current gametype
	void UnloadScript(const string& sScriptName);

	//! retrieve pointer to the script system in use by the weapon system
	IScriptSystem* GetScriptSystem() const	{	return m_pScriptSystem;	}
	//! retrieve pointer to the game using this weapon system
	CXGame* GetGame() const	{	return m_pGame;	}

	//! retrieve weapon class object based on the ID 
	CWeaponClass* GetWeaponClassByID(int ID) const;
	//! retrieve the ID of the weapon based on the name
	//!
	//! @param name name of the weapon class to determine the ID for
	//! @return on success an ID >= 0, -1 otherwise
	int GetWeaponClassIDByName(const string& name) const;
	//! retrieve weapon class object based on its name 
	CWeaponClass* GetWeaponClassByName(const string& name) const;

	//! memory statistics
	//! @return number of bytes used
	unsigned	MemStats() const;

	// serialization
	void Read(CStream& stm);
	void Write(CStream& stm) const;

	IScriptObject*	GetWeaponClassesTable() const	{	return m_soWeaponClassesTable;	}

	INameIterator * GetAvailableWeapons();
	INameIterator * GetAvailableProjectiles();

	bool AddWeapon(const string& name);
	//!
	bool IsLeftHanded ();
	//!
	bool IsProjectileClass(const EntityClassId clsid);

	//! preloads a CGF and keeps a reference to it until the system is reset
	void CacheObject(const string& name);

	//! retrieve number of weapon classes
	unsigned int	GetNumWeaponClasses() const	{	return (unsigned)m_vWeaponClasses.size();	}
	CWeaponClass*	GetWeaponClass(unsigned int index) const;

private:
	//! expose the enums above to the script
	void RegisterScriptConstants() const;

	// parameters set on init
	CXGame*					m_pGame;								//!<	pointer to the game using the weapon system
	IScriptSystem*	m_pScriptSystem;				//!<	pointer to the script system
	string					m_sGameType;						//!<	the name of the game type we are currently in
	bool						m_bRaiseScriptError;		//!<	should we raise errors when executing scripts? (default = true)
	IScriptObject*	m_soWeaponClassesTable;	//!<	pointer to table containing the weapon classes
	IScriptObject*	m_soProjectileTable;		//!<	pointer to table containing the projectile classes

	TWeaponClasses			m_vWeaponClasses;			//!<	list of weapon classes
	TProjectileClasses	m_vProjectileClasses;	//!<	list of projectile classes
	TObjectVector				m_vCachedObjects;			//!<  list of cached CGFs (used for Muzzleflashes)
};

class CProjectileClass
{
public:
	EntityClassId			m_ClassID;
	string						m_sName;
	IStatObj*					m_pObject;
	CWeaponSystemEx&	m_rWeaponSystem;

	CProjectileClass(const EntityClassId ClassID, const string& sName, IStatObj* pObject, CWeaponSystemEx& rWeaponSystem) :
		m_ClassID(ClassID),
		m_sName(sName),
		m_pObject(pObject),
		m_rWeaponSystem(rWeaponSystem)
	{
	}

	~CProjectileClass()
	{
		if (m_pObject && m_rWeaponSystem.GetGame()->GetSystem()->GetI3DEngine())
		{
			m_rWeaponSystem.GetGame()->GetSystem()->GetI3DEngine()->ReleaseObject(m_pObject);
		}
	}
};

#endif //WEAPONSYSTEMEX_H__
