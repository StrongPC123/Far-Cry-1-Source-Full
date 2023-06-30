/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//	
//  Notes:
//    CControllerCryBone class declaration
//    CControllerCryBone is implementation of IController which is compatible with
//    the old (before 6/27/02) caf file format that contained only CryBoneKey keys.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CRYTEK_CONTROLLER_CRY_BONE_HEADER_
#define _CRYTEK_CONTROLLER_CRY_BONE_HEADER_

// #include "Controller.h" // must be included for the declaration of IController

// old motion format cry bone controller
class CControllerCryBone: public IController
{
public:
	unsigned numKeys()const
	{
		return (unsigned)m_arrKeys.size();
	}

	unsigned GetID () const {return m_nControllerId;}

  CControllerCryBone();
  ~CControllerCryBone();
  
	// Loads (initializes) controller from the given chunk. The chunk descriptor is followed by the 
	// chunk data immediately. Returns true if successful
  bool Load(const CONTROLLER_CHUNK_DESC_0826* pChunk, float scale); 

	// Loads (initializes) controller from the given chunk. The chunk descriptor is followed by the 
	// chunk data immediately. Returns true if successful
	bool Load (const CONTROLLER_CHUNK_DESC_0827* pChunk, unsigned nSize, float fScale);

	// retrieves the position and orientation within one call
	// may be optimal in some applications
	void GetValue(float t, CryQuat& q, Vec3 &p)
	{
		PQLog pq;
		GetValue2(t, pq);

		q = exp( quaternionf(0,pq.vRotLog) );

		p = pq.vPos;
	}

	// retrieves the position and orientation (in the logarithmic space,
	// i.e. instead of quaternion, its logarithm is returned)
	// may be optimal for motion interpolation
	void GetValue2 (float t, PQLog& pq);

	void LogKeys(const char* szFileName, const char* szVarName);

	CryQuat GetOrientation (float fTime)
	{
		PQLog pq;
		GetValue2(fTime, pq);
		return exp( quaternionf(0,pq.vRotLog) );
	}

	// returns the orientation of the controller at the given time, in logarithmic space
	Vec3 GetOrientation2 (float t)
	{
		PQLog pq;
		GetValue2(t, pq);
		return pq.vRotLog;
	}

	Vec3 GetPosition (float fTime)
	{
		PQLog pq;
		GetValue2(fTime, pq);
		return pq.vPos;
	}

	virtual Vec3 GetScale (float t)
	{
		return Vec3(1,1,1);
	}

	// returns the start time
	float GetTimeStart ()
	{
		return float(m_arrTimes[0]);
	}

	// returns the end time
	float GetTimeEnd()
	{
		assert (numKeys() > 0);
		return float(m_arrTimes[numKeys()-1]);
	}

	ILog* GetLog() const;

	size_t sizeofThis ()const;
protected:
	TFixedArray<PQLog> m_arrKeys;
	TElementaryArray<int> m_arrTimes;

	unsigned m_nControllerId;
};

TYPEDEF_AUTOPTR(CControllerCryBone);

#endif

