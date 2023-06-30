////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   cbuffer.h
//  Version:     v1.00
//  Created:     30/5/2002 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: Occlusion buffer
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CBUFFER_H
#define _CBUFFER_H

#define COVERAGEBUFFER_SIZE 128
#define COVERAGEBUFFER_OCCLUDERS_MAX_DISTANCE 32
#define COVERAGEBUFFER_OCCLUSION_TESTERS_MIN_DISTANCE 32

class CCoverageBuffer
{
  uchar m_Buffer[COVERAGEBUFFER_SIZE][COVERAGEBUFFER_SIZE];
  IRenderer * m_pRenderer;
  int   m_matViewPort[4];
  float m_matCombined[16];
  Plane m_Planes[6];

  struct Point2d 
  { 
    float x,y; 
    Point2d(){};
    Point2d(float _x, float _y) { x=_x; y=_y; }
    Point2d operator - (Point2d & o) { return Point2d (x - o.x,y - o.y); }
  };

  Point2d ProjectToScreen(const float & x, const float & y, const float & z);
  void ScanTriangle(Point2d p1,Point2d p2,Point2d p3);
  void ScanTriangleWithCliping(Point2d p1,Point2d p2,Point2d p3,
                               const Vec3d & v1,const Vec3d & v2,const Vec3d & v3);
  void ScanLine(int x1, int x2, int y, float & dxdyl, float & dxdyr);
  bool IsQuadVisible(const Point2d & min2d, const Point2d & max2d);

	#if defined(WIN32) && defined(_CPU_X86)
  inline int fastfround(float f) // note: only positive numbers works correct
  {
    int i;
    __asm fld [f]
    __asm fistp [i]
    return i;
  }
	#else
  inline int fastfround(float f) { int i; i=(int)(f+0.5f); return i; } // note: only positive numbers works correct
	#endif

  void TransformPoint(float out[4], const float m[16], const float in[4]);
  void MatMul4( float *product, const float *a, const float *b );
  static int ClipEdge(const Vec3d & v1, const Vec3d & v2, const Plane & ClipPlane, Vec3d & vRes1, Vec3d & vRes2);

  // tmp containers
  list2<Vec3d> m_arrVerts3dWS_AddMesh;
  list2<Point2d> m_arrVerts_AddMesh;

  int m_nTrisInBuffer;

public:

  CCoverageBuffer(IRenderer * pRenderer)
  { 
    m_pRenderer = pRenderer; 
    memset(m_Buffer,0,sizeof(m_Buffer)); 
    m_nTrisInBuffer=0;
  }

  // start new frame
  void BeginFrame(const CCamera & camera);

  // render into buffer
  void AddBBox(const Vec3d & vMins, const Vec3d & vMaxs, const Vec3d & vDirToCamera);
  void AddMesh(const Vec3d * arrVerts3d, int nVertCount, const int * pIndices, int nIndCount, Matrix44 * pTranRotMatrix);
  
  // test visibility
  bool IsBBoxVisible(const Vec3d & vMins, const Vec3d & vMaxs);
  bool __IsSphereVisible(const Vec3d & vCenter, float fRadius, float fDistance);
  bool IsPixelVisible(int nScreenX, int nScreenY);

  // draw content to the screen for debug
  void DrawDebug(int nStep);

  // can be used by other classes
  static void ClipPolygon(list2<Vec3d> * pPolygon, const Plane & ClipPlane);
};



#endif 
