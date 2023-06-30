/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	Created by Sergiy Migdalskiy
//	
//  Notes:
//    IController interface declaration
//    See the IController comment for more info
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRYTEK_CONTROLLER_HEADER_
#define _CRYTEK_CONTROLLER_HEADER_

#include "StringUtils.h"
#include "CryHeaders.h"

//////////////////////////////////////////////////////////////////////////////////////////
// interface IController  
// Describes the position and orientation of an object, changing in time.
// Responsible for loading itself from a binary file, calculations
//////////////////////////////////////////////////////////////////////////////////////////
class IController: public _reference_target_t
{
public:

	// each controller has an ID, by which it is identifiable
	virtual unsigned GetID () const = 0;

	// returns the orientation of the controller at the given time
	virtual CryQuat GetOrientation (float t) = 0;

	// returns the orientation of the controller at the given time, in logarithmic space
	virtual Vec3 GetOrientation2 (float t) = 0;

	// returns position of the controller at the given time
	virtual Vec3 GetPosition (float t) = 0;

	// returns scale of the controller at the given time
	virtual Vec3 GetScale (float t) = 0;
	
	// retrieves the position and orientation within one call
	// may be optimal in some applications
	virtual void GetValue (float t, CryQuat& q, Vec3 &p) = 0;

	// retrieves the position and orientation (in the logarithmic space, i.e. instead of quaternion, its logarithm is returned)
	// may be optimal for motion interpolation
	struct PQLog
	{
		Vec3 vPos;
		Vec3 vRotLog; // logarithm of the rotation

		string toString()const {return "{pos:" + CryStringUtils::toString(vPos)+", rot="+CryStringUtils::toString (vRotLog) + "}";}

		// returns the rotation quaternion
		CryQuat getOrientation()const;
		// resets the state to zero
		void reset ();
		// blends the pqSource[0] and pqSource[1] with weights 1-fBlend and fBlend into pqResult
		void blendPQ (const PQLog* pqSource, float fBlend);
		// blends the pqFrom and pqTo with weights 1-fBlend and fBlend into pqResult
		void blendPQ (const PQLog& pqFrom, const PQLog& pqTo, float fBlend);
		// builds the matrix out of the position and orientation stored in this PQLog
		void buildMatrix(Matrix44& mat)const;
		// a special version of the buildMatrix that adds a rotation to the rotation of this PQLog
		void buildMatrixPlusRot(Matrix44& mat, const CryQuat& qRotPlus) const;
		// returns the equivalent rotation in logarithmic space (the quaternion of which is negative the original)
		Vec3 getComplementaryRotLog() const;
		// constructs the position/rotation from the given matrix
		void assignFromMatrix (const Matrix44& mat);

		PQLog operator * (float f)const
		{
			PQLog res;
			res.vPos = this->vPos * f;
			res.vRotLog = this->vRotLog * f;
			return res;
		}

		const PQLog& operator += (const PQLog& pq)
		{
			this->vPos += pq.vPos;
			this->vRotLog += pq.vRotLog;
			return *this;
		}
	};
	virtual void GetValue2 (float t, PQLog& pq) = 0;

	// returns the start time
	virtual float GetTimeStart () = 0;

	// returns the end time
	virtual float GetTimeEnd() = 0;

	virtual size_t sizeofThis () const = 0;

	virtual bool IsLooping() const { return false; };
};

TYPEDEF_AUTOPTR(IController);
//typedef IController*IController_AutoPtr;

// adjusts the rotation of these PQs: if necessary, flips them or one of them (effectively NOT changing the whole rotation,but
// changing the rotation angle to Pi-X and flipping the rotation axis simultaneously)
// this is needed for blending between animations represented by quaternions rotated by ~PI in quaternion space
// (and thus ~2*PI in real space)
extern void AdjustLogRotations (Vec3& vRotLog1, Vec3& vRotLog2);
extern void AdjustLogRotationTo (const Vec3& vRotLog1, Vec3& vRotLog2);
extern void AdjustLogRotation(Vec3& vRotLog);


//////////////////////////////////////////////////////////////////////////
// This class is a non-trivial predicate used for sorting an
// ARRAY OF smart POINTERS  to IController's. That's why I need a separate
// predicate class for that. Also, to avoid multiplying predicate classes,
// there are a couple of functions that are also used to find a IController
// in a sorted by ID array of IController* pointers, passing only ID to the
// lower_bound function instead of creating and passing a dummy IController*
class AnimCtrlSortPred
{
public:
	bool operator() (const IController_AutoPtr& a, const IController_AutoPtr& b) {assert (a!=(IController*)NULL && b != (IController*)NULL); return a->GetID() < b->GetID();}
	bool operator() (const IController_AutoPtr& a, unsigned nID) {assert (a != (IController*)NULL);return a->GetID() < nID;}
};


#endif