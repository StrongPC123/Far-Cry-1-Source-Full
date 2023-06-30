////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   IObjectManager.h
//  Version:     v1.00
//  Created:     4/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IObjectManager_h__
#define __IObjectManager_h__
#pragma once

// forward declarations.
struct DisplayContext;
struct IGizmoManager;
class CObjectLayer;
struct IAnimNode;
class CUsedResources;
class CSelectionGroup;
class CObjectLayerManager;
class CObjectClassDesc;
class CObjectArchive;

#include "ObjectEvent.h"

enum SerializeFlags
{
	SERIALIZE_ALL = 0,
	SERIALIZE_ONLY_SHARED = 1,
	SERIALIZE_ONLY_NOTSHARED = 2,
	SERIALIZE_IGNORE_EXTERNAL = 4,
};

//////////////////////////////////////////////////////////////////////////
typedef std::vector<CBaseObject*> CBaseObjectsArray;

struct IObjectSelectCallback
{
	//! Called when object is selected.
	//! Return true if selection should proceed, or false to abort object selection.
	virtual bool OnSelectObject( CBaseObject *obj ) = 0;
};

struct ObjectHitInfo
{
	CViewport *view;			//!< Viewport that originates hit testing.
	CCamera	*camera;			//!< Camera for hit testing (can be NULL).
	BBox bounds;					//!< World bounds for hit testing (If Camera is NULL).
	CPoint point2d;				//!< 2D point on view that was use to shoot the ray.
	CBaseObject *object;	//!< Object that was hit.
	int axis;							//!< Axis of object that was hit.
	float distance;				//!< Distance to the hit point.
	bool bIgnoreAxis;			//! True if axis collision must be ignored.

	ObjectHitInfo( CViewport *pView,CPoint point )
	{
		view = pView;
		object = 0;
		axis = 0;
		point2d = point;
		distance = 0;
		camera = NULL;
		bounds.min=Vec3d(-10000,-10000,-10000);
		bounds.max=Vec3d(10000,10000,10000);
		bIgnoreAxis = false;
	}
};

//////////////////////////////////////////////////////////////////////////
//
// Interface to access editor objects scene graph.
//
//////////////////////////////////////////////////////////////////////////
struct IObjectManager
{
public:
	//! This callback will be called on response to object event.
	typedef Functor2<CBaseObject*,int> EventCallback;

	virtual CBaseObject* NewObject( CObjectClassDesc *cls,CBaseObject *prev=0,const CString &file="" ) = 0;
	virtual CBaseObject* NewObject( const CString &typeName,CBaseObject *prev=0,const CString &file="" ) = 0;
	virtual CBaseObject* NewObject( CObjectArchive &archive,CBaseObject *pUndoObject=0,bool bMakeNewId=false ) = 0;

	virtual	void DeleteObject( CBaseObject *obj ) = 0;
	virtual void DeleteAllObjects() = 0;
	virtual CBaseObject* CloneObject( CBaseObject *obj ) = 0;

	virtual void BeginEditParams( CBaseObject *obj,int flags ) = 0;
	virtual void EndEditParams( int flags=0 ) = 0;

	//! Get number of objects manager by ObjectManager, (not contain sub objects of groups).
	virtual int GetObjectCount() const = 0;
	
	//! Get array of objects, managed by manager, (not contain sub objects of groups).
	//! @param layer if 0 get objects for all layers, or layer to get objects from.
	virtual void GetObjects( CBaseObjectsArray &objects,CObjectLayer *layer=0 ) = 0;

	//! Display objects on specified display context.
	virtual void Display( DisplayContext &dc ) = 0;

	//! Check intersection with objects.
	//! Find intersection with nearest to ray origin object hit by ray.
	//! If distance tollerance is specified certain relaxation applied on collision test.
	//! @return true if hit any object, and fills hitInfo structure.
	virtual bool HitTest( Vec3 &raySrc,Vec3 &rayDir,float distanceTollerance,ObjectHitInfo &hitInfo ) = 0;

	//! Send event to all objects.
	//! Will cause OnEvent handler to be called on all objects.
	virtual void SendEvent( ObjectEvent event ) = 0;

	//! Send event to all objects within givven bounding box.
	//! Will cause OnEvent handler to be called on objects withing bounding box.
	virtual void SendEvent( ObjectEvent event,const BBox &bounds ) = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Find object by ID.
	virtual CBaseObject* FindObject( REFGUID guid ) const = 0;
	//////////////////////////////////////////////////////////////////////////
	//! Find object by name.
	virtual CBaseObject* FindObject( const CString &sName ) const = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Find BaseObject who is owner of specified animation node.
	virtual CBaseObject* FindAnimNodeOwner( IAnimNode* node ) const = 0;

	//////////////////////////////////////////////////////////////////////////
	// Operations on objects.
	//////////////////////////////////////////////////////////////////////////
	//! Makes object visible or invisible.
	virtual void HideObject( CBaseObject *obj,bool hide ) = 0;
	//! Freeze object, making it unselectable.
	virtual void FreezeObject( CBaseObject *obj,bool freeze ) = 0;
	//! Unhide all hidden objects.
	virtual void UnhideAll() = 0;
	//! Unfreeze all frozen objects.
	virtual void UnfreezeAll() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Object Selection.
	//////////////////////////////////////////////////////////////////////////
	virtual bool	SelectObject( CBaseObject *obj ) = 0;
	virtual void	UnselectObject( CBaseObject *obj ) = 0;

	//! Select objects withing specified distance from givven position.
	//! Return number of selected objects.
	virtual int SelectObjects( const BBox &box,bool bUnselect=false ) = 0;

	//! Selects/Unselects all objects within 2d rectangle in given viewport.
	virtual void SelectObjectsInRect( CViewport *view,const CRect &rect,bool bSelect ) = 0;

	//! Clear default selection set.
	//! @Return number of objects removed from selection.
	virtual int ClearSelection() = 0;

	//! Deselect all current selected objects and selects object that were unselected.
	//! @Return number of selected objects.
	virtual int InvertSelection() = 0;

	//! Get current selection.
	virtual CSelectionGroup*	GetSelection() const = 0;
	//! Get named selection.
	virtual CSelectionGroup*	GetSelection( const CString &name ) const = 0;
	//! Change name of current selection group.
	//! And store it in list.
	virtual void	NameSelection( const CString &name ) = 0;
	//! Set one of name selections as current selection.
	virtual void	SetSelection( const CString &name ) = 0;
	//! Removes one of named selections.
	virtual	void	RemoveSelection( const CString &name ) = 0;

	//! Delete all objects in current selection group.
	virtual void DeleteSelection() = 0;

	//! Generates uniq name base on type name of object.
	virtual CString	GenUniqObjectName( const CString &typeName ) = 0;
	//! Register object name in object manager, needed for generating uniq names.
	virtual void RegisterObjectName( const CString &name ) = 0;
	//! Enable/Disable generating of unique object names (Enabled by default).
	//! Return previous value.
	virtual bool EnableUniqObjectNames( bool bEnable ) = 0;

	//! Find object class by name.
	virtual CObjectClassDesc* FindClass( const CString &className ) = 0;
	virtual void GetClassCategories( std::vector<CString> &categories ) = 0;
	virtual void GetClassTypes( const CString &category,std::vector<CString> &types ) = 0;

	//! Export objects to xml.
	//! When onlyShared is true ony objects with shared flags exported, overwise only not shared object exported.
	virtual void Export( const CString &levelPath,XmlNodeRef &rootNode,bool onlyShared ) = 0;
	
	//! Serialize Objects in manager to specified XML Node.
	//! @param flags Can be one of SerializeFlags.
	virtual void Serialize( XmlNodeRef &rootNode,bool bLoading,int flags=SERIALIZE_ALL ) = 0;

	//! Load objects from object archive.
	//! @param bSelect if set newly loaded object will be selected.
	virtual void LoadObjects( CObjectArchive &ar,bool bSelect ) = 0;

	virtual void ChangeObjectId( CBaseObject *obj,int newId ) = 0;
	virtual void ChangeObjectName( CBaseObject *obj,const CString &newName ) = 0;

	//! Convert object of one type to object of another type.
	//! Original object is deleted.
	virtual bool ConvertToType( CBaseObject *object,ObjectType objectType ) = 0;

	//! Set new selection callback.
	//! @return previous selection callback.
	virtual IObjectSelectCallback* SetSelectCallback( IObjectSelectCallback* callback ) = 0;
	
	// Enables/Disables creating of game objects.
	virtual void SetCreateGameObject( bool enable ) = 0;
	//! Return true if objects loaded from xml should immidiatly create game objects associated with them.
	virtual bool IsCreateGameObjects() const = 0;

	virtual IGizmoManager* GetGizmoManager() = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Get acess to object layers manager.
	virtual CObjectLayerManager* GetLayersManager() const = 0;

	//////////////////////////////////////////////////////////////////////////
	//! Invalidate visibily settings of objects.
	virtual void InvalidateVisibleList() = 0;

	//////////////////////////////////////////////////////////////////////////
	// ObjectManager notification Callbacks.
	//////////////////////////////////////////////////////////////////////////
	virtual void AddObjectEventListener( const EventCallback &cb ) = 0;
	virtual void RemoveObjectEventListener( const EventCallback &cb ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// Used to indicate starting and ending of objects loading.
	//////////////////////////////////////////////////////////////////////////
	virtual void StartObjectsLoading( int numObjects ) = 0;
	virtual void EndObjectsLoading() = 0;

	//////////////////////////////////////////////////////////////////////////
	// Gathers all resources used by all objects.
	virtual void GatherUsedResources( CUsedResources &resources ) = 0;
};

#endif // __IObjectManager_h__
