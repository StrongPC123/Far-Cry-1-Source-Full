////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brush.cpp
//  Version:     v1.00
//  Created:     8/7/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History: Based on Andrey's Indoor editor.
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Brush.h"

#include "BrushPlane.h"
#include "BrushPoly.h"
#include "BrushFace.h"
#include "Settings.h"
#include "Grid.h"
#include "list2.h"
#include "EdMesh.h"

#include <I3Dengine.h>

#define MIN_BOUNDS_SIZE 0.01f

#define SIDE_FRONT  0
#define SIDE_BACK   1

//////////////////////////////////////////////////////////////////////////
// SBrushSubSelection implementation.
//////////////////////////////////////////////////////////////////////////
bool SBrushSubSelection::AddPoint( Vec3 *pnt )
{
	if (std::find(points.begin(),points.end(),pnt) != points.end())
		return false;

	points.push_back(pnt);
	return true;
};

//////////////////////////////////////////////////////////////////////////
void SBrushSubSelection::Clear()
{
	points.clear();
}

SBrush::SBrush()
{
	m_flags = 0;
	m_prefabGeom = 0;
	m_indoorGeom = 0;
//	m_indoorGeom = GetIEditor()->Get3DEngine()->MakeObject();
}

//////////////////////////////////////////////////////////////////////////
//
// Brush implementations.
//
//////////////////////////////////////////////////////////////////////////
SBrush::~SBrush()
{
  ClearFaces();

	if (m_indoorGeom)
	{
		//m_indoorGeom->Release(); // May be will be deleted in Indoors.
		m_indoorGeom = 0;
	}

  //TBrush *b = (TBrush *)this;
  
  //b->Unlink();
  //UnlinkFromObj();
}

SBrush& SBrush::operator = (const SBrush& b)
{
	int i;

	m_bounds.min = b.m_bounds.min;
	m_bounds.max = b.m_bounds.max;

	for (i=0; i<m_Faces.size(); i++)
	{
		SBrushFace *f = m_Faces[i];
		delete f;
	}
	m_Faces.clear();
	for (i=0; i<b.m_Faces.size(); i++)
	{
		SBrushFace *f = new SBrushFace;
		m_Faces.push_back(f);
		*f = *b.m_Faces[i];
	}
	m_flags = b.m_flags;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
void SBrush::MakeFacePlanes()
{
  for (int i = 0; i < m_Faces.size(); i++)
  {
    m_Faces[i]->MakePlane();
  }
}

//////////////////////////////////////////////////////////////////////////
void SBrush::ClearFaces()
{
  for (int i = 0; i < m_Faces.size(); i++)
  {
    delete m_Faces[i];
  }
  m_Faces.clear();
}

//////////////////////////////////////////////////////////////////////////
float SBrush::GetVolume()
{
  int     i;
  SBrushPoly   *p;
  Vec3d   corner;
  float   d, area, volume;
  SBrushPlane  *plane;

  p = NULL;
  for (i=0; i<m_Faces.size(); i++)
  {
    p = m_Faces[i]->m_Poly;
    if (p)
      break;
  }
  if (!p)
    return 0;
  corner = p->m_Pts[0].xyz;

  volume = 0;
  for ( ; i<m_Faces.size(); i++)
  {
    p = m_Faces[i]->m_Poly;
    if (!p)
      continue;
    plane = &m_Faces[i]->m_Plane;
    d = -((corner | plane->normal) - plane->dist);
    area = p->Area();
    volume += d*area;
  }

  volume /= 3.0f;
  return volume;
}

//////////////////////////////////////////////////////////////////////////
void SBrush::ClipPrefabModels(bool bClip)
{
	//@ASK Andrey.
	/*
  SBrushFace *f;
  int i, j;
  TPrefabItem *pi;

  for (i=0; i<m_Faces.size(); i++)
  {
    f = m_Faces[i];
    for (pi=f->m_Prefabs; pi; pi=pi->Next)
    {
      if (!bClip)
      {
        if (pi->m_Model)
          delete pi->m_Model;
        pi->m_Model = NULL;
        continue;
      }
      if (!pi->m_Geom || !pi->m_Geom->mModel)
        continue;
      for (j=0; j<m_Faces.size(); j++)
      {
        if (j == i)
          continue;
        m_Faces[j]->ClipPrefab(pi);
      }
    }
  }
	*/
}

//////////////////////////////////////////////////////////////////////////
SBrushPoly*	SBrush::CreateFacePoly( SBrushFace *face )
{
  SBrushPoly *p;
  SBrushFace *clip;
  SBrushPlane plane;
  bool past;

  p = face->m_Plane.CreatePoly();

	int numFaces = m_Faces.size();

  past = false;
  for (int i = 0; i < numFaces; i++)
  {
    clip = m_Faces[i];
    if (!p)
      return p;
    if (clip == face)
    {
      past = true;
      continue;
    }
    if ((face->m_Plane.normal.Dot(clip->m_Plane.normal) > 0.999) &&
				(fabs(face->m_Plane.dist - clip->m_Plane.dist) < DIST_EPSILON))
    {
      if (past)
      {
        delete p;
        return NULL;
      }
      continue;
    }
    plane.Invert(&clip->m_Plane);
    p = p->ClipByPlane(&plane, false);
  }
  if (!p)
    return NULL;

  if (p->m_Pts.size() < 3)
  {
    delete p;
    p = NULL;
  }

	//@ASK Andrey.
  //p->m_Pts.Shrink();
  return p;
}

//////////////////////////////////////////////////////////////////////////
bool SBrush::BuildSolid( bool bRemoveEmptyFaces,bool bBuildRE )
{
	BBox box;
	box.Reset();

	int i,j;

	// Clear render element valid flag.
	m_flags &= ~BRF_RE_VALID;

	MakeFacePlanes();

  for (i = 0; i < m_Faces.size(); i++)
  {
    SBrushFace *f = m_Faces[i];
    
		if (f->m_Poly)
      delete f->m_Poly;
    
    f->m_Poly = CreateFacePoly(f);

		//@ASK Andrey
		/*
    f->m_Shader = gcMapEf.mfForName( f->m_TexInfo.name );
		*/

    if (!f->m_Poly)
      continue;

    // Build geometry tiles if needed
    f->BuildPrefabs();

		// Add this polygon to bounding box.
		int numPts = f->m_Poly->m_Pts.size();
    for (j=0; j < numPts; j++)
    {
			box.Add( f->m_Poly->m_Pts[j].xyz );
    }

    for (j=0; j < numPts; j++)
    {
			//@ASK Andrey.
      //f->CalcTexCoords(f->m_Poly->m_Pts[j], gNE[f->m_Shader->m_Id].m_Tex);
    }
  }
  if (bRemoveEmptyFaces)
    RemoveEmptyFaces();


	// Rebuild render elements.
	if (bBuildRE)
		BuildRenderElements();
  
	//@ASK Andrey
	/*
	ClipPrefabModels(gcpCryIndEd->m_wndTextureBar.m_bClipPrefab);
	*/
	m_bounds.min = box.min;
	m_bounds.max = box.max;

	// Check if valid.
	if (m_bounds.min.x > m_bounds.max.x ||
			m_bounds.min.y > m_bounds.max.y ||
			m_bounds.min.z > m_bounds.max.z)
	{
		return false;
	}
	if (m_Faces.empty())
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool SBrush::BuildRenderElements()
{
	//@ HACK
	m_flags |= BRF_RE_VALID;
	return true;

	if (m_indoorGeom && m_prefabGeom != NULL)
	{
		CLeafBuffer *srcBuf = m_prefabGeom->GetGeometry()->GetLeafBuffer();
		CLeafBuffer *trgBuf = m_indoorGeom->GetLeafBuffer();

		Vec3 omin = m_prefabGeom->GetGeometry()->GetBoxMin();
		Vec3 omax = m_prefabGeom->GetGeometry()->GetBoxMax();

		Vec3 bb = (m_bounds.max - m_bounds.min);
		Vec3 ob = (omax - omin);
		Vec3 scl;
		scl.x = bb.x / ob.x;
		scl.y = bb.y / ob.y;
		scl.z = bb.z / ob.z;

		Vec3 objofs = -(omax + omin)/2;
		objofs = Vec3( objofs.x*scl.x,objofs.y*scl.y,objofs.z*scl.z );

		Matrix44 stm;
		stm.SetIdentity();

		//SCALE_CHANGED_BY_IVO
		//stm.ScaleMatrix(scl.x,scl.y,scl.z);
		stm=Matrix33::CreateScale( Vec3d(scl.x,scl.y,scl.z) )*stm;

		stm.SetTranslationOLD( objofs );
		Matrix44 tm = stm * m_matrix;
//		tm.SetTranslation( m_matrix.GetTranslation() );

		int numv = srcBuf->m_SecVertCount;
		int stride1,stride2;
		unsigned char *src = srcBuf->GetPosPtr( stride1 );
		unsigned char *trg = trgBuf->GetPosPtr( stride2 );
		for (int i = 0; i < numv; i++)
		{
			Vec3 &sv = *(Vec3*)src;
			Vec3 &tv = *(Vec3*)trg;
			//Vec3 sv1 = stm.TransformPoint(sv);
			tv = tm.TransformPointOLD( sv );
			src += stride1;
			trg += stride2;
		}

		trgBuf->InvalidateVideoBuffer();
		m_flags |= BRF_RE_VALID;

		return true;
	}
	// Clear render element valid flag.
	m_flags &= ~BRF_RE_VALID;

	// Make leaf buffer.
	//CLeafBuffer *buf = GetIEditor()->GetRenderer()->CreateLeafBufferInitialized(
	//m_indoorGeom->MakeLeafBuffer( m_prefabGeom->GetTriData(),false );

	if (m_Faces.empty())
		return false;

	typedef struct_VERTEX_FORMAT_P3F_TEX2F VertexFormat;

	VertexFormat vtx;
	list2<VertexFormat> verts;
	list2<ushort> indices;

	int j;
	int baseIndex = 0;
	indices.reserve( 12*3 );
	verts.reserve( 12 );

	for (int i = 0; i < m_Faces.size(); i++)
  {
		SBrushFace *f = m_Faces[i];
		if (!f->m_Poly)
			continue;

		//m_bTesselated = false;
		int numPts = f->m_Poly->m_Pts.size();
		for (j = 0; j < numPts; j++)
		{
			SBrushVert *v = &f->m_Poly->m_Pts[numPts-j-1];
			Vec3 p = m_matrix.TransformPointOLD( v->xyz );
			vtx.xyz = p;
			vtx.st[0] = v->st[0];
			vtx.st[1] = v->st[1];
			verts.push_back( vtx );
		}
		for (j = 0; j < numPts-2; j++)
		{
			indices.push_back( baseIndex );
			indices.push_back( baseIndex+j+1 );
			indices.push_back( baseIndex+j+2 );
		}
		baseIndex += numPts;
  }

	IRenderer *renderer = GetIEditor()->GetRenderer();

	CLeafBuffer *pLeafBuffer = renderer->CreateLeafBufferInitialized( 
		&verts[0],verts.size(),VERTEX_FORMAT_P3F_TEX2F,&indices[0], indices.Count(), R_PRIMV_TRIANGLES,"Brush",eBT_Dynamic );
	assert( pLeafBuffer );
	CLeafBuffer *prevBuf = m_indoorGeom->GetLeafBuffer();
	if (prevBuf)
	{
		renderer->DeleteLeafBuffer(prevBuf);
	}

	SInputShaderResources rc;

	CMatInfo mt;
	mt.m_Id = 0;
  mt.nFirstIndexId = 0;
	mt.nNumIndices = indices.size();
  mt.nFirstVertId = 0;
  mt.nNumVerts = verts.size();
  mt.shaderItem = renderer->EF_LoadShaderItem( "Default",eSH_Indoor,true,NULL,0,&rc );
	mt.pRE = (CREOcLeaf*)renderer->EF_CreateRE(eDATA_OcLeaf);
	
	pLeafBuffer->m_pMats->Add(mt);

	mt.pRE->m_pBuffer = pLeafBuffer;
	mt.pRE->m_pChunk = &(*pLeafBuffer->m_pMats)[0];
	
		
	m_indoorGeom->SetLeafBuffer( pLeafBuffer );

	m_flags |= BRF_RE_VALID;

	/*
	for (int i = 0; i < m_Faces.size(); i++)
  {
    SBrushFace *f = m_Faces[i];
    if (f->m_Poly)
		{
			// Build render element for this face.
			f->BuildRE( GetMatrix() );
			// Set render element valid flag.
			m_flags |= BRF_RE_VALID;
		}
  }
	*/
	return m_flags & BRF_RE_VALID;
}

//////////////////////////////////////////////////////////////////////////
void SBrush::RemoveEmptyFaces()
{
  int i;
	std::vector<SBrushFace*> fc;

	// delete faces that doesnt contain polygon.
  for (i=0; i < m_Faces.size(); i++)
  {
    SBrushFace *f = m_Faces[i];
    if (f->m_Poly)
      fc.push_back(f);
    else
      delete f;
  }

	m_Faces.clear();

	// If less then 3 faces left, remove all faces.
  if (fc.size() < 3)
  {
    for (i = 0; i < fc.size(); i++)
      delete fc[i];
    return;
  }
	// Copy left brushes to brush face.
  m_Faces = fc;
}

//////////////////////////////////////////////////////////////////////////
void SBrush::FitTexture(int nX, int nY)
{
  int i;

  for (i=0; i<m_Faces.size(); i++)
  {
    m_Faces[i]->FitTexture(nX, nY);
  }
}

////////////////////////////////////////////////////////////////////
bool SBrush::IsIntersect(SBrush *Vol)
{    

  /*
  for (int k=0;k<m_Faces.size();k++)
  {
    SBrushFace *f = &m_Faces[k];
    if (f->IsIntersecting(Vol))
      return (true);          
  } //k

  return (false);
  */

  //return (true);
  
	int i;
  
  Vec3d Cent = (m_bounds.min + m_bounds.max) * 0.5f;

  for (i=0; i<Vol->m_Faces.size(); i++)
  {
    SBrushFace *f = Vol->m_Faces[i];
    if ((f->m_Plane.normal | Cent) > f->m_Plane.dist)
      break;
  }
  if (i == Vol->m_Faces.size())
    return true;

  return false;
  
}

//////////////////////////////////////////////////////////////////////////
bool SBrush::IsInside(const Vec3d &vPoint)
{
	for (int i=0;i<m_Faces.size();i++)
	{
    SBrushFace *f = m_Faces[i];
    if ((f->m_Plane.normal.Dot(vPoint)) > f->m_Plane.dist)
      return(false);
	} //i

	return(true);
}

//////////////////////////////////////////////////////////////////////////
bool SBrush::Intersect(SBrush *Vol, std::vector<SBrush*>& List)
{
  SBrush  *front, *back;
  int n = 0;
  int i;

  for (i=0 ; i<3 ; i++)
  {
    if (m_bounds.min[i] >= Vol->m_bounds.max[i] - DIST_EPSILON || m_bounds.max[i] <= Vol->m_bounds.min[i] + DIST_EPSILON)
      break;
  }
  if (i != 3)
    return false;


  SBrush *b = Vol;
  SBrush *Die = this;
  float vol;

  for (int j=0; j<b->m_Faces.size(); j++)
  {
    SBrushFace *f = b->m_Faces[j];
    SBrushFace *fc = new SBrushFace;
    *fc = *f;

    fc->m_PlanePts[0] += f->m_Plane.normal * 16;
    fc->m_PlanePts[1] += f->m_Plane.normal * 16;
    fc->m_PlanePts[2] += f->m_Plane.normal * 16;
    fc->MakePlane();
    for (i=0; i<Die->m_Faces.size(); i++)
    {
      for (int k=0; k<Die->m_Faces[i]->m_Poly->m_Pts.size(); k++)
      {
        if ((fc->m_Plane.normal | Die->m_Faces[i]->m_Poly->m_Pts[k].xyz) > fc->m_Plane.dist)
          break;
      }
      if (k!=Die->m_Faces[i]->m_Poly->m_Pts.size())
        break;
    }
    delete fc;
    if (i!=Die->m_Faces.size())
      break;
  }
  if (j == b->m_Faces.size())
  {
    List.push_back( Die );
    vol = Die->GetVolume();
    return true;
  }

  Vec3d Mins = m_bounds.min;
  Vec3d Maxs = m_bounds.max;

  for (j=0; j<b->m_Faces.size() && Die; j++)
  {
    SBrushFace *f = b->m_Faces[j];

    Die->SplitByFace(f, front, back);
    if (back)
      back->m_flags |= BRF_TEMP;
    if (front)
      delete front;

    if (Die->m_flags & BRF_TEMP)
      delete Die;
    Die = back;
  }
  if (Die)
  {
    SBrush *b = Die;
    List.push_back(b);
    vol = b->GetVolume();
    //gCurMap->m_ChoppedBrushes++;
    n++;
  }

  if (n)
    return true;
  return false;
}

//////////////////////////////////////////////////////////////////////////
int SBrush::GetMemorySize()
{
  int Size = sizeof(*this);
  for (int i=0; i<m_Faces.size(); i++)
  {
    Size += sizeof(SBrushFace);
    Size += sizeof(SBrushPoly);
    Size += sizeof(SBrushVert) * m_Faces[i]->m_Poly->m_Pts.size();
  }
  
  return Size;
}

//////////////////////////////////////////////////////////////////////////
void SBrush::SnapToGrid()
{
  SBrushFace *f;
  int i;

  for (int nf=0; nf<m_Faces.size(); nf++)
  {
    f = m_Faces[nf];
    for (i=0; i<3; i++)
    {
			f->m_PlanePts[i] = gSettings.pGrid->Snap( f->m_PlanePts[i] );
    }
  }
  BuildSolid();
}

//////////////////////////////////////////////////////////////////////////
void SBrush::SplitByFace (SBrushFace *f, SBrush* &front, SBrush* &back)
{
  SBrush  *b;
  SBrushFace   *nf;
  Vec3d   temp;
  SBrush *sb;

  b = new SBrush();

  sb = b;
  *sb = *this;
  nf = new SBrushFace;
  *nf = *f;

  nf->m_TexInfo = b->m_Faces[0]->m_TexInfo;
  b->m_Faces.push_back(nf);

  b->BuildSolid();
  b->RemoveEmptyFaces();
  if ( !b->m_Faces.size() )
  {
    delete b;
    back = NULL;
  }
  else
  {
		//@ASK Andrey.
		/*
    b->LinkToObj(m_Owner);
		*/
    back = b;
  }

  b = new SBrush;
  sb = b;
  *sb = *this;
  nf = new SBrushFace;
  *nf = *f;

  temp = nf->m_PlanePts[0];
  nf->m_PlanePts[0] = nf->m_PlanePts[1];
  nf->m_PlanePts[1] = temp;

  nf->m_TexInfo = b->m_Faces[0]->m_TexInfo;
  b->m_Faces.push_back(nf);

  b->BuildSolid();
  b->RemoveEmptyFaces ();
  if ( !b->m_Faces.size() )
  {
    delete b;
    front = NULL;
  }
  else
  {
		//@ASK Andrey.
		/*
    b->LinkToObj(m_Owner);
		*/
    front = b;
  }
}

int SBrush::OnSide (SBrushPlane *plane)
{
  int     i, j;
  SBrushPoly *p;
  float   d, max;
  int     side;

  max = 0;
  side = SIDE_FRONT;
  for (i=0;i<m_Faces.size(); i++)
  {
    p = m_Faces[i]->m_Poly;
    if (!p)
      continue;
    for (j=0; j<p->m_Pts.size(); j++)
    {
      d = (p->m_Pts[j].xyz | plane->normal) - plane->dist;
      if (d > max)
      {
        max = d;
        side = SIDE_FRONT;
      }
      if (-d > max)
      {
        max = -d;
        side = SIDE_BACK;
      }
    }
  }
  return side;
}

SBrush* SBrush::Clone( bool bBuildSoilid )
{
  SBrush *sb = new SBrush;
  *sb = *this;

  if (bBuildSoilid)
    sb->BuildSolid();

  return sb;
}

void SBrush::SplitByPlane (SBrushPlane *plane, SBrush* &front, SBrush* &back)
{
  SBrushPoly *tp = plane->CreatePoly();
  SBrushFace fc;
  ZeroStruct(fc);
  fc.m_PlanePts[0] = tp->m_Pts[0].xyz;
  fc.m_PlanePts[1] = tp->m_Pts[1].xyz;
  fc.m_PlanePts[2] = tp->m_Pts[2].xyz;
  SplitByFace(&fc, front, back);
  delete tp;
  return;
  
  SBrush *b[2];
  int     i, j;
  SBrushPoly *p, *cp[2], *midpoly;
  SBrushPlane   plane2;
  SBrushFace    *f;
  float   d, f_front, f_back;
  
  front = back = NULL;
  
  f_front = f_back = 0;
  for (i=0; i<m_Faces.size(); i++)
  {
    p = m_Faces[i]->m_Poly;
    if (!p)
      continue;
    for (j=0; j<p->m_Pts.size(); j++)
    {
      d = (p->m_Pts[j].xyz | plane->normal) - plane->dist;
      if (d > 0 && d > f_front)
        f_front = d;
      if (d < 0 && d < f_back)
        f_back = d;
    }
  }
  if (f_front < 0.1)
  {
    SBrush *nb = Clone(true);
    back = nb;
    return;
  }
  if (f_back > -0.1)
  { 
    SBrush *nb = Clone(true);
    front = nb;
    return;
  }
    
  p = plane->CreatePoly();
  for (i=0; i<m_Faces.size() && p; i++)
  {
    SBrushPlane *pp = &m_Faces[i]->m_Plane;
    plane2.Invert(pp);
    p = p->ClipByPlane(&plane2, true);
  }
  
  if (!p)
  { 
    int   side;
    
    side = OnSide (plane);
    if (side == SIDE_FRONT)
    {
      SBrush *nb = Clone(true);
      front = nb;
    }
    if (side == SIDE_BACK)
    {
      SBrush *nb = Clone(true);
      back = nb;
    }
    return;
  }
  
  midpoly = p;
  
  for (i=0; i<2; i++)
  {
    b[i] = Clone(false);
    for (j=0; j<b[i]->m_Faces.size(); j++)
    {
      delete b[i]->m_Faces[j];
    }
    b[i]->m_Faces.clear();
  }
    
  for (i=0; i<m_Faces.size(); i++)
  {
    f = m_Faces[i];
    p = f->m_Poly;
    if (!p)
      continue;
    p->ClipByPlane(plane, 0, &cp[0], &cp[1]);
    if (cp[0] == p)
    {
      cp[0] = new SBrushPoly;
      *(cp[0]) = *p;
    }
    if (cp[1] == p)
    {
      cp[1] = new SBrushPoly;
      *(cp[1]) = *p;
    }
    for (j=0; j<2; j++)
    {
      if (!cp[j])
        continue;
      SBrushFace *ff = new SBrushFace;
      *ff = *f;
      ff->m_Poly = cp[j];
      b[j]->m_Faces.push_back(ff);
    }
  }
    
  for (i=0; i<2; i++)
  {
    if (b[i]->m_Faces.size() < 3)
    {
      delete b[i];
      b[i] = NULL;
    }
  }
  
  if ( !(b[0] && b[1]) )
  {
    if (!b[0] && !b[1])
			CLogFile::WriteLine("Split removed brush\n");
    else
			CLogFile::WriteLine("Split not on both sides\n");
    if (b[0])
    {
      delete b[0];
      SBrush *nb = Clone(true);
      front = nb;
    }
    if (b[1])
    {
      delete b[1];
      SBrush *nb = Clone(true);
      back = nb;
    }
    return;
  }
  
  for (i=0; i<2; i++)
  {
    SBrushFace *ff = new SBrushFace;
    if (!i)
      ff->m_Plane = *plane;
    else
    {
      plane2.Invert(plane);
      ff->m_Plane = plane2;
    }
		// @FIXME ??
    //b[i]->CreateFacePoly( ff );
    ff->m_TexInfo = b[i]->m_Faces[0]->m_TexInfo;
    b[i]->m_Faces.push_back(ff);
    if (i==0)
    {
      SBrushPoly *pl = new SBrushPoly;
      *pl = *midpoly;
      ff->m_Poly = pl;
    }
    else
      ff->m_Poly = midpoly;
  }
    
  front = b[0];
  back = b[1];
}

/*
TBrush *SBrush::Parse()
{
  TBrush *b;
  SBrushFace  *f;
  int     i,j;
  
  gCurMap->m_ParsedBrushes++;
  b = new TBrush;

  do
  {
    if (!GetToken (true))
      break;
    if (!strcmp (token, "}") )
      break;
    
    if (strcmpi(token, "patchDef2") == 0 || strcmpi(token, "patchDef3") == 0)
    {
      delete b;
      
      b = SPatch::Parse(strcmpi(token, "patchDef2") == 0);
      if (b == NULL)
      {
        CLogFile::WriteLine ("Warning: parsing patch/brush\n");
        return NULL;
      }
      else
        continue;
    }
    else
    {      
      f = new SBrushFace;
      memset(f, 0, sizeof(SBrushFace));
      
      for (i=0; i<3; i++)
      {
        if (i != 0)
          GetToken (true);
        if (strcmp (token, "(") )
        {
          CLogFile::WriteLine("Warning: Parsing brush error\n");
          delete b;
          return NULL;
        }
        
        for (j=0 ; j<3 ; j++)
        {
          GetToken (false);
          f->m_PlanePts[i][j] = atof(token);
        }
        
        GetToken (false);
        if (strcmp (token, ")") )
        {
          CLogFile::WriteLine("Warning: Parsing brush error\n");
          delete b;
          return NULL;
        }
      }
    }

    {
      // read the TextureInfo
      GetToken (false);
      f->m_TexInfo.SetName(token);
      GetToken (false);
      f->m_TexInfo.shift[0] = atof(token);
      GetToken (false);
      f->m_TexInfo.shift[1] = atof(token);
      GetToken (false);
      f->m_TexInfo.rotate = atof(token); 
      GetToken (false);
      f->m_TexInfo.scale[0] = atof(token);
      GetToken (false);
      f->m_TexInfo.scale[1] = atof(token);
            
      // the flags and value field aren't necessarily present
      f->m_Shader = gcMapEf.mfForName( f->m_TexInfo.name );
      if (f->m_Shader->m_SM)
      {
        f->m_TexInfo.contents = f->m_Shader->m_SM->m_Contents;
        f->m_TexInfo.flags = f->m_Shader->m_SM->m_SurfFlags;
      }
      else
      {
        f->m_TexInfo.contents = 0;
        f->m_TexInfo.flags = 0;
      }
      
      if (TokenAvailable ())
      {
        GetToken (false);
        f->m_TexInfo.contents = atoi(token);
        GetToken (false);
        f->m_TexInfo.flags = atoi(token);
        GetToken (false);
        f->m_TexInfo.value = atoi(token);
      }
      if (!f->m_TexInfo.value)
        f->m_TexInfo.value = gcMapEf.mfGetVal(f->m_Shader, NULL);
      b->m_Faces.push_back(f);
      
    }
  } while (1);
  
  return b;
}

bool SBrush::ParseFromInd()
{
  gCurMap->m_ParsedBrushes++;
  GetToken(true);
  GetToken(true);
  if (token[0] != '[')
  {
    CLogFile::WriteLine("Error: Couldn't parsing IND file\n");
    return false;
  }
  while (true)
  {
    GetToken(true);
    if (token[0] == ']')
      break;
    if (!stricmp(token, "Face"))
    {
      SBrushFace *f = new SBrushFace;
      memset(f, 0, sizeof(SBrushFace));
      m_Faces.push_back(f);
      f->m_Poly = new SBrushPoly;
      GetToken(true);
      int nv = atoi(token);
      for (int i=0; i<nv; i++)
      {
        SBrushVert v;
        GetToken(true);
        for (int j=0; j<3; j++)
        {
          GetToken(true);
          v.xyz[j]  = atof(token);
        }
        for (j=0; j<2; j++)
        {
          GetToken(true);
          v.st[j]  = atof(token);
        }
        GetToken(true);

        f->m_Poly->m_Pts.push_back(v);

      }
      f->m_PlanePts[0] = f->m_Poly->m_Pts[0].xyz;
      f->m_PlanePts[1] = f->m_Poly->m_Pts[1].xyz;
      f->m_PlanePts[2] = f->m_Poly->m_Pts[2].xyz;

      // Parse TexInfo
      GetToken(true);
      GetToken(true);
      strcpy(f->m_TexInfo.name, token);
      GetToken(true);
      f->m_TexInfo.shift[0] = atof(token);
      GetToken(true);
      f->m_TexInfo.shift[1] = atof(token);
      GetToken(true);
      f->m_TexInfo.rotate = atof(token);
      GetToken(true);
      f->m_TexInfo.scale[0] = atof(token);
      GetToken(true);
      f->m_TexInfo.scale[1] = atof(token);
      GetToken(true);
      f->m_TexInfo.flags = atoi(token);
      GetToken(true);
    }
    else
    {
      CLogFile::WriteLine("Error: Couldn't parsing IND file\n");
      return false;
    }
  }

  return true;
}
*/

void SBrush::Create( const Vec3& mins, const Vec3& maxs, SMapTexInfo *ti)
{
  int   i, j;
  Vec3d pts[4][2];
  SBrushFace *f;

	ClearFaces();

  for (i=0 ; i<3 ; i++)
  {
    if (maxs[i] < mins[i])
      CLogFile::WriteLine ("Error: SBrush::Create: backwards");
  }

  pts[0][0][0] = mins[0];
  pts[0][0][1] = mins[1];
  
  pts[1][0][0] = mins[0];
  pts[1][0][1] = maxs[1];
  
  pts[2][0][0] = maxs[0];
  pts[2][0][1] = maxs[1];
  
  pts[3][0][0] = maxs[0];
  pts[3][0][1] = mins[1];
  
  for (i=0; i<4; i++)
  {
    pts[i][0][2] = mins[2];
    pts[i][1][0] = pts[i][0][0];
    pts[i][1][1] = pts[i][0][1];
    pts[i][1][2] = maxs[2];
  }

  for (i=0; i<4; i++)
  {
    f = new SBrushFace;
    ZeroStruct(*f);
    f->m_TexInfo = *ti;
    j = (i+1)%4;

    f->m_PlanePts[0] = pts[j][1];
    f->m_PlanePts[1] = pts[i][1];
    f->m_PlanePts[2] = pts[i][0];
    m_Faces.push_back(f);
  }
  
  f = new SBrushFace;
  ZeroStruct(*f);
  f->m_TexInfo = *ti;
  f->m_PlanePts[0] = pts[0][1];
  f->m_PlanePts[1] = pts[1][1];
  f->m_PlanePts[2] = pts[2][1];
  m_Faces.push_back(f);

  f = new SBrushFace;
  ZeroStruct(*f);
  f->m_TexInfo = *ti;
  f->m_PlanePts[0] = pts[2][0];
  f->m_PlanePts[1] = pts[1][0];
  f->m_PlanePts[2] = pts[0][0];
  m_Faces.push_back(f);
}

//===================================================================

void SBrush::MakeSelFace (SBrushFace *f)
{
	//@ASK Andrey.
	/*
  SBrushPoly *p;
  int   i;
  int   pnum[128];

  p = f->CreatePoly(this);
  if (!p)
    return;
  for (i=0; i<p->m_Pts.size(); i++)
  {
    pnum[i] = gCurMap->FindPoint(&p->m_Pts[i]);
  }
  for (i=0; i<p->m_Pts.size(); i++)
  {
    gCurMap->FindEdge (pnum[i], pnum[(i+1)%p->m_Pts.size()], f);
  }
  delete p;
	*/
}

SBrushFace* SBrush::Ray( Vec3d Origin, Vec3d dir, float *dist )
{
  SBrushFace *f, *FirstFace=0;
  Vec3d p1, p2;
  float frac, d1, d2;
  int   i, j;

  p1 = Origin;
  for (i=0 ; i<3 ; i++)
  {
    p2[i] = p1[i] + dir[i]*32768;
  }

  for (j=0; j<m_Faces.size(); j++)
  {
    f = m_Faces[j];

    d1 = (p1 | f->m_Plane.normal) - f->m_Plane.dist;
    d2 = (p2 | f->m_Plane.normal) - f->m_Plane.dist;
    if (d1 >= 0 && d2 >= 0)
    {
      *dist = 0;
      return NULL;
    }
    if (d1 <=0 && d2 <= 0)
      continue;
    frac = d1 / (d1 - d2);
    if (d1 > 0)
    {
      FirstFace = f;
      p1 = p1 + frac * (p2 - p1);
    }
    else
      p2 = p1 + frac * (p2 - p1);
  }

  p1 -= Origin;
  d1 = p1 | dir;

  *dist = d1;

  return FirstFace;
}

//////////////////////////////////////////////////////////////////////////
void SBrush::SelectSide( Vec3d Origin,Vec3d Dir,bool shear,SBrushSubSelection &subSelection )
{
  SBrushFace *f;
  Vec3d p1, p2;

	Vec3d Target = Origin + Dir * 10000.0f; 

  for (int j=0; j < m_Faces.size(); j++)
  {
    f = m_Faces[j];
    p1 = Origin;
    p2 = Target;

		// Clip ray by every face of brush, except tested one.
    for (int i=0; i < m_Faces.size(); i++)
    {
      if (m_Faces[i] != f)
				m_Faces[i]->ClipLine(p1, p2);
    }

    //if (i != m_Faces.size())
      //continue;

    if (IsEquivalent(p1,Origin,0))
      continue;

    if (f->ClipLine(p1, p2))
      continue;

    SelectFace( f,shear,subSelection );
  }
}

//////////////////////////////////////////////////////////////////////////
void SBrush::Move(Vec3d& Delta)
{
  int   i;
  SBrushFace *f;

  for (int j=0; j<m_Faces.size(); j++)
  {
    f = m_Faces[j];
    for (i=0; i<3; i++)
    {
      f->m_PlanePts[i] += Delta;
    }
  }
  BuildSolid();
}

//////////////////////////////////////////////////////////////////////////
void SBrush::Transform( Matrix44 &tm,bool bBuild )
{
	int   i;
  SBrushFace *f;

  for (int j=0; j<m_Faces.size(); j++)
  {
    f = m_Faces[j];
    for (i=0; i<3; i++)
    {
      f->m_PlanePts[i] = tm.TransformPointOLD(f->m_PlanePts[i]);
    }
  }
	if (bBuild)
		BuildSolid();
}

//////////////////////////////////////////////////////////////////////////
void SBrush::SetShader (SMapTexInfo *ti)
{
  for (int i=0; i<m_Faces.size(); i++)
  {
    m_Faces[i]->m_TexInfo = *ti;
  }
  BuildSolid();
}

/*
void SBrush::WriteToInd(FILE *f, char *st, int num)
{
  pfprintf (st, f, "Brush '%d'\n", num);
  pfprintf (st, f, "[\n");

  for (int i=0; i<m_Faces.size(); i++)
  {
    SBrushFace *fa = m_Faces[i];

    pfprintf (st, f, "  Face %d", fa->m_Poly->m_Pts.size());
    for (int j=0; j<fa->m_Poly->m_Pts.size(); j++)
    {
      SBrushVert *v = &fa->m_Poly->m_Pts[j];
      fprintf (f, " (");
      if (v->xyz[0] - (int)v->xyz[0])
        fprintf(f, " %f", v->xyz[0]);
      else
        fprintf(f, " %d", (int)v->xyz[0]);

      if (v->xyz[1] - (int)v->xyz[1])
        fprintf(f, " %f", v->xyz[1]);
      else
        fprintf(f, " %d", (int)v->xyz[1]);

      if (v->xyz[2] - (int)v->xyz[2])
        fprintf(f, " %f", v->xyz[2]);
      else
        fprintf(f, " %d", (int)v->xyz[2]);

      fprintf(f, " %f", v->st[0]);
      fprintf(f, " %f", v->st[1]);

      fprintf (f, " )");
    }
    SMapTexInfo *ti= &fa->m_TexInfo;
    fprintf(f, " ( %s %f %f %f %f %f %d )\n", ti->name, ti->shift[0], ti->shift[1], ti->rotate, ti->scale[0], ti->scale[1], ti->flags);
  }
  pfprintf (st, f, "]\n");
}

void SBrush::Select (void)
{
  TBrush *b;
  CMapObject *o;

  gCurMap->m_SelectedFace = NULL;
  if (gCurMap->m_SelectCount < 2)
    gCurMap->m_SelectOrder[gCurMap->m_SelectCount] = this;
  gCurMap->m_SelectCount++;

  o = m_Owner;
  if (o)
  {
    if (o != gCurMap->m_WorldObject)
    {
      for (b=gCurMap->m_SelectedBrushes; b; b=b->Next)
      {
        if (b->m_Owner == o)
          goto ss;
      }
      for (b=o->m_Brushes.m_NextInObj; b != &o->m_Brushes; b=b->m_NextInObj)
      {
        b->Unlink();
        b->Link(gCurMap->m_SelectedBrushes);
      }
    }
    else
    {
ss:
      b = (TBrush *)this;
      b->Unlink();
      b->Link(gCurMap->m_SelectedBrushes);
    }
  }
}

void SBrush::Deselect (void)
{
  TBrush *b;

  b = (TBrush *)this;
  b->Unlink();
  b->Link(gCurMap->m_ActiveBrushes);
}
*/

void SBrush::SelectFace( SBrushFace *f,bool shear,SBrushSubSelection &subSelection )
{
  int   i;
  SBrushFace *f2;
  SBrushPoly *p;
  float d;
  int   c;

  c = 0;
  for (i = 0; i<3; i++)
  {
    if (subSelection.AddPoint(&f->m_PlanePts[i]))
			c++;
  }
  if (c == 0)
    return;

	/*
	 TBrush *b2;
  for (b2=gCurMap->m_SelectedBrushes; b2; b2=b2->Next)
  {
    if (b2 == this)
      continue;
    for (int j=0; j<b2->m_Faces.size(); j++)
    {
      f2 = b2->m_Faces[j];
      for (i=0; i<3; i++)
      {
        if (fabs(f2->m_PlanePts[i] | f->m_Plane.normal) - f->m_Plane.dist > DIST_EPSILON)
          break;
      }
      if (i == 3)
      {
        b2->SelectFaceForDragging (f2, shear);
        break;
      }
    }
  }
	*/

  if (!shear)
    return;

  for (int j=0; j<m_Faces.size(); j++)
  {
    f2 = m_Faces[j];
    if (f2 == f)
      continue;
    
		p = CreateFacePoly(f2);
    if (!p)
      continue;

    for (i=0; i<p->m_Pts.size(); i++)
    {
      d = (p->m_Pts[i].xyz | f->m_Plane.normal) - f->m_Plane.dist;
      if (d > -DIST_EPSILON && d < DIST_EPSILON)
			{
        break;
			}
    }

    if (i != p->m_Pts.size())
    {
      if (i == 0)
      {
        d = p->m_Pts[p->m_Pts.size()-1].xyz.Dot(f->m_Plane.normal) - f->m_Plane.dist;
        if (d > -DIST_EPSILON && d < DIST_EPSILON)
          i = p->m_Pts.size() - 1;
      }

      subSelection.AddPoint( &f2->m_PlanePts[0] );
      f2->m_PlanePts[0] = p->m_Pts[i].xyz;

      if (++i == p->m_Pts.size())
        i = 0;

      d = p->m_Pts[i].xyz.Dot(f->m_Plane.normal) - f->m_Plane.dist;
      if (d > -DIST_EPSILON && d < DIST_EPSILON)
			{
        subSelection.AddPoint( &f2->m_PlanePts[1] );
			}
      f2->m_PlanePts[1] = p->m_Pts[i].xyz;
      if (++i == p->m_Pts.size())
        i = 0;

      f2->m_PlanePts[2] = p->m_Pts[i].xyz;
    }

    delete p;
  }
}

//void gDrawLightFrustum(CMapObject *o, Vec3d org);

bool SBrush::DrawModel(bool b)
{
	//@ASK Andrey.
	/*
  CComModel *m = (CComModel *)m_Owner->m_Class->m_Model;
  if (!m)
    return false;
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glEnable (GL_TEXTURE_2D);
//  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  Vec3d org = m_Owner->m_Brushes.m_NextInObj->m_bounds.min - m_Owner->m_Class->m_bounds.min;

  glTranslatef(org[0], org[1], org[2]);

  float a = m_Owner->FloatForKey("angle");
  Vec3d angs;
  angs[YAW] = a;
  if (!a)
    m_Owner->VectorForKey("angles", angs);
  glRotatef(angs[YAW], 0, 0, 1);
  glRotatef(angs[PITCH], 0, 1, 0);
  glRotatef(angs[ROLL], 1, 0, 0);

  glColor4f(1,1,1,1);
  bool bRes = m->mfDraw(NULL, true);

  glPopMatrix();
  if (!strnicmp(m_Owner->m_Class->m_Name, "light", 5))
  {
    Vec3d dir;
    m_Owner->VectorForKey("direction", dir);
    if (dir != Vec3d(0,0,0))
      gDrawLightFrustum(m_Owner, org);
  }
  glPopAttrib();

  return bRes;
	*/
	return false;
}

void SBrush::Draw()
{
	//@ASK Andrey.
	/*
  int i, j;
  SShader   *prev = 0;
  SBrushPoly *p;

  if (m_flags & BRF_HIDE)
    return;

  if (m_Patch)
  {
    m_Patch->Draw(1);
    return;
  }
  
  EDrawMode dm = gcpMapPersp->m_eDrawMode;
  
  if (m_Owner->m_Class->m_flags & OCF_NORESIZE)
  {
    bool bp = (m_flags & BRF_FAILMODEL) ? false : DrawModel(true);
    
    if (bp)
      return;
  }
  
  prev = NULL;
  for (i=0; i<m_Faces.size(); i++)
  {
    SBrushFace *f = m_Faces[i];
    p = f->m_Poly;
    if (!p)
      continue;
        
    if (f->m_Prefabs)
    {
      for (TPrefabItem *pi=f->m_Prefabs; pi; pi=pi->Next)
      {
        glPushMatrix();
        glMultMatrixf(&pi->m_Matrix.m_values[0][0]);
        if (pi->m_Model)
          pi->m_Model->mfDraw(f->m_Shader, NULL);
        else
          pi->m_Geom->mfDraw(f->m_Shader, NULL);
        glPopMatrix();
      }
      continue;
    }
    if ((dm == eDM_Textured || dm == eDM_Light) && f->m_Shader != prev)
    {
      prev = f->m_Shader;
      glBindTexture(GL_TEXTURE_2D, gNE[f->m_Shader->m_Id].m_Tex->m_Bind);
    }
    
    if (!m_Patch)
      glColor4fv(&f->m_Color[0]);
    else
      glColor4f (f->m_Color[0], f->m_Color[1], f->m_Color[2], 0.13 );

    float a;
    if ((a=gcMapEf.mfGetAlpha(f->m_Shader)) != 1.0f)
      glColor4f (f->m_Color[0], f->m_Color[1], f->m_Color[2], a);
    
    glBegin(GL_POLYGON);
    
    for (j=0; j<p->m_Pts.size(); j++)
    {
      if (dm == eDM_Textured || dm == eDM_Light)
        glTexCoord2fv( &p->m_Pts[j].st[0] );
      glVertex3fv(&p->m_Pts[j].xyz[0]);
    }
    glEnd();
  }
    
  if ((m_Owner->m_Class->m_flags & OCF_NORESIZE) && (dm == eDM_Textured || dm == eDM_Light))
    glEnable (GL_TEXTURE_2D);
  
  glBindTexture( GL_TEXTURE_2D, 0 );
	*/
}

bool SBrush::AddToListModel(bool b)
{
	bool bRes = false;
	//@ASK Andrey.
	/*
  if (!strnicmp(m_Owner->m_Class->m_Name, "light", 5))
    return true;
  
  Vec3d org = m_Owner->m_Brushes.m_NextInObj->m_bounds.min - m_Owner->m_Class->m_bounds.min;
  
  CComModel *m = (CComModel *)m_Owner->m_Class->m_Model;
  if (!m)
    return false;
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glEnable (GL_TEXTURE_2D);
//  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glTranslatef(org[0], org[1], org[2]);

  float a = m_Owner->FloatForKey("angle");
  Vec3d angs;
  angs[YAW] = a;
  if (!a)
    m_Owner->VectorForKey("angles", angs);
  glRotatef(angs[YAW], 0, 0, 1);
  glRotatef(angs[PITCH], 0, 1, 0);
  glRotatef(angs[ROLL], 1, 0, 0);

  glColor4f(1,1,1,1);
  bRes = m->mfDraw(NULL, true);

  glPopMatrix();
  glPopAttrib();
	*/

  return bRes;
}

void SBrush::AddToList(CCObject *obj)
{
	//@ASK Andrey.
	/*
  int i;
  SShader   *prev = 0;
  SBrushPoly *p;

  if (m_flags & BRF_HIDE)
    return;

  if (m_Patch)
  {
    m_Patch->Draw(1);
    return;
  }
  
  EDrawMode dm = gcpMapPersp->m_eDrawMode;
  
  if (m_Owner->m_Class->m_flags & OCF_NORESIZE)
  {
    bool bp = (m_flags & BRF_FAILMODEL) ? false : AddToListModel(true);
    
    if (bp)
      return;
  }
  
  prev = NULL;
  for (i=0; i<m_Faces.size(); i++)
  {
    SBrushFace *f = m_Faces[i];
    p = f->m_Poly;
    if (!p)
      continue;

    ECull cl = f->m_Shader->m_eCull;
    if (cl != eCULL_None)
    {
      float d = f->m_Plane.normal * g_Globals.m_Origin;
      if (cl == eCULL_Back)
      {
        if (d < f->m_Plane.dist-8.0f)
          continue;
      }
      else
      {
        if (d > f->m_Plane.dist+8.0f)
          continue;
      }
    }
    
    if (f->m_Prefabs)
    {
      for (TPrefabItem *pi=f->m_Prefabs; pi; pi=pi->Next)
      {
        glPushMatrix();
        glMultMatrixf(&pi->m_Matrix.m_values[0][0]);
        if (pi->m_Model)
          pi->m_Model->mfDraw(f->m_Shader, NULL);
        else
          pi->m_Geom->mfDraw(f->m_Shader, NULL);
        glPopMatrix();
      }
      continue;
    }

    if (g_Globals.m_TemplateId < EFT_USER_FIRST)
      f->m_Shader->AddTemplate(g_Globals.m_TemplateId);
    else
      f->m_Shader->AddTemplate(g_Globals.m_TemplateId, g_Globals.m_CustomTemplate.GetBuffer(0));
    SShader *e = f->m_Shader->mfGetTemplate(g_Globals.m_TemplateId);
    if ((gNE[e->m_Id].m_TessSize && !f->m_bTesselated) || (!gNE[e->m_Id].m_TessSize && f->m_bTesselated))
      f->BuildRE();
    
    if (obj)
      gRenDev->EF_AddEf(0, f->m_RE, f->m_Shader, obj, g_Globals.m_TemplateId, NULL);
    else
      gRenDev->EF_AddEf(0, f->m_RE, f->m_Shader, g_Globals.m_TemplateId, NULL);
  }
	*/
}

//============================================================

/*
void SBrush::Write(CMemFile* pMemFile)
{
  SBrushFace *fa;
  const char *pname;
  int   i, j;
  
  if (m_Patch)
  {
    m_Patch->Write(pMemFile);
    return;
  }
  {
    gCurMap->FPrintfToMem (pMemFile, "{\n");
    for (j=0; j<m_Faces.size(); j++)
    {
      fa = m_Faces[j];
      for (i=0; i<3; i++)
      {
        gCurMap->FPrintfToMem(pMemFile, "( ");
        for (int j = 0; j < 3; j++)
        {
          if (fa->m_PlanePts[i][j] == static_cast<int>(fa->m_PlanePts[i][j]))
            gCurMap->FPrintfToMem(pMemFile, "%i ", static_cast<int>(fa->m_PlanePts[i][j]));
          else
            gCurMap->FPrintfToMem(pMemFile, "%f ", fa->m_PlanePts[i][j]);
        }
        gCurMap->FPrintfToMem(pMemFile, ") ");
      }
      
      {
        pname = fa->m_TexInfo.name;
        if (pname[0] == 0)
          pname = "unnamed";
        
        gCurMap->FPrintfToMem (pMemFile, "%s %i %i %i ", pname, (int)fa->m_TexInfo.shift[0], (int)fa->m_TexInfo.shift[1], (int)fa->m_TexInfo.rotate);
        
        if (fa->m_TexInfo.scale[0] == (int)fa->m_TexInfo.scale[0])
          gCurMap->FPrintfToMem (pMemFile, "%i ", (int)fa->m_TexInfo.scale[0]);
        else
          gCurMap->FPrintfToMem (pMemFile, "%f ", (float)fa->m_TexInfo.scale[0]);
        if (fa->m_TexInfo.scale[1] == (int)fa->m_TexInfo.scale[1])
          gCurMap->FPrintfToMem (pMemFile, "%i", (int)fa->m_TexInfo.scale[1]);
        else
          gCurMap->FPrintfToMem (pMemFile, "%f", (float)fa->m_TexInfo.scale[1]);
        
        gCurMap->FPrintfToMem (pMemFile, " %i %i %i", fa->m_TexInfo.contents, fa->m_TexInfo.flags, fa->m_TexInfo.value);
      }
      gCurMap->FPrintfToMem (pMemFile, "\n");
    }
    gCurMap->FPrintfToMem (pMemFile, "}\n");
  }
}
*/

//////////////////////////////////////////////////////////////////////////
void SBrush::Serialize( XmlNodeRef &xmlNode,bool bLoading )
{
	if (bLoading)
	{
		// Loading.
		// Load all faces.
		int numFaces = xmlNode->getChildCount();
		m_Faces.resize( numFaces );
		for (int i = 0; i < numFaces; i++)
		{
			SBrushFace *fc = new SBrushFace;
			m_Faces[i] = fc;

			XmlNodeRef faceNode = xmlNode->getChild(i);
			faceNode->getAttr( "p1",fc->m_PlanePts[0] );
			faceNode->getAttr( "p2",fc->m_PlanePts[1] );
			faceNode->getAttr( "p3",fc->m_PlanePts[2] );

			Vec3 texScale( 1,1,1 );
			Vec3 texShift( 0,0,0 );
			faceNode->getAttr( "TexScale",texScale );
			faceNode->getAttr( "TexShift",texShift );
			fc->m_TexInfo.scale[0] = texScale.x;
			fc->m_TexInfo.scale[1] = texScale.y;
			fc->m_TexInfo.shift[0] = texShift.x;
			fc->m_TexInfo.shift[1] = texShift.y;
		}
		BuildSolid(false);
	}
	else
	{
		// Saving.
		//char str[1024];
		// Save all faces.
		for (int i = 0; i < m_Faces.size(); i++)
    {
			SBrushFace *fc = m_Faces[i];
			XmlNodeRef faceNode = xmlNode->newChild( "Face" );
			faceNode->setAttr( "p1",fc->m_PlanePts[0] );
			faceNode->setAttr( "p2",fc->m_PlanePts[1] );
			faceNode->setAttr( "p3",fc->m_PlanePts[2] );

			Vec3 texScale( fc->m_TexInfo.scale[0],fc->m_TexInfo.scale[1],0 );
			Vec3 texShift( fc->m_TexInfo.shift[0],fc->m_TexInfo.shift[1],0 );
			faceNode->setAttr( "TexScale",texScale );
			faceNode->setAttr( "TexShift",texShift );

			//sprintf( 
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void SBrush::SetMatrix( const Matrix44 &tm )
{
	m_matrix = tm;
	// clear RE valid flag.
	m_flags &= ~BRF_RE_VALID;
}

//////////////////////////////////////////////////////////////////////////
void SBrush::Render( IRenderer *renderer,const Vec3 &objectSpaceCamSrc )
{
	assert( renderer );

	if (m_prefabGeom)
	{
		Vec3 omin = m_prefabGeom->GetGeometry()->GetBoxMin();
		Vec3 omax = m_prefabGeom->GetGeometry()->GetBoxMax();

		Matrix44 mov1;
		Matrix44 mov2;
		mov1.SetIdentity();
		mov2.SetIdentity();
		mov1.SetTranslationOLD( -omin );
		mov2.SetTranslationOLD( m_bounds.min );

		Vec3 bb = (m_bounds.max - m_bounds.min);
		Vec3 ob = (omax - omin);
		Vec3 scl;
		scl.x = bb.x / ob.x;
		scl.y = bb.y / ob.y;
		scl.z = bb.z / ob.z;

		Matrix44 stm;
		stm.SetIdentity();

		stm=Matrix33::CreateScale( Vec3d(scl.x,scl.y,scl.z) )*stm;

		Matrix44 tm = mov1 * stm * mov2;
		tm = tm * m_matrix;

		SRendParams rp;
		rp.pMatrix = &tm;
		rp.nShaderTemplate = -1;
		rp.nDLightMask = 0xFFFFFFFF;
    rp.dwFObjFlags |= FOB_TRANS_MASK;
		//m_indoorGeom->Render( rp );
		m_prefabGeom->Render( rp );

		return;
	}

	if (!(m_flags&BRF_RE_VALID))
	{
		if (!BuildRenderElements())
			return;
	}

	int i;

	//CCObject *pObj = renderer->EF_GetObject(true);
	//pObj->m_Matrix = m_matrix;
	//pObj->m_ObjFlags |= FOB_USEMATRIX;

	ECull cl = eCULL_Back;

	for (i = 0; i < m_Faces.size(); i++)
  {
    SBrushFace *f = m_Faces[i];
    SBrushPoly *p = f->m_Poly;
    if (!p)
      continue;

    //ECull cl = f->m_Shader->m_eCull;
    //if (cl != eCULL_None)
    {
      float d = f->m_Plane.normal * objectSpaceCamSrc;
      if (cl == eCULL_Back)
      {
        if (d < f->m_Plane.dist-0.001f)
          continue;
      }
      else
      {
        if (d > f->m_Plane.dist+0.001f)
          continue;
      }
    }

		/*
    if (f->m_Prefabs)
    {
      for (TPrefabItem *pi=f->m_Prefabs; pi; pi=pi->Next)
      {
        glPushMatrix();
        glMultMatrixf(&pi->m_Matrix.m_values[0][0]);
        if (pi->m_Model)
          pi->m_Model->mfDraw(f->m_Shader, NULL);
        else
          pi->m_Geom->mfDraw(f->m_Shader, NULL);
        glPopMatrix();
      }
      continue;
    }
		*/

		/*
    if (g_Globals.m_TemplateId < EFT_USER_FIRST)
      f->m_Shader->AddTemplate(g_Globals.m_TemplateId);
    else
      f->m_Shader->AddTemplate(g_Globals.m_TemplateId, g_Globals.m_CustomTemplate.GetBuffer(0));
    SShader *e = f->m_Shader->mfGetTemplate(g_Globals.m_TemplateId);
    if ((gNE[e->m_Id].m_TessSize && !f->m_bTesselated) || (!gNE[e->m_Id].m_TessSize && f->m_bTesselated))
      f->BuildRE();
		*/

		if (f->m_RE)
		{
			static IShader *pShader = 0;
			if (!pShader)
				pShader = renderer->EF_LoadShader( "Textures\\Detail.jpg",eSH_World );

			/*
			static CCObject *pObj = 0;
			if (!pObj)
			{
				pObj = renderer->EF_GetObject(true);
				pObj->m_Matrix.Identity();
				pObj->m_ObjFlags |= FOB_USEMATRIX;
			}
			*/

			renderer->EF_AddEf( 0,f->m_RE,pShader, NULL, NULL, -1 );
		}
    //else
      //gRenDev->EF_AddEf(0, f->m_RE, f->m_Shader, g_Globals.m_TemplateId, NULL);
  }
}

//////////////////////////////////////////////////////////////////////////
void SBrush::SetPrefabGeom( CEdMesh *obj )
{
	m_prefabGeom = obj;
	
	if (m_indoorGeom)
	{
		//m_indoorGeom->Release();
		//m_indoorGeom = 0;
	}

	if (!m_prefabGeom)
		return;

	bool bNewCreated = false;
	if (m_Faces.empty())
	{
		BBox box;
		m_prefabGeom->GetBounds(box);

		if (box.max.x - box.min.x == 0)
			box.max.x = box.min.x + MIN_BOUNDS_SIZE;
		if (box.max.y - box.min.y == 0)
			box.max.y = box.min.y + MIN_BOUNDS_SIZE;
		if (box.max.z - box.min.z == 0)
			box.max.z = box.min.z + MIN_BOUNDS_SIZE;

		// Create brush with specified bounds.
		SMapTexInfo ti;
		Create( box.min,box.max,&ti );
		bNewCreated = true;
	}

	/*
	// Clone prefab geometry to make indoor geometry.
	m_indoorGeom->MakeLeafBuffer( m_prefabGeom->GetTriData(),false );

	m_indoorGeom->SetBBoxMin( box.min );
	m_indoorGeom->SetBBoxMax( box.max );
	*/

	if (!bNewCreated)
	{
		SMapTexInfo ti;
		Create( m_bounds.min,m_bounds.max,&ti );
	}

	BuildSolid(false);
}

//////////////////////////////////////////////////////////////////////////
void SBrush::ResetToPrefabSize()
{
	if (!m_prefabGeom)
		return;

	BBox box;
	m_prefabGeom->GetBounds( box );
	if (box.max.x - box.min.x == 0)
		box.max.x = box.min.x + MIN_BOUNDS_SIZE;
	if (box.max.y - box.min.y == 0)
		box.max.y = box.min.y + MIN_BOUNDS_SIZE;
	if (box.max.z - box.min.z == 0)
		box.max.z = box.min.z + MIN_BOUNDS_SIZE;

	// Create brush with specified bounds.
	SMapTexInfo ti;
	Create( box.min,box.max,&ti );
	BuildSolid(false);
}

//////////////////////////////////////////////////////////////////////////
IStatObj* SBrush::GetIndoorGeom()
{
	if (!(m_flags&BRF_RE_VALID))
	{
		if (!BuildRenderElements())
			return 0;
	}
	return m_indoorGeom;
};

/*

//////////////////////////////////////////////////////////////////////////
CIndexedMesh* SBrush::CreateGeometry()
{
	CIndexedMesh *mesh = 0;
	if (m_prefabGeom)
	{
		CIndexedMesh *src = m_prefabGeom->GetTriData();

		Vec3 omin = m_prefabGeom->GetBoxMin();
		Vec3 omax = m_prefabGeom->GetBoxMax();
		Vec3 bb = (m_bounds.max - m_bounds.min);
		Vec3 ob = (omax - omin);
		Vec3 scl;
		scl.x = bb.x / ob.x;
		scl.y = bb.y / ob.y;
		scl.z = bb.z / ob.z;

		Vec3 objofs = -(omax + omin)/2;
		objofs = Vec3( objofs.x*scl.x,objofs.y*scl.y,objofs.z*scl.z );
		Matrix stm;
		stm.Identity();
		stm.ScaleMatrix(scl.x,scl.y,scl.z);
		stm.SetTranslation( objofs );
		Matrix tm = stm * m_matrix;

		// Allocated data in indexed mesh.
		CryFace
		mesh = new CIndexedMesh
		//m_pFaces = (CObjFace*)malloc(sizeof(CObjFace)*nFaces);
		//mesh->

		int numv = srcBuf->m_SecVertCount;
		int stride1,stride2;
		unsigned char *src = srcBuf->GetPosPtr( stride1 );
		unsigned char *trg = trgBuf->GetPosPtr( stride2 );
		for (int i = 0; i < numv; i++)
		{
			Vec3 &sv = *(Vec3*)src;
			Vec3 &tv = *(Vec3*)trg;
			//Vec3 sv1 = stm.TransformPoint(sv);
			tv = tm.TransformPoint( sv );
			src += stride1;
			trg += stride2;
		}

		trgBuf->InvalidateVideoBuffer();
		m_flags |= BRF_RE_VALID;

		return true;
	}
}
*/