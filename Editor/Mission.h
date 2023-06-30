////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Mission.h
//  Version:     v1.00
//  Created:     14/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Mission class definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Mission_h__
#define __Mission_h__

#if _MSC_VER > 1000
#pragma once
#endif

// forward declaratsion.
struct LightingSettings;
class CMissionScript;

/*!	
		CMission represent single Game Mission on same map.
		Multiple Missions share same map, and stored in one .cry file.
		
 */
class CMission
{
public:
	//! Ctor of mission.
	CMission( CCryEditDoc *doc );
	//! Dtor of mission.
	virtual ~CMission();

	void SetName( const CString &name ) { m_name = name; }
	const CString& GetName() const { return m_name; }

	void SetDescription( const CString &dsc ) { m_description = dsc; }
	const CString& GetDescription() const { return m_description; }

	XmlNodeRef GetEnvironemnt() { return m_environment; };

	//! Return weapons ammo definitions for this mission.
	XmlNodeRef GetWeaponsAmmo() { return m_weaponsAmmo; };

	//! Return lighting used for this mission.
	LightingSettings*	GetLighting() const { return m_lighting; };

	//! Used weapons.
	void GetUsedWeapons( std::vector<CString> &weapons );
	void SetUsedWeapons( const std::vector<CString> &weapons );

	void SetTime( const CTime &time ) { m_time = time; };
	const CTime& GetTime() const { return m_time; };

	void SetPlayerEquipPack(const CString &sEquipPack) { m_sPlayerEquipPack=sEquipPack; }
	const CString& GetPlayerEquipPack() { return m_sPlayerEquipPack; }

	CMissionScript* GetScript() { return m_pScript; }

	void SetMusicScriptFilename(const CString &sMusicScript);
	const CString& GetMusicScriptFilename() { return m_sMusicScript; }

	//! Call OnReset callback
	void ResetScript();

	//! Called when this mission must be synchonized with current data in Document.
	//! if bRetrieve is true, data is retrieved from Mission to global structures.
	void	SyncContent( bool bRetrieve,bool bIgnoreObjects );

	//! Create clone of this mission.
	CMission*	Clone();

	//! Serialize mission.
	void Serialize( class CXmlArchive &ar );

	//! Export mission to game.
	void Export( XmlNodeRef &root );

	//! Export mission-animations to game.
	void ExportAnimations( XmlNodeRef &root );

	//! Add shared objects to mission objects.
	void AddObjectsNode( XmlNodeRef &node );
	void SetLayersNode( XmlNodeRef &node );

	void OnEnvironmentChange();
	int GetNumCGFObjects() const { return m_numCGFObjects; };
	
private:
	//! Update used weapons in game.
	void UpdateUsedWeapons();

	//! Document owner of this mission.
	CCryEditDoc *m_doc;

	CString m_name;
	CString m_description;

	CString m_sPlayerEquipPack;

	CString m_sMusicScript;

	//! Mission time;
	CTime	m_time;

	//! Root node of objects defined only in this mission.
	XmlNodeRef m_objects;

	//! Object layers.
	XmlNodeRef m_layers;

	//! Exported data of this mission.
	XmlNodeRef m_exportData;

	//! Environment settings of this mission.
	XmlNodeRef m_environment;

	//! Weapons ammo definition.
	XmlNodeRef m_weaponsAmmo;

	//! Animation definition
	XmlNodeRef m_Animations;

	//! Weapons used by this mission.
	std::vector<CString> m_usedWeapons;

	LightingSettings *m_lighting;

	//! MissionScript Handling
	CMissionScript *m_pScript;
	int m_numCGFObjects;
};


#endif // __Mission_h__
