////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   controllertcb.h
//  Version:     v1.00
//  Created:     12/11/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: TCB controller implementation.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __controllertcb_h__
#define __controllertcb_h__
#pragma once

#include "Controller.h"
#include "TCBSpline.h"

///////////////////////////////////////////////////////////////////////////
// class CControllerTCB
// Implementation of IController interface (see File:Controller.h)
// Controller implementing the TCB (Kochanek-Bartles Hermit spline).
///////////////////////////////////////////////////////////////////////////
class CControllerTCB: public IController
{
public:
	// each controller has an ID, by which it is identifiable
	unsigned GetID () const {return m_nControllerId;}

	//  returns orientation of the controller at the given time
	CryQuat GetOrientation (float t);

	//  returns the orientation of the controller at the given time, in logarithmic space
	Vec3 GetOrientation2(float t) { return Vec3(0,0,0); };

	// returns position of the controller at the given time
	Vec3 GetPosition (float t);

	// returns scale of the controller at the given time
	Vec3 GetScale (float t);
	
	// retrieves the position and orientation within one call
	// may be optimal in some applications
	void GetValue (float t, CryQuat& q, Vec3 &p);

	// ignore.
	void GetValue2 (float t, PQLog& pq) {};

	// returns the start time
	virtual float GetTimeStart ()
	{
		return m_timeStart;
	}

	// returns the end time
	virtual float GetTimeEnd()
	{
		return m_timeEnd;
	}

	ILog* GetLog()const;

	size_t sizeofThis() const { return sizeof(*this); }
protected:
	// Controller ID, used for identification purposes (bones are bound to controllers using their IDs
	unsigned m_nControllerId;

	float m_timeStart;
	float m_timeEnd;
};

//////////////////////////////////////////////////////////////////////////
// TCB Controller implementation for vector.
//////////////////////////////////////////////////////////////////////////
class CControllerTCBVec3 : public CControllerTCB
{
public:
	// Loads (initializes) controller from the given chunk. The chunk descriptor is followed by the 
	// chunk data immediately. Returns true if successful
	bool Load( const CONTROLLER_CHUNK_DESC_0826* pChunk,float secsPerTick );

	// returns position of the controller at the given time
	Vec3 GetPosition (float t);

	// returns scale of the controller at the given time
	Vec3 GetScale (float t);

	virtual bool IsLooping() const;
	
	size_t sizeofThis()const
	{
		return sizeof(*this) + m_spline.sizeofThis();
	}

protected:
	// TCB Splines.
	TCBSpline<Vec3> m_spline;
};

//////////////////////////////////////////////////////////////////////////
// TCB Controller implementation for quaternion.
//////////////////////////////////////////////////////////////////////////
class CControllerTCBQuat : public CControllerTCB
{
public:
	// Loads (initializes) controller from the given chunk. The chunk descriptor is followed by the 
	// chunk data immediately. Returns true if successful
	bool Load( const CONTROLLER_CHUNK_DESC_0826* pChunk,float secsPerTick );

	//  returns orientation of the controller at the given time
	CryQuat GetOrientation(float t);

	virtual bool IsLooping() const;

	size_t sizeofThis()const
	{
		return sizeof(*this) + m_spline.sizeofThis();
	}
protected:
	// TCB Splines.
	TCBAngleAxisSpline m_spline;
};

//////////////////////////////////////////////////////////////////////////
TYPEDEF_AUTOPTR(CControllerTCB);

#endif // __controllertcb_h__