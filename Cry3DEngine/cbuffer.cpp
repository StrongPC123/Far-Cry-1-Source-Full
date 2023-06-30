////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cbuffer.cpp
//  Version:     v1.00
//  Created:     30/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Occlusion (coverage) buffer
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "cbuffer.h"

void CCoverageBuffer::TransformPoint(float out[4], const float m[16], const float in[4])
{
#define M(row,col)  m[col*4+row]
  out[0] = M(0, 0) * in[0] + M(0, 1) * in[1] + M(0, 2) * in[2] + M(0, 3) * in[3];
  out[1] = M(1, 0) * in[0] + M(1, 1) * in[1] + M(1, 2) * in[2] + M(1, 3) * in[3];
  out[2] = M(2, 0) * in[0] + M(2, 1) * in[1] + M(2, 2) * in[2] + M(2, 3) * in[3];
  out[3] = M(3, 0) * in[0] + M(3, 1) * in[1] + M(3, 2) * in[2] + M(3, 3) * in[3];
#undef M
}

void CCoverageBuffer::MatMul4( float *product, const float *a, const float *b )
{
#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]
  int i;
  for (i=0; i<4; i++)
  {
    float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
    P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
    P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
    P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
    P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
  }
#undef A
#undef B
#undef P
}

CCoverageBuffer::Point2d CCoverageBuffer::ProjectToScreen(const float & x, const float & y, const float & z)
{
  Point2d res; float res_z;

  float _in[4], out[4];
  _in[0] = x;
  _in[1] = y;
  _in[2] = z;
  _in[3] = 1.0;

  TransformPoint(out, m_matCombined, _in);

  if (out[3] == 0.0)
    res.x = res.y = -1000; // mark vertex as bad - skip this triangle
  else
  {
    res.x = out[0] / out[3];
    res.y = out[1] / out[3];
    res_z = out[2] / out[3];

    res.x = m_matViewPort[0] + (1.f + res.x) * m_matViewPort[2] * 0.5f;
    res.y = m_matViewPort[1] + (1.f + res.y) * m_matViewPort[3] * 0.5f;

    res_z = (1.f + res_z) * 0.5f;

    if( res_z < 0 || res_z > 1 )
      res_z=res_z;//res.x = res.y = -1000; // mark vertex as bad - skip this triangle
  }

  return res;
}

void CCoverageBuffer::AddBBox(const Vec3d & mins, const Vec3d & maxs, const Vec3d & vDirToCamera)
{
  Vec3d arrVerts3d[8] = 
  {
    Vec3d(mins.x,mins.y,mins.z),
    Vec3d(mins.x,maxs.y,mins.z),
    Vec3d(maxs.x,mins.y,mins.z),
    Vec3d(maxs.x,maxs.y,mins.z),
    Vec3d(mins.x,mins.y,maxs.z),
    Vec3d(mins.x,maxs.y,maxs.z),
    Vec3d(maxs.x,mins.y,maxs.z),
    Vec3d(maxs.x,maxs.y,maxs.z)
  };

  Point2d arrVerts[8] = 
  { // transform into screen space
    ProjectToScreen(mins.x,mins.y,mins.z),
    ProjectToScreen(mins.x,maxs.y,mins.z),
    ProjectToScreen(maxs.x,mins.y,mins.z),
    ProjectToScreen(maxs.x,maxs.y,mins.z),
    ProjectToScreen(mins.x,mins.y,maxs.z),
    ProjectToScreen(mins.x,maxs.y,maxs.z),
    ProjectToScreen(maxs.x,mins.y,maxs.z),
    ProjectToScreen(maxs.x,maxs.y,maxs.z)
  };

  // find 2d bounds in screen space
  Point2d min2d = arrVerts[0], max2d = arrVerts[0];
  for(int i=0; i<8; i++)
  {
    if(arrVerts[i].x < min2d.x)
      min2d.x = arrVerts[i].x;
    if(arrVerts[i].x > max2d.x)
      max2d.x = arrVerts[i].x;
    if(arrVerts[i].y < min2d.y)
      min2d.y = arrVerts[i].y;
    if(arrVerts[i].y > max2d.y)
      max2d.y = arrVerts[i].y;
  }

  // adjust 2d bounds to make it 1 pixel smaler
  min2d.x += 1;
  min2d.y += 1;
  max2d.x -= 1;
  max2d.y -= 1;

  // adjust 2d vertices
  for(int i=0; i<8; i++)
  {
    if(arrVerts[i].x < min2d.x)
      arrVerts[i].x = min2d.x;
    if(arrVerts[i].x > max2d.x)
      arrVerts[i].x = max2d.x;
    if(arrVerts[i].y < min2d.y)
      arrVerts[i].y = min2d.y;
    if(arrVerts[i].y > max2d.y)
      arrVerts[i].y = max2d.y;
  }

  // render only front faces of box
  if(vDirToCamera.x>0)
  {
    ScanTriangleWithCliping(arrVerts[2],arrVerts[3],arrVerts[6],arrVerts3d[2],arrVerts3d[3],arrVerts3d[6]);
    ScanTriangleWithCliping(arrVerts[6],arrVerts[3],arrVerts[7],arrVerts3d[6],arrVerts3d[3],arrVerts3d[7]);
  }                                                                              
  else                                                                           
  {                                                                              
    ScanTriangleWithCliping(arrVerts[1],arrVerts[0],arrVerts[4],arrVerts3d[1],arrVerts3d[0],arrVerts3d[4]);
    ScanTriangleWithCliping(arrVerts[1],arrVerts[4],arrVerts[5],arrVerts3d[1],arrVerts3d[4],arrVerts3d[5]);
  }                                                                              
                                                                                 
  if(vDirToCamera.y>0)                                                           
  {                                                                              
    ScanTriangleWithCliping(arrVerts[3],arrVerts[1],arrVerts[7],arrVerts3d[3],arrVerts3d[1],arrVerts3d[7]);
    ScanTriangleWithCliping(arrVerts[7],arrVerts[1],arrVerts[5],arrVerts3d[7],arrVerts3d[1],arrVerts3d[5]);
  }                                                                              
  else                                                                           
  {                                                                              
    ScanTriangleWithCliping(arrVerts[0],arrVerts[2],arrVerts[4],arrVerts3d[0],arrVerts3d[2],arrVerts3d[4]);
    ScanTriangleWithCliping(arrVerts[4],arrVerts[2],arrVerts[6],arrVerts3d[4],arrVerts3d[2],arrVerts3d[6]);
  }
}

void CCoverageBuffer::AddMesh(const Vec3d * arrVerts3dOS, int nVertCount, const int * pIndices, int nIndCount, Matrix44* pTranRotMatrix)
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_3DENGINE );
  m_arrVerts3dWS_AddMesh.Clear();
  m_arrVerts3dWS_AddMesh.PreAllocate(nVertCount,nVertCount);

  m_arrVerts_AddMesh.Clear();
  m_arrVerts_AddMesh.PreAllocate(nVertCount,nVertCount);

  // transform vertices
  for(int i=0; i<nVertCount; i++)
  {
    m_arrVerts3dWS_AddMesh[i] = pTranRotMatrix->TransformPointOLD(arrVerts3dOS[i]);
    m_arrVerts_AddMesh[i] = ProjectToScreen(m_arrVerts3dWS_AddMesh[i].x,m_arrVerts3dWS_AddMesh[i].y,m_arrVerts3dWS_AddMesh[i].z);
  }
/*
  // find 2d bounds in screen space
  Point2d min2d = m_arrVerts_AddMesh[0], max2d = m_arrVerts_AddMesh[0];
  for(int i=0; i<nVertCount; i++)
  {
    if(m_arrVerts_AddMesh[i].x < min2d.x)
      min2d.x = m_arrVerts_AddMesh[i].x;
    if(m_arrVerts_AddMesh[i].x > max2d.x)
      max2d.x = m_arrVerts_AddMesh[i].x;
    if(m_arrVerts_AddMesh[i].y < min2d.y)
      min2d.y = m_arrVerts_AddMesh[i].y;
    if(m_arrVerts_AddMesh[i].y > max2d.y)
      max2d.y = m_arrVerts_AddMesh[i].y;
  }

  // adjust 2d bounds to make it 1 pixel smaler
  min2d.x += 1;
  min2d.y += 1;
  max2d.x -= 1;
  max2d.y -= 1;

  // adjust 2d vertices
  for(int i=0; i<nVertCount; i++)
  {
    if(m_arrVerts_AddMesh[i].x < min2d.x)
      m_arrVerts_AddMesh[i].x = min2d.x;
    if(m_arrVerts_AddMesh[i].x > max2d.x)
      m_arrVerts_AddMesh[i].x = max2d.x;
    if(m_arrVerts_AddMesh[i].y < min2d.y)
      m_arrVerts_AddMesh[i].y = min2d.y;
    if(m_arrVerts_AddMesh[i].y > max2d.y)
      m_arrVerts_AddMesh[i].y = max2d.y;
  }
  */
  // render tris
  for(int i=0; i<nIndCount; i+=3)
  {
    assert(pIndices[i+0]<nVertCount);
    assert(pIndices[i+1]<nVertCount);
    assert(pIndices[i+2]<nVertCount);
/*     
    float x1 = arrVerts[pIndices[i+0]].x;
    float y1 = arrVerts[pIndices[i+0]].y;
    float x2 = arrVerts[pIndices[i+1]].x;
    float y2 = arrVerts[pIndices[i+1]].y;
    float x3 = arrVerts[pIndices[i+2]].x;
    float y3 = arrVerts[pIndices[i+2]].y;

    float fDot = (x1-x2)*(y3-y2)-(y1-y2)*(x3-x2);
         
    if(fDot<0)*/
      ScanTriangleWithCliping(
        m_arrVerts_AddMesh[pIndices[i+0]],
        m_arrVerts_AddMesh[pIndices[i+1]],
        m_arrVerts_AddMesh[pIndices[i+2]],
        m_arrVerts3dWS_AddMesh[pIndices[i+0]],
        m_arrVerts3dWS_AddMesh[pIndices[i+1]],
        m_arrVerts3dWS_AddMesh[pIndices[i+2]]);
  }
}

bool CCoverageBuffer::IsBBoxVisible(const Vec3d & mins, const Vec3d & maxs)
{
  if(!m_nTrisInBuffer)
    return true;

  Point2d verts[8] = 
  { // transform into screen space
    // todo: check only front faces
    ProjectToScreen(mins.x,mins.y,mins.z),
    ProjectToScreen(mins.x,maxs.y,mins.z),
    ProjectToScreen(maxs.x,mins.y,mins.z),
    ProjectToScreen(maxs.x,maxs.y,mins.z),
    ProjectToScreen(mins.x,mins.y,maxs.z),
    ProjectToScreen(mins.x,maxs.y,maxs.z),
    ProjectToScreen(maxs.x,mins.y,maxs.z),
    ProjectToScreen(maxs.x,maxs.y,maxs.z)
  };

  // find 2d bounds to use it as 2d quad
  Point2d min2d = verts[0], max2d = verts[0];
  for(int i=0; i<8; i++)
  {
    if(verts[i].x < min2d.x)
      min2d.x = verts[i].x;
    if(verts[i].x > max2d.x)
      max2d.x = verts[i].x;
    if(verts[i].y < min2d.y)
      min2d.y = verts[i].y;
    if(verts[i].y > max2d.y)
      max2d.y = verts[i].y;
  }

  if(min2d.x<-900 || min2d.y<-900 || max2d.x<-900 || max2d.y<-900)
    return true; // object intersect near plane

  // make it little bigger to be sure that it's bigger than 1 pixel
  min2d.x -= 0.25f;
  min2d.y -= 0.25f;
  max2d.x += 0.25f;
  max2d.y += 0.25f;

  return IsQuadVisible(min2d, max2d);
}

bool CCoverageBuffer::__IsSphereVisible(const Vec3d & vCenter, float fRadius, float fDistance)
{
  if(!m_nTrisInBuffer)
    return true;

  Point2d Center2d = ProjectToScreen(vCenter.x,vCenter.y,vCenter.z);
  float fRadius2d = fRadius * COVERAGEBUFFER_SIZE / fDistance / 2;

  // find 2d quad
  Point2d min2d(Center2d.x-fRadius2d, Center2d.y-fRadius2d*0.5f);
  Point2d max2d(Center2d.x+fRadius2d, Center2d.y+fRadius2d*0.5f);

  return IsQuadVisible(min2d, max2d);
}

bool CCoverageBuffer::IsQuadVisible(const Point2d & min2d, const Point2d & max2d)
{
  // make ints
  int x1 = fastfround(min2d.x);
  int y1 = fastfround(min2d.y);
  int x2 = fastfround(max2d.x);
  int y2 = fastfround(max2d.y);

  // clip quads by screen bounds and reject quads totaly outside of the screen
  if(x1<0)
    x1=0;
  else if(x1>=COVERAGEBUFFER_SIZE)
    return false;

  if(y1<0)
    y1=0;
  else if(y1>=COVERAGEBUFFER_SIZE)
    return false;

  if(x2<0)
    return false;
  else if(x2>=COVERAGEBUFFER_SIZE)
    x2=COVERAGEBUFFER_SIZE-1;

  if(y2<0)
    return false;
  else if(y2>=COVERAGEBUFFER_SIZE)
    y2=COVERAGEBUFFER_SIZE-1;

  // check each pixel inside this quad, if some pixel is zero - quad is visible
  for(int x=x1; x<=x2; x++)
  for(int y=y1; y<=y2; y++)
    if(!m_Buffer[x][y])
      return true;

  return 0;
}

bool CCoverageBuffer::IsPixelVisible(int nScreenX, int nScreenY)
{
  if(!m_nTrisInBuffer)
    return true;

  nScreenX = nScreenX*COVERAGEBUFFER_SIZE/m_pRenderer->GetWidth();
  nScreenY = nScreenY*COVERAGEBUFFER_SIZE/m_pRenderer->GetHeight();

  if(nScreenX<0 || nScreenX>=COVERAGEBUFFER_SIZE)
    return false;

  if(nScreenY<0 || nScreenY>=COVERAGEBUFFER_SIZE)
    return false;

  return (!m_Buffer[nScreenX][nScreenY]);
}

void CCoverageBuffer::DrawDebug(int nStep)
{ // project buffer to the screen
  if(!nStep)
    return;

  m_pRenderer->Set2DMode(true,COVERAGEBUFFER_SIZE,COVERAGEBUFFER_SIZE);

  for(int x=0; x<COVERAGEBUFFER_SIZE; x+=nStep)
  for(int y=0; y<COVERAGEBUFFER_SIZE; y+=nStep)
    if(m_Buffer[x][y])
      m_pRenderer->DrawPoint((float)x,COVERAGEBUFFER_SIZE-(float)y,0,1);

  m_pRenderer->Set2DMode(false,COVERAGEBUFFER_SIZE,COVERAGEBUFFER_SIZE);
}

void CCoverageBuffer::ScanLine(int x1, int x2, int y, float & dxdyl, float & dxdyr)
{
  if(y<0 || y>=COVERAGEBUFFER_SIZE)
    return;

  // if we reach left border - stop changing line start x
  if(x1<0)
  {
//    dxdyl = 0;
    x1 = 0;
  }

  // if we reach right border - stop changing line stop x
  if(x2>=COVERAGEBUFFER_SIZE)
  {
//    dxdyr = 0;
    x2 = COVERAGEBUFFER_SIZE-1;
  }

  // draw line
  for(int x = x1; x <= x2; x++)
  {
    assert(x>=0 && x<COVERAGEBUFFER_SIZE);
    m_Buffer[x][y]=1;
  }
}

// return number of vertices to add
int CCoverageBuffer::ClipEdge(const Vec3d & v1, const Vec3d & v2, const Plane & ClipPlane, Vec3d & vRes1, Vec3d & vRes2)
{
  float d1 = -ClipPlane.DistFromPlane(v1);
  float d2 = -ClipPlane.DistFromPlane(v2);
  if(d1<0 && d2<0)
    return 0; // all clipped = do not add any vertices

  if(d1>=0 && d2>=0)
  {
    vRes1 = v2;
    return 1; // both not clipped - add second vertex
  }

  // calc new vertex
  Vec3d vIntersectionPoint = v1 + (v2-v1)*(Ffabs(d1)/(Ffabs(d2)+Ffabs(d1)));
  float fNewDist = -ClipPlane.DistFromPlane(vIntersectionPoint);
  assert(fabs(fNewDist)<0.01f);

  if(d1>=0 && d2<0)
  { // from vis to no vis
    vRes1 = vIntersectionPoint;
    return 1;
  }
  else if(d1<0 && d2>=0)
  { // from novis to vis
    vRes1 = vIntersectionPoint;
    vRes2 = v2;
    return 2;
  }

  assert(0);
  return 0;
}

void CCoverageBuffer::ClipPolygon(list2<Vec3d> * pPolygon, const Plane & ClipPlane)
{
  static list2<Vec3d> PolygonOut; // Keep this list static to not perform reallocation every time.
  PolygonOut.Clear();
  // clip edges, make list ov new vertices
  for(int i=0; i<pPolygon->Count(); i++)
  {
    Vec3d vNewVert1(0,0,0), vNewVert2(0,0,0);
    if(int nNewVertNum = ClipEdge(pPolygon->GetAt(i), pPolygon->GetAt((i+1)%pPolygon->Count()), ClipPlane, vNewVert1, vNewVert2))
    {
      PolygonOut.Add(vNewVert1);
      if(nNewVertNum>1)
        PolygonOut.Add(vNewVert2);
    }
  }

  // check result
  for(int i=0; i<PolygonOut.Count(); i++)
  {
    float d1 = -ClipPlane.DistFromPlane(PolygonOut.GetAt(i));
    assert(d1>=-0.01f);
  }

  assert(PolygonOut.Count()==0 || PolygonOut.Count() >= 3);

	//Timur, optimizes memory allocation here.
	pPolygon->Clear();
	pPolygon->AddList( PolygonOut );
  //*pPolygon = PolygonOut;
}

void CCoverageBuffer::ScanTriangleWithCliping(Point2d p1,Point2d p2,Point2d p3,
                                              const Vec3d & v1,const Vec3d & v2,const Vec3d & v3)
{
  if(IsEquivalent(v1,v2,VEC_EPSILON) || IsEquivalent(v1,v3,VEC_EPSILON) || IsEquivalent(v2,v3,VEC_EPSILON))
    return; // skip degenerative triangles

  // test if clipping needed 
  // todo: try to clip all triangles without this test (cache transformed vertices)
  bool bClipingNeeded = false;
  for(int p=0; p<6 ;p++)
  {
    float d1 = -m_Planes[p].DistFromPlane(v1);
    float d2 = -m_Planes[p].DistFromPlane(v2);
    float d3 = -m_Planes[p].DistFromPlane(v3);
    if(d1<0 || d2<0 || d3<0)
    {
      bClipingNeeded = true;
      break;
    }
  }

  if(!bClipingNeeded)
  { // just use already calculated 2d points
    ScanTriangle(p1,p2,p3);
    return;
  }

  // make polygon
  list2<Vec3d> arrTriangle;
  arrTriangle.Clear();

  arrTriangle.Add(v1);
  arrTriangle.Add(v2);
  arrTriangle.Add(v3);

  assert(
		!IsEquivalent(v1,v2,VEC_EPSILON) && 
		!IsEquivalent(v1,v3,VEC_EPSILON) && 
		!IsEquivalent(v2,v3,VEC_EPSILON)
		);


  // clip polygon
  for(int p=0; p<6 ;p++)
  { 
    ClipPolygon(&arrTriangle, m_Planes[p]);
  }

  // remove duplicates
  for(int i=1; i<arrTriangle.Count(); i++)
  {
    if(
				IsEquivalent(arrTriangle.GetAt(i),arrTriangle.GetAt(i-1),VEC_EPSILON)
			)
    {
      arrTriangle.Delete(i);
      i--;
    }
  }

  assert(arrTriangle.Count()<8);

  // scan
  if(arrTriangle.Count()>2 && arrTriangle.Count()<8)
  {
    Point2d arrVerts[8];

    for( int i=0; i<arrTriangle.Count(); i++ )
    { // transform into screen space
      arrVerts[i] = ProjectToScreen(arrTriangle[i].x, arrTriangle[i].y, arrTriangle[i].z);

      assert(arrVerts[i].x>-100);
      assert(arrVerts[i].y>-100);
      assert(arrVerts[i].x< 200);
      assert(arrVerts[i].y< 200);
    }

    ScanTriangle(arrVerts[0],arrVerts[1],arrVerts[2]);
    
    if(arrTriangle.Count()>3)
      ScanTriangle(arrVerts[0],arrVerts[2],arrVerts[3]);

    if(arrTriangle.Count()>4)
      ScanTriangle(arrVerts[0],arrVerts[3],arrVerts[4]);

    if(arrTriangle.Count()>5)
      ScanTriangle(arrVerts[0],arrVerts[4],arrVerts[5]);

    if(arrTriangle.Count()>6)
      ScanTriangle(arrVerts[0],arrVerts[5],arrVerts[6]);

    if(arrTriangle.Count()>7)
      ScanTriangle(arrVerts[0],arrVerts[6],arrVerts[7]);
  }
}

void CCoverageBuffer::ScanTriangle(Point2d p1,Point2d p2,Point2d p3)
{
  // back face culling // todo: move one level up
  float fDot = (p1.x-p2.x)*(p3.y-p2.y)-(p1.y-p2.y)*(p3.x-p2.x);        
  if(fDot>0)
    return;

  // make ints
  p1.x = (float)fastfround(p1.x);
  p1.y = (float)fastfround(p1.y);
  p2.x = (float)fastfround(p2.x);
  p2.y = (float)fastfround(p2.y);
  p3.x = (float)fastfround(p3.x);
  p3.y = (float)fastfround(p3.y);

  // sort vertices by y
  if(p2.y<p3.y)
  {
    Point2d tmp = p2;
    p2 = p3;
    p3 = tmp;
  }

  if(p1.y<p2.y)
  {
    Point2d tmp = p1;
    p1 = p2;
    p2 = tmp;
  }

  if(p2.y<p3.y)
  {
    Point2d tmp = p2;
    p2 = p3;
    p3 = tmp;
  }

  { // draw top part
    Point2d vLeft  = (p1-p3);
    Point2d vRight = (p1-p2);

    float dxdyl = vLeft.x /vLeft.y;
    float dxdyr = vRight.x/vRight.y;

    float x1 = p1.x;
    float x2 = p1.x;

    if(dxdyl<dxdyr)
    { // swap
      float x = x1;
      x1 = x2;
      x2 = x;

      float d = dxdyl;
      dxdyl = dxdyr;
      dxdyr = d;
    }
  
    int y2 = max(fastfround(p2.y), 0); // limit by screen bottom
    if(p1.y>p2.y && y2<COVERAGEBUFFER_SIZE) // if top side of triangle is visible
    {
      for( int y = fastfround(p1.y); y >= y2; y-- )
      {        
        ScanLine(fastfround(x1), fastfround(x2), y, dxdyl, dxdyr);
        x1 -= dxdyl;
        x2 -= dxdyr;
      }
    }
  }

  { // draw bottom part
    Point2d vLeft  = (p3-p1);
    Point2d vRight = (p3-p2);

    float dxdyl = vLeft.x /vLeft.y;
    float dxdyr = vRight.x/vRight.y;

    float x1 = p3.x;
    float x2 = p3.x;

    if(dxdyl>dxdyr)
    { // swap
      float x = x1;
      x1 = x2;
      x2 = x;

      float d = dxdyl;
      dxdyl = dxdyr;
      dxdyr = d;
    }
  
    int y2 = min(fastfround(p2.y),COVERAGEBUFFER_SIZE);// limit by screen top
    if(p2.y>p3.y && y2>0) // if bottom side of triangle is visible ((y2) line was already drawn)
    {
      for( int y = fastfround(p3.y); y < y2; y++ ) // ((y2) line was already drawn)
      {
        ScanLine(fastfround(x1), fastfround(x2), y, dxdyl, dxdyr);
        x1 += dxdyl;
        x2 += dxdyr;
      }
    }
  }

  m_nTrisInBuffer++;
}

void CCoverageBuffer::BeginFrame(const CCamera & camera)
{
  for(int p=0; p<6 ;p++)
    m_Planes[p] = *camera.GetFrustumPlane(p);

  // make matrices
  m_matViewPort[0] = 0;
  m_matViewPort[1] = 0;
  m_matViewPort[2] = COVERAGEBUFFER_SIZE;
  m_matViewPort[3] = COVERAGEBUFFER_SIZE;

  float matModel[16];
  float matProj[16];
  m_pRenderer->GetModelViewMatrix(matModel);
  m_pRenderer->GetProjectionMatrix(matProj);
  MatMul4(m_matCombined,matProj,matModel);

  // reset buffer
  memset(m_Buffer,0,sizeof(m_Buffer)); 

  m_nTrisInBuffer=0;
}