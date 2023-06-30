////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ObjectManager.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: ObjectManager implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "ObjectManager.h"

#include "..\DisplaySettings.h"
#include "Util\Crc32.h"

#include "TagPoint.h"
#include "TagComment.h"
//#include "StatObj.h"
#include "Entity.h"
#include "Group.h"
#include "Volume.h"
#include "SoundObject.h"
#include "ShapeObject.h"
#include "AIPoint.h"
#include "BrushObject.h"
#include "CameraObject.h"
#include "AIAnchor.h"
#include "AreaBox.h"
#include "AreaSphere.h"
#include "WaterShapeObject.h"
#include "VisAreaShapeObject.h"
#include "ProtEntityObject.h"
#include "PrefabObject.h"

#include "Settings.h"
#include "Viewport.h"

#include "GizmoManager.h"
#include "ObjectLayerManager.h"
#include "AxisGizmo.h"

#include <io.h>

/*!
 *	Class Description used for object templates.
 *	This description filled from Xml template files.
 */
class CXMLObjectClassDesc : public CObjectClassDesc
{
public:
	CObjectClassDesc*	superType;
	CString type;
	CString category;
	CString fileSpec;
	GUID guid;
	
public:
	REFGUID ClassID()
	{
		return guid;
	}
	ObjectType GetObjectType() { return superType->GetObjectType(); };
	const char* ClassName() { return type; };
	const char* Category() { return category; };
	CRuntimeClass* GetRuntimeClass() { return superType->GetRuntimeClass(); };
	const char* GetFileSpec()
	{
		if (!fileSpec.IsEmpty())
			return fileSpec;
		else
			return superType->GetFileSpec();
	};
	virtual int GameCreationOrder() { return superType->GameCreationOrder(); };
};

//////////////////////////////////////////////////////////////////////////
//! Undo New Object
class CUndoBaseObjectNew : public IUndoObject
{
public:
	CUndoBaseObjectNew( CBaseObject *obj ) { m_object = obj; }
protected:
	virtual int GetSize() { return sizeof(*this); }; // Return size of xml state.
	virtual const char* GetDescription() { return "New BaseObject"; };

	virtual void Undo( bool bUndo )
	{
		if (bUndo)
		{
			m_redo = new CXmlNode("Redo");
			// Save current object state.
			CObjectArchive ar(GetIEditor()->GetObjectManager(),m_redo,false);
			ar.bUndo = true;
			m_object->Serialize( ar );
		}
		// Delete this object.
		GetIEditor()->DeleteObject( m_object );
	}
	virtual void Redo()
	{
		if (m_redo)
		{
			IObjectManager *pObjMan = GetIEditor()->GetObjectManager();
			CObjectArchive ar( pObjMan,m_redo,true );
			ar.bUndo = true;
			ar.LoadObject( m_redo,m_object );
			pObjMan->SelectObject( m_object );
		}
	}
private:
	CBaseObjectPtr m_object;
	XmlNodeRef m_redo;
};

//////////////////////////////////////////////////////////////////////////
//! Undo Delete Object
class CUndoBaseObjectDelete : public IUndoObject
{
public:
	CUndoBaseObjectDelete( CBaseObject *obj )
	{
		m_object = obj;
		// Save current object state.
		m_undo = new CXmlNode("Undo");
		CObjectArchive ar(GetIEditor()->GetObjectManager(),m_undo,false);
		ar.bUndo = true;
		m_object->Serialize( ar );
	}
protected:
	virtual int GetSize() { return sizeof(*this); }; // Return size of xml state.
	virtual const char* GetDescription() { return "Delete BaseObject"; };

	virtual void Undo( bool bUndo )
	{
		IObjectManager *pObjMan = GetIEditor()->GetObjectManager();
		CObjectArchive ar( pObjMan,m_undo,true );
		ar.bUndo = true;
		ar.LoadObject( m_undo,m_object );
		pObjMan->SelectObject( m_object );
	}
	virtual void Redo()
	{
		// Delete this object.
		GetIEditor()->DeleteObject( m_object );
	}
private:
	CBaseObjectPtr m_object;
	XmlNodeRef m_undo;
};

//////////////////////////////////////////////////////////////////////////
//! Undo Select Object
class CUndoBaseObjectSelect : public IUndoObject
{
public:
	CUndoBaseObjectSelect( CBaseObject *obj )
	{
		assert( obj != 0 );
		m_object = obj;
		m_bUndoSelect = obj->IsSelected();
	}
protected:
	virtual void Release() { delete this; };
	virtual int GetSize() { return sizeof(*this); }; // Return size of xml state.
	virtual const char* GetDescription() { return "Select Object"; };

	virtual void Undo( bool bUndo )
	{
		if (bUndo)
		{
			m_bRedoSelect = m_object->IsSelected();
		}
		if (m_bUndoSelect)
			GetIEditor()->GetObjectManager()->SelectObject(m_object);
		else
			GetIEditor()->GetObjectManager()->UnselectObject(m_object);
	}
	virtual void Redo()
	{
		if (m_bRedoSelect)
			GetIEditor()->GetObjectManager()->SelectObject(m_object);
		else
			GetIEditor()->GetObjectManager()->UnselectObject(m_object);
	}
private:
	CBaseObjectPtr m_object;
	bool m_bUndoSelect;
	bool m_bRedoSelect;
};

//////////////////////////////////////////////////////////////////////////
// CObjectManager implementation.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CObjectManager::CObjectManager()
{
	m_currSelection = &m_defaultSelection;
	m_currEditObject = 0;

	m_selectCallback = 0;
	m_createGameObjects = true;
	m_bSingleSelection = false;
	m_nLastSelCount=0;

	m_bGenUniqObjectNames = true;

	m_bSelectionChanged = false;
	m_bVisibleObjectValid = true;
	m_lastHideMask = 0;

	m_pLoadProgress = 0;
	m_totalObjectsToLoad = 0;
	m_loadedObjects = 0;

	// Creates objects layers manager.
	m_pLayerManager = new CObjectLayerManager( this );

	// Creates gizmo manager.
	m_gizmoManager = new CGizmoManager;

	// Register default classes.
	CClassFactory *cf = CClassFactory::Instance();
	cf->RegisterClass( new CTagPointClassDesc );
	cf->RegisterClass( new CRespawnPointClassDesc );
	cf->RegisterClass( new CTagCommentClassDesc );
	//cf->RegisterClass( new CStaticObjectClassDesc );
	//cf->RegisterClass( new CBuildingClassDesc );
	cf->RegisterClass( new CEntityClassDesc );
	cf->RegisterClass( new CSimpleEntityClassDesc );
	cf->RegisterClass( new CGroupClassDesc );
	cf->RegisterClass( new CVolumeClassDesc );
	cf->RegisterClass( new CSoundObjectClassDesc );
	cf->RegisterClass( new CShapeObjectClassDesc );
	cf->RegisterClass( new CAIPathObjectClassDesc );
	cf->RegisterClass( new CAIForbiddenAreaObjectClassDesc );
	cf->RegisterClass( new CAINavigationModifierObjectClassDesc );
	cf->RegisterClass( new CAIOcclusionPlaneObjectClassDesc );
	cf->RegisterClass( new CAIPointClassDesc );
	cf->RegisterClass( new CBrushObjectClassDesc );
	cf->RegisterClass( new CCameraObjectClassDesc );
	cf->RegisterClass( new CCameraObjectTargetClassDesc );
	cf->RegisterClass( new CAIAnchorClassDesc );
	cf->RegisterClass( new CAreaBoxClassDesc );
	cf->RegisterClass( new CAreaSphereClassDesc );
	cf->RegisterClass( new CWaterShapeObjectClassDesc );
	cf->RegisterClass( new CVisAreaShapeObjectClassDesc );
	cf->RegisterClass( new CPortalShapeObjectClassDesc );
	cf->RegisterClass( new COccluderShapeObjectClassDesc );
	cf->RegisterClass( new CProtEntityObjectClassDesc );
	cf->RegisterClass( new CPrefabObjectClassDesc );

	LoadRegistry();
}

//////////////////////////////////////////////////////////////////////////
CObjectManager::~CObjectManager()
{
	SaveRegistry();
	DeleteAllObjects();

	if (m_gizmoManager)
		delete m_gizmoManager;

	if (m_pLayerManager)
		delete m_pLayerManager;
}

void	CObjectManager::SaveRegistry()
{

}

void	CObjectManager::LoadRegistry()
{
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectManager::NewObject( CObjectClassDesc *cls,CBaseObject *prev,const CString &file )
{
	ASSERT( cls != 0 );
	CRuntimeClass *rtClass = cls->GetRuntimeClass();
	ASSERT( rtClass->IsDerivedFrom(RUNTIME_CLASS(CBaseObject)) );
	if (prev)
	{
		// Both current and previous object must be of same type.
		ASSERT( cls == prev->GetClassDesc() );
	}

	// Suspend undo operations when initialzing object.
	GetIEditor()->SuspendUndo();

	CBaseObjectPtr obj = (CBaseObject*)rtClass->CreateObject();
	obj->SetIEditor( GetIEditor() );
	obj->SetClassDesc( cls );
	obj->SetLayer( m_pLayerManager->GetCurrentLayer() );
	obj->m_objectManager = this;
	CoCreateGuid( &obj->m_guid ); // generate uniq GUID for this object.

	GetIEditor()->GetErrorReport()->SetCurrentValidatorObject( obj );
	if (obj->Init( GetIEditor(),prev,file ))
	{
		if (obj->GetName().IsEmpty())
		{
			obj->SetName( GenUniqObjectName( cls->ClassName() ) );;
		}
		/*
		// Check if object Init function changed its name.
		if (objName.Compare(obj->GetName()) != 0)
		{
			// Object changed name. we must make sure it unique and generate new id.
			obj->SetName( GenUniqObjectName(obj->GetName()) );
			// Generate new unique id for this object.
			int id = Crc32Gen::GetCRC32(obj->GetName());
			obj->SetId(id);	
		}
		*/

		// Create game object itself.
		obj->CreateGameObject();

		if (!AddObject( obj ))
			obj = 0;
	}
	else
	{
		obj = 0;
	}
	GetIEditor()->GetErrorReport()->SetCurrentValidatorObject( NULL );

	GetIEditor()->ResumeUndo();
	
	if (obj != 0 && GetIEditor()->IsUndoRecording())
		GetIEditor()->RecordUndo( new CUndoBaseObjectNew(obj) );

	return obj;
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectManager::NewObject( CObjectArchive &ar,CBaseObject *pUndoObject,bool bMakeNewId )
{
	// Suspend undo operations when initialzing object.
	CUndoSuspend undoSuspender;

	XmlNodeRef objNode = ar.node;

	// Load all objects from XML.
	CString typeName;
	GUID id = GUID_NULL;
	if (!objNode->getAttr( "Type",typeName ))
		return 0;

	if (!objNode->getAttr( "Id",id ))
	{
		// Make new ID for object that doesnt have if.
		CoCreateGuid( &id );
	}

	if (bMakeNewId)
	{
		// Make new guid for this object.
		GUID newId;
		CoCreateGuid( &newId );
		ar.RemapID( id,newId ); // Mark this id remaped.
		id = newId;
	}

	CBaseObjectPtr pObject;
	if (pUndoObject)
	{
		// if undoing restore object pointer.
		pObject = pUndoObject;
	}
	else
	{
		// New object creation.

		CObjectClassDesc *cls = FindClass( typeName );
		if (!cls)
		{
			CLogFile::FormatLine( "Error: RuntimeClass %s not registered",(const char*)typeName );
			return 0;
		}

		CRuntimeClass *rtClass = cls->GetRuntimeClass();
		assert( rtClass->IsDerivedFrom(RUNTIME_CLASS(CBaseObject)) );

		pObject = (CBaseObject*)rtClass->CreateObject();
		pObject->SetIEditor( GetIEditor() );
		pObject->SetClassDesc( cls );
		pObject->m_objectManager = this;
		pObject->m_guid = id;
		pObject->SetLayer( m_pLayerManager->GetCurrentLayer() );

		// @FIXME: Make sure this id not taken.
		CBaseObject *obj = FindObject(pObject->GetId());
		if (obj)
		{
			// If id is taken.
			CString objName;
			objNode->getAttr( "Name",objName );
			CString error;
			error.Format( _T("Object with ID %s (%s) already exist in Object Manager\r\nWhen trying to create object: %s\r\nNew Object Ignored."),GuidUtil::ToString(obj->GetId()),(const char*)obj->GetName(),(const char*)objName );
			CLogFile::WriteLine( error );
			MessageBox( AfxGetMainWnd()->GetSafeHwnd(),error,_T("Object Id Collision"),MB_OK|MB_ICONWARNING );

			return 0;
			//CoCreateGuid( &pObject->m_guid ); // generate uniq GUID for this object.
		}
	}

	GetIEditor()->GetErrorReport()->SetCurrentValidatorObject( pObject );
	if (!pObject->Init( GetIEditor(),0,"" ))
	{
		GetIEditor()->GetErrorReport()->SetCurrentValidatorObject( NULL );
		return 0;
	}

	if (!AddObject( pObject ))
	{
		GetIEditor()->GetErrorReport()->SetCurrentValidatorObject( NULL );
		return 0;
	}

	pObject->Serialize( ar );

	GetIEditor()->GetErrorReport()->SetCurrentValidatorObject( NULL );

	if (!pObject->GetLayer())
	{
		// Cannot be.
		assert(0);
	}

	if (pObject != 0 && pUndoObject == 0)
	{
		// If new object with no undo, record it.
		GetIEditor()->RecordUndo( new CUndoBaseObjectNew(pObject) );
	}

	m_loadedObjects++;
	if (m_pLoadProgress && m_totalObjectsToLoad > 0)
		m_pLoadProgress->Step( (m_loadedObjects*100)/m_totalObjectsToLoad );

	return pObject;
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectManager::NewObject( const CString &typeName,CBaseObject *prev,const CString &file )
{ 
	CObjectClassDesc *cls = FindClass( typeName );
	if (!cls)
	{
		GetIEditor()->GetSystem()->GetILog()->Log( "Warning: RuntimeClass %s not registered",(const char*)typeName );
		return 0;
	}	
	CBaseObject *pObject = NewObject( cls,prev,file );
	return pObject;
}
	
//////////////////////////////////////////////////////////////////////////
void	CObjectManager::DeleteObject( CBaseObject *obj )
{
	if (m_currEditObject == obj)
		EndEditParams();

	if (!obj)
		return;

	// If object already deleted.
	if (obj->CheckFlags(OBJFLAG_DELETED))
		return;

	// Check if object is a group then delete all childs.
	if (obj->IsKindOf(RUNTIME_CLASS(CGroup)))
	{
		((CGroup*)obj)->DeleteAllChilds();
	}

	// This will detach all childs and store Undo for each detachment (So they can be restored with Undo)
	//obj->DetachAll();

	// Must be after object DetachAll to support restoring Parent/Child relations.
	if (CUndo::IsRecording())
	{
		// Store undo for all child objects.
		for (int i = 0; i < obj->GetChildCount(); i++)
		{
			if (!obj->GetChild(i)->CheckFlags(OBJFLAG_PREFAB))
				obj->GetChild(i)->StoreUndo("DeleteParent");
		}
		CUndo::Record( new CUndoBaseObjectDelete(obj) );
	}

	GetIEditor()->SuspendUndo();
	//! If object is child remove it from parent.
	obj->DetachThis();

	// Release game resources.
	obj->Done();

	NotifyObjectListeners( obj,CBaseObject::ON_DELETE );

	RemoveObject( obj );
	GetIEditor()->ResumeUndo();
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::DeleteAllObjects()
{
	EndEditParams();

	ClearSelection();
	int i;

	InvalidateVisibleList();

	// Delete all selection groups.
	std::vector<CSelectionGroup*> sel;
	stl::map_to_vector( m_selections,sel );
	for (i = 0; i < sel.size(); i++)
	{
		delete sel[i];
	}
	m_selections.clear();

	std::vector<CBaseObjectPtr> objectsHolder;
	GetAllObjects( objectsHolder );
	for (i = 0; i < objectsHolder.size(); i++)
	{
		objectsHolder[i]->Done();
	}
	// Clear map.
	m_objects.clear();
	//! Delete object instances.
	objectsHolder.clear();

	// Clear name map.
	m_nameNumbersMap.clear();
}

CBaseObject* CObjectManager::CloneObject( CBaseObject *obj )
{
	ASSERT( obj );
	//CRuntimeClass *cls = obj->GetRuntimeClass();
	//CBaseObject *clone = (CBaseObject*)cls->CreateObject();
	//clone->CloneCopy( obj );
	CBaseObject *clone = NewObject( obj->GetClassDesc(),obj );
	return clone;
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectManager::FindObject( REFGUID guid ) const
{
	return stl::find_in_map( m_objects,guid,(CBaseObject*)0 );
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectManager::FindObject( const CString &sName ) const
{
	for (Objects::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *pObj = it->second;
		if (stricmp(pObj->GetName(), sName)==0)
			return pObj;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CObjectManager::AddObject( CBaseObject *obj )
{
	CBaseObjectPtr p = stl::find_in_map( m_objects,obj->GetId(),0 );
	if (p)
	{
		CErrorRecord err;
		err.error.Format( "New Object %s have Duplicate GUID %s, New Object Ignored",(const char*)obj->GetName(),GuidUtil::ToString(obj->GetId()) );
		err.severity = CErrorRecord::ESEVERITY_ERROR;
		err.pObject = obj;
		err.flags = CErrorRecord::FLAG_OBJECTID;
		GetIEditor()->GetErrorReport()->ReportError(err);

		return false;
	}
	m_objects[obj->GetId()] = obj;
	InvalidateVisibleList();
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::RemoveObject( CBaseObject *obj )
{
	assert( obj != 0 );
	
	InvalidateVisibleList();

	m_objects.erase(obj->GetId());
	
	// Remove this object from selection groups.
	m_currSelection->RemoveObject(obj);
	std::vector<CSelectionGroup*> sel;
	stl::map_to_vector( m_selections,sel );
	for (int i = 0; i < sel.size(); i++)
	{
		sel[i]->RemoveObject(obj);
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::GetAllObjects( std::vector<CBaseObjectPtr> &objects ) const
{
	objects.clear();
	objects.reserve( m_objects.size() );
	for (Objects::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		objects.push_back( it->second );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::ChangeObjectId( CBaseObject *obj,int newId )
{
	/*
	assert( obj );
	CBaseObjectPtr pObject = obj;
	CBaseObjectPtr p;
	if (m_objects.Find(obj->GetId(),p))
	{
		// If object is already added to object list, change it in the map.
		m_objects.Erase( obj->GetId() );
		m_objects[newId] = pObject;
	}
	pObject->SetId( newId );
	*/
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::ChangeObjectName( CBaseObject *obj,const CString &newName )
{
	assert( obj );

	if (newName != obj->GetName())
	{
		/*
		// Check if this name already used.
		CBaseObject *pExisting = FindObject( newName );
		if (pExisting)
		{
			// If id is taken.
			CString str;
			str.Format( "Duplicate Object Name: %s\r\nName change ignored",(const char*)newName );
			AfxMessageBox( str,MB_OK|MB_ICONWARNING );
			return;
		}
		*/

		obj->SetName(newName);
		/*
		// Change ID of object together with its name.
		int id = Crc32Gen::GetCRC32(obj->GetName());
		ChangeObjectId( obj,id );
		*/
	}
}

//////////////////////////////////////////////////////////////////////////
int CObjectManager::GetObjectCount() const
{
	return m_objects.size();
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::GetObjects( CBaseObjectsArray &objects,CObjectLayer* layer )
{
	objects.clear();
	objects.reserve( m_objects.size() );
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		if (layer == 0 || it->second->GetLayer() == layer)
			objects.push_back( it->second );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::GetCameras( std::vector<CCameraObject*> &objects )
{
	objects.clear();
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *object = it->second;
		if (object->IsKindOf(RUNTIME_CLASS(CCameraObject)))
		{
			// Only consider camera sources.
			if (object->IsLookAtTarget())
				continue;
			objects.push_back( (CCameraObject*)object );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::SendEvent( ObjectEvent event )
{
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;
		if (obj->GetGroup())
			continue;
		obj->OnEvent( event );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::SendEvent( ObjectEvent event,const BBox &bounds )
{
	BBox box;
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;
		if (obj->GetGroup())
			continue;
		obj->GetBoundBox(box);
		if (bounds.IsIntersectBox(box))
			obj->OnEvent( event );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::Update()
{
	HWND hFocusWnd = GetFocus();

	if (m_currSelection->GetCount() == 0)
	{
		// Nothing selected.
		EndEditParams();
	}
	else if (m_currSelection->GetCount() == 1)
	{
		if (!m_bSingleSelection)
			EndEditParams();

		// Single object selecte.
		if (m_currEditObject != m_currSelection->GetObject(0))
		{
			m_bSelectionChanged = false;
			CBaseObject *newSelObject = m_currSelection->GetObject(0);
			if (!m_currEditObject || (m_currEditObject->GetRuntimeClass() != newSelObject->GetRuntimeClass()))
			{
				// If old object and new objects are of different classes.
				EndEditParams();
			}
			BeginEditParams( newSelObject,OBJECT_EDIT );
		}
	}
	else if (m_currSelection->GetCount() > 1)
	{
		// Multiple objects are selected.
		if (m_bSelectionChanged)
		{
			m_bSelectionChanged = false;
			m_nLastSelCount=m_currSelection->GetCount();
			EndEditParams();
			bool bAllSameType = m_currSelection->SameObjectType();

			m_currEditObject = m_currSelection->GetObject(0);
			m_currEditObject->BeginEditMultiSelParams( bAllSameType );
		}
	}

	// Restore focus if it changed.
	if (hFocusWnd != GetFocus() && hFocusWnd)
		SetFocus( hFocusWnd );
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::HideObject( CBaseObject *obj,bool hide )
{
	assert( obj != 0 );
	// Remove object from main object set and put it to hidden set.
	obj->SetHidden( hide );
	InvalidateVisibleList();
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::UnhideAll()
{
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;
		obj->SetHidden(false);
	}
	InvalidateVisibleList();
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::FreezeObject( CBaseObject *obj,bool freeze )
{
	assert( obj != 0 );
	// Remove object from main object set and put it to hidden set.
	obj->SetFrozen( freeze );
	InvalidateVisibleList();
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::UnfreezeAll()
{
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;
		obj->SetFrozen(false);
	}
	InvalidateVisibleList();
}

//////////////////////////////////////////////////////////////////////////
bool CObjectManager::SelectObject( CBaseObject *obj )
{
	assert( obj );

	// Check if can be selected.
	if (!(obj->GetType() & gSettings.objectSelectMask))
		return false;

	if (!obj->IsSelectable())
		return false;

	if (m_selectCallback)
	{
		if (!m_selectCallback->OnSelectObject( obj ))
			return true;
	}

	/*
	if (GetIEditor()->IsUndoRecording() && !obj->IsSelected())
	{
		GetIEditor()->RecordUndo( new CUndoBaseObjectSelect(obj) );
	}
	*/

	m_currSelection->AddObject( obj );
	SetObjectSelected( obj,true );
	return true;
}

void CObjectManager::UnselectObject( CBaseObject *obj )
{
	/*
	if (GetIEditor()->IsUndoRecording() && obj->IsSelected())
	{
		GetIEditor()->RecordUndo( new CUndoBaseObjectSelect(obj) );
	}
	*/
	SetObjectSelected( obj,false );
	m_currSelection->RemoveObject( obj );
}

CSelectionGroup* CObjectManager::GetSelection( const CString &name ) const
{
	CSelectionGroup *selection = stl::find_in_map( m_selections,name,(CSelectionGroup*)0 );
	return selection;
}

void CObjectManager::NameSelection( const CString &name )
{
	if (m_currSelection->IsEmpty())
		return;

	CSelectionGroup *selection = stl::find_in_map( m_selections,name,(CSelectionGroup*)0 );
	if (selection)
	{
		ASSERT( selection != 0 );
		// Check if trying to rename itself to the same name.
		if (selection == m_currSelection)
			return;
		m_selections.erase( name );
		delete selection;
	}
	selection = new CSelectionGroup;
	selection->Copy( *m_currSelection );
	selection->SetName( name );
	m_selections[name] = selection;
	m_currSelection = selection;
	m_defaultSelection.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////
int CObjectManager::ClearSelection()
{
	int numSel = m_currSelection->GetCount();
	UnselectCurrent();
	m_defaultSelection.RemoveAll();
	m_currSelection = &m_defaultSelection;
	m_bSelectionChanged = true;

	return numSel;
}

//////////////////////////////////////////////////////////////////////////
int CObjectManager::InvertSelection()
{
	int selCount = 0;
	// iterate all objects.
	for (Objects::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *pObj = it->second;
		if (pObj->IsSelected())
			UnselectObject( pObj );
		else
		{
			if (SelectObject( pObj ))
				selCount++;
		}
	}
	return selCount;
}

void CObjectManager::SetSelection( const CString &name )
{
	CSelectionGroup *selection = stl::find_in_map( m_selections,name,(CSelectionGroup*)0 );
	if (selection)
	{
		UnselectCurrent();
		ASSERT( selection != 0 );
		m_currSelection = selection;
		SelectCurrent();
	}
}

void CObjectManager::RemoveSelection( const CString &name )
{
	CString selName = name;
	CSelectionGroup *selection = stl::find_in_map( m_selections,name,(CSelectionGroup*)0 );
	if (selection)
	{
		if (selection == m_currSelection)
		{
			UnselectCurrent();
			m_currSelection = &m_defaultSelection;
			m_defaultSelection.RemoveAll();
		}
		delete selection;
		m_selections.erase( selName );
	}
}

void CObjectManager::SelectCurrent()
{
	for (int i = 0; i < m_currSelection->GetCount(); i++)
	{
		CBaseObject *obj = m_currSelection->GetObject(i);
		if (GetIEditor()->IsUndoRecording() && !obj->IsSelected())
			GetIEditor()->RecordUndo( new CUndoBaseObjectSelect(obj) );

		SetObjectSelected( obj,true );
	}
}

void CObjectManager::UnselectCurrent()
{
	// Make sure to unlock selection.
	GetIEditor()->LockSelection( false );

	for (int i = 0; i < m_currSelection->GetCount(); i++)
	{
		CBaseObject *obj = m_currSelection->GetObject(i);
		if (GetIEditor()->IsUndoRecording() && obj->IsSelected())
			GetIEditor()->RecordUndo( new CUndoBaseObjectSelect(obj) );

		SetObjectSelected( obj,false );
	}
}

//////////////////////////////////////////////////////////////////////////
void	CObjectManager::Display( DisplayContext &dc )
{
	Vec3 org;

	FUNCTION_PROFILER( GetIEditor()->GetSystem(),PROFILE_EDITOR );

	if (!m_bVisibleObjectValid || gSettings.objectHideMask != m_lastHideMask)
	{
		m_lastHideMask = gSettings.objectHideMask;
		UpdateVisibilityList();
	}

	if (!dc.settings->IsDisplayHelpers())
	{
		return;
	}

	CCamera camera = GetIEditor()->GetSystem()->GetViewCamera();
	BBox bbox;

	if (dc.flags & DISPLAY_2D)
	{
		int numVis = m_visibleObjects.size();
		for (int i = 0; i < numVis; i++)
		{
			CBaseObject *obj = m_visibleObjects[i];

			obj->GetBoundBox( bbox );
			if (dc.box.IsIntersectBox( bbox ))
			{
				obj->Display( dc );
			}
		}
	}
	else
	{
		m_displayedObjects.resize(0);
		m_displayedObjects.reserve( m_visibleObjects.size() );

		int numVis = m_visibleObjects.size();
		for (int i = 0; i < numVis; i++)
		{
			CBaseObject *obj = m_visibleObjects[i];

			obj->GetBoundBox( bbox );
			if (camera.IsAABBVisibleFast( AABB(bbox.min,bbox.max) ))
				//if (camera.CheckOverlap(AABB(bbox.min,bbox.max)) != CULL_EXCLUSION)
			{
				m_displayedObjects.push_back( obj );
				obj->Display( dc );
			}
		}
	}

	if (m_gizmoManager)
	{
		m_gizmoManager->Display( dc );
	}
}

void CObjectManager::BeginEditParams( CBaseObject *obj,int flags )
{
	ASSERT( obj != 0 );
	if (obj == m_currEditObject)
		return;

	if (GetSelection()->GetCount() > 1)
		return;

	HWND hFocusWnd = GetFocus();
	
	if (m_currEditObject)
	{
		//if (obj->GetClassDesc() != m_currEditObject->GetClassDesc())
		if (!obj->IsSameClass(m_currEditObject))
			EndEditParams( flags );
	}

	m_currEditObject = obj;

	if (flags & OBJECT_CREATE)
	{
		// Unselect all other objects.
		ClearSelection();
		// Select this object.
		SelectObject( obj );
	}
	
	m_bSingleSelection = true;
	m_currEditObject->BeginEditParams( GetIEditor(),flags );

	// Restore focus if it changed.
	if (hFocusWnd != GetFocus() && hFocusWnd)
		SetFocus( hFocusWnd );
}
	
void CObjectManager::EndEditParams( int flags )
{
	if (m_currEditObject)
	{
		if (m_bSingleSelection)
			m_currEditObject->EndEditParams( GetIEditor() );
		else
			m_currEditObject->EndEditMultiSelParams();
	}
	m_bSingleSelection = false;
	m_currEditObject = 0;
	m_bSelectionChanged = false;
}

//! Select objects withing specified distance from givven position.
int CObjectManager::SelectObjects( const BBox &box,bool bUnselect )
{
	int numSel = 0;

	BBox objBounds;
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;

		if (obj->IsHidden())
			continue;
		if (obj->IsFrozen())
			continue;

		if (obj->GetGroup())
			continue;
		

		obj->GetBoundBox( objBounds );
		if (box.IsIntersectBox(objBounds))
		{
			numSel++;
			if (!bUnselect)
				SelectObject( obj );
			else
				UnselectObject( obj );
		}
		// If its group.
		if (obj->GetRuntimeClass() == RUNTIME_CLASS(CGroup))
		{
			numSel += ((CGroup*)obj)->SelectObjects( box,bUnselect );
		}
	}
	return numSel;
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::DeleteSelection()
{
	// Make sure to unlock selection.
	GetIEditor()->LockSelection( false );

	int i;
	std::vector<CBaseObjectPtr> objects;
	for (i = 0; i < m_currSelection->GetCount(); i++)
	{
		objects.push_back( m_currSelection->GetObject(i) );
	}
	
	RemoveSelection( m_currSelection->GetName() );
	m_currSelection = &m_defaultSelection;
	m_defaultSelection.RemoveAll();

	// When deleting group, subobject of group must become selected.
	std::vector<CBaseObjectPtr> toselect;

	for (i = 0; i < objects.size(); i++)
	{
		DeleteObject( objects[i] );
	}
}

/*
//! Check if new hit object is better suitable for selection then former selected hit object.
//! @return true if obj1 is better for selection.
inline bool CompareHitObjects( CBaseObject *obj1,HitContext &hc1,CBaseObject *obj2,HitContext &hc2 )
{
	if (hc1.geometryHit && hc2.geometryHit)
	{
		if (hc1.dist < hc2.dist)
			return true;
		else
			return false;
	}
	if (!hc1.geometryHit && hc2.geometryHit)
	{
		return false;
	}

}
*/

//////////////////////////////////////////////////////////////////////////
bool CObjectManager::HitTest( Vec3 &raySrc,Vec3 &rayDir,float distanceTollerance,ObjectHitInfo &hitInfo )
{
	FUNCTION_PROFILER( GetIEditor()->GetSystem(),PROFILE_EDITOR );

	hitInfo.object = 0;
	hitInfo.distance = 0;
	hitInfo.axis = 0;

	HitContext hcOrg;
	hcOrg.view = hitInfo.view;
	if (hcOrg.view)
	{
		hcOrg.b2DViewport = hcOrg.view->GetType() != ET_ViewportCamera;
	}
	hcOrg.point2d = hitInfo.point2d;
	hcOrg.rayDir = GetNormalized(rayDir);
	hcOrg.raySrc = raySrc;
	hcOrg.distanceTollerance = distanceTollerance;

	HitContext hc = hcOrg;
	HitContext hcSelected;
	
	float mindist = FLT_MAX;

	if (!hitInfo.bIgnoreAxis)
	{
		// Test gizmos.
		if (m_gizmoManager->HitTest( hc ))
		{
			if (hc.axis != 0)
			{
				hitInfo.object = hc.object;
				hitInfo.axis = hc.axis;
				hitInfo.distance = hc.dist;
				return true;
			}
		}
	}

	BBox box;
	Vec3 tempPnt;

	// Only HitTest objects, that where previously Displayed.
	CBaseObject *selected = 0;
	int numVis = 0;
	if (hcOrg.b2DViewport)
		numVis = m_visibleObjects.size();
	else
		numVis = m_displayedObjects.size();
	for (int i = 0; i < numVis; i++)
	{
		CBaseObject *obj;
		if (hcOrg.b2DViewport)
			obj = m_visibleObjects[i];
		else
			obj = m_displayedObjects[i];

		if (obj->IsFrozen())
			continue;

		//! Only check root objects.
		if (obj->GetGroup())
			continue;

		obj->GetBoundBox( box );
		if (hitInfo.camera)
		{
			if (!hitInfo.camera->IsAABBVisibleFast( AABB(box.min,box.max)))
			{
				continue;
			}
		}
		else
		{
			if (!box.IsIntersectBox(hitInfo.bounds))
				continue;
		}
		// Fast bounding box check.
		if (!box.IsIntersectRay(raySrc,rayDir,tempPnt))
			continue;

		hc = hcOrg;

		/*
		if (obj->IsSelected())
		{
			if (!hitInfo.bIgnoreAxis)
			{
				int axis = obj->HitTestAxis( hc );
				if (axis != 0)
				{
					hitInfo.object = obj;
					hitInfo.axis = axis;
					hitInfo.distance = hc.dist;
					return true;
				}
			}
		}
		*/

		ObjectType objType = obj->GetType();

		// Check if this object type is masked for selection.
		if (!(objType & gSettings.objectSelectMask))
			continue;

		if (obj->HitTest(hc))
		{
			// Check if this object is nearest.
			if (hc.axis != 0)
			{
				hitInfo.object = obj;
				hitInfo.axis = hc.axis;
				hitInfo.distance = hc.dist;
				return true;
			}
			
			// Compare objects obj and selected.
//			CompareHitObjects( obj,hc,selected,hcSelected );

			// If object is selected prefere unselected object.
			//if (hc.dist < mindist || (selected != 0 && selected->IsSelected()))
			if (hc.dist < mindist)
			{
				// If collided object specified, accept it, overwise take tested object itself.
				CBaseObject *hitObj = hc.object;
				if (!hitObj)
					hitObj = obj;
				hc.object = 0;

				// If object is selected prefere unselected object.
				//if (selected != 0 && !selected->IsSelected() && hitObj->IsSelected())
					//continue;

				mindist = hc.dist;
				selected = hitObj;
				//hcSelected = hc;
			}
		}
	}

	if (selected)
	{
		hitInfo.object = selected;
		hitInfo.distance = mindist;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::SelectObjectsInRect( CViewport *view,const CRect &rect,bool bSelect )
{
	// Ignore too small rectangles.
	if (rect.Width() < 1 || rect.Height() < 1)
		return;

	CUndo undo( "Select Object(s)" );

	BBox box;

	HitContext hc;
	hc.view = view;
	hc.b2DViewport = view->GetType() != ET_ViewportCamera;
	hc.rect = rect;

	int numVis;
	if (hc.b2DViewport)
		numVis = m_visibleObjects.size();
	else
		numVis = m_displayedObjects.size();
	for (int i = 0; i < numVis; i++)
	{
		CBaseObject *pObj;
		if (hc.b2DViewport)
			pObj = m_visibleObjects[i];
		else
			pObj = m_displayedObjects[i];
		
		if (!pObj->IsSelectable())
			continue;

		// Retrieve world space bound box.
		pObj->GetBoundBox( box );
		
		// Check if object visible in viewport.
		if (!view->IsBoundsVisible(box))
			continue;

		if (pObj->HitTestRect(hc))
		{
			if (bSelect)
				SelectObject(pObj);
			else
				UnselectObject(pObj);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::RegisterObjectName( const CString &name )
{
	// Remove all numbers from the end of typename.
	CString typeName = name;
	int nameLen = typeName.GetLength();
	int len = nameLen;
	while (len > 0 && isdigit(typeName[len-1]))
		len--;

	typeName = typeName.Left(len);

	int num = 0;
	if (len < nameLen)
		num = atoi((const char*)name+len) + 0;

	int lastNumber = stl::find_in_map( m_nameNumbersMap,typeName,0 );
	int newLastNumber = MAX( num,lastNumber );
	if (newLastNumber != lastNumber)
		m_nameNumbersMap[typeName] = newLastNumber;
}

//////////////////////////////////////////////////////////////////////////
CString	CObjectManager::GenUniqObjectName( const CString &theTypeName )
{
	if (!m_bGenUniqObjectNames)
		return theTypeName;

	// Remove all numbers from the end of typename.
	CString typeName = theTypeName;
	int len = typeName.GetLength();
	while (len > 0 && isdigit(typeName[len-1]))
		len--;

	typeName = typeName.Left(len);

	int lastNumber = stl::find_in_map( m_nameNumbersMap,typeName,0 );
	lastNumber++;
	m_nameNumbersMap[typeName] = lastNumber;

	CString str;
	str.Format( "%s%d",(const char*)typeName,lastNumber );

	/*
	CString tpName = typeName;
	char str[1024];
	int num = 0;
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;
		const char *name = it->second->GetName();
		if (strncmp(name,tpName,len) == 0)
		{
			int n = atoi(name+len) + 1;
			num = MAX( num,n );
		}
	}
	//sprintf( str,"%s%02d",(const char*)typeName,num );
	sprintf( str,"%s%d",(const char*)typeName,num );
	*/
	return str;
}

//////////////////////////////////////////////////////////////////////////
bool CObjectManager::EnableUniqObjectNames( bool bEnable )
{
	bool bPrev = m_bGenUniqObjectNames;
	m_bGenUniqObjectNames = bEnable;
	return bPrev;
}

//////////////////////////////////////////////////////////////////////////
CObjectClassDesc* CObjectManager::FindClass( const CString &className )
{
	IClassDesc *cls = CClassFactory::Instance()->FindClass( className );
	if (cls != NULL && cls->SystemClassID() == ESYSTEM_CLASS_OBJECT)
	{
		return (CObjectClassDesc*)cls;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::GetClassCategories( std::vector<CString> &categories )
{
	std::vector<IClassDesc*> classes;
	CClassFactory::Instance()->GetClassesBySystemID( ESYSTEM_CLASS_OBJECT,classes );
	std::set<CString> cset;
	for (int i = 0; i < classes.size(); i++)
	{
		const char *category = classes[i]->Category();
		if (strlen(category) > 0)
			cset.insert( category );
	}
	categories.clear();
	categories.reserve( cset.size() );
	for (std::set<CString>::iterator cit = cset.begin(); cit != cset.end(); ++cit)
	{
		categories.push_back( *cit );
	}
}
	
void CObjectManager::GetClassTypes( const CString &category,std::vector<CString> &types )
{
	std::vector<IClassDesc*> classes;
	CClassFactory::Instance()->GetClassesBySystemID( ESYSTEM_CLASS_OBJECT,classes );
	for (int i = 0; i < classes.size(); i++)
	{
		const char* cat = classes[i]->Category();
		if (stricmp(cat,category) == 0)
		{
			types.push_back( classes[i]->ClassName() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::RegisterClassTemplate( XmlNodeRef &templ )
{
	CString typeName = templ->getTag();
	CString superTypeName;
	if (!templ->getAttr( "SuperType",superTypeName ))
		return;

	CObjectClassDesc *superType = FindClass( superTypeName );
	if (!superType)
		return;

	CString category,fileSpec,initialName;
	templ->getAttr( "Category",category );
	templ->getAttr( "File",fileSpec );
	templ->getAttr( "Name",initialName );

	CXMLObjectClassDesc *classDesc = new CXMLObjectClassDesc;
	classDesc->superType = superType;
	classDesc->type = typeName;
	classDesc->category = category;
	classDesc->fileSpec = fileSpec;
	CoCreateGuid( &classDesc->guid );
	//classDesc->properties = templ->findChild( "Properties" );

	CClassFactory::Instance()->RegisterClass( classDesc );
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::LoadClassTemplates( const CString &path )
{
	XmlParser parser;

	CString dir = Path::AddBackslash(path);

	std::vector<CFileUtil::FileDesc> files;
	CFileUtil::ScanDirectory( dir,"*.xml",files,false );

	for (int k = 0; k < files.size(); k++)
	{

		// Construct the full filepath of the current file
		XmlNodeRef node = parser.parse( dir + files[k].filename );
		if (node != 0 && node->isTag("ObjectTemplates"))
		{
			CString name;
			for (int i = 0; i < node->getChildCount(); i++)
			{
				RegisterClassTemplate( node->getChild(i) );
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::Serialize( XmlNodeRef &xmlNode,bool bLoading,int flags )
{
	if (!xmlNode)
		return;

	bool bIgnoreExternal = flags & SERIALIZE_IGNORE_EXTERNAL;

	if (bLoading)
	{
		m_loadedObjects = 0;

		if (flags == SERIALIZE_ONLY_NOTSHARED)
			DeleteNotSharedObjects();
		else if (flags == SERIALIZE_ONLY_SHARED)
			DeleteSharedObjects();
		else
			DeleteAllObjects();


		XmlNodeRef root = xmlNode->findChild( "Objects" );
		
		int totalObjects = 0;
		if (root)
			root->getAttr( "NumObjects",totalObjects );

		StartObjectsLoading( totalObjects );

		// Load layers.
		CObjectArchive ar( this,xmlNode,true );
	
		// Load layers.
		m_pLayerManager->Serialize( ar,bIgnoreExternal );

		// Loading.
		if (root)
		{
			ar.node = root;
			LoadObjects( ar,false );
		}
		EndObjectsLoading();
	}
	else
	{
		// Saving.
		XmlNodeRef root = xmlNode->newChild( "Objects" );

		CObjectArchive ar( this,root,false );

		int totalObjects = m_objects.size();
		root->setAttr( "NumObjects",totalObjects );

		// Save all objects to XML.
		for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
		{
			CBaseObject *obj = it->second;
			if (obj->GetGroup())
				continue;

			if ((flags == SERIALIZE_ONLY_SHARED) && !obj->CheckFlags(OBJFLAG_SHARED))
				continue;
			else if ((flags == SERIALIZE_ONLY_NOTSHARED) && obj->CheckFlags(OBJFLAG_SHARED))
				continue;

			// Not save objects in prefabs.
			if (obj->CheckFlags(OBJFLAG_PREFAB))
				continue;

			CObjectLayer *pLayer = obj->GetLayer();
			if (pLayer->IsExternal() || pLayer->IsParentExternal())
				continue;
			
			XmlNodeRef objNode = root->newChild( "Object" );
			ar.node = objNode;
			obj->Serialize( ar );
		}

		// Save layers.
		ar.node = xmlNode;
		m_pLayerManager->Serialize( ar,bIgnoreExternal );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::LoadObjects( CObjectArchive &objectArchive,bool bSelect )
{
	XmlNodeRef objectsNode = objectArchive.node;
	int numObjects = objectsNode->getChildCount();
	for (int i = 0; i < numObjects; i++)
	{
		objectArchive.node = objectsNode->getChild(i);
		CBaseObject *obj = objectArchive.LoadObject( objectsNode->getChild(i) );
		if (obj && bSelect)
			SelectObject( obj );
	}
	objectArchive.ResolveObjects();

	InvalidateVisibleList();
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::Export( const CString &levelPath,XmlNodeRef &rootNode,bool onlyShared )
{
	// Clear export files.
	DeleteFile( levelPath+"TagPoints.ini" );
	DeleteFile( levelPath+"Volumes.ini" );
	
	// Save all objects to XML.
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;
		CObjectLayer *pLayer = obj->GetLayer();
		if (!pLayer->IsExportable())
			continue;
		// Export Only shared objects.
		if ((obj->CheckFlags(OBJFLAG_SHARED) && onlyShared) ||
				(!obj->CheckFlags(OBJFLAG_SHARED) && !onlyShared))
		{
			obj->Export( levelPath,rootNode );
		}
	}
}

void CObjectManager::DeleteNotSharedObjects()
{
	std::vector<CBaseObjectPtr> objects;
	GetAllObjects( objects );
	for (int i = 0; i < objects.size(); i++)
	{
		CBaseObject *obj = objects[i];
		if (!obj->CheckFlags(OBJFLAG_SHARED))
		{
			DeleteObject( obj );
		}
	}
}

void CObjectManager::DeleteSharedObjects()
{
	std::vector<CBaseObjectPtr> objects;
	GetAllObjects( objects );
	for (int i = 0; i < objects.size(); i++)
	{
		CBaseObject *obj = objects[i];
		if (obj->CheckFlags(OBJFLAG_SHARED))
		{
			DeleteObject( obj );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
IObjectSelectCallback* CObjectManager::SetSelectCallback( IObjectSelectCallback* callback )
{
	IObjectSelectCallback* prev = m_selectCallback;
	m_selectCallback = callback;
	return prev;
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::InvalidateVisibleList()
{
	m_bVisibleObjectValid = false;
	m_visibleObjects.clear();
	m_displayedObjects.clear();
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::UpdateVisibilityList()
{
	m_visibleObjects.clear();
	for (Objects::iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;
		bool visible = obj->IsPotentiallyVisible();
    obj->UpdateVisibility( visible );
		if (visible)
		{
			// Prefabs are not added into visible list.
			if (!obj->CheckFlags(OBJFLAG_PREFAB))
				m_visibleObjects.push_back( obj );
		}
	}
	m_bVisibleObjectValid = true;
}

//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjectManager::FindAnimNodeOwner( IAnimNode* node ) const
{
	for (Objects::const_iterator it = m_objects.begin(); it != m_objects.end(); ++it)
	{
		CBaseObject *obj = it->second;
		if (obj->GetAnimNode() == node)
		{
			return obj;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CObjectManager::ConvertToType( CBaseObject *object,ObjectType objectType )
{
	if (objectType == OBJTYPE_BRUSH)
	{
		if (object->IsKindOf(RUNTIME_CLASS(CEntity)))
		{
			CUndo undo( "Convert To Brush" );
			// Can convert Entity to brush.
			CBaseObjectPtr brushObject = GetIEditor()->NewObject( "Brush" );
			if (!brushObject)
				return false;

			if (!brushObject->ConvertFromObject( object ))
			{
				// delete this object.
				DeleteObject( brushObject );
				return false;
			}
			DeleteObject( object );
			return true;
		}
	}
	else if (objectType == OBJTYPE_ENTITY)
	{
		if (object->IsKindOf(RUNTIME_CLASS(CBrushObject)))
		{
			CUndo undo( "Convert To SimpleEntity" );
			// Can convert Entity to brush.
			CBaseObjectPtr ent = GetIEditor()->NewObject( "SimpleEntity" );
			if (!ent)
				return false;

			if (!ent->ConvertFromObject( object ))
			{
				// delete this object.
				DeleteObject( ent );
				return false;
			}
			DeleteObject( object );
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::SetObjectSelected( CBaseObject* pObject,bool bSelect )
{
	// Only select/unselect once.
	if ((pObject->IsSelected() && bSelect) || (!pObject->IsSelected() && !bSelect))
		return;

	// Store selection undo.
	if (CUndo::IsRecording())
		CUndo::Record( new CUndoBaseObjectSelect(pObject) );

	pObject->SetSelected(bSelect);
	m_bSelectionChanged = true;

	if (bSelect)
	{
		if (CAxisGizmo::GetGlobalAxisGizmoCount() < gSettings.gizmo.axisGizmoMaxCount)
		{
			// Create axis gizmo for this object.
			m_gizmoManager->AddGizmo( new CAxisGizmo(pObject) );
		}
	}
	if (bSelect)
		NotifyObjectListeners( pObject,CBaseObject::ON_SELECT );
	else
		NotifyObjectListeners( pObject,CBaseObject::ON_UNSELECT );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void CObjectManager::AddObjectEventListener( const EventCallback &cb )
{
	stl::push_back_unique( m_objectEventListeners,cb );
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::RemoveObjectEventListener( const EventCallback &cb )
{
	stl::find_and_erase( m_objectEventListeners,cb );
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::NotifyObjectListeners( CBaseObject *pObject,CBaseObject::EObjectListenerEvent event )
{
	std::list<EventCallback>::iterator next;
	for (std::list<EventCallback>::iterator it = m_objectEventListeners.begin(); it != m_objectEventListeners.end(); it = next)
	{
		next = it;
		++next;
		// Call listener callback.
		(*it)( pObject,event );
	}
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::StartObjectsLoading( int numObjects )
{
	if (m_pLoadProgress)
		return;
	m_pLoadProgress = new CWaitProgress( _T("Loading Objects") );
	m_totalObjectsToLoad = numObjects;
	m_loadedObjects = 0;
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::EndObjectsLoading()
{
	if (m_pLoadProgress)
		delete m_pLoadProgress;
	m_pLoadProgress = 0;
}

//////////////////////////////////////////////////////////////////////////
void CObjectManager::GatherUsedResources( CUsedResources &resources )
{
	CBaseObjectsArray objects;
	GetIEditor()->GetObjectManager()->GetObjects( objects );

	for (int i = 0; i < objects.size(); i++)
	{
		CBaseObject *pObject = objects[i];
		pObject->GatherUsedResources( resources );
	}
}

//////////////////////////////////////////////////////////////////////////
IGizmoManager* CObjectManager::GetGizmoManager()
{
	return m_gizmoManager;
}
