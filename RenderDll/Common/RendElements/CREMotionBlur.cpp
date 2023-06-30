#if !defined(LINUX)

#include "stdafx.h"
#include "RendElement.h"
#include "CREMotionBlur.h"

#ifdef OPENGL
#include "..\..\XRenderOGL\GL_Renderer.h"
#endif

void CREMotionBlur::mfPrepare()
{
  gRenDev->EF_CheckOverflow(0, 0, this);

  gRenDev->m_RP.m_DynLMask |= m_DynMask;
  gRenDev->m_RP.m_pRE = this;
  gRenDev->m_RP.m_RendNumIndices = 0;
  gRenDev->m_RP.m_RendNumVerts = 0;
}

bool CREMotionBlur::mfDraw(SShader *ef, SShaderPass *sfm)
{
  gRenDev->ResetToDefault();

  static int counter=0;

  gRenDev->EnableBlend(true);
  gRenDev->SetBlendMode();
  gRenDev->Set2DMode(true,100,100);
  gRenDev->EnableDepthWrites(false);
  gRenDev->EnableDepthTest(false);

  for(int pass=-1; pass>-16 ; pass--)
  {
    if(!sfm->m_TUnits[0]->mfSetTexture(counter+pass))
      break;

    gRenDev->SetEnviMode(R_MODE_MODULATE);
    gRenDev->SetMaterialColor(1,1,1,1.f/(float)fabs(pass+1));

    struct_VERTEX_FORMAT_P3F_TEX2F data[] =
    {
      100,   100, 0, 1,1.f-1,
      100,     0, 0, 1,1.f-0,
        0,   100, 0, 0,1.f-1,
        0,     0, 0, 0,1.f-0,
    };

    gRenDev->PushMatrix();
    gRenDev->TranslateMatrix(Vec3d(50,50,0));

    // scale
    if(m_nEffectType == 1)
    {
      float scale = 1.0f-(float)fabs(pass/16.f)/3.0f;
      gRenDev->ScaleMatrix(scale,scale,0);
    }
    if(m_nEffectType == 2)
    {
      float scale = 1.f+(float)fabs(pass/16.f)/3;
      gRenDev->ScaleMatrix(scale,scale,0);
    }
    // rotation
    else if(m_nEffectType == 3)
    {
      gRenDev->RotateMatrix(Vec3d(0,0,pass&1 ? (float)fabs(pass/16.f)*4.0f : -(float)fabs(pass/16.f)*4.0f));
    }
    else if(m_nEffectType == 4)
    {
      gRenDev->RotateMatrix(Vec3d(0,0,pass&1 ? (float)fabs(pass/16.f)*40 : -(float)fabs(pass/16.f)*40));
    }
    // translation
    else if(m_nEffectType == 5)
    {
      gRenDev->TranslateMatrix(Vec3d(pass&1 ? (float)fabs(pass/16.f)*4.0f : -(float)fabs(pass/16.f)*4.0f,0,0));
    }
    else if(m_nEffectType == 6)
    {
      gRenDev->TranslateMatrix(Vec3d(0,pass&1 ? (float)fabs(pass/16.f)*4.0f : -(float)fabs(pass/16.f)*4.0f,0));
    }

    gRenDev->TranslateMatrix(Vec3d(-50,-50,0));
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
    gRenDev->PopMatrix();
  }

  counter++;

  gRenDev->Set2DMode(false,100,100);
  gRenDev->ResetToDefault();


  return true;
}

#endif // !defined(LINUX)