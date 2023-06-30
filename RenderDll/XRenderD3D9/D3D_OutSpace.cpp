#include "RenderPCH.h"
#include "DriverD3D9.h"
#include "I3dengine.h"

void CD3D9Renderer::PrepareOutSpaceTextures(CREOutSpace * pRE)
{
  int arrnResult[2];
  ScanOutSpaceCube(pRE->m_TexID, iSystem->GetViewCamera().GetPos(), arrnResult);

}

void CD3D9Renderer::DrawOutSpaceSide( const float *angle, const Vec3d & Pos, int tex_size, int offsetX, int offsetY)
{
  if (!iSystem)
    return;
  
  CRenderer * renderer = gRenDev;
  CCamera tmp_camera = renderer->GetCamera();
  CCamera prevCamera = tmp_camera;
  
  tmp_camera.Init(tex_size,tex_size);
  tmp_camera.SetPos(Pos);
  tmp_camera.SetAngle(Vec3d(angle[0], angle[1], angle[2]));
  tmp_camera.Update();

  iSystem->SetViewCamera(tmp_camera);
  gRenDev->SetCamera(tmp_camera);

  gRenDev->SetViewport(tex_size*offsetX, tex_size*offsetY, tex_size, tex_size);

  I3DEngine *eng = (I3DEngine *)iSystem->GetIProcess();

  eng->SetCamera(tmp_camera,false );

  eng->DrawLowDetail(0);

  iSystem->SetViewCamera(prevCamera);
  gRenDev->SetCamera(prevCamera);
}

void CD3D9Renderer::ScanOutSpaceCube(uint & nTexID, const Vec3d & vPos, int * pResult)
{
}

bool CREOutSpace::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  return true;
}
