////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   EntityScript.h
//  Version:     v1.00
//  Created:     10/12/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: EntityScript definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __EntityScript_h__
#define __EntityScript_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IEntitySystem.h"				// EntityClassId

#define PROPERTIES_TABLE "Properties"
#define PROPERTIES2_TABLE "PropertiesInstance"
#define FIRST_ENTITY_CLASS_ID 200

// forward declaration
class CEntity;
struct IScriptObject;

#define EVENT_PREFIX "Event_"

/*!
 *  CEntityScript holds information about Entity lua script.
 */
class CEntityScript : public CRefCountBase
{
public:
	CEntityScript( const EntityClassId ClassId,const char *sName,const char *sFile );
	virtual ~CEntityScript();

	//! Get name of entity script.
	const CString& GetName() const { return m_name; }
	const CString& GetFile() const { return m_file; }
	const CString& GetRelativeFile() const { return m_relFile; }
	EntityClassId	GetClsId() const { return m_ClassId; };

	int GetMethodCount() const { return m_methods.size(); }
	const CString& GetMethod( int index ) const { return m_methods[index]; }

	//////////////////////////////////////////////////////////////////////////
	int	GetEventCount();
	CString GetEvent( int i );

	//////////////////////////////////////////////////////////////////////////
	//! Get properties of this sacript.
	CVarBlock* GetProperties() const { return m_properties; }
	CVarBlock* GetProperties2() const { return m_properties2; }
	//////////////////////////////////////////////////////////////////////////

	bool	Load();
	void	Reload();
	bool	IsValid() const { return m_valid; };

	//! Marks script not valid, must be loaded on next access.
	void Invalidate() { m_valid = false; };

	//! Takes current values of properties from Entity and put it to entity table.
	void	SetProperties( IEntity *entity,CVarBlock *properties,bool bCallUpdate );
	void	SetProperties2( IEntity *entity,CVarBlock *properties,bool bCallUpdate );

	//! Setup entity target events table
	void	SetEventsTable( CEntity *entity );

	//! Run method.
	void	RunMethod( IEntity *entity,const CString &method );
	void	SendEvent( IEntity *entity,const CString &event );

	// Edit methods.
	void	GotoMethod( const CString &method );
	void	AddMethod( const CString &method );

	//! Get visual object for this entity script.
	const CString&	GetVisualObject() { return m_visualObject; };

	//! Is Standart class
	bool IsStandart() const { return m_standart; };
	int GetVisibilityMask() const { return m_visibilityMask; };

	//! Check if entity of this class can be used in editor.
	bool IsUsable() const { return m_usable; }

	// Set class as placable or not.
	void SetUsable( bool usable ) { m_usable = usable; }

private:
	bool	ParseScript();
	int FindLineNum( const CString &line );

	//! Put var block to script properties.
	void VarToScriptObject( IVariable *var,IScriptObject *obj );

	CString m_name;
	CString m_file;
	CString m_relFile;
	EntityClassId m_ClassId;
	bool m_valid;
	//! True if standart entity class.
	bool m_standart;

	bool m_haveEventsTable;
	
	//! True if entity script have update entity
	bool m_bUpdatePropertiesImplemented;

	CString m_visualObject;
	int m_visibilityMask;

	bool m_usable;

	//! Array of methods in this script.
	std::vector<CString> m_methods;

	//! Array of events supported by this script.
	std::vector<CString> m_events;

	TSmartPtr<CVarBlock> m_properties;
	TSmartPtr<CVarBlock> m_properties2;
};

typedef TSmartPtr<CEntityScript> CEntityScriptPtr;

/*!
 *	CEntityScriptRegistry	manages all known CEntityScripts instances.
 */
class CEntityScriptRegistry
{
public:
	CEntityScriptRegistry();
	~CEntityScriptRegistry();

	CEntityScript* Find( const CString &name );
	void	Insert( CEntityScript *script );

	void LoadScripts();

	//! Get all scripts as array.
	void	GetScripts( std::vector<CEntityScript*> &scripts );

	static CEntityScriptRegistry* Instance();
	static void Release();

private:
	StdMap<CString,CEntityScriptPtr> m_scripts;
	static CEntityScriptRegistry* m_instance;
};

#endif // __EntityScript_h__
