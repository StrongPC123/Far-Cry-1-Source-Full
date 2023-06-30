//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:BSplineOpen
//  Declaration of class BSplineVec3d (originally only open, but now
//  capable of being a closed spline)
//
//	History:
//	06/17/2002 :Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "BSplineKnots.h"

////////////////////////////////////////////////////////////////
// This is a general B-Spline that can interpolate any values (like vectors or scalars)
// that support linear combination operations + and *
// Upon construction, you specify the number of unique knots (the extra knots are
// automatically added to maintain inclusive boundary condition) and the degree
// The number of control ponints you are to supply afterwards through the []
// operator is:
//   numCPs() == nNumKnots + d - 1  // for open spline
//   numCPs() == nNumKnots - 1      // for closed spline
// This is natural spline without derivative boundary conditions.
// The total number of extra knots added is 2*d, d for each boundary
/////////////////////////////////////////////////////////////////

// Value is the value that's interpolated by this BSpline
// Desc is the descriptor class with some static members describing
//   how to treat the spline: as a closed or open one
class BSplineVec3d:
	public _reference_target_t
{
public:
	typedef Vec3 Value;
	typedef BSplineKnots::Time Time;
	typedef BSplineKnots::Value BlendValue;

	// number of unique knots passed, and the degree
	// the array of control points is NOT initialized
	BSplineVec3d (int numKnots, int nDegree, bool isOpen);
	~BSplineVec3d(void);

	bool isOpen() {return m_isOpen;}
	
	// retrieves the number of basis functions (and thus unique control points) involved
	int numCPs()const
	{
		return m_isOpen ? numKnots() + m_nDegree - 1 : numKnots() - 1;
	}
	// number of unique knots maintained
	int numKnots()const
	{
		return m_Knots.numKnots() - 2 * m_nDegree;
	}

	// retrieves a reference to the i-th control point
	Value& operator [] (int i)
	{
		assert (i >= 0 && i < numCPs());
		return m_arrCPs[i];
	}

	// retrieves a reference to the i-th control point
	const Value& operator [] (int i)const
	{
		return getCP(i);
	}

	// retrieves a reference to the i-th control point
	const Value& getCP (int i) const
	{
		assert (i >= 0 && i < numCPs());
		return m_arrCPs[i];
	}

	// sets the given control point
	void setCP (int i, const Value& pt)
	{
		assert (i >= 0 && i < numCPs());
		m_arrCPs[i] = pt;
	}

	// computes the value of the spline at the given time
	Value getValue (Time fTime)const;

	// computes the value of the spline at the given time t
	Value operator () (Time fTime) const
	{
		return getValue (fTime);
	}

	// sets the given unique knot time
	void setKnotTime (int nKnot, Time fTime);

	// call this after setting all knot times
	void finalizeKnotTimes();

	// retrieves the knot time, knot index is in the unique knot indexation (i.e. normally 0th != 1st)
	Time getKnotTime (int nKnot)const;

	// retrives the basis function value for the given control point at the given time
	BlendValue getBasis (int nControlPoint, Time fTime) const;

	// Returns the basis function starting at knot i (m_Knot internal numeration)
	// The function is calculated at time t, on the interval nIntervalT (m_Knot numeration)
	BlendValue getBasis (int i, Time t, int nIntervalT) const;

	// get the d+1 derivative (delta) amplitude at knot k
	Value getDelta (int k);

	// return the spline degree
	int getDegree()const
	{
		return m_nDegree;
	}

	// retrives the boundaries of the influence of the given control point, in time:
	// pTime[0] is the starting knot, pTime[1] is the end knot
	// NOTE: end knot may be < start knot; it means the support interval is cyclic
	void getInfluenceInterval (int nControlPoint, int pTime[2]) const;

	// Returns the P (product) penalty for knot closeness, as defined by Mary J. Lindstrom "Penalized Estimation of Free Knot Splines" 1999,
	// logarithmic form
	double getKnotProductPenalty()
	{
		return m_Knots.getKnotProductPenalty(m_nDegree, m_isOpen?numCPs() : m_nDegree+numKnots()-1);
	}

	// returns the time at which the given CP has the maximum influence at the curve
	BSplineKnots::Time getCPPeak (int nControlPoint)
	{
		return m_Knots.getBasisPeak (nControlPoint + (m_isOpen?0:m_nDegree), m_nDegree);
	}
protected:
	// Modifies the given time so that it fits inside the spline boundaries.
	void fixupTime(Time& fTime)const;

	// degree of the spline
	const int m_nDegree;
	
	// array of control ponits
	TElementaryArray<Value> m_arrCPs;
	
	// knot array
	BSplineKnots m_Knots;

	// is the spline open?
	bool m_isOpen;
};

TYPEDEF_AUTOPTR(BSplineVec3d);
