#ifndef primitives_h
#define primitives_h

namespace primitives {

////////////////////////// primitives //////////////////////

	struct primitive {
	};

	struct box : primitive {
		enum entype { type=0 };
		matrix3x3f Basis;
		int bOriented;
		vectorf center;
		vectorf size;
	};

	struct triangle : primitive {
		enum entype { type=1 };
		vectorf pt[3];
		vectorf n;
	};

	struct indexed_triangle : triangle {
		int idx;
	};

	struct grid : primitive {
		matrix3x3f Basis;
		int bOriented;
		vectorf origin;
		vector2df step,stepr;
		vector2di size;
		vector2di stride;

		int inrange(int ix, int iy) {	return isneg(ix-size.x) & isnonneg(ix) & isneg(iy-size.y) & isnonneg(iy); }
		int getcell_safe(int ix,int iy) { int mask=-inrange(ix,iy); return iy*stride.y+ix*stride.x&mask | size.x*size.y&~mask; }
	};

	struct heightfield : grid {
		enum entype { type=2 };
		heightfield& operator=(const heightfield &src) {
			step = src.step; stepr = src.stepr;
			size = src.size; stride = src.stride;
			pdata = src.pdata; heightscale = src.heightscale;
			pflags = src.pflags; typemask = src.typemask;
			typehole = src.typehole; typepower = src.typepower;
			return *this;
		}

		float getheight(int ix,int iy) const { return getheight(ix*stride.x+iy*stride.y); }
		int gettype(int ix,int iy) const { return gettype(ix*stride.x+iy*stride.y); }
		float getheight(int icell) const { return pdata[icell]*heightscale; }
		int gettype(int icell) const { 
			int itype=pflags[icell]>>typepower & typemask, idelta=itype-typehole;
			return itype | ((idelta-1)>>31 ^ idelta>>31);
		}

		unsigned short *pdata;
		float heightscale;
		unsigned short *pflags;
		unsigned short typemask;
		int typehole;
		int typepower;
	};

	struct ray : primitive {
		enum entype { type=3 };
		vectorf origin;
		vectorf dir;
	};

	struct sphere : primitive {
		enum entype { type=4 };
		vectorf center;
		float r;
	};

	struct cylinder : primitive {
		enum entype { type=5 };
		vectorf center;
		vectorf axis;
		float r,hh;
	};

	struct plane : primitive {
		enum entype { type=6 };
		vectorf n;
		vectorf origin;
	};

	struct coord_plane : plane {
		vectorf axes[2];
	};
}

struct prim_inters {
	prim_inters() { minPtDist2=0.0f; }
	vectorf pt[2];
	vectorf n;
	unsigned char iFeature[2][2];
	float minPtDist2;
	short id[2];
	int iNode[2];
	vectorf *ptborder;
	int nborderpt,nbordersz;
	vectorf ptbest;
	int nBestPtVal;
};

struct contact {
	real t,taux;
	vectorf pt;
	vectorf n;
	unsigned int iFeature[2];
};

const int NPRIMS = 8;

///////////////////// geometry contact structures ///////////////////

struct geom_contact_area {
	enum entype { polygon,polyline };
	int type;
	int npt;
	int nmaxpt;
	float minedge;
	int *piPrim[2];
	int *piFeature[2];
	vectorf *pt;
	vectorf n1; // normal of other object surface (or edge)
};

struct geom_contact {
	real t;
	vectorf pt;
	vectorf n;
	vectorf dir; // unprojection direction
	int iUnprojMode;
	float vel; // original velocity along this direction, <0 if least squares normal was used
	int id[2]; // external ids for colliding geometry parts
	int iPrim[2];
	int iFeature[2];
	int iNode[2]; // BV-tree nodes of contacting primitives
	vectorf *ptborder; // intersection border
	int nborderpt;
	vectorf center;
	bool bBorderConsecutive;
	geom_contact_area *parea;
};

#endif