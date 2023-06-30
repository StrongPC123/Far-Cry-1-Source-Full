/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//	
//  Notes:
//    CControllerPackedBSpline class declaration
//    See the CControllerPackedBSpline comment for more info
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRYTEK_CONTROLLER_PACKED_BSPLINE_HEADER_
#define _CRYTEK_CONTROLLER_PACKED_BSPLINE_HEADER_

#include "BSplineVec3dPacked.h"

///////////////////////////////////////////////////////////////////////////
// class CControllerPackedBSpline
// Implementation of IController interface (see File:Controller.h)
// Controller implementing the packed representation of BSpline, exported
// from the Motion Optimizer utility
///////////////////////////////////////////////////////////////////////////
class CControllerPackedBSpline: public IController
{
public:
	// Loads (initializes) controller from the given chunk. The chunk descriptor is followed by the 
	// chunk data immediately. Returns true if successful
  bool Load(const CONTROLLER_CHUNK_DESC_0826* pChunk, int nSize, float scale); 

	// each controller has an ID, by which it is identifiable
	unsigned GetID () const {return m_nControllerId;}

	//  returns orientation of the controller at the given time
	CryQuat GetOrientation (float t);

	//  returns the orientation of the controller at the given time, in logarithmic space
	Vec3 GetOrientation2(float t);

	// returns position of the controller at the given time
	Vec3 GetPosition (float t);

	// returns scale of the controller at the given time
	Vec3 GetScale (float t)
	{
		return Vec3(1,1,1);
	}
	
	// retrieves the position and orientation within one call
	// may be optimal in some applications
	void GetValue (float t, CryQuat& q, Vec3 &p);

	// retrieves the position and orientation (in the logarithmic space, i.e. instead of quaternion, its logarithm is returned)
	// may be optimal for motion interpolation
	void GetValue2 (float t, PQLog& pq);

	// returns the start time
	virtual float GetTimeStart ()
	{
		return min(m_pPos->getTimeMin(), m_pRot->getTimeMin());
	}

	// returns the end time
	virtual float GetTimeEnd()
	{
		return max(m_pPos->getTimeMax(), m_pRot->getTimeMax());
	}

	ILog* GetLog()const;

	size_t sizeofThis ()const;
protected:
	// Controller ID, used for identification purposes (bones are bound to controllers using their IDs
	unsigned m_nControllerId;

	// packed splines for position and orientation.
	// orientational data is represented by logarithmic mapping of the quaternion.
	// may be one of 4 formats of packed splines.
	IBSpline3Packed_AutoPtr m_pPos, m_pRot;
};

TYPEDEF_AUTOPTR(CControllerPackedBSpline);

#endif