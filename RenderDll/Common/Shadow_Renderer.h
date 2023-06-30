#ifndef SHADOWMAP_H
#define SHADOWMAP_H

enum EShadowType
{
  EST_DEPTH_BUFFER,
  EST_2D_BUFFER,
  EST_PENUMBRA
};

#include "IRenderer.h"

#define SMFF_ACTIVE_SHADOW_MAP 1

struct ShadowMapFrustum
{
  float debugLightFrustumMatrix[16];
  float debugLightViewMatrix[16];
  unsigned int depth_tex_id;
  struct ShadowMapLightSource * pLs;
  EShadowType shadow_type;
  float FOV;
  float min_dist;
  float max_dist;
  Vec3d target;
  bool  bUpdateRequested;
	int nDLightId;
  int   nTexSize;
  ShadowMapFrustum * pPenumbra;
  struct IEntityRender * pOwner;
	IStatObj * pOwnerGroup;
  int nTexIdSlot;
  int nResetID;
	int dwFlags;
	float fAlpha;
	float ProjRatio;
	float fOffsetScale;

  list2<struct IStatObj*> * pModelsList;
  list2<struct IEntityRender*> * pEntityList;

	Plane arrFrusrumPlanes[4];

  float m_fCeachedFrustumScale;
  int m_nCeachedFrustumFrameId;
	float m_fBending;

  ShadowMapFrustum()
  {
    ZeroStruct(*this);
    nTexIdSlot = -1;
		fAlpha = 1.f;
		ProjRatio = 1.f;
		nDLightId = -1;
		fOffsetScale = 1.f;
  }

  ~ShadowMapFrustum()
  {
    delete pModelsList;
    delete pEntityList;
    delete pPenumbra;
  }

  void UnProject(float sx, float sy, float sz, float *px, float *py, float *pz, IRenderer * pRend )
  {
    const int shadowViewport[4] = {0,0,1,1};

			
    pRend->UnProject(sx,sy,sz,
      px,py,pz,
      debugLightViewMatrix,
      debugLightFrustumMatrix,
      shadowViewport);
  }

  Vec3d & UnProjectVertex3d(int sx, int sy, int sz, Vec3d & vert, IRenderer * pRend)
  {
    float px; 
    float py; 
    float pz;
    UnProject((float)sx, (float)sy, (float)sz, &px, &py, &pz, pRend);
    vert.x=(float)px;
    vert.y=(float)py;
    vert.z=(float)pz;

//		pRend->DrawBall(vert,10);

    return vert;
  }
  
  void DrawFrustum(IRenderer * pRend, Vec3d vPos, float fScale)
  {
  
    Vec3d vert1,vert2;
    {
      pRend->Draw3dBBox(
        vPos+fScale*UnProjectVertex3d(0,0,0,vert1,pRend),
        vPos+fScale*UnProjectVertex3d(0,0,1,vert2,pRend), true);

      pRend->Draw3dBBox(
        vPos+fScale*UnProjectVertex3d(1,0,0,vert1,pRend),
        vPos+fScale*UnProjectVertex3d(1,0,1,vert2,pRend), true);

      pRend->Draw3dBBox(
        vPos+fScale*UnProjectVertex3d(1,1,0,vert1,pRend),
        vPos+fScale*UnProjectVertex3d(1,1,1,vert2,pRend), true);

      pRend->Draw3dBBox(
        vPos+fScale*UnProjectVertex3d(0,1,0,vert1,pRend),
        vPos+fScale*UnProjectVertex3d(0,1,1,vert2,pRend), true);
    }

    for(int i=0; i<=1; i++)
    {
      pRend->Draw3dBBox(
        vPos+fScale*UnProjectVertex3d(0,0,i,vert1,pRend),
        vPos+fScale*UnProjectVertex3d(1,0,i,vert2,pRend), true);

      pRend->Draw3dBBox(
        vPos+fScale*UnProjectVertex3d(1,0,i,vert1,pRend),
        vPos+fScale*UnProjectVertex3d(1,1,i,vert2,pRend), true);

      pRend->Draw3dBBox(
        vPos+fScale*UnProjectVertex3d(1,1,i,vert1,pRend),
        vPos+fScale*UnProjectVertex3d(0,1,i,vert2,pRend), true);

      pRend->Draw3dBBox(
        vPos+fScale*UnProjectVertex3d(0,1,i,vert1,pRend),
        vPos+fScale*UnProjectVertex3d(0,0,i,vert2,pRend), true);
    }
  }

  void InitFrustum(float fFrustumScale, IRenderer * pRend)
  {
    Vec3d v1,v2,v3;

    // top
    UnProjectVertex3d(0,0,0,v1,pRend),
      UnProjectVertex3d(0,0,1,v2,pRend);
    UnProjectVertex3d(1,0,1,v3,pRend);
    arrFrusrumPlanes[0].Init(v1*fFrustumScale, v2*fFrustumScale, v3*fFrustumScale);

    // left
    UnProjectVertex3d(0,1,0,v1,pRend),
      UnProjectVertex3d(0,1,1,v2,pRend);
    UnProjectVertex3d(0,0,1,v3,pRend);
    arrFrusrumPlanes[1].Init(v1*fFrustumScale, v2*fFrustumScale, v3*fFrustumScale);

    // bottom
    UnProjectVertex3d(1,1,0,v1,pRend),
      UnProjectVertex3d(1,1,1,v2,pRend);
    UnProjectVertex3d(0,1,1,v3,pRend);
    arrFrusrumPlanes[2].Init(v1*fFrustumScale, v2*fFrustumScale, v3*fFrustumScale);

    // right
    UnProjectVertex3d(1,0,0,v1,pRend),
      UnProjectVertex3d(1,0,1,v2,pRend);
    UnProjectVertex3d(1,1,1,v3,pRend);
    arrFrusrumPlanes[3].Init(v1*fFrustumScale, v2*fFrustumScale, v3*fFrustumScale);
  }

  bool IsSphereInsideFrustum(Vec3d vSphereCenter, float fSphereRadius, float fFrustumScale, IRenderer * pRend)
  {
    // todo: optimize this
    static int a=0, b=0;
    if(/*debugLightFrustumMatrix[0] && */m_fCeachedFrustumScale != fFrustumScale || m_nCeachedFrustumFrameId != pRend->GetFrameID())
    {
      InitFrustum(fFrustumScale, pRend);
      m_fCeachedFrustumScale = fFrustumScale;
      m_nCeachedFrustumFrameId = pRend->GetFrameID();
      a++;
    }
    else
    {
      b++;
    }

    float fDistance0 = arrFrusrumPlanes[0].DistFromPlane(vSphereCenter);
    float fDistance1 = arrFrusrumPlanes[1].DistFromPlane(vSphereCenter);
    float fDistance2 = arrFrusrumPlanes[2].DistFromPlane(vSphereCenter);
    float fDistance3 = arrFrusrumPlanes[3].DistFromPlane(vSphereCenter);

    bool bRes = fDistance0>-fSphereRadius && fDistance1>-fSphereRadius && fDistance2>-fSphereRadius && fDistance3>-fSphereRadius;
/*
#ifdef _DEBUG
    {
      InitFrustum(fFrustumScale, pRend);
      float fDistance0 = arrFrusrumPlanes[0].DistFromPlane(vSphereCenter);
      float fDistance1 = arrFrusrumPlanes[1].DistFromPlane(vSphereCenter);
      float fDistance2 = arrFrusrumPlanes[2].DistFromPlane(vSphereCenter);
      float fDistance3 = arrFrusrumPlanes[3].DistFromPlane(vSphereCenter);
      bool bResTest = fDistance0>-fSphereRadius && fDistance1>-fSphereRadius && fDistance2>-fSphereRadius && fDistance3>-fSphereRadius;
      assert(bResTest == bRes);
    }
#endif
*/
    return bRes;
  }
};

struct ShadowMapLightSource
{
	ShadowMapLightSource(){ ZeroStruct(*this); nDLightId = -1; }
	Vec3 vSrcPos; // relative world space
	Vec3 vObjSpaceSrcPos; // objects space
  float fRadius;
  list2<ShadowMapFrustum> m_LightFrustums;
	int nDLightId;
	int SizeOf();
	ShadowMapFrustum * GetShadowMapFrustum(int nId=0)
	{
		if(nId<m_LightFrustums.Count())
			return &m_LightFrustums[nId];
		return 0;
	}
};

struct ShadowMapLightSourceInstance
{
  ShadowMapLightSourceInstance() { ZeroStruct(*this); }
  ShadowMapLightSource * m_pLS;
  Vec3 m_vProjTranslation;
  float m_fProjScale;
  float m_fDistance;
  struct IEntityRender * m_pReceiver;
	bool m_bNoDepthTest;
};

#endif 