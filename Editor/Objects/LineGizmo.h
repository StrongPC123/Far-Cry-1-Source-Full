////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   LineGizmo.h
//  Version:     v1.00
//  Created:     6/11/2003 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __LineGizmo_h__
#define __LineGizmo_h__
#pragma once

#include "BaseObject.h"
#include "Gizmo.h"

// forward declarations.
struct DisplayContext;

/** Gizmo of link line connecting two Objects.
*/
class CLineGizmo : public CGizmo
{
public:
	CLineGizmo();
	~CLineGizmo();

	//////////////////////////////////////////////////////////////////////////
	// Ovverides from CGizmo
	//////////////////////////////////////////////////////////////////////////
	virtual void GetWorldBounds( BBox &bbox );
	virtual void Display( DisplayContext &dc );
	virtual bool HitTest( HitContext &hc );

	//////////////////////////////////////////////////////////////////////////
	void SetObjects( CBaseObject *pObject1,CBaseObject *pObject2 );
	void SetColor( const Vec3 &color1,const Vec3 &color2,float alpha1=1.0f,float alpha2=1.0f );
	void SetName( const char *sName );

private:
	void OnObjectEvent( CBaseObject *object,int event );
	void CalcBounds();

	CBaseObjectPtr m_object[2];
	Vec3 m_point[2];
	BBox m_bbox;
	CFColor m_color[2];
	CString m_name;
};

#endif // __LineGizmo_h__

