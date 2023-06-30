
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// EntityClassRegistry.cpp: implementation of the CEntityClassRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EntityClassRegistry.h"
#include "IScriptSystem.h"

#include <StlUtils.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CEntityClassRegistry::CEntityClassRegistry()
{
	MoveFirst();
}

//////////////////////////////////////////////////////////////////////////
CEntityClassRegistry::~CEntityClassRegistry()
{
}

//////////////////////////////////////////////////////////////////////////
void CEntityClassRegistry::ResetClassRegistry()
{
	if(m_pScriptSystem){
		EntityClassMapItor itor=m_vEntityClasses.begin();
		while(itor!=m_vEntityClasses.end())
		{
			m_pScriptSystem->SetGlobalToNull(itor->second.strClassName.c_str());
			m_pScriptSystem->UnloadScript(itor->second.strFullScriptFile.c_str());
			++itor;
		}
		m_vEntityClasses.clear();
	}
}

// Initializes the ClassRegistry. Must be called before usage of any other functions in this class.
//////////////////////////////////////////////////////////////////////////
void CEntityClassRegistry::Init( ISystem *pSystem )
{
	m_pSystem = pSystem;
	m_pScriptSystem = m_pSystem->GetIScriptSystem();

	ResetClassRegistry();
	InitRegistry();
}

//////////////////////////////////////////////////////////////////////////
void CEntityClassRegistry::SetGameType( const string &sGameType )
{
	m_sGameType=sGameType;
	
	// Unload all entity scripts.
	if(m_pScriptSystem)
	{
		EntityClassMapItor itor=m_vEntityClasses.begin();
		while(itor!=m_vEntityClasses.end())
		{
			EntityClass *pEntClass = &(itor->second);
			if (pEntClass->bLoaded)
			{
				m_pScriptSystem->SetGlobalToNull(itor->second.strClassName.c_str());
				m_pScriptSystem->UnloadScript(itor->second.strFullScriptFile.c_str());
			}
			pEntClass->bLoaded = false;
			++itor;
		}
	}
}

void CEntityClassRegistry::Debug()
{
	EntityClassMapItor itor;
	itor=m_vEntityClasses.begin();
	while(itor!=m_vEntityClasses.end())
	{
		char str[256];

		sprintf(str," CEntityClassRegistry::Debug %c %d '%s'\n",
			itor->second.bLoaded?'1':'0',itor->first,itor->second.strClassName.c_str());

		OutputDebugString(str);

		++itor;
	}
}



// Register a new class in the registry.
//////////////////////////////////////////////////////////////////////////
bool CEntityClassRegistry::AddClass(const EntityClassId _ClassId,const char* sClassName,const char* sScriptFile,bool bReserved, bool bForceReload)
{
	ILog *pLog = m_pSystem->GetILog();
	EntityClass ec;

	//is the id already used?
	if(GetByClassId(_ClassId,false)!=NULL && !bForceReload)
	{
		CryError( "<EntityClassRegistry> AddClass called with duplicate ClassID ID=%d Class=\"%s\"",(int)_ClassId,sClassName );	
		return false;
	}

	if (GetByClass(sClassName,false)!=NULL)
	{
		CryError( "<EntityClassRegistry> AddClass called with duplicate Class Name ID=%d Class=\"%s\"",(int)_ClassId,sClassName );	
		return false;
	}

	string sFilename;

	//Timur[5/8/2002]
	if (strlen(sScriptFile) > 0)
	{
		// Adds class with no script file.

		// lets try to load the script from the current gametype-folder
		if (m_sGameType.empty())
#if defined(LINUX)
			sFilename=string("Scripts/Default/Entities/")+sScriptFile;
#else
			sFilename=string("Scripts\\Default\\Entities\\")+sScriptFile;
#endif
		else
#if defined(LINUX)
			sFilename="Scripts/"+m_sGameType+"/Entities/"+sScriptFile;
#else
			sFilename="Scripts\\"+m_sGameType+"\\Entities\\"+sScriptFile;
#endif
	}

	ec.ClassId=_ClassId;
	ec.strClassName=sClassName;
	ec.strScriptFile=sScriptFile;
	ec.strFullScriptFile = sFilename;
	ec.bReserved=bReserved;
	m_vEntityClasses[_ClassId] = ec;

	return true;
}

// Retrieve a class-description by the class-name
//////////////////////////////////////////////////////////////////////////
EntityClass *CEntityClassRegistry::GetByClass(const char *sClassName,bool bAutoLoadScript)
{
	//<<FIXME>> optimize this
	EntityClassMapItor itor;
	itor=m_vEntityClasses.begin();
	while(itor!=m_vEntityClasses.end())
	{
		if (stricmp(itor->second.strClassName.c_str(),sClassName) == 0)
		{
			if(!itor->second.bLoaded && bAutoLoadScript)
			{
				if (!LoadRegistryEntry(&itor->second,true))
					return NULL;
			}
			return &(itor->second);
		}
		++itor;
	}
	return NULL;
}

// Retrieve a class-description by the class-id
//////////////////////////////////////////////////////////////////////////
EntityClass *CEntityClassRegistry::GetByClassId(const EntityClassId _ClassId,bool bAutoLoadScript)
{
	//<<FIXME>> optimize this
	EntityClassMapItor itor = m_vEntityClasses.find( _ClassId );

	if (itor!=m_vEntityClasses.end())
	{
		if(!itor->second.bLoaded && bAutoLoadScript)
		{
			if (!LoadRegistryEntry(&itor->second,true))
				return NULL;
		}
		return &itor->second;
	}
	return NULL;
}


// Move the ClassIterator to the first element.
//////////////////////////////////////////////////////////////////////////
void CEntityClassRegistry::MoveFirst()
{
	m_itor=m_vEntityClasses.begin();
}

// Retrieve the next class-description.
//////////////////////////////////////////////////////////////////////////
EntityClass *CEntityClassRegistry::Next()
{
	EntityClass *pEntityClass=NULL;
	if(m_itor==m_vEntityClasses.end())
		return NULL;
	pEntityClass=&(m_itor->second);
	++m_itor;
	return pEntityClass;
}

// Retrieves the number of classes in the registry.
//////////////////////////////////////////////////////////////////////////
int CEntityClassRegistry::Count()
{
	return m_vEntityClasses.size();
}

//////////////////////////////////////////////////////////////////////////
bool CEntityClassRegistry::LoadRegistryEntry(EntityClass * pClass,bool bForceReload )
{
	assert( pClass );
	ILog *pLog = m_pSystem->GetILog();

	if (pClass->bLoaded && !bForceReload)
		return true;

	if (!m_pScriptSystem || pClass->strScriptFile.size()==0)
		return true;	// no script attached (entities like cameras etc...)
#if defined(LINUX)
	bool bStartsWithSlash = false;
	if(m_sGameType.size() > 0)
		if((m_sGameType.c_str()[0] == '/') || (m_sGameType.c_str()[0] == '\\'))
			bStartsWithSlash = true;
	string sFilename=(bStartsWithSlash?"Scripts/":"Scripts")+m_sGameType+"/Entities/"+pClass->strScriptFile;
#else
	string sFilename="Scripts\\"+m_sGameType+"\\Entities\\"+pClass->strScriptFile;
#endif
	pClass->strFullScriptFile = sFilename;
	if (m_sGameType.empty() || !m_pScriptSystem->ExecuteFile(sFilename.c_str(), false,bForceReload))
	{
		// failed, so try to load it from the default-folder
		if (pLog)
		{
			string sMessage=sFilename +" is not available. Loading default script.";
			pLog->LogToFile(sMessage.c_str());
		}
#if defined(LINUX)
		sFilename=string("Scripts/Default/Entities/")+pClass->strScriptFile;
#else
		sFilename=string("Scripts\\Default\\Entities\\")+pClass->strScriptFile;
#endif
		pClass->strFullScriptFile = sFilename;
		if (!m_pScriptSystem->ExecuteFile(sFilename.c_str(),true,bForceReload))
		{
			// failed too: return...
			string sMessage=sFilename+" Can't be loaded, script not found !";
			GameWarning( sMessage.c_str() );
			return false;
		}
	}
	pClass->bLoaded=true;
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CEntityClassRegistry::InitRegistry()
{
	m_pSystem->GetILog()->Log("<EntityClassRegistry> Initializing");

	const char *sFilename = "Scripts/ClassRegistry.lua";
	// load registry lua script.
	m_pScriptSystem->ExecuteFile(sFilename);

	_SmartScriptObject pTable(m_pScriptSystem,true);
	_SmartScriptObject pLineObj(m_pScriptSystem,true);
	if (!m_pScriptSystem->GetGlobalValue("EntityClassRegistry",*pTable))
	{
		CryError("Cannot find EntityClassRegistry table in scripts (wrong working folder?)");
		return false;
	}
	int i=0;

	// Scan table.
	while (pTable->GetAt(++i,*pLineObj))
	{
		int clsid;
		const char *scriptfile;
		const char *tablename;
		const char *entity_type;

		if (pLineObj->GetAt(1,entity_type) &&
				pLineObj->GetAt(2,tablename) &&
				pLineObj->GetAt(3,clsid) &&
				pLineObj->GetAt(4,scriptfile))
		{
			//EntityClassMapItor itor = m_vEntityClasses.find( cTypeID );
			//if (itor!=m_vEntityClasses.end())

			EntityClassId ClassId=(EntityClassId)clsid;

			EntityClass *pEntityClass = GetByClassId(ClassId,false);
			if (pEntityClass)
			{
				GameWarning( "<EntityClassRegistry> Duplicate Class ID, ClsId=%d already registered for class %s",clsid,pEntityClass->strClassName.c_str() );
			}
			if (!AddClass(ClassId,tablename,scriptfile,true))
			{
				GameWarning( "<EntityClassRegistry> AddClass failed, Class group='%s' clsid=%d name='%s' script='%s'",entity_type,clsid,tablename,scriptfile );
				continue;
			}
			pEntityClass = GetByClassId(ClassId,false);
			assert( pEntityClass ); // Cannot happen.

			pEntityClass->bReserved = true;
			pEntityClass->strGameType = entity_type;

			// No need to log it now.
			//m_pSystem->GetILog()->Log( "Registering ClassId: Type='%s' Clsid=%d Name='%s' Script='%s'\n",entity_type,clsid,tablename,scriptfile);
		}
	}
	return true;
}