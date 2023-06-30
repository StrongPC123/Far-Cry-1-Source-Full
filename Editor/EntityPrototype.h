////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   centityprototype.h
//  Version:     v1.00
//  Created:     22/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __centityprototype_h__
#define __centityprototype_h__
#pragma once

#include "BaseLibraryItem.h"

class CEntityPrototypeLibrary;
class CEntityScript;

//////////////////////////////////////////////////////////////////////////
/** Prototype of entity, contain specified entity properties.
*/
class CRYEDIT_API CEntityPrototype : public CBaseLibraryItem
{
public:
	typedef Functor0 UpdateCallback;

	CEntityPrototype();
	~CEntityPrototype();

	//! Set prototype description.
	void SetDescription( const CString &description ) { m_description = description; };
	//! Get prototype description.
	const CString& GetDescription() const { return m_description; };

	//! Set class name of entity.
	void SetEntityClassName( const CString &className );
	//! Get class name of entity.
	const CString& GetEntityClassName() const { return m_className; }

	//! Reload entity class.
	void Reload();

	//! Return properties of entity.
	CVarBlock* GetProperties();
	//! Get entity script of this prototype.
	CEntityScript* GetScript();

	//////////////////////////////////////////////////////////////////////////
	//! Serialize prototype to xml.
	virtual void Serialize( SerializeContext &ctx );

	//////////////////////////////////////////////////////////////////////////
	// Update callback.
	//////////////////////////////////////////////////////////////////////////

	void AddUpdateListener( UpdateCallback cb );
	void RemoveUpdateListener( UpdateCallback cb );

	//! Called after prototype is updated.
	void Update();

private:
	//! Name of entity class name.
	CString m_className;
	//! Description of this prototype.
	CString m_description;
	//! Entity properties.
	CVarBlockPtr m_properties;

	// Pointer to entity script.
	TSmartPtr<CEntityScript> m_script;

	//! List of update callbacks.
	std::list<UpdateCallback> m_updateListeners;
};

TYPEDEF_AUTOPTR(CEntityPrototype);

#endif // __centityprototype_h__
