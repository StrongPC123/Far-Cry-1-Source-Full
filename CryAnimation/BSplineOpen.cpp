#include "StdAfx.h"
#include "bsplineopen.h"

//////////////////////////////////////////////////////////////////////////
// retrives the boundaries of the influence of the given control point, in time:
// pTime[0] is the starting knot, pTime[1] is the end knot
// NOTE: end knot may be < start knot; it means the support interval is cyclic
void BSplineVec3d::getInfluenceInterval (int nControlPoint, int pTime[2])const
{
	assert (nControlPoint >= 0 && nControlPoint < numCPs());
	if (m_isOpen)
	{
		pTime[0] = nControlPoint - m_nDegree;
		pTime[1] = nControlPoint + 1;
	}
	else
	if (numKnots() - m_nDegree < 2)
	{
		pTime[0] = 0;
		pTime[1] = numKnots()-1;
	}
	else
	{
		pTime[0] = nControlPoint;
		pTime[1] = (nControlPoint + m_nDegree + 1) % (numKnots()-1);
	}
}

//////////////////////////////////////////////////////////////////////////
// get the d+1 derivative (delta) amplitude at knot k (so-called "comb" delta amplitude)
BSplineVec3d::Value BSplineVec3d::getDelta (int nKnot)
{
	assert (nKnot >= 1 && nKnot < numKnots()-1);

	Value ptResult = m_arrCPs[nKnot-1] * m_Knots.getDelta (nKnot-1, m_nDegree, nKnot+m_nDegree);
	for (int i = 0; i <= m_nDegree; ++i)
		ptResult += m_arrCPs[(nKnot+i)%numCPs()] * m_Knots.getDelta (nKnot + i, m_nDegree, nKnot+m_nDegree);

	return ptResult;
}

//////////////////////////////////////////////////////////////////
// number of unique knots passed, and the degree
// the array of control points is NOT initialized
BSplineVec3d::BSplineVec3d (int numKnots, int nDegree, bool isOpen):
	m_nDegree (nDegree),
	m_Knots (numKnots + 2 * nDegree),
	m_isOpen(isOpen),
	m_arrCPs ("BSplineVec3d.CPs")
{
	m_arrCPs.reinit(numCPs());
}

//////////////////////////////////////////////////////////////////
BSplineVec3d::~BSplineVec3d()
{
}


//////////////////////////////////////////////////////////////////
// Calculates spline value at the given time
BSplineVec3d::Value BSplineVec3d::getValue (Time fTime)const
{
	fixupTime(fTime);
	int nInterval = m_Knots.findInterval (fTime);

	if (m_isOpen)
	{
		if (nInterval < m_nDegree)
			return m_arrCPs[0];

		// NOTE: in the case of open spline, we guarantee here nInterval >= m_nDegree

		if (nInterval - m_nDegree >= numCPs())
			return m_arrCPs[numCPs()-1];
	}
	else
	{
		// as the result of clamping, ..
		assert (nInterval >= m_nDegree);
		assert (nInterval - m_nDegree <= numCPs()); // == only if the time is at the end of the loop
		if (nInterval - m_nDegree >= numCPs()) // cycle to the start of the loop
		{
			nInterval = m_nDegree;
			fTime = getKnotTime(0);
		}
	}

	// the index of the CP corresponding to the interval starting at nInterval knot
	int idxCP = m_isOpen ? nInterval : nInterval - m_nDegree;

	Value vCP = m_arrCPs[idxCP];
	BlendValue fBasis = m_Knots.getBasis (nInterval,m_nDegree, fTime, nInterval);
	Value ptResult = vCP * fBasis;
	for (int i = -1; i >= -m_nDegree; --i)
	{
		int nCurrentCP = idxCP+i;
		if (!m_isOpen)
		{	// the CP is cyclic in closed splines; in case the number of CPs is less than the degree,
			// we have to run this cycle (otherwise we could just make (nCurrentCP + numCPs()) % numCPs()
			assert (numCPs() > 0);
			while (nCurrentCP < 0)
				nCurrentCP += numCPs();
			nCurrentCP %= numCPs();
		}
		vCP = m_arrCPs[nCurrentCP];
		fBasis = m_Knots.getBasis (nInterval + i, m_nDegree, fTime, nInterval);
		ptResult += vCP * fBasis;
	}

	return ptResult;
}


//////////////////////////////////////////////////////////////////
// Sets the given knot time, maintaining boundary knot multiplicity
void BSplineVec3d::setKnotTime (int nKnot, Time fTime)
{
	assert (nKnot >= 0 && nKnot < numKnots());

	m_Knots[nKnot + m_nDegree] = fTime;
}

//////////////////////////////////////////////////////////////////////////
// call this after setting all knot times
void BSplineVec3d::finalizeKnotTimes()
{
	Time fStart = getKnotTime(0);
	Time fEnd   = getKnotTime(numKnots()-1);
	if (m_isOpen)
	{
		for (int i = 0; i < m_nDegree; ++i)
		{
			// multiplicity of the first knot is D
			m_Knots [i] = fStart;
			// .. and the last one, too
			m_Knots [m_nDegree + numKnots() + i] = fEnd;
		}
	}
	else
	{
		// this algorithm of filling in the extra knots also takes into account
		// the possibility of underdetermined knot vector
		for (int i = 1; i <= m_nDegree; ++i)
		{
			m_Knots[m_nDegree-i] = fStart - (fEnd - getKnotTime(numKnots()-1 - i));
      m_Knots[m_nDegree+numKnots()-1 + i] = fEnd + (getKnotTime(i)- fStart);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Retrieves the given knot time, maintaining boundary knot multiplicity
BSplineVec3d::Time BSplineVec3d::getKnotTime (int nKnot)const
{
	return m_Knots[nKnot + m_nDegree];
}


//////////////////////////////////////////////////////////////////
// retrives the basis function value for the given control point at the given time
BSplineVec3d::BlendValue BSplineVec3d::getBasis (int nControlPoint, Time fTime)const
{
	if (m_isOpen)
		return m_Knots.getBasis(nControlPoint, m_nDegree, fTime);
	else
	{
		fixupTime(fTime);
		return getBasis(nControlPoint+m_nDegree, fTime, m_Knots.findInterval(fTime));
	}
}

//////////////////////////////////////////////////////////////////////////
// Returns the basis function starting at knot i (m_Knot internal numeration)
// The function is calculated at time t, on the interval nIntervalT (m_Knot numeration)
BSplineVec3d::BlendValue BSplineVec3d::getBasis (int i, Time t, int nIntervalT) const
{
	if (m_isOpen)
		return m_Knots.getBasis (i,m_nDegree,t,nIntervalT);
	else
	{
		BlendValue fResult = m_Knots.getBasis (i,m_nDegree,t,nIntervalT);
		while (i+1 >= numKnots()) // the i is i+d actually
		{
			i = m_nDegree - (numKnots()-1 - (i - m_nDegree));
			if (i < 0)
				break;
			fResult += m_Knots.getBasis (i,m_nDegree,t,nIntervalT);					
		}
		return fResult;
	}
}

//////////////////////////////////////////////////////////////////////////
// Modifies the given time so that it fits inside the spline boundaries.
void BSplineVec3d::fixupTime (Time& fTime)const
{
	if (!m_isOpen)
	{
		float fTimeStart = getKnotTime (0);
		float fTimeEnd   = getKnotTime (numKnots()-1);
		if (fTime >= fTimeStart)
			fTime = fTimeStart + cry_fmod (fTime - fTimeStart, fTimeEnd-fTimeStart);
		else
			fTime = fTimeEnd - cry_fmod (fTimeStart - fTime, fTimeEnd - fTimeStart);
	}
}
