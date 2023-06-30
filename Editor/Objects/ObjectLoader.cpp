////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   objectloader.cpp
//  Version:     v1.00
//  Created:     15/8/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ObjectLoader.h"
#include "ObjectManager.h"

//////////////////////////////////////////////////////////////////////////
// CObjectArchive Implementation.
//////////////////////////////////////////////////////////////////////////
CObjectArchive::CObjectArchive( IObjectManager *objMan,XmlNodeRef xmlRoot,bool loading )
{
	m_objectManager = objMan;
	bLoading = loading;
	bUndo = false;
	m_bMakeNewIds = false;
	node = xmlRoot;
	m_pCurrentErrorReport = GetIEditor()->GetErrorReport();
}

//////////////////////////////////////////////////////////////////////////
CObjectArchive::~CObjectArchive()
{
	// Always make sure objects are resolved when loading from archive.
	if (bLoading)
	{
		ResolveObjects();
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::SetResolveCallback( CBaseObject *fromObject,REFGUID objectId,ResolveObjRefFunctor1 &func )
{
	if (objectId == GUID_NULL)
	{
		func( 0 );
		return;
	}

	CBaseObject *object = m_objectManager->FindObject( objectId );
	if (object && !m_bMakeNewIds)
	{
		// Object is already resolved. immidiatly call callback.
		func( object );
	}
	else
	{
		Callback cb;
		cb.fromObject = fromObject;
		cb.func1 = func;
		m_resolveCallbacks.insert( Callbacks::value_type(objectId,cb) );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::SetResolveCallback( CBaseObject *fromObject,REFGUID objectId,ResolveObjRefFunctor2 &func,uint userData )
{
	if (objectId == GUID_NULL)
	{
		func( 0,userData );
		return;
	}

	CBaseObject *object = m_objectManager->FindObject( objectId );
	if (object && !m_bMakeNewIds)
	{
		// Object is already resolved. immidiatly call callback.
		func( object,userData );
	}
	else
	{
		Callback cb;
		cb.fromObject = fromObject;
		cb.func2 = func;
		cb.userData = userData;
		m_resolveCallbacks.insert( Callbacks::value_type(objectId,cb) );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::ResolveObjects()
{
	for (Callbacks::iterator it = m_resolveCallbacks.begin(); it != m_resolveCallbacks.end(); it++)
	{
		Callback &cb = it->second;
		GUID objectId = it->first;
		if (!m_IdRemap.empty())
		{
			objectId = stl::find_in_map( m_IdRemap,objectId,objectId );
		}
		CBaseObject *object = m_objectManager->FindObject(objectId);
		if (!object)
		{
			CString from;
			if (cb.fromObject)
			{
				from = cb.fromObject->GetName();
			}
			// Cannot resolve this object id.
			CErrorRecord err;
			err.error.Format( _T("Unresolved ObjectID: %s, Referenced from Object %s"),GuidUtil::ToString(objectId),
													(const char*)from );
			err.severity = CErrorRecord::ESEVERITY_ERROR;
			err.flags = CErrorRecord::FLAG_OBJECTID;
			err.pObject = cb.fromObject;
			GetIEditor()->GetErrorReport()->ReportError(err);

			//Warning( "Cannot resolve ObjectID: %s\r\nObject with this ID was not present in loaded file.\r\nFor instance Trigger referencing another object which is not loaded in Level.",GuidUtil::ToString(objectId) );
			continue;
		}
		m_pCurrentErrorReport->SetCurrentValidatorObject( object );
		// Call callback with this object.
		if (cb.func1)
			(cb.func1)( object );
		if (cb.func2)
			(cb.func2)( object,cb.userData );
	}
	m_resolveCallbacks.clear();

	/*
	// Send After Load event for all loaded objects.
	for (SavedObjects::iterator it = m_savedObjects.begin(); it != m_savedObjects.end(); ++it)
	{
		CBaseObject *pObject = *it;
		if (!pObject->GetGroup())
			pObject->OnEvent( EVENT_AFTER_LOAD );
	}
	*/

	// Create GameObject for all loaded objects.
	for (OrderedObjects::iterator it = m_orderedObjects.begin(); it != m_orderedObjects.end(); ++it)
	{
		CBaseObject *pObject = it->second;
		m_pCurrentErrorReport->SetCurrentValidatorObject( pObject );
		pObject->CreateGameObject();
	}
	m_pCurrentErrorReport->SetCurrentValidatorObject( NULL );

	// Send After Load event for all loaded objects.
	m_objectManager->SendEvent( EVENT_AFTER_LOAD );

	m_objectsSet.clear();
	m_orderedObjects.clear();
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::SaveObject( CBaseObject *pObject )
{
	if (m_objectsSet.find(pObject) == m_objectsSet.end())
	{
		m_objectsSet.insert( pObject );
		// If this object was not saved before.
		XmlNodeRef objNode = node->newChild( "Object" );
		XmlNodeRef prevRoot = node;
		node = objNode;
		pObject->Serialize( *this );
		node = prevRoot;
	}
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectArchive::LoadObject( XmlNodeRef &objNode,CBaseObject *pPrevObject )
{
	XmlNodeRef prevNode = node;
	node = objNode;
	CBaseObject *pObject;
	
	pObject = m_objectManager->NewObject( *this,pPrevObject,m_bMakeNewIds );
	if (pObject)
	{
		m_objectsSet.insert( pObject );
		int sortOrder = pObject->GetClassDesc()->GameCreationOrder();
		m_orderedObjects.insert( OrderedObjects::value_type(sortOrder,pObject) );
	}
	node = prevNode;
	return pObject;
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::LoadObjects( XmlNodeRef &rootObjectsNode )
{
	int numObjects = rootObjectsNode->getChildCount();
	for (int i = 0; i < numObjects; i++)
	{
		XmlNodeRef objNode = rootObjectsNode->getChild(i);
		LoadObject( objNode );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::ReportError( CErrorRecord &err )
{
	if (m_pCurrentErrorReport)
		m_pCurrentErrorReport->ReportError( err );
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::SetErrorReport( CErrorReport *errReport )
{
	if (errReport)
		m_pCurrentErrorReport = errReport;
	else
		m_pCurrentErrorReport = GetIEditor()->GetErrorReport();
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::ShowErrors()
{
	GetIEditor()->GetErrorReport()->Display();
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::MakeNewIds( bool bEnable )
{
	m_bMakeNewIds = bEnable;
}

//////////////////////////////////////////////////////////////////////////
void CObjectArchive::RemapID( REFGUID oldId,REFGUID newId )
{
	m_IdRemap[oldId] = newId;
}
