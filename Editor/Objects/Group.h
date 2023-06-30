////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Group.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: Group object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Group_h__
#define __Group_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"

/*!
 *	CGroup groups object together.
 *  Object can only be assigned to one group.
 *
 */
class CGroup : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CGroup)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	
	void Display( DisplayContext &disp );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );

	virtual void SetPos( const Vec3d &pos );
	virtual void SetAngles( const Vec3d &angles );
	virtual void SetScale( const Vec3d &scale );

	//! Attach new child node.
	void AttachChild( CBaseObject* child,bool bKeepPos=true );

	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );
	bool HitTest( HitContext &hc );

	void Serialize( CObjectArchive &ar );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );

	//! Ovveride event handler from CBaseObject.
	void OnEvent( ObjectEvent event );
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Group interface
	//////////////////////////////////////////////////////////////////////////
	//! Select objects withing specified distance from givven position.
	//! Return number of selected objects.
	int SelectObjects( const BBox &box,bool bUnselect=false );

	//! Remove all childs from this group.
	void	Ungroup();
	
	//! Open group.
	//! Make childs accesseble from top user interface.
	void	Open();
	
	//! Close group.
	//! Make childs unacesseble from top user interface.
	//! In Closed state group only display objects but not allow to select them.
	void	Close();

	//! Return true if group is in Opened.
	bool	IsOpen() const { return m_opened; };

	//! Called by child object, when it changes.
	void	OnChildModified();

	void DeleteAllChilds();

	//! If of this group used for geometry merging.
	int GetGeomMergeId() const;

protected:
	//! Dtor must be protected.
	CGroup();
	void InvalidateBBox() { m_bBBoxValid = false; };

	bool HitTestChilds( HitContext &hc );
	void SerializeChilds( CObjectArchive &ar );
	virtual void CalcBoundBox();

	static void RecursivelySetGroup( CBaseObject *object,CGroup *pGroup );
	static void RecursivelySetFlags( CBaseObject *object,int flags );
	static void RecursivelySetLayer( CBaseObject *object,CObjectLayer *pLayer );
	//! Get combined bounding box of all childs in hierarchy.
	static void RecursivelyGetBoundBox( CBaseObject *object,BBox &box );

	// Ovveriden from CBaseObject.
	void RemoveChild( CBaseObject *node );
	void DeleteThis() { delete this; };
	void OnMergeStaticGeom( IVariable *pVar );

	BBox m_bbox;
	bool m_bBBoxValid;
	bool m_opened;
	bool m_bAlwaysDrawBox;
	bool m_ignoreChildModify;

	CVariable<bool> mv_mergeStaticGeom;

	// Geometry merging group assigned to this Group.
	int m_geomMergeId;

	static int s_groupGeomMergeId;

	IEditor *m_ie;
};

/*!
 * Class Description of Group.
 */
class CGroupClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {1ED5BF40-BECA-4ba1-8E90-0906FF862A75}
		static const GUID guid = { 0x1ed5bf40, 0xbeca, 0x4ba1, { 0x8e, 0x90, 0x9, 0x6, 0xff, 0x86, 0x2a, 0x75 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_GROUP; };
	const char* ClassName() { return "Group"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CGroup); };
};

#endif // __Group_h__