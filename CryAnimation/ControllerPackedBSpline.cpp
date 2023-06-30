// Implementation of Packed BSpline Controller

#include "stdafx.h"
#include "Controller.h"
#include "ControllerPackedBSpline.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loads (initializes) controller from the given chunk. The chunk descriptor is followed by the 
// chunk data immediately. Returns true if successful
// PARAMETERS:
//  pChunk - the header of the chunk to load data from. This is the start of the chunk in memory-mapped file normally.
//  nSize  - size of the chunk in bytes, including the header structure
//  scale  - factor to scale the positional data after successful load
//  pILog  - ILog
// RETURNS:
//  true if successfully loaded the controller, false otherwise (controller is not ready)
bool CControllerPackedBSpline::Load (const CONTROLLER_CHUNK_DESC_0826* pChunk, int nSize, float scale)
{
	// create one of the possible representations of packed spline, depending on the
	// format of the chunk: either 1- or 2-byte fixed point numbers and either closed or open spline
	switch (pChunk->type)
	{
	case CTRL_BSPLINE_1C:
		m_pRot = new TBSplineVec3dPacked<false,unsigned char>();
		m_pPos = new TBSplineVec3dPacked<false,unsigned char>();
		break;
	case CTRL_BSPLINE_2C:
		m_pRot = new TBSplineVec3dPacked<false,unsigned short>();
		m_pPos = new TBSplineVec3dPacked<false,unsigned short>();
		break;
	case CTRL_BSPLINE_1O:
		m_pRot = new TBSplineVec3dPacked<true,unsigned char>();
		m_pPos = new TBSplineVec3dPacked<true,unsigned char>();
		break;
	case CTRL_BSPLINE_2O:
		m_pRot = new TBSplineVec3dPacked<true,unsigned short>();
		m_pPos = new TBSplineVec3dPacked<true,unsigned short>();
		break;
	default:
		return false;
	};

	// The chunk must be at least big enough to contain the header structure
	if (nSize < sizeof (CONTROLLER_CHUNK_DESC_0826))
		return false;

	// controller identification will be subsequently used for bone binding
	m_nControllerId = pChunk->nControllerId;

	// 
	if(pChunk->chdr.ChunkType != ChunkType_Controller || pChunk->chdr.ChunkVersion != CONTROLLER_CHUNK_DESC_0826::VERSION)
	{
		GetLog()->LogToFile("CControllerPackedBSpline::Load: File version error");
    return false;
	}

	// go to the positional raw data subchunk
	char* pRawData = (char*)(pChunk+1);
	// remaining chunk piece size
	int nRawDataSize = nSize - sizeof (CONTROLLER_CHUNK_DESC_0826);

	// try to construct the position spline from the raw data
  int nPosDataSize = m_pPos->unpack (pRawData, nRawDataSize);
	if (!nPosDataSize)
		return false; // couldn't construct

	// the positional data has been successfully constructed
	// now scale it by the given factor
	m_pPos->scale (scale);
	
	// proceed with the rotation subchunk
	nRawDataSize -= nPosDataSize;
	pRawData += nPosDataSize;

	int nRotDataSize = m_pRot->unpack (pRawData, nRawDataSize);
	if (!nRotDataSize)
		return false;

	nRawDataSize -= nRotDataSize;
	pRawData += nRotDataSize;

	if (nRawDataSize != 0)
	{
		// the chunk loaded ok, but not all of its data has been used
		GetLog()->LogToFile("CControllerPackedBSpline::Load: %d extra bytes at hte end of the chunk", nRawDataSize );
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns position of the controller at the given time
CryQuat CControllerPackedBSpline::GetOrientation (float t)
{

	return exp ( quaternionf(0,CControllerPackedBSpline::GetOrientation2(t)) );

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns position of the controller at the given time
Vec3 CControllerPackedBSpline::GetPosition (float t)
{
	return m_pPos->getValue (t/*/BSplineKnots::g_nStdTicksPerSecond*/);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieves the position and orientation within one call
// may be optimal in some applications
void CControllerPackedBSpline::GetValue (float t, CryQuat& q, Vec3 &p)
{
	q = GetOrientation(t);
	p = GetPosition(t);
}

// retrieves the position and orientation (in the logarithmic space, i.e. instead of quaternion, its logarithm is returned)
// may be optimal for motion interpolation
void CControllerPackedBSpline::GetValue2 (float t, PQLog& pq)
{
	pq.vPos = GetPosition(t);
	pq.vRotLog = GetOrientation2(t);
}

//  returns the orientation of the controller at the given time, in logarithmic space
Vec3 CControllerPackedBSpline::GetOrientation2(float t)
{
	return m_pRot->getValue (t/*/BSplineKnots::g_nStdTicksPerSecond*/);
}

ILog* CControllerPackedBSpline::GetLog()const
{
	return g_GetLog();
}


size_t CControllerPackedBSpline::sizeofThis ()const
{
	return sizeof(*this) + m_pPos->sizeofThis() + m_pRot->sizeofThis();
}