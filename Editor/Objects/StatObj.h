////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   StatObj.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: StaticObject object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __StatObj_h__
#define __StatObj_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"
#include "Entity.h"

/*!
 *	CStatObj is an static object on terrain.
 *
 */
class CStaticObject : public CEntity
{
public:
	DECLARE_DYNCREATE(CStaticObject)

	// Operators.
	void LoadObject( const CString &file,bool bForceReload=false );
	void ReloadObject();
	void UnloadObject();

	// Accessors.
	CString GetObjectName() { return m_objectName; };

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	void Display( DisplayContext &disp );

	void SetScale( const Vec3d &scale );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );
	void BeginEditMultiSelParams( bool bAllOfSameType );
	void EndEditMultiSelParams();

	//void GetBoundSphere( Vec3d &pos,float &radius );
	//void GetBoundBox( BBox &box );
	//bool HitTest( HitContext &hc );

	void Serialize( CObjectArchive &ar );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );

	void OnEvent( ObjectEvent event );
	//////////////////////////////////////////////////////////////////////////

	void SetRigidBody( bool enable );
	void SetMass( float mass );
	void SetHidable( bool enable );

	bool IsRigidBody() const { return mv_rigidBody; }
	bool IsHideable() const { return mv_hidable; }
	float GetMass() const { return mv_mass; }

protected:
	//! Dtor must be protected.
	CStaticObject();
	void DeleteThis() { delete this; };

	void OnRigidBodyChange( IVariable *var );
	void OnMassChange( IVariable *var );
	void OnAnimationChange( IVariable *var );

	CString m_objectName;

	CVariable<bool> mv_rigidBody;
	CVariable<bool> mv_hidable;
	CVariable<float> mv_mass;
	CVariable<float> mv_density;
	CVariable<CString> mv_animation;
	CVariable<bool> mv_animationLoop;
	CVariable<float> mv_animationSpeed;

	bool m_loadFailed;
	bool m_bCharacter;

	static int m_rollupId;
	static class CStatObjPanel* m_panel;
};

/*!
 * Class Description of StaticObject	
 */
class CStaticObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {BE4B917B-CA10-4f3e-8B93-F54450FE840E}
		static const GUID guid = { 0xbe4b917b, 0xca10, 0x4f3e, { 0x8b, 0x93, 0xf5, 0x44, 0x50, 0xfe, 0x84, 0xe } };
		return guid;
	}
	void Release() { delete this; }
	ObjectType GetObjectType() { return OBJTYPE_STATIC; };
	const char* ClassName() { return "StdStatic"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CStaticObject); };
	const char* GetFileSpec() { return "Objects\\*.cgf;*.ccgf;*.cga"; };
};

#endif // __CStaticObject_h__