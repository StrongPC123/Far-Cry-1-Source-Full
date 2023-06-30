//////////////////////////////////////////////////////////////////////
//
//	Grid
//	
//	File: grid.h
//	Description : Grid class declaration and inlined implementation
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef grid_h
#define grid_h
#pragma once

#include "vector3d.h"
#include "vector2d.h"

struct grid {
	vector_tpl<int> ax;
	vectorf org;
	vector2df step,stepr;
	vector2d_tpl<unsigned int> sz;
	float parity;
	float scale;
	unsigned short *pflags;
	unsigned short holeflag;
	int holepower;

	int inrange(unsigned int ix, unsigned int iy) {
		return ix<sz.x && iy<sz.y;
	}
	int inrange(int ix, int iy) {
		return (unsigned int)ix<sz.x && (unsigned int)iy<sz.y;
	}
	int getcell(unsigned int ix, unsigned int iy) {
		return ix<sz.x && iy<sz.y ? iy*sz.x+ix : sz.x*sz.y;
	}
	int getcell(int ix,int iy) {
		return (unsigned int)ix<sz.x && (unsigned int)iy<sz.y ? iy*sz.x+ix : sz.x*sz.y;
	}
};

#endif