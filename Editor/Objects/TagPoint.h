////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TagPoint.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: TagPoint object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __TagPoint_h__
#define __TagPoint_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"

/*!
 *	CTagPoint is an object that represent named 3d position in world.
 *
 */
class CTagPoint : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CTagPoint)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();
	void Display( DisplayContext &disp );

	//////////////////////////////////////////////////////////////////////////
	virtual void SetName( const CString &name );
	virtual void SetScale( const Vec3d &scale );
	virtual void InvalidateTM();

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );

	//! Called when object is being created.
	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );
	bool HitTest( HitContext &hc );

	void GetLocalBounds( BBox &box );
	void GetBoundBox( BBox &box );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////

	virtual void SetHelperScale( float scale ) { m_helperScale = scale; };
	virtual float GetHelperScale() { return m_helperScale; };

protected:
	//! Dtor must be protected.
	CTagPoint();

	virtual void CreateITagPoint();
	float GetRadius();

	void DeleteThis() { delete this; };

	IEditor *m_ie;
	struct ITagPoint *m_ITag;
	struct IAIObject *m_aiTag;

	//! Static, common to all tag points.
	static float m_helperScale;
};

/** Respawn point is a special tag point where player will be respawn at begining or after death.
*/
class CRespawnPoint : public CTagPoint
{
public:
	DECLARE_DYNCREATE(CRespawnPoint)

	void Done();
	virtual void CreateITagPoint();
};

/*!
 * Class Description of TagPoint.	
 */
class CTagPointClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {7826D64A-080E-46cc-8C50-BA6A6CAE5175}
		static const GUID guid = { 0x7826d64a, 0x80e, 0x46cc, { 0x8c, 0x50, 0xba, 0x6a, 0x6c, 0xae, 0x51, 0x75 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_TAGPOINT; };
	const char* ClassName() { return "StdTagPoint"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CTagPoint); };
};

/*!
 * Class Description of TagPoint.	
 */
class CRespawnPointClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {03A22E8A-0AB8-41fe-8503-75687A8A50BC}
		static const GUID guid = { 0x3a22e8a, 0xab8, 0x41fe, { 0x85, 0x3, 0x75, 0x68, 0x7a, 0x8a, 0x50, 0xbc } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_TAGPOINT; };
	const char* ClassName() { return "Respawn"; };
	const char* Category() { return "TagPoint"; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CRespawnPoint); };
};

#endif // __TagPoint_h__