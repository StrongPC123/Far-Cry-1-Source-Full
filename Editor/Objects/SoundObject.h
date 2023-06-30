////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   SoundObject.h
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: SoundObject object definition.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __SoundObject_h__
#define __SoundObject_h__

#if _MSC_VER > 1000
#pragma once
#endif

#include "BaseObject.h"

/*!
 *	CSoundObject is an object that represent named 3d position in world.
 *
 */
class CSoundObject : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CSoundObject)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	bool Init( IEditor *ie,CBaseObject *prev,const CString &file );
	void Done();

	void Display( DisplayContext &dc );

	//////////////////////////////////////////////////////////////////////////
	virtual void SetName( const CString &name );
	virtual void SetPos( const Vec3d &pos );
	virtual void SetAngles( const Vec3d &angles );
	virtual void SetScale( const Vec3d &angles );

	void BeginEditParams( IEditor *ie,int flags );
	void EndEditParams( IEditor *ie );

	//! Called when object is being created.
	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );

	void GetBoundSphere( Vec3d &pos,float &radius );
	bool HitTest( HitContext &hc );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////

protected:
	//! Dtor must be protected.
	CSoundObject();
	void DeleteThis() { delete this; };

	float m_innerRadius;
	float m_outerRadius;

	IEditor *m_ie;
	//ISoundObject *m_ITag;
	
	static int m_rollupId;
	static class CSoundObjectPanel* m_panel;
};

/*!
 * Class Description of SoundObject.	
 */
class CSoundObjectClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {6B3EDDE5-7BCF-4936-B891-39CD7A8DC021}
		static const GUID guid = { 0x6b3edde5, 0x7bcf, 0x4936, { 0xb8, 0x91, 0x39, 0xcd, 0x7a, 0x8d, 0xc0, 0x21 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_TAGPOINT; };
	const char* ClassName() { return "StdSoundObject"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CSoundObject); };
};

#endif // __SoundObject_h__