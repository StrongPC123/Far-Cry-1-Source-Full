////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   PrefabObject.h
//  Version:     v1.00
//  Created:     13/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PrefabObject_h__
#define __PrefabObject_h__
#pragma once

#include "Group.h"

class CPrefabItem;

#define PREFAB_OBJECT_CLASS_NAME "Prefab"

/*!
*		CPrefabObject is prefabricated object which can contain multiple other objects, in a group like manner, 
		but internal objects can not be modified, they are only created from PrefabItem.
*/
class CPrefabObject : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CPrefabObject)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();

	void Display( DisplayContext &disp );
	bool HitTest( HitContext &hc );
	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );

	void OnEvent( ObjectEvent event );

	void Serialize( CObjectArchive &ar );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// CPrefabObject.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetPrefab( REFGUID guid,bool bForceReload );
	virtual void SetPrefab( CPrefabItem *pPrefab,bool bForceReload );
	CPrefabItem* GetPrefab() const;

	// Extract all objects inside.
	void ExtractAll();
	void ExtractObject( CBaseObject *pObj );

protected:
	//! Dtor must be protected.
	CPrefabObject();

	virtual void PostClone( CBaseObject *pFromObject,CObjectCloneContext &ctx );
	virtual void CalcBoundBox();
	void DeleteThis() { delete this; };
	
	void RecursivelySetObjectInPrefab( CBaseObject *object );
	void RecursivelyDisplayObject( CBaseObject *object,DisplayContext &dc );
	void DeleteAllPrefabObjects();

	void InvalidateBBox() { m_bBBoxValid = false; };

protected:
	_smart_ptr<CPrefabItem> m_pPrefabItem;

	CString m_prefabName;
	GUID m_prefabGUID;

	BBox m_bbox;
	bool m_bBBoxValid;

	//////////////////////////////////////////////////////////////////////////
	// Per Instance Entity events.
	//////////////////////////////////////////////////////////////////////////
	
};

/*!
* Class Description of Group.
*/
class CPrefabObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {931962ED-450F-443e-BFA4-1BBDAA061202}
		static const GUID guid = { 0x931962ed, 0x450f, 0x443e, { 0xbf, 0xa4, 0x1b, 0xbd, 0xaa, 0x6, 0x12, 0x2 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_PREFAB; };
	const char* ClassName() { return PREFAB_OBJECT_CLASS_NAME; };
	const char* Category() { return "Prefabs"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CPrefabObject); };

	//! Select all prefabs.
	//! ObjectTreeBrowser object can recognize this hardcoded name.
	const char* GetFileSpec() { return "*Prefabs"; };
	int GameCreationOrder() { return 210; };
};

#endif // __PrefabObject_h__

