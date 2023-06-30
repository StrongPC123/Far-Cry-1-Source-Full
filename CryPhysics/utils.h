//////////////////////////////////////////////////////////////////////
//
//	Utils header
//	
//	File: utils.h
//	Description : Various utility functions definitions
//
//	History:
//	-:Created by Anton Knyazev
//
//////////////////////////////////////////////////////////////////////

#ifndef utils_h
#define utils_h

#include <math.h>

#include "Cry_Math.h"
using namespace primitives;

void ExtrudeBox(const primitives::box *pbox, const vectorf& dir,float step, primitives::box *pextbox);

#define USE_MATRIX_POOLS

#include "matrixnm.h"
#include "vectorn.h"
#include "quotient.h"
#include "polynomial.h"
#include "primitives.h"

const float fmag = 1.5f*(1<<23);

union floatint {
	float fval;
	int ival;
};

#if defined(WIN64) || defined(LINUX64)
const int	imag	= (23+127)<<23 | 1<<22; 
int float2int(float x);

#elif defined(LINUX32)
const int	imag	= (23+127)<<23 | 1<<22; 
inline int float2int(float x) {
	floatint u;
	u.fval = x+fmag;
	return u.ival-imag;
}

#else //WIN64

__declspec(naked) inline int64 GetTicks() { 
	__asm rdtsc 
	__asm ret
}

const int	imag	= -((23+127)<<23 | 1<<22);
// had to rewrite in assembly, since it's impossible to tell the compiler that in this case 
// addition is not associative, i.e. (x+0.5)+fmag!=x+(fmag+0.5)
__declspec(naked) inline int float2int(float x)
{ __asm {
	fld [esp+4]
	fadd fmag
	mov eax, imag
	fstp [esp-4]
	add eax, [esp-4]
	ret
}}

#endif //WIN64


int qhull(strided_pointer<vectorf> pts, int npts, index_t*& pTris);

struct ptitem2d {
	vector2df pt;
	ptitem2d *next,*prev;
	int iContact;
};
struct edgeitem {
	ptitem2d *pvtx;
	ptitem2d *plist;
	edgeitem *next,*prev;
	float area,areanorm2;
	edgeitem *next1,*prev1;
	int idx;
};
int qhull2d(ptitem2d *pts,int nVtx, edgeitem *edges);

real ComputeMassProperties(strided_pointer<const vectorf> points, const index_t *faces, int nfaces, vectorr &center,matrix3x3 &I);

template<class ftype,int stridei,int stridej> 
void OffsetInertiaTensor(Matrix33_tpl<ftype,stridei,stridej> &I,const Vec3_tpl<ftype> &center,ftype M)
{
	I(0,0) += M*(center.y*center.y + center.z*center.z);
	I(1,1) += M*(center.x*center.x + center.z*center.z);
	I(2,2) += M*(center.x*center.x + center.y*center.y);
	I(1,0) = I(0,1) = I(0,1)-M*center.x*center.y;
	I(2,0) = I(0,2) = I(0,2)-M*center.x*center.z;
	I(2,1) = I(1,2) = I(1,2)-M*center.y*center.z;
}

void ComputeMeshEigenBasis(strided_pointer<const vectorf> pVertices,const index_t *pIndices,int nTris, vectorr *eigen_axes,vectorr &center);

enum booltype { bool_intersect=1 };

int boolean2d(booltype type, vector2df *ptbuf1,int npt1, vector2df *ptbuf2,int npt2,int bClosed, vector2df *&ptbuf,int *&pidbuf);

real RotatePointToPlane(const vectorr &pt, const vectorr &axis,const vectorr &center, const vectorr &n,const vectorr &origin);

inline void LeftOffsetSpatialMatrix(matrixf &mtx, const vectorf &offset){
	matrix3x3f rmtx;
	matrix3x3in6x6f &A((matrix3x3in6x6f&)mtx.data[0]),&B((matrix3x3in6x6f&)mtx.data[3]),
									&C((matrix3x3in6x6f&)mtx.data[18]),&D((matrix3x3in6x6f&)mtx.data[21]);
	C += (crossproduct_matrix(offset, rmtx)*=A);
	D += (crossproduct_matrix(offset, rmtx)*=B);
}

inline void RightOffsetSpatialMatrix(matrixf &mtx, const vectorf &offset){
	matrix3x3f rmtx,tmtx;
	matrix3x3in6x6f &A((matrix3x3in6x6f&)mtx.data[0]),&B((matrix3x3in6x6f&)mtx.data[3]),
									&C((matrix3x3in6x6f&)mtx.data[18]),&D((matrix3x3in6x6f&)mtx.data[21]);
	A += ((tmtx=B)*=crossproduct_matrix(offset,rmtx));
	C += ((tmtx=D)*=rmtx);
}

inline void SpatialTranspose(matrixf &src, matrixf& dst) {
	int i,j;
	dst.nRows = src.nCols; dst.nCols = src.nRows;
	for(i=0;i<src.nRows;i++) for(j=0;j<3;j++) {
		dst[j][i] = src[i][j+3];
		dst[j+3][i] = src[i][j];
	}
}

template<class ftype> inline Vec3_tpl<ftype> cross_with_ort(const Vec3_tpl<ftype> &vec, int iz) {
	Vec3_tpl<ftype> res(zero);
	int ix=inc_mod3[iz], iy=dec_mod3[iz];
	res[iz]=0; res[ix]=vec[iy]; res[iy]=-vec[ix];
	return res;
}

void get_xqs_from_matrices(float *pMtx3x3,float *pMtx3x3T,float *pMtx4x4,float *pMtx4x4T, vectorf &pos,quaternionf &q,float &scale);

int BreakPolygon(vector2df *ptSrc,int nPt, int nCellx,int nCelly, int maxPatchTris, vector2df *&ptout,int *&nPtOut, 
								 float jointhresh=0.5f,int seed=-1);

void RasterizePolygonIntoCubemap(const vectorf *pt,int npt, int iPass, int *pGrid[6],int nRes, float rmin,float rmax,float zscale);

void GrowAndCompareCubemaps(int *pGridOcc[6],int *pGrid[6],int nRes, int nGrow, int &nCells,int &nOccludedCells);

void CalcMediumResistance(const vectorf *ptsrc,int npt, const vectorf& n, const plane &waterPlane,
													const vectorf &vworld,const vectorf &wworld,const vectorf &com, vectorf &P,vectorf &L);

int CoverPolygonWithCircles(strided_pointer<vector2df> pt,int npt,bool bConsecutive, const vector2df &center, vector2df *&centers,
														float *&radii, float minCircleRadius);

int ChoosePrimitiveForMesh(strided_pointer<const vectorf> pVertices,const index_t *pIndices,int nTris, const vectorr *eigen_axes,const vectorr &center, 
													 int flags, float tolerance, primitive *&pprim);
void ExtrudeBox(const box *pbox, const vectorf& dir,float step, box *pextbox);

template<class CellChecker> 
int DrawRayOnGrid(primitives::grid *pgrid, vectorf &origin,vectorf &dir, CellChecker &cell_checker) 
{
	int i,ishort,ilong,bStep,bPrevStep,ilastcell;
	float dshort,dlong,frac;
	quotientf tx[2],ty[2],t[2];
	vector2di istep,icell,idirsgn;

	// crop ray with grid bounds
	idirsgn.set(sgnnz(dir.x),sgnnz(dir.y));
	i = idirsgn.x;
	tx[1-i>>1].set(-origin.x*i, dir.x*i);
	tx[1+i>>1].set((pgrid->size.x*pgrid->step.x-origin.x)*i, dir.x*i);
	i = idirsgn.y;
	ty[1-i>>1].set(-origin.y*i, dir.y*i);
	ty[1+i>>1].set((pgrid->size.y*pgrid->step.y-origin.y)*i, dir.y*i);
	t[0] = max(t[0].set(0,1),max(tx[0],ty[0]));
	t[1] = min(t[1].set(1,1),min(tx[1],ty[1]));
	if (t[0]>=t[1])
		return 0;
	if (t[0]>0)
		origin += dir*t[0].val();
	if (t[0]>0 || t[1]<1)
		dir *= (t[1]-t[0]).val();

	ilong = isneg(fabs_tpl(dir.x)-fabs_tpl(dir.y));
	ishort = ilong^1;
	dshort = fabs_tpl(dir[ishort]); dlong = fabs_tpl(dir[ilong]);
	istep[ilong] = idirsgn[ilong];
	istep[ishort] = idirsgn[ishort];

	icell.set(float2int(origin.x*pgrid->stepr.x-0.5f), float2int(origin.y*pgrid->stepr.y-0.5f));
	ilastcell = float2int((origin.y+dir.y)*pgrid->stepr.y-0.5f)<<16 | float2int((origin.x+dir.x)*pgrid->stepr.x-0.5f);
	if (cell_checker.check_cell(icell,ilastcell) || (icell.y<<16|icell.x)==ilastcell)
		return 1;

	t[0].set((icell[ilong]+(istep[ilong]+1>>1))*pgrid->step[ilong]-origin[ilong], dir[ilong]);
	frac = (origin[ishort]+dir[ishort]*t[0].val() - icell[ishort]*pgrid->step[ishort])*pgrid->stepr[ishort];
	i = isneg(dir[ishort]);
	frac = i+frac*(1-(i<<1));
	if (frac>1.0f) {
		icell[ishort] += istep[ishort];
		if (cell_checker.check_cell(icell,ilastcell) || (icell.y<<16|icell.x)==ilastcell)
			return 1;
		frac -= 1;
	}
	frac *= dlong;
	icell[ilong] += istep[ilong];

	bPrevStep = 0;
	do {
		if (cell_checker.check_cell(icell,ilastcell) || (icell.y<<16|icell.x)==ilastcell)
			return 1;
		frac += dshort*(bPrevStep^1); bStep = isneg(dlong-frac);
		icell[ishort] += bStep*istep[ishort]; frac -= dlong*bStep;
		icell[ilong] += istep[ilong]&~-bStep; 
		bPrevStep = bStep;
	} while(true);

	return 0;
}


int bin2ascii(const unsigned char *pin,int sz, unsigned char *pout);
int ascii2bin(const unsigned char *pin,int sz, unsigned char *pout);

template<class T> inline T *_align16(T *ptr) { return (T*)(((INT_PTR)ptr-1&~15)+16); }
#if !defined(LINUX)
	template<class dtype> bool is_valid(const dtype &op) { return is_valid(op|op); }

	inline bool is_valid(float op) { return op*op>=0 && op*op<1E30f; }
	inline bool is_valid(int op) { return true; }
	inline bool is_valid(unsigned int op) { return true; }
#endif

void WritePacked(CStream &stm, int num);
void WritePacked(CStream &stm, uint64 num);
void ReadPacked(CStream &stm,int &num);
void ReadPacked(CStream &stm,uint64 &num);
void WriteCompressedPos(CStream &stm, const vectorf &pos, bool bCompress=true);
void ReadCompressedPos(CStream &stm, vectorf &pos, bool &bWasCompressed);
vectorf CompressPos(const vectorf &pos);
const int CMP_QUAT_SZ = 49;
void WriteCompressedQuat(CStream &stm, const quaternionf &q);
void ReadCompressedQuat(CStream &stm,quaternionf &q);
quaternionf CompressQuat(const quaternionf &q);
const int CMP_VEL_SZ = 48;
inline void WriteCompressedVel(CStream &stm, const vectorf &vel, const float maxvel) {
	stm.Write((short)max(-32768,min(32767,float2int(vel.x*(32767.0f/maxvel)))));
	stm.Write((short)max(-32768,min(32767,float2int(vel.y*(32767.0f/maxvel)))));
	stm.Write((short)max(-32768,min(32767,float2int(vel.z*(32767.0f/maxvel)))));
}
inline void ReadCompressedVel(CStream &stm, vectorf &vel, const float maxvel) {
	short i;
	stm.Read(i); vel.x = i*(maxvel/32767.0f);
	stm.Read(i); vel.y = i*(maxvel/32767.0f);
	stm.Read(i); vel.z = i*(maxvel/32767.0f);
}
inline vectorf CompressVel(const vectorf &vel, const float maxvel) {
	vectorf res;
	res.x = max(-32768,min(32767,float2int(vel.x*(32767.0f/maxvel)))) * (maxvel/32767.0f);
	res.y = max(-32768,min(32767,float2int(vel.y*(32767.0f/maxvel)))) * (maxvel/32767.0f);
	res.z = max(-32768,min(32767,float2int(vel.z*(32767.0f/maxvel)))) * (maxvel/32767.0f);
	return res;
}
inline void WriteCompressedFloat(CStream &stm, float f, const float maxval,const int nBits) {
	const int imax = (1<<nBits-1)-1;
	stm.WriteNumberInBits(max(-imax-1,min(imax,float2int(f*(imax/maxval)))),nBits);
}
inline void ReadCompressedFloat(CStream &stm, float &f, const float maxval,const int nBits) {
	const int imax = (1<<nBits-1)-1;
	unsigned int num;
	stm.ReadNumberInBits(num,nBits);
	f = ((int)num | ((int)num<<32-nBits & 1<<31)>>31-nBits)*(maxval/imax);
}
inline float CompressFloat(float f,const float maxval,const int nBits) {
	const int imax = (1<<nBits-1)-1;
	return max(-imax-1,min(imax,float2int(f*(imax/maxval)))) * (maxval/imax);
}


inline intptr_t iszero_mask(void *ptr) { return (intptr_t)ptr>>sizeof(intptr_t)*8-1 ^ (intptr_t)ptr-1>>sizeof(intptr_t)*8-1; }
template<class ftype> inline int AABB_overlap(Vec3_tpl<ftype> *BBox0,Vec3_tpl<ftype> *BBox1) {
	return isneg(fabs_tpl(BBox0[0].x+BBox0[1].x-BBox1[0].x-BBox1[1].x) - (BBox0[1].x-BBox0[0].x)-(BBox1[1].x-BBox1[0].x)) & 
				 isneg(fabs_tpl(BBox0[0].y+BBox0[1].y-BBox1[0].y-BBox1[1].y) - (BBox0[1].y-BBox0[0].y)-(BBox1[1].y-BBox1[0].y)) &
				 isneg(fabs_tpl(BBox0[0].z+BBox0[1].z-BBox1[0].z-BBox1[1].z) - (BBox0[1].z-BBox0[0].z)-(BBox1[1].z-BBox1[0].z));
}

extern int g_bitcount[256];
const int SINCOSTABSZ = 1024, SINCOSTABSZ_LOG2 = 10;
extern float g_costab[SINCOSTABSZ],g_sintab[SINCOSTABSZ];

// uncomment the following block to effectively disable validations
/*#define VALIDATOR_LOG(pLog,str)
#define VALIDATORS_START
#define VALIDATOR(member)
#define VALIDATOR_NORM(member)
#define VALIDATOR_RANGE(member,minval,maxval)
#define VALIDATOR_RANGE2(member,minval,maxval)
#define VALIDATORS_END
#define ENTITY_VALIDATE(strSource,pStructure)*/
#if defined(WIN64) || defined(LINUX64)
#define DoBreak {assert(0);}
#else
#define DoBreak { DEBUG_BREAK; }
#endif

#if defined LINUX
	#include "validator.h"
#else
	#define VALIDATOR_LOG(pLog,str) pLog->Log(str) //OutputDebugString(str)
	#define VALIDATORS_START bool validate( const char *strSource, ILog *pLog, const vectorf &pt,\
		IPhysicsStreamer *pStreamer, void *param0, int param1, int param2 ) { bool res=true; char errmsg[256];
	#define VALIDATOR(member) if (!is_unused(member) && !is_valid(member)) { \
		res=false; sprintf(errmsg,"\002%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid",strSource,\
			pStreamer->GetForeignName(param0,param1,param2),pt.x,pt.y,pt.z,#member); \
		VALIDATOR_LOG(pLog,errmsg); } 
	#define VALIDATOR_NORM(member) if (!is_unused(member) && !(is_valid(member) && fabs_tpl((member|member)-1.0f)<0.01f)) { \
		res=false; sprintf(errmsg,"\002%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid or unnormalized",\
		strSource,pStreamer->GetForeignName(param0,param1,param2),pt.x,pt.y,pt.z,#member); VALIDATOR_LOG(pLog,errmsg); }
	#define VALIDATOR_NORM_MSG(member,msg,member1) if (!is_unused(member) && !(is_valid(member) && fabs_tpl((member|member)-1.0f)<0.01f)) { \
		res=false; sprintf(errmsg,"\002%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid or unnormalized %s",\
		strSource,pStreamer->GetForeignName(param0,param1,param2),pt.x,pt.y,pt.z,#member,msg); \
		if (!is_unused(member1)) sprintf(errmsg+strlen(errmsg)," "#member1": %.1f,%.1f,%.1f",member1.x,member1.y,member1.z); \
		VALIDATOR_LOG(pLog,errmsg); }
	#define VALIDATOR_RANGE(member,minval,maxval) if (!is_unused(member) && !(is_valid(member) && member>=minval && member<=maxval)) { \
		res=false; sprintf(errmsg,"\002%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid or out of range",\
		strSource,pStreamer->GetForeignName(param0,param1,param2),pt.x,pt.y,pt.z,#member); VALIDATOR_LOG(pLog,errmsg); }
	#define VALIDATOR_RANGE2(member,minval,maxval) if (!is_unused(member) && !(is_valid(member) && member*member>=minval*minval && \
			member*member<=maxval*maxval)) { \
		res=false; sprintf(errmsg,"\002%s: (%.50s @ %.1f,%.1f,%.1f) Validation Error: %s is invalid or out of range",\
		strSource,pStreamer->GetForeignName(param0,param1,param2),pt.x,pt.y,pt.z,#member); VALIDATOR_LOG(pLog,errmsg); }
	#define VALIDATORS_END return res; }

	#define ENTITY_VALIDATE(strSource,pStructure) if (!pStructure->validate(strSource,m_pWorld->m_pLog,m_pos,\
		m_pWorld->m_pPhysicsStreamer,m_pForeignData,m_iForeignData,m_iForeignFlags)) { \
		if (m_pWorld->m_vars.bBreakOnValidation) DoBreak return 0; }
	#define ENTITY_VALIDATE_ERRCODE(strSource,pStructure,iErrCode) if (!pStructure->validate(strSource,m_pWorld->m_pLog,m_pos, \
		m_pWorld->m_pPhysicsStreamer,m_pForeignData,m_iForeignData,m_iForeignFlags)) { \
		if (m_pWorld->m_vars.bBreakOnValidation) DoBreak return iErrCode; }
#endif //LINUX

#endif