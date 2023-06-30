//////////////////////////////////////////////////////////////////////
//
//  CryEngine Source code
//	
//	File:BSplineknots.h
//  Declaration of class BSplineKnots
//
//	History:
//	06/17/2002 :Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////////////////////////////////
// class BSplineKnots
//
// This class represents the array of knots used to calculate
// basis functions of the given degree over the range
// [knot[degree] .. knot [N-degree]] , N being the total number of knots
// 
// NOTE:
//   order == degree+1
//   There are always N-degree-1 control points
//   This class incorporates neither control point nor degree info
//////////////////////////////////////////////////////////////

class BSplineKnots
{
public:
	// the abscissae and ordinates
	typedef float Time;
	typedef float Value;

	// standard scaling between Max and BSpline knot times: max time = g_nStdTicksPerSecond * bspline time
	//enum {g_nStdTicksPerSecond = 160};

	// allocates the array of the given number of knots, does NOT initialize the knots
	BSplineKnots(int numKnots);
	~BSplineKnots(void);

	Time& operator [] (int nKnot)
	{
		assert (nKnot >= 0 && nKnot < m_numKnots);
		return m_pKnots[nKnot];
	}

	Time operator [] (int nKnot) const
	{
		assert (nKnot >= 0 && nKnot < m_numKnots);
		return m_pKnots[nKnot];
	}

	// searches and returns the interval to which the given time belongs.
	// each pair of knots defines interval [k1,k2)
	// -1 is before the 0-th knot, numKnots() is after the last knot
	int findInterval (Time fTime)const;

	// number of knots
	int numKnots() const
	{
		return m_numKnots;
	}

	// returns the i-th basis function of degree d, given the time t
	Value getBasis (int i, int d, Time t) const;

	// returns the i-th basis function of degree d, given the time t and a hint, in which interval to search for it
	Value getBasis (int i, int d, Time t, int nIntervalT) const;

	// returns the time where the i-th given basis reaches its maximum
	Time getBasisPeak (int i/*nStartKnot*/, int d/*nDegree*/);

	// returns the i-th basis d-th derivative discontinuity at knot k
	// (amplitude of the delta function)
	Value getDelta (int i, int d, int k) const;

	// Returns the P (product) penalty for knot closeness, as defined by Mary J. Lindstrom "Penalized Estimation of Free Knot Splines" 1999,
	// logarithmic form
	double getKnotProductPenalty(int nStartKnot, int nEndKnot);
protected:
	// returns the value of the basis function of degree d, given the time t
	// the knots of the basis function start at pKnotBegin, and the first knot before t is pKnotBeforeT
	Value getBasis (const Time* pKnotBegin, int d, Time t, const Time* pKnotBeforeT)const;

	// number of knots
	int m_numKnots;

	// the array of knots
	// no assumption is made about the multiplicity of knots
	Time *m_pKnots;
};
