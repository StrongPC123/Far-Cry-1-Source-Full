//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
// 
//	History:
//	- May 2003: Created by MarcoK
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"
#include "WeaponSystemEx.h"
#include "WeaponClass.h"
#include "Game.h"
#include "XPlayer.h"


//////////////////////////////////////////////////////////////////////////
// Iterator class for weapon names

class CNameIterator : public INameIterator
{
public:
	CNameIterator() { m_itPos = m_lNames.end(); };
	~CNameIterator() { };
	void AddNameString(const string strName) 
	{ m_lNames.push_back(strName); MoveFirst(); };
	// INameInterator Implementation
	void Release() { delete this; };
	void MoveFirst() { m_itPos = m_lNames.begin(); };
	bool MoveNext() { 
		if (m_itPos != m_lNames.end()) { 
			m_itPos++; 
			return true;
		}
		return false;
	};
	bool Get(char *pszBuffer, INT *pSize) { 
		if (m_itPos == m_lNames.end())
			return false;
		if ((INT) (* m_itPos).length() >= *pSize) {
			*pSize = -1;
			return false;
		}
		strcpy(pszBuffer, (* m_itPos).c_str());
		*pSize = (* m_itPos).length();
		return true;
	}
protected:
	std::list<string> m_lNames;
	std::list<string>::iterator m_itPos;
};


CWeaponSystemEx::CWeaponSystemEx()
{
	m_pScriptSystem					= NULL;
	m_bRaiseScriptError			= true;
	m_soWeaponClassesTable	= NULL;
	m_soProjectileTable			= NULL;
	m_pGame									= NULL;
}

CWeaponSystemEx::~CWeaponSystemEx()
{
	// free memory
	Reset();
}

bool CWeaponSystemEx::AddWeaponClass(CWeaponClass *weaponClass)
{
	for (TWeaponClasses::const_iterator i = m_vWeaponClasses.begin(); i != m_vWeaponClasses.end(); ++i)
	{
		// check if we have a duplicate weapon class
		if ((*i)->GetName() == weaponClass->GetName())
		{
			return false;
		}
	}

	// no duplicates found, so append the new weapon class to the back
	m_vWeaponClasses.push_back(weaponClass);

	return true;
}


INameIterator * CWeaponSystemEx::GetAvailableWeapons()
{
	CNameIterator *pIterator = new CNameIterator;
	for (TWeaponClasses::iterator it=m_vWeaponClasses.begin(); it!=m_vWeaponClasses.end(); it++)
		pIterator->AddNameString((* it)->GetName().c_str());
	return (INameIterator *) pIterator;
}

INameIterator * CWeaponSystemEx::GetAvailableProjectiles()
{
	CNameIterator *pIterator = new CNameIterator;
	for (TProjectileClasses::iterator it=m_vProjectileClasses.begin(); it!=m_vProjectileClasses.end(); it++)
		pIterator->AddNameString((* it)->m_sName.c_str());
	return (INameIterator *) pIterator;
}

bool CWeaponSystemEx::Init(CXGame *pGame, bool bRaiseError)
{
	Reset();

	m_pGame = pGame;
	m_pScriptSystem = m_pGame->GetScriptSystem();
	m_sGameType = m_pGame->g_GameType->GetString();
	m_bRaiseScriptError = bRaiseError;

	// Add some global variables
	RegisterScriptConstants();

	if (!ExecuteScript("Weapons/WeaponsParams.lua"))
	{
		return false;
	}

	if (!ExecuteScript("Weapons/WeaponSystem.lua"))
	{
		return false;
	}

	m_soWeaponClassesTable = m_pScriptSystem->CreateEmptyObject();
	// now, we can load the weapon classes and firemode descriptions
	if (!m_pScriptSystem->GetGlobalValue("WeaponClassesEx", m_soWeaponClassesTable))
	{
		TRACE("Cannot access WeaponClassesEx");
		return false;
	}

	// iterate over all weapon classes
	m_soWeaponClassesTable->BeginIteration();
	while(m_soWeaponClassesTable->MoveNext())
	{
		// we assume that the WeaponParams table always contains 'key = subtable' pairs
		assert(m_soWeaponClassesTable->GetCurrentType() == svtObject);
		const char *sWeaponClassName;

		m_soWeaponClassesTable->GetCurrentKey(sWeaponClassName);

		// create a new weapon class
		CWeaponClass *pWeaponClass = new CWeaponClass(*this);
		if (pWeaponClass->Init(sWeaponClassName))
		{
			//TRACE("WeaponSystemEx: ADDING %s", sWeaponClassName);			
			AddWeaponClass(pWeaponClass);
			// for multiplayer we always want to load all classes immediately (bug 5825)
			if (GetGame()->IsMultiplayer())
			{
				pWeaponClass->Load();
			}
		}
		else
		{
			delete pWeaponClass;
		}
	}
	m_soWeaponClassesTable->EndIteration();

	m_soProjectileTable = m_pScriptSystem->CreateEmptyObject();
	// now, we can load the weapon classes and firemode descriptions
	if (!m_pScriptSystem->GetGlobalValue("Projectiles", m_soProjectileTable))
	{
		TRACE("Cannot access Projectiles");
		return false;
	}

	// iterate over all weapon classes
	m_soProjectileTable->BeginIteration();
	while(m_soProjectileTable->MoveNext())
	{
		// we assume that the Projectiles table always contains 'key = subtable' pairs
		assert(m_soProjectileTable->GetCurrentType() == svtObject);
		const char *sProjectileName;

		_SmartScriptObject soProjectile(m_pScriptSystem);
		m_soProjectileTable->GetCurrentKey(sProjectileName);
		m_soProjectileTable->GetCurrent(*soProjectile);

		{
			const char *model="";
			bool bOk=soProjectile->GetValue("model",model);				assert(bOk);

			if(!bOk)continue;

			IStatObj *pObj = GetGame()->GetSystem()->GetI3DEngine()->MakeObject(model);

			IEntityClassRegistry *pReg=GetGame()->GetClassRegistry();

			EntityClass *pClass = pReg->GetByClass(sProjectileName);					assert(pClass);

			if (pClass)
			{
				GetGame()->GetClassRegistry()->LoadRegistryEntry(pClass);			// load the associated script object
				m_vProjectileClasses.push_back(new CProjectileClass(pClass->ClassId, sProjectileName, pObj, *this));
			}
		}
	}
	m_soProjectileTable->EndIteration();

	return true;
}

void CWeaponSystemEx::AddProjectileClass( int classid )
{
	// currently not needed
}

void CWeaponSystemEx::Reset()
{
	if(m_pGame && m_pGame->GetMyPlayer())
	{
		IEntityContainer *pCnt;
		CPlayer *pPlayer = NULL;

		pCnt=m_pGame->GetMyPlayer()->GetContainer();

		if(pCnt)
		{
			pCnt->QueryContainerInterface(CIT_IPLAYER,(void **) &pPlayer);
			if(pPlayer)
			{
				pPlayer->SelectWeapon(-1, false);
			}
		}
	}

	// kill registered weapon classes
	while (!m_vWeaponClasses.empty())
	{
		delete m_vWeaponClasses.back();
		m_vWeaponClasses.pop_back();
	}

	SAFE_RELEASE(m_soWeaponClassesTable);

	// kill registered projectile classes
	while (!m_vProjectileClasses.empty())
	{
		delete m_vProjectileClasses.back();
		m_vProjectileClasses.pop_back();
	}

	SAFE_RELEASE(m_soProjectileTable);

	// kill the CGF cache
	while (!m_vCachedObjects.empty())
	{
		GetGame()->GetSystem()->GetI3DEngine()->ReleaseObject(m_vCachedObjects.back());
		m_vCachedObjects.pop_back();
	}

	UnloadScript("Weapons/WeaponsParams.lua");
}

bool CWeaponSystemEx::ExecuteScript(const string& sScriptName)
{
	if (m_pScriptSystem == NULL)
		return false;

	string sFilename = "Scripts\\" + m_sGameType + "\\Entities\\" + sScriptName;
	m_pGame->GetSystem()->GetILog()->Log("WEAPONEX : Loading %s",sFilename.c_str());
	if (!m_pScriptSystem->ExecuteFile(sFilename.c_str(), m_bRaiseScriptError))
	{
		sFilename = "Scripts\\Default\\Entities\\" + sScriptName;
		m_pGame->GetSystem()->GetILog()->Log("WEAPONEX : Loading %s",sFilename.c_str());
		if (!m_pScriptSystem->ExecuteFile(sFilename.c_str(), true))
		{
			return false;
		}
	}

	return true;
}

void CWeaponSystemEx::UnloadScript(const string& sScriptName)
{
	if (m_pScriptSystem == NULL)
		return;

	string sFilename = "Scripts\\" + m_sGameType + "\\Entities\\" + sScriptName;

	m_pGame->GetSystem()->GetILog()->Log("WEAPONEX : UNLoading %s",sFilename.c_str());	
	m_pScriptSystem->UnloadScript(sFilename.c_str());

	sFilename = "Scripts\\Default\\Entities\\" + sScriptName;
	m_pGame->GetSystem()->GetILog()->Log("WEAPONEX : UNLoading %s",sFilename.c_str());

	m_pScriptSystem->UnloadScript(sFilename.c_str());
}

CWeaponClass* CWeaponSystemEx::GetWeaponClassByID(int ID) const
{
	for (TWeaponClasses::const_iterator i = m_vWeaponClasses.begin(); i != m_vWeaponClasses.end(); ++i)
	{
		if ((*i)->GetID() == ID)
			return (*i);
	}

	return NULL;
}

int CWeaponSystemEx::GetWeaponClassIDByName(const string& name) const
{
	CWeaponClass *pWC = GetWeaponClassByName(name);

	if (pWC)
		return pWC->GetID();

	return -1;
}

CWeaponClass* CWeaponSystemEx::GetWeaponClassByName(const string& name) const
{
	for (TWeaponClasses::const_iterator i = m_vWeaponClasses.begin(); i != m_vWeaponClasses.end(); ++i)
	{
		if ((*i)->GetName() == name)
			return (*i);
	}

	return NULL;
}

unsigned CWeaponSystemEx::MemStats() const
{
	unsigned memSize = sizeof *this;

	memSize += m_sGameType.capacity();

	memSize += m_vWeaponClasses.capacity() * sizeof(void*);
	for (TWeaponClasses::const_iterator i = m_vWeaponClasses.begin(); i != m_vWeaponClasses.end(); ++i)
	{
		memSize += (*i)->MemStats();
	}

	memSize += m_vProjectileClasses.capacity() * sizeof(void*);
	for (TProjectileClasses::const_iterator i = m_vProjectileClasses.begin(); i != m_vProjectileClasses.end(); ++i)
	{
		memSize += sizeof(CProjectileClass);
		memSize += (*i)->m_sName.capacity();
	}

	memSize += m_vCachedObjects.capacity() * sizeof(void*);

	return memSize;
}

void CWeaponSystemEx::Read(CStream &stm)
{
	// reset our internal state
	Reset();

	BYTE n;

	// get number of weapon classes
	stm.Read(n);

	// get weapon classes
	for (int i = 0; i < n; ++i)
	{
		CWeaponClass *pWeaponClass = new CWeaponClass(*this);
		pWeaponClass->Read(stm);

		AddWeaponClass(pWeaponClass);
	}
}

void CWeaponSystemEx::Write(CStream &stm) const
{
	assert(m_vWeaponClasses.size() <= 255);

	// number of weapon classes
	stm.Write((BYTE)m_vWeaponClasses.size());

	for (TWeaponClasses::const_iterator i = m_vWeaponClasses.begin(); i != m_vWeaponClasses.end(); ++i)
	{
		(*i)->Write(stm);
	}
}

bool CWeaponSystemEx::AddWeapon(const string& name)
{
	for (TWeaponClasses::const_iterator i = m_vWeaponClasses.begin(); i != m_vWeaponClasses.end(); ++i)
	{
		if ((*i)->GetName() == name && !(*i)->IsLoaded())
		{
			if((*i)->Load())
			{
				_SmartScriptObject cWeaponLoadedTable(m_pScriptSystem, true);

				if (m_pScriptSystem->GetGlobalValue("WeaponsLoaded", cWeaponLoadedTable))
				{
					cWeaponLoadedTable->SetValue(name.c_str(), true);
				}
				return true;
			}
		}
	}

	return false;
}

bool CWeaponSystemEx::IsLeftHanded ()
{
	return m_pGame->g_LeftHanded->GetIVal() != 0;
}

void CWeaponSystemEx::CacheObject(const string& name)
{
	IStatObj *pObject = GetGame()->GetSystem()->GetI3DEngine()->MakeObject(name.c_str());
	if (pObject)
	{
		m_vCachedObjects.push_back(pObject);
	}
}

bool CWeaponSystemEx::IsProjectileClass( const EntityClassId clsid )
{
	for (unsigned int i=0;i<m_vProjectileClasses.size();i++)
	{
		if (m_vProjectileClasses[i]->m_ClassID == clsid)
			return true;
	}
	return false;
}

CWeaponClass* CWeaponSystemEx::GetWeaponClass(unsigned int index) const
{
	if (index < m_vWeaponClasses.size())
		return m_vWeaponClasses[index];
	else
		return NULL;
}

void CWeaponSystemEx::RegisterScriptConstants() const
{
	// Add some global variables
	#define SET_GLOBAL(name)\
		m_pScriptSystem->SetGlobalValue(#name, name);

	SET_GLOBAL(FireMode_Instant);
	SET_GLOBAL(FireMode_Projectile);
	SET_GLOBAL(FireMode_Melee);
	SET_GLOBAL(FireMode_EngineerTool);

	#undef SET_GLOBAL
}
