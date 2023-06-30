////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   BaseObject.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Definition of basic Editor object.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __BaseObject_h__
#define __BaseObject_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "IObjectManager.h"
#include "ClassDesc.h"
#include "DisplayContext.h"
#include "ObjectLoader.h"

//////////////////////////////////////////////////////////////////////////
// forward declarations.
class CGroup;
class CUndoBaseObject;
class CObjectManager;
class CObjectLayer;
class CGizmo;
class CObjectArchive;
class CMaterial;

//! Collision structure passed to HitTest function.
struct HitContext
{
	CViewport *view;			//!< Viewport that originates hit testing.
	bool	b2DViewport;		//!< Testing performed in 2D viewport.
	CPoint point2d;				//!< 2D point on view that was use to shoot the ray.
	CRect rect;						//!< 2d Selection rectangle (Only when HitTestRect)

	// Input parameters.
	Vec3 raySrc;								//!< Ray origin.
	Vec3 rayDir;								//!< Ray direction.
	float distanceTollerance;		//!< Relaxation parameter for hit testing.
	bool weakHit;								//!< True if this hit should have less priority then non weak hits.
		//													//!< (exp: Ray hit entity bounding box but not entity geometry.)
	int axis;										//! Collision axis.

	// Output parameters.
	//! Distance to the object from src.
	float dist;
	//! Actual object that been hit.
	CBaseObject* object;

	//Ctor
	HitContext()
	{
		rect.top = 0;
		rect.left = 0;
		rect.bottom = 0;
		rect.right = 0;
		b2DViewport = false;
		view = 0;
		point2d.x = 0; point2d.y = 0;
		axis = 0;
		distanceTollerance = 0;
		raySrc(0,0,0);
		rayDir(0,0,0);
		dist = 0;
		object = 0;
		weakHit = false;
//		geometryHit = false;
	}
};

//////////////////////////////////////////////////////////////////////////
/*!
This class used for object references remapping dusring cloning operation.
*/
class CObjectCloneContext
{
public:
	//! Add cloned object.
	SANDBOX_API void AddClone( CBaseObject *pFromObject,CBaseObject *pToObject );

	//! Find cloned object for givven object.
	SANDBOX_API CBaseObject* FindClone( CBaseObject *pFromObject );

private:
	typedef std::map<CBaseObject*,CBaseObject*> ObjectsMap;
	ObjectsMap m_objectsMap;
};

//////////////////////////////////////////////////////////////////////////
enum ObjectFlags
{
	OBJFLAG_SELECTED	= 0x0001,	//!< Object is selected. (Do not set this flag explicitly).
	OBJFLAG_HIDDEN		= 0x0002,	//!< Object is hidden.
	OBJFLAG_FROZEN		= 0x0004,	//!< Object is frozen (Visible but cannot be selected)
	OBJFLAG_FLATTEN		= 0x0008,	//!< Flatten area arround object.
	OBJFLAG_SHARED		= 0x0010,	//!< This object is shared between missions.

	OBJFLAG_PREFAB		= 0x0020,	//!< This object is part of prefab object.

	// object is in editing mode.
	OBJFLAG_EDITING		= 0x01000,
	OBJFLAG_ATTACHING	= 0x02000,	//!< Object in attaching to group mode.
	OBJFLAG_DELETED		= 0x04000,	//!< This object is deleted.
	OBJFLAG_HIGHLIGHT	= 0x08000,	//!< Object is highlighted (When mouse over).
	OBJFLAG_VISIBLE		= 0x10000,	//!< This object is visible.

	OBJFLAG_PERSISTMASK = OBJFLAG_HIDDEN|OBJFLAG_FROZEN|OBJFLAG_FLATTEN,
};

//////////////////////////////////////////////////////////////////////////
//! This flags passed to CBaseObject::BeginEditParams method.
enum ObjectEditFlags
{
	OBJECT_CREATE	= 0x001,
	OBJECT_EDIT		= 0x002,
};

//////////////////////////////////////////////////////////////////////////
//! Return values from CBaseObject::MouseCreateCallback method.
enum MouseCreateResult
{
	MOUSECREATE_CONTINUE = 0,	//!< Continue placing this object.
	MOUSECREATE_ABORT,				//!< Abort creation of this object.
	MOUSECREATE_OK,						//!< Accept this object.
};

/*!
 *	CBaseObject is the base class for all objects which can be placed in map.
 *	Every object belongs to class specified by ClassDesc.
 *	Specific object classes must ovveride this class, to provide specific functionality.
 *	Objects are reference counted and only destroyed when last reference to object
 *	is destroyed.
 *
 */
class CBaseObject : public CVarObject
{
	DECLARE_DYNAMIC(CBaseObject);
public:
	//! Events sent by object to listeners in EventCallback.
	enum EObjectListenerEvent
	{
		ON_DELETE = 0,//!< Sent after object was deleted from object manager.
		ON_SELECT,		//!< Sent when objects becomes selected.
		ON_UNSELECT,	//!< Sent when objects unselected.
		ON_TRANSFORM, //!< Sent when object transformed.
		ON_VISIBILITY, //!< Sent when object visibility changes.
	};

	//! This callback will be called if object is deleted.
	typedef Functor2<CBaseObject*,int> EventCallback;

	//! Childs structure.
	typedef std::vector<TSmartPtr<CBaseObject> > Childs;

	//! Retrieve class description of this object.
	CObjectClassDesc*	GetClassDesc() const { return m_classDesc; };

	/** Check if both object are of same class.
	*/
	virtual bool IsSameClass( CBaseObject *obj );

	ObjectType GetType() const { return m_classDesc->GetObjectType(); };
	const char* GetTypeName() const { return m_classDesc->ClassName(); };
	virtual CString GetTypeDescription() const { return m_classDesc->ClassName(); };

	//////////////////////////////////////////////////////////////////////////
	// Layer support.
	//////////////////////////////////////////////////////////////////////////
	void SetLayer( CObjectLayer *layer );
	CObjectLayer* GetLayer() const { return m_layer; };

	//////////////////////////////////////////////////////////////////////////
	// Flags.
	//////////////////////////////////////////////////////////////////////////
	void SetFlags( int flags ) { m_flags |= flags; };
	void ClearFlags( int flags ) { m_flags &= ~flags; };
	bool CheckFlags( int flags ) const { return (m_flags&flags) != 0; };

	//! Returns true if object hidden.
	bool IsHidden() const;
	//! Returns true if object frozen.
	bool IsFrozen() const;
	//! Returns true if object is selected.
	bool IsSelected() const { return CheckFlags(OBJFLAG_SELECTED); }
	//! Returns true if object is shared between missions.
	bool IsShared()	const { return CheckFlags(OBJFLAG_SHARED); }

	// Check if object potentially can be selected.
	bool IsSelectable() const;

	//! Set shared between missions flag.
	virtual void SetShared( bool bShared );
	//! Set object hidden status.
	virtual void SetHidden( bool bHidden );
	//! Set object frozen status.
	virtual void SetFrozen( bool bFrozen );
	//! Set object selected status.
	virtual void SetSelected( bool bSelect );

	//! Set object highlighted (Note: not selected)
	void SetHighlight( bool bHighlight );
	//! Check if object is highlighted.
	bool IsHighlighted() const { return CheckFlags(OBJFLAG_HIGHLIGHT); }

	//////////////////////////////////////////////////////////////////////////
	// Object Id.
	//////////////////////////////////////////////////////////////////////////
	//! Get uniq object id.
	//! Every object will have it own uniq id assigned.
	REFGUID GetId() const { return m_guid; };

	//////////////////////////////////////////////////////////////////////////
	// Name.
	//////////////////////////////////////////////////////////////////////////
	//! Get name of object.
	const CString& GetName() const;

	//! Change name of object.
	virtual void SetName( const CString &name );
	//! Set object name and make sure it is uniq.
	void SetUniqName( const CString &name );	

	//////////////////////////////////////////////////////////////////////////
	// Geometry.
	//////////////////////////////////////////////////////////////////////////
	//! Set object position.
	virtual void SetPos( const Vec3d &pos );
	//! Set object rotation angles.
	virtual void SetAngles( const Vec3d &angles );
	//! Set object scale.
	virtual void SetScale( const Vec3d &scale );

	//! Get object position.
	Vec3d GetPos() const { return m_pos; }
	//! Get object rotation angles.
	Vec3d GetAngles() const { return m_angles; }
	//! Get object scale.
	Vec3d GetScale() const { return m_scale; }

	//! Set flatten area.
	void SetArea( float area );
	float GetArea() const { return m_flattenArea; };

	//! Assign display color to the object.
	virtual void	SetColor( COLORREF color );
	//! Get object color.
	COLORREF GetColor() const { return m_color; };

	//////////////////////////////////////////////////////////////////////////
	// CHILDS
	//////////////////////////////////////////////////////////////////////////

	//! Return true if node have childs.
	bool HaveChilds() const { return !m_childs.empty(); };
	//! Return true if have attached childs.
	int GetChildCount() const { return m_childs.size(); };

	//! Get child by index.
	CBaseObject* GetChild( int i ) const;
	//! Return parent node if exist.
	CBaseObject* GetParent() const { return m_parent; };
	//! Scans hiearachy up to determine if we child of specified node.
	virtual bool IsChildOf( CBaseObject* node );
	
	//! Attach new child node.
	//! @param bKeepPos if true Child node will keep its world space position.
	virtual void AttachChild( CBaseObject* child,bool bKeepPos=true );
	//! Detach all childs of this node.
	virtual void DetachAll( bool bKeepPos=true );
	// Detach this node from parent.
	virtual void DetachThis( bool bKeepPos=true );

	//////////////////////////////////////////////////////////////////////////
	// MATRIX
	//////////////////////////////////////////////////////////////////////////
	//! Get objects's local transformation marix.
	const Matrix44& GetLocalTM() const;

	//! Get objects's world-space transformation matrix.
	const Matrix44& GetWorldTM() const;

	//! Get position in world space.
	Vec3 GetWorldPos() const { return GetWorldTM().GetTranslationOLD(); };
	Vec3 GetWorldAngles() const;

	//! Set xform of object givven in world space.
	void SetWorldTM( const Matrix44 &tm );

	//! Set object xform.
	void SetLocalTM( const Matrix44 &tm );

	//////////////////////////////////////////////////////////////////////////
	// Interface to be implemented in plugins.
	//////////////////////////////////////////////////////////////////////////

	//! Called when object is being created.
	virtual	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );
	
	//! Draw object to specified viewport.
	virtual	void Display( DisplayContext &disp ) = 0;
	
	//! Perform intersection testing of this object.
	//! Return true if was hit.
	virtual bool HitTest( HitContext &hc ) { return false; };

	//! Perform intersection testing of this object with rectangle.
	//! Return true if was hit.
	virtual bool HitTestRect( HitContext &hc );

	//! Called when user starts editing object parameters.
	//! Flags is comnination of ObjectEditFlags flags.
	//! Prev is a previous created base object.
	virtual void BeginEditParams( IEditor *ie,int flags );
	//! Called when user ends editing object parameters.
	virtual void EndEditParams( IEditor *ie );

	//! Called when user starts editing of multiple selected objects.
	//! @param bAllOfSameType true if all objects in selection are of same type.
	virtual void BeginEditMultiSelParams( bool bAllOfSameType );
	//! Called when user ends editing of multiple selected objects.
	virtual void EndEditMultiSelParams();

	//! Get bounding box of object in world coordinate space.
	virtual void GetBoundBox( BBox &box );

	//! Get bounding box of object in local object space.
	virtual void GetLocalBounds( BBox &box );
	
	//! Called after some parameter been modified.
	virtual void SetModified();

	//! Called when UI of objects is updated.
	virtual void OnUIUpdate();

	//! Called when visibility of this object changes.
	//! Derived classs may ovverride this to respond to new visibility setting.
	virtual void UpdateVisibility( bool visible );

	//! Serialize object to/from xml.
	//! @param xmlNode XML node to load/save serialized data to.
	//! @param bLoading true if loading data from xml.
	//! @param bUndo true if loading or saving data for Undo/Redo purposes.
	virtual void Serialize( CObjectArchive &ar );

	//! Export object to xml.
	//! Return created object node in xml.
	virtual XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );

	//! Handle events recieved by object.
	//! Override in derived classes, to handle specific events.
	virtual void OnEvent( ObjectEvent event );

	//////////////////////////////////////////////////////////////////////////
	// Group support.
	//////////////////////////////////////////////////////////////////////////
	//! Set group this object belong to.
	void	SetGroup( CGroup *group ) { m_group = group; };
	//! Get group this object belnog to.
	CGroup*	GetGroup() const { return m_group; };
	//! Check to see if this object part of any group.
	bool IsInGroup() const { return m_group != NULL; }

	//////////////////////////////////////////////////////////////////////////
	// LookAt Target.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetLookAt( CBaseObject *target );
	CBaseObject* GetLookAt() const { return m_lookat; };
	//! Returns true if this object is a look-at target.
	bool IsLookAtTarget() const;

	//! Perform hit testing of Axis.
	//! @return 0 if no axis hit, return 1-X Axis, 2=Y Axis, 3=Z Axis.
	virtual int HitTestAxis( HitContext &hc );

	//! Get Animation node assigned to this object.
	virtual struct IAnimNode* GetAnimNode() const { return 0; };

	//! Gets physics collision entity of this object.
	virtual class IPhysicalEntity* GetCollisionEntity() const { return 0; };
	
	//////////////////////////////////////////////////////////////////////////
	//! Return IEditor interface.
	IEditor* GetIEditor() const { return m_ie; }

	IObjectManager*	GetObjectManager() const { return m_objectManager; };

	//! Store undo information for this object.
	void StoreUndo( const char *UndoDescription,bool minimal=false );

	//! Add event listener callback.
	void AddEventListener( const EventCallback &cb );
	//! Remove event listener callback.
	void RemoveEventListener( const EventCallback &cb );

	//////////////////////////////////////////////////////////////////////////
	//! Material handlaing for this base object.
	//! Override in derived classes.
	//////////////////////////////////////////////////////////////////////////
	//! Assign new material to this object.
	virtual void SetMaterial( CMaterial *mtl ) {};
	//! Get assigned material for this object.
	virtual CMaterial* GetMaterial() const { return 0; };

	//////////////////////////////////////////////////////////////////////////
	//! Analyze errors for this object.
	virtual void Validate( CErrorReport *report );

	//////////////////////////////////////////////////////////////////////////
	//! Gather resources of this object.
	virtual void GatherUsedResources( CUsedResources &resources );

	//////////////////////////////////////////////////////////////////////////
	//! Check if specified object is very similar to this one.
	virtual bool IsSimilarObject( CBaseObject *pObject );

protected:
	friend CObjectManager;
	friend CObjectLayer;

	//! Ctor is protected to restrict direct usage.
	CBaseObject();
	//! Dtor is protected to restrict direct usage.
	virtual ~CBaseObject();

	//! Initialize Object.
	//! If previous object specified it must be of exactly same class as this object.
	//! All data is copied from previous object.
	//! Optional file parameter specify initial object or script for this object.
	virtual bool Init( IEditor *ie,CBaseObject *prev,const CString &file );

	//////////////////////////////////////////////////////////////////////////
	//! Must be called after cloning the object on clone of object.
	//! This will make sure object references are cloned correctly.
	virtual void PostClone( CBaseObject *pFromObject,CObjectCloneContext &ctx );

	//! Must be implemented by derived class to create game related objects.
	virtual bool CreateGameObject() { return true; };

	/** Called when object is about to be deleted.
			All Game resources should be freed in this function.
	*/
	virtual void Done();

	/** Change current id of object.
	*/
	//virtual void SetId( uint objectId ) { m_id = objectId; };
	
	//! Call this to delete an object.
	virtual void DeleteThis() = 0;

	//! Called when object need to be converted from different object.
	virtual bool ConvertFromObject( CBaseObject *object );

	//! Invalidates cached transformation matrix.
	virtual void InvalidateTM();

	//! Called when local transformation matrix is caluclated.
	void CalcLocalTM( Matrix44 &tm ) const;

	//! Update editable object UI params.
	void UpdateEditParams();

	//! Called when child position changed.
	virtual void OnChildModified() {};

	//! Called when GUID of object is changed for any reason.
	virtual void OnChangeGUID( REFGUID newGUID );

	//! Remove child from our childs list.
	virtual void RemoveChild( CBaseObject *node );
	
	//! Resolve parent from callback.
	void ResolveParent( CBaseObject *object );

	//! Draw default object items.
	void	DrawDefault( DisplayContext &dc,COLORREF labelColor=RGB(255,255,255) );
	//! Draw object label.
	void	DrawLabel( DisplayContext &dc,const Vec3 &pos,COLORREF labelColor=RGB(255,255,255) );
	//! Draw 3D Axis at object position.
	void	DrawAxis( DisplayContext &dc,const Vec3 &pos,float size );
	//! Draw area arround object.
	void	DrawArea( DisplayContext &dc );

	//! Draw highlight.
	virtual void DrawHighlight( DisplayContext &dc );

	CBaseObject* FindObject( REFGUID id ) const;

	// Returns true if game objects should be created.
	bool IsCreateGameObjects() const;

	// Helper gizmo funcitons.
	void AddGizmo( CGizmo *gizmo );
	void RemoveGizmo( CGizmo *gizmo );

	//! Notify all listeners about event.
	void NotifyListeners( EObjectListenerEvent event );

	//! Only used by ObjectManager.
	bool IsPotentiallyVisible() const;

	//////////////////////////////////////////////////////////////////////////
	// May be overrided in derived classes to handle helpers scaling.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetHelperScale( float scale ) {};
	virtual float GetHelperScale() { return 1; };

private:
	friend class CUndoBaseObject;
	friend class CPropertiesPanel;
	friend class CObjectArchive;
	friend class CSelectionGroup;
	
	//! Modify object properties, from modified property template.
	//! @templ Template to take parameters from.
	//! @modifiedNode XML Node that was modified in properties.
	void ModifyProperties( XmlNodeRef &templ,const XmlNodeRef &modifiedNode );

	//! Set class description for this object,
	//! Only called once after creation by ObjectManager.
	void SetClassDesc( CObjectClassDesc *classDesc );
	
	//! Set pointer to main editor Interface.
	//! Only called once after creation by ObjectManager.
	void SetIEditor( IEditor *ie ) { m_ie = ie; }

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE FIELDS
	//////////////////////////////////////////////////////////////////////////
private:
	//! ID Assigned to this object.
	//uint m_id;
	//! Unique object Id.
	GUID m_guid;

	//! Layer this object is assigned to.
	CObjectLayer *m_layer;

	//! Flags of this object.
	int m_flags;

	//! World space object's position.
	Vec3d m_pos;
	//! Object's Rotation angles.
	Vec3d m_angles;
	//! Object's scale value.
	Vec3d m_scale;
	
	//! Display color.
	COLORREF m_color;

	//! World transformation matrix of this object.
	mutable Matrix44 m_worldTM;
	mutable bool m_bMatrixInWorldSpace;
	mutable bool m_bMatrixValid;

	//////////////////////////////////////////////////////////////////////////
	//! Look At target entity.
	TSmartPtr<CBaseObject>	m_lookat;
	//! If we are lookat target. this is pointer to source.
	CBaseObject* m_lookatSource;

	//////////////////////////////////////////////////////////////////////////
	//! Area radius arround object, where terrain is flatten and static objects removed.
	float m_flattenArea;
	//! Every object keeps for itself height above terrain.
	float m_height;
	//! Object's name.
	CString m_name;
	//! Class description for this object.
	CObjectClassDesc*	m_classDesc;
	//! If object belongs to group, this will point to parent group.
	CGroup*	m_group;
	//! Pointer to main editor Interface.
	IEditor* m_ie;
	//! Pointer to object manager.
	IObjectManager *m_objectManager;
	//! Number of reference to this object.
	//! When reference count reach zero, object will delete itself.
	int m_numRefs;
	//////////////////////////////////////////////////////////////////////////
	//! Child animation nodes.
	Childs m_childs;
	//! Pointer to parent node.
	mutable CBaseObject *m_parent;

	//////////////////////////////////////////////////////////////////////////
	// Listeners.
	std::list<EventCallback> m_eventListeners;
	std::list<EventCallback>::iterator m_nextListener;
};

//////////////////////////////////////////////////////////////////////////
typedef TSmartPtr<CBaseObject> CBaseObjectPtr;

#endif // __BaseObject_h__
