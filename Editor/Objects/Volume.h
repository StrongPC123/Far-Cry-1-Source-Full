////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   Volume.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: TagPoint object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __Volume_h__
#define __Volume_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"

/*!
 *	CVolume is an spherical or box volume in space.
 *
 */
class CVolume : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CVolume)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();

	void Display( DisplayContext &dc );

	void SetAngles( const Vec3d &angles );
	void SetScale( const Vec3d &scale );

	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );
	bool HitTest( HitContext &hc );

	void Serialize( CObjectArchive &ar );
	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////

protected:
	//! Dtor must be protected.
	CVolume();

	//! Called when one of size parameters change.
	void OnSizeChange( IVariable *var );

	void DeleteThis() { delete this; };

	//! Can be either sphere or box.
	bool m_sphere;

	CVariable<float> mv_width;
	CVariable<float> mv_length;
	CVariable<float> mv_height;
	CVariable<float> mv_viewDistance;
	CVariable<CString> mv_shader;
	CVariable<Vec3> mv_fogColor;

	//! Local volume space bounding box.
	BBox m_box;
};

/*!
 * Class Description of TagPoint.
 */
class CVolumeClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {3141824B-6455-417b-B9A0-76CF1A672369}
		static const GUID guid = { 0x3141824b, 0x6455, 0x417b, { 0xb9, 0xa0, 0x76, 0xcf, 0x1a, 0x67, 0x23, 0x69 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_VOLUME; };
	const char* ClassName() { return "StdVolume"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CVolume); };
	int GameCreationOrder() { return 15; };
};

#endif // __Volume_h__