#include "StdAfx.h"
#include "bsplinevec3dpacked.h"

// returns the normalized knot and the base - how many cycles the knot rolled back or forth
int PackedSplineClosedGetKnotTime(int &nKnot, int numKnots)
{
	int nBase = nKnot / (numKnots-1);
	nKnot = nKnot % (numKnots-1);
	if (nKnot < 0)
	{
		--nBase;
		nKnot += numKnots-1;
	}
	return nBase;
}
