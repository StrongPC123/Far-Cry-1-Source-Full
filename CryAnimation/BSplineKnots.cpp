#include "StdAfx.h"
#include "BSplineKnots.h"

//////////////////////////////////////////////////////////////////////////
// allocates the array of the given number of knots, does NOT initialize the knots
BSplineKnots::BSplineKnots(int numKnots):
	m_numKnots (numKnots)
{
	m_pKnots = new Time [numKnots];
}

BSplineKnots::~BSplineKnots(void)
{
	delete []m_pKnots;
}

//////////////////////////////////////////////////////////////////////////
// searches and returns the interval to which the given time belongs.
// each pair of knots defines interval [k1,k2)
// -1 is before the 0-th knot, numKnots() is after the last knot
int BSplineKnots::findInterval (Time fTime)const
{
	return std::upper_bound (m_pKnots, m_pKnots + m_numKnots, fTime) - m_pKnots - 1;
}

//////////////////////////////////////////////////////////////////////////
// returns the i-th basis function of degree d, given the time t
BSplineKnots::Value BSplineKnots::getBasis (int i, int d, Time t) const
{
	// the requested basis must have defined supporting knots
	assert (i>= 0 && i + d + 1 < m_numKnots);
	
	// starting and ending knots - they demark the support interval: [*begin,*(end-1)]
	const Time* pKnotBegin = m_pKnots + i;
	const Time* pKnotEnd  = pKnotBegin + d + 2;
	// find the interval where the t is, among the supporting intervals
	// the upper bound of that interval is searched for
	const Time* pKnotAfterT = std::upper_bound (pKnotBegin, pKnotEnd, t);
	if (pKnotAfterT == pKnotBegin)
	{
		assert (t < pKnotBegin[0]);
		return 0; // the time t is before the supporting interval of the basis function
	}

	if (pKnotAfterT == pKnotEnd)
	{
		if (t > pKnotEnd[-1])
			return 0; // the time t is after the supporting interval

		assert (t == pKnotEnd[-1]);
		// scan down multiple knots
		while (t == pKnotAfterT[-1])
			--pKnotAfterT;
	}
	
	return getBasis (pKnotBegin,d,t,pKnotAfterT - 1);
}


//////////////////////////////////////////////////////////////////////////
// returns the i-th basis function of degree d, given the time t and a hint, in which interval to search for it
// the interval may be outside the supporting interval for the basis function
BSplineKnots::Value BSplineKnots::getBasis (int i, int d, Time t, int nIntervalT) const
{
	if (nIntervalT < 0 || nIntervalT >= m_numKnots)
		return 0;
	
	assert (t >= m_pKnots[nIntervalT] && t < m_pKnots[nIntervalT+1]);

	// the requested basis must have defined supporting knots
	if (i < 0 || i + d + 1 >= m_numKnots)
		return 0;

	if (nIntervalT < i || nIntervalT > i + d)
		return 0; // the time is outside supporting base
	
	return getBasis (m_pKnots + i, d,t,m_pKnots + nIntervalT);
}


//////////////////////////////////////////////////////////////////////////
// returns the value of the basis function of degree d, given the time t
// the knots of the basis function start at pKnotBegin, and the first knot before t is pKnotBeforeT
BSplineKnots::Value BSplineKnots::getBasis (const Time* pKnotBegin, int d, Time t, const Time* pKnotBeforeT)const
{
	assert (t >= pKnotBeforeT[0] && t <= pKnotBeforeT[1]);
	assert (pKnotBegin >= m_pKnots && pKnotBegin + d + 1 < m_pKnots + m_numKnots);
	assert (pKnotBeforeT >= pKnotBegin && pKnotBeforeT <= pKnotBegin + d);

	switch (d)
	{
	case 0:
		// trivial case
		return 1;
	case 1:
		if (pKnotBeforeT == pKnotBegin)
		{
			// the t is in the first interval
			return (t - pKnotBegin[0]) / (pKnotBegin[1] - pKnotBegin[0]);
		}
		else
		{
			assert (pKnotBeforeT == pKnotBegin + 1);
			return (pKnotBegin[2] - t) / (pKnotBegin[2] - pKnotBegin[1]);
		}
	default:
		{
			Value fResult = 0;
			if (pKnotBeforeT < pKnotBegin + d)
				fResult += getBasis (pKnotBegin, d-1, t, pKnotBeforeT) * (t - pKnotBegin[0]) / (pKnotBegin[d] - pKnotBegin[0]);
			if (pKnotBeforeT > pKnotBegin)
				fResult += getBasis (pKnotBegin+1, d-1, t, pKnotBeforeT) * (pKnotBegin[d+1] - t) / (pKnotBegin[d+1] - pKnotBegin[1]);
			return fResult;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// returns the time where the i-th given basis reaches its maximum
BSplineKnots::Time BSplineKnots::getBasisPeak (int i/*nStartKnot*/, int d/*nDegree*/)
{
	assert (i >= 0 && i < m_numKnots-d-1);
	if (d == 1)
		return m_pKnots[i+1];

	// plain search
	float fLeft = m_pKnots[i],  fRight = m_pKnots[i+d+1], fStep = (fRight-fLeft)/128.0f;
	float fBestTime = (fLeft + fRight)/2, fBestValue = getBasis (i, d, fBestTime);
	for (float t = fLeft; t < fRight; t += fStep)
	{
		Value fValue = getBasis (i, d, t);
		if (fValue > fBestValue)
		{
			fBestValue = fValue;
			fBestTime  = t;
		}
	}
	return fBestTime;
}


//////////////////////////////////////////////////////////////////////////
// Returns the P (product) penalty for knot closeness, as defined by Mary J. Lindstrom "Penalized Estimation of Free Knot Splines" 1999,
// logarithmic form the end knot must exist
double BSplineKnots::getKnotProductPenalty (int nStartKnot, int nEndKnot)
{
	if (nEndKnot - nStartKnot == 1)
	{
		return 0;
	}
	else
	{
		double fSum = (log(double(m_pKnots[nEndKnot]-m_pKnots[nStartKnot])) - log(double(nEndKnot-nStartKnot)))*(nEndKnot-nStartKnot);
		for (int nKnotInterval = nStartKnot; nKnotInterval < nEndKnot; ++nKnotInterval)
		{
			fSum -= log_tpl (m_pKnots[nKnotInterval + 1] - m_pKnots[nKnotInterval]);
		}

		assert (fSum > -1e-10);

		return max(0.0,fSum);
	}
}


//////////////////////////////////////////////////////////////////////////
// returns the i-th basis d-th derivative discontinuity at knot k
// (amplitude of the delta function)
// The result is undefined for non-existing delta functions
BSplineKnots::Value BSplineKnots::getDelta (int i, int d, int k) const
{
	// the support interval is [i..i+d+1]
	assert (k >= i && k <= i+d+1);

	double dResult = 1;
	int j;
	int nMultiplicity = 1;
	
	for (j = i; j < k; ++j)
	{
		double dt = m_pKnots[k] - m_pKnots[j];
		if (fabs(dt) > 1e-10)
			dResult *= dt;
		else
			++nMultiplicity;
	}
	// skip the k-th knot itself
	for (++j; j <= i+d+1; ++j)
	{
		double dt = m_pKnots[k] - m_pKnots[j];
		if (fabs(dt) > 1e-10)
			dResult *= dt;
		else
			++nMultiplicity;
	}

	// de facto, we can return something larger if there was a >1 multiplicity
	return /*nMultiplicity **/ Value((m_pKnots[i+d+1] - m_pKnots[i]) / dResult);
}
