////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushface.cpp
//  Version:     v1.00
//  Created:     8/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Based on Andrey's Indoor editor.
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BrushFace.h"

#include "BrushPlane.h"
#include "BrushPoly.h"
#include "Brush.h"
#include "BrushMtl.h"

#include <CREPolyMesh.h>

// Local namespace.
namespace
{
	//! Creates boundary plane.
	inline void CreateBoundaryPlane( SBrushPlane *src, Vec3d& p1, Vec3d& p2, SBrushPlane *dst )
	{
		Vec3d vv;

		vv = p2 - p1;
		dst->normal = src->normal ^ vv;
		dst->normal.Normalize();
		dst->dist = dst->normal | p1;
	}

  //static Vec3d m_TexVecs[2];
  //static float m_TexShift[2];

}

//////////////////////////////////////////////////////////////////////////
//
// SBrushFace implementation.
//
//////////////////////////////////////////////////////////////////////////
SBrushFace& SBrushFace::operator = (const SBrushFace& f)
{
  int i;

  m_Prefabs = NULL;
  for (i=0; i<3; i++)
  {
    m_PlanePts[i] = f.m_PlanePts[i];
  }
  m_TexInfo = f.m_TexInfo;
  m_Plane = f.m_Plane;
  
  if (f.m_Poly)
  {
    m_Poly = new SBrushPoly;
    *m_Poly = *f.m_Poly;
  }
  m_color = f.m_color;
  m_mtl = f.m_mtl;
  m_RE = NULL;

  return *this;
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::MakePlane()
{
	m_Plane.Make( m_PlanePts[0],m_PlanePts[1],m_PlanePts[2] );
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::TextureVectors(Vec3d& tVecx, Vec3d& tVecy, float& tShiftx, float& tShifty)
{
  Vec3d   pvecs[2];
  int     sv, tv;
  float   ang, sinv, cosv;
  float   ns, nt;
  int     i;
  SMapTexInfo  *td;

  td = &m_TexInfo;
  //e = m_shader;

  tVecx(0,0,0);
  tVecy(0,0,0);
  tShiftx = 0;
  tShifty = 0;

  if (!td->scale[0])
    td->scale[0] = 0.5;
  if (!td->scale[1])
    td->scale[1] = 0.5;

  m_Plane.CalcTextureAxis(pvecs[0], pvecs[1], 1);

  // rotate axis
  if (td->rotate == 0)
  {
    sinv = 0 ; cosv = 1;
  }
  else
  if (td->rotate == 90)
  {
    sinv = 1 ; cosv = 0;
  }
  else
  if (td->rotate == 180)
  {
    sinv = 0 ; cosv = -1;
  }
  else
  if (td->rotate == 270)
  {
    sinv = -1 ; cosv = 0;
  }
  else
  { 
    ang = td->rotate / 180 * gf_PI;
    sinv = sin(ang);
    cosv = cos(ang);
  }

  if (pvecs[0][0])
    sv = 0;
  else
  if (pvecs[0][1])
    sv = 1;
  else
    sv = 2;
        
  if (pvecs[1][0])
    tv = 0;
  else
  if (pvecs[1][1])
    tv = 1;
  else
    tv = 2;
          
  for (i=0 ; i<2 ; i++)
  {
    ns = cosv * pvecs[i][sv] - sinv * pvecs[i][tv];
    nt = sinv * pvecs[i][sv] +  cosv * pvecs[i][tv];
    if (!i)
    {
      tVecx[sv] = ns;
      tVecx[tv] = nt;
    }
    else
    {
      tVecy[sv] = ns;
      tVecy[tv] = nt;
    }
  }

  // scale
  tVecx /= td->scale[0];
  tVecy /= td->scale[1];

	// defaults.
	int texWidth = 32;
	int texHeight = 32;

	//@ASK Andrey.
	if (m_mtl)
	{
		ITexPic *tex = m_mtl->GetEditorTexture();
		if (tex)
		{
			texWidth = tex->GetWidth();
			texHeight = tex->GetHeight();
		}
	}
	// shift
	tShiftx = td->shift[0] / texWidth;
	tShifty = td->shift[1] / texHeight;

	tVecx /= texWidth;
	tVecy /= texHeight;
}

//////////////////////////////////////////////////////////////////////////
bool SBrushFace::ClipLine( Vec3d& p1,Vec3d& p2 )
{
  float d1, d2;

  d1 = p1.Dot(m_Plane.normal) - m_Plane.dist;
  d2 = p2.Dot(m_Plane.normal) - m_Plane.dist;

  if (d1 >= 0 && d2 >= 0)
    return false;

  if (d1 <= 0 && d2 <= 0)
    return true;

  float dt = d1 / (d1 - d2);

  if (d1 > 0)
    p1 = p1 + dt * (p2 - p1);
  else
    p2 = p1 + dt * (p2 - p1);

  return true;
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::CalcTexCoords( SBrushVert &v )
{
  Vec3d tVecx, tVecy;
  float tShiftx, tShifty;

  TextureVectors (tVecx, tVecy, tShiftx, tShifty);
  v.st[0] = (v.xyz.Dot(tVecx)) + tShiftx;
  v.st[1] = -((v.xyz.Dot(tVecy)) + tShifty);
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::DeletePrefabs(void)
{
	/*
  TPrefabItem *pi, *Next;

  for (pi=m_Prefabs; pi; pi=Next)
  {
    Next = pi->Next;
    delete pi;
  }
	*/
  m_Prefabs = NULL;
}

///////////////////////////////////////////////////////////////////////////
float SBrushFace::CalcArea()
{
  int nNumVertices=m_Poly->m_Pts.size();
  Vec3d *vTempVecs=new Vec3d [nNumVertices];

  for (int k=0;k<nNumVertices;k++)
  {
    SBrushVert vOrigVert=m_Poly->m_Pts[k];
    vTempVecs[k]=vOrigVert.xyz;
  } //k
  
	m_fArea=::CalcArea(vTempVecs,nNumVertices,m_Plane.normal);

  delete [] vTempVecs;

  return (m_fArea);
}

///////////////////////////////////////////////////////////////////////////
void SBrushFace::CalcCenter()
{
  m_vCenter(0,0,0);
  int nNumVertices=m_Poly->m_Pts.size();
  for (int k=0;k<nNumVertices;k++)
  {
    SBrushVert vOrigVert=m_Poly->m_Pts[k];
    m_vCenter+=vOrigVert.xyz;
  } //k

  m_vCenter=m_vCenter/(float)nNumVertices;
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::BuildPrefabs_r(SBrushPoly *p, Vec3d& Area, Vec3d& Scale)
{
  int i, i1, i2;
  Vec3d mins, maxs;
  Vec3d v, v1, v2;
  Vec3d vup, vright, vpn, org;
  SBrushPlane pl;
  SBrushPoly *front, *back;
  float size;

  if (!p)
    return;

  p->CalcBounds(mins, maxs);

  float a0 = Area[0];
  float a1 = Area[1];
  if (a0 < 2) a0 = 2;
  if (a1 < 2) a1 = 2;
  size = 99999.0f;
  i = -1;
  for (int n=0; n<p->m_Pts.size(); n++)
  {
    float s;
    v = p->m_Pts[n].xyz - mins;
    if ((s=v.Length()) < size)
    {
      i = n;
      size = s;
    }
  }
  if (!i)
    i1 = p->m_Pts.size()-1;
  else
    i1 = i-1;
  i2 = (i+1)%p->m_Pts.size();
  v = p->m_Pts[i].xyz;
  v1 = p->m_Pts[i1].xyz;
  v2 = p->m_Pts[i2].xyz;
  vup = v1 - v;
  vright = v2 - v;
  float si = vright.Length();
  if (si-a0 > 0.1f)
  {
    Vec3d sp1, sp2;
    sp1 = vright;
    sp1.Normalize();
    sp1 = v + sp1 * a0;
    sp2 = vup;
    sp2.Normalize();
    sp2 = sp1 + sp2 * a1;
    CreateBoundaryPlane(&m_Plane, sp1, sp2, &pl);
    p->ClipByPlane(&pl, 0.1f, &front, &back);
    BuildPrefabs_r(front, Area, Scale);
    BuildPrefabs_r(back, Area, Scale);
    if (front)
      delete front;
    if (back)
      delete back;
    return;
  }

  si = vup.Length();
  if (si-a1 > 0.1f)
  {
    Vec3d sp1, sp2;
    sp1 = vup;
    sp1.Normalize();
    sp1 = v + sp1 * a1;
    sp2 = vright;
    sp2.Normalize();
    sp2 = sp1 + sp2 * a0;
    CreateBoundaryPlane(&m_Plane, sp1, sp2, &pl);
    p->ClipByPlane(&pl, 0.1f, &front, &back);
    BuildPrefabs_r(front, Area, Scale);
    BuildPrefabs_r(back, Area, Scale);
    if (front)
      delete front;
    if (back)
      delete back;
    return;
  }

  SPrefabItem *pi = new SPrefabItem;
  //Timur[9/16/2002]pi->Next = NULL;
  //Timur[9/16/2002]pi->PrevLink =NULL;
  //Timur[9/16/2002]pi->Link(m_Prefabs);

	//Timur[7/3/2002]
	//@ASK Andrey.
	/*
  pi->m_Geom = (CREPrefabGeom *)m_shader->m_REs[0];

  CModelCgf *m = (CModelCgf *)pi->m_Geom->mModel;
	*/
  p->MakePlane(&pl);

  vpn = pl.normal;
  vup.Normalize();
  vright.Normalize();
  vright *= Scale[0];
  vup *= Scale[1];
  org = v;

  Matrix44 mat;
  mat.BuildFromVectors(vright, vup, vpn, org);
  pi->m_Matrix = mat;
}

//////////////////////////////////////////////////////////////////////////
SBrushFace::~SBrushFace()
{
  DeletePrefabs();
  if (m_Poly)
    delete m_Poly;

  if (m_RE)
  {
		ReleaseRE();
  }
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::BuildRE( const Matrix44 &worldTM )
{
  if (!m_Prefabs)
  {
		// Delete previous render element.
    if (m_RE)
      ReleaseRE();

    CREPolyMesh *re = (CREPolyMesh *)GetIEditor()->GetRenderer()->EF_CreateRE(eDATA_Poly);
    m_RE = re;
    //m_RE->m_DynMask = -1;
    re->m_Plane.n = m_Plane.normal;
    re->m_Plane.d = m_Plane.dist;
    re->Srf = this;
		
		//Timur[9/16/2002]
		/*
    SShader *eft = m_shader->mfGetTemplate(g_Globals.m_TemplateId);
    if (m_shader && gNE[eft->m_Id].m_TessSize)
    {
      m_bTesselated = true;
      TessSurf(re, this, gNE[eft->m_Id].m_TessSize);
    }
    else
		*/
    {
      //m_bTesselated = false;
			int numPts = m_Poly->m_Pts.size();
      re->NumVerts = numPts;
      re->TriVerts = new SMTriVert[numPts];
			re->NumIndices = (numPts-2)*3;
      re->Indices = new ushort[re->NumIndices];

      SBrushVert *v = &m_Poly->m_Pts[0];
      for (int i=re->NumVerts-1; i>=0; i--, v++)
      {
        re->TriVerts[i].vert = worldTM.TransformPointOLD(v->xyz);
        re->TriVerts[i].dTC[0] = v->st[0];
        re->TriVerts[i].dTC[1] = v->st[1];
        re->TriVerts[i].lmTC[0] = v->st[0];
        re->TriVerts[i].lmTC[1] = v->st[1];
      }
      ushort *vrtind = re->Indices;
      for (i=0; i<numPts-2; i++, vrtind+=3)
      {
        vrtind[0] = 0;
        vrtind[1] = i+1;
        vrtind[2] = i+2;
      }
    }
  }

	//Timur[9/16/2002]
	/*
  if (m_shader && m_shader->m_REs.size()!=0)
    m_shader->m_REs[0]->mfBuildGeometry(m_shader);
		*/
}

void SBrushFace::ReleaseRE()
{
	if (m_RE)
	{
		// Needed to delete pointers in same module.
		CREPolyMesh *re = (CREPolyMesh*)m_RE;
		if (re->TriVerts)
			delete []re->TriVerts;
		if (re->Indices)
			delete []re->Indices;

		re->TriVerts = 0;
		re->Indices = 0;
		re->NumVerts = 0;
		re->NumIndices = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::BuildPrefabs(void)
{
	//@ASK Andrey.

	/*
  DeletePrefabs();
  if (!m_shader || m_shader->m_REs.size()==0 || m_shader->m_REs[0]->mfGetType() != eDATA_Prefab)
    return;

  CREPrefabGeom *pg = (CREPrefabGeom *)m_shader->m_REs[0];
  CModelCgf *m = (CModelCgf *)pg->mModel;

  Vec3d Area = m->m_BBox.max - m->m_BBox.min;
  Vec3d Scale;
  Scale[0] = m_TexInfo.scale[0];
  Scale[1] = m_TexInfo.scale[1];
  Scale[2] = 1.0f;

  if (gcpCryIndEd->m_wndTextureBar.m_bAutoPrefab)
  {
    Vec3d v, v1, v2, vup, vright, mins, maxs;
    int i, i1, i2;
    SBrushPoly *p = m_Poly;
    float size = 99999.0f;
    p->CalcBounds(mins, maxs);
    float a0 = Scale[0] * Area[0];
    float a1 = Scale[1] * Area[1];
    i = -1;
    for (int n=0; n<p->m_Pts.size(); n++)
    {
      float s;
      v = p->m_Pts[n].xyz - mins;
      if ((s=v.Length()) < size)
      {
        i = n;
        size = s;
      }
    }
    if (!i)
      i1 = p->m_Pts.size()-1;
    else
      i1 = i-1;
    i2 = (i+1)%p->m_Pts.size();
    v = p->m_Pts[i].xyz;
    v1 = p->m_Pts[i1].xyz;
    v2 = p->m_Pts[i2].xyz;
    vup = v1 - v;
    vright = v2 - v;

    float s = vright.Length();
    float t = s / a0;
    int ti = (int)t;
    float tf = t - ti;
    if (tf > 0.5f)
      ti++;
    if (ti <= 0)
      ti = 1;
    Area[0] = s / ti;
    Scale[0] = Area[0] / (m->m_BBox.max.x - m->m_BBox.min.x);

    s = vup.Length();
    t = s / a1;
    ti = (int)t;
    tf = t - ti;
    if (tf > 0.5f)
      ti++;
    if (ti <= 0)
      ti = 1;
    Area[1] = s / ti;
    Scale[1] = Area[1] / (m->m_BBox.max.y - m->m_BBox.min.y);
  }
  else
  {
    Area[0] *= Scale[0];
    Area[1] *= Scale[1];
  }

  BuildPrefabs_r(m_Poly, Area, Scale);
	*/
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::ClipPrefab(SPrefabItem *pi)
{
	//@ASK Andrey.
	/*
  CComModel *fr, *md;

  if (pi->m_Model)
    md = pi->m_Model;
  else
    md = (CComModel *)pi->m_Geom->mModel;
  fr = md->mfClip(&m_Plane, pi->m_Matrix);
  if (fr)
  {
    if (pi->m_Model)
      delete md;
    pi->m_Model = fr;
  }
	*/
}

////////////////////////////////////////////////////////////////////
bool SBrushFace::IsIntersecting(const SBrushPlane &Plane)
{
  if (!m_Poly)
    return (false);

  bool  bFront=false;
  bool  bBack=false;
  for (int k=0;k<m_Poly->m_Pts.size();k++)
  {
    Vec3d Vert=m_Poly->m_Pts[k].xyz;

    float dist=(Plane.normal|Vert)-Plane.dist;
    if (dist>=0)
    {
      if (bBack)
        return (true);
      bFront=true;
    }
    else
    {
      if (bFront)
        return (true);
      bBack=true;
    }
  } //k

  return (false);
}

////////////////////////////////////////////////////////////////////
bool  SBrushFace::IsIntersecting(SBrush *Vol)
{
  for (int i=0; i<Vol->m_Faces.size(); i++)
  {   
    SBrushFace *f = Vol->m_Faces[i];

    //check if the face intersect the volume side's plane
    if (IsIntersecting(f->m_Plane))       
    {
      //if is it the case,check if the volume face intersect the face's plane 
      if (f->IsIntersecting(m_Plane))       
        return (true);
    }
      
  }//i  

  return (false);
}

//////////////////////////////////////////////////////////////////////////
void SBrushFace::FitTexture(int nX, int nY)
{
  SBrushPoly *w;
  Vec3d   mins,maxs;
  int i;
  float width, height;
  float rot_width, rot_height;
  float cosv,sinv,ang;
  float min_t, min_s, max_t, max_s;
  float s,t;
  Vec3d vecs[2];
  Vec3d   coords[4];
  SMapTexInfo *td;

  if (nY < 1)
    nY = 1;
  if (nX < 1)
    nX = 1;

  mins=SetMaxBB();
  maxs=SetMinBB();

  td = &m_TexInfo;
  w = m_Poly;
  if (!w)
    return;
  w->CalcBounds(mins, maxs);
  ang = td->rotate / 180 * gf_PI;
  sinv = sinf(ang);
  cosv = cosf(ang);

  m_Plane.CalcTextureAxis(vecs[0], vecs[1], 1);

  min_s = mins | vecs[0];
  min_t = mins | vecs[1];
  max_s = maxs | vecs[0];
  max_t = maxs | vecs[1];
  width = max_s - min_s;
  height = max_t - min_t;
  coords[0][0] = min_s;
  coords[0][1] = min_t;
  coords[1][0] = max_s;
  coords[1][1] = min_t;
  coords[2][0] = min_s;
  coords[2][1] = max_t;
  coords[3][0] = max_s;
  coords[3][1] = max_t;
  min_s = min_t = 99999;
  max_s = max_t = -99999;
  for (i=0; i<4; i++)
  {
    s = cosv * coords[i][0] - sinv * coords[i][1];
    t = sinv * coords[i][0] + cosv * coords[i][1];
    if (i&1)
    {
      if (s > max_s) 
        max_s = s;
    }
    else
    {
      if (s < min_s) 
        min_s = s;
      if (i<2)
      {
        if (t < min_t) 
          min_t = t;
      }
      else
      {
        if (t > max_t) 
          max_t = t;
      }
    }
  }
  rot_width =  (max_s - min_s);
  rot_height = (max_t - min_t);

	//@ASK Andrey.
	/*
	float temp;
  td->scale[0] = -(rot_width/((float)(gNE[m_shader->m_Id].m_Tex->m_Width*nX)));
  td->scale[1] = -(rot_height/((float)(gNE[m_shader->m_Id].m_Tex->m_Height*nY)));

  td->shift[0] = min_s/td->scale[0];
  temp = (int)(td->shift[0] / (gNE[m_shader->m_Id].m_Tex->m_Width*nX));
  temp = (temp+1)*gNE[m_shader->m_Id].m_Tex->m_Width*nX;
  td->shift[0] = (int)(temp - td->shift[0])%(gNE[m_shader->m_Id].m_Tex->m_Width*nX);

  td->shift[1] = min_t/td->scale[1];
  temp = (int)(td->shift[1] / (gNE[m_shader->m_Id].m_Tex->m_Height*nY));
  temp = (temp+1)*(gNE[m_shader->m_Id].m_Tex->m_Height*nY);
  td->shift[1] = (int)(temp - td->shift[1])%(gNE[m_shader->m_Id].m_Tex->m_Height*nY);
	*/
}