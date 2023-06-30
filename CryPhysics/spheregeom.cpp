#include "stdafx.h"

#include "utils.h"
#include "primitives.h"
#include "unprojectionchecks.h"
#include "bvtree.h"
#include "singleboxtree.h"
#include "geometry.h"
#include "spheregeom.h"


CSphereGeom* CSphereGeom::CreateSphere(sphere *psphere)
{
	m_sphere.center = psphere->center;
	m_sphere.r = psphere->r;

	box bbox;
	bbox.Basis.SetIdentity();
	bbox.center = m_sphere.center;
	bbox.size.Set(m_sphere.r,m_sphere.r,m_sphere.r);
	m_Tree.SetBox(&bbox);
	m_Tree.Build(this);
	m_minVtxDist = m_sphere.r*1E-4f;
	return this;
}


int CSphereGeom::CalcPhysicalProperties(phys_geometry *pgeom)
{ 
	pgeom->pGeom = this;
	pgeom->origin = m_sphere.center;
	pgeom->q.SetIdentity();
	pgeom->V = 4.0f/3*pi*cube(m_sphere.r);
	float x2 = sqr(m_sphere.r)*0.4f;
	pgeom->Ibody.Set(x2,x2,x2);
	return 1;
}


int CSphereGeom::PointInsideStatus(const vectorf &pt)
{
	return (isneg((pt-m_sphere.center).len2()-sqr(m_sphere.r))<<1)-1;
}


int CSphereGeom::PrepareForIntersectionTest(geometry_under_test *pGTest, CGeometry *pCollider,geometry_under_test *pGTestColl, bool bKeepPrevContacts)
{
	static short g_SphIdBuf[1];

	pGTest->pGeometry = this;
	pGTest->pBVtree = &m_Tree;
	m_Tree.PrepareForIntersectionTest(pGTest);

	pGTest->primbuf = pGTest->primbuf1 = g_SphBuf+g_SphBufPos++;
	pGTest->szprimbuf = 1;
	pGTest->typeprim = sphere::type;
	pGTest->szprim = sizeof(sphere);
	pGTest->idbuf = g_SphIdBuf;
	pGTest->surfaces = 0;
	pGTest->edges = 0;
	pGTest->minAreaEdge = 1E10f;
	return 1;
}


int CSphereGeom::PreparePrimitive(geom_world_data *pgwd,primitive *&pprim)
{
	sphere *psph = g_SphBuf+g_SphBufPos;
	g_SphBufPos = g_SphBufPos+1 & sizeof(g_SphBuf)/sizeof(g_SphBuf[0])-1;
	psph->center = pgwd->R*m_sphere.center*pgwd->scale + pgwd->offset;
	psph->r = m_sphere.r*pgwd->scale;
	pprim = psph;
	return sphere::type;
}


int CSphereGeom::GetPrimitiveList(int iStart,int nPrims, int typeCollider,primitive *pCollider,int bColliderLocal,
																		geometry_under_test *pGTest,geometry_under_test *pGTestOp, primitive *pRes,short *pResId)
{
	((sphere*)pRes)->center = pGTest->R*m_sphere.center*pGTest->scale + pGTest->offset;
	((sphere*)pRes)->r = m_sphere.r*pGTest->scale;
	pGTest->bTransformUpdated = 0; *pResId = -1;
	return 1;
}


int CSphereGeom::GetUnprojectionCandidates(int iop,const contact *pcontact, primitive *&pprim,int *&piFeature, geometry_under_test *pGTest)
{
	pprim = pGTest->primbuf1;
	((sphere*)pprim)->center = pGTest->R*m_sphere.center*pGTest->scale + pGTest->offset;
	((sphere*)pprim)->r = m_sphere.r*pGTest->scale;
	
	pGTest->idbuf[0] = -1;
	pGTest->nSurfaces = 0;
	pGTest->nEdges = 0;

	return 1;
}


int CSphereGeom::FindClosestPoint(geom_world_data *pgwd, int &iPrim,int &iFeature, const vectorf &ptdst0,const vectorf &ptdst1,
																	vectorf *ptres, int nMaxIters)
{
	vectorf center = pgwd->R*m_sphere.center*pgwd->scale + pgwd->offset; 
	float r = m_sphere.r*pgwd->scale;
	ptres[1] = ptdst0;
	if ((ptdst0-center).len2()<r*r) {
		ptres[0].Set(1E10f,1E10f,1E10f);
		return -1;
	}
	ptres[0] = center + (ptdst0-center).normalized()*r;
	return 1;
}


int CSphereGeom::UnprojectSphere(vectorf center,float r,float rsep, contact *pcontact)
{
	vectorf dc = center-m_sphere.center;
	if (dc.len2() > sqr(m_sphere.r+rsep))
		return 0;
	pcontact->n = dc.normalized();
	pcontact->t = dc*pcontact->n-m_sphere.r-r;
	pcontact->pt = m_sphere.center+pcontact->n*m_sphere.r;
	return 1;
}


float CSphereGeom::CalculateBuoyancy(const plane *pplane, const geom_world_data *pgwd, vectorf &massCenter)
{
	float r=m_sphere.r*pgwd->scale,x;
	vectorf n=pplane->n, center=pgwd->R*m_sphere.center+pgwd->offset;
	massCenter = center;
	x = (pplane->origin-center)*n;

	if (x>r)
		return 0;
	if (x<-r)
		return (4.0f/3)*pi*cube(r);
	massCenter += n*(pi*0.5f*(x*x*(r*r-x*x*0.5f)-r*r*r*r*0.5f)-x);
	return pi*((2.0f/3)*cube(r)+x*(r*r-x*x*(1.0f/3)));
}


void CSphereGeom::CalculateMediumResistance(const plane *pplane, const geom_world_data *pgwd, vectorf &dPres,vectorf &dLres)
{
	vectorf center,v,vn,axisx,axisy,n;
	float r,x,vxn,vxninv,nv,cx,ry,l,lx,ly,circ_S,circ_x,ell_S,ell_x,S;
	center = pgwd->R*m_sphere.center*pgwd->scale+pgwd->offset;
	r = m_sphere.r*pgwd->scale;	n = pplane->n;
	v = pgwd->v + (pgwd->w ^ center-pgwd->centerOfMass);
	x = (pplane->origin-center)*n;

	if (fabsf(x)>r) {
		dLres.zero(); dPres.zero();
		if (x>r)
			dPres = v*(-pi*r*r);
		return;
	}
	vn = v.normalized();
	axisy = vn^n; vxn = axisy.len();
	if (vxn<0.01f) {
		//axisy = vn.orthogonal().normalized(); l = sgnnz(x)*r;
		axisy = GetOrthogonal(vn).normalized(); l = sgnnz(x)*r;
	} else { // l - water plane - v+ plane intersection line along x
		axisy *= (vxninv=1.0f/vxn); l = x*vxninv;
	}
	axisx = axisy^vn;
	nv = n*v;
	cx = x*vxn;	// ellipse center along x
	ry = max(0.001f,sqrt_tpl(r*r-x*x)); // ellipse radius along y
	lx = max(-r*0.999f,min(r*0.999f,l)); ly = sqrt_tpl(r*r-lx*lx);
	circ_S = (lx*ly+r*r*(pi*0.5f+asin_tpl(lx/r)))*0.5f;
	circ_x = cube(ly)*(-1.0f/3);
	lx = max(-ry*0.999f,min(ry*0.999f,(l-cx)/max(0.001f,fabs_tpl(nv)))); ly = sqrt_tpl(ry*ry-lx*lx);
	ell_S = (lx*ly+ry*ry*(pi*0.5f+asin_tpl(lx/ry)*sgnnz(nv)))*fabs_tpl(nv)*0.5f;
	ell_x = cube(ly)*(-1.0f/3)*nv+cx;
	S = circ_S+ell_S;
	x = (circ_x*circ_S+ell_x*ell_S)/S;
	dPres = -v*S;
	center += axisx*x + vn*sqrt_tpl(max(0.0f,r*r-x*x));
	dLres = center-pgwd->centerOfMass ^ dPres;
}


void CSphereGeom::CalcVolumetricPressure(geom_world_data *gwd, const vectorf &epicenter,float k,float rmin, 
																				 const vectorf &centerOfMass, vectorf &P,vectorf &L)
{
	vectorf dc = gwd->R*m_sphere.center*gwd->scale+gwd->offset-epicenter;
	P += dc*(k/(dc.len()*max(dc.len2(),sqr(rmin))));
}


int CSphereGeom::DrawToOcclusionCubemap(const geom_world_data *pgwd, int iStartPrim,int nPrims, int iPass, int *pGrid[6],int nRes, 
																				float rmin,float rmax,float zscale)
{
	vectorf center=pgwd->R*m_sphere.center*pgwd->scale+pgwd->offset, axisx,axisy,pt[8];
	float x=m_sphere.r*pgwd->scale,y=0;
	const float step=1.0f/sqrt2;
	//axisx = center.orthogonal().normalized(); axisy = (axisx^center).normalized();
	axisx = GetOrthogonal(center).normalized(); axisy = (axisx^center).normalized();
	for(int i=0;i<8;i++) {
		pt[i] = center + axisx*x + axisy*y;
		x = (x-y)*(1.0f/sqrt2);
		y = x+y*sqrt2;
	}
	RasterizePolygonIntoCubemap(pt,8, iPass, pGrid,nRes, rmin,rmax,zscale);
	return 1;
}


void CSphereGeom::DrawWireframe(void (*DrawLineFunc)(float*,float*), geom_world_data *gwd, int iLevel)
{
	if (iLevel==0) {
		int i,ix,iy,iz;
		vectorf pt0,pt1;
		const float stepcos=cry_cosf(pi/8),stepsin=cry_sinf(pi/8);

		for(iz=0;iz<3;iz++) {
			ix = inc_mod3[iz]; iy = dec_mod3[iz];
			pt0[ix]=m_sphere.r*gwd->scale; pt0[iy]=pt0[iz]=pt1[iz]=0;
			for(i=0;i<16;i++) {
				pt1[ix] = pt0[ix]*stepcos-pt0[iy]*stepsin;
				pt1[iy] = pt0[ix]*stepsin+pt0[iy]*stepcos;
				DrawLineFunc(gwd->R*(pt0+m_sphere.center)+gwd->offset, gwd->R*(pt1+m_sphere.center)+gwd->offset);
				pt0 = pt1;
			}
		}
	}	else {
		BV *pbbox;
		ResetGlobalPrimsBuffers();
		m_Tree.GetNodeBV(gwd->R,gwd->offset,gwd->scale, pbbox);
		DrawBBox(DrawLineFunc,gwd, &m_Tree,(BBox*)pbbox,iLevel-1);
	}
}


void CSphereGeom::GetMemoryStatistics(ICrySizer *pSizer)
{
	pSizer->AddObject(this, sizeof(CSphereGeom));
}


void CSphereGeom::Save(CMemStream &stm)
{
	stm.Write(m_sphere);
	m_Tree.Save(stm);
}

void CSphereGeom::Load(CMemStream &stm)
{
	stm.Read(m_sphere);
	m_Tree.Load(stm,this);
}