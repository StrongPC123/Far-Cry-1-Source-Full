
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
//////////////////////////////////////////////////////////////////////

#ifndef FIRETYPE_H
#define FIRETYPE_H

enum eFireType
{
	ePressing = 0x01,
	eHolding = 0x02,
	eReleasing = 0x04,
	eCancel= 0x08,				// cancel current target
	eNotFiring = 0x10,
};

#endif