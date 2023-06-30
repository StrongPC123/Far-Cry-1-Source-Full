////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   zviewport.h
//  Version:     v1.00
//  Created:     9/1/2003 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __zviewport_h__
#define __zviewport_h__
#pragma once

#include "2DViewport.h"

// forward declarations.
class CBaseObject;

// Predeclare because of friend declaration
class CZViewport : public C2DViewport
{
	DECLARE_DYNCREATE(CZViewport)
public:
	CZViewport();
	virtual ~CZViewport();

	//////////////////////////////////////////////////////////////////////////
	// Override of Viewport interface.
	//////////////////////////////////////////////////////////////////////////
	virtual EViewportType GetType() { return ET_ViewportZ; }
	virtual void SetType( EViewportType type );

	CPoint	WorldToView( Vec3d wp );
	Vec3d		ViewToWorld( CPoint vp,bool *collideWithTerrain=0,bool onlyTerrain=false );
	void		ViewToWorldRay( CPoint vp,Vec3 &raySrc,Vec3 &rayDir );

	virtual void SetScrollOffset( float x,float y,bool bLimits=true );
	virtual void GetScrollOffset( float &x,float &y );

	virtual void SetZoomFactor(float fZoomFactor);
	virtual float GetZoomFactor() const;
	
	virtual Vec3 GetCPVector( const Vec3 &p1,const Vec3 &p2 );
	virtual Vec3 MapViewToCP( CPoint point );
	virtual bool HitTest( CPoint point,ObjectHitInfo &hitInfo,int flags=0 );

	//! Get prefered original size for this viewport.
	//! if 0, then no preference.
	virtual CSize GetIdealSize() const;

protected:
	virtual void CalculateViewTM();
	// Draw everything.
	virtual void Draw( DisplayContext &dc );
	void DrawSelectedObjects( DisplayContext &dc );
	void DrawObject( DisplayContext &dc,CBaseObject *obj );
	void DrawViewer( DisplayContext &dc );
	
	//{{AFX_MSG(CZViewport)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	Vec3 m_origin;
	float m_zoom;
};

#endif // __zviewport_h__
