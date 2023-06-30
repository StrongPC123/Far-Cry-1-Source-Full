////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Entity.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: StaticObject object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Entity_h__
#define __Entity_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"
#include "EntityScript.h"
#include "TrackGizmo.h"

#include "IMovieSystem.h"
#include "EntityPrototype.h"

#include "Material\Material.h"


/*!
 *	CEntityEventTarget is an Entity event target and type.
 */
struct CEntityEventTarget
{
	CBaseObject* target; //! Target object.
	_smart_ptr<CGizmo> pLineGizmo;
	CString event;
	CString sourceEvent;
};

/*!
 *	CEntity is an static object on terrain.
 *
 */
class CRYEDIT_API CEntity : public CBaseObject,public IAnimNodeCallback
{
public:
	DECLARE_DYNCREATE(CEntity)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	//! Return type name of Entity.
	CString GetTypeDescription() const { return GetEntityClass(); };
	
	//////////////////////////////////////////////////////////////////////////
	bool IsSameClass( CBaseObject *obj );

	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	bool CreateGameObject();
	void Display( DisplayContext &disp );

	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	void SetName( const CString &name );
	void SetPos( const Vec3d &pos );
	void SetAngles( const Vec3d &angles );
	void SetScale( const Vec3d &scale );

	void SetSelected( bool bSelect );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );
	void BeginEditMultiSelParams( bool bAllOfSameType );
	void EndEditMultiSelParams();

	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );

	bool HitTest( HitContext &hc );
	void UpdateVisibility( bool visible );
	bool ConvertFromObject( CBaseObject *object );

	void Serialize( CObjectArchive &ar );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );

	void SetLookAt( CBaseObject *target );

	//////////////////////////////////////////////////////////////////////////
	void OnEvent( ObjectEvent event );
	
	//////////////////////////////////////////////////////////////////////////
	virtual IAnimNode* GetAnimNode() const { return m_animNode; };
	virtual IPhysicalEntity* GetCollisionEntity() const;

	//! Return entity prototype class if present.
	virtual CEntityPrototype* GetPrototype() const { return m_prototype; };

	//! Attach new child node.
	virtual void AttachChild( CBaseObject* child,bool bKeepPos=true );
	//! Detach all childs of this node.
	virtual void DetachAll( bool bKeepPos=true );
	// Detach this node from parent.
	virtual void DetachThis( bool bKeepPos=true );

	virtual void SetHelperScale( float scale );
	virtual float GetHelperScale() { return m_helperScale; };

	//////////////////////////////////////////////////////////////////////////
	// CEntity interface.
	//////////////////////////////////////////////////////////////////////////
	virtual bool LoadScript( const CString &entityClass,bool bForceReload=false,bool bGetScriptProperties=true,XmlNodeRef xmlProperties=XmlNodeRef(),XmlNodeRef xmlProperties2=XmlNodeRef() );
	virtual void SpawnEntity();
	virtual void DeleteEntity();
	virtual void UnloadScript();
	
	CString GetEntityClass() const { return m_entityClass; };
	int GetEntityId() const { return m_entityId; };

	//! Get EntityScript object associated with this entity.
	CEntityScript* GetScript() { return m_script; }
	//! Reload entity script.
	void Reload( bool bReloadScript=false );

	//! Return number of event targets of Script.
	int		GetEventTargetCount() const { return m_eventTargets.size(); };
	CEntityEventTarget& GetEventTarget( int index ) { return m_eventTargets[index]; };
	//! Add new event target, returns index of created event target.
	//! Event targets are Always entities.
	int AddEventTarget( CBaseObject *target,const CString &event,const CString &sourceEvent,bool bUpdateScript=true );
	//! Remove existing event target by index.
	void RemoveEventTarget( int index,bool bUpdateScript=true );

	//! Get Game entity interface.
	IEntity*	GetIEntity() { return m_entity; };

	bool IsCastShadow() const { return mv_castShadows; }
	bool IsSelfShadowing() const { return mv_selfShadowing; }
  bool IsCastShadowMaps() const { return mv_castShadowMaps; }
  bool IsRecvShadowMaps() const { return mv_recvShadowMaps; }
	bool IsCastLightmap() const { return mv_castLightmap; }
	bool IsRecvLightmap() const { return mv_recvLightmap; }
	bool IsHideable() const { return mv_recvLightmap; }
	float GetRatioLod() const { return mv_ratioLOD; };
	float GetRatioViewDist() const { return mv_ratioViewDist; }

	CVarBlock* GetProperties() const { return m_properties; };
	CVarBlock* GetProperties2() const { return m_properties2; };
//	unsigned MemStats();

	//////////////////////////////////////////////////////////////////////////
	// Override materials.
	//////////////////////////////////////////////////////////////////////////
	virtual void SetMaterial( CMaterial *mtl );
	virtual CMaterial* GetMaterial() const { return m_pMaterial; };

	void Validate( CErrorReport *report );

	//! Takes position/orientation from the game entity and apply on editor entity.
	void AcceptPhysicsState();
	void ResetPhysicsState();

	//////////////////////////////////////////////////////////////////////////
	virtual void GatherUsedResources( CUsedResources &resources );
	virtual bool IsSimilarObject( CBaseObject *pObject );

protected:
	bool HitTestEntity( HitContext &hc,bool &bHavePhysics );

	//////////////////////////////////////////////////////////////////////////
	//! Must be called after cloning the object on clone of object.
	//! This will make sure object references are cloned correctly.
	virtual void PostClone( CBaseObject *pFromObject,CObjectCloneContext &ctx );

	// Called to create AnimNode for this object.
	virtual IAnimNode* CreateAnimNode();
	// overrided from IAnimNodeCallback
	virtual void OnNodeAnimated();

	void OnLoadFailed();

	CVarBlock* CloneProperties( CVarBlock *srcProperties );
	void UpdatePropertyPanel();

	//////////////////////////////////////////////////////////////////////////
	//! Callback called when one of entity properties have been modified.
	void OnPropertyChange( IVariable *var );

	//////////////////////////////////////////////////////////////////////////
	void OnEventTargetEvent( CBaseObject *target,int event );
	void ResolveEventTarget( CBaseObject *object,unsigned int index );
	void ReleaseEventTargets();
	void UpdateMaterialInfo();

	//! Dtor must be protected.
	CEntity();
	void DeleteThis() { delete this; };

	//overrided from CBaseObject.
	void InvalidateTM();

	//! Draw target lines.
	void DrawTargets( DisplayContext &dc );

	//! Recalculate bounding box of entity.
	void CalcBBox();

	//! Force IEntity to the local position/angles/scale.
	void XFormGameEntity();

	//! Sets correct binding for IEntity.
	void BindToParent();
	void BindIEntityChilds();
	void UnbindIEntity();

	void DrawAIInfo( DisplayContext &dc,struct IAIObject *aiObj );

	//////////////////////////////////////////////////////////////////////////
	// Callbacks.
	//////////////////////////////////////////////////////////////////////////
	void OnRenderFlagsChange( IVariable *var );

	//////////////////////////////////////////////////////////////////////////
	// Radius callbacks.
	//////////////////////////////////////////////////////////////////////////
	void OnRadiusChange( IVariable *var );
	void OnInnerRadiusChange( IVariable *var );
	void OnOuterRadiusChange( IVariable *var );
	//////////////////////////////////////////////////////////////////////////

	//! Entity class.
	CString m_entityClass;
	//! Id of spawned entity.
	int m_entityId;

	IEntity* m_entity;
	IStatObj* m_visualObject;
	BBox m_box;

	bool m_loadFailed;
	bool m_bEntityXfromValid;
	bool m_bCalcPhysics;

	// Cached quaternion.
	Quat m_rotate;

  TSmartPtr<CEntityScript> m_script;

	bool m_displayBBox;
	bool m_visible;
	//! True if this is static entity.
	//bool m_staticEntity;

	//////////////////////////////////////////////////////////////////////////
	// Main entity parameters.
	//////////////////////////////////////////////////////////////////////////
	CVariable<bool> mv_castShadows;
	CVariable<bool> mv_selfShadowing;
	CVariable<bool> mv_castShadowMaps;
	CVariable<bool> mv_recvShadowMaps;
	CVariable<bool> mv_castLightmap;
	CVariable<bool> mv_recvLightmap;
	CVariable<int> mv_ratioLOD;
	CVariable<int> mv_ratioViewDist;
  CVariable<int> mv_UpdateVisLevel;
	CVariable<bool> mv_hiddenInGame; // Entity is hidden in game (on start).
	CVariable<bool> mv_notOnLowSpec;

	//////////////////////////////////////////////////////////////////////////
	// Temp variables (Not serializable) just to display radiuses from properties.
	//////////////////////////////////////////////////////////////////////////
	// Used for proximity entities.
	float m_proximityRadius;
	float m_innerRadius;
	float m_outerRadius;
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	// Event Targets.
	//////////////////////////////////////////////////////////////////////////
	//! Array of event targets of this Entity.
	typedef std::vector<CEntityEventTarget> EventTargets;
	EventTargets m_eventTargets;

	//! Animation node, assigned to this object.
	TSmartPtr<IAnimNode> m_animNode;

	//! Entity prototype. only used by EntityPrototypeObject.
	TSmartPtr<CEntityPrototype> m_prototype;

	//! Per instance properties table.
	CVarBlockPtr m_properties2;

	//! Entity Properties variables.
	CVarBlockPtr m_properties;

	// Can keep reference to one track gizmo.
	CTrackGizmoPtr m_trackGizmo;
	
	//! Material of this entity.
	TSmartPtr<CMaterial> m_pMaterial;
	GUID m_materialGUID;

	// Physics state, as a string.
	CString m_physicsState;


	static int m_rollupId;
	static class CEntityPanel* m_panel;
	static float m_helperScale;
};

/*!
 * Class Description of Entity
 */
class CEntityClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {C80F8AEA-90EF-471f-82C7-D14FA80B9203}
		static const GUID guid = { 0xc80f8aea, 0x90ef, 0x471f, { 0x82, 0xc7, 0xd1, 0x4f, 0xa8, 0xb, 0x92, 0x3 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_ENTITY; };
	const char* ClassName() { return "StdEntity"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CEntity); };
	const char* GetFileSpec() { return "*EntityClass"; };
	int GameCreationOrder() { return 200; };
};

//////////////////////////////////////////////////////////////////////////
// Simple entity.
//////////////////////////////////////////////////////////////////////////
class CSimpleEntity : public CEntity
{
public:
	DECLARE_DYNCREATE(CSimpleEntity)

	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	bool ConvertFromObject( CBaseObject *object );
	
	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );
	
	void Validate( CErrorReport *report );
	bool IsSimilarObject( CBaseObject *pObject );

private:
	CString GetGeometryFile() const;
	void SetGeometryFile( const CString &filename );
	void OnFileChange( CString filename );
};

/*!
 * Class Description of Entity
 */
class CSimpleEntityClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {F7820713-E4EE-44b9-867C-C0F9543B4871}
		static const GUID guid = { 0xf7820713, 0xe4ee, 0x44b9, { 0x86, 0x7c, 0xc0, 0xf9, 0x54, 0x3b, 0x48, 0x71 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_ENTITY; };
	const char* ClassName() { return "SimpleEntity"; };
	const char* Category() { return "Simple Entity"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CSimpleEntity); };
	const char* GetFileSpec() { return "Objects\\*.cgf;*.ccgf;*.cga"; };
	int GameCreationOrder() { return 201; };
};

#endif // __CEntity_h__