////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001.
// -------------------------------------------------------------------------
//  File name:   TagPoint.cpp
//  Version:     v1.00
//  Created:     10/10/2001 by Timur.
//  Compilers:   Visual C++ 6.0
//  Description: CVolume implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Volume.h"

#include "..\Viewport.h"

//////////////////////////////////////////////////////////////////////////
// CBase implementation.
//////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CVolume,CBaseObject)

//////////////////////////////////////////////////////////////////////////
CVolume::CVolume()
{
	m_box.min = Vec3(-1,-1,0);
	m_box.max = Vec3(1,1,2);

	// Initials.
	mv_width = 4;
	mv_length = 4;
	mv_height = 1;
	mv_viewDistance = 4;

	AddVariable( mv_width,"Width",functor(*this,&CVolume::OnSizeChange) );
	AddVariable( mv_length,"Length",functor(*this,&CVolume::OnSizeChange) );
	AddVariable( mv_height,"Height",functor(*this,&CVolume::OnSizeChange) );
	AddVariable( mv_viewDistance,"ViewDistance" );
	AddVariable( mv_shader,"Shader",0,IVariable::DT_SHADER );
	AddVariable( mv_fogColor,"Color",0,IVariable::DT_COLOR );
}

//////////////////////////////////////////////////////////////////////////
void CVolume::Done()
{
	CBaseObject::Done();
}

//////////////////////////////////////////////////////////////////////////
bool CVolume::Init( IEditor *ie,CBaseObject *prev,const CString &file )
{
	bool res = CBaseObject::Init( ie,prev,file );
	SetColor( RGB(0,0,255) );
	if (prev)
	{
		m_sphere = ((CVolume*)prev)->m_sphere;
		m_box = ((CVolume*)prev)->m_box;
	}
	return res;
}

//////////////////////////////////////////////////////////////////////////
void CVolume::GetBoundBox( BBox &box )
{
	box = m_box;
	box.Transform( GetWorldTM() );
}

//////////////////////////////////////////////////////////////////////////
void CVolume::GetLocalBounds( BBox &box )
{
	box = m_box;
}

//////////////////////////////////////////////////////////////////////////
void CVolume::SetAngles( const Vec3d &angles )
{
	// Ignore angles on Volume.
}
	
void CVolume::SetScale( const Vec3d &scale )
{
	// Ignore scale on Volume.
}

//////////////////////////////////////////////////////////////////////////
bool CVolume::HitTest( HitContext &hc )
{
	Vec3 p;
	BBox box;
	GetBoundBox( box );
	if (box.IsIntersectRay(hc.raySrc,hc.rayDir,p ))
	{
		hc.dist = Vec3(hc.raySrc - p).Length();
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CVolume::OnSizeChange( IVariable *var )
{
	Vec3 size( 0,0,0 );
	size.x = mv_width;
	size.y = mv_length;
	size.z = mv_height;

	m_box.min = -size/2;
	m_box.max = size/2;
	// Make volume position bottom of bounding box.
	m_box.min.z = 0;
	m_box.max.z = size.z;
}

//////////////////////////////////////////////////////////////////////////
void CVolume::Display( DisplayContext &dc )
{
	COLORREF wireColor,solidColor;
	float wireOffset = 0;
	float alpha = 0.3f;
	if (IsSelected())
	{
		wireColor = dc.GetSelectedColor();
		solidColor = GetColor();
		wireOffset = -0.1f;
	}
	else
	{
		wireColor = GetColor();
		solidColor = GetColor();
	}
	
	dc.renderer->SetCullMode( R_CULL_DISABLE );
	int rstate = dc.ClearStateFlag( GS_DEPTHWRITE );

	BBox box;
	const Matrix44 &tm = GetWorldTM();
	box.min = tm.TransformPointOLD(m_box.min);
	box.max = tm.TransformPointOLD(m_box.max);
	
	
	bool bFrozen = IsFrozen();
	
	if (bFrozen)
		dc.SetFreezeColor();
	//////////////////////////////////////////////////////////////////////////
	if (!bFrozen)
		dc.SetColor( solidColor,alpha );
	//dc.DrawSolidBox( box.min,box.max );

	if (!bFrozen)
		dc.SetColor( wireColor,1 );
	dc.DrawWireBox( box.min,box.max,wireOffset );
	//////////////////////////////////////////////////////////////////////////

	dc.renderer->SetCullMode( R_CULL_BACK );
	dc.SetState( rstate  );


	DrawDefault(dc);
}

//////////////////////////////////////////////////////////////////////////
void CVolume::Serialize( CObjectArchive &ar )
{
	CBaseObject::Serialize( ar );
	XmlNodeRef xmlNode = ar.node;
	if (ar.bLoading)
	{
		xmlNode->getAttr( "BoxMin",m_box.min );
		xmlNode->getAttr( "BoxMax",m_box.max );
	}
	else
	{
		// Saving.
		xmlNode->setAttr( "BoxMin",m_box.min );
		xmlNode->setAttr( "BoxMax",m_box.max );
	}
}

//////////////////////////////////////////////////////////////////////////
XmlNodeRef CVolume::Export( const CString &levelPath,XmlNodeRef &xmlNode )
{
	XmlNodeRef objNode = CBaseObject::Export( levelPath,xmlNode );

	// Export position in world space.
	Matrix44 wtm = GetWorldTM();
	AffineParts affineParts;
	affineParts.SpectralDecompose(wtm);
	Vec3 worldPos = affineParts.pos;
	Vec3 worldAngles = RAD2DEG(Ang3::GetAnglesXYZ(Matrix33(affineParts.rot)));
		
	if (!IsEquivalent(worldPos,Vec3(0,0,0),0))
		objNode->setAttr( "Pos",worldPos );
		
	if (!IsEquivalent(worldAngles,Vec3(0,0,0),0))
		objNode->setAttr( "Angles",worldAngles );

	objNode->setAttr( "BoxMin",wtm.TransformPointOLD(m_box.min) );
	objNode->setAttr( "BoxMax",wtm.TransformPointOLD(m_box.max) );
	return objNode;

	/*
	CString volumeName,volumeParam;

	BBox box;
	GetBoundBox( box );

	volumeName = GetClassDesc()->GetTypeName() + "_" + GetName();
	volumeParam.Format( "Box{ %.2f,%.2f,%.2f,%.2f,%.2f,%.2f } ",box.min.x,box.min.y,box.min.z,box.max.x,box.max.y,box.max.z );

	if (GetParams())
	{
		CString str;
		XmlAttributes attributes = GetParams()->getAttributes();
		for (XmlAttributes::iterator it = attributes.begin(); it != attributes.end(); ++it)
		{
			str.Format( "%s{ %s } ",(const char*)it->key,(const char*)it->value );
			volumeParam += str;
		}
	}
	WritePrivateProfileString( "",volumeName,volumeParam,levelPath+"Volumes.ini" );

	return CBaseObject::Export( levelPath,xmlNode );
	*/
}