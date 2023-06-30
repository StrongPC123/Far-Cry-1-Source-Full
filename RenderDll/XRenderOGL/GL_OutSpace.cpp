#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "I3dengine.h"

void CGLRenderer::PrepareOutSpaceTextures(CREOutSpace * pRE)
{
  int arrnResult[2];
  ScanOutSpaceCube(pRE->m_TexID, iSystem->GetViewCamera().GetPos(), arrnResult);

//  vPrevPos[arrnResult[0]] = iSystem->GetViewCamera().GetPos();
  //vPrevPos[arrnResult[1]] = iSystem->GetViewCamera().GetPos();

/*  
  int nTempX, nTempY, nTempWidth, nTempHeight;
  int nTexSize = 512;
  gRenDev->GetViewport(&nTempX, &nTempY, &nTempWidth, &nTempHeight);
  gRenDev->SetViewport( 0, 0, nTexSize, nTexSize );
  
  I3DEngine * pEngine = iSystem->GetI3DEngine();

  CCamera prevCamera = gRenDev->GetCamera();
  CCamera tmp_cam = prevCamera;

  tmp_cam.SetAngle(Vec3d(0,0,0));
  tmp_cam.Init(nTexSize, nTexSize);
  tmp_cam.Update();  

  iSystem->SetViewCamera(tmp_cam);
  gRenDev->SetCamera(tmp_cam);
  pEngine->SetCamera(tmp_cam,false);

  pEngine->DrawLowDetail();

  if(!pRE->m_TexID[0])
    glGenTextures(1,&(pRE->m_TexID[0]));
  
  glBindTexture(GL_TEXTURE_2D, pRE->m_TexID[0]);

  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glCopyTexImage2D(
      GL_TEXTURE_2D,           
      0,             
      GL_RGB,
      0, 0,                 
      nTexSize, nTexSize,          
      0);
  }

  glBindTexture(GL_TEXTURE_2D, 0);


  pEngine->SetCamera(prevCamera);
  iSystem->SetViewCamera(prevCamera);
  gRenDev->SetCamera(prevCamera);

  gRenDev->SetViewport(nTempX, nTempY, nTempWidth, nTempHeight);
	glClear(GL_DEPTH_BUFFER_BIT);*/
}

void CGLRenderer::DrawOutSpaceSide( const float *angle, const Vec3d & Pos, int tex_size, int offsetX, int offsetY)
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

void CGLRenderer::ScanOutSpaceCube(uint & nTexID, const Vec3d & vPos, int * pResult)
{
  // get normalized camera angle
  float fAngle = iSystem->GetViewCamera().GetAngles().z;
  while(fAngle>360)
    fAngle-=360;
  while(fAngle<0)
    fAngle+=360;

  // get 2 visible sides
  int nSide = int(fAngle+45)/90;
  int nSide2 = int(fAngle+45+45)/90;  
  if(nSide == nSide2)
    nSide2 = int(fAngle+45-45)/90;

  // normalize
  if(nSide2<0)
    nSide2=3;
  if(nSide2>3)
    nSide2=0;

  if(nSide<0)
    nSide=3;
  if(nSide>3)
    nSide=0;

  static Vec3d vPrevPos[4];

  // is images valid
  if (GetDistance(vPrevPos[nSide ],iSystem->GetViewCamera().GetPos())<1.0f &&
      GetDistance(vPrevPos[nSide2],iSystem->GetViewCamera().GetPos())<1.0f )
      return; // no need to update

  iLog->Log("nSides = %d, %d",nSide,nSide2);

  // now render 2 cubemap sides

  CRenderer * renderer = gRenDev;
  int tex_size = 512;
  while(true)
  {
    if (tex_size>renderer->GetWidth() || tex_size>renderer->GetHeight())
      tex_size >>= 1;
    else
      break;
  }
  if (tex_size <= 8)
    return;

  static const GLenum OutSpaceCubeFaces[6] = 
  {
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
  };

  static const float sAngles[6][5] = 
  {
    {   0, 180,   0,  0, 0 },
    {   0, 180,  90,  1, 0 },
    {   0, 180, 180,  2, 0 },   
    {   0, 180, 270,  0, 1 },
    { -90, 180,   0,  1, 1 },
    {  90, 180,   0,  2, 1 }
  };

  if(!nTexID)
  { // create empty if not created
    glGenTextures(1,&nTexID);
  assert(nTexID<14000);
    glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, nTexID);
    for(int n=0; n<6; n++)
      glTexImage2D(OutSpaceCubeFaces[n], 0, GL_RGB, tex_size, tex_size, 0, GL_RGB, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, 0);    
  }

	glClear(GL_DEPTH_BUFFER_BIT);

  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  int bFog=0; // remember fog value
  glGetIntegerv(GL_FOG,&bFog);
  m_RP.m_bDrawToTexture = true;

  DrawOutSpaceSide( &sAngles[nSide ][0], vPos, tex_size, 0, 0 );
  DrawOutSpaceSide( &sAngles[nSide2][0], vPos, tex_size, 1, 0 );

  glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, nTexID);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
  /*
  int width = tex_size*3;
  int height = tex_size*2;
  byte *pic = new byte [width * height * 3];
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pic);
  CImage::SaveJPG(pic,FORMAT_24_BIT,width,height,"CubeEnv.jpg"); 
  delete [] pic;
    */
 
  glCopyTexSubImage2D(OutSpaceCubeFaces[nSide ], 0, 0, 0, (int)(tex_size*0), (int)(tex_size*0), tex_size, tex_size);
  glCopyTexSubImage2D(OutSpaceCubeFaces[nSide2], 0, 0, 0, (int)(tex_size*1), (int)(tex_size*0), tex_size, tex_size);

  vPrevPos[nSide] = iSystem->GetViewCamera().GetPos();
  vPrevPos[nSide2] = iSystem->GetViewCamera().GetPos();

  pResult[0] = nSide;
  pResult[1] = nSide2;

  gRenDev->SetViewport(vX, vY, vWidth, vHeight);

  glClear(GL_DEPTH_BUFFER_BIT);
  m_RP.m_bDrawToTexture = false;

  if(bFog)
    glEnable(GL_FOG);
}

bool CREOutSpace::mfDraw(SShader *ef, SShaderPass *sfm)
{    
  if(!m_TexID)
    return true;

  gRenDev->ResetToDefault();
  gRenDev->SetEnviMode(R_MODE_MODULATE);
  gRenDev->EnableDepthTest(false);
  gRenDev->EnableDepthWrites(false);

  int bFog=0; // remember fog value
  glGetIntegerv(GL_FOG,&bFog);
  glDisable(GL_FOG);

  glBindTexture(GL_TEXTURE_CUBE_MAP_EXT,m_TexID);
  glEnable(GL_TEXTURE_CUBE_MAP_EXT);

  float scale = 1.f/1;
  float eyePlaneS[] = { scale, 0, 0, 0 };
  float eyePlaneT[] = { 0, scale, 0, 0 };
  float eyePlaneR[] = { 0, 0, scale, 0 };

  glTexGenfv(GL_S, GL_OBJECT_PLANE, eyePlaneS);
  glTexGenfv(GL_T, GL_OBJECT_PLANE, eyePlaneT);
  glTexGenfv(GL_R, GL_OBJECT_PLANE, eyePlaneR);

  glTexGenf(GL_S,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);
  glTexGenf(GL_T,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);  
  glTexGenf(GL_R,GL_TEXTURE_GEN_MODE,GL_OBJECT_LINEAR);  

  glEnable(GL_TEXTURE_GEN_S);
  glEnable(GL_TEXTURE_GEN_T);  
  glEnable(GL_TEXTURE_GEN_R);  

  glMatrixMode(GL_TEXTURE);
  glRotatef(-90,1,0,0);
  glMatrixMode(GL_MODELVIEW);

//  glRotatef(-45,0,0,1);

  float D = 8; // box size

  { // top
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    {
       D,-D, D, 1, 1.f-1,
      -D,-D, D, 0, 1.f-1,
       D, D, D, 1, 1.f-0,
      -D, D, D, 0, 1.f-0,
    };

//    gRenDev->SetTexture(m_TexID[0]);
//    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
  }

  { // s
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    { 
     -D,-D, D, 1.0, 1.f-1.0,
      D,-D, D, 0.0, 1.f-1.0,
     -D,-D,-D, 1.0, 1.f-0.0,
      D,-D,-D, 0.0, 1.f-0.0,
    };

//    gRenDev->SetTexture(m_TexID[0]);
//    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
  }
  { // e
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    { 
     -D, D, D, 1.0, 1.f-0.0,
     -D,-D, D, 0.0, 1.f-0.0,
     -D, D,-D, 1.0, 1.f-0.0,
     -D,-D,-D, 0.0, 1.f-0.0,
    };

//    gRenDev->SetTexture(m_TexID[0]);
//    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
  }
  { // n
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    { 
      D, D, D, 1.0, 1.f-1.0,
     -D, D, D, 0.0, 1.f-1.0,
      D, D,-D, 1.0, 1.f-0.0,
     -D, D,-D, 0.0, 1.f-0.0,
    };

//    gRenDev->SetTexture(m_TexID[0]);
//    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
  }
  { // w
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    { 
      D,-D, D, 1.0, 1.f-0.0,
      D, D, D, 0.0, 1.f-0.0,
      D,-D,-D, 1.0, 1.f-0.0,
      D, D,-D, 0.0, 1.f-0.0,
    };

//    gRenDev->SetTexture(m_TexID[0]);
//    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
  }
  
  { // bottom
    struct_VERTEX_FORMAT_P3F_TEX2F data[] = 
    {
       D,-D, -D, 1, 1.f-1,
      -D,-D, -D, 0, 1.f-1,
       D, D, -D, 1, 1.f-0,
      -D, D, -D, 0, 1.f-0,
    };

//    gRenDev->SetTexture(m_TexID[0]);
//    gRenDev->SetTexClampMode(true);
    gRenDev->DrawTriStrip(&(CVertexBuffer (data,VERTEX_FORMAT_P3F_TEX2F)),4);
  }

  gRenDev->ResetToDefault();
  gRenDev->ResetTextureMatrix();
  glDisable(GL_TEXTURE_GEN_S);
  glDisable(GL_TEXTURE_GEN_T);  
  glDisable(GL_TEXTURE_GEN_R);  
  glDisable(GL_TEXTURE_GEN_Q);  
  glDisable(GL_TEXTURE_CUBE_MAP_EXT);

  if(bFog)
    glEnable(GL_FOG);

  return true;

}
