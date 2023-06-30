////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   tagcomment.h
//  Version:     v1.00
//  Created:     6/5/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: Special tag point for comment.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __tagcomment_h__
#define __tagcomment_h__
#pragma once

#include "BaseObject.h"

/*!
 *	CTagComment is an object that represent text commentary added to named 3d position in world.
 *
 */
class CTagComment : public CBaseObject
{
public:
	DECLARE_DYNCREATE(CTagComment)

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CBaseObject.
	//////////////////////////////////////////////////////////////////////////
	void Display( DisplayContext &disp );

	//////////////////////////////////////////////////////////////////////////
	virtual void SetScale( const Vec3d &scale );

	//! Called when object is being created.
	int MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags );
	bool HitTest( HitContext &hc );

	void GetBoundBox( BBox &box );
	void GetLocalBounds( BBox &box );

	XmlNodeRef Export( const CString &levelPath,XmlNodeRef &xmlNode );
	//////////////////////////////////////////////////////////////////////////

	virtual void SetHelperScale( float scale ) { m_helperScale = scale; };
	virtual float GetHelperScale() { return m_helperScale; };

protected:
	//! Dtor must be protected.
	CTagComment();
	float GetRadius();

	void DeleteThis() { delete this; };

	CVariable<CString> mv_comment;
	CVariable<bool> mv_fixed;
	
	static float m_helperScale;
};

/*!
 * Class Description of CTagComment.	
 */
class CTagCommentClassDesc : public CObjectClassDesc
{
public:
	REFGUID ClassID()
	{
		// {FAAA3955-EFE0-4888-85E8-C5481DC16FA5}
		static const GUID guid = { 0xfaaa3955, 0xefe0, 0x4888, { 0x85, 0xe8, 0xc5, 0x48, 0x1d, 0xc1, 0x6f, 0xa5 } };
		return guid;
	}
	ObjectType GetObjectType() { return OBJTYPE_TAGPOINT; };
	const char* ClassName() { return "StdTagComment"; };
	const char* Category() { return ""; };
	CRuntimeClass* GetRuntimeClass() { return RUNTIME_CLASS(CTagComment); };
};

#endif // __tagcomment_h__