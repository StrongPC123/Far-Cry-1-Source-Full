////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   ObjectManager.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: ObjectManager definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __ObjectManager_h__
#define __ObjectManager_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IObjectManager.h"
#include "BaseObject.h"
#include "SelectionGroup.h"
#include "ClassDesc.h"
#include "ObjectLayer.h"
#include "ObjectLayerManager.h"

// forward declarations.
class CGizmoManager;
class CCameraObject;

/*!
 *  CObjectManager is a singleton object that
 *  manages global set of objects in level.	
 */
class CObjectManager : public IObjectManager
{
public:
	//! Selection functor callback.
	//! Callback function must return a boolean value.
	//! Return true if selection should proceed, or false to abort object selection.
	CObjectManager();
	~CObjectManager();

	void RegisterObjectClasses();

	CBaseObject* NewObject( CObjectClassDesc *cls,CBaseObject *prev=0,const CString &file="" );
	CBaseObject* NewObject( const CString &typeName,CBaseObject *prev=0,const CString &file="" );
	
	/** Creates and serialize object from xml node.
		@param objectNode Xml node to serialize object info from.
		@param pUndoObject Pointer to deleted object for undo.
	*/
	CBaseObject* NewObject( CObjectArchive &archive,CBaseObject *pUndoObject=0,bool bMakeNewId=false );
	
	void	DeleteObject( CBaseObject *obj );
	void	DeleteAllObjects();
	CBaseObject* CloneObject( CBaseObject *obj );

	void BeginEditParams( CBaseObject *obj,int flags );
	void EndEditParams( int flags=0 );

	//! Get number of objects manager by ObjectManager, (not contain sub objects of groups).
	int		GetObjectCount() const;
	
	//! Get array of objects, managed by manager, (not contain sub objects of groups).
	//! @param layer if 0 get objects for all layers, or layer to get objects from.
	void	GetObjects( CBaseObjectsArray &objects,CObjectLayer *layer=0 );

	//! Get all camera sources in object manager
	void	GetCameras( std::vector<CCameraObject*> &objects );
	
	//! Update objects.
	void	Update();

	//! Display objects on display context.
	void	Display( DisplayContext &dc );

	//! Check intersection with objects.
	//! Find intersection with nearest to ray origin object hit by ray.
	//! If distance tollerance is specified certain relaxation applied on collision test.
	//! @return true if hit any object, and fills hitInfo structure.
	bool HitTest( Vec3 &raySrc,Vec3 &rayDir,float distanceTollerance,ObjectHitInfo &hitInfo );

	//! Send event to all objects.
	//! Will cause OnEvent handler to be called on all objects.
	void	SendEvent( ObjectEvent event );

	//! Send event to all objects within givven bounding box.
	//! Will cause OnEvent handler to be called on objects withing bounding box.
	void	SendEvent( ObjectEvent event,const BBox &bounds );

	//////////////////////////////////////////////////////////////////////////
	//! Find object by ID.
	CBaseObject* FindObject( REFGUID guid ) const;
	//////////////////////////////////////////////////////////////////////////
	//! Find object by name.
	CBaseObject* FindObject( const CString &sName ) const;

	//////////////////////////////////////////////////////////////////////////
	//! Find BaseObject who is owner of specified animation node.
	CBaseObject*	FindAnimNodeOwner( IAnimNode* node ) const;

	//////////////////////////////////////////////////////////////////////////
	// Operations on objects.
	//////////////////////////////////////////////////////////////////////////
	//! Makes object visible or invisible.
	void HideObject( CBaseObject *obj,bool hide );
	//! Freeze object, making it unselectable.
	void FreezeObject( CBaseObject *obj,bool freeze );
	//! Unhide all hidden objects.
	void UnhideAll();
	//! Unfreeze all frozen objects.
	void UnfreezeAll();

	//////////////////////////////////////////////////////////////////////////
	// Object Selection.
	//////////////////////////////////////////////////////////////////////////
	bool	SelectObject( CBaseObject *obj );
	void	UnselectObject( CBaseObject *obj );

	//! Select objects withing specified distance from givven position.
	//! Return number of selected objects.
	int SelectObjects( const BBox &box,bool bUnselect=false );

	//! Selects/Unselects all objects within 2d rectangle in given viewport.
	void SelectObjectsInRect( CViewport *view,const CRect &rect,bool bSelect );

	//! Clear default selection set.
	//! @Return number of objects removed from selection.
	int	ClearSelection();

	//! Deselect all current selected objects and selects object that were unselected.
	//! @Return number of selected objects.
	int InvertSelection();

	//! Get current selection.
	CSelectionGroup*	GetSelection() const { return m_currSelection; };
	//! Get named selection.
	CSelectionGroup*	GetSelection( const CString &name ) const;
	//! Change name of current selection group.
	//! And store it in list.
	void	NameSelection( const CString &name );
	//! Set one of name selections as current selection.
	void	SetSelection( const CString &name );
	void	RemoveSelection( const CString &name );

	//! Delete all objects in selection group.
	void DeleteSelection();

	//! Generates uniq name base on type name of object.
	CString	GenUniqObjectName( const CString &typeName );
	//! Register object name in object manager, needed for generating uniq names.
	void RegisterObjectName( const CString &name );
	//! Enable/Disable generating of unique object names (Enabled by default).
	//! Return previous value.
	bool EnableUniqObjectNames( bool bEnable );

	//! Register XML template of runtime class.
	void	RegisterClassTemplate( XmlNodeRef &templ );
	//! Load class templates for specified directory,
	void	LoadClassTemplates( const CString &path );
	
	//! Find object class by name.
	CObjectClassDesc* FindClass( const CString &className );
	void	GetClassCategories( std::vector<CString> &categories );
	void	GetClassTypes( const CString &category,std::vector<CString> &types );

	//! Export objects to xml.
	//! When onlyShared is true ony objects with shared flags exported, overwise only not shared object exported.
	void	Export( const CString &levelPath,XmlNodeRef &rootNode,bool onlyShared );
	
	//! Serialize Objects in manager to specified XML Node.
	//! @param flags Can be one of SerializeFlags.
	void	Serialize( XmlNodeRef &rootNode,bool bLoading,int flags=SERIALIZE_ALL );

	//! Load objects from object archive.
	//! @param bSelect if set newly loaded object will be selected.
	void LoadObjects( CObjectArchive &ar,bool bSelect );

	//! Delete from Object manager all objects without SHARED flag.
	void	DeleteNotSharedObjects();
	//! Delete from Object manager all objects with SHARED flag.
	void	DeleteSharedObjects();

	bool AddObject( CBaseObject *obj );
	void RemoveObject( CBaseObject *obj );
	void ChangeObjectId( CBaseObject *obj,int newId );
	void ChangeObjectName( CBaseObject *obj,const CString &newName );

	//! Convert object of one type to object of another type.
	//! Original object is deleted.
	bool ConvertToType( CBaseObject *object,ObjectType objectType );

	//! Set new selection callback.
	//! @return previous selection callback.
	IObjectSelectCallback* SetSelectCallback( IObjectSelectCallback* callback );
	
	// Enables/Disables creating of game objects.
	void SetCreateGameObject( bool enable ) { m_createGameObjects = enable; };
	//! Return true if objects loaded from xml should immidiatly create game objects associated with them.
	bool IsCreateGameObjects() const { return m_createGameObjects; };

	//void ExportLayer( CObjectLayer *pLayer,CObjectArchive &ar );
	//CObjectLayer* ImportLayer( CObjectArchive &ar );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//! Get access to gizmo manager.
	IGizmoManager* GetGizmoManager();

	//////////////////////////////////////////////////////////////////////////
	//! Get acess to object layers manager.
	CObjectLayerManager* GetLayersManager() const { return m_pLayerManager; }

	//////////////////////////////////////////////////////////////////////////
	//! Invalidate visibily settings of objects.
	void InvalidateVisibleList();

	//////////////////////////////////////////////////////////////////////////
	// ObjectManager notification Callbacks.
	//////////////////////////////////////////////////////////////////////////
	void AddObjectEventListener( const EventCallback &cb );
	void RemoveObjectEventListener( const EventCallback &cb );

	//////////////////////////////////////////////////////////////////////////
	// Used to indicate starting and ending of objects loading.
	//////////////////////////////////////////////////////////////////////////
	void StartObjectsLoading( int numObjects );
	void EndObjectsLoading();

	//////////////////////////////////////////////////////////////////////////
	// Gathers all resources used by all objects.
	void GatherUsedResources( CUsedResources &resources );

private:
	//! Update visibility of all objects.
	void UpdateVisibilityList();
	//! Get array of all objects in manager.
	void GetAllObjects( std::vector<CBaseObjectPtr> &objects ) const;

	void UnselectCurrent();
	void SelectCurrent();
	void SetObjectSelected( CBaseObject* pObject,bool bSelect );

	void SaveRegistry();
	void LoadRegistry();

	void NotifyObjectListeners( CBaseObject *pObject,CBaseObject::EObjectListenerEvent event );

private:
	typedef std::map<GUID,CBaseObjectPtr,guid_less_predicate> Objects;
	Objects m_objects;

	std::map<CString,CSelectionGroup*> m_selections;

	//! Array of currently visible objects.
	std::vector<CBaseObjectPtr> m_visibleObjects;
	bool m_bVisibleObjectValid;
	unsigned int m_lastHideMask;
	//! List of objects that was displayed at last frame.
	std::vector<CBaseObject*> m_displayedObjects;

	//////////////////////////////////////////////////////////////////////////
	// Selection.
	//! Current selection group.
	CSelectionGroup* m_currSelection;
	int m_nLastSelCount;
	bool m_bSelectionChanged;
	IObjectSelectCallback* m_selectCallback;

	//! Default selection.
	CSelectionGroup m_defaultSelection;

	CBaseObjectPtr m_currEditObject;
	bool m_bSingleSelection;

	bool m_createGameObjects;
	bool m_bGenUniqObjectNames;
	
	// Object manager also handles Gizmo manager.
	CGizmoManager* m_gizmoManager;

	friend CObjectLayerManager;
	//! Manager of object layers.
	CObjectLayerManager* m_pLayerManager;

	//////////////////////////////////////////////////////////////////////////
	// Loading progress.
	CWaitProgress *m_pLoadProgress;
	int m_loadedObjects;
	int m_totalObjectsToLoad;
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Numbering for names.
	//////////////////////////////////////////////////////////////////////////
	typedef std::map<CString,int,stl::less_stricmp<CString> > NameNumbersMap;
	NameNumbersMap m_nameNumbersMap;

	//////////////////////////////////////////////////////////////////////////
	// Listeners.
	std::list<EventCallback> m_objectEventListeners;
};

#endif // __ObjectManager_h__
