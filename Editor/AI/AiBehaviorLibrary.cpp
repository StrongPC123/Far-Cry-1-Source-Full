////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   AiBehaviorLibrary.h
//  Version:     v1.00
//  Created:     21/3/2002 by Timur.
//  Compilers:   Visual C++ 7.0
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "AiBehaviorLibrary.h"
#include "AiBehavior.h"

#include "IScriptSystem.h"

#define AI_CONFIG_FILE "Scripts\\AI\\aiconfig.lua"

//////////////////////////////////////////////////////////////////////////
struct CAIBehaviorsDump : public IScriptObjectDumpSink
{
	CAIBehaviorsDump( CAIBehaviorLibrary* lib,IScriptObject *obj )
	{
		m_pLibrary = lib;
		m_pScriptObject = obj;
	}

	virtual void OnElementFound(int nIdx,ScriptVarType type) {}
	virtual void OnElementFound(const char *sName,ScriptVarType type)
	{
		if (type == svtString)
		{
			// New behavior.
			const char *scriptFile;
			if (m_pScriptObject->GetValue(sName,scriptFile))
			{
				CAIBehavior *behavior = new CAIBehavior;
				behavior->SetName(sName);
				CString file = scriptFile;
				file.Replace( '/','\\' );
				behavior->SetScript(file);
				behavior->SetDescription( "" );
				m_pLibrary->AddBehavior( behavior );
			}
		}
	}
private:
	CAIBehaviorLibrary *m_pLibrary;
	IScriptObject *m_pScriptObject;
};

//////////////////////////////////////////////////////////////////////////
struct CAICharactersDump : public IScriptObjectDumpSink
{
	CAICharactersDump( CAIBehaviorLibrary* lib,IScriptObject *obj )
	{
		m_pLibrary = lib;
		m_pScriptObject = obj;
	}

	virtual void OnElementFound(int nIdx,ScriptVarType type) {}
	virtual void OnElementFound(const char *sName,ScriptVarType type)
	{
		if (type == svtString)
		{
			// New behavior.
			const char *scriptFile;
			if (m_pScriptObject->GetValue(sName,scriptFile))
			{
				CAICharacter* c = new CAICharacter;
				c->SetName(sName);
				CString file = scriptFile;
				file.Replace( '/','\\' );
				c->SetScript(file);
				c->SetDescription( "" );
				m_pLibrary->AddCharacter( c );
			}
		}
	}
private:
	CAIBehaviorLibrary *m_pLibrary;
	IScriptObject *m_pScriptObject;
};

//////////////////////////////////////////////////////////////////////////
// CAIBehaviorLibrary implementation.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CAIBehaviorLibrary::CAIBehaviorLibrary()
{
}

//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::AddBehavior( CAIBehavior* behavior )
{
	CAIBehaviorPtr g;
	if (m_behaviors.Find(behavior->GetName(),g))
	{
		// Behavior with this name already exist in the library.
	}
	m_behaviors[behavior->GetName()] = behavior;
}

	
//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::RemoveBehavior( CAIBehavior* behavior )
{
	m_behaviors.Erase( behavior->GetName() );
}

//////////////////////////////////////////////////////////////////////////
CAIBehavior* CAIBehaviorLibrary::FindBehavior( const CString &name ) const
{
	CAIBehaviorPtr behavior=0;
	m_behaviors.Find( name,behavior );
	return behavior;
}

//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::ClearBehaviors()
{
	m_behaviors.Clear();
}

//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::LoadBehaviors( const CString &path )
{
	m_scriptsPath = path;

	ClearBehaviors();

	IScriptSystem *pScriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
	
	_SmartScriptObject pAIBehaviorTable( pScriptSystem,true );
	if (pScriptSystem->GetGlobalValue( "AIBehaviour",pAIBehaviorTable ))
	{
		_SmartScriptObject pAvailableTable( pScriptSystem,true );
		if (pAIBehaviorTable->GetValue( "AVAILABLE",pAvailableTable ))
		{
			// Enumerate all behaviours.
			CAIBehaviorsDump bdump( this,pAvailableTable );
			pAvailableTable->Dump( &bdump );
		}
	}
/*
	int i;
	XmlParser xmlParser;

	// Scan all behavior files.
	std::vector<CString> files;
	CFileEnum::ScanDirectory( path,"*.ai",files,true );
	for (i = 0; i < files.size(); i++)
	{
		// Load behavior scripts.
		CString file = path + files[i];
		XmlNodeRef behaviorsNode = xmlParser.parse(file);
		if (!behaviorsNode)
			continue;

		for (int j = 0; j < behaviorsNode->getChildCount(); j++)
		{
			XmlNodeRef bhNode = behaviorsNode->getChild(j);
			CString name,script,desc;
			bhNode->getAttr( "Name",name );
			bhNode->getAttr( "Script",script );
			bhNode->getAttr( "Description",desc );
			
			char fdir[_MAX_PATH];
			_splitpath(files[i],0,fdir,0,0 );
			CString scriptFile = path + fdir + script;

			// New behavior.
			CAIBehavior *behavior = new CAIBehavior;
			behavior->SetName(name);
			behavior->SetScript(scriptFile);
			behavior->SetDescription( desc );
			AddBehavior( behavior );
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::GetBehaviors( std::vector<CAIBehaviorPtr> &behaviors )
{
	// Load behaviours again.
	LoadBehaviors( m_scriptsPath );
	m_behaviors.GetAsVector(behaviors);
}

//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::ReloadScripts()
{
	LoadBehaviors( m_scriptsPath );

	int i;
	// Generate uniq set of script files used by behaviors.
	std::set<CString> scriptFiles;

	std::vector<CAIBehaviorPtr> behaviors;
	GetBehaviors( behaviors );
	for (i = 0; i < behaviors.size(); i++)
	{
		scriptFiles.insert( behaviors[i]->GetScript() );
	}

	IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
	// Load script files to script system.
	for (std::set<CString>::iterator it = scriptFiles.begin(); it != scriptFiles.end(); it++)
	{
		CString file = *it;
		CLogFile::FormatLine( "Loading AI Behavior Script: %s",(const char*)file );
		scriptSystem->ExecuteFile( file );
	}

	// Reload main AI script.
//	IScriptSystem *scriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
	scriptSystem->ExecuteFile( AI_CONFIG_FILE,true,true );
}

//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::LoadCharacters()
{
	m_characters.Clear();

	IScriptSystem *pScriptSystem = GetIEditor()->GetSystem()->GetIScriptSystem();
	
	_SmartScriptObject pAIBehaviorTable( pScriptSystem,true );
	if (pScriptSystem->GetGlobalValue( "AICharacter",pAIBehaviorTable ))
	{
		_SmartScriptObject pAvailableTable( pScriptSystem,true );
		if (pAIBehaviorTable->GetValue( "AVAILABLE",pAvailableTable ))
		{
			// Enumerate all characters.
			CAICharactersDump bdump( this,pAvailableTable );
			pAvailableTable->Dump( &bdump );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::GetCharacters( std::vector<CAICharacterPtr> &characters )
{
	LoadCharacters();
	m_characters.GetAsVector(characters);
}

//////////////////////////////////////////////////////////////////////////
CAICharacter* CAIBehaviorLibrary::FindCharacter( const CString &name ) const
{
	CAICharacterPtr chr=0;
	m_characters.Find( name,chr );
	return chr;
}

//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::AddCharacter( CAICharacter* chr )
{
	CAICharacterPtr g;
	if (m_characters.Find(chr->GetName(),g))
	{
		// Behavior with this name already exist in the library.
	}
	m_characters[chr->GetName()] = chr;
}

	
//////////////////////////////////////////////////////////////////////////
void CAIBehaviorLibrary::RemoveCharacter( CAICharacter* chr )
{
	m_characters.Erase( chr->GetName() );
}