////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objectloader.h
//  Version:     v1.00
//  Created:     15/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __objectloader_h__
#define __objectloader_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "ErrorReport.h"

/** CObjectLoader used to load Bas Object and resolve ObjectId references while loading.
*/
class CObjectArchive
{
public:
	XmlNodeRef node; //!< Current archive node.
	bool bLoading;
	bool bUndo;

	CObjectArchive( IObjectManager *objMan,XmlNodeRef xmlRoot,bool loading );
	~CObjectArchive();

	//! Resolve callback with only one parameter of CBaseObject.
	typedef Functor1<CBaseObject*> ResolveObjRefFunctor1;
	//! Resolve callback with two parameters one is pointer to CBaseObject and second use data integer.
	typedef Functor2<CBaseObject*,unsigned int> ResolveObjRefFunctor2;

	/** Register Object id.
		@param objectId Original object id from the file.
		@param realObjectId Changed object id.
	*/
	//void RegisterObjectId( int objectId,int realObjectId );
	
	//! Set object resolve callback, it will be called once object with specified Id is loaded.
	void SetResolveCallback( CBaseObject *fromObject,REFGUID objectId,ResolveObjRefFunctor1 &func );
	//! Set object resolve callback, it will be called once object with specified Id is loaded.
	void SetResolveCallback( CBaseObject *fromObject,REFGUID objectId,ResolveObjRefFunctor2 &func,uint userData );
	//! Resolve all object ids and call callbacks on resolved objects.
	void ResolveObjects();

	// Save object to archive.
	void SaveObject( CBaseObject *pObject );
	
	//! Load multiple objects from archive.
	void LoadObjects( XmlNodeRef &rootObjectsNode );

	//! Load one object from archive.
	CBaseObject* LoadObject( XmlNodeRef &objNode,CBaseObject *pPrevObject=NULL );

	//! If true new loaded objects will be assigned new GUIDs.
	void MakeNewIds( bool bEnable );

	//! Remap object ids.
	void RemapID( REFGUID oldId,REFGUID newId );

	//! Report error during loading.
	void ReportError( CErrorRecord &err );
	//! Assigner different error report class.
	void SetErrorReport( CErrorReport *errReport );
	//! Display collected error reports.
	void ShowErrors();

private:
	IObjectManager* m_objectManager;
	struct Callback {
		ResolveObjRefFunctor1 func1;
		ResolveObjRefFunctor2 func2;
		uint userData;
		TSmartPtr<CBaseObject> fromObject;
		Callback() { func1 = 0; func2 = 0; userData = 0; };
	};
	typedef std::multimap<GUID,Callback,guid_less_predicate> Callbacks;
	Callbacks m_resolveCallbacks;

	// Set of all saved objects to this archive.
	typedef std::set<CBaseObject*> ObjectsSet;
	ObjectsSet m_objectsSet;

	typedef std::multimap<int,CBaseObject*> OrderedObjects;
	OrderedObjects m_orderedObjects;

	// Loaded objects IDs, used for remapping of GUIDs.
	std::map<GUID,GUID,guid_less_predicate> m_IdRemap;

	//! If true new loaded objects will be assigned new GUIDs.
	bool m_bMakeNewIds;
	CErrorReport *m_pCurrentErrorReport;
};

#endif // __objectloader_h__
