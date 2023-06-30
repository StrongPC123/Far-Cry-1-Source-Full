//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:BSplineOpen
//  Declaration of
//     interface IBSpline3Packed
//     template <> struct TBSplineVec3dPackedBase
//     template <> struct TBSplineVec3dPackedDesc
//     template <> struct TBSplineVec3dPacked
//
//	History:
//	06/21/2002 :Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "BSplineOpen.h"

////////////////////////////////////////////////////////////////
// Packed (minimal memory footprint) representation of BSpline.
// Keeps the data in 1- or 2-byte fixed-point scaled numbers
// CAUTION: Avoid copying it too much, the data gets copied upon
// each invokation of the copy constructor.
////////////////////////////////////////////////////////////////

// interface to all representations of the packed spline
class IBSpline3Packed:
	public _reference_target<unsigned short> // we don't need the alignment of 4 bytes here
{
public:
	// apply the given scale to the spline (multiply the spline by scale
	virtual void scale (float fScale) = 0;

	// returns the size of the buffer required to pack in the spline in the current state
	virtual int getPackedSize()const = 0; // used

	// returns the size of the array that follows the descriptor structure
	virtual int getRawDataSize()const = 0;

	// returns the min corner of the bounding box for CPs
	virtual Vec3 getCPMin ()const = 0;
	// returns the max corner of the bounding box for CPs
	virtual Vec3 getCPMax ()const = 0;

	// returns the min time
	virtual float getTimeMin() const = 0;
	// returns the max time
	virtual float getTimeMax () const = 0;
	// returns the timespan range
	virtual float getTimeRange() const = 0;

	virtual int numKnots()const = 0;
	virtual int numCPs()const = 0;

	// unpacks this object out of the given storage;
	// returns the size of storage used for unpacking; 0 means unpack failed
	virtual int unpack(void* pBuffer, int nSize) = 0; // used

	// clears the contents of the spline
	virtual void clear() = 0;

	// constructs a packed version of the given unpacked bspline
	virtual void copy (BSplineVec3d* pSpline) = 0; // used

	// packs the spline into the given buffer
	virtual void pack (void* pBuffer) = 0; // used

	// calculates the value of the spline at the specified point
	virtual Vec3 getValue (float t) = 0;

	// returns the CP from the array of value triplets
	virtual Vec3 getCP (int nCP) = 0;

	// returns the time
	virtual float getKnotTime (int nKnot)const = 0;

	virtual unsigned sizeofThis()const = 0;
};

TYPEDEF_AUTOPTR(IBSpline3Packed);


////////////////////////////////////////////////////////////////////////
template <bool isOpen, typename FixedPointType>
struct TBSplineVec3dPackedBase
{
public:
	// fixed-point values.
	// may be unsigned short or unsigned char ONLY
	typedef FixedPointType fixed;

	enum
	{
		// total number of fixed values - may be overflow in the case of fixed == int
		//numFixedValues = 1<<(sizeof(fixed)*8)),
		// maximum fixed value representible
		nMaxFixedValue = ((fixed)~0)
	};
protected:
	TBSplineVec3dPackedBase():
		m_nDegree(1),
		m_numKnots(0)
		{
		}

	// degree of the spline
	short m_nDegree;
	// number of knots
	short m_numKnots;
	
	// the scale of the time values
	struct Scale
	{
		// the default scale is make all fixed values represent the interval [0,1)
		Scale():
			fBase(0),
			fScale(1.0f / (float)nMaxFixedValue)
			{
			}

		float fBase;
		float fScale;
		
		inline float unpack (fixed fVal)const
		{
			return fBase + fScale * fVal;
		}

		fixed pack (float fVal)const
		{
			float f=fScale;
			int i=nMaxFixedValue;
			
			if (fVal <= fBase)
				return 0;
			if (fVal >= fBase + f*i)
				return nMaxFixedValue;

			return (fixed)((fVal-fBase) / fScale + 0.5);
		}

		// unpacks the values, assuming that there are 2 more Scale structures after this
		Vec3 unpack (fixed*pVec)
		{
			return Vec3 (this[0].unpack(pVec[0]), this[1].unpack(pVec[1]), this[2].unpack(pVec[2]));
		}

		// initialize the structure base and scale so that the resulting fixed value range
		// maps effectively the [min,max] closed interval
		void initFromMinMax (float fMin, float fMax)
		{
			fBase  = fMin;
			fScale = (fMax-fMin) / nMaxFixedValue;
		}

		// returns the min representable value
		float getMin () const
		{
			return fBase;
		}

		// rehtrns the max reprsentable value
		float getMax () const
		{
			return fBase + getRange();
		}

		float getRange() const
		{
			float f=fScale;
			int i=nMaxFixedValue;
			return f * i;
		}

		void scale (float _scale)
		{
			fBase *= _scale;
			fScale *= _scale;
		}
	};
	// scale for each
	Scale m_TimeScale;
	Scale m_PosScale[3];

public:
};

//////////////////////////////////////////////////////////////////////////
// This is the descriptor of class TBSplineVec3dPacked.
// it's used to plainly pack/unpack the spline and reuse this structure
template <bool isOpen = true, typename FixedPointType = unsigned char>
struct TBSplineVec3dPackedDesc:
	public IBSpline3Packed,
	public TBSplineVec3dPackedBase<isOpen,FixedPointType>
{
	typedef TBSplineVec3dPackedBase<isOpen, FixedPointType> Base;

	// apply the given scale to the spline (multiply the spline by scale
	void scale (float fScale);

	// returns the size of the buffer required to pack in the spline in the current state
	int getPackedSize()const
	{
		return sizeof (Base) + this->getRawDataSize();
	}

	unsigned sizeofThis()const
	{
		return getPackedSize();
	}

	int numKnots()const
	{
		assert ((this->m_numKnots >= 2 && this->m_numKnots < 10000) || (this->m_numKnots==0 && this->m_nDegree==1));// boundary check
		return this->m_numKnots;
	}
	int numCPs()const
	{
		assert (this->m_nDegree >= 1 && this->m_nDegree < 5);// boundary check
		return this->m_numKnots + (isOpen? this->m_nDegree - 1 : -1);
	}
	// returns the number of raw data elements
	int numRawDataElements()const
	{
		return (numKnots() - 2 + 3*numCPs());
	}
	// returns the size of the array that follows the descriptor structure
	int getRawDataSize()const
	{
		return numRawDataElements()*sizeof(typename TBSplineVec3dPackedBase<isOpen,FixedPointType>::fixed);
	}

	// returns the min corner of the bounding box for CPs
	Vec3 getCPMin ()const;
	// returns the max corner of the bounding box for CPs
	Vec3 getCPMax ()const;

	// returns the min time
	float getTimeMin() const;
	// returns the max time
	float getTimeMax () const;
	// returns the timespan range
	float getTimeRange() const;
};

////////////////////////////////////////////////////////////////////////
// The spline packed implementation
template <bool isOpen = true, typename FixedPointType = unsigned char>
class TBSplineVec3dPacked:
	public TBSplineVec3dPackedDesc<isOpen, FixedPointType>
{
public:
	typedef TBSplineVec3dPackedDesc<isOpen, FixedPointType> Desc;

	// constructs an empty spline (no knots, no keys)
	// you can unpack or copy the 
	TBSplineVec3dPacked ();

	// destructs
	~TBSplineVec3dPacked (void);

	// copies the packed version of the BSpline
	TBSplineVec3dPacked (const TBSplineVec3dPacked<isOpen, FixedPointType>& that);

	// unpacks this object out of the given storage;
	// returns the size of storage used for unpacking; 0 means unpack failed
	int unpack(void* pBuffer, int nSize);

	// clears the contents of the spline
	void clear();

	// constructs a packed version of the given unpacked bspline
	void copy (BSplineVec3d* pSpline);

	// packs the spline into the given buffer
	void pack (void* pBuffer);

	// calculates the value of the spline at the specified point
	Vec3 getValue (float t);

	// returns the CP from the array of value triplets
	Vec3 getCP (int nCP);

	// returns the time
	float getKnotTime (int nKnot)const;

	// searches and returns the interval to which the given time belongs.
	// each pair of knots defines interval [k1,k2)
	// -1 is before the 0-th knot, numKnots()-1 is after the last knot
	int findInterval (float fTime)const;

	// returns the i-th basis function of degree d, given the time t
	float getBasis (int i, int d, float t) const;

	// returns the i-th basis function of degree d, given the time t and a hint, in which interval to search for it
	float getBasis (int i, int d, float t, int nIntervalT) const;
protected:
	// returns the value of the basis function of degree d, given the time t
	// the knots of the basis function start at pKnotBegin, and the first knot before t is pKnotBeforeT
	float getBasisUnsafe (int nKnotBegin, int d, float t, int nKnotBeforeT) const;

	// plain array of fixed-point values scaled between the min and max time and position
	// there are m_numKnots knot time values, immediately followed by m_numKnots+m_nDegree-1
	// control point values, each control point value is 3 fixed values
	FixedPointType* m_pData;
};

template <bool isOpen, typename FixedPointType>
TBSplineVec3dPacked<isOpen, FixedPointType>::TBSplineVec3dPacked(void):
	m_pData(NULL)
{
}

template <bool isOpen, typename FixedPointType>
TBSplineVec3dPacked<isOpen, FixedPointType>::~TBSplineVec3dPacked(void)
{
	clear();
}


// copies the packed version of the BSpline
template <bool isOpen, typename FixedPointType>
TBSplineVec3dPacked<isOpen, FixedPointType>::TBSplineVec3dPacked (const TBSplineVec3dPacked<isOpen,FixedPointType>& that):
	Desc(that)
{
	if (that.numKnots()) // if not an empty one
	{
		m_pData = new typename TBSplineVec3dPackedBase<isOpen,FixedPointType>::fixed[that.numRawDataElements()];
		memcpy (m_pData, that.m_pData, that.getRawDataSize());
	}
	else
		m_pData = NULL;
}

//////////////////////////////////////////////////////////////////////////
// unpacks this object out of the given storage;
// returns the size of storage used for unpacking; 0 means unpack failed
template <bool isOpen, typename FixedPointType>
int TBSplineVec3dPacked<isOpen, FixedPointType>::unpack(void* pBuffer, int nSize)
{
	clear();
	
	if (nSize < sizeof(typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base))
		return 0; // failed

	memcpy (static_cast<typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base*>(this), pBuffer, sizeof(typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base));

	if (nSize < (int)sizeof(typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base) + this->getRawDataSize())
	{
		clear();
		return 0;
	}

	m_pData = (new typename TBSplineVec3dPackedBase<isOpen,FixedPointType>::fixed[this->numRawDataElements()]);
	memcpy (m_pData, ((const typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base*)pBuffer)+1, this->getRawDataSize());
	return sizeof(typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base) + this->getRawDataSize();
}

//////////////////////////////////////////////////////////////////////////
// clears the contents of the spline
template <bool isOpen, typename FixedPointType>
void TBSplineVec3dPacked<isOpen, FixedPointType>::clear()
{
	if (m_pData)
	{
		delete[]m_pData;
		m_pData = NULL;
	}
	
	this->m_nDegree = 1;
	this->m_numKnots = 0;
}

//////////////////////////////////////////////////////////////////////////
// constructs a packed version of the given unpacked bspline
template <bool isOpen, typename FixedPointType>
void TBSplineVec3dPacked<isOpen, FixedPointType>::copy (BSplineVec3d* pSpline)
{
	int i, nCoord;
	assert (isOpen == pSpline->isOpen());

	clear();
	this->m_nDegree = pSpline->getDegree();
	this->m_numKnots = pSpline->numKnots();

	assert (this->numCPs() == pSpline->numCPs());

	// find the time scale
	this->m_TimeScale.initFromMinMax (pSpline->getKnotTime(0), pSpline->getKnotTime(this->m_numKnots-1));

	// find the spacial scale for the CPs - min/max CP coordinates
	Vec3 vMinCP, vMaxCP;
	vMinCP = vMaxCP = pSpline->getCP(0);

	for (i = 1; i < pSpline->numCPs(); ++i)
	{
		const Vec3& vCP = pSpline->getCP(i);
		for (nCoord = 0; nCoord < 3; ++nCoord)
		{
			if (vMinCP[nCoord] > vCP[nCoord])
				vMinCP[nCoord] = vCP[nCoord];
			else
			if (vMaxCP[nCoord] < vCP[nCoord])
				vMaxCP[nCoord] = vCP[nCoord];
		}
	}

	// map the min/max coordinates to the scales
	for (nCoord = 0; nCoord < 3; ++nCoord)
	{
		this->m_PosScale[nCoord].initFromMinMax(vMinCP[nCoord], vMaxCP[nCoord]);
	}

	m_pData = new typename TBSplineVec3dPackedBase<isOpen,FixedPointType>::fixed[this->numRawDataElements()];

	// copy the data (knots and CPs)
	// We ignore the first and last knot, because information about them is already present in the scale structure
	for (i = 0; i < pSpline->numKnots()-2; ++i)
		m_pData[i] = this->m_TimeScale.pack(pSpline->getKnotTime(i+1));

	// each CP has 3 coordinates, each mapped with different pos scale
	for (i = 0; i < pSpline->numCPs(); ++i)
		for (nCoord = 0; nCoord < 3; ++nCoord)
			m_pData[i*3+nCoord+this->numKnots()-2] = this->m_PosScale[nCoord].pack (pSpline->getCP(i)[nCoord]);
}

//////////////////////////////////////////////////////////////////////////
// packs the spline into the given buffer
template <bool isOpen, typename FixedPointType>
void TBSplineVec3dPacked<isOpen, FixedPointType>::pack (void* pBuffer)
{
	memcpy (pBuffer, static_cast<const typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base*>(this), sizeof(typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base));
	memcpy (static_cast<typename TBSplineVec3dPackedDesc<isOpen, FixedPointType>::Base*>(pBuffer) + 1, m_pData, this->getRawDataSize());
}

//////////////////////////////////////////////////////////////////////////
// returns the CP from the array of value triplets
template <bool isOpen, typename FixedPointType>
Vec3 TBSplineVec3dPacked<isOpen, FixedPointType>::getCP (int nCP)
{
	
	if (isOpen)
	{
		if (nCP < 0)
			nCP = 0;
		else
		if (nCP >= this->numCPs())
			nCP = this->numCPs()-1;
	}
	else
	{
		nCP = nCP % this->numCPs();
		if (nCP < 0)
			nCP += this->numCPs();
	}

  assert (nCP >= 0 && nCP < this->numCPs());
	return this->m_PosScale->unpack(m_pData + this->numKnots()-2 + 3 * nCP);
}

//////////////////////////////////////////////////////////////////////////
// returns the normalized knot and the base - how many cycles the knot rolled back or forth
extern int PackedSplineClosedGetKnotTime(int &nKnot, int numKnots);


//////////////////////////////////////////////////////////////////////////
// returns the time
template <bool isOpen, typename FixedPointType>
float TBSplineVec3dPacked<isOpen, FixedPointType>::getKnotTime (int nKnot)const
{
	if (!isOpen)
	{
		int nBase = PackedSplineClosedGetKnotTime(nKnot, this->numKnots());

		if (nKnot == 0)
			return this->m_TimeScale.getMin() + this->m_TimeScale.getRange()*nBase;
		else
		{
			assert (nKnot < this->numKnots()-1);
			return this->m_TimeScale.getRange()*nBase + this->m_TimeScale.unpack (m_pData[nKnot-1]);
		}
	}
	else
	{
		if (nKnot <= 0)
			return this->m_TimeScale.getMin();
		else
		if (nKnot >= this->numKnots()-1)
			return this->m_TimeScale.getMax();
		else
		return this->m_TimeScale.unpack (m_pData[nKnot-1]);
	}
}


//////////////////////////////////////////////////////////////////////////
// returns the max corner of the bounding box for CPs
template <bool isOpen, typename FixedPointType>
Vec3 TBSplineVec3dPackedDesc<isOpen, FixedPointType>::getCPMax ()const
{
	return Vec3(this->m_PosScale[0].getMax(), this->m_PosScale[1].getMax(), this->m_PosScale[2].getMax());
}


//////////////////////////////////////////////////////////////////////////
// returns the min corner of the bounding box for CPs
template <bool isOpen, typename FixedPointType>
Vec3 TBSplineVec3dPackedDesc<isOpen, FixedPointType>::getCPMin ()const
{
	return Vec3(this->m_PosScale[0].getMin(), this->m_PosScale[1].getMin(), this->m_PosScale[2].getMin());
}


//////////////////////////////////////////////////////////////////////////
// returns the min time
template <bool isOpen, typename FixedPointType>
float TBSplineVec3dPackedDesc<isOpen, FixedPointType>::getTimeMin() const
{
	return this->m_TimeScale.getMin();
}


//////////////////////////////////////////////////////////////////////////
// returns the max time
template <bool isOpen, typename FixedPointType>
float TBSplineVec3dPackedDesc<isOpen, FixedPointType>::getTimeMax () const
{
	return this->m_TimeScale.getMax();
}


//////////////////////////////////////////////////////////////////////////
// returns the max-min time
template <bool isOpen, typename FixedPointType>
float TBSplineVec3dPackedDesc<isOpen, FixedPointType>::getTimeRange () const
{
	return this->m_TimeScale.getRange();
}


//////////////////////////////////////////////////////////////////////////
// calculates the value of the spline at the specified point
template <bool isOpen, typename FixedPointType>
Vec3 TBSplineVec3dPacked<isOpen, FixedPointType>::getValue (float fTime)
{
	int nInterval = findInterval (fTime);
	assert (nInterval < 0 || nInterval >= this->numKnots()-1 || (getKnotTime (nInterval) <= fTime && fTime <= getKnotTime(nInterval+1)));

	if (nInterval < 0)
		return getCP(0);

	if (nInterval >= this->numKnots()-1)
		return getCP(this->numCPs()-1);
	
	Vec3 vResult(0,0,0);
	for (int i = 0; i >= -this->m_nDegree; --i)
	{
		Vec3 vCP = getCP(nInterval + (isOpen?this->m_nDegree:0) + i);
		float fBasis = getBasis (nInterval + i, this->m_nDegree, fTime, nInterval);
		vResult += vCP * fBasis;
	}

	return vResult;
}


//////////////////////////////////////////////////////////////////////////
// searches and returns the interval to which the given time belongs.
// each pair of knots defines interval [k1,k2)
// -1 is before the 0-th knot, numKnots()-1 is after the last knot
template <bool isOpen, typename FixedPointType>
int TBSplineVec3dPacked<isOpen, FixedPointType>::findInterval (float fTime)const
{
	if (fTime < this->m_TimeScale.getMin())
		return -1;
	if (fTime < getKnotTime (1))
		return 0;
	if (fTime >= this->m_TimeScale.getMax())
		return this->numKnots()-1;
	if (fTime >= getKnotTime (this->numKnots()-2))
		return this->numKnots()-2;

	typename TBSplineVec3dPackedBase<isOpen,FixedPointType>::fixed fTimePacked = this->m_TimeScale.pack(fTime);
	// depending on how we rounded the fTIme value, we seek either the bigger value or bigger or equal
	if (fTime > this->m_TimeScale.unpack(fTimePacked))
		// now, the time is clamped and can be safely packed 
		// search for the value that's bigger than fTimePacked
		return std::upper_bound (m_pData, m_pData + this->numKnots()-2, fTimePacked) - m_pData;
	else
		// search for the fTimePacked or bigger value
		return std::lower_bound (m_pData, m_pData + this->numKnots()-2, fTimePacked) - m_pData;
}

// returns the i-th basis function of degree d, given the time t
template <bool isOpen, typename FixedPointType>
float TBSplineVec3dPacked<isOpen, FixedPointType>::getBasis (int i, int d, float t) const
{
	// the requested basis must have defined supporting knots
	assert (i>= -d && i < this->numKnots()-1);
	
	// starting and ending knots - they demark the support interval: [*begin,*(end-1)]
	int nKnotBegin = i;
	int nKnotEnd  = nKnotBegin + d + 2;
	// find the interval where the t is, among the supporting intervals
	// the upper bound of that interval is searched for
	int nKnotAfterT( nKnotBegin );
	for ( ; nKnotAfterT < nKnotEnd && getKnotTime (nKnotAfterT) < t; ++nKnotAfterT)
	continue;

	if (nKnotAfterT == nKnotBegin)
	{
		assert (t < getKnotTime(nKnotBegin));
		return 0; // the time t is before the supporting interval of the basis function
	}

	if (nKnotAfterT == nKnotEnd)
	{
		if (t > getKnotTime(nKnotEnd - 1))
			return 0; // the time t is after the supporting interval

		assert (t == getKnotTime (nKnotEnd - 1));
		// scan down multiple knots
		while (t == getKnotTime (nKnotAfterT - 1));
			--nKnotAfterT;
	}
	
	return getBasis (nKnotBegin, d, t, nKnotAfterT - 1);
}

// returns the i-th basis function of degree d, given the time t and a hint, in which interval to search for it
template <bool isOpen, typename FixedPointType>
float TBSplineVec3dPacked<isOpen, FixedPointType>::getBasis (int i, int d, float t, int nIntervalT) const
{
	if (nIntervalT < -d || nIntervalT >= this->numKnots()-1)
		return 0;
	
	assert (t >= getKnotTime(nIntervalT) && (t < getKnotTime(nIntervalT+1) || (t==getKnotTime(nIntervalT+1)&&getKnotTime(nIntervalT+1)==getKnotTime(nIntervalT))));

	// the requested basis must have defined supporting knots
	if (i < -d || i >= this->numKnots()-1)
		return 0;

	if (nIntervalT < i || nIntervalT > i + d)
		return 0; // the time is outside supporting base
	
	return getBasisUnsafe (i, d, t, nIntervalT);
}

// returns the value of the basis function of degree d, given the time t
// the knots of the basis function start at pKnotBegin, and the first knot before t is pKnotBeforeT
template <bool isOpen, typename FixedPointType>
float TBSplineVec3dPacked<isOpen, FixedPointType>::getBasisUnsafe (int nKnotBegin, int d, float t, int nKnotBeforeT)const
{
	assert (t >= getKnotTime(nKnotBeforeT) && t <= getKnotTime(nKnotBeforeT+1));
	assert (nKnotBegin >= -d && nKnotBegin < this->numKnots()-1);
	assert (nKnotBeforeT >= nKnotBegin && nKnotBeforeT <= nKnotBegin + d);

	switch (d)
	{
	case 0:
		// trivial case
		return 1;
	case 1:
		if (nKnotBeforeT == nKnotBegin)
		{
			// the t is in the first interval
			return (t - getKnotTime(nKnotBegin)) / (getKnotTime(nKnotBegin+1) - getKnotTime(nKnotBegin));
		}
		else
		{
			assert (nKnotBeforeT == nKnotBegin + 1);
			return (getKnotTime(nKnotBegin+2) - t) / (getKnotTime(nKnotBegin+2) - getKnotTime(nKnotBegin+1));
		}
	default:
		{
			float fResult = 0;
			if (nKnotBeforeT < nKnotBegin + d)
				fResult += getBasis (nKnotBegin, d-1, t, nKnotBeforeT) * (t - getKnotTime(nKnotBegin)) / (getKnotTime(nKnotBegin+d) - getKnotTime(nKnotBegin));
			if (nKnotBeforeT > nKnotBegin)
				fResult += getBasis (nKnotBegin+1, d-1, t, nKnotBeforeT) * (getKnotTime(nKnotBegin+d+1) - t) / (getKnotTime(nKnotBegin+d+1) - getKnotTime(nKnotBegin+1));
			return fResult;
		}
	}
}

// apply the given scale to the spline (multiply the spline by scale
template <bool isOpen, typename FixedPointType>
void TBSplineVec3dPackedDesc<isOpen,FixedPointType>::scale (float fScale)
{
	this->m_PosScale[0].scale (fScale);
	this->m_PosScale[1].scale (fScale);
	this->m_PosScale[2].scale (fScale);
}

typedef TBSplineVec3dPacked<true,unsigned char> BSplineVec3dPackedOpen;
typedef TBSplineVec3dPacked<false,unsigned char> BSplineVec3dPackedClosed;
