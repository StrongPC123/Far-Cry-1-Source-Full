////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   grid.cpp
//  Version:     v1.00
//  Created:     8/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Grid.h"

//////////////////////////////////////////////////////////////////////////
CGrid::CGrid()
{
	scale = 1;
	size = 1;
	majorLine = 10;
	bEnabled = true;

	bAngleSnapEnabled = true;
	angleSnap = 5;
}

//////////////////////////////////////////////////////////////////////////
Vec3 CGrid::Snap( const Vec3 &vec ) const
{
	if (!bEnabled || size<0.001f)
		return vec;
	Vec3 snapped;
	snapped.x = floor((vec.x/size)/scale + 0.5f) * size * scale;
	snapped.y = floor((vec.y/size)/scale + 0.5f) * size * scale;
	snapped.z = floor((vec.z/size)/scale + 0.5f) * size * scale;
	return snapped;
}

//////////////////////////////////////////////////////////////////////////
float CGrid::SnapAngle( float angle ) const
{
	if (!bAngleSnapEnabled)
		return angle;
	return floor(angle/angleSnap + 0.5f) * angleSnap;
}

//////////////////////////////////////////////////////////////////////////
Vec3 CGrid::SnapAngle( const Vec3 &vec ) const
{
	if (!bAngleSnapEnabled)
		return vec;
	Vec3 snapped;
	snapped.x = floor(vec.x/angleSnap + 0.5f) * angleSnap;
	snapped.y = floor(vec.y/angleSnap + 0.5f) * angleSnap;
	snapped.z = floor(vec.z/angleSnap + 0.5f) * angleSnap;
	return snapped;
}

//////////////////////////////////////////////////////////////////////////
void CGrid::Serialize( XmlNodeRef &xmlNode,bool bLoading )
{
	if (bLoading)
	{
		// Loading.
		xmlNode->getAttr( "Size",size );
		xmlNode->getAttr( "Scale",scale );
		xmlNode->getAttr( "Enabled",bEnabled );
		xmlNode->getAttr( "MajorSize",majorLine );
		xmlNode->getAttr( "AngleSnap",angleSnap );
		xmlNode->getAttr( "AngleSnapEnabled",bAngleSnapEnabled );
	}
	else
	{
		// Saving.
		xmlNode->setAttr( "Size",size );
		xmlNode->setAttr( "Scale",scale );
		xmlNode->setAttr( "Enabled",bEnabled );
		xmlNode->setAttr( "MajorSize",majorLine );
		xmlNode->setAttr( "AngleSnap",angleSnap );
		xmlNode->setAttr( "AngleSnapEnabled",bAngleSnapEnabled );
	}
}