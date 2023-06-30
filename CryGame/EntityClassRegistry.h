
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// EntityClassRegistry.h: interface for the CEntityClassRegistry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ENTITYCLASSREGISTRY_H__CDAB4B12_9A92_4747_BEB3_C4B80B4ACB3D__INCLUDED_)
#define AFX_ENTITYCLASSREGISTRY_H__CDAB4B12_9A92_4747_BEB3_C4B80B4ACB3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct IScriptSystem;

typedef std::map<EntityClassId,EntityClass> EntityClassMap;
typedef EntityClassMap::iterator EntityClassMapItor;

/*
	*
	* This class store all entity types information (classid,classname,scriptpath)
	*	All types of entities must be registered here, in order to be able to spawn in the game.
	*	
	*/

class CEntityClassRegistry :
public IEntityClassRegistry
{
public:
	//! constructor
	CEntityClassRegistry();
	//! destructor
	virtual ~CEntityClassRegistry();
	//!
	void ResetClassRegistry();
	//!
	void Init( ISystem *pSystem );
	//!
	void SetGameType( const string &sGameType );
	//!
	bool InitGameClasses();

	// interface IEntityClassRegistry --------------------------------------------------------------

	virtual EntityClass* GetByClass(const char *str,bool bAutoLoadScript=true);
	virtual EntityClass* GetByClassId(const EntityClassId ClassId,bool bAutoLoadScript=true);
	virtual bool AddClass( const EntityClassId TypeID,const char* sClassName,const char* sScriptFile,bool bReserved=false,bool bForceReload=false );
	virtual void MoveFirst();
	virtual EntityClass *Next();
	virtual int Count();
	virtual bool LoadRegistryEntry(EntityClass * pClass, bool bForceReload=false);
	virtual void Debug();

	// --------------------------------------------------------------------------------------------

	//
	unsigned MemStats();

private:
	//!
	bool InitRegistry();

	EntityClassMap				m_vEntityClasses;
	ISystem *							m_pSystem;
	IScriptSystem *				m_pScriptSystem;
	string								m_sGameType;
	EntityClassMapItor		m_itor;
}; 

#endif // !defined(AFX_ENTITYCLASSREGISTRY_H__CDAB4B12_9A92_4747_BEB3_C4B80B4ACB3D__INCLUDED_)
