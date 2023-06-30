////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   VisAreaShapeObject.cpp
//  Version:     v1.00
//  Created:     10/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "VisAreaShapeObject.h"

#include <I3DEngine.h>
#include <ISound.h> // to RecomputeSoundOcclusion() when deleting a vis area

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CVisAreaShapeObject,CShapeObject)
IMPLEMENT_DYNCREATE(COccluderShapeObject,CVisAreaShapeObject)
IMPLEMENT_DYNCREATE(CPortalShapeObject,CVisAreaShapeObject)

//////////////////////////////////////////////////////////////////////////
CVisAreaShapeObject::CVisAreaShapeObject()
{
	m_area = 0;
	mv_height = 5;
	m_bDisplayFilledWhenSelected = true;

	mv_vAmbientColor = Vec3d(0.25f,0.25f,0.25f);
	mv_vDynAmbientColor = Vec3d(0,0,0);
	mv_bAffectedBySun = false;
	mv_fViewDistRatio = 100.f;
	mv_bSkyOnly = false;

	AddVariable( mv_vAmbientColor,"AmbientColor",functor(*this, &CVisAreaShapeObject::OnShapeChange), IVariable::DT_COLOR );
	AddVariable( mv_vDynAmbientColor,"DynAmbientColor",functor(*this, &CVisAreaShapeObject::OnShapeChange), IVariable::DT_COLOR );
	AddVariable( mv_bAffectedBySun,"AffectedBySun",functor(*this, &CVisAreaShapeObject::OnShapeChange) );
	AddVariable( mv_fViewDistRatio,"ViewDistRatio",functor(*this, &CVisAreaShapeObject::OnShapeChange) );
	AddVariable( mv_bSkyOnly,"SkyOnly",functor(*this, &CVisAreaShapeObject::OnShapeChange) );

	SetColor( RGB(255,128,0) );
}

//////////////////////////////////////////////////////////////////////////
bool CVisAreaShapeObject::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	bool res = CShapeObject::Init( ie,prev,file );

	return res;
}

//////////////////////////////////////////////////////////////////////////
void CVisAreaShapeObject::Done()
{
	if (m_area)
	{
		// reset the listener vis area in the unlucky case that we are deleting the
		// vis area where the listener is currently in
		GetIEditor()->GetSystem()->GetISoundSystem()->RecomputeSoundOcclusion(false,false,true);
		GetIEditor()->Get3DEngine()->DeleteVisArea(m_area);
		m_area = 0;
	}
	CShapeObject::Done();
}

bool CVisAreaShapeObject::CreateGameObject()
{
	if (!m_area)
	{
		m_area = GetIEditor()->Get3DEngine()->CreateVisArea();
		m_bAreaModified = true;
		UpdateGameArea(false);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CVisAreaShapeObject::UpdateGameArea( bool bRemove )
{
	if (bRemove)
		return;
	if (!m_bAreaModified)
		return;

	if (m_area)
	{
		std::vector<Vec3> points;
		if (GetPointCount() > 3)
		{
			const Matrix44 &wtm = GetWorldTM();
			points.resize(GetPointCount());
			for (int i = 0; i < GetPointCount(); i++)
			{
				points[i] = wtm.TransformPointOLD( GetPoint(i) );
			}

			Vec3d vAmbClr = mv_vAmbientColor;
			Vec3d vDynAmbClr = mv_vDynAmbientColor;
			GetIEditor()->Get3DEngine()->UpdateVisArea( m_area, &points[0],points.size(), GetName(), GetHeight(), vAmbClr, mv_bAffectedBySun, mv_bSkyOnly, vDynAmbClr, mv_fViewDistRatio, true, false, false );
		}
	}
	m_bAreaModified = false;
}

//////////////////////////////////////////////////////////////////////////
// CPortalShapeObject
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
CPortalShapeObject::CPortalShapeObject()
{
	m_bDisplayFilledWhenSelected = true;
	SetColor( RGB(100,250,60) );

	mv_bUseDeepness = false;
	mv_bDoubleSide = true;
	AddVariable( mv_bUseDeepness,"UseDeepness",functor(*this,&CPortalShapeObject::OnShapeChange) );
	AddVariable( mv_bDoubleSide,"DoubleSide",functor(*this,&CPortalShapeObject::OnShapeChange) );
}

//////////////////////////////////////////////////////////////////////////
void CPortalShapeObject::UpdateGameArea( bool bRemove )
{
	if (bRemove)
		return;
	
	if (!m_bAreaModified)
		return;

	if (m_area)
	{
		std::vector<Vec3> points;
		if (GetPointCount() > 3)
		{
			const Matrix44 &wtm = GetWorldTM();
			points.resize(GetPointCount());
			for (int i = 0; i < GetPointCount(); i++)
			{
				points[i] = wtm.TransformPointOLD( GetPoint(i) );
			}

			CString name = CString("Portal_") + GetName();

			Vec3d vAmbClr = mv_vAmbientColor;
			Vec3d vDynAmbClr = mv_vDynAmbientColor;
			GetIEditor()->Get3DEngine()->UpdateVisArea( m_area, &points[0],points.size(), name, GetHeight(), vAmbClr, mv_bAffectedBySun, mv_bSkyOnly, vDynAmbClr, mv_fViewDistRatio, mv_bDoubleSide, mv_bUseDeepness, false );
		}
	}
	m_bAreaModified = false;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
COccluderShapeObject::COccluderShapeObject()
{
	m_bDisplayFilledWhenSelected = true;
	SetColor( RGB(200,128,60) );

	mv_bUseInIndoors = false;
	mv_bDoubleSide = true;
	AddVariable( mv_bUseInIndoors,"UseInIndoors",functor(*this,&COccluderShapeObject::OnShapeChange) );
	AddVariable( mv_bDoubleSide,"DoubleSide",functor(*this,&COccluderShapeObject::OnShapeChange) );
}

//////////////////////////////////////////////////////////////////////////
void COccluderShapeObject::UpdateGameArea( bool bRemove )
{
	if (bRemove)
		return;
	if (!m_bAreaModified)
		return;

	if (m_area)
	{
		std::vector<Vec3> points;
		if (GetPointCount() > 1)
		{
			const Matrix44 &wtm = GetWorldTM();
			points.resize(GetPointCount());
			for (int i = 0; i < GetPointCount(); i++)
			{
				points[i] = wtm.TransformPointOLD( GetPoint(i) );
			}

			CString name = CString("OcclArea_") + GetName();

			Vec3d vAmbClr = mv_vAmbientColor;
			Vec3d vDynAmbClr = mv_vDynAmbientColor;
			GetIEditor()->Get3DEngine()->UpdateVisArea( m_area, &points[0],points.size(), name, GetHeight(), vAmbClr, mv_bAffectedBySun, mv_bSkyOnly, vDynAmbClr, mv_fViewDistRatio, mv_bDoubleSide, false, mv_bUseInIndoors );
		}
	}
	m_bAreaModified = false;
}