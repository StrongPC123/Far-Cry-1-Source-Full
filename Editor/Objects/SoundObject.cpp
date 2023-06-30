////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   SoundObject.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CSoundObject implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "SoundObject.h"
#include "..\SoundObjectPanel.h"

#include "..\Viewport.h"

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CSoundObject,CBaseObject)

int CSoundObject::m_rollupId = 0;
CSoundObjectPanel* CSoundObject::m_panel = 0;

//////////////////////////////////////////////////////////////////////////
CSoundObject::CSoundObject()
{
	//m_ITag = 0;
	
	m_innerRadius = 1;
	m_outerRadius = 10;
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::Done()
{
	//if (m_ITag)
	{
	//	GetIEditor()->GetGame()->RemoveSoundObject( m_ITag );
	}
	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CSoundObject::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	m_ie = ie;
	
	SetColor( RGB(255,255,0) );
	bool res = CBaseObject::Init( ie,prev,file );
	
	// Create Tag point in game.
	//m_ITag = GetIEditor()->GetGame()->CreateSoundObject( (const char*)GetName(),GetPos(),GetAngles() );

	return res;
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::SetName( const CString &name )
{
	CBaseObject::SetName( name );
	//if (m_ITag)
		//m_ITag->SetName( name );
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::SetPos( const Vec3d &pos )
{
	CBaseObject::SetPos( pos );
	//if (m_ITag)
		//m_ITag->SetPos( pos );
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::SetAngles( const Vec3d &angles )
{
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::SetScale( const Vec3d &scale )
{
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::GetBoundSphere( Vec3d &pos,float &radius )
{
	pos = GetPos();
	radius = m_outerRadius;
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::BeginEditParams( IEditor *ie,int flags )
{
	m_ie = ie;
	CBaseObject::BeginEditParams( ie,flags );
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::EndEditParams( IEditor *ie )
{
	CBaseObject::EndEditParams( ie );
}

/*
//////////////////////////////////////////////////////////////////////////
void CSoundObject::OnPropertyChange( const CString &property )
{
	CBaseObject::OnPropertyChange(property);

	if (!GetParams())
		return;

	GetParams()->getAttr( "InnerRadius",m_innerRadius );
	GetParams()->getAttr( "OuterRadius",m_outerRadius );
}
*/

//////////////////////////////////////////////////////////////////////////
int CSoundObject::MouseCreateCallback( CViewport *view,EMouseEvent event,CPoint &point,int flags )
{
	if (event == eMouseMove || event == eMouseLDown)
	{
		Vec3 pos;
		// Position 1 meter above ground when creating.
		if (GetIEditor()->GetAxisConstrains() != AXIS_TERRAIN)
		{
			pos = view->MapViewToCP(point);
		}
		else
		{
			// Snap to terrain.
			bool hitTerrain;
			pos = view->ViewToWorld( point,&hitTerrain );
			if (hitTerrain)
			{
				pos.z = GetIEditor()->GetTerrainElevation(pos.x,pos.y) + 1.0f;
			}
			pos = view->SnapToGrid(pos);
		}
		SetPos( pos );
		if (event == eMouseLDown)
			return MOUSECREATE_OK;
		return MOUSECREATE_CONTINUE;
	}
	return CBaseObject::MouseCreateCallback( view,event,point,flags );
}

//////////////////////////////////////////////////////////////////////////
bool CSoundObject::HitTest( HitContext &hc )
{
	Vec3 origin = GetPos();
	float radius = 1;

	Vec3 w = origin - hc.raySrc;
	w = hc.rayDir.Cross( w );
	float d = w.Length();

	if (d < radius + hc.distanceTollerance)
	{
		hc.dist = GetDistance(hc.raySrc,origin);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CSoundObject::Display( DisplayContext &dc )
{
	COLORREF color = GetColor();
	
	dc.SetColor( color,0.8f );
	dc.DrawBall( GetPos(),1 );

	dc.SetColor( 0,1,0,0.3f );
	dc.DrawWireSphere( GetPos(),m_innerRadius );

	dc.SetColor( color,1 );
	dc.DrawWireSphere( GetPos(),m_outerRadius );
	//dc.renderer->DrawBall( GetPos(),m_outerRadius );

	if (IsSelected())
	{
		dc.SetSelectedColor( 0.5f );
		dc.DrawBall( GetPos(),1.3f );
	}
	DrawDefault( dc );
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CSoundObject::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	XmlNodeRef objNode = CBaseObject::Export( levelPath,xmlNode );
	return objNode;
}
