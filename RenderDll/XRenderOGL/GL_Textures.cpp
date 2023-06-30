//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Gl_Textures.cpp
//  mostrly not used now
//
//	History:
//	-Jan 31,2001:Created by Vladimir Kajalin
//
//////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "GL_Renderer.h"

#include "IStatObj.h"

void CGLRenderer::SetTexture(int tnum, ETexType Type)
{
  m_TexMan->SetTexture(tnum, Type);
}

void WriteTGA(byte *data, int width, int height, char *filename, int dest_bits_per_pixel);

uint CGLRenderer::Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj)
{
  char name[128];
  int flags = FT_HASALPHA;
  int flags2 = FT2_NODXT | FT2_DISCARDINCACHE;
  sprintf(name, "$Spr_%s", pStatObj->GetFileName());
  //STexPic *ti = m_TexMan->LoadFromCache(NULL, flags, flags2, name, pStatObj->GetFileName());
  //if (ti)
  //  return ti->m_Bind;

  int bFog=0; // remember fog value
  glGetIntegerv(GL_FOG,&bFog);
  glDisable(GL_FOG);

  int nTexCount = int(360.f/fAngleStep);
  uchar * pMemBuffer = new uchar [nTexSize*nTexSize*nTexCount*4];

  for(int i=0; i<nTexCount; i++)
  {
    // render object
    ClearColorBuffer(Vec3d(0.15f, 0.15f, 0.15f));
    ClearDepthBuffer();

    glReadBuffer(GL_BACK);
    glDrawBuffer(GL_BACK);

    // cals vertical/horisontal radiuses
    float dxh = (float)max( fabs(pStatObj->GetBoxMax().x), fabs(pStatObj->GetBoxMin().x));
    float dyh = (float)max( fabs(pStatObj->GetBoxMax().y), fabs(pStatObj->GetBoxMin().y));
    float fRadiusHors = (float)sqrt_tpl(dxh*dxh + dyh*dyh);//max(dxh,dyh);
    float fRadiusVert = (pStatObj->GetBoxMax().z-pStatObj->GetBoxMin().z)*0.5f;

    float fDrawDist = fRadiusVert*25.f*8.f;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( 0.58f, fRadiusHors/fRadiusVert, fDrawDist-fRadiusHors*2, fDrawDist+fRadiusHors*2);
    glMatrixMode(GL_MODELVIEW);

    glViewport(0, 0, nTexSize, nTexSize);
    glLoadIdentity();

    Vec3d vCenter = (pStatObj->GetBoxMax()+pStatObj->GetBoxMin())*0.5f;

    gluLookAt( 0,0,0, -1,0,0, 0,0,1 );

    glColor4f(1,1,1,1);
    glPushMatrix();
    glTranslatef(-fDrawDist,0,0);
    glRotatef(fAngleStep*i, 0,0,1);
    glTranslatef(-vCenter.x,-vCenter.y,-vCenter.z);  
    
    ResetToDefault();
    EF_SetWorldColor(0.5,0.5,0.5);

    EF_StartEf();  
    SRendParams rParms;
    pStatObj->Render(rParms,Vec3(zero),0);
    EF_EndEf3D(true);

    glPopMatrix();    

    // read into buffer and make texture
    assert(nTexSize<=1024);//just warning

    glReadPixels(0, 0, nTexSize, nTexSize, GL_RGBA, GL_UNSIGNED_BYTE, &pMemBuffer[nTexSize*nTexSize*i*4]);
  		
    SetTextureAlphaChannelFromRGB(&pMemBuffer[nTexSize*nTexSize*i*4], nTexSize);
/*
    {
      int width = 64;
      int height = 64;
      int imid=i;
      char buff[32]="";
      sprintf(buff, "sprite%d.tga", imid);
      imid++;
      for(int x=0; x<nTexSize*nTexSize*4; x+=4)
      {
        Exchange(pMemBuffer[x+0], pMemBuffer[x+2]);
      }

      ::WriteTGA(&pMemBuffer[nTexSize*nTexSize*i*4], width, height, buff, 32); 
    }
  */
  }

  // make 3d texture
  /*glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  uint nTexId=0;
  glGenTextures(1,&nTexId);
  SetTexture3D(nTexId);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R_EXT, GL_REPEAT);
  glTexImage3DEXT(GL_TEXTURE_3D_EXT, 0, GL_RGBA8, nTexSize, nTexSize, nTexCount, 0, GL_RGBA, GL_UNSIGNED_BYTE, pMemBuffer);*/

  STexPic *tp = m_TexMan->CreateTexture(name, nTexSize, nTexSize, nTexCount, flags, flags2, pMemBuffer, eTT_3D, -1.0f, -1.0f, 0, NULL, 0, eTF_8888, pStatObj->GetFileName());

  delete [] pMemBuffer;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  SetViewport();

  m_TexMan->SetTexture(0, eTT_Base);

  ResetToDefault();

  if(bFog)
    glEnable(GL_FOG);

  return tp->m_Bind;
}
