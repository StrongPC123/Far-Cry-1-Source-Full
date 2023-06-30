////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EntityScript.cpp
//  Version:     v1.00
//  Created:     10/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CEntityScript class implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "EntityScript.h"
#include "Entity.h"

#include <IScriptSystem.h>
#include <IEntitySystem.h>
#include <IGame.h>

struct CScriptMethodsDump : public IScriptObjectDumpSink
{
	std::vector<CString> methods;
	std::vector<CString> events;
	virtual void OnElementFound(int nIdx,ScriptVarType type){}
	virtual void OnElementFound(const char *sName,ScriptVarType type)
	{
		if (type == svtFunction)
		{
			if (strncmp(sName,EVENT_PREFIX,6) == 0)
				events.push_back( sName+6 );
			else
				methods.push_back( sName );
		}/* else if (type == svtObject && stricmp(sName,PROPERTIES_TABLE)==0)
		{
			// Properties found.
		}
		*/
	}
};

enum EScriptParamFlags
{
	SCRIPTPARAM_POSITIVE = 0x01,
};

//////////////////////////////////////////////////////////////////////////
static struct {
	const char *prefix;
	IVariable::EType type;
	IVariable::EDataType dataType;
	int flags;
} s_paramTypes[] =
{
	{ "n",				IVariable::INT,			IVariable::DT_SIMPLE, SCRIPTPARAM_POSITIVE },
	{ "i",				IVariable::INT,			IVariable::DT_SIMPLE,0 },
	{ "b",				IVariable::BOOL,		IVariable::DT_SIMPLE,0 },
	{ "f",				IVariable::FLOAT,		IVariable::DT_SIMPLE,0 },
	{ "s",				IVariable::STRING,	IVariable::DT_SIMPLE,0 },

	{ "shader",		IVariable::STRING,	IVariable::DT_SHADER,0 },
	{ "clr",			IVariable::VECTOR,	IVariable::DT_COLOR,0 },
	{ "color",		IVariable::VECTOR,	IVariable::DT_COLOR,0 },

	{ "vector",		IVariable::VECTOR,	IVariable::DT_SIMPLE,0 },

	{ "snd",			IVariable::STRING,	IVariable::DT_SOUND,0 },
	{ "sound",		IVariable::STRING,	IVariable::DT_SOUND,0 },

	{ "tex",			IVariable::STRING,	IVariable::DT_TEXTURE,0 },
	{ "texture",	IVariable::STRING,	IVariable::DT_TEXTURE,0 },

	{ "obj",			IVariable::STRING,	IVariable::DT_OBJECT,0 },
	{ "object",		IVariable::STRING,	IVariable::DT_OBJECT,0 },

	{ "file",			IVariable::STRING,	IVariable::DT_FILE,0 },
	{ "aibehavior",	IVariable::STRING,IVariable::DT_AI_BEHAVIOR,0 },
	{ "aicharacter",IVariable::STRING,IVariable::DT_AI_CHARACTER,0 },

	{ "text",			IVariable::STRING,	IVariable::DT_LOCAL_STRING,0 },
	{ "equip",		IVariable::STRING,	IVariable::DT_EQUIP,0 },
	{ "sndpreset",IVariable::STRING,	IVariable::DT_SOUNDPRESET,0 },
	{ "eaxpreset",IVariable::STRING,	IVariable::DT_EAXPRESET,0 },

	{ "aianchor",IVariable::STRING,IVariable::DT_AI_ANCHOR,0 },
};

//////////////////////////////////////////////////////////////////////////
struct CScriptPropertiesDump : public IScriptObjectDumpSink
{
private:
	struct Variable {
		CString name;
		ScriptVarType type;
	};
	std::vector<Variable> m_elements;
	
	CVarBlock *m_varBlock;
	IVariable *m_parentVar;

public:
	explicit CScriptPropertiesDump( CVarBlock *pVarBlock,IVariable *pParentVar=NULL )
	{
		m_varBlock = pVarBlock;
		m_parentVar = pParentVar;
	}

	//////////////////////////////////////////////////////////////////////////
	inline bool IsPropertyTypeMatch( const char *type,const char *name,int nameLen )
	{
		int typeLen = strlen(type);
		if (typeLen < nameLen)
		{
			// After type name Must follow Upper case or _.
			if (name[typeLen] != tolower(name[typeLen]) || name[typeLen] == '_' )
			{
				if (strncmp(name,type,strlen(type)) == 0)
				{
					return true;
				}
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	IVariable* CreateVarByType( IVariable::EType type )
	{
		switch(type)
		{
		case IVariable::FLOAT:		return new CVariable<float>;
		case IVariable::INT:			return new CVariable<int>;
		case IVariable::STRING:	return new CVariable<CString>;
		case IVariable::BOOL:		return new CVariable<bool>;
		case IVariable::VECTOR:	return new CVariable<Vec3>;
		case IVariable::QUAT:		return new CVariable<Quat>;
		default:
			assert(0);
		}
		return NULL;
	}
	//////////////////////////////////////////////////////////////////////////
	IVariable* CreateVar( const char *name,IVariable::EType defaultType,const char* &displayName )
	{
		displayName = name;
		// Resolve type from variable name.
		int nameLen = strlen(name);

		// if starts from capital no type encoded.
		if (name[0] == tolower(name[0]))
		{
			// Try to detect type.
			for (int i = 0; i < sizeof(s_paramTypes)/sizeof(s_paramTypes[0]); i++)
			{
				if (IsPropertyTypeMatch(s_paramTypes[i].prefix,name,nameLen))
				{
					//if (s_paramTypes[i].type != var->GetType())
					//continue;
					displayName = name + strlen(s_paramTypes[i].prefix);
					if (displayName[0] == '_')
						displayName++;

					IVariable *var = CreateVarByType(s_paramTypes[i].type);
					if (!var)
						continue;

					var->SetName(name);
					var->SetHumanName(displayName);
					var->SetDataType(s_paramTypes[i].dataType);

					if (s_paramTypes[i].flags & SCRIPTPARAM_POSITIVE)
					{
						float lmin=0,lmax=10000;
						var->GetLimits( lmin,lmax );
						// set min Limit to 0 to make it positive only value.
						var->SetLimits( 0,lmax );
					}
					return var;
				}
			}
		}
		if (defaultType != IVariable::UNKNOWN)
		{
			IVariable *var = CreateVarByType(defaultType);
			var->SetName(name);
			return var;
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	void OnElementFound(int nIdx,ScriptVarType type)
	{
		/*ignore non string indexed values*/
	};
	//////////////////////////////////////////////////////////////////////////
	virtual void OnElementFound(const char *sName,ScriptVarType type)
	{
		if (sName && sName[0] != 0)
		{
			Variable var;
			var.name = sName;
			var.type = type;
			m_elements.push_back(var);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	void Dump( IScriptObject *pObject )
	{
		m_elements.reserve(20);
		pObject->Dump( this );
		std::map<CString,IVariablePtr> nodes;
		std::map<CString,IVariablePtr> listNodes;

		for (int i = 0; i < m_elements.size(); i++)
		{
			const char *sName = m_elements[i].name;
			ScriptVarType type = m_elements[i].type;

			const char *sDisplayName = sName;

			if (type == svtNumber)
			{
				float fVal;
				pObject->GetValue( sName,fVal );
				IVariable *var = CreateVar(sName,IVariable::FLOAT,sDisplayName);
				if (var)
				{
					var->Set(fVal);
					nodes[sDisplayName] = var;
				}
			} else if (type == svtString)
			{
				const char *sVal;
				pObject->GetValue( sName,sVal );
				IVariable *var = CreateVar(sName,IVariable::STRING,sDisplayName);
				if (var)
				{
					var->Set(sVal);
					nodes[sDisplayName] = var;
				}
			} else if (type == svtFunction)
			{
				// Ignore functions.
			} else if (type == svtObject)
			{
				// Some Table.
				_SmartScriptObject pTable(GetIEditor()->GetSystem()->GetIScriptSystem(),true);
				if (pObject->GetValue( sName,pTable ))
				{
					IVariable *var = CreateVar(sName,IVariable::UNKNOWN,sDisplayName);
					if (var && var->GetType() == IVariable::VECTOR)
					{
						nodes[sDisplayName] = var;
						float x,y,z;
						if (pTable->GetValue("x",x) && pTable->GetValue("y",y) && pTable->GetValue("z",z))
						{
							var->Set( Vec3(x,y,z) );
						}
						else
						{
							pTable->GetAt(1,x);
							pTable->GetAt(2,y);
							pTable->GetAt(3,z);
							var->Set( Vec3(x,y,z) );
						}
					}
					else
					{
						var = new CVariableArray;
						var->SetName(sName);
						listNodes[sName] = var;
						
						CScriptPropertiesDump dump(m_varBlock,var);
						dump.Dump( *pTable );
					}
				}
			}
		}

		for (std::map<CString,IVariablePtr>::iterator nit = nodes.begin(); nit != nodes.end(); nit++)
		{
			if (m_parentVar)
				m_parentVar->AddChildVar(nit->second);
			else
				m_varBlock->AddVariable(nit->second);
		}
		for (std::map<CString,IVariablePtr>::iterator nit1 = listNodes.begin(); nit1 != listNodes.end(); nit1++)
		{
			if (m_parentVar)
				m_parentVar->AddChildVar(nit1->second);
			else
				m_varBlock->AddVariable(nit1->second);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CEntityScript::CEntityScript( const EntityClassId ClassId,const char *sName,const char *sFile )
{
	m_ClassId = ClassId;
	m_valid = false;
	m_haveEventsTable = false;
	m_standart = false;
	m_visibilityMask = 0;
	m_usable = false;
	
	m_name = sName;
	m_file = sFile;
	m_relFile = sFile;

}

//////////////////////////////////////////////////////////////////////////
CEntityScript::~CEntityScript()
{
}

//////////////////////////////////////////////////////////////////////////
int	CEntityScript::GetEventCount()
{
	//return sizeof(sEntityEvents)/sizeof(sEntityEvents[0]);
	return m_events.size();
}

//////////////////////////////////////////////////////////////////////////
CString CEntityScript::GetEvent( int i )
{
	//return sEntityEvents[i].name;
	return m_events[i];
}

//////////////////////////////////////////////////////////////////////////
bool CEntityScript::Load()
{
	m_valid = false;
	IGame *pGame = GetIEditor()->GetGame();
	EntityClass *entCls = pGame->GetClassRegistry()->GetByClassId( m_ClassId );
	if (!entCls)
	{
		m_valid = false;
		//Warning( "Load of entity script %s failed.",(const char*)m_name );

		CErrorRecord err;
		err.error.Format( "Entity Script %s Failed to Load",(const char*)m_name );
		err.file = m_relFile;
		err.severity = CErrorRecord::ESEVERITY_WARNING;
		err.flags = CErrorRecord::FLAG_SCRIPT;
		GetIEditor()->GetErrorReport()->ReportError(err);

		return false;
	}

	m_standart = entCls->bReserved;
	m_relFile = entCls->strScriptFile.c_str();
	m_file = entCls->strFullScriptFile.c_str();
	m_ClassId = entCls->ClassId;

	if (m_file.IsEmpty())
	{
		m_valid = true;
		return true;
	}

	return ParseScript();
}

//////////////////////////////////////////////////////////////////////////
bool CEntityScript::ParseScript()
{
	// Parse .lua file.
	IScriptSystem *script = GetIEditor()->GetSystem()->GetIScriptSystem();

	_SmartScriptObject pEntity(script,true);
	if (!script->GetGlobalValue( m_name,*pEntity ))
		return false;

	m_valid = true;

	CScriptMethodsDump dump;
	pEntity->Dump( &dump );
	m_methods = dump.methods;
	m_events = dump.events;

	//! Sort methods and events.
	std::sort( m_methods.begin(),m_methods.end() );
	std::sort( m_events.begin(),m_events.end() );

	{
		// Normal properties.
		m_properties = 0;
		_SmartScriptObject pProps(script,true);
		if (pEntity->GetValue( PROPERTIES_TABLE,*pProps ))
		{
			// Properties found in entity.
			m_properties = new CVarBlock;
			CScriptPropertiesDump dump(m_properties);
			dump.Dump( *pProps );
		}
	}

	{
		// Second set of properties.
		m_properties2 = 0;
		_SmartScriptObject pProps(script,true);
		if (pEntity->GetValue( PROPERTIES2_TABLE,*pProps ))
		{
			// Properties found in entity.
			m_properties2 = new CVarBlock;
			CScriptPropertiesDump dump(m_properties2);
			dump.Dump( *pProps );
		}
	}

	// Destroy variable block if empty.
	if (m_properties != 0 && m_properties->GetVarsCount() < 1)
		m_properties = 0;

	// Destroy variable block if empty.
	if (m_properties2 != 0 && m_properties2->GetVarsCount() < 1)
		m_properties2 = 0;

	// Load visual object.
	_SmartScriptObject pEditorTable(script,true);
	if (pEntity->GetValue( "Editor",*pEditorTable ))
	{
		const char *modelName;
		if (pEditorTable->GetValue( "Model",modelName ))
		{
			m_visualObject = modelName;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CEntityScript::Reload()
{
	// First try compiling script and see if it have any errors.
	bool bLoadScript = CFileUtil::CompileLuaFile( GetFile() );

	if (bLoadScript)
	{
		EntityClass *entCls = GetIEditor()->GetGame()->GetClassRegistry()->GetByClassId( m_ClassId );
		if (entCls)
		{
			entCls->bLoaded = false;
		}
		else
			bLoadScript = false;
	}

	if (bLoadScript)
	{
		// Script compiled succesfully.
		Load();/*
		if (GetIEditor()->GetSystem()->GetIScriptSystem()->ReloadScript( GetFile(),false ))
		{
			ParseScript();
		}
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityScript::GotoMethod( const CString &method )
{
	CString line;
	line.Format( "%s:%s",(const char*)GetName(),(const char*)method );

	// Search this line in script file.
	int lineNum = FindLineNum( line );
	if (lineNum >= 0)
	{
		// Open UltraEdit32 with this line.
		CFileUtil::EditTextFile( GetFile(),lineNum );
	}
}

void CEntityScript::AddMethod( const CString &method )
{
	// Add a new method to the file. and start Editing it.
	FILE *f = fopen( GetFile(),"at" );
	if (f)
	{
		fprintf( f,"\n" );
		fprintf( f,"-------------------------------------------------------\n" );
		fprintf( f,"function %s:%s()\n",(const char*)m_name,(const char*)method );
		fprintf( f,"end\n" );
		fclose(f);
	}
}

//////////////////////////////////////////////////////////////////////////
int CEntityScript::FindLineNum( const CString &line )
{
	FILE *f = fopen( GetFile(),"rb" );
	if (!f)
		return -1;

	int lineFound = -1;
	int lineNum = 1;

	fseek( f,0,SEEK_END );
	int size = ftell(f);
	fseek( f,0,SEEK_SET );
	
	char *text = new char[size+16];
	fread( text,size,1,f );
	text[size] = 0;

	char *token = strtok( text,"\n" );
	while (token)
	{
		if (strstr( token,line ) != 0)
		{
			lineFound = lineNum;
			break;
		}
		token = strtok( NULL,"\n" );
		lineNum++;
	}
	fclose(f);
	delete []text;

	return lineFound;
}

//////////////////////////////////////////////////////////////////////////
void CEntityScript::SetProperties( IEntity *ientity,CVarBlock *vars,bool bCallUpdate )
{
	if (!IsValid())
		return;
	
	assert( ientity != 0 );
	assert( vars != 0 );

	IScriptObject *scriptObject = ientity->GetScriptObject();
	if (!scriptObject)
		return;

	IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();

	_SmartScriptObject pProperties( scriptSystem,true);
	if (!scriptObject->GetValue( PROPERTIES_TABLE,*pProperties ))
	{
		return;
	}

	for (int i = 0; i < vars->GetVarsCount(); i++)
	{
		VarToScriptObject( vars->GetVariable(i),pProperties );
	}

	if (bCallUpdate)
	{
		HSCRIPTFUNCTION pf;
		if (scriptObject->GetValue( "OnPropertyChange",pf ))
		{
			scriptSystem->BeginCall(pf);
			scriptSystem->PushFuncParam(scriptObject);
			scriptSystem->EndCall();
			//Alberto
			scriptSystem->ReleaseFunc(pf);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityScript::SetProperties2( IEntity *ientity,CVarBlock *vars,bool bCallUpdate )
{
	if (!IsValid())
		return;
	
	assert( ientity != 0 );
	assert( vars != 0 );

	IScriptObject *scriptObject = ientity->GetScriptObject();
	if (!scriptObject)
		return;

	IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();

	_SmartScriptObject pProperties( scriptSystem,true);
	if (!scriptObject->GetValue( PROPERTIES2_TABLE,*pProperties ))
	{
		return;
	}

	for (int i = 0; i < vars->GetVarsCount(); i++)
	{
		VarToScriptObject( vars->GetVariable(i),pProperties );
	}

	if (bCallUpdate)
	{
		HSCRIPTFUNCTION pf;
		if (scriptObject->GetValue( "OnPropertyChange",pf ))
		{
			scriptSystem->BeginCall(pf);
			scriptSystem->PushFuncParam(scriptObject);
			scriptSystem->EndCall();
			//Alberto
			scriptSystem->ReleaseFunc(pf);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CEntityScript::VarToScriptObject( IVariable *var,IScriptObject *obj )
{
	assert(var);

	if (var->GetType() == IVariable::ARRAY)
	{
		int type = obj->GetValueType( var->GetName() );
		if (type != svtObject)
			return;

		IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
		_SmartScriptObject pTableObj( scriptSystem,true );
		if (obj->GetValue( var->GetName(),*pTableObj ))
		{
			for (int i = 0; i < var->NumChildVars(); i++)
			{
				IVariable *child = var->GetChildVar(i);
				VarToScriptObject( child,pTableObj );
			}
		}
		return;
	}

	const char *name = var->GetName();
	int type = obj->GetValueType( name );

	if (type == svtString)
	{
		CString value;
		var->Get(value);
		obj->SetValue( name,value );
	}
	else if (type == svtNumber)
	{
		float val = 0;
		var->Get(val);
		obj->SetValue( name,val );
	}
	else if (type == svtObject)
	{
		IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
		// Probably Color/Vector.
		_SmartScriptObject pTable( scriptSystem,true );
		if (obj->GetValue( name,pTable ))
		{
			if (var->GetType() == IVariable::VECTOR)
			{
				Vec3 vec;
				var->Get(vec);

				float temp;
				if (pTable->GetValue( "x",temp ))
				{
					// Named vector.
					pTable->SetValue( "x",vec.x );
					pTable->SetValue( "y",vec.y );
					pTable->SetValue( "z",vec.z );
				}
				else
				{
					// Indexed vector.
					pTable->SetAt(1,vec.x);
					pTable->SetAt(2,vec.y);
					pTable->SetAt(3,vec.z);
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void	CEntityScript::RunMethod( IEntity *ientity,const CString &method )
{
	if (!IsValid())
		return;

	assert( ientity != 0 );

	IScriptObject *scriptObject = ientity->GetScriptObject();
	if (!scriptObject)
		return;

	IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();

	scriptSystem->BeginCall( GetName(),method );
	scriptSystem->PushFuncParam( scriptObject );
	scriptSystem->EndCall();
}

//////////////////////////////////////////////////////////////////////////
void	CEntityScript::SendEvent( IEntity *entity,const CString &method )
{
	RunMethod( entity,CString(EVENT_PREFIX)+method );
}

//////////////////////////////////////////////////////////////////////////
void	CEntityScript::SetEventsTable( CEntity *entity )
{
	if (!IsValid())
		return;
	assert( entity != 0 );

	IEntity *ientity = entity->GetIEntity();
	if (!ientity)
		return;

	IScriptObject *scriptObject = ientity->GetScriptObject();
	if (!scriptObject)
		return;

	// If events target table is null, set event table to null either.
	if (entity->GetEventTargetCount() == 0)
	{
		if (m_haveEventsTable)
		{
			scriptObject->SetToNull( "Events" );
		}
		m_haveEventsTable = false;
		return;
	}

	IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
	_SmartScriptObject pEvents( scriptSystem,false );

	scriptObject->SetValue( "Events",*pEvents );
	m_haveEventsTable = true;
	
	std::set<CString> sourceEvents;
	for (int i = 0; i < entity->GetEventTargetCount(); i++)
	{
		CEntityEventTarget &et = entity->GetEventTarget(i);
		sourceEvents.insert( et.sourceEvent );
	}
	for (std::set<CString>::iterator it = sourceEvents.begin(); it != sourceEvents.end(); it++)
	{
		_SmartScriptObject pTrgEvents( scriptSystem,false );
		CString sourceEvent = *it;

		//int srcEventId = EventNameToId( sourceEvent );
		//pEvents->SetAt( srcEventId,*pTrgEvents );
		pEvents->SetValue( sourceEvent,*pTrgEvents );

		// Put target events to table.
		int trgEventIndex = 1;
		for (int i = 0; i < entity->GetEventTargetCount(); i++)
		{
			CEntityEventTarget &et = entity->GetEventTarget(i);
			if (stricmp(et.sourceEvent,sourceEvent) == 0)
			{
				int entityId = 0;
				if (et.target)
				{
					if (et.target->IsKindOf( RUNTIME_CLASS(CEntity) ))
						entityId = ((CEntity*)et.target)->GetEntityId();
				}

				_SmartScriptObject pTrgEvent( scriptSystem,false );
				pTrgEvents->SetAt( trgEventIndex,*pTrgEvent );
				trgEventIndex++;

				pTrgEvent->SetAt( 1,entityId );
				pTrgEvent->SetAt( 2,et.event );
			}
		}
	}
	
}

//////////////////////////////////////////////////////////////////////////
// CEntityScriptRegistry implementation.
//////////////////////////////////////////////////////////////////////////
CEntityScriptRegistry* CEntityScriptRegistry::m_instance = 0;

CEntityScriptRegistry::CEntityScriptRegistry()
{
	m_instance = this;
}

CEntityScriptRegistry::~CEntityScriptRegistry()
{
	m_instance = 0;
	m_scripts.Clear();
}

CEntityScript* CEntityScriptRegistry::Find( const CString &name )
{
	CEntityScriptPtr script = 0;
	if (m_scripts.Find( name,script ))
	{
		return script;
	}
	return 0;
}
	
void CEntityScriptRegistry::Insert( CEntityScript *script )
{
	// Check if inserting already exist script, if so ignore.
	CEntityScriptPtr temp;
	if (m_scripts.Find( script->GetName(),temp ))
	{
		Error( "Inserting duplicate Entity Script %s",(const char*)script->GetName() );
		return;
	}
	m_scripts[script->GetName()] = script;
}

void CEntityScriptRegistry::GetScripts( std::vector<CEntityScript*> &scripts )
{
	std::vector<CEntityScriptPtr> s;
	m_scripts.GetAsVector( s );

	scripts.resize( s.size() );
	for (int i = 0; i < s.size(); i++)
	{
		scripts[i] = s[i];
	}
}

void CEntityScriptRegistry::LoadScripts()
{
	IGame *game = GetIEditor()->GetGame();
	if (!game)
		return;

	m_scripts.Clear();

	EntityClass *entCls;
	// Enumerate entity classes inside Game.
	game->GetClassRegistry()->MoveFirst();
	do {
		entCls = game->GetClassRegistry()->Next();
		if (entCls)
		{
			CEntityScript *script = new CEntityScript( entCls->ClassId,entCls->strClassName.c_str(),entCls->strScriptFile.c_str() );
			if (!entCls->strScriptFile.empty())
				script->SetUsable( true );
			Insert( script );
		}
	} while (entCls);

/*
	int lastClassId = FIRST_ENTITY_CLASS_ID;

	//////////////////////////////////////////////////////////////////////////
	// Load class registry.
	// Enumerate all XML files in Editor\ subfolder to look for EntityRegistry.
	//////////////////////////////////////////////////////////////////////////
	
	CString masterCD = GetIEditor()->GetMasterCDFolder();
	std::vector<CFileUtil::FileDesc> files;
	CString dir = Path::AddBackslash(masterCD) + "Editor\\";
	CFileUtil::ScanDirectory( dir,"*.xml",files,false );

	XmlParser parser;

	FILE *file;
	file = fopen( "Scripts\\classreg1.lua","wt" );

	for (int k = 0; k < files.size(); k++)
	{
		// Construct the full filepath of the current file
		XmlNodeRef registry = parser.parse( dir + files[k].filename );
		if (registry != 0 && registry->isTag("EntityRegistry"))
		{
			for (int i = 0; i < registry->getChildCount(); i++)
			{
				XmlNodeRef child = registry->getChild(i);
				if (child->isTag("EntityDesc"))
				{
					const char *fname = child->getAttr( "File" );
					const char *tableName = child->getAttr( "Name" );
					// Save new format.
					fprintf( file,"\t{ \"\",\t\"%s\",\t%.3d,\t\"%s\" },\n",tableName,i+100,fname );


					CString file,name;
					int clsId;
					if (child->getAttr( "File",file ) &&
						child->getAttr( "Name",name ))
					{
						clsId = lastClassId++;

						CEntityScript* script = Find(name);
						if (script)
						{
							script->Invalidate();
						}
						else
						{
							script = new CEntityScript( clsId,name,file );

							// Only scripts from entity registry can be used.
							script->SetUsable( true );
							Insert( script );
						}
					}
				}
			}
		}
	}
	fclose( file );
	*/
}

//////////////////////////////////////////////////////////////////////////
CEntityScriptRegistry* CEntityScriptRegistry::Instance()
{
	if (!m_instance)
	{
		m_instance = new CEntityScriptRegistry;
	}
	return m_instance;
}

//////////////////////////////////////////////////////////////////////////
void CEntityScriptRegistry::Release()
{
	if (m_instance)
	{
		delete m_instance;
	}
	m_instance = 0;
}