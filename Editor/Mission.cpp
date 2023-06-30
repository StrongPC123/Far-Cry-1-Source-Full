////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Mission.cpp
//  Version:     v1.00
//  Created:     14/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CMission class implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Mission.h"
#include "CryEditDoc.h"
#include "TerrainLighting.h"
#include "GameEngine.h"
#include "MissionScript.h"
#include "imoviesystem.h"
#include "Objects\ObjectManager.h"

#include "Objects\Entity.h"
#include "Objects\EntityScript.h"

#include <IEntitySystem.h>
#include <IGame.h>
#include <I3DEngine.h>

//////////////////////////////////////////////////////////////////////////
CMission::CMission( CCryEditDoc *doc )
{
	m_doc = doc;
	m_objects = new CXmlNode( "Objects" );
	m_layers = new CXmlNode( "ObjectLayers" );
	//m_exportData = XmlNodeRef( "ExportData" );
	m_weaponsAmmo = new CXmlNode( "Ammo" );
	m_Animations = new CXmlNode("MovieData");
	m_environment = new CXmlNode( "Environment" );
	CXmlTemplate::SetValues( m_doc->GetEnvironmentTemplate(),m_environment );

	m_time = 0;

	m_lighting = new LightingSettings;			// default values are set in the constructor

	m_sMusicScript="";

	m_pScript=new CMissionScript();
	m_numCGFObjects = 0;
}

//////////////////////////////////////////////////////////////////////////
CMission::~CMission()
{
	delete m_lighting;
	delete m_pScript;
}

//////////////////////////////////////////////////////////////////////////
CMission* CMission::Clone()
{
	CMission *m = new CMission(m_doc);
	m->SetName( m_name );
	m->SetDescription( m_description );
	m->m_objects = m_objects->clone();
	m->m_layers = m_layers->clone();
	m->m_environment = m_environment->clone();
	m->m_time = m_time;
	m->m_sMusicScript=m_sMusicScript;
	return m;
}

//////////////////////////////////////////////////////////////////////////
void CMission::SetMusicScriptFilename(const CString &sMusicScript)
{
	m_sMusicScript=sMusicScript;
	if (m_sMusicScript.GetLength())
	{
		//if (!GetIEditor()->GetSystem()->LoadMusicDataFromLUA(m_sMusicScript))
			//GetIEditor()->GetSystem()->GetILog()->Log("WARNING: Cannot load music-script %s !", m_sMusicScript);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMission::Serialize( CXmlArchive &ar )
{
	if (ar.bLoading)
	{
		// Load.
		ar.root->getAttr( "Name",m_name );
		ar.root->getAttr( "Description",m_description );

		//time_t time = 0;
		//ar.root->getAttr( "Time",time );
		//m_time = time;

		m_sPlayerEquipPack="";
		ar.root->getAttr("PlayerEquipPack", m_sPlayerEquipPack);
		CString sMusicScript="";
		ar.root->getAttr("MusicScript", sMusicScript);
		SetMusicScriptFilename(sMusicScript);
		CString sScript="";
		ar.root->getAttr("Script", sScript);
		m_pScript->SetScriptFile( sScript );

		XmlNodeRef objects = ar.root->findChild( "Objects" );
		if (objects)
			m_objects = objects;

		XmlNodeRef layers = ar.root->findChild( "ObjectLayers" );
		if (layers)
			m_layers = layers;

		XmlNodeRef anims = ar.root->findChild( "MovieData" );
		if (anims)
		{
			m_Animations = anims;
		}else
			m_Animations=new CXmlNode("MovieData");

		/*
		XmlNodeRef expData = ar.root->findChild( "ExportData" );
		if (expData)
		{
			m_exportData = expData;
		}
		*/

		XmlNodeRef env = ar.root->findChild( "Environment" );
		if (env)
		{
			m_environment = env;
		}

		m_lighting->Serialize( ar.root,ar.bLoading );

		m_usedWeapons.clear();
	
		XmlNodeRef weapons = ar.root->findChild("Weapons");
		if (weapons)
		{
			XmlNodeRef usedWeapons = weapons->findChild("Used");
			if (usedWeapons)
			{
				CString weaponName;
				for (int i = 0; i < usedWeapons->getChildCount(); i++)
				{
					XmlNodeRef weapon = usedWeapons->getChild(i);
					if (weapon->getAttr( "Name",weaponName ))
						m_usedWeapons.push_back(weaponName);
				}
			}
			XmlNodeRef ammo = weapons->findChild("Ammo");
			if (ammo)
				m_weaponsAmmo = ammo->clone();
		}
	}
	else
	{
		ar.root->setAttr( "Name",m_name );
		ar.root->setAttr( "Description",m_description );

		//time_t time = m_time.GetTime();
		//ar.root->setAttr( "Time",time );

		ar.root->setAttr("PlayerEquipPack", m_sPlayerEquipPack);
		ar.root->setAttr("MusicScript", m_sMusicScript);
		ar.root->setAttr("Script", m_pScript->GetFilename());

		CString timeStr;
		timeStr.Format( "%.2d:%.2d",m_time.GetHour(),m_time.GetMinute() );
		ar.root->setAttr( "MissionTime",timeStr );

		// Saving.
		XmlNodeRef layers = m_layers->clone();
		layers->setTag( "ObjectLayers" );
		ar.root->addChild( layers );

		///XmlNodeRef objects = m_objects->clone();
		m_objects->setTag( "Objects" );
		ar.root->addChild( m_objects );

		//XmlNodeRef anims = m_Animations->clone();
		ar.root->addChild( m_Animations );

		//ar.root->addChild( m_exportData->clone() );

		XmlNodeRef env = m_environment->clone();
		env->setTag( "Environment" );
		ar.root->addChild( env );

		m_lighting->Serialize( ar.root,ar.bLoading );

		XmlNodeRef weapons = ar.root->newChild("Weapons");
		XmlNodeRef usedWeapons = weapons->newChild("Used");
		for (int i = 0; i < m_usedWeapons.size(); i++)
		{
			XmlNodeRef weapon = usedWeapons->newChild( "Weapon" );
			weapon->setAttr( "Name",m_usedWeapons[i] );
		}
		weapons->addChild( m_weaponsAmmo->clone() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMission::Export( XmlNodeRef &root )
{
	// Also save exported objects data.
	root->setAttr( "Name",m_name );
	root->setAttr( "Description",m_description );

	CString timeStr;
	timeStr.Format( "%.2d:%.2d",m_time.GetHour(),m_time.GetMinute() );
	root->setAttr( "Time",timeStr );

	root->setAttr("PlayerEquipPack", m_sPlayerEquipPack);
	root->setAttr("MusicScript", m_sMusicScript);
	root->setAttr("Script", m_pScript->GetFilename());
		
	// Saving.
	//XmlNodeRef objects = m_exportData->clone();
	//objects->setTag( "Objects" );
	//root->addChild( objects );
		
	XmlNodeRef envNode = m_environment->clone();
	m_lighting->Serialize( envNode,false );
	root->addChild( envNode );

	XmlNodeRef weapons = root->newChild("Weapons");
	XmlNodeRef usedWeapons = weapons->newChild("Used");
	for (int i = 0; i < m_usedWeapons.size(); i++)
	{
		XmlNodeRef weapon = usedWeapons->newChild( "Weapon" );
		weapon->setAttr( "Name",m_usedWeapons[i] );
		IEntity *pEnt=GetIEditor()->GetSystem()->GetIEntitySystem()->GetEntity(m_usedWeapons[i]);
		if (pEnt)
			weapon->setAttr( "id",pEnt->GetId());
	}
	weapons->addChild( m_weaponsAmmo->clone() );

	IObjectManager *pObjMan = GetIEditor()->GetObjectManager();

	//////////////////////////////////////////////////////////////////////////
	// Serialize entity descriptions.
	//////////////////////////////////////////////////////////////////////////
	XmlNodeRef entityDescriptions = root->newChild( "EntityDescriptions" );
	// Get entity scripts of all used entities.
	std::set<CEntityScript*> scripts;
	std::vector<CBaseObject*> objects;
	pObjMan->GetObjects( objects );
	for (i = 0; i < objects.size(); i++)
	{
		CBaseObject *obj = objects[i];
		if (obj->GetType() == OBJTYPE_ENTITY)
		{
			CEntityScript *script = ((CEntity*)obj)->GetScript();
			if (script)
				scripts.insert(script);
		}
	}

	// Save entity class info.
	int clsId = FIRST_ENTITY_CLASS_ID;
	for (std::set<CEntityScript*>::iterator it = scripts.begin(); it != scripts.end(); it++)
	{
		CEntityScript *script = *it;
		if (!script->IsStandart() && script->IsValid())
		{
			XmlNodeRef ed = entityDescriptions->newChild( "EntityDesc" );
			ed->setAttr( "ClassId",clsId++ );
			ed->setAttr( "Name",script->GetName() );
			ed->setAttr( "File",script->GetRelativeFile() );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Serialize objects.
	//////////////////////////////////////////////////////////////////////////
	XmlNodeRef objectsNode = root->newChild("Objects");
	char path[1024];
	strcpy( path,m_doc->GetPathName() );
	PathRemoveFileSpec(path);
	PathAddBackslash(path);
	pObjMan->Export( path,objectsNode,true ); // Export shared.
	pObjMan->Export( path,objectsNode,false );	// Export not shared.

	/*
	CObjectManager objectManager;
	XmlNodeRef loadRoot = root->createNode("Root");
	loadRoot->addChild( m_objects );

	std::vector<CObjectClassDesc*> classes;
	GetIEditor()->GetObjectManager()->GetClasses( classes );
	objectManager.SetClasses( classes );
	objectManager.SetCreateGameObject(false);
	objectManager.Serialize( loadRoot,true,SERIALIZE_ALL );
	objectManager.Export( path,objects,false );
	*/
}

//////////////////////////////////////////////////////////////////////////
void CMission::ExportAnimations( XmlNodeRef &root )
{
	XmlNodeRef mission=new CXmlNode("Mission");
	mission->setAttr("Name", m_name);
	for (int i=0;i<m_Animations->getChildCount();i++)
	{
		mission->addChild(m_Animations->getChild(i));
	}
	root->addChild(mission);
}

//////////////////////////////////////////////////////////////////////////
void CMission::SyncContent( bool bRetrieve,bool bIgnoreObjects )
{
	// Save data from current Document to Mission.
	IObjectManager *objMan = GetIEditor()->GetObjectManager();
	if (bRetrieve)
	{
		// Activating this mission.
		CGameEngine *gameEngine = GetIEditor()->GetGameEngine();
		

		// Load mission script.
		if (m_pScript)
			m_pScript->Load();

		if (!bIgnoreObjects)
		{
			// Retrieve data from Mission and put to document.
			XmlNodeRef root = new CXmlNode("Root");
			root->addChild( m_objects );
			root->addChild( m_layers );
			objMan->Serialize( root,true,SERIALIZE_ONLY_NOTSHARED );
		}

		m_doc->GetFogTemplate() = m_environment;

		CXmlTemplate::GetValues( m_doc->GetEnvironmentTemplate(),m_environment );

//		GetIEditor()-> void CTrackViewDialog::SetCurrentSequence( IAnimSequence *curr )
		GetIEditor()->GetMovieSystem()->Serialize(m_Animations, true);

		UpdateUsedWeapons();
		gameEngine->SetPlayerEquipPack(m_sPlayerEquipPack);

		gameEngine->ReloadEnvironment();

		gameEngine->LoadAI( gameEngine->GetLevelPath(),GetName() );
		objMan->SendEvent( EVENT_MISSION_CHANGE );
		m_doc->ChangeMission();

		m_numCGFObjects = GetIEditor()->Get3DEngine()->GetLoadedObjectCount();
	}
	else
	{
		if (!bIgnoreObjects)
		{
			XmlNodeRef root = new CXmlNode("Root");
			objMan->Serialize( root,false,SERIALIZE_ONLY_NOTSHARED );
			m_objects = root->findChild("Objects");
			XmlNodeRef layers = root->findChild("ObjectLayers");
			if (layers)
				m_layers = layers;
		}

		// Also save exported objects data.
		//char path[1024];
		//strcpy( path,m_doc->GetPathName() );
		//PathRemoveFileSpec(path);
		//PathAddBackslash(path);

		m_Animations=new CXmlNode("MovieData");
		GetIEditor()->GetMovieSystem()->Serialize(m_Animations, false);
		
		//m_exportData = XmlNodeRef( "ExportData" );
		//objMan->Export( path,m_exportData,false );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMission::OnEnvironmentChange()
{
	m_environment = new CXmlNode( "Environment" );
	CXmlTemplate::SetValues( m_doc->GetEnvironmentTemplate(),m_environment );
}

//////////////////////////////////////////////////////////////////////////
void CMission::GetUsedWeapons( std::vector<CString> &weapons )
{
	weapons = m_usedWeapons;
}

//////////////////////////////////////////////////////////////////////////
void CMission::SetUsedWeapons( const std::vector<CString> &weapons )
{
	m_usedWeapons = weapons;
	UpdateUsedWeapons();
}

//////////////////////////////////////////////////////////////////////////
void CMission::UpdateUsedWeapons()
{
	IGame *game = GetIEditor()->GetGame();
	//game->RemoveAllWeapons();


	
	for (int i = 0; i < m_usedWeapons.size(); i++)
	{
		CLogFile::FormatLine( "Using Weapon: %s",(const char*)m_usedWeapons[i] );
		
#ifndef _ISNOTFARCRY
			GetIXGame( game )->AddWeapon( m_usedWeapons[i] );
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
void CMission::ResetScript()
{
	if (m_pScript)
	{
		m_pScript->OnReset();
	}
}

//////////////////////////////////////////////////////////////////////////
void CMission::AddObjectsNode( XmlNodeRef &node )
{
	for (int i = 0; i < node->getChildCount(); i++)
	{
		m_objects->addChild( node->getChild(i)->clone() );
	}
}

//////////////////////////////////////////////////////////////////////////
void CMission::SetLayersNode( XmlNodeRef &node )
{
	m_layers = node->clone();
}
