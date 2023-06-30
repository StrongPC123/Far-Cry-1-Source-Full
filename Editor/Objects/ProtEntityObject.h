////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   protentityobject.h
//  Version:     v1.00
//  Created:     24/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __protentityobject_h__
#define __protentityobject_h__
#pragma once

#include "Entity.h"

class CEntityPrototype;
/*!
 *	Prototype entity object.
 *
 */
class CProtEntityObject : public CEntity
{
public:
	DECLARE_DYNCREATE(CProtEntityObject)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	bool CreateGameObject();

	CString GetTypeDescription() const { return m_prototypeName; };

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );
	void BeginEditMultiSelParams( bool bAllOfSameType );
	void EndEditMultiSelParams();

	void Serialize( CObjectArchive &ar );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////

	void SetPrototype( REFGUID guid,bool bForceReload=false );
	void SetPrototype( CEntityPrototype *prototype,bool bForceReload );

protected:
	//! Dtor must be protected.
	CProtEntityObject();

	virtual void SpawnEntity();

	//! Callback called by prototype when its updated.
	void OnPrototypeUpdate();

	//! Entity prototype name.
	CString m_prototypeName;
	//! Full prototype name.
	GUID m_prototypeGUID;
};

/*!
 * Class Description of StaticObject	
 */
class CProtEntityObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {B9FB3D0B-ADA8-4380-A96F-A091E66E6EC1}
		static const GUID guid = { 0xb9fb3d0b, 0xada8, 0x4380, { 0xa9, 0x6f, 0xa0, 0x91, 0xe6, 0x6e, 0x6e, 0xc1 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_ENTITY; };
	const char* ClassName() { return "PrototypeEntity"; };
	const char* Category() { return "Archetype Entity"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CProtEntityObject); };

	//! Select all entity prototypes.
	//! ObjectTreeBrowser object can recognize this hardcoded name.
	const char* GetFileSpec() { return "*EntityPrototype"; };
	int GameCreationOrder() { return 205; };
};

#endif // __protentityobject_h__
