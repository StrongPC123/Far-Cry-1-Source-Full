////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   controllertcb.cpp
//  Version:     v1.00
//  Created:     12/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ControllerTCB.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns position of the controller at the given time
CryQuat CControllerTCB::GetOrientation (float t)
{
	return CryQuat(1,0,0,0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns position of the controller at the given time
Vec3d CControllerTCB::GetPosition (float t)
{
	return Vec3d(0,0,0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns scale of the controller at the given time
Vec3d CControllerTCB::GetScale (float t)
{
	return Vec3d(1,1,1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieves the position and orientation within one call
// may be optimal in some applications
void CControllerTCB::GetValue (float t, CryQuat& q, Vec3d &p)
{
	q = GetOrientation(t);
	p = GetPosition(t);
}

//////////////////////////////////////////////////////////////////////////
ILog* CControllerTCB::GetLog()const
{
	return g_GetLog();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// CControllerTCBVec3 spline implementation.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool CControllerTCBVec3::Load( const CONTROLLER_CHUNK_DESC_0826* pChunk,float secsPerTick )
{
	CryTCB3Key *pKeys = (CryTCB3Key*)(pChunk+1);

	int nkeys = pChunk->nKeys;
	m_spline.resize(nkeys);
	for (int i = 0; i < nkeys; i++)
	{
		CryTCB3Key &ck = pKeys[i];
		TCBSpline<Vec3d>::key_type &key = m_spline.key(i);
		key.flags = 0;
		key.time = ck.time * secsPerTick;
		key.value = ck.val;
		key.tens = ck.t;
		key.cont = ck.c;
		key.bias = ck.b;
		key.easefrom = ck.eout;
		key.easeto = ck.ein;
	}

	if (pChunk->nFlags & CTRL_ORT_CYCLE)
    m_spline.ORT( TCBSpline<Vec3d>::ORT_CYCLE );
	else if (pChunk->nFlags & CTRL_ORT_LOOP)
		m_spline.ORT( TCBSpline<Vec3d>::ORT_LOOP );

	// Precompute spline tangents.
	m_spline.comp_deriv();
	return true;
}

//////////////////////////////////////////////////////////////////////////
Vec3d CControllerTCBVec3::GetPosition( float t )
{
	Vec3d val;
	m_spline.interpolate( t,val );
	// Position controller from Max must be scalled 100 times down.
	return val * (1.0f/100.0f);
}

//////////////////////////////////////////////////////////////////////////
Vec3d CControllerTCBVec3::GetScale( float t )
{
	// equialent to position.
	Vec3d val;
	m_spline.interpolate( t,val );
	return val;
}

//////////////////////////////////////////////////////////////////////////
bool CControllerTCBVec3::IsLooping() const
{
	if (m_spline.ORT() > TCBSpline<Vec3d>::ORT_CONSTANT)
		return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// CControllerTCBQuat spline implementation.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool CControllerTCBQuat::Load( const CONTROLLER_CHUNK_DESC_0826* pChunk,float secsPerTick )
{
	CryTCBQKey *pKeys = (CryTCBQKey*)(pChunk+1);

	int nkeys = pChunk->nKeys;
	m_spline.resize(nkeys);
	for (int i = 0; i < nkeys; i++)
	{
		CryTCBQKey &ck = pKeys[i];
		TCBAngAxisKey &key = m_spline.key(i);
		key.flags = 0;
		key.time = ck.time * secsPerTick;
		
		// TCBAngAxisSpline stores relative rotation angle-axis.
		//@FIXME rotation direction somehow differ from Max.
		// also invert direction of rotation by negating axis component.
		key.angle = ck.val.w;
//		key.axis = -Vec3d(ck.val.v.x,ck.val.v.y,ck.val.v.z);
		key.axis = Vec3d(ck.val.v.x,ck.val.v.y,ck.val.v.z);

		key.tens = ck.t;
		key.cont = ck.c;
		key.bias = ck.b;
		key.easefrom = ck.eout;
		key.easeto = ck.ein;
	}

	if (pChunk->nFlags & CTRL_ORT_CYCLE)
    m_spline.ORT( TCBAngleAxisSpline::ORT_CYCLE );
	else if (pChunk->nFlags & CTRL_ORT_LOOP)
		m_spline.ORT( TCBAngleAxisSpline::ORT_LOOP );
	// Precompute spline tangents.
	m_spline.comp_deriv();
	return true;
}

//////////////////////////////////////////////////////////////////////////
CryQuat CControllerTCBQuat::GetOrientation( float t )
{
	CryQuat q;
	m_spline.interpolate( t,q );
	return q;
}

//////////////////////////////////////////////////////////////////////////
bool CControllerTCBQuat::IsLooping() const
{
	if (m_spline.ORT() > TCBAngleAxisSpline::ORT_CONSTANT)
		return true;
	return false;
}