////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Building.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: StaticObject object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Building_h__
#define __Building_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "Group.h"

/*!
 *	CBuilding is an building on the terrain.
 *
 */
class CBuilding : public CGroup
{
public:
	DECLARE_DYNCREATE(CBuilding)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	void Display( DisplayContext &disp );

	void SetPos( const Vec3d &pos );
	void SetAngles( const Vec3d &angles );
	void SetScale( const Vec3d &scale );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );

	void GetBoundBox( BBox &box );
	bool HitTest( HitContext &hc );
	void UpdateVisibility( bool visible );
	void InvalidateTM();

	void OnEvent( ObjectEvent event );
	void Serialize( CObjectArchive &ar );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////
	// Building interface.

	//! Load Building file and make indoor building representation.
	void LoadBuilding( const CString &object,bool bForceReload=false );
	//! Release Indoor building in engine.
	void ReleaseBuilding();

	//! Get object name by index.
	const CString& GetObjectName() const { return m_objectName; }
	int GetBuildingId() { return m_buildingId; };

	//! Return true if building in Indoor Engine is correctly initialized.
	//! And building id reference valid id.
	bool ValidBuildingId( int id ) { return id >= 0; };

	//! Returns number of sectors in the building.
	int GetNumSectors() const { return m_sectorHidden.size(); };
	//! Hides or shows specified sector.
	void HideSector( int index,bool bHide );
	// Returns true if specified sector is hidden.
	bool IsSectorHidden( int index ) const;

	//! Set wireframe rending mode.
	void SetWireframe( bool bEnable );
	//! Check if rendered in wireframe.
	bool IsWireframe() const;

	//////////////////////////////////////////////////////////////////////////
	// Building Helpers.
	//////////////////////////////////////////////////////////////////////////

	//! This structure is a single instance of helper in building.
	struct ObjectHelper
	{
		CString name;
		CString className;
		Vec3 pos;
		Vec3 angles;
		CBaseObject *object;
		ObjectHelper() { object = 0; };
	};
	std::vector<ObjectHelper>& GetHelpers() { return m_helpers; };
	void SpawnEntities();

	void BindToHelper( int helperIndex,CBaseObject *obj );
	void UnbindHelper( int helperIndex );
	void UnbindAllHelpers();

	void SetPortals( bool enable );
	bool IsPortals() const { return m_portals; };

protected:
	//! Dtor must be protected.
	CBuilding();

	void SerializeHelpers( CObjectArchive &ar );
	void ResolveHelper( CBaseObject *object,uint helperIndex );
	void OnHelperEvent( CBaseObject *object,int event );

	struct IIndoorBase* GetBuildMgr();
	
	// Ovverides from CBaseObject.
	void DeleteThis() { delete this; };

	//! Calculate bounding box of building.
	virtual void CalcBoundBox();

	IEditor* m_ie;
	CString m_objectName;

	int m_buildingId;
	int m_numSectors;

	//! When true building displayed in wireframe.
	bool m_wireFrame;
	//! When true building rendered with portals.
	static bool m_portals;

	std::vector<ObjectHelper> m_helpers;
	std::vector<bool> m_sectorHidden;

	static int m_rollupId;
	static class CBuildingPanel* m_panel;
};

/*!
 * Class Description of Building
 */
class CBuildingClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {5237F903-CF62-48de-979A-20C4E989D22D}
		static const GUID guid = { 0x5237f903, 0xcf62, 0x48de, { 0x97, 0x9a, 0x20, 0xc4, 0xe9, 0x89, 0xd2, 0x2d } };
		return guid;
	}
	void Release() { delete this; }
	ObjectType GetObjectType() { return OBJTYPE_BUILDING; };
	const char* ClassName() { return "StdBuilding"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CBuilding); };
	const char* GetFileSpec() { return "Objects\\Buildings\\*.bld"; };
};

#endif // __Building_h__