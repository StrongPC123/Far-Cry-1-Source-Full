#include "stdafx.h"

#include "utils.h"
#include "primitives.h"
#include "overlapchecks.h"

COverlapChecker g_Overlapper;

COverlapChecker::COverlapChecker() 
{
	for(int i=0;i<NPRIMS;i++) for(int j=0;j<NPRIMS;j++)
		table[i][j] = default_overlap_check;
	table[box::type][box::type] = (overlap_check)box_box_overlap_check;
	table[box::type][triangle::type] = (overlap_check)box_tri_overlap_check;
	table[triangle::type][box::type] = (overlap_check)tri_box_overlap_check;
	table[box::type][heightfield::type] = (overlap_check)box_heightfield_overlap_check;
	table[heightfield::type][box::type] = (overlap_check)heightfield_box_overlap_check;
	table[box::type][ray::type] = (overlap_check)box_ray_overlap_check;
	table[ray::type][box::type] = (overlap_check)ray_box_overlap_check;
	table[box::type][sphere::type] = (overlap_check)box_sphere_overlap_check;
	table[sphere::type][box::type] = (overlap_check)sphere_box_overlap_check;
	table[triangle::type][sphere::type] = (overlap_check)tri_sphere_overlap_check;
	table[sphere::type][triangle::type] = (overlap_check)sphere_tri_overlap_check;
	table[sphere::type][sphere::type] = (overlap_check)sphere_sphere_overlap_check;
	table[heightfield::type][sphere::type] = (overlap_check)heightfield_sphere_overlap_check;
	table[sphere::type][heightfield::type] = (overlap_check)sphere_heightfield_overlap_check;
}

int default_overlap_check(const primitive*, const primitive *)
{
	return 1;
}


int box_box_overlap_check(const box *box1, const box *box2)
{
	int i;
	matrix3x3RMf &Basis21((matrix3x3RMf&)*(matrix3x3RMf*)g_Overlapper.Basis21);

	if ((box1->bOriented|box2->bOriented<<16)!=g_Overlapper.iPrevCode) {
		if (!box1->bOriented)
			Basis21 = box2->Basis.T();
		else if (box2->bOriented)
			Basis21 = box1->Basis*box2->Basis.T();
		else
			Basis21 = box1->Basis;
		for(i=0;i<9;i++) 
			g_Overlapper.Basis21abs[i] = fabsf(g_Overlapper.Basis21[i]);
		g_Overlapper.iPrevCode = box1->bOriented|box2->bOriented<<16;
	}

	vectorf center21 = box2->center-box1->center;
	if (box1->bOriented)
		center21 = box1->Basis*center21;
	const vectorf &a=box1->size, &b=box2->size;
	float t1,t2,t3,e=(a.x+a.y+a.z)*1e-4f;

	// node1 basis vectors
	if (fabsf(center21.x) > a.x+b*vectorf(g_Overlapper.Basis21abs+0))
		return 0;
	if (fabsf(center21.y) > a.y+b*vectorf(g_Overlapper.Basis21abs+3))
		return 0;
	if (fabsf(center21.z) > a.z+b*vectorf(g_Overlapper.Basis21abs+6))
		return 0;

	// node2 basis vectors
	if (fabsf(center21.x*g_Overlapper.Basis21[0]+center21.y*g_Overlapper.Basis21[3]+center21.z*g_Overlapper.Basis21[6]) > 
			a.x*g_Overlapper.Basis21abs[0]+a.y*g_Overlapper.Basis21abs[3]+a.z*g_Overlapper.Basis21abs[6]+b.x)
		return 0;
	if (fabsf(center21.x*g_Overlapper.Basis21[1]+center21.y*g_Overlapper.Basis21[4]+center21.z*g_Overlapper.Basis21[7]) > 
			a.x*g_Overlapper.Basis21abs[1]+a.y*g_Overlapper.Basis21abs[4]+a.z*g_Overlapper.Basis21abs[7]+b.y)
		return 0;
	if (fabsf(center21.x*g_Overlapper.Basis21[2]+center21.y*g_Overlapper.Basis21[5]+center21.z*g_Overlapper.Basis21[8]) > 
			a.x*g_Overlapper.Basis21abs[2]+a.y*g_Overlapper.Basis21abs[5]+a.z*g_Overlapper.Basis21abs[8]+b.z)
		return 0;

	// node1->axes[0] x node2->axes[0]
	t1 = a.y*g_Overlapper.Basis21abs[6] + a.z*g_Overlapper.Basis21abs[3];
	t2 = b.y*g_Overlapper.Basis21abs[2] + b.z*g_Overlapper.Basis21abs[1];
	t3 = center21.z*g_Overlapper.Basis21[3] - center21.y*g_Overlapper.Basis21[6];
	if (fabsf(t3) > t1+t2+e)
		return 0;

	// node1->axes[0] x node2->axes[1]
	t1 = a.y*g_Overlapper.Basis21abs[7] + a.z*g_Overlapper.Basis21abs[4];
	t2 = b.x*g_Overlapper.Basis21abs[2] + b.z*g_Overlapper.Basis21abs[0];
	t3 = center21.z*g_Overlapper.Basis21[4] - center21.y*g_Overlapper.Basis21[7];;
	if(fabsf(t3) > t1+t2+e)
		return 0;

	// node1->axes[0] x node2->axes[2]
	t1 = a.y*g_Overlapper.Basis21abs[8] + a.z*g_Overlapper.Basis21abs[5];
	t2 = b.x*g_Overlapper.Basis21abs[1] + b.y*g_Overlapper.Basis21abs[0];
	t3 = center21.z*g_Overlapper.Basis21[5] - center21.y*g_Overlapper.Basis21[8];
	if(fabsf(t3) > t1+t2+e)
		return 0;

	// node1->axes[1] x node2->axes[0]
	t1 = a.x*g_Overlapper.Basis21abs[6] + a.z*g_Overlapper.Basis21abs[0];
	t2 = b.y*g_Overlapper.Basis21abs[5] + b.z*g_Overlapper.Basis21abs[4];
	t3 = center21.x*g_Overlapper.Basis21[6] - center21.z*g_Overlapper.Basis21[0];
	if(fabsf(t3) > t1+t2+e)
		return 0;

	// node1->axes[1] x node2->axes[1]
	t1 = a.x*g_Overlapper.Basis21abs[7] + a.z*g_Overlapper.Basis21abs[1];
	t2 = b.x*g_Overlapper.Basis21abs[5] + b.z*g_Overlapper.Basis21abs[3];
	t3 = center21.x*g_Overlapper.Basis21[7] - center21.z*g_Overlapper.Basis21[1];
	if(fabsf(t3) > t1+t2+e)
		return 0;

	// node1->axes[1] x node2->axes[2]
	t1 = a.x*g_Overlapper.Basis21abs[8] + a.z*g_Overlapper.Basis21abs[2];
	t2 = b.x*g_Overlapper.Basis21abs[4] + b.y*g_Overlapper.Basis21abs[3];
	t3 = center21.x*g_Overlapper.Basis21[8] - center21.z*g_Overlapper.Basis21[2];
	if(fabsf(t3) > t1+t2+e)
		return 0;

	// node1->axes[2] x node2->axes[0]
	t1 = a.x*g_Overlapper.Basis21abs[3] + a.y*g_Overlapper.Basis21abs[0];
	t2 = b.y*g_Overlapper.Basis21abs[8] + b.z*g_Overlapper.Basis21abs[7];
	t3 = center21.y*g_Overlapper.Basis21[0] - center21.x*g_Overlapper.Basis21[3];
	if(fabsf(t3) > t1+t2+e)
		return 0;
	
	// node1->axes[2] x node2->axes[1]
	t1 = a.x*g_Overlapper.Basis21abs[4] + a.y*g_Overlapper.Basis21abs[1];
	t2 = b.x*g_Overlapper.Basis21abs[8] + b.z*g_Overlapper.Basis21abs[6];
	t3 = center21.y*g_Overlapper.Basis21[1] - center21.x*g_Overlapper.Basis21[4];
	if(fabsf(t3) > t1+t2+e)
		return 0;

	// node1->axes[2] x node2->axes[2]
	t1 = a.x*g_Overlapper.Basis21abs[5] + a.y*g_Overlapper.Basis21abs[2];
	t2 = b.x*g_Overlapper.Basis21abs[7] + b.y*g_Overlapper.Basis21abs[6];
	t3 = center21.y*g_Overlapper.Basis21[2] - center21.x*g_Overlapper.Basis21[5];
	if(fabsf(t3) > t1+t2+e)
		return 0;

	return 1;
}


int box_heightfield_overlap_check(const box *pbox, const heightfield *phf)
{
	box boxtr;
	vectorf vtx[4];
	triangle hftri;
	vector2df sz,ptmin,ptmax;
	int ix,iy,ix0,iy0,ix1,iy1,icell;
	float hmax;

	// find the 3 lowest vertices
	if (phf->bOriented) {
		if (pbox->bOriented)
			boxtr.Basis = pbox->Basis*phf->Basis.T();
		else
			boxtr.Basis = phf->Basis.T();
		boxtr.center = phf->Basis*(pbox->center-phf->origin);
	} else {
		boxtr.Basis = pbox->Basis;
		boxtr.center = pbox->center-phf->origin;
	}
	boxtr.bOriented = 1;
	boxtr.Basis.SetRow(0,boxtr.Basis.GetRow(0)*-sgnnz(boxtr.Basis(0,2)));
	boxtr.Basis.SetRow(1,boxtr.Basis.GetRow(1)*-sgnnz(boxtr.Basis(1,2)));
	boxtr.Basis.SetRow(2,boxtr.Basis.GetRow(2)*-sgnnz(boxtr.Basis(2,2)));
	boxtr.size = pbox->size;
	vtx[0] = pbox->size*boxtr.Basis;
	boxtr.Basis.SetRow(0,-boxtr.Basis.GetRow(0));	vtx[1]=pbox->size*boxtr.Basis; boxtr.Basis.SetRow(0,-boxtr.Basis.GetRow(0));
	boxtr.Basis.SetRow(1,-boxtr.Basis.GetRow(1));	vtx[2]=pbox->size*boxtr.Basis; boxtr.Basis.SetRow(1,-boxtr.Basis.GetRow(1));
	boxtr.Basis.SetRow(2,-boxtr.Basis.GetRow(2));	vtx[3]=pbox->size*boxtr.Basis; boxtr.Basis.SetRow(2,-boxtr.Basis.GetRow(2));

	// find the underlying grid rectangle
	sz.x = sz.y = 0;
	sz.x = max(sz.x, fabsf(vtx[1].x)); sz.y = max(sz.y, fabsf(vtx[1].y));
	sz.x = max(sz.x, fabsf(vtx[2].x)); sz.y = max(sz.y, fabsf(vtx[2].y));
	sz.x = max(sz.x, fabsf(vtx[3].x)); sz.y = max(sz.y, fabsf(vtx[3].y));
	ptmin.x = (boxtr.center.x-sz.x)*phf->stepr.x; ptmin.y = (boxtr.center.y-sz.y)*phf->stepr.y; 
	ptmax.x = (boxtr.center.x+sz.x)*phf->stepr.x; ptmax.y = (boxtr.center.y+sz.y)*phf->stepr.y;
	ix0 = float2int(ptmin.x-0.5f); iy0 = float2int(ptmin.y-0.5f); ix0 &= ~(ix0>>31); iy0 &= ~(iy0>>31);
	ix1 = min(float2int(ptmax.x+0.5f),phf->size.x); iy1 = min(float2int(ptmax.y+0.5f),phf->size.y);

	//if (ix0>ix1 || iy0>iy1 || phf->gettype(ix0,iy0)==-1) // now checks are in CTriMesh on per-tri basis, using materials
	//	return 0;
	vtx[0]+=boxtr.center;	vtx[1]+=boxtr.center;	vtx[2]+=boxtr.center;	vtx[3]+=boxtr.center;

	// check if all heightfield points are below the lowest box point
	for(ix=ix0,hmax=0;ix<=ix1;ix++) for(iy=iy0;iy<=iy1;iy++) 
		hmax = max(hmax,phf->getheight(ix,iy));
	if (hmax<vtx[0].z)
		return 0;

	if ((ix1-ix0)*(iy1-iy0)<=4) {
		// check for intersection with each underlying triangle
		for(ix=ix0;ix<ix1;ix++) for(iy=iy0;iy<iy1;iy++) {
			icell = ix*phf->stride.x+iy*phf->stride.y;
			hftri.pt[0].x=hftri.pt[2].x = ix*phf->step.x; hftri.pt[0].y=hftri.pt[1].y = iy*phf->step.y; 
			hftri.pt[1].x = hftri.pt[0].x+phf->step.x; hftri.pt[2].y = hftri.pt[0].y+phf->step.y;
			hftri.pt[0].z = phf->getheight(icell); hftri.pt[1].z = phf->getheight(icell+phf->stride.x); 
			hftri.pt[2].z = phf->getheight(icell+phf->stride.y);
			hftri.n = hftri.pt[1]-hftri.pt[0] ^ hftri.pt[2]-hftri.pt[0];
			if (box_tri_overlap_check(&boxtr,&hftri))
				return 1;
			hftri.pt[0] = hftri.pt[2]; hftri.pt[2].x += phf->step.x; hftri.pt[2].z = phf->getheight(icell+phf->stride.x+phf->stride.y);
			hftri.n = hftri.pt[1]-hftri.pt[0] ^ hftri.pt[2]-hftri.pt[0];
			if (box_tri_overlap_check(&boxtr,&hftri))
				return 1;
		}
		return 0;
	}
	return 1;
}

int heightfield_box_overlap_check(const heightfield *phf, const box *pbox) 
{
	return box_heightfield_overlap_check(pbox,phf);
}


int box_tri_overlap_check(const box *pbox, const triangle *ptri)
{
	vectorf pt[3],n;
	float l1,l2,l3,l,c;
	if (pbox->bOriented) {
		pt[0] = pbox->Basis*(ptri->pt[0]-pbox->center); 
		pt[1] = pbox->Basis*(ptri->pt[1]-pbox->center);	
		pt[2] = pbox->Basis*(ptri->pt[2]-pbox->center);
		n = pbox->Basis*ptri->n;
	} else {
		pt[0] = ptri->pt[0]-pbox->center; 
		pt[1] = ptri->pt[1]-pbox->center;	
		pt[2] = ptri->pt[2]-pbox->center;
		n = ptri->n;
	}

	// check box normals
	l1=fabsf(pt[0].x-pt[1].x); l2=fabsf(pt[1].x-pt[2].x); l3=fabsf(pt[2].x-pt[0].x); l=l1+l2+l3; // half length = l/4
	c = (l1*(pt[0].x+pt[1].x)+l2*(pt[1].x+pt[2].x)+l3*(pt[2].x+pt[0].x))*0.5f; // center = c/l
	if (fabsf(c) > (pbox->size.x+l*0.25f)*l) 
		return 0;
	l1=fabsf(pt[0].y-pt[1].y); l2=fabsf(pt[1].y-pt[2].y); l3=fabsf(pt[2].y-pt[0].y); l=l1+l2+l3;
	c = (l1*(pt[0].y+pt[1].y)+l2*(pt[1].y+pt[2].y)+l3*(pt[2].y+pt[0].y))*0.5f; 
	if (fabsf(c) > (pbox->size.y+l*0.25f)*l) 
		return 0;
	l1=fabsf(pt[0].z-pt[1].z); l2=fabsf(pt[1].z-pt[2].z); l3=fabsf(pt[2].z-pt[0].z); l=l1+l2+l3;
	c = (l1*(pt[0].z+pt[1].z)+l2*(pt[1].z+pt[2].z)+l3*(pt[2].z+pt[0].z))*0.5f; 
	if (fabsf(c) > (pbox->size.z+l*0.25f)*l) 
		return 0;

	// check triangle normal
	if (fabsf(n.x)*pbox->size.x+fabsf(n.y)*pbox->size.y+fabsf(n.z)*pbox->size.z < fabsf(n*pt[0]))
		return 0;

	vectorf edge,triproj1,triproj2;
	float boxproj;

	// check triangle edges - box edges cross products
	for(int i=0;i<3;i++) {
		edge = pt[inc_mod3[i]]-pt[i]; triproj1 = edge^pt[i]; triproj2 = edge^pt[dec_mod3[i]];
		boxproj = fabsf(pbox->size.y*edge.z) + fabsf(pbox->size.z*edge.y);
		if (fabsf((triproj1.x+triproj2.x)*0.5f) > boxproj+fabsf(triproj1.x-triproj2.x)*0.5f)
			return 0;
		boxproj = fabsf(pbox->size.x*edge.z) + fabsf(pbox->size.z*edge.x);
		if (fabsf((triproj1.y+triproj2.y)*0.5f) > boxproj+fabsf(triproj1.y-triproj2.y)*0.5f)
			return 0;
		boxproj = fabsf(pbox->size.x*edge.y) + fabsf(pbox->size.y*edge.x);
		if (fabsf((triproj1.z+triproj2.z)*0.5f) > boxproj+fabsf(triproj1.z-triproj2.z)*0.5f)
			return 0;
	}

	return 1;
}


int tri_box_overlap_check(const triangle *ptri, const box *pbox)
{
	return box_tri_overlap_check(pbox,ptri);
}

int box_ray_overlap_check(const box *pbox, const ray *pray)
{
	vectorf l,m,al;
	if (pbox->bOriented) {
		l = pbox->Basis*pray->dir*0.5f;
		m = pbox->Basis*(pray->origin-pbox->center)+l;
	} else {
		l = pray->dir*0.5f;
		m = pray->origin+l-pbox->center;
	}
	al.x=fabsf(l.x); al.y=fabsf(l.y); al.z=fabsf(l.z);

	// separating axis check for line and box
	if (fabsf(m.x) > pbox->size.x+al.x)
		return 0;
	if (fabsf(m.y) > pbox->size.y+al.y)
		return 0;
	if (fabsf(m.z) > pbox->size.z+al.z)
		return 0;

	if (fabsf(m.z*l.y-m.y*l.z) > pbox->size.y*al.z+pbox->size.z*al.y)
		return 0;
	if (fabsf(m.x*l.z-m.z*l.x) > pbox->size.x*al.z+pbox->size.z*al.x)
		return 0;
	if (fabsf(m.x*l.y-m.y*l.x) > pbox->size.x*al.y+pbox->size.y*al.x)
		return 0;

	return 1;
}

int ray_box_overlap_check(const ray *pray, const box *pbox)
{
	return box_ray_overlap_check(pbox,pray);
}


int box_sphere_overlap_check(const box *pbox, const sphere *psph)
{
	vectorf center=psph->center-pbox->center, dist;
	if (pbox->bOriented)
		center = pbox->Basis*center;
	dist.x = max(0.0f, fabsf(center.x)-pbox->size.x);
	dist.y = max(0.0f, fabsf(center.y)-pbox->size.y);
	dist.z = max(0.0f, fabsf(center.z)-pbox->size.z);
	return isneg(dist.len2()-sqr(psph->r));
}

int sphere_box_overlap_check(const sphere *psph, const box *pbox)
{
	return box_sphere_overlap_check(pbox,psph);
}


quotientf tri_sphere_dist2(const triangle* ptri, const sphere* psph, int& bFace)
{
	int i,bInside[2];
	float rvtx[3],elen2[2],denom;
	vectorf edge[2],dp;
	bFace = 0;

	rvtx[0] = (ptri->pt[0]-psph->center).len2();
	rvtx[1] = (ptri->pt[1]-psph->center).len2();
	rvtx[2] = (ptri->pt[2]-psph->center).len2();

	i = idxmin3(rvtx);
	dp = psph->center-ptri->pt[i];

	edge[0] = ptri->pt[inc_mod3[i]]-ptri->pt[i]; elen2[0] = edge[0].len2();
	edge[1] = ptri->pt[dec_mod3[i]]-ptri->pt[i]; elen2[1] = edge[1].len2();
	bInside[0] = isneg((dp^edge[0])*ptri->n);
	bInside[1] = isneg((edge[1]^dp)*ptri->n);
	rvtx[i] = rvtx[i]*elen2[bInside[0]] - sqr(max(0.0f,dp*edge[bInside[0]]))*(bInside[0]|bInside[1]); 
	denom = elen2[bInside[0]];

	if (bInside[0]&bInside[1]) {
		if (edge[0]*edge[1]<0) {
			edge[0] = ptri->pt[dec_mod3[i]]-ptri->pt[inc_mod3[i]];
			dp = psph->center-ptri->pt[inc_mod3[i]];
			if ((dp^edge[0])*ptri->n>0)
				return quotientf(rvtx[inc_mod3[i]]*edge[0].len2()-sqr(dp*edge[0]), edge[0].len2());
		}
		bFace = 1;
		return quotientf(sqr((psph->center-ptri->pt[0])*ptri->n),1.0f);
	}
	return quotientf(rvtx[i],denom);
}


int tri_sphere_overlap_check(const triangle *ptri, const sphere *psph)
{
	int bFace;
	return isneg(tri_sphere_dist2(ptri,psph,bFace)-sqr(psph->r));

	/*float r2=sqr(psph->r),edgelen2;
	vectorf edge,dp0,dp1;

	if ((ptri->pt[0]-psph->center).len2()<r2)
		return 1;
	if ((ptri->pt[1]-psph->center).len2()<r2)
		return 1;
	if ((ptri->pt[2]-psph->center).len2()<r2)
		return 1;

	edgelen2 = (edge = ptri->pt[1]-ptri->pt[0]).len2(); dp0 = ptri->pt[0]-psph->center; dp1 = ptri->pt[1]-psph->center;
	if (isneg((dp0^edge).len2()-r2*edgelen2) & isneg((dp0*edge)*(dp1*edge)))
		return 1;
	edgelen2 = (edge = ptri->pt[2]-ptri->pt[1]).len2(); dp0 = ptri->pt[1]-psph->center; dp1 = ptri->pt[2]-psph->center;
	if (isneg((dp0^edge).len2()-r2*edgelen2) & isneg((dp0*edge)*(dp1*edge)))
		return 1;
	edgelen2 = (edge = ptri->pt[0]-ptri->pt[2]).len2(); dp0 = ptri->pt[2]-psph->center; dp1 = ptri->pt[0]-psph->center;
	if (isneg((dp0^edge).len2()-r2*edgelen2) & isneg((dp0*edge)*(dp1*edge)))
		return 1;

	return 0;*/
}

int sphere_tri_overlap_check(const sphere *psph, const triangle *ptri)
{
	return tri_sphere_overlap_check(ptri,psph);
}


int sphere_sphere_overlap_check(const sphere *psph1, const sphere *psph2)
{
	return isneg((psph1->center-psph2->center).len2()-sqr(psph1->r+psph2->r));
}


int heightfield_sphere_overlap_check(const heightfield *phf, const sphere *psph)
{
	vectorf center;
	vector2di irect[2];
	int ix,iy,bContact=0;

	center = phf->Basis*(psph->center-phf->origin);
	irect[0].x = min(phf->size.x,max(0,float2int((center.x-psph->r)*phf->stepr.x-0.5f)));
	irect[0].y = min(phf->size.y,max(0,float2int((center.y-psph->r)*phf->stepr.y-0.5f)));
	irect[1].x = min(phf->size.x,max(0,float2int((center.x+psph->r)*phf->stepr.x+0.5f)));
	irect[1].y = min(phf->size.y,max(0,float2int((center.y+psph->r)*phf->stepr.y+0.5f)));

	for(ix=irect[0].x;ix<irect[1].x;ix++) for(iy=irect[0].y;iy<irect[1].y;iy++)
		bContact |= isneg(center.z-psph->r-phf->getheight(ix,iy));

	return bContact;
}

int sphere_heightfield_overlap_check(const sphere *psph, const heightfield *phf)
{
	return heightfield_sphere_overlap_check(phf,psph);
}