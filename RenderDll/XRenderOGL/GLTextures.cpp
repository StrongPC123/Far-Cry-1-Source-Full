/*=============================================================================
  GLTextures.cpp : OpenGL specific texture manager implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "GL_Renderer.h"
#include "GLPBuffer.h"
#include "I3dengine.h"
#include "GLCubeMaps.h"
#include "CryHeaders.h"

// tiago: added
#include "GLCGPShader.h"
#include "GLCGVProgram.h"

TTextureMap CGLTexMan::m_RefTexs;

SGLTexUnit CGLTexMan::m_TUState[16];

extern int BindSizes[TX_LASTBIND];
extern int BindFrame[TX_LASTBIND];

//===============================================================================

/*** NORMALIZATION CUBE MAP CONSTRUCTION ***/

/* Given a cube map face index, cube map size, and integer 2D face position,
 * return the cooresponding normalized vector.
 */
void CGLTexMan::GetCubeVector(int i, int cubesize, int x, int y, float *vector)
{
  float s, t, sc, tc, mag;

  s = ((float)x + 0.5f) / (float)cubesize;
  t = ((float)y + 0.5f) / (float)cubesize;
  sc = s*2.0f - 1.0f;
  tc = t*2.0f - 1.0f;

  switch (i)
  {
    case 0:
      vector[0] = 1.0f;
      vector[1] = -tc;
      vector[2] = -sc;
      break;
    case 1:
      vector[0] = -1.0f;
      vector[1] = -tc;
      vector[2] = sc;
      break;
    case 2:
      vector[0] = sc;
      vector[1] = 1.0f;
      vector[2] = tc;
      break;
    case 3:
      vector[0] = sc;
      vector[1] = -1.0f;
      vector[2] = -tc;
      break;
    case 4:
      vector[0] = sc;
      vector[1] = -tc;
      vector[2] = 1.0f;
      break;
    case 5:
      vector[0] = -sc;
      vector[1] = -tc;
      vector[2] = -1.0f;
      break;
  }

  mag = 1.0f/cry_sqrtf(vector[0]*vector[0] + vector[1]*vector[1] + vector[2]*vector[2]);
  vector[0] *= mag;
  vector[1] *= mag;
  vector[2] *= mag;
}

void CGLTexMan::MakeNormalizeVectorCubeMap(int size, STexPic *tp)
{
  float vector[3];
  int i, x, y;
  GLubyte *pixels;

  pixels = new GLubyte [size*size*3];

  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  tp->m_Width = size;
  tp->m_Height = size;

  for (i = 0; i < 6; i++)
  {
    for (y = 0; y < size; y++)
    {
      for (x = 0; x < size; x++)
      {
        GetCubeVector(i, size, x, y, vector);
        pixels[3*(y*size+x) + 0] = (byte)(128.0f + 127.0f*vector[0]);
        pixels[3*(y*size+x) + 1] = (byte)(128.0f + 127.0f*vector[1]);
        pixels[3*(y*size+x) + 2] = (byte)(128.0f + 127.0f*vector[2]);
      }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT+i, 0, GL_RGB8, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  }
  CGLTexMan::CalcMipsAndSize(tp);
  tp->m_Size *= 6;
  AddToHash(tp->m_Bind, tp);
  tp->Unlink();
  tp->Link(&STexPic::m_Root);
  gRenDev->m_TexMan->m_StatsCurTexMem += tp->m_Size;
  CheckTexLimits(NULL);

  delete [] pixels;
}

void CGLTexMan::MakePhongLookupTexture(float shininess, STexPic *ti)
{
  int specular_size = 256;
  int diffuse_size = 256;
  unsigned char * img = new unsigned char[specular_size*diffuse_size*4];
  unsigned char * ip = img;
  for(int j=0; j<specular_size; j++)
  {
    unsigned char a = (unsigned char)(255.99 * pow(double(j/(specular_size-1.0)), (double)shininess));
    for(int i=0; i<diffuse_size; i++)
    {
      byte b = (unsigned char)(255.99 * (i/(diffuse_size-1.0))); 
      *ip++ = b;
      *ip++ = b;
      *ip++ = b;
      *ip++ = a;
    }
  }
  ti->m_Size = 0;
  ti->m_nMips = 0;
  int wdt = diffuse_size;
  int hgt = specular_size;
  int mode = GL_RGBA8;
  ti->m_Width = wdt;
  ti->m_Height = hgt;
  ti->m_ETF = eTF_8888;
  CGLTexMan::CalcMipsAndSize(ti);
  AddToHash(ti->m_Bind, ti);
  ti->Unlink();
  ti->Link(&STexPic::m_Root);
  gRenDev->m_TexMan->m_StatsCurTexMem += ti->m_Size;

  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, mode, diffuse_size, specular_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
  //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);

  delete [] img;
}

void STexPic::SaveTGA(const char *nam, bool bMips)
{
  char name[256];
  StripExtension(nam, name);

  Set();
  CGLRenderer *r = gcpOGL;
  const GLenum cubefaces[6] = 
  {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
  };
  const char* scubefaces[6] = {"posx","negx","posy","negy","posz","negz"};

  if (m_eTT == eTT_Cubemap)
  {
    for (int CubeSide=0; CubeSide<6; CubeSide++)
    {
      char nm[256];
      sprintf(nm, "%s_%s.tga", name, scubefaces[CubeSide]);
      int tgt = cubefaces[CubeSide];
      int level = 0;
      int w, h;

      glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_WIDTH,  &w);
      glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_HEIGHT, &h);
      if (!w || !h)
        continue;
    
      byte *pDst = new byte [w*h*4];
      glGetTexImage(tgt, level, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

      WriteTGA(pDst, w, h, nm, 32);
      delete [] pDst;
    }
  }
  else
  {
    int tgt = GL_TEXTURE_2D;
    int level = 0;
    int w, h;
    char nm[256];

    if (!bMips)
    {
      sprintf(nm, "%s.tga", name);

      glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_WIDTH,  &w);
      glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_HEIGHT, &h);
      if (w && h)
      {
        byte *pDst = new byte [w*h*4];
      
        glGetTexImage(tgt, level, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

        WriteTGA(pDst, w, h, nm, 32);
        delete [] pDst;
      }
    }
    else
    {
      for (int i=0; i<m_nMips; i++)
      {
        sprintf(nm, "%s[%d].tga", name, i);

        glGetTexLevelParameteriv(tgt, i, GL_TEXTURE_WIDTH,  &w);
        glGetTexLevelParameteriv(tgt, i, GL_TEXTURE_HEIGHT, &h);
        if (w && h)
        {
          byte *pDst = new byte [w*h*4];
        
          glGetTexImage(tgt, i, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

          WriteTGA(pDst, w, h, nm, 32);
          delete [] pDst;
        }
      }
    }
  }
}

void STexPic::SaveJPG(const char *nam, bool bMips)
{
  char name[256];
  StripExtension(nam, name);

//  Set();
  CGLRenderer *r = gcpOGL;
  static const GLenum cubefaces[6] = 
  {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
  };
  static char* scubefaces[6] = {"posx","negx","posy","negy","posz","negz"};

  if (m_eTT == eTT_Cubemap)
  {
    if (!bMips)
    {
      for (int CubeSide=0; CubeSide<6; CubeSide++)
      {
        char nm[256];
        sprintf(nm, "%s_%s.jpg", name, scubefaces[CubeSide]);
        int tgt = cubefaces[CubeSide];
        int level = 0;
        int w, h;

        glGetTexLevelParameteriv(tgt, level, GL_TEXTURE_WIDTH,  &w);
        glGetTexLevelParameteriv(tgt, level, GL_TEXTURE_HEIGHT, &h);
        if (!w || !h)
          continue;
      
        byte *pDst = new byte [w*h*4];
        glGetTexImage(tgt, level, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

        WriteJPG(pDst, w, h, nm);
        delete [] pDst;
      }
    }
    else
    {
      for (int i=0; i<m_nMips; i++)
      {
        for (int CubeSide=0; CubeSide<6; CubeSide++)
        {
          char nm[256];
          sprintf(nm, "%s_%s[%d].jpg", name, scubefaces[CubeSide], i);
          int tgt = cubefaces[CubeSide];
          int level = i;
          int w, h;

          glGetTexLevelParameteriv(tgt, level, GL_TEXTURE_WIDTH,  &w);
          glGetTexLevelParameteriv(tgt, level, GL_TEXTURE_HEIGHT, &h);
          if (!w || !h)
            continue;

          byte *pDst = new byte [w*h*4];
          glGetTexImage(tgt, level, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

          WriteJPG(pDst, w, h, nm);
          delete [] pDst;
        }
      }
    }
  }
  else
  {
    int tgt = GL_TEXTURE_2D;
    int level = 0;
    int w, h;
    char nm[256];
    if (!bMips)
    {
      sprintf(nm, "%s.jpg", name);

      glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_WIDTH,  &w);
      glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_HEIGHT, &h);
      if (w && h)
      {
        byte *pDst = new byte [w*h*4];
      
        glGetTexImage(tgt, level, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

        /*for (int i=0; i<w*h; i++)
        {
          pDst[i*4+0] = pDst[i*4+1] = pDst[i*4+2] = pDst[i*4+3];
        }*/

        WriteJPG(pDst, w, h, nm);
        delete [] pDst;
      }
    }
    else
    {
      for (int i=0; i<m_nMips; i++)
      {
        sprintf(nm, "%s[%d].jpg", name, i);

        glGetTexLevelParameteriv(tgt, i, GL_TEXTURE_WIDTH,  &w);
        glGetTexLevelParameteriv(tgt, i, GL_TEXTURE_HEIGHT, &h);
        if (w && h)
        {
          byte *pDst = new byte [w*h*4];
        
          glGetTexImage(tgt, i, GL_RGBA, GL_UNSIGNED_BYTE, pDst);

          WriteJPG(pDst, w, h, nm);
          delete [] pDst;
        }
      }
    }
  }
}

byte *STexPic::GetData32()
{
  CGLRenderer *r = gcpOGL;
  static const GLenum cubefaces[6] = 
  {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
  };

  byte *pDst = NULL;
  if (m_eTT == eTT_Cubemap)
  {
    glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, m_Bind);
    for (int CubeSide=0; CubeSide<6; CubeSide++)
    {
      int tgt = cubefaces[CubeSide];
      int w, h;

      glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_WIDTH,  &w);
      glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_HEIGHT, &h);
      if (!w || !h)
        continue;
      if (!pDst)
        pDst = new byte [w*h*4*6];

      glGetTexImage(tgt, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pDst[w*h*4*CubeSide]);
    }
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, m_Bind);
    int tgt = GL_TEXTURE_2D;
    int w, h;
    glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_WIDTH,  &w);
    glGetTexLevelParameteriv(tgt, 0, GL_TEXTURE_HEIGHT, &h);
    if (w && h)
    {
      if (!pDst)
        pDst = new byte [w*h*4];

      glGetTexImage(tgt, 0, GL_RGBA, GL_UNSIGNED_BYTE, pDst);
    }
  }
  CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Bind = 0;
  CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target = 0;
  return pDst;
}

void STexPic::ReleaseDriverTexture()
{
  if (!(m_Flags2 & FT2_WASUNLOADED) && (m_Bind && m_Bind != TX_FIRSTBIND)) 
  {
    m_Flags2 &= ~FT2_PARTIALLYLOADED;
    if (m_LoadedSize)
      gRenDev->m_TexMan->m_StatsCurTexMem -= m_LoadedSize;
    else
      gRenDev->m_TexMan->m_StatsCurTexMem -= m_Size;
    if (m_Mips[0])
    {
      int nSides = m_eTT == eTT_Cubemap ? 6 : 1;
      for (int nS=0; nS<nSides; nS++)
      {
        for (int i=0; i<m_nMips; i++)
        {
          SMipmap *mp = m_Mips[nS][i];
          if (!mp)
            continue;
          mp->m_bUploaded = false;
        }
      }
    }
    m_LoadedSize = 0;
    Unlink();
    glDeleteTextures(1, &m_Bind);
  }
}

void STexPic::SetWrapping()
{
  int tgt = m_TargetType;
  if (m_eTT != eTT_Cubemap)
  {
    glTexParameteri(tgt, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(tgt, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (m_Flags & FT_CLAMP)
    {
      glTexParameteri(tgt, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(tgt, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    if (m_Flags2 & FT2_UCLAMP)
      glTexParameteri(tgt, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    if (m_Flags2 & FT2_VCLAMP)
      glTexParameteri(tgt, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }
}

bool STexPic::SetFilter(int nFilter)
{
  Set();
  switch(nFilter)
  {
    case FILTER_LINEAR:
      glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
  	  break;
    case FILTER_BILINEAR:
      glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
      glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
  	  break;
    case FILTER_TRILINEAR:
      glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);      
      break;
    default:
      return false;
  }
  return true;
}

void STexPic::SetFilter()
{
  if (!(m_Flags & FT_NOMIPS))
  {
    if (!(m_Flags2 & FT2_FILTER))
    {
      glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, gcpOGL->m_TexMan->GetMinFilter());
      glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, gcpOGL->m_TexMan->GetMagFilter());
      if (int anf = gcpOGL->GetAnisotropicLevel())
        glTexParameterf(m_TargetType, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)anf);
    }
    else
    {
      switch(m_Flags2 & FT2_FILTER)
      {
        case FT2_FILTER_NEAREST:
          glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
          glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          break;

        case FT2_FILTER_BILINEAR:
          glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
          glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          break;

        case FT2_FILTER_TRILINEAR:
          glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
          glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          break;

        case FT2_FILTER_ANISOTROPIC:
          glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
          glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          if (int anf = gcpOGL->GetAnisotropicLevel())
            glTexParameterf(m_TargetType, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)anf);
          break;
      }
    }
  }
  else
  {
    if (!(m_Flags2 & FT2_FILTER))
    {
      glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
      switch(m_Flags2 & FT2_FILTER)
      {
        case FT2_FILTER_NEAREST:
          glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
          glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          break;

        case FT2_FILTER_BILINEAR:
          glTexParameteri(m_TargetType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(m_TargetType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          break;
      }
    }
  }
}

STexPic *CGLTexMan::GetByID(int Id)
{
  if (Id >= TX_FIRSTBIND)
  {
    int n = Id - TX_FIRSTBIND;
    if (n < m_Textures.Num())
    {
      STexPic *tp = m_Textures[n];
      if (tp && tp->m_Bind == Id)
        return tp;
    }
  }
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it != m_RefTexs.end())
    return it->second;
  return NULL;
}

void CGLTexMan::RemoveFromHash(int Id, STexPic *ti)
{
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it != m_RefTexs.end())
  {
    if (ti)
      assert(ti == it->second);
    m_RefTexs.erase(Id);
  }
}

STexPic *CGLTexMan::AddToHash(int Id, STexPic *ti)
{
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it == m_RefTexs.end())
    m_RefTexs.insert(TTextureMapItor::value_type(Id, ti));
  else
    assert(ti == it->second);
  return ti;
}

void STexPic::SetClamp(bool bEnable)
{
  if (m_Bind == TX_FIRSTBIND)
    return;

  Set();  

  int tgt = m_TargetType;

  if (bEnable)
  {
    glTexParameteri(tgt, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(tgt, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (tgt == GL_TEXTURE_CUBE_MAP_EXT)
      glTexParameteri(tgt, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }
  else
  {
    glTexParameteri(tgt, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(tgt, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (tgt == GL_TEXTURE_CUBE_MAP_EXT)
      glTexParameteri(tgt, GL_TEXTURE_WRAP_R, GL_REPEAT);
  }
  gRenDev->m_TexMan->SetTexture(0, eTT_Base);
}

void CGLTexMan::SetTexture(int Id, ETexType eTT)
{
  if(Id < 0 || Id >= TX_LASTBIND)
  {
    iLog->Log("Error: CGLTexMan::SetTexture: Texture id is out of range: %d", Id);
    return;
  }
  STexPic *tp = GetByID(Id);
  if (tp)
  {
    tp->Set();
    //tp->SaveJPG("hud.jpg", true);
    return;
  }

  if (Id < 0)
    return;

  //PROFILE_FRAME_TOTAL(Texture_Changes);

  int i = m_CurStage;
  if (CRenderer::CV_r_log == 3 || CRenderer::CV_r_log == 4)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "CGLTexMan::SetTexture: (%d) \"%d\"\n", i, Id);
  if (CGLTexMan::m_TUState[i].m_Bind != Id)
  {
    CGLTexMan::m_TUState[i].m_Bind = Id;
    int tgt;
    switch(eTT)
    {
      case eTT_Cubemap:
        tgt = GL_TEXTURE_CUBE_MAP_EXT;
        break;
      case eTT_Rectangle:
        if (SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
          tgt = GL_TEXTURE_RECTANGLE_NV;
        else
          tgt = GL_TEXTURE_2D;
        break;
      case eTT_3D:
        tgt = GL_TEXTURE_3D_EXT;
        break;
      default:
        tgt = GL_TEXTURE_2D;
        break;
    }

    if (Id)
      SetTextureTarget(i, tgt);
    else
      ResetTextureTarget(i);
    glBindTexture(tgt, Id);
  }
}

void STexPic::Set(int nTexSlot)
{
  //PROFILE_FRAME_TOTAL(Texture_Changes);

  static int sRecursion = 0;
  if (!sRecursion)
  {
    if (CRenderer::CV_r_texbindmode>=2)
    {
      if (CRenderer::CV_r_texbindmode==2 && (m_Flags2 & FT2_WASLOADED) && m_eTT == eTT_Base)
      {
        if (sRecursion)
          return;
        sRecursion++;
        gRenDev->m_TexMan->SetGridTexture(this);
        sRecursion--;
        return;
      }
      if (CRenderer::CV_r_texbindmode==3 && (m_Flags2 & FT2_WASLOADED) && m_eTT == eTT_Bumpmap)
      {
        if (sRecursion)
          return;
        sRecursion++;
        gRenDev->m_TexMan->SetGridTexture(this);
        sRecursion--;
        return;
      }
      if (CRenderer::CV_r_texbindmode==4 && (m_Flags2 & FT2_WASLOADED) && m_eTT == eTT_Base)
      {
        if (sRecursion)
          return;
        sRecursion++;
        gRenDev->m_TexMan->m_Text_Gray->Set();
        sRecursion--;
        return;
      }
      if (CRenderer::CV_r_texbindmode==5 && nTexSlot==EFTT_DIFFUSE)
      {
        sRecursion++;
        gRenDev->m_TexMan->m_Text_White->Set();
        sRecursion--;
        return;
      }
    }
  }

  assert(m_Bind >= 0 && m_Bind < TX_LASTBIND);
  if ((m_Flags2 & (FT2_WASUNLOADED | FT2_PARTIALLYLOADED)))
  {
    int Size = m_LoadedSize;
    Restore();
    if (Size != m_LoadedSize)
      CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Bind = -1;
  }
  else
    Relink(&STexPic::m_Root);
  int i = gRenDev->m_TexMan->m_CurStage;
  if (CRenderer::CV_r_log == 3 || CRenderer::CV_r_log == 4)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "STexPic::Set(): (%d) \"%s\"\n", i, m_SourceName.c_str());
  if (m_AccessFrame != gRenDev->GetFrameID())
  {
    m_AccessFrame = gRenDev->GetFrameID();
    gRenDev->m_RP.m_PS.m_NumTextures++;
    gRenDev->m_RP.m_PS.m_TexturesSize += m_Size;
  }
  //if (m_Bind != 0x1000)
  //  assert (m_Size == BindSizes[m_Bind]);

  if (CGLTexMan::m_TUState[i].m_Bind != m_Bind)
  {
    CGLTexMan::m_TUState[i].m_Bind = m_Bind;
    int tgt = m_TargetType;
    CGLTexMan::SetTextureTarget(i, tgt);
    glBindTexture(tgt, m_Bind);
  }
}

void WriteTGA8(byte *data8, int width, int height, char *filename);

int SShaderTexUnit::mfSetTexture(int nt)
{
  CGLRenderer *rd = gcpOGL;
  int tgt = GL_TEXTURE_2D;
  
  if (nt >= 0)
    rd->EF_SelectTMU(nt);

  SShaderTexUnit *pSTU = this;

  int nSetID = -1;
  if (m_TexPic && m_TexPic->m_Bind < EFTT_MAX)
  {
    if (m_TexPic->m_Bind >= EFTT_LIGHTMAP && m_TexPic->m_Bind <= EFTT_OCCLUSION)
    {
      if (m_TexPic->m_Bind == EFTT_LIGHTMAP && rd->m_RP.m_pCurObject->m_nLMId)
        nSetID = rd->m_RP.m_pCurObject->m_nLMId;
      else
      if (m_TexPic->m_Bind == EFTT_LIGHTMAP_DIR && rd->m_RP.m_pCurObject->m_nLMDirId)
        nSetID = rd->m_RP.m_pCurObject->m_nLMDirId;
      else
      if (m_TexPic->m_Bind == EFTT_OCCLUSION && rd->m_RP.m_pCurObject->m_nOcclId)
        nSetID = rd->m_RP.m_pCurObject->m_nOcclId;
    }
    else
    if (nSetID < 0)
    {
      if (!rd->m_RP.m_pShaderResources || !rd->m_RP.m_pShaderResources->m_Textures[m_TexPic->m_Bind])
        iLog->Log("WARNING: SShaderTexUnit::mfSetTexture: Missed template texture '%s' for shader '%s'\n", gRenDev->m_cEF.mfTemplateTexIdToName(m_TexPic->m_Bind), rd->m_RP.m_pShader->GetName());
      else
      {
        pSTU = &rd->m_RP.m_pShaderResources->m_Textures[m_TexPic->m_Bind]->m_TU;
        rd->m_RP.m_pShaderResources->m_Textures[m_TexPic->m_Bind]->Update(nt);
      }
    }
  }

  if (pSTU->m_AnimInfo)
    pSTU->mfUpdate();

  if (pSTU->m_TexPic)
  {
    tgt = pSTU->m_TexPic->m_TargetType;
    if (nSetID > 0)
		{
			rd->m_TexMan->SetTexture(nSetID, eTT_Base);
			tgt = GL_TEXTURE_2D;
		}
    else
    {
      int bind = pSTU->m_TexPic->m_Bind;
      if (bind >= TX_FIRSTBIND)
      {
        pSTU->m_TexPic->Set();
        //pSTU->m_TexPic->SaveJPG("Bug.jpg", true);
      }
      else
      {
        switch (bind)
        {
          case TO_FROMRE0:
          case TO_FROMRE1:
          case TO_FROMRE2:
          case TO_FROMRE3:
          case TO_FROMRE4:
          case TO_FROMRE5:
          case TO_FROMRE6:
          case TO_FROMRE7:
            {
              if (rd->m_RP.m_pRE)
                bind = rd->m_RP.m_pRE->m_CustomTexBind[bind-TO_FROMRE0];
              else
                bind = rd->m_RP.m_RECustomTexBind[bind-TO_FROMRE0];
              if (bind < 0)
                return 0;
              rd->SetTexture(bind, pSTU->m_TexPic->m_eTT);
              /*{
                int width;
                int height;
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &width);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
                byte *pic = new byte [width * height];
                glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, pic);
                char buff[128];
                sprintf(buff, "ShadowMap.tga");
                WriteTGA8(pic,width,height,buff); 
                delete [] pic;
              }*/
            }
            break;

          case TO_FROMOBJ:
            {
              if (rd->m_RP.m_pCurObject)
                bind = rd->m_RP.m_pCurObject->m_NumCM;
              if (bind <= 0)
                return 0;
              rd->SetTexture(bind, eTT_Base);
            }
            break;

          case TO_FROMLIGHT:
            {
              bool bRes = false;
              if (rd->m_RP.m_nCurLight < rd->m_RP.m_DLights[SRendItem::m_RecurseLevel].Num())
              {
                CDLight *dl = rd->m_RP.m_pCurLight;
                if (dl && dl->m_pLightImage!=0)
                {
                  bRes = true;
                  STexPic *tp = (STexPic *)((ITexPic*)dl->m_pLightImage);
                  //dl->m_NumCM = 0;
                  if (dl->m_NumCM >= 0)
                    tp = gRenDev->m_TexMan->m_CustomCMaps[dl->m_NumCM].m_Tex;
                  else
                  if (dl->m_fAnimSpeed)
                  {
                    int n = 0;
                    STexPic *t = tp;
                    while (t)
                    {
                      t = t->m_NextTxt;
                      n++;
                    }
                    if (n > 1)
                    {
                      int m = (int)(gRenDev->m_RP.m_RealTime / dl->m_fAnimSpeed) % n;
                      for (int i=0; i<m; i++)
                      {
                        tp = tp->m_NextTxt;
                      }
                    }
                  }
									if(tp)
                  {
										tp->Set();
                    //tp->SaveJPG("CubeProj.jpg", true);
                  }
                  else
                    assert (tp);
                  //tp->SaveJPG("CubeLight");
                }
              }
              if (!bRes)
                iLog->Log("Warning: Couldn't set projected texture for %d light source (Shader: '%s')\n", rd->m_RP.m_nCurLight, rd->m_RP.m_pShader->m_Name.c_str());
            }
            break;

          case TO_ENVIRONMENT_CUBE_MAP:
            {
              SEnvTexture *cm = NULL;
              cm = gRenDev->m_cEF.mfFindSuitableEnvCMap(rd->m_RP.m_pCurObject->GetTranslation(), true, 0, 0);
              if (cm)
                cm->m_Tex->Set();
              else
                return 0;
            }
            break;

        case TO_ENVIRONMENT_LIGHTCUBE_MAP:
          {
            SEnvTexture *cm = NULL;
            cm = gRenDev->m_cEF.mfFindSuitableEnvLCMap(rd->m_RP.m_pCurObject->GetTranslation(), true, 0, 0);
            if (cm)
              cm->m_Tex->Set();
            else
              return false;
          }
          break;

          case TO_ENVIRONMENT_TEX:
            {
              SEnvTexture *cm = NULL;
              CCamera cam = rd->GetCamera();
              Vec3d Angs = cam.GetAngles();
              Vec3d Pos = cam.GetPos();
              bool bReflect = false;
              if ((gRenDev->m_RP.m_pShader->m_Flags3 & (EF3_CLIPPLANE_FRONT | EF3_REFLECTION)))
                bReflect = true;
              cm = gRenDev->m_cEF.mfFindSuitableEnvTex(Pos, Angs, true, 0, false, gRenDev->m_RP.m_pShader, gRenDev->m_RP.m_pShaderResources, gRenDev->m_RP.m_pCurObject, bReflect, gRenDev->m_RP.m_pRE);
              if (cm)
                cm->m_Tex->Set();
              else
                return false;
            }
            break;

          default:
            {
              if (bind >= TO_CUSTOM_CUBE_MAP_FIRST && bind <= TO_CUSTOM_CUBE_MAP_LAST)
              {
                SEnvTexture *cm = &gRenDev->m_TexMan->m_CustomCMaps[bind-TO_CUSTOM_CUBE_MAP_FIRST];
                if (!cm->m_bReady)
                {
                  iLog->Log("Warning: Custom CubeMap %d doesn't ready\n", bind-TO_CUSTOM_CUBE_MAP_FIRST);
                  return 0;
                }
                cm->m_Tex->Set();
              }
              else
              if (bind >= TO_CUSTOM_TEXTURE_FIRST && bind <= TO_CUSTOM_TEXTURE_LAST)
              {
                SEnvTexture *cm = &gRenDev->m_TexMan->m_CustomTextures[bind-TO_CUSTOM_TEXTURE_FIRST];
                if (!cm->m_bReady)
                {
                  iLog->Log("Warning: Custom Texture %d doesn't ready\n", bind-TO_CUSTOM_TEXTURE_FIRST);
                  return 0;
                }
                cm->m_Tex->Set();
              }
              else
              if (pSTU->m_TexPic->m_TargetType)
                pSTU->m_TexPic->Set();
            }
            break;
        }
      }
    }

    if (pSTU->m_fTexFilterLodBias != CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_fTexFilterLodBias)
    {
      CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_fTexFilterLodBias = pSTU->m_fTexFilterLodBias;
      glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, pSTU->m_fTexFilterLodBias);
    }
  }
  else
  if (CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target)
  {
    glDisable(CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target);
    CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Target = 0;
    CGLTexMan::m_TUState[gRenDev->m_TexMan->m_CurStage].m_Bind = 0;
  }

  if (m_GTC)
  {
    if (!m_GTC->mfSet(true))
      return 0;
    if (m_GTC->m_bDependsOnObject)
    {
      rd->m_RP.m_pGTC[nt] = m_GTC;
      rd->m_RP.m_FrameGTC = rd->m_RP.m_Frame;
    }
    else
      rd->m_RP.m_pGTC[nt] = NULL;
  }
  else
    rd->m_RP.m_pGTC[nt] = NULL;

  if (m_eColorOp != eCO_NOSET)
  {
    rd->m_RP.m_TexStages[nt].m_CO = m_eColorOp;
    rd->m_RP.m_TexStages[nt].m_AO = m_eAlphaOp;
    rd->m_RP.m_TexStages[nt].m_CA = m_eColorArg;
    rd->m_RP.m_TexStages[nt].m_AA = m_eAlphaArg;
  }

  return tgt;
}

bool SShaderPass::mfSetTextures()
{
  int i;
  for (i=0; i<m_TUnits.Num() && i<gcpOGL->m_MaxActiveTexturesARB_VP; i++)
  {
    SShaderTexUnit *tl = &m_TUnits[i];
    tl->mfSetTexture(i);
  }
  CGLTexMan::BindNULL(i);
  return true;
}

void SShaderPass::mfResetTextures()
{
  int i;
  for (i=0; i<m_TUnits.Num(); i++)
  {
    SShaderTexUnit *tl = &m_TUnits[i];
    if (tl->m_GTC)
    {
      gcpOGL->EF_SelectTMU(i);
      tl->m_GTC->mfSet(false);
    }
  }
}

CGLTexMan::~CGLTexMan()
{
  if (m_PBuffer_256)
  {
    delete m_PBuffer_256;
    m_PBuffer_256 = NULL;
  }
  if (m_EnvPBuffer)
  {
    delete m_EnvPBuffer;
    m_EnvPBuffer = NULL;
  }
  for (int i=0; i<m_BufRegions.Num(); i++)
  {
    SBufRegion *br = &m_BufRegions[i];
    if (br->m_BRHandle)
    {
      wglDeleteBufferRegionARB(br->m_BRHandle);
      br->m_BRHandle = 0;
    }
  }
  m_BufRegions.Free();
}


STexPic *CGLTexMan::CreateTexture()
{
#ifdef DEBUGALLOC
#undef new
#endif
  return new STexPic;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
}

bool CGLTexMan::SetFilter(char *tex)  
{
  int i;
  struct textype
  {
    char *name;
    uint typemin;
    uint typemag;
  };

  static textype tt[] =
  {
    {"GL_NEAREST", GL_NEAREST, GL_NEAREST},
    {"GL_LINEAR", GL_LINEAR, GL_LINEAR},
    {"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
    {"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
    {"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
    {"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
  };

  strcpy(m_CurTexFilter, tex);
  m_CurAnisotropic = CLAMP(CRenderer::CV_r_texture_anisotropic_level, 1, gcpOGL->m_MaxAnisotropicLevel);
  CRenderer::CV_r_texture_anisotropic_level = m_CurAnisotropic;
  if ((gRenDev->GetFeatures() & RFT_ALLOWANISOTROPIC) && m_CurAnisotropic > 1)
  {
    CGLRenderer::CV_gl_texturefilter->Set("GL_LINEAR_MIPMAP_LINEAR");
    tex = CGLRenderer::CV_gl_texturefilter->GetString();
  }

  for (i=0; i<6; i++)
  {
    if ( !stricmp(tex, tt[i].name) )
    {
      m_MinFilter = tt[i].typemin;
      m_MagFilter = tt[i].typemag;
      for (i=0; i<m_Textures.Num(); i++)
      {
        if (m_Textures[i] && m_Textures[i]->m_bBusy && !(m_Textures[i]->m_Flags & FT_NOMIPS))
        {
          gRenDev->SetTexture(m_Textures[i]->m_Bind);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetMinFilter());
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetMagFilter());
          if (int anf = gcpOGL->GetAnisotropicLevel())
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)anf);
        }
      }
      return true;
    }
  }
  iLog->Log("Warning: Bad texture filter name <%s>\n", tex);
  return false;
}

void CGLTexMan::CalcMipsAndSize(STexPic *ti)
{
  ti->m_nMips = 0;
  ti->m_Size = 0;
  int wdt = ti->m_Width;
  int hgt = ti->m_Height;
  int depth = ti->m_Depth;
  int mode = GetTexDstFormat(ti->m_ETF);
  while (wdt || hgt || depth)
  {
    if (!wdt)
      wdt = 1;
    if (!hgt)
      hgt = 1;
    if (!depth)
      depth = 1;
    ti->m_nMips++;
    ti->m_Size += CGLTexMan::TexSize(wdt,hgt,depth,mode);
    if (ti->m_Flags & FT_NOMIPS)
      break;
    wdt >>= 1;
    hgt >>= 1;
    depth >>= 1;
  }
}

ETEX_Format CGLTexMan::GetTexFormat(int GLFormat)
{
  switch(GLFormat)
  {
    case GL_COLOR_INDEX8_EXT:
      return eTF_Index;
    case GL_RGB8:
      return eTF_0888;
    case GL_RGBA8:
      return eTF_8888;
    case GL_RGBA4:
      return eTF_4444;
    case GL_RGB5:
      return eTF_0555;
    case GL_ALPHA:
      return eTF_8000;
    case GL_LUMINANCE_ALPHA:
      return eTF_0088;
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      return eTF_DXT1;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      return eTF_DXT3;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      return eTF_DXT5;
    case GL_SIGNED_HILO16_NV:
      return eTF_SIGNED_HILO16;
    case GL_SIGNED_HILO_NV:
      return eTF_SIGNED_HILO8;
    case GL_SIGNED_RGB8_NV:
      return eTF_SIGNED_RGB8;
    case GL_DSDT_MAG_NV:
      return eTF_DSDT_MAG;
    case GL_DSDT_NV:
      return eTF_DSDT;
    case GL_HILO_NV:
      return eTF_V8U8;
    case GL_HILO16_NV:
      return eTF_V16U16;
    case GL_ALPHA8:
      return eTF_8000;
    default:
      assert(0);
  }
  return eTF_Unknown;
}

int CGLTexMan::GetTexDstFormat(ETEX_Format eTF)
{
  switch (eTF)
  {
    case eTF_Index:
      return GL_COLOR_INDEX8_EXT;
    case eTF_0888:
      return GL_RGB8;
    case eTF_8888:
      return GL_RGBA8;
    case eTF_4444:
      return GL_RGBA4;
    case eTF_0555:
      return GL_RGB5;
    case eTF_8000:
      return GL_ALPHA;
    case eTF_0088:
      return GL_LUMINANCE_ALPHA;
    case eTF_DXT1:
      return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case eTF_DXT3:
      return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    case eTF_DXT5:
      return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case eTF_SIGNED_HILO16:
      return GL_SIGNED_HILO16_NV;
    case eTF_SIGNED_HILO8:
      return GL_SIGNED_HILO_NV;
    case eTF_SIGNED_RGB8:
      return GL_SIGNED_RGB8_NV;
    case eTF_RGB8:
      return GL_RGB8;
    case eTF_DSDT_MAG:
      return GL_DSDT_MAG_NV;
    case eTF_DSDT:
      return GL_DSDT_NV;
    case eTF_V8U8:
      return GL_HILO_NV;
    case eTF_V16U16:
      return GL_HILO16_NV;
    default:
      assert(0);
  }
  return 0;
}

int CGLTexMan::GetTexSrcFormat(ETEX_Format eTF)
{
  switch (eTF)
  {
    case eTF_Index:
      return GL_COLOR_INDEX;
    case eTF_0888:
      return GL_RGB;
    case eTF_0555:
      return GL_RGB;
    case eTF_8888:
      return GL_RGBA;
    case eTF_4444:
      return GL_RGBA;
    case eTF_0088:
      return GL_LUMINANCE_ALPHA;
    case eTF_8000:
      return GL_ALPHA;
    case eTF_DXT1:
      return GL_RGB;
    case eTF_DXT3:
      return GL_RGBA;
    case eTF_DXT5:
      return GL_RGBA;
    case eTF_SIGNED_HILO16:
      return GL_SIGNED_HILO16_NV;
    case eTF_SIGNED_HILO8:
      return GL_SIGNED_HILO_NV;
    case eTF_SIGNED_RGB8:
      return GL_SIGNED_RGB8_NV;
    case eTF_RGB8:
      return GL_RGB;
    case eTF_DSDT_MAG:
      return GL_DSDT_MAG_NV;
    case eTF_DSDT:
      return GL_DSDT_NV;
    default:
      assert(0);
  }
  return 0;
}

int STexPic::DstFormatFromTexFormat(ETEX_Format eTF)
{
  return CGLTexMan::GetTexDstFormat(eTF);
}
int STexPic::TexSize(int Width, int Height, int DstFormat)
{
  return CGLTexMan::TexSize(Width, Height, 1, DstFormat);
}

int CGLTexMan::TexSize(int wdt, int hgt, int depth, int mode)
{
  switch (mode)
  {
    case GL_RGB8:
    case GL_RGB:
    case 3:
    case GL_BGR_EXT:
      return wdt * hgt * depth * 3;

    case GL_RGBA8:
    case GL_RGBA:
    case GL_BGRA_EXT:
    case 4:
      return wdt * hgt * depth * 4;

    case GL_RGBA4:
    case GL_RGB5:
      return wdt * hgt * depth * 2;

    case GL_COMPRESSED_ALPHA_ARB:
      return wdt * hgt * depth;

    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      {
        int blockSize = (mode == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || mode == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
        return ((wdt+3)/4)*((hgt+3)/4)*blockSize;
      }

    case 0x83a0:
      {
        int blockSize = 8;
        return ((wdt+3)/4)*((hgt+3)/4)*blockSize;
      }

    case GL_COLOR_INDEX8_EXT:
      return wdt * hgt * depth;

    case GL_LUMINANCE_ALPHA:
      return wdt * hgt * depth * 2;

    case GL_DSDT_MAG_NV:
      return wdt * hgt * depth * 3;

    case GL_DSDT_NV:
      return wdt * hgt * depth * 2;

    case GL_ALPHA:
    case GL_ALPHA8:
      return wdt * hgt * depth;

    case GL_DEPTH_COMPONENT24_SGIX:
      return wdt * hgt * 3;

    case GL_DEPTH_COMPONENT16_SGIX:
      return wdt * hgt * 2;

    case GL_LUMINANCE8:
      return wdt * hgt * depth;

    case GL_DEPTH_COMPONENT:
      return wdt * hgt * depth;

    case GL_HILO16_NV:
    case GL_SIGNED_HILO16_NV:
      return wdt * hgt * depth * 4;

    case GL_HILO_NV:
    case GL_SIGNED_HILO_NV:
      return wdt * hgt * depth * 2;

    default:
			assert(0);
      break;
      
  }
  return 0;
}

#if DO_ASM

#pragma warning(push)
#pragma warning(disable:4731) // frame pointer register 'ebp' modified by inline assembly code

_inline byte *ASM_BuildMipLine(byte *src1, byte *dst1, int wd, int wdt)
{
  __asm
  {
    push ebp
    mov edi, dst1
    mov esi, src1
    mov ebx, wd
    mov ebp, wdt

    xor eax, eax
    xor ecx, ecx
    xor edx, edx
    jmp ll
    align 16
ll:
    mov al, [esi]
    mov dl, [esi+4]
    mov cl, [esi+ebx]
    add eax, edx
    add eax, ecx
    mov dl, [esi+ebx+4]
    mov cl, [esi+ebx+1]
    add eax, edx
    shr eax, 2
    mov dl, [esi+5]
    mov [edi], al
    mov al, [esi+1]
    add eax, edx
    add eax, ecx
    mov dl, [esi+ebx+5]
    mov cl, [esi+ebx+2]
    add eax, edx
    shr eax, 2
    mov dl, [esi+6]
    mov [edi+1], al
    mov al, [esi+2]
    add eax, edx
    add eax, ecx
    mov dl, [esi+ebx+6]
    mov cl, [esi+ebx+3]
    add eax, edx
    shr eax, 2
    mov dl, [esi+7]
    mov [edi+2], al
    mov al, [esi+3]
    add eax, edx
    add eax, ecx
    mov dl, [esi+ebx+7]
    add edi, 4
    add eax, edx
    add esi, 8
    shr eax, 2
    dec ebp
    mov [edi-1], al
    jne ll
    pop ebp
    mov eax, edi
  }
  //return dst1;
}
#pragma warning(pop)

#endif

_inline uint filter3x3(uint* data, int x, int y, int dx, int dy)
{
  int r, g, b, a;

  static int filter[3][3] =
  {
    {0, 1, 0},
    {1, 2, 1},
    {0, 1, 0}
  };

  r = b = g = a = 0;

  for (int i=0; i<3; i++)
  {
    for (int j=0; j<3; j++)
    {
      uint col = data[((y + dy + i - 1) % dy) * dx + ((x + dx + j - 1) % dx)];
      r += ( col        & 0xff) << filter[i][j];
      g += ((col >> 8)  & 0xff) << filter[i][j];
      b += ((col >> 16) & 0xff) << filter[i][j];
      a += ((col >> 24) & 0xff) << filter[i][j];
    }
  }

  r >>= 4; g >>= 4; b >>= 4; a >>= 4;

  return r + (g<<8) + (b<<16) + (a<<24);
}

void CGLTexMan::GenerateMips_SW(GLenum tgt, byte* src, int wdt, int hgt, int mode, STexPic *ti)
{
  int wd;
  int i, j;
  byte *src1, *dst1;
  int num;

  glTexImage2D(tgt, 0, mode, wdt, hgt, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
  num = 1;
  while (wdt!=1 && hgt!=1)
  {
    wd = wdt<<2;
    wdt >>= 1;
    hgt >>= 1;
    if (wdt < 1)
      wdt = 1;
    if (hgt < 1)
      hgt = 1;
    if (CRenderer::CV_r_texsimplemips || (ti->m_Flags2 & FT2_FORCEMIPS2X2))
    {
      src1 = dst1 = src;
      for (i=0; i<hgt; i++)
      {
#if !DO_ASM
        byte *src2 = src1;
        for (j=0; j<wdt; j++)
        {
          dst1[0] = (src2[0]+src2[4]+src2[wd]+src2[wd+4])>>2;
          dst1[1] = (src2[1]+src2[5]+src2[wd+1]+src2[wd+5])>>2;
          dst1[2] = (src2[2]+src2[6]+src2[wd+2]+src1[wd+6])>>2;
          dst1[3] = (src2[3]+src2[7]+src2[wd+3]+src2[wd+7])>>2;
          dst1 += 4;
          src2 += 8;
        }
#else
        dst1 = ASM_BuildMipLine(src1, dst1, wd, wdt);
#endif
        src1 += wd<<1;
      }
      glTexImage2D(tgt, num, mode, wdt, hgt, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
    }
    else
    {
      uint *ds = new uint[wdt*hgt];
      memset(ds, 0, wdt*hgt*4);
      uint *ds1 = ds;
      for (i=0; i<hgt; i++)
      {
        for (j=0; j<wdt; j++)
        {
          *ds++ = filter3x3((uint *)src, j<<1, i<<1, wdt<<1, hgt<<1);
        }
      }
      glTexImage2D(tgt, num, mode, wdt, hgt, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, ds1);
      memcpy(src, ds1, wdt*hgt*4);
      delete [] ds1;
    }
    num++;
  }
}

static inline bool IsDXTFormat(int format)
{
  if (format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
      format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
      format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
      format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
    return true;
  return false;
}

void CGLTexMan::BuildMips(GLenum tgt, byte* src, int wdt, int hgt, int depth, STexPic *ti, int srcFormat, int dstFormat, int blockSize, int DXTSize, int nMips)
{
  int offset = 0;
  ti->m_nMips = 0;
  if (nMips)
  {
    signed char *data = (signed char *)src;
    int w = wdt;
    int h = hgt;
    if (dstFormat == GL_DSDT_MAG_NV)
    {
      for (int l=0; l<nMips; l++)
      {
        if (!w)
          w = 1;
        if (!h)
          h = 1;
        float *fd = new float[w*h*3];
        for (int i=0; i<w*h; i++)
        {
          fd[i*3+0] = data[i*4+0]/127.0f;
          fd[i*3+1] = data[i*4+1]/127.0f;
          fd[i*3+2] = data[i*4+2]/127.0f;
        }
        glTexImage2D(GL_TEXTURE_2D, l, GL_DSDT_MAG_NV, w, h, 0, GL_DSDT_MAG_NV, GL_FLOAT, fd);
        delete [] fd;
        data += w*h*4;
        ti->m_Size += w*h*3;
        w >>= 1;
        h >>= 1;
      }
    }
    else
    if (dstFormat == GL_COLOR_INDEX8_EXT && srcFormat == GL_COLOR_INDEX)
    {
      for (int l=0; l<nMips; l++)
      {
        if (!w)
          w = 1;
        if (!h)
          h = 1;
        glTexImage2D(tgt, l, dstFormat, w, h, 0, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, data );
        data += w*h;
        ti->m_Size += w*h;
        w >>= 1;
        h >>= 1;
      }
    }
    else
    {
      if (dstFormat == GL_SIGNED_HILO_NV || dstFormat == GL_SIGNED_HILO16_NV || dstFormat == GL_HILO_NV)
      {
        for (int l=0; l<nMips; l++)
        {
          if (!w)
            w = 1;
          if (!h)
            h = 1;
          glTexImage2D(tgt, l, dstFormat, w, h, 0, GL_HILO_NV, GL_BYTE, data);
          data += w*h*2;
          ti->m_Size += w*h*2;
          w >>= 1;
          h >>= 1;
        }
      }
      else
      if (ti->m_eTT == eTT_DSDTBump)
      {
        srcFormat = GL_RGBA;
        for (int l=0; l<nMips; l++)
        {
          if (!w)
            w = 1;
          if (!h)
            h = 1;
          /*for (int i=0; i<w*h; i++)
          {
            Exchange(data[i*4+2], data[i*4+0]);
          }*/
          glTexImage2D(tgt, l, dstFormat, w, h, 0, srcFormat, GL_BYTE, data);
          data += w*h*4;
          ti->m_Size += w*h*3;
          w >>= 1;
          h >>= 1;
        }
      }
      else
      if (IsDXTFormat(dstFormat) && dstFormat == srcFormat)
      {
        int l = 0;
        int size;
        while (w>0 || h>0)
        {
          if (offset >= DXTSize)
            break;
          ti->m_nMips++;
          if (w == 0)
            w = 1;
          if (h == 0)
            h = 1;

          size = ((w+3)/4)*((h+3)/4)*blockSize;

          ti->m_Size += size;
          glCompressedTexImage2DARB(tgt, l, dstFormat, w, h, 0, size, src + offset);
          
          l++;
          offset += size;
          w >>= 1;
          h >>= 1;
        }
        if (l == 1)
          ti->m_Flags |= FT_NOMIPS;
        else
        {
          if (w <= 2 && h <= 2)
          {
            offset -= size;
            while (w>0 || h>0)
            {
              ti->m_nMips++;
              if (w == 0)
                w = 1;
              if (h == 0)
                h = 1;

              ti->m_Size += size;
              glCompressedTexImage2DARB(tgt, l, dstFormat, w, h, 0, size, src + offset);

              l++;
              offset += size;
              w >>= 1;
              h >>= 1;
            }
          }
          assert (!w && !h);
		      if(w || h)
			      Warning(0, ti->GetName(), "CGLTexMan::BuildMips_DXT: Texture has no requested mips: %s", ti->GetName());
        }
      }
      else
      {
        for (int l=0; l<nMips; l++)
        {
          if (!w)
            w = 1;
          if (!h)
            h = 1;
          glTexImage2D(tgt, l, dstFormat, w, h, 0, srcFormat, GL_UNSIGNED_BYTE, data);
          data += TexSize(w, h, depth, srcFormat);
          ti->m_Size += TexSize(w, h, depth, dstFormat);
          w >>= 1;
          h >>= 1;
        }
      }
    }
  }
  else
  if (CRenderer::CV_r_texhwmipsgeneration && SUPPORTS_GL_SGIS_generate_mipmap)
  {
    if (tgt >= GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT && tgt <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT)
      glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    else
      glTexParameteri(tgt, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

    if (tgt == GL_TEXTURE_3D_EXT)
      glTexImage3DEXT(tgt, 0, dstFormat, wdt, hgt, depth, 0, srcFormat, GL_UNSIGNED_BYTE, src);  
    else
      glTexImage2D(tgt, 0, dstFormat, wdt, hgt, 0, srcFormat, GL_UNSIGNED_BYTE, src);  
  }
  else
  {
    GenerateMips_SW(tgt, src, wdt, hgt, dstFormat, ti);
  }
  CalcMipsAndSize(ti);
}

void CGLTexMan::BuildMips8(GLenum tgt, STexPic *ti, byte *data, bool bSub)
{
  GLuint SourceFormat   = GL_COLOR_INDEX;
  GLuint InternalFormat = GL_COLOR_INDEX8_EXT;
  int width = ti->m_Width;
  int height = ti->m_Height;

  ti->m_Size = 0;
  if (bSub)
    glTexSubImage2D(tgt, 0, 0, 0, width, height, SourceFormat, GL_UNSIGNED_BYTE, data);
  else
  {
    glTexImage2D(tgt, 0, InternalFormat, width, height, 0, SourceFormat, GL_UNSIGNED_BYTE, data);
    ti->m_Size += TexSize(width, height, 1, InternalFormat);
  }

  int   miplevel;
  miplevel = 0;

  byte *out = new byte [width*height];
  byte *outRet = out;
  ti->m_nMips = 0;
  while (width > 1 || height > 1)
  {
    ti->m_nMips++;
    MipMap8Bit (ti, data, out, width, height);
    width >>= 1;
    height >>= 1;
    if (width < 1)
      width = 1;
    if (height < 1)
      height = 1;
    miplevel++;
    if (bSub)
      glTexSubImage2D(tgt, miplevel, 0, 0, width, height, SourceFormat, GL_UNSIGNED_BYTE, out);
    else
    {
      glTexImage2D(tgt, miplevel, InternalFormat, width, height, 0, SourceFormat, GL_UNSIGNED_BYTE, out);
      ti->m_Size += TexSize(width, height, 1, InternalFormat);
    }
    Exchange(out, data);
  }
  delete [] outRet;
}

byte *CGLTexMan::GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips)
{
  int mode = 0;

  uint tnum = 0;
  glGenTextures(1, &tnum);	
  assert(tnum<14000);
  SetTexture(tnum, eTT_Base);

  if (eF == eIF_DXT1)
    mode = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
  else
  if (eF == eIF_DXT3)
    mode = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
  else
  if (eF == eIF_DXT5)
    mode = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      
  int wdt = ti->m_Width;
  int hgt = ti->m_Height;
  int Size = TexSize(wdt,hgt,1,mode);
  if (SUPPORTS_GL_SGIS_generate_mipmap)
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
  else
    return NULL;
  
  glTexImage2D(GL_TEXTURE_2D, 0, mode, wdt, hgt, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, dst);
  int nMips = 1;
  while (wdt>1 || hgt>1)
  {
    wdt >>= 1;
    hgt >>= 1;
    if (wdt < 1)
      wdt = 1;
    if (hgt < 1)
      hgt = 1;
    Size += TexSize(wdt,hgt,1,mode);
    if (!bMips)
      break;
    nMips++;
  }
  ti->m_nMips = nMips;
  *DXTSize = Size;
  *numMips = nMips;

  int mip_size = 0;
  int level = 0;
  int w = 0, h = 0;
  byte *data = new byte [Size];
  int nOffs = 0;
  do
  {
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH,  &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_HEIGHT, &h);
    if (!w || !h)
    {
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
      glDeleteTextures(1, &tnum);
      delete [] data;
      return NULL;
    }
    
    mip_size=0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_IMAGE_SIZE_ARB, &mip_size);

    glGetCompressedTexImageARB(GL_TEXTURE_2D, level, &data[nOffs]);
    nOffs += mip_size;
    
    level++;
    if (!bMips)
      break;
  }
  while((w!=1 || h!=1));

  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
  glDeleteTextures(1, &tnum);
  SetTexture(0, eTT_Base);

  return data;
}

//TArray<STexPic *> sTestStr;
//TArray<STexPic *> sTestTx;

STexPic *CGLTexMan::CopyTexture(const char *name, STexPic *tiSrc, int CubeSide)
{
  STexPic *ti = TextureInfoForName(name, -1, tiSrc->m_eTT, tiSrc->m_Flags, tiSrc->m_Flags2, 0);

  ti->m_bBusy = true;
  ti->m_Flags = tiSrc->m_Flags;
  ti->m_Flags2 = tiSrc->m_Flags2;
  ti->m_Bind = TX_FIRSTBIND + ti->m_Id;
  AddToHash(ti->m_Bind, ti);
  ti->m_Width = tiSrc->m_Width;
  ti->m_Height = tiSrc->m_Height;
  ti->m_nMips = tiSrc->m_nMips;
  ti->m_ETF = tiSrc->m_ETF;
  ti->m_CubeSide = CubeSide;
  ti->m_DstFormat = tiSrc->m_DstFormat;

  int w, h;
  tiSrc->Set();
  if (tiSrc->m_eTT == eTT_Cubemap)
  {
    const GLenum cubefaces[6] = 
    {
      GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
    };
    for (int i=0; i<tiSrc->m_nMips; i++)
    {
      int tgt = cubefaces[tiSrc->m_CubeSide];
      glGetTexLevelParameteriv(tgt, i, GL_TEXTURE_WIDTH,  &w);
      glGetTexLevelParameteriv(tgt, i, GL_TEXTURE_HEIGHT, &h);
      if (!w || !h)
      {
        ti->Release(false);
        return NULL;
      }
      if (tiSrc->m_ETF == eTF_DXT1 || tiSrc->m_ETF == eTF_DXT3 || tiSrc->m_ETF == eTF_DXT5)
      {
        int mip_size;
        glGetTexLevelParameteriv(tgt, i, GL_TEXTURE_IMAGE_SIZE_ARB, &mip_size);
        byte *data = new byte[mip_size];
        glGetCompressedTexImageARB(tgt, i, data);
        tgt = cubefaces[ti->m_CubeSide];
        glCompressedTexImage2DARB(tgt, i, ti->m_DstFormat, w, h, 0, mip_size, data);
        delete [] data;
      }
      else
      {
        byte *data = new byte[w*h*4];
        glGetTexImage(tgt, i, GL_RGBA, GL_UNSIGNED_BYTE, data);
        tgt = cubefaces[ti->m_CubeSide];
        glTexImage2D(tgt, i, tiSrc->m_DstFormat, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        delete [] data;
      }
    }
  }
  return ti;
}

STexPic *CGLTexMan::CreateTexture(const char *name, int wdt, int hgt, int depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1, float fAmount2, int DXTSize, STexPic *ti, int bind, ETEX_Format eTF, const char *szSourceName)
{
  byte *dst1 = NULL;
  int m;
  int i;

  GLenum tgt = GL_TEXTURE_2D;
  int DxtBlockSize = 0;
  int DxtOneSize = 0;
  bool bMips;

  int w = ilog2(wdt);
  int h = ilog2(hgt);
  assert (w == wdt && h == hgt);

  if (!ti)
  {
    ti = TextureInfoForName(name, -1, eTT, flags, flags2, bind);

    ti->m_bBusy = true;
    ti->m_Flags = flags;
    ti->m_Flags2 = flags2;
    ti->m_Bind = TX_FIRSTBIND + ti->m_Id;
    AddToHash(ti->m_Bind, ti);
    ti->m_Height = hgt;
    ti->m_Width = wdt;
    ti->m_Depth = depth;
    ti->m_nMips = 0;
    ti->m_ETF = eTF_8888;
    eTF = eTF_8888;
    bind = ti->m_Bind;
  }
  if (szSourceName)
    ti->m_SourceName = szSourceName;

  if ((ti->m_Flags & FT_NOMIPS) || ti->m_nMips == 1)
  {
    bMips = false;
    ti->m_Flags |= FT_NOMIPS;
  }
  else
    bMips = true;
  if (ti->m_Flags & FT_DXT)
  {
    DxtBlockSize = (ti->m_Flags & FT_DXT1) ? 8 : 16;
    DxtOneSize = ((wdt+3)/4)*((hgt+3)/4)*DxtBlockSize;
  }
  ti->m_DXTSize = DXTSize;
  ti->m_fAmount1 = fAmount1;
  ti->m_fAmount2 = fAmount2;

  if (dst)
  {
    if (ti->m_Flags & FT_CONV_GREY)
      ti->m_pData32 = ConvertRGB_Gray(dst, ti, ti->m_Flags, eTF);
    if (ti->m_Flags & FT_NODOWNLOAD)
    {
      if (ti->m_Flags & FT_DXT)
        ti->m_pData32 = ImgConvertDXT_RGBA(dst, ti, DXTSize);
      else
        ti->m_pData32 = dst;
      return ti;
    }

    int dstFormat = 0;
    int srcFormat = GL_BGRA_EXT;
    int SizeSrc = 0;
    if (eTF == eTF_8888 || eTF == eTF_RGBA)
    {
      srcFormat = GL_BGRA_EXT;
      SizeSrc = wdt * hgt * 4;
    }
    else
    if (eTF == eTF_4444)
    {
      srcFormat = GL_BGRA_EXT;
      int nSize = 0;
      int w = wdt;
      int h = hgt;
      int i;
      for (i=0; i<ti->m_nMips; i++)
      {
        if (!w)
          w = 1;
        if (!h)
          h = 1;
        nSize += TexSize(w, h, 1, GL_RGBA8);
        w >>= 1;
        h >>= 1;
      }
      dst1 = new byte[nSize];
      w = wdt;
      h = hgt;
      byte *ds = dst1;
      byte *sr = dst;
      for (i=0; i<ti->m_nMips; i++)
      {
        if (!w)
          w = 1;
        if (!h)
          h = 1;
        for (int j=0; j<w*h; j++)
        {
          ds[0] = (sr[0]&0xf)<<4;
          ds[1] = (sr[0]&0xf0);
          ds[2] = (sr[1]&0xf)<<4;
          ds[3] = (sr[1]&0xf0);
          sr += 2;
          ds += 4;
        }
        w >>= 1;
        h >>= 1;
      }
      dst = dst1;
      dstFormat = GL_RGBA4;
      SizeSrc = nSize;
    }
    else
    if (eTF == eTF_0888)
    {
      srcFormat = GL_BGR_EXT;
      SizeSrc = wdt * hgt * 3;
    }
    else
    if (eTF == eTF_8000)
    {
      SizeSrc = wdt * hgt;
      srcFormat = GL_ALPHA8;
      dstFormat = GL_ALPHA8;
      ti->m_Flags |= FT_HASALPHA;
    }
    else
    if (eTF == eTF_DXT1 || eTF == eTF_DXT3 || eTF == eTF_DXT5)
    {
      if (eTF == eTF_DXT1)
        srcFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      else
      if (eTF == eTF_DXT3)
        srcFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      else
      if (eTF == eTF_DXT5)
        srcFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    }
    else
    if (eTF == eTF_0565 || eTF == eTF_0555)
      srcFormat = GL_RGB5;

    if (!(ti->m_Flags & FT_DXT) && !ti->m_pPalette)
    {
      if (dstFormat != GL_ALPHA8 && !(gRenDev->GetFeatures() & RFT_HWGAMMA))
      {
        if (!CRenderer::CV_r_noswgamma && ti->m_eTT != eTT_Bumpmap)
          BuildImageGamma(ti->m_Width, ti->m_Height, dst, false);
      }
    }
    if (ti->m_Flags2 & FT2_FORCEDXT)
    {
      if (!(ti->m_Flags & FT_HASALPHA))
        dstFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      else
        dstFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    }
    else
    if (ti->m_Flags & FT_DXT)
    {
      if (ti->m_Flags & FT_DXT1)
        dstFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      else
      if (ti->m_Flags & FT_DXT3)
        dstFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      else
      if (ti->m_Flags & FT_DXT5)
        dstFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
      else
      {
        Warning( VALIDATOR_FLAG_TEXTURE,ti->m_SearchName.c_str(),"Unknown DXT format for texture %s", ti->m_SearchName.c_str());      
        dstFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      }
      srcFormat = dstFormat;
      SizeSrc = DxtOneSize;
    }
    else
    if (dstFormat == 0)
    {
      if (ti->m_Flags & FT_DYNAMIC)
      {
        if (ti->m_Flags & FT_HASALPHA)
          dstFormat = GL_RGBA8;
        else
          dstFormat = GL_RGB8;
      }
      else
      {
        if (ti->m_eTT == eTT_Bumpmap)
        {
          if (CRenderer::CV_r_texbumpquality == 0)
          {
            if (ti->m_Flags & FT_HASALPHA)
              dstFormat = GL_RGBA8;
            else
              dstFormat = GL_RGB8;
          }
          else
          if (CRenderer::CV_r_texbumpquality == 1 && SUPPORTS_GL_EXT_paletted_texture)
          {
            dstFormat = GL_COLOR_INDEX8_EXT;
            dst1 = ConvertNMToPalettedFormat(dst, ti);
            dst = dst1;
            eTF = eTF_Index;
            if (!m_bPaletteWasLoaded)
            {
              glEnable( GL_SHARED_TEXTURE_PALETTE_EXT );
              glColorTableEXT(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGB, 256, GL_RGBA, GL_UNSIGNED_BYTE, (void *)&m_NMPalette[0][0]);
              m_bPaletteWasLoaded = true;
            }
          }
          else
          if ((CRenderer::CV_r_texbumpquality == 2 || strstr(ti->m_SourceName.c_str(), "_cct")) && SUPPORTS_GL_ARB_texture_compression)
          {
            dstFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            if (!CRenderer::CV_r_texhwdxtcompression)
            {
              ti->m_ETF = eTF_DXT1;
              ti->m_Flags |= FT_DXT1;
              dst1 = ImgConvertRGBA_DXT(dst, ti, DXTSize, ti->m_nMips, 24, true);
              dst = dst1;
              ti->m_DXTSize = DXTSize;
            }
          }
          else
            dstFormat = GL_RGB8;
        }
        else
        if (ti->m_eTT == eTT_DSDTBump)
        {
          if (SUPPORTS_GL_NV_texture_shader)
            dstFormat = GL_DSDT_MAG_NV;
          else
            dstFormat = GL_RGB8;
        }
        else
        if (ti->m_Flags & FT_SKY)
        {
          if (CRenderer::CV_r_texskyquality == 0)
            dstFormat = GL_RGB8;
          else
          if (CRenderer::CV_r_texskyquality == 1)
            dstFormat = GL_RGB5;
          else
          if (CRenderer::CV_r_texskyquality == 2 && SUPPORTS_GL_ARB_texture_compression)
            dstFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
          else
            dstFormat = GL_RGB8;
        }
        else
        if (!(ti->m_Flags & FT_HASALPHA))
        {
          if (CRenderer::CV_r_texquality == 0)
            dstFormat = GL_RGB8;
          else
          if (CRenderer::CV_r_texquality == 1)
            dstFormat = GL_RGB5;
          else
          if (CRenderer::CV_r_texquality == 2 && SUPPORTS_GL_ARB_texture_compression)
            dstFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
          else
            dstFormat = GL_RGB8;
        }
        else
        if (ti->m_Flags & FT_FONT)
        {
          dstFormat = GL_ALPHA8;
        }
        else
        {
          if (CRenderer::CV_r_texquality == 0)
            dstFormat = GL_RGBA8;
          else
          if (CRenderer::CV_r_texquality == 1)
            dstFormat = GL_RGBA4;
          else
          if (CRenderer::CV_r_texquality == 2 && SUPPORTS_GL_ARB_texture_compression)
            dstFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
          else
            dstFormat = GL_RGBA8;
        }
      }
    }
    else
    {
      if (ti->m_Flags & FT_FONT)
      {
        dstFormat = GL_ALPHA8;
      }
    }
    ti->m_DstFormat = dstFormat;
    ti->m_ETF = CGLTexMan::GetTexFormat(dstFormat);
    bool bFirstCube = false;
    if (ti->m_eTT == eTT_Cubemap)
    {
      ti->m_TargetType = GL_TEXTURE_CUBE_MAP_EXT;
      int n = strlen(ti->m_SearchName.c_str()) - 4;
      if (!strcmp(&ti->m_SearchName.c_str()[n], "posx"))
      {
        m_CurCubemapBind = bind;
        m_CurCubemapFormat = ti->m_ETF;
        tgt = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT;
        bFirstCube = true;
        ti->m_CubeSide = 0;
        m_LastCMSide = NULL;
      }
      else
      if (!strcmp(&ti->m_SearchName.c_str()[n], "negx"))
      {
        tgt = GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT;
        ti->m_CubeSide = 1;
      }
      else
      if (!strcmp(&ti->m_SearchName.c_str()[n], "posy"))
      {
        tgt = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT;
        ti->m_CubeSide = 2;
      }
      else
      if (!strcmp(&ti->m_SearchName.c_str()[n], "negy"))
      {
        tgt = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT;
        ti->m_CubeSide = 3;
      }
      else
      if (!strcmp(&ti->m_SearchName.c_str()[n], "posz"))
      {
        tgt = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT;
        ti->m_CubeSide = 4;
      }
      else
      if (!strcmp(&ti->m_SearchName.c_str()[n], "negz"))
      {
        tgt = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT;
        ti->m_CubeSide = 5;
      }
      else
      {
        m_CurCubemapBind = bind;
        m_CurCubemapFormat = ti->m_ETF;
        tgt = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT;
        bFirstCube = true;
        ti->m_CubeSide = 0;
        m_LastCMSide = NULL;
      }
      if (!bFirstCube && m_CurCubemapFormat != ti->m_ETF)
      {
        if (m_CurCubemapFormat != eTF_0888 && ti->m_ETF != eTF_8888)
          iLog->Log("Warning: CubeMap faces format mismath for texture '%s'\n", ti->m_SearchName.c_str());
      }

      if (tgt == GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT)
        glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, m_CurCubemapBind);
      glEnable(GL_TEXTURE_CUBE_MAP_EXT);
      CGLTexMan::m_TUState[m_CurStage].m_Target = GL_TEXTURE_CUBE_MAP_EXT;
    }
    else
    {
      if (ti->m_eTT == eTT_3D)
        ti->m_TargetType = GL_TEXTURE_3D_EXT;
      else
      if (ti->m_eTT == eTT_Rectangle)
        ti->m_TargetType = GL_TEXTURE_RECTANGLE_NV;
      else
        ti->m_TargetType = GL_TEXTURE_2D;

      if (CGLTexMan::m_TUState[m_CurStage].m_Target && CGLTexMan::m_TUState[m_CurStage].m_Target != ti->m_TargetType)
        glDisable(CGLTexMan::m_TUState[m_CurStage].m_Target);

      CGLTexMan::m_TUState[m_CurStage].m_Target = ti->m_TargetType;
      glBindTexture(ti->m_TargetType, ti->m_Bind);
      glEnable(ti->m_TargetType);

      tgt = ti->m_TargetType;
    }
    CGLTexMan::m_TUState[m_CurStage].m_Bind = ti->m_Bind;

    if (ti->m_pPalette && (ti->m_pData || dst))
    {
      if (!(ti->m_Flags & FT_PALETTED) || !(gRenDev->GetFeatures() & RFT_PALTEXTURE))
      {
        byte *d = gRenDev->m_TexMan->m_TexData;
        byte *src = ti->m_pData ? ti->m_pData : dst;
        for (m=0; m<ti->m_Width * ti->m_Height; m++)
        {
          int l = ti->m_pData[m];
          *(uint *)d = *(uint *)&ti->m_pPalette[l];
          d += 4;
        }
        int nMips = 0;
        if (!CGLRenderer::CV_gl_mipprocedures || (ti->m_Flags & FT_NOMIPS))
          nMips = 1;
        srcFormat = GL_BGRA_EXT;
        dstFormat = GL_RGBA8;
        if (!(ti->m_Flags & FT_HASALPHA))
          dstFormat = GL_RGB8;
        BuildMips(tgt, gRenDev->m_TexMan->m_TexData, ti->m_Width, ti->m_Height, ti->m_Depth, ti, srcFormat, dstFormat, 0, 0, nMips);
      }
      else 
      {
        if (!ti->m_p8to24table)
          ti->m_p8to24table = new uint [256];
        if (!ti->m_p15to8table)
          ti->m_p15to8table = new uchar [32768];
        uint *table = ti->m_p8to24table;
        SRGBPixel *pal = ti->m_pPalette;
        uint as = 255;
        uint r, g, b, a;
        uint v;
        for (i=0; i<256; i++)
        {
          r = pal->red;
          g = pal->green;
          b = pal->blue;
          a = pal->alpha;
          as &= a;
          pal++;
        
          v = (a<<24) + (r<<0) + (g<<8) + (b<<16);
          *table++ = v;
        }
        if (as < 255)
          ti->m_bAlphaPal = 1;

        for (i=0; i<(1<<15); i++)
        {
          /* Maps
            0000 0000 0000 0000
            0000 0000 0001 1111 = Red  = 0x1F
            0000 0011 1110 0000 = Blue = 0x03E0
            0111 1100 0000 0000 = Grn  = 0x7C00
          */
          r = ((i & 0x1F) << 3)+4;
          g = ((i & 0x03E0) >> 2)+4;
          b = ((i & 0x7C00) >> 7)+4;
          uchar *pal = (uchar *)ti->m_p8to24table;
          uint v;
          int j,k,l;
          int r1,g1,b1;
          for (v=0,k=0,l=10000*10000; v<256; v++,pal+=4)
          {
            r1 = r-pal[0];
            g1 = g-pal[1];
            b1 = b-pal[2];
            j = (r1*r1)+(g1*g1)+(b1*b1);
            if (j<l)
            {
              k = v;
              l = j;
            }
          }
          ti->m_p15to8table[i] = k;
        }

        glColorTableEXT( tgt, GL_RGBA, 256, GL_BGRA_EXT, GL_UNSIGNED_BYTE, ti->m_pPalette );
        GLuint SourceFormat   = GL_COLOR_INDEX;
        GLuint InternalFormat = GL_COLOR_INDEX8_EXT;
        byte *src = ti->m_pData ? ti->m_pData : dst;
        if (CGLRenderer::CV_gl_mipprocedures && !(ti->m_Flags & FT_NOMIPS))
        {
          BuildMips8(tgt, ti, src, false);
        }
        else
        {
          ti->m_Flags |= FT_NOMIPS;
          glTexImage2D(tgt, 0, InternalFormat, ti->m_Width, ti->m_Height, 0, SourceFormat, GL_UNSIGNED_BYTE, src);
          ti->m_Size = TexSize(ti->m_Width, ti->m_Height, ti->m_Depth, InternalFormat);
        }
      }
    }
    else 
    {
      int nMips = ti->m_nMips;
      if (ti->m_Flags & FT_NOMIPS)
        nMips = 1;
      if (IsDXTFormat(dstFormat))
      {
        bool bComp = ((gRenDev->GetFeatures() & RFT_COMPRESSTEXTURE) != 0);
        if (!bComp)
        {
          dst1 = ImgConvertDXT_RGBA(dst, ti, DXTSize);
          dst = dst1;
          dstFormat = GL_RGBA8;
        }
        else
        {
          DxtBlockSize = (dstFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 8 : 16;
          DxtOneSize = ((wdt+3)/4)*((hgt+3)/4)*DxtBlockSize;
        }
      }
      BuildMips(tgt, dst, wdt, hgt, depth, ti, srcFormat, dstFormat, DxtBlockSize, DXTSize, nMips);
    }
    if (ti->m_eTT == eTT_Cubemap)
    {
      if (tgt == GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT)
      {
        ti->SetFilter();
        ti->SetWrapping();
      }
    }
    else
    {
      ti->SetFilter();
      ti->SetWrapping();
    }

    if (tgt == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT)
    {
      int nnn = 0;
      SetTexture(0, eTT_Cubemap);
    }
    else
    if (tgt == GL_TEXTURE_2D)
      SetTexture(0, eTT_Base);
  }  // if (dst)
  if (ti->m_Flags & FT_NOMIPS)
    ti->m_nMips = 1;
  if (ti->m_eTT == eTT_Cubemap)
  {
    ti->m_Size *= 6;
    if (m_LastCMSide)
      m_LastCMSide->m_NextCMSide = ti;
    if (tgt == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT)
      m_LastCMSide = NULL;
    else
      m_LastCMSide = ti;
  }
  if (ti->m_eTT != eTT_Cubemap || !ti->m_CubeSide)
  {
    gRenDev->m_TexMan->m_StatsCurTexMem += ti->m_Size;
    ti->Unlink();
    ti->Link(&STexPic::m_Root);
    //sTestStr.AddElem(ti);
  }
  CheckTexLimits(NULL);
  if (m_Streamed == 2)
    ti->Unload();

  SAFE_DELETE_ARRAY (dst1);

  return ti;
}

//============================================================================

void CGLTexMan::BuildMipsSub(byte* src, int wdt, int hgt)
{
  int wd;
  int i;
  byte *src1, *dst1;
  int num;

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, wdt, hgt, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
  num = 1;
  while (wdt!=1 && hgt!=1)
  {
    wd = wdt<<2;
    wdt >>= 1;
    hgt >>= 1;
    if (wdt < 1)
      wdt = 1;
    if (hgt < 1)
      hgt = 1;
    //if (wdt == 1 || hgt == 1)  // Riva TNT Bug
    //  break;

    src1 = dst1 = src;
    for (i=0; i<hgt; i++)
    {
      byte *src2 = src1;
      int j;
      for (j=0; j<wdt; j++)
      {
        dst1[0] = (src2[0]+src2[4]+src2[wd]+src2[wd+4])>>2;
        dst1[1] = (src2[1]+src2[5]+src2[wd+1]+src2[wd+5])>>2;
        dst1[2] = (src2[2]+src2[6]+src2[wd+2]+src1[wd+6])>>2;
        dst1[3] = (src2[3]+src2[7]+src2[wd+3]+src2[wd+7])>>2;
        dst1 += 4;
        src2 += 8;
      }
      src1 += wd<<1;
    }
    glTexSubImage2D(GL_TEXTURE_2D, num, 0, 0, wdt, hgt, GL_BGRA_EXT, GL_UNSIGNED_BYTE, src);
    num++;
  }
}

void CGLTexMan::BuildMipsSub_DSDT(byte* src, int wdt, int hgt)
{
  int wd;
  int i;
  byte *src1, *dst1;
  int num;

  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, wdt, hgt, GL_DSDT_MAG_NV, GL_UNSIGNED_BYTE, src);
  num = 1;
  while (wdt!=1 && hgt!=1)
  {
    wd = wdt<<2;
    wdt >>= 1;
    hgt >>= 1;
    if (wdt < 1)
      wdt = 1;
    if (hgt < 1)
      hgt = 1;
    //if (wdt == 1 || hgt == 1)  // Riva TNT Bug
    //  break;

    src1 = dst1 = src;
    for (i=0; i<hgt; i++)
    {
      byte *src2 = src1;
      int j;
      for (j=0; j<wdt; j++)
      {
        dst1[0] = (src2[0]+src2[3]+src2[wd]+src2[wd+3])>>2;
        dst1[1] = (src2[1]+src2[4]+src2[wd+1]+src2[wd+4])>>2;
        dst1[2] = (src2[2]+src2[5]+src2[wd+2]+src1[wd+5])>>2;
        dst1 += 3;
        src2 += 6;
      }
      src1 += wd<<1;
    }
    glTexSubImage2D(GL_TEXTURE_2D, num, 0, 0, wdt, hgt, GL_DSDT_MAG_NV, GL_UNSIGNED_BYTE, src);
    num++;
  }
}


_inline void CGLTexMan::mfMakeS8T8_EdgePix(int x, int y, byte *src, byte *dst, int wdt, int hgt, int lSrcPitch, int lDstPitch)
{
  int x1 = x-1==-1 ? wdt : x-1;
  int y1 = y-1==-1 ? hgt : y-1;
  LONG v00 = src[y*lSrcPitch+x]; // Get the current pixel
  LONG v01 = src[y*lSrcPitch+((x+1)&wdt)]; // and the pixel to the right
  LONG vM1 = src[y*lSrcPitch+x1]; // and the pixel to the left
  LONG v10 = src[((y+1)&hgt)*lSrcPitch+x]; // and the pixel one line below.
  LONG v1M = src[y1*lSrcPitch+x]; // and the pixel one line above.

  LONG iDu = (vM1-v01); // The delta-u bump value
  LONG iDv = (v1M-v10); // The delta-v bump value

  if ( (v00 < vM1) && (v00 < v01) )  // If we are at valley
  {
    iDu = vM1-v00;                   // Choose greater of 1st order diffs
    if ( iDu < v00-v01 )
      iDu = v00-v01;
  }

  byte *ds = &dst[y*lDstPitch+(x*3)];

  ds[0] = (byte)iDu;
  ds[1] = (byte)iDv;
  ds[2] = 255;
}

void CGLTexMan::UpdateTextureRegion(STexPic *pic, byte *data, int X, int Y, int USize, int VSize)
{
  pic->Set();
  switch(pic->m_ETF)
  {
    case eTF_8888:
      glTexSubImage2D(pic->m_TargetType, 0, X, Y, USize, VSize, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);
  	  break;
    case eTF_8000:
      {
        int nSize = USize*VSize;
        byte *pBuf = new byte[nSize];
        for (int i=0; i<nSize; i++)
        {
          pBuf[i] = data[i*4+3];
        }
        glTexSubImage2D(pic->m_TargetType, 0, X, Y, USize, VSize, GL_ALPHA, GL_UNSIGNED_BYTE, pBuf);
        delete [] pBuf;
        break;
      }
    default:
      assert(0);
  }
  //pic->SaveTGA("Font.tga", 0);
}

void CGLTexMan::UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal)
{
  pic->Set();
  
  if (State == GS_BUMP)
  {
    if (pic->m_eTT == eTT_DSDTBump)
    {
      int x, y;
      byte *dst = new byte [USize * VSize * 3];
      byte *src = data;
      int wdt = USize;
      int hgt = VSize;
      int lSrcPitch = wdt;
      int lDstPitch = wdt*3;
      byte *pDst = dst+lDstPitch+3;
      byte *pSrc = src+lSrcPitch+1;
      for(y=1; y<hgt-1; y++ )
      {
        byte* pDstT = pDst;
        byte* pSrcB0 = (BYTE*)pSrc;
        byte* pSrcB1 = ( pSrcB0 + lSrcPitch );
        byte* pSrcB2 = ( pSrcB0 - lSrcPitch );
        
        for(x=1; x<wdt-1; x++ )
        {
          int v00 = *(pSrcB0+0); // Get the current pixel
          int v01 = *(pSrcB0+1); // and the pixel to the right
          int vM1 = *(pSrcB0-1); // and the pixel to the left
          int v10 = *(pSrcB1+0); // and the pixel one line below.
          int v1M = *(pSrcB2+0); // and the pixel one line above.
          
          int iDu = (vM1-v01); // The delta-u bump value
          int iDv = (v1M-v10); // The delta-v bump value
          
          if ( (v00 < vM1) && (v00 < v01) )  // If we are at valley
          {
            iDu = vM1-v00;                 // Choose greater of 1st order diffs
            if ( iDu < v00-v01 )
              iDu = v00-v01;
          }
          
          pDstT[0] = (byte)iDu;
          pDstT[1] = (byte)iDv;
          pDstT[2] = 255;
          
          pDstT += 3;
          
          pSrcB0+=1; // Move one pixel to the left (src is 32-bpp)
          pSrcB1+=1;
          pSrcB2+=1;
        }
        pSrc += lSrcPitch; // Move to the next line
        pDst += lDstPitch;
      }
      int UMask = wdt-1;
      int VMask = hgt-1;
      for (x=1; x<wdt-1; x++)
      {
        mfMakeS8T8_EdgePix(x, 0, src, dst, UMask, VMask, lSrcPitch, lDstPitch);
        mfMakeS8T8_EdgePix(x, hgt-1, src, dst, UMask, VMask, lSrcPitch, lDstPitch);
      }
      for (y=0; y<hgt; y++)
      {
        mfMakeS8T8_EdgePix(0, y, src, dst, UMask, VMask, lSrcPitch, lDstPitch);
        mfMakeS8T8_EdgePix(wdt-1, y, src, dst, UMask, VMask, lSrcPitch, lDstPitch);
      }
      if (CGLRenderer::CV_gl_mipprocedures)
        BuildMipsSub(dst, USize, VSize);
      else
      {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, USize, VSize, GL_DSDT_MAG_NV, GL_BYTE, dst);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_REPEAT);
        pic->m_Flags |= FT_NOMIPS;
      }
      delete [] dst;
    }
    else
    {
      int nMips, nSize;
      byte *dst = GenerateNormalMap(data, USize, VSize, CGLRenderer::CV_gl_mipprocedures ? 0 : FT_NOMIPS, 0, eTT_Bumpmap, 50.0f, pic, nMips, nSize, eTF_8888);
      int w = USize;
      int h = VSize;
      if (nMips == 1)
      {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA8, GL_UNSIGNED_BYTE, dst);
        pic->m_nMips = 1;
      }
      else
      if (nMips > 1)
      {
        byte *data = dst;
        for (int l=0; l<nMips; l++)
        {
          if (!w)
            w = 1;
          if (!h)
            h = 1;
          glTexSubImage2D(GL_TEXTURE_2D, l, 0, 0, w, h, GL_RGBA8, GL_UNSIGNED_BYTE, data);
          data += w*h*4;
          w >>= 1;
          h >>= 1;
        }
        pic->m_nMips = nMips;
      }
      delete [] dst;
    }
    return;
  }

  if (bPal)
  {
    GLuint SourceFormat   = GL_COLOR_INDEX;
    GLuint InternalFormat = GL_COLOR_INDEX8_EXT;
    if (CGLRenderer::CV_gl_mipprocedures && !(pic->m_Flags & FT_NOMIPS))
    {
      BuildMips8(GL_TEXTURE_2D, pic, data, true);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GetMinFilter());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GetMagFilter());
    }
    else
    {
      glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, USize, VSize, SourceFormat, GL_UNSIGNED_BYTE, data );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      pic->m_Flags |= FT_NOMIPS;
    }
    return;
  }
  if (CGLRenderer::CV_gl_mipprocedures)
    BuildMipsSub((byte *)data, USize, VSize);
  else
  {
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, USize, VSize, GL_BGRA_EXT, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    pic->m_Flags |= FT_NOMIPS;
  }
}

//TODO:replace with ARB_Buffer_Region and move into the class
void ClearBufferWithQuad(int x2,int y2,int x1,int y1,float fR,float fG,float fB,STexPic *pImage, int Side)
{ 
  if (pImage)
  {
		if (gRenDev->CV_r_ReplaceCubeMap==1)
			pImage = (STexPic *)gRenDev->EF_LoadTexture("textures/cube_face", 0, 0, eTT_Cubemap);
    gRenDev->Set2DMode(true, 256, 256);
    Vec3d crd[4];
    if (pImage->m_eTT == eTT_Cubemap)
    {
      switch(Side)
      {
        case 0: //posx
          crd[0] = Vec3d(1,1,1);
          crd[1] = Vec3d(1,-1,1);
          crd[2] = Vec3d(1,-1,-1);
          crd[3] = Vec3d(1,1,-1);
          break;
        case 1: //negx
          crd[0] = Vec3d(-1,1,1);
          crd[1] = Vec3d(-1,-1,1);
          crd[2] = Vec3d(-1,-1,-1);
          crd[3] = Vec3d(-1,1,-1);
          break;
        case 2: //posy
          crd[0] = Vec3d(1,1,1);
          crd[1] = Vec3d(-1,1,1);
          crd[2] = Vec3d(-1,1,-1);
          crd[3] = Vec3d(1,1,-1);
          break;
        case 3: //negy
          crd[0] = Vec3d(1,-1,1);
          crd[1] = Vec3d(-1,-1,1);
          crd[2] = Vec3d(-1,-1,-1);
          crd[3] = Vec3d(1,-1,-1);
          break;
        case 4: //posz
          crd[0] = Vec3d(1,1,1);
          crd[1] = Vec3d(-1,1,1);
          crd[2] = Vec3d(-1,-1,1);
          crd[3] = Vec3d(1,-1,1);
          break;
        case 5: //negz
          crd[0] = Vec3d(1,1,-1);
          crd[1] = Vec3d(-1,1,-1);
          crd[2] = Vec3d(-1,-1,-1);
          crd[3] = Vec3d(1,-1,-1);
          break;
      }
    }

    glDisable(GL_BLEND);   
	  glDisable(GL_STENCIL_TEST);
  	
    glDisable(GL_DEPTH_TEST);
    glDepthMask(1);
    glDisable(GL_CULL_FACE);

    glEnable(GL_TEXTURE_2D);  		
	  pImage->Set();
    glEnable(pImage->m_TargetType);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		

    glBegin(GL_QUADS);

	  glColor3f(fR,fG,fB);

    glTexCoord3f(crd[0][0], crd[0][1], crd[0][2]);
	  glVertex3f(0.0f, 0.0f, 0.0f);				// Top Left

    glTexCoord3f(crd[1][0], crd[1][1], crd[1][2]);
	  glVertex3f(0.0f, 256.0f, 0.0f);				// Bottom Left

    glTexCoord3f(crd[2][0], crd[2][1], crd[2][2]);
	  glVertex3f(256.0f, 256.0f, 0.0f);				// Bottom Right

    glTexCoord3f(crd[3][0], crd[3][1], crd[3][2]);
	  glVertex3f(256.0f, 0.0f, 0.0f);				// Top Right

    glEnd();    

    if (pImage->m_eTT == eTT_Cubemap)
      glDisable(pImage->m_TargetType);

    gRenDev->Set2DMode(false, 256, 256);

	  glEnable(GL_TEXTURE_2D);
	  glEnable(GL_CULL_FACE);
	  glEnable(GL_DEPTH_TEST); 
    glClear(GL_DEPTH_BUFFER_BIT);

    {
      /*int width = 256;
      int height = 256;
      byte *pic = new byte [width * height * 4];
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
      WriteTGA(pic,width,height,"EndCube.tga"); 
      delete [] pic;*/
    }
  }
  else
  {
    //glClearColor(1,0,0,0);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glScissor(0, 0, x2, y2);
    //glDisable (GL_SCISSOR_TEST);
    gcpOGL->EF_ClearBuffers(true, false);
    //glScissor(0, 0, gcpOGL->GetWidth(), gcpOGL->GetHeight());
    /*gcpOGL->m_bWasCleared = true;
    glClearColor(gcpOGL->m_vClearColor.x,gcpOGL->m_vClearColor.y,gcpOGL->m_vClearColor.z,0);
    if (gRenDev->GetStencilBpp())
	    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    else
	    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);*/
  }
}


void CGLTexMan::CreateBufRegion(int Width, int Height)
{
  if (SUPPORTS_WGL_ARB_buffer_region)
  {
    for (int i=0; i<m_BufRegions.Num(); i++)
    {
      SBufRegion *br = &m_BufRegions[i];
      if (br->m_Width == Width && br->m_Height == Height)
        break;
    }
    if (i == m_BufRegions.Num())
    {
      SBufRegion b;
      b.m_Width = Width;
      b.m_Height = Height;
      b.m_BRHandle = wglCreateBufferRegionARB(gcpOGL->m_CurrContext->m_hDC, 0, WGL_DEPTH_BUFFER_BIT_ARB | WGL_BACK_COLOR_BUFFER_BIT_ARB);
      if (b.m_BRHandle)
      {
        glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        // Copy the framebuffer to the buffer region (only depth).
        wglSaveBufferRegionARB(b.m_BRHandle, 0, 0, Width, Height);
      }
      m_BufRegions.AddElem(b);
    }
  }
}

STextureTarget *CGLTexMan::CreateTextureTarget(int Bind, int Width, int Height)
{
  int i;

  if (!SUPPORTS_WGL_ARB_render_texture)
    return NULL;

  for (i=0; i<m_TexTargets.Num(); i++)
  {
    STextureTarget *pTT = &m_TexTargets[i];
    if (pTT->m_Bind == Bind)
      return pTT;
  }
  STextureTarget texT;
  texT.m_Bind = Bind;
  texT.m_Width = Width;
  texT.m_Height = Height;
  texT.m_DrawCount = 0;
  texT.m_pBuffer = new CPBuffer(Width, Height, FPB_SINGLE | FPB_DRAWTOTEXTURE | FPB_DEPTH);
  texT.m_pBuffer->mfInitialize(true);
  m_TexTargets.AddElem(texT);

  return &m_TexTargets[i];
}

static _inline int sLimitSizeByScreenRes(int size)
{
  while(true)
  {
    if (size>gRenDev->GetWidth() || size>gRenDev->GetHeight())
      size >>= 1;
    else
      break;
  }
  return size;
}

void CGLTexMan::ClearBuffer(int Width, int Height, bool bEnd,STexPic *pImage, int Side)
{
  SBufRegion *br = NULL;
  if (SUPPORTS_WGL_ARB_buffer_region)
  {
    for (int i=0; i<m_BufRegions.Num(); i++)
    {
      br = &m_BufRegions[i];
      if (br->m_Width == Width && br->m_Height == Height)
        break;
    }
    if (i == m_BufRegions.Num())
      return;
  } 
  // Clear the frame buffer (only depth)
  if (br && br->m_BRHandle)
  {
    // Restore the buffer region.
    wglRestoreBufferRegionARB(br->m_BRHandle, 0, 0, Width, Height, 0, 0);
  }
  else
	{
		if (bEnd)
			ClearBufferWithQuad(Width, Height,0,0,0,0,0,NULL,0);
		else
			ClearBufferWithQuad(Width, Height,0,0,1,1,1,pImage,Side);
	}

}

//===================================================================================

void CGLTexMan::DrawCubeSide( const float *angle, Vec3d& Pos, int tex_size, int side, int RendFlags)
{
  if (!iSystem)
    return;
  
  CRenderer * renderer = gRenDev;
  CCamera tmp_camera = renderer->GetCamera();
  CCamera prevCamera = tmp_camera;

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  tmp_camera.Init(tex_size,tex_size, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  tmp_camera.SetPos(Pos);
  tmp_camera.SetAngle(Vec3d(angle[0], angle[1], angle[2]));
  tmp_camera.Update();

  iSystem->SetViewCamera(tmp_camera);
  gRenDev->SetCamera(tmp_camera);
  gRenDev->m_RP.m_bDrawToTexture = true;
  
  gRenDev->SetViewport(tex_size*(int)angle[3], tex_size*(int)angle[4], tex_size, tex_size);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. DrawLowDetail .. (DrawCubeSide %d)\n", side);

  eng->DrawLowDetail(RendFlags);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. End DrawLowDetail .. (DrawCubeSide %d)\n", side);

  iSystem->SetViewCamera(prevCamera);
  gRenDev->SetCamera(prevCamera);
  gRenDev->m_RP.m_bDrawToTexture = false;
}

static const GLenum cubefaces[6] = 
{
  GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
};

static float sAngles[6][5] = 
{
  {   90, -90, 0,  0, 0 },  //posx
  {   90, 90,  0,  1, 0 },  //negx
  {  180, 180, 0,  2, 0 },  //posy
  {   0, 180,  0,  0, 1 },  //negy
  {   90, 180, 0,  1, 1 },  //posz
  {   90, 0,   0,  2, 1 },  //negz
};

int gEnvFrame;


bool CGLTexMan::ScanEnvironmentCM (const char *name, int size, Vec3d& Pos)
{
  char szName[256];

  ClearBuffer(size,size,true,NULL,0);
  int RendFlags = -1;
  RendFlags &= ~DLD_ENTITIES;
  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  StripExtension(name, szName);

  int *pFR = (int *)gRenDev->EF_Query(EFQ_Pointer2FrameID);
  for(int n=0; n<6; n++)
  { 
    (*pFR)++;

    DrawCubeSide( &sAngles[n][0], Pos, size, n, RendFlags);
    static char* cubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
    char str[256];
    int width = size;
    int height = size;
    byte *pic = new byte [width * height * 4];
    glReadPixels(size*(int)sAngles[n][3], size*(int)sAngles[n][4], width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
    sprintf(str, "%s_%s.jpg", szName, cubefaces[n]);
    WriteJPG(pic, width, height, str); 
    delete [] pic;
  }

  gRenDev->SetViewport(vX, vY, vWidth, vHeight);
  ClearBuffer(size,size,true,NULL,0);

  return true;
}

void CGLTexMan::GetAverageColor(SEnvTexture *cm, int nSide)
{
}

void CGLTexMan::ScanEnvironmentCube(SEnvTexture *cm, int RendFlags, int Size, bool bLightCube)
{
  if (cm->m_bInprogress)
    return;
  
  CRenderer * renderer = gRenDev;
  int tex_size;
  switch (CRenderer::CV_r_envcmresolution)
  {
    case 0:
      tex_size = 64;
      break;
    case 1:
      tex_size = 128;
      break;
    case 2:
      tex_size = 256;
      break;
    case 3:
    default:
      tex_size = 512;
      break;
  }
  while(true)
  {
    if (tex_size*3>renderer->GetWidth() || tex_size*2>renderer->GetHeight())
      tex_size >>= 1;
    else
      break;
  }
  if (tex_size <= 8)
    return;

  int n;
  Vec3d cur_pos;

  cm->m_bInprogress = true;
  int tid = TO_ENVIRONMENT_CUBE_MAP_REAL + cm->m_Id;
  if (!(cm->m_Tex->m_Flags & FT_ALLOCATED) || tex_size != cm->m_TexSize)
  {
    cm->m_Tex->m_Flags |= FT_ALLOCATED;
    SetTexture(tid, eTT_Cubemap);
    cm->m_TexSize = tex_size;
    for(int n=0; n<6; n++)
    { 
      glTexImage2D(cubefaces[n], 0, GL_RGB, tex_size, tex_size, 0, GL_RGB, GL_FLOAT, 0);
    }
    SetTexture(0, eTT_Cubemap);
  }

  CreateBufRegion(tex_size, tex_size);
  ClearBuffer(tex_size,tex_size,true,NULL,0);
  //gRenDev->EF_SaveDLights();

  int Start, End;
  if (!cm->m_bReady || CRenderer::CV_r_envcmwrite)
  {
    Start = 0;
    End = 6;
  }
  else
  {
    Start = cm->m_MaskReady;
    End = cm->m_MaskReady+1;
  }
  int vX, vY, vWidth, vHeight;
  gRenDev->GetViewport(&vX, &vY, &vWidth, &vHeight);
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;
  Vec3d Pos;
  gRenDev->m_RP.m_pRE->mfCenter(Pos, gRenDev->m_RP.m_pCurObject);
  Pos += cm->m_CamPos;
  
  int *pFR = (int *)gRenDev->EF_Query(EFQ_Pointer2FrameID);
  for(n=Start; n<End; n++)
  { 
    *pFR++;
    DrawCubeSide( &sAngles[n][0], Pos, tex_size, n, RendFlags);
    if (CRenderer::CV_r_envcmwrite)
    {
      static char* cubefaces[6] = {"posx","negx","posy","negy","posz","negz"};
      char str[256];
      int width = tex_size;
      int height = tex_size;
      byte *pic = new byte [width * height * 4];
      glReadPixels(tex_size*(int)sAngles[n][3], tex_size*(int)sAngles[n][4], width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
      sprintf(str, "Cube_%s.jpg", cubefaces[n]);
      WriteJPG(pic, width, height, str); 
      delete [] pic;
    }
  }
  CRenderer::CV_r_envcmwrite = 0;

  cm->m_Tex->m_Bind = tid;
  SetTexture(tid, eTT_Cubemap);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

  for(n=Start; n<End; n++ )
  { 
    glCopyTexSubImage2D(cubefaces[n], 0, 0, 0, (int)(tex_size*sAngles[n][3]), (int)(tex_size*sAngles[n][4]), tex_size, tex_size);
  }

  gRenDev->SetViewport(vX, vY, vWidth, vHeight);
  ClearBuffer(tex_size,tex_size,true,NULL,0);
  cm->m_bInprogress = false;
  cm->m_MaskReady = End;
  if (cm->m_MaskReady == 6)
  {
    cm->m_MaskReady = 0;
    cm->m_bReady = true;
  }
  SetTexture(0, eTT_Cubemap);

  gRenDev->m_RP.m_bDrawToTexture = false;
  gRenDev->EF_PopFog();
  //gRenDev->EF_RestoreDLights();
}

//////////////////////////////////////////////////////////////////////////
static Matrix44 sMatrixLookAt( const Vec3d &dir,const Vec3d &up,float rollAngle=0 )
{
  Matrix44 M;
  // LookAt transform.
  Vec3d xAxis,yAxis,zAxis;
  Vec3d upVector = up;

  yAxis = GetNormalized(-dir);

  //if (zAxis.x == 0.0 && zAxis.z == 0)	up.Set( -zAxis.y,0,0 );	else up.Set( 0,1.0f,0 );

  xAxis = GetNormalized(upVector.Cross(yAxis));
  zAxis = GetNormalized(xAxis.Cross(yAxis));

  // OpenGL kind of matrix.
  M[0][0] = xAxis.x;
  M[1][0] = yAxis.x;
  M[2][0] = zAxis.x;
  M[3][0] = 0;

  M[0][1] = xAxis.y;
  M[1][1] = yAxis.y;
  M[2][1] = zAxis.y;
  M[3][1] = 0;

  M[0][2] = xAxis.z;
  M[1][2] = yAxis.z;
  M[2][2] = zAxis.z;
  M[3][2] = 0;

  M[0][3] = 0;
  M[1][3] = 0;
  M[2][3] = 0;
  M[3][3] = 1;

  if (rollAngle != 0)
  {
    Matrix44 RollMtx;
    RollMtx.SetIdentity();

    float cossin[2];
    cry_sincosf(rollAngle*gf_DEGTORAD, cossin);

    RollMtx[0][0] = cossin[0]; RollMtx[2][0] = -cossin[1];
    RollMtx[0][2] = cossin[1]; RollMtx[2][2] = cossin[0];

    // Matrix multiply.
    M = RollMtx * M;
  }

  return M;
}

void CGLTexMan::ScanEnvironmentTexture(SEnvTexture *cm, SShader *pSH, SRenderShaderResources *pRes, int RendFlags, bool bUseExistingREs)
{
  static float Smat[16] = 
  {
    0.5f, 0,    0,    0,
    0,    0.5f, 0,    0,
    0,    0,    0.5f, 0,
    0.5f, 0.5f, 0.5f, 1.0f
  };

  if (cm->m_bInprogress)
    return;
  
  CRenderer * rn = gRenDev;
  int tex_size;

  bool bUseClipPlanes = false;
  bool bUseReflection = false;
  if (pSH)
  {
    if (pSH->m_Flags3 & (EF3_CLIPPLANE_BACK | EF3_CLIPPLANE_FRONT))
      bUseClipPlanes = true;
    if (bUseClipPlanes || (pSH->m_Flags3 & EF3_REFLECTION))
      bUseReflection = true;
  }

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();
  float fMinDist = 0.25f;

  ECull eCull = eCULL_None;
  if (pSH)
    eCull = pSH->m_eCull;
  if (pRes && (pRes->m_ResFlags & MTLFLAG_2SIDED))
    eCull = eCULL_None;

  bool bWater = (pSH && ((pSH->m_nPreprocess & FSPR_SCANTEXWATER) != 0));
  Plane Pl, PlTr;
  float plane[4];
  CCObject *obj = rn->m_RP.m_pCurObject;
  if (bUseClipPlanes || bUseReflection)
  {
    if (!bWater)
      rn->m_RP.m_pRE->mfGetPlane(Pl);
    else
    {
      Pl.n = Vec3d(0,0,1);
      Pl.d = eng->GetWaterLevel();
    }
    if (obj)
    {
      rn->m_RP.m_FrameObject++;
      PlTr = TransformPlane(obj->GetMatrix(), Pl);
    }
    else
      PlTr = Pl;
    if (pSH && pSH->m_Flags3 & EF3_CLIPPLANE_BACK)
    {
      PlTr.n = -PlTr.n;
      PlTr.d = -PlTr.d;
    }
    if (eCull != eCULL_None)
    {
      CCamera tmp_camera = rn->GetCamera();
      Vec3d pos = tmp_camera.GetPos();
      float dot = pos.Dot(PlTr.n) - PlTr.d;
      if (dot <= 0.1f)
        return;
    }
    plane[0] = PlTr.n[0];
    plane[1] = PlTr.n[1];
    plane[2] = PlTr.n[2];
    plane[3] = -PlTr.d;

    if (!bWater)
      fMinDist = rn->m_RP.m_pRE->mfMinDistanceToCamera(obj);
  }

  switch (CRenderer::CV_r_envtexresolution)
  {
    case 0:
      tex_size = 64;
      break;
    case 1:
      tex_size = 128;
      break;
    case 2:
    default:
      tex_size = 256;
      break;
    case 3:
      tex_size = 512;
      break;
  }
  tex_size = sLimitSizeByScreenRes(tex_size);
  if (tex_size <= 8)
    return;

  Vec3d cur_pos;

  int tid = TO_ENVIRONMENT_TEX_MAP_REAL + cm->m_Id;
  if (!(cm->m_Tex->m_Flags & FT_ALLOCATED) || tex_size != cm->m_TexSize)
  {
    cm->m_Tex->m_Flags |= FT_ALLOCATED;
    cm->m_TexSize = tex_size;
    SetTexture(tid, eTT_Base);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_size, tex_size, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    SetTexture(0, eTT_Base);
  }

  cm->m_bInprogress = true;
  CreateBufRegion(tex_size,tex_size);
  ClearBuffer(tex_size,tex_size,true,NULL,0);
  //rn->EF_SaveDLights();

  int vX, vY, vWidth, vHeight;
  rn->GetViewport(&vX, &vY, &vWidth, &vHeight);
  rn->EF_PushFog();
  rn->m_RP.m_bDrawToTexture = true;

  float fMaxDist = eng->GetMaxViewDistance();

  int prevFlags = rn->m_RP.m_PersFlags;

  {
    CCamera tmp_camera = rn->GetCamera();
    CCamera prevCamera = tmp_camera;

    // camere reflection by plane
    if (bUseReflection || (pSH && (pSH->m_Flags3 & EF3_CLIPPLANE_FRONT)))
    {
      // mirror case
      Vec3d vPrevPos = tmp_camera.GetPos();
      Matrix44 camMat = tmp_camera.GetVCMatrixD3D9();
      Vec3d vPrevDir = Vec3d(-camMat(0,2), -camMat(1,2), -camMat(2,2));
      Vec3d vPrevUp = Vec3d(camMat(0,1), camMat(1,1), camMat(2,1));
      Vec3d vNewDir = PlTr.MirrorVector(vPrevDir);
      Vec3d vNewUp = PlTr.MirrorVector(vPrevUp);
      Matrix44 m = sMatrixLookAt( vNewDir, vNewUp, tmp_camera.GetAngles()[1] );

      float fDot = vPrevPos.Dot(PlTr.n) - PlTr.d;
      Vec3d vNewPos = vPrevPos - PlTr.n * 2.0f*fDot;
      Vec3d vNewOccPos = vPrevPos - PlTr.n * 0.99f*fDot;
      if (fDot < 0)
      {
        plane[0] = -plane[0];
        plane[1] = -plane[1];
        plane[2] = -plane[2];
        plane[3] = -plane[3];
      }

			CryQuat q = Quat( GetTransposed44(m) );
			Vec3d vNewAngs = Ang3::GetAnglesXYZ(Matrix33(q));
      vNewAngs = RAD2DEG(vNewAngs);

      rn->m_RP.m_pRE->mfCenter(vNewOccPos, obj);
      tmp_camera.SetAngle(vNewAngs);
      tmp_camera.SetPos(vNewPos);
      tmp_camera.SetOccPos(vNewOccPos);
      tmp_camera.Init(tex_size, tex_size, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
      tmp_camera.Update();

      iSystem->SetViewCamera(tmp_camera);
      rn->SetCamera(tmp_camera);

      if (!cm->m_Tex->m_Matrix)
        cm->m_Tex->m_Matrix = new float[16];
    }
    else
    {    
      iSystem->SetViewCamera(tmp_camera);
      rn->SetCamera(tmp_camera);
    }
    if (bUseClipPlanes || bUseReflection)
    {
      Plane p;
      p.n.Set(plane[0], plane[1], plane[2]);
      p.d = plane[3];
      tmp_camera.SetFrustumPlane(FR_PLANE_NEAR, p);
    }

    if (!cm->m_Tex->m_Matrix)
      cm->m_Tex->m_Matrix = new float[16];
    
    float matProj[16], matView[16], m2[16];

    glGetFloatv(GL_PROJECTION_MATRIX, matProj);
    glGetFloatv(GL_MODELVIEW_MATRIX, matView);
    mathMatrixMultiply(m2, &Smat[0], matProj, g_CpuFlags);
    mathMatrixMultiply(cm->m_Tex->m_Matrix, m2, matView, g_CpuFlags);
    //SGLFuncs::glMultMatrix(cm->m_Tex->m_Matrix, m1, rn->m_RP.m_pCurObject->GetMatrix().GetData());
    //memcpy(cm->m_Tex->m_Matrix, m1, 4*4*4);

    if (bUseClipPlanes)
      gcpOGL->EF_SetClipPlane(true, plane, bWater);

    rn->SetViewport(0, 0, tex_size, tex_size);

    if (rn->m_LogFile)
      rn->Logv(SRendItem::m_RecurseLevel, ".. DrawLowDetail .. (ScanEnvironmentTexture)\n");

    if (bUseExistingREs)
      gcpOGL->EF_RenderPipeLine(CGLRenderer::EF_Flush);
    else
      eng->DrawLowDetail(RendFlags);

    if (rn->m_LogFile)
      rn->Logv(SRendItem::m_RecurseLevel, ".. End DrawLowDetail .. (ScanEnvironmentTexture)\n");

    iSystem->SetViewCamera(prevCamera);
    rn->SetCamera(prevCamera);    
  }

  if (bUseClipPlanes)
    gcpOGL->EF_SetClipPlane(false, NULL, false);

  rn->m_RP.m_PersFlags &= ~(RBPF_DRAWMIRROR | RBPF_DRAWPORTAL);
  rn->m_RP.m_PersFlags |= prevFlags & (RBPF_DRAWMIRROR | RBPF_DRAWPORTAL);

  cm->m_Tex->m_Bind = tid;
  SetTexture(tid, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, tex_size, tex_size);

  /*{
      int width = tex_size;
      int height = tex_size;
      byte *pic = new byte [width * height * 4];
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
      WriteJPG(pic, width, height, "Bug.jpg"); 
      delete [] pic;
  }*/

  rn->SetViewport(vX, vY, vWidth, vHeight);
  ClearBuffer(tex_size,tex_size,true,NULL,0);
  cm->m_bInprogress = false;
  cm->m_bReady = true;
  cm->m_MaskReady = 1;
  rn->m_RP.m_bDrawToTexture = false;
  rn->EF_PopFog();

  SetTexture(0, eTT_Base);

  //rn->EF_RestoreDLights();
}

//========================================================================================

static CCamera sPrevCamera;

void CGLTexMan::StartCubeSide(CCObject *obj)
{
  int tex_size = 256;
  int numCM = obj->m_NumCM;
  if (numCM < 0 || numCM > 16)
    numCM = 0;
  bool bCube = true;
  if (obj->m_ObjFlags & FOB_TEXTURE)
    bCube = false;
  SEnvTexture *cm;
  if (bCube)
    cm = &gRenDev->m_TexMan->m_CustomCMaps[numCM];
  else
    cm = &gRenDev->m_TexMan->m_CustomTextures[numCM];
  int tid = cm->m_Tex->m_Bind;
  int n = obj->m_ObjFlags & FOB_CUBE_MASK;
  switch (n)
  {
    case FOB_CUBE_POSX:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** Start cube side (POSX) ***\n");
      n = 0;
      break;
    case FOB_CUBE_NEGX:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** Start cube side (NEGX) ***\n");
      n = 1;
      break;
    case FOB_CUBE_POSY:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** Start cube side (POSY) ***\n");
      n = 2;
      break;
    case FOB_CUBE_NEGY:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** Start cube side (NEGY) ***\n");
      n = 3;
      break;
    case FOB_CUBE_POSZ:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** Start cube side (POSZ) ***\n");
      n = 4;
      break;
    case FOB_CUBE_NEGZ:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** Start cube side (NEGZ) ***\n");
      n = 5;
      break;
  }
  if (!cm->m_MaskReady)
  {
    if (obj->m_ObjFlags & FOB_TEXTURE)
    {
      SetTexture(tid, eTT_Base);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_size, tex_size, 0, GL_RGB, GL_FLOAT, 0);
      SetTexture(0, eTT_Base);
    }
    else
    {
      SetTexture(tid, eTT_Cubemap);
      for(int n=0; n<6; n++)
      { 
        glTexImage2D(cubefaces[n], 0, GL_RGB, tex_size, tex_size, 0, GL_RGB, GL_FLOAT, 0);
      }
      SetTexture(0, eTT_Cubemap);
    }
  }
  sPrevCamera = gRenDev->GetCamera();
  CCamera tmp_camera= gRenDev->GetCamera();
  tmp_camera.Init(tex_size,tex_size);	
	tmp_camera.SetPos(Vec3d(obj->m_Trans2[0], obj->m_Trans2[1], obj->m_Trans2[2]));
	tmp_camera.SetAngle(Vec3d(obj->m_Angs2[0], obj->m_Angs2[1], obj->m_Angs2[2]));
  tmp_camera.Update();
  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport(0, 0, tex_size, tex_size);	

  gRenDev->SetCamera(tmp_camera);
  //gRenDev->LoadMatrix(0);
  //gRenDev->RotateMatrix(obj->m_Angs2.z, 0,0,1);
  //gRenDev->RotateMatrix(obj->m_Angs2.y, 0,1,0);
  //gRenDev->RotateMatrix(obj->m_Angs2.x, 1,0,0);
  //gRenDev->TranslateMatrix(-obj->m_Trans2);

  CreateBufRegion(tex_size, tex_size);
  ClearBuffer(tex_size,tex_size,false,(STexPic*)obj->m_pLightImage, n);
  
  /*int width = 256;
  int height = 256;
  byte *pic = new byte [width * height * 3];
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pic);
  CImage::SaveTga(pic,FORMAT_24_BIT,width,height,"StartCube.tga",false); 
  delete [] pic;*/
}

void CGLTexMan::EndCubeSide(CCObject *obj, bool bNeedClear)
{
  CRenderer * renderer = gRenDev;
  int tex_size = 256;
  int numCM = obj->m_NumCM;
  if (numCM < 0 || numCM > 16)
    numCM = 0;
  bool bCube = true;
  if (obj->m_ObjFlags & FOB_TEXTURE)
    bCube = false;
  SEnvTexture *cm;
  if (bCube)
    cm = &gRenDev->m_TexMan->m_CustomCMaps[numCM];
  else
    cm = &gRenDev->m_TexMan->m_CustomTextures[numCM];
  int tid = cm->m_Tex->m_Bind;

  int type;
  ETexType eTT;
  if (bCube)
  {
    type = GL_TEXTURE_CUBE_MAP_EXT;
    eTT = eTT_Cubemap;
  }
  else
  {
    type = GL_TEXTURE_2D;
    eTT = eTT_Base;
  }
  SetTexture(tid, eTT);
  glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(type, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  glTexParameteri(type, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexParameteri(type, GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);

  int n = obj->m_ObjFlags & FOB_CUBE_MASK;
  cm->m_MaskReady |= n >> FOB_CUBE_SHIFT;
  if (cm->m_MaskReady == 0x3f || cm->m_MaskReady == 0x40)
    cm->m_bReady = true;
  switch (n)
  {
    case FOB_CUBE_POSX:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** End cube side (POSX) ***\n");
      n = GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT;
      break;
    case FOB_CUBE_POSY:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** End cube side (POSY) ***\n");
      n = GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT;
      break;
    case FOB_CUBE_POSZ:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** End cube side (POSZ) ***\n");
      n = GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT;
      break;
    case FOB_CUBE_NEGX:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** End cube side (NEGX) ***\n");
      n = GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT;
      break;
    case FOB_CUBE_NEGY:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** End cube side (NEGY) ***\n");
      n = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT;
      break;
    case FOB_CUBE_NEGZ:
      if (gcpOGL->m_LogFile)
        gcpOGL->Logv(SRendItem::m_RecurseLevel, "*** End cube side (NEGZ) ***\n");
      n = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT;
      break;
    case FOB_TEXTURE:
      n = GL_TEXTURE_2D;
      break;
  }

  glCopyTexSubImage2D(n, 0, 0, 0, 0, 0, tex_size, tex_size);

  SetTexture(0, eTT);

  /*int width = 800;
  int height = 600;
  byte *pic = new byte [width * height * 4];
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
  WriteTGA(pic,width,height,"EndCube.tga"); 
  delete [] pic;*/
  
  if (bNeedClear)
    ClearBuffer(tex_size,tex_size,true,NULL,0);
  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);
  gRenDev->SetCamera(sPrevCamera);
}

inline void MultGLMatrix(const double *a, const double *b, double *product )
{
   /* This matmul was contributed by Thomas Malik */
   int i;

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

   /* i-te Zeile */
   for (i = 0; i < 4; i++) {
      double ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }

#undef A
#undef B
#undef P
}

inline void _multMatrices(float *dst, const float *a, const float *b)
{
  int i, j;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      dst[i * 4 + j] =
        b[i * 4 + 0] * a[0 * 4 + j] +
        b[i * 4 + 1] * a[1 * 4 + j] +
        b[i * 4 + 2] * a[2 * 4 + j] +
        b[i * 4 + 3] * a[3 * 4 + j];
    }
  }
}

void CGLTexMan::DrawToTexture(Plane& Pl, STexPic *Tex, int RendFlags)
{
  static float Smat[16] = 
  {
    0.5f, 0,    0,    0,
    0,    0.5f, 0,    0,
    0,    0,    0.5f, 0,
    0.5f, 0.5f, 0.5f, 1.0f
  };

  if (Tex->m_Flags & FT_BUILD)
    return;
  float plane[4];

  plane[0] = Pl.n[0];
  plane[1] = Pl.n[1];
  plane[2] = Pl.n[2];
  plane[3] = -Pl.d;
  
  int nWidth = sLimitSizeByScreenRes(512);
  int nHeight = nWidth;
  if (nWidth != Tex->m_Width || nHeight != Tex->m_Height)
  {
    int nSize = min(nWidth, nHeight);
    Tex->m_Width = nSize;
    Tex->m_Height = nSize;
    Tex->m_Flags &= ~FT_ALLOCATED;
  }

  Tex->m_Flags |= FT_BUILD;
  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    Tex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    AddToHash(Tex->m_Bind, Tex);
    Tex->m_RefTex.m_VidTex = CreateTextureTarget(Tex->m_Bind, Tex->m_Width, Tex->m_Height);
    if (!Tex->m_RefTex.m_VidTex)
    {
      SetTexture(Tex->m_Bind, eTT_Base);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Tex->m_Width, Tex->m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
      Tex->m_Size = CGLTexMan::TexSize(Tex->m_Width, Tex->m_Height, 1, GL_RGB8);
      SetTexture(0, eTT_Base);
    }
  }

  STextureTarget *tt = (STextureTarget *)Tex->m_RefTex.m_VidTex;
  if (!tt)
  {
    CreateBufRegion(Tex->m_Width, Tex->m_Height);
    ClearBuffer(Tex->m_Width, Tex->m_Height,true,NULL,0);
  }
  else
  {
    glBindTexture(GL_TEXTURE_2D, Tex->m_Bind);
    if (tt->m_DrawCount)
      tt->m_pBuffer->mfReleaseFromTexture();
    tt->m_pBuffer->mfMakeCurrent();
    // clear all pixels.
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  }

  //gRenDev->EF_SaveDLights();

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  float fMinDist = min(SKY_BOX_SIZE*0.5f, eng->GetDistanceToSectorWithWater());
  float fMaxDist = eng->GetMaxViewDistance();

  CCamera tmp_cam = gRenDev->GetCamera();
  CCamera prevCamera = tmp_cam;
  Vec3d vPrevPos = tmp_cam.GetPos();
  Matrix44 camMat = tmp_cam.GetVCMatrixD3D9();
  Vec3d vPrevDir = Vec3d(-camMat(0,2), -camMat(1,2), -camMat(2,2));
  Vec3d vPrevUp = Vec3d(camMat(0,1), camMat(1,1), camMat(2,1));
  Vec3d vNewDir = Pl.MirrorVector(vPrevDir);
  Vec3d vNewUp = Pl.MirrorVector(vPrevUp);
  Matrix44 mMir = sMatrixLookAt( vNewDir, vNewUp, tmp_cam.GetAngles()[1] );

  float fDot = vPrevPos.Dot(Pl.n) - Pl.d;
  Vec3d vNewPos = vPrevPos - Pl.n * 2.0f*fDot;
  Vec3d vOccPos = vPrevPos - Pl.n * 0.99f*fDot;

  CryQuat q = Quat( GetTransposed44(mMir) );

  Vec3d vNewAngs = Ang3::GetAnglesXYZ(Matrix33(q));
  vNewAngs = RAD2DEG(vNewAngs);

  tmp_cam.SetAngle(vNewAngs);
  tmp_cam.SetPos(vNewPos);
  tmp_cam.SetOccPos(vOccPos);
  tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  tmp_cam.Update();

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;
  
  //iSystem->SetViewCamera(tmp_cam);
  //gRenDev->SetCamera(tmp_cam);
  eng->SetCamera(tmp_cam,false);

  if (!Tex->m_Matrix)
    Tex->m_Matrix = new float[16];
  
  float matProj[16], matView[16], m2[16];

  glGetFloatv(GL_PROJECTION_MATRIX, matProj);
  glGetFloatv(GL_MODELVIEW_MATRIX, matView);
  mathMatrixMultiply(m2, &Smat[0], matProj, g_CpuFlags);
  mathMatrixMultiply(Tex->m_Matrix, m2, matView, g_CpuFlags);
  //SGLFuncs::glMultMatrix(Tex->m_Matrix, m1, &gRenDev->m_ObjMatrix.m_values[0][0]);

  gcpOGL->EF_SetClipPlane(true, plane, false);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. DrawLowDetail .. (DrawToTexture)\n");

  eng->DrawLowDetail(RendFlags);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. DrawLowDetail .. (End DrawToTexture)\n");

  gcpOGL->EF_SetClipPlane(false, plane, false);
  Tex->m_Flags &= ~FT_BUILD;

  /*int width = 512;
  int height = 512;
  byte *pic = new byte [width * height * 4];
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pic);
  WriteJPG(pic,width,height,"WaterMap.jpg"); 
  delete [] pic;*/

  if (tt)
  {
    tt->m_DrawCount++;
    tt->m_pBuffer->mfMakeMainCurrent();
    glBindTexture(GL_TEXTURE_2D, Tex->m_Bind);
    tt->m_pBuffer->mfTextureBind();
  }
  else
  {
    glReadBuffer(GL_BACK);
    SetTexture(Tex->m_Bind, eTT_Base);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Tex->m_Width, Tex->m_Height);
    //glTexImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Tex->m_Width, Tex->m_Height);
    //glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, Tex->m_Width, Tex->m_Height, 0);
    
    ClearBuffer(Tex->m_Width, Tex->m_Height, true, NULL,0);
  }
  SetTexture(0, eTT_Base);

  gRenDev->SetCamera(prevCamera);
  iSystem->SetViewCamera(prevCamera);
  
  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);
  gRenDev->m_RP.m_bDrawToTexture = false;
  gRenDev->EF_PopFog();

  //gRenDev->EF_RestoreDLights();
}

// ===============================================================
// FlashBang fx
// Last Update: 03/05/2003

// render flashbang fx
void CGLTexMan::DrawFlashBangMap(int iId, int iRenderFlags, CREFlashBang *pRE)
{
  if (!iSystem || pRE->GetFlashTimeOut()!=1.0f)
  {
    return;
  }
/*      
  I3DEngine *pEng = (I3DEngine *)iSystem->GetI3DEngine();   // get engine interface
  STexPic *pTex = gRenDev->m_TexMan->m_Text_FlashBangMap;             // get flashbang texture
  STexPic *pScreenTex = gRenDev->m_TexMan->m_Text_ScreenMap;  

  if(pTex->m_Flags & FT_BUILD)
  {
    return;
  }

  pTex->m_Flags |= FT_BUILD;

  // recreate flashbang texture if needed
  if(CRenderer::CV_r_flashbangsize != gRenDev->m_RP.m_FlashBangSize)
  {
    if (CRenderer::CV_r_flashbangsize <= 64)
      CRenderer::CV_r_flashbangsize = 64;
    else 
    if (CRenderer::CV_r_flashbangsize <= 128)
      CRenderer::CV_r_flashbangsize = 128;
    else
    if (CRenderer::CV_r_flashbangsize <= 256)
      CRenderer::CV_r_flashbangsize = 256;
    else
    if (CRenderer::CV_r_flashbangsize <= 512)
      CRenderer::CV_r_flashbangsize = 512;

    CRenderer::CV_r_flashbangsize = sLimitSizeByScreenRes(CRenderer::CV_r_flashbangsize);
    gRenDev->m_RP.m_FlashBangSize = CRenderer::CV_r_flashbangsize;
    pTex->m_Flags &= ~FT_ALLOCATED;
  }

  // set properties
  float fMaxDist = pEng->GetMaxViewDistance(),
  fMinDist = 0.25f;

  int nTexWidth = gRenDev->m_RP.m_FlashBangSize,
  nTexHeight = gRenDev->m_RP.m_FlashBangSize;

  // create screen texture
  if (!(pTex->m_Flags & FT_ALLOCATED))
  {
    pTex->m_Width = pTex->m_WidthReal = nTexWidth;
    pTex->m_Height = pTex->m_HeightReal = nTexHeight;
    pTex->m_Flags |= FT_ALLOCATED;

    // Preallocate texture
    AddToHash(pTex->m_Bind, pTex);

    // create texture
    SetTexture(pTex->m_Bind, eTT_Base);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, pTex->m_Width, pTex->m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    SetTexture(0, eTT_Base);
  }

  // create texture ?
  CreateBufRegion(pTex->m_Width, pTex->m_Height);

  // setup viewport
  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height );

  // save states
  if(gRenDev->m_LogFile)
  {
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** StartFlashBangMap... ***\n");
  }

  // setup flags
  gRenDev->m_RP.m_bDrawToTexture = true;

  // get scene into texture

  // set screen texture
  SetTexture(pScreenTex->m_Bind, eTT_Rectangle); 
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // set renderstate
  gRenDev->EnableDepthWrites(false);
  gRenDev->EnableDepthTest(false);

  // screen aligned quad 
  struct_VERTEX_FORMAT_P3F_TEX2F pScreenData[]=
  {
    800, 600, 0, (float)pScreenTex->m_Width, 0,
    800,   0, 0, (float)pScreenTex->m_Width, (float)pScreenTex->m_Height,
      0, 600, 0,                          0, 0,
      0,   0, 0,                          0, (float)pScreenTex->m_Height,
  };

  // set screen mode
  gRenDev->Set2DMode(true, 800, 600);
  gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenData,VERTEX_FORMAT_P3F_TEX2F)), 4);  
  gRenDev->Set2DMode(false, 800, 600);

  // render flashbang flash 
  STexPic *pFlashBangFlash = gRenDev->m_TexMan->m_Text_FlashBangFlash;

  gRenDev->EnableBlend(true);
  glBlendFunc(GL_DST_COLOR, GL_ONE);

  gRenDev->EnableDepthWrites(false);
  gRenDev->EnableDepthTest(false);

  // get flashbang properties
  float fPosX, fPosY, fSizeX, fSizeY;
  pRE->GetProperties(fPosX, fPosY, fSizeX, fSizeY);
  gRenDev->Draw2dImage(fPosX-fSizeX*0.5f, fPosY-fSizeY*0.5f, fSizeX, fSizeY, pFlashBangFlash->GetTextureID());
  gRenDev->EnableBlend(false); 

  // set texture/texture state 
  SetTexture(pTex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); 

  // get screen 
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, pTex->m_Width, pTex->m_Height);

  // blur screen texture

  // get renderer
  static CGLRenderer *pRenderer = gcpOGL;
  // get vertex program
  CCGVProgram_GL *vpBlur=(CCGVProgram_GL *) gRenDev->m_RP.m_VPBlur;//(CCGVProgram_GL *) CVProgram::mfForName("CGVProgBlur", true);
  // get fragment program
  CPShader *fpBlur=gRenDev->m_RP.m_RCBlur;//CPShader::mfForName("RCBlur", false);

  assert(pRenderer && "CREFlashBang::mfDraw - Invalid CGLRenderer pointer");
  assert(vpBlur && "CREFlashBang::mfDraw - Invalid CCGVProgram_GL pointer");
  assert(fpBlur && "CREFlashBang::mfDraw - Invalid CPShader pointer");

  // setup vertex/fragment program

  // set current vertex/fragment program
  vpBlur->mfSet(true, NULL);
  fpBlur->mfSet(true, NULL);

  // enable vertex/fragment programs
  pRenderer->EF_CommitVS();
  pRenderer->EF_CommitPS();

  // setup texture offsets, for texture neighboors sampling
  float s1=1.0f/(float)(CRenderer::CV_r_flashbangsize);
  float t1=1.0f/(float)(CRenderer::CV_r_flashbangsize);
  float s_off=s1*0.5f;
  float t_off=t1*0.5f;

  float fOffset0[]={ s1*0.5f+s_off,       t1+t_off, 0.0f, 0.0f};
  float fOffset1[]={     -s1+s_off,  t1*0.5f+t_off, 0.0f, 0.0f};
  float fOffset2[]={-s1*0.5f+s_off,      -t1+t_off, 0.0f, 0.0f};
  float fOffset3[]={      s1+s_off, -t1*0.5f+t_off, 0.0f, 0.0f};

  // set vertex program consts
  vpBlur->mfParameter4f("Offset0", fOffset0);
  vpBlur->mfParameter4f("Offset1", fOffset1);
  vpBlur->mfParameter4f("Offset2", fOffset2);
  vpBlur->mfParameter4f("Offset3", fOffset3);

  // screen aligned quad
  static struct_VERTEX_FORMAT_P3F_TEX2F pData[]=
  {
    1, 1, 0, 1-s_off,   t_off,
    1, 0, 0, 1-s_off, 1-t_off,
    0, 1, 0,   s_off,   t_off,
    0, 0, 0,   s_off, 1-t_off,
  };

  // render quad

  // set render/texture state
  gRenDev->EnableDepthWrites(false);
  gRenDev->EnableDepthTest(false);

  pRenderer->EF_SelectTMU(0);
  SetTexture(pTex->m_Bind, eTT_Base);

  pRenderer->EF_SelectTMU(1);
  SetTexture(pTex->m_Bind, eTT_Base);

  pRenderer->EF_SelectTMU(2);
  SetTexture(pTex->m_Bind, eTT_Base);

  pRenderer->EF_SelectTMU(3);
  SetTexture(pTex->m_Bind, eTT_Base);

  // set screen mode
  gRenDev->Set2DMode(true, 1, 1);

  // get projection/modelview matrix
  float fMatrixModelView[16];
  float fMatrixProjection[16];
  gRenDev->GetModelViewMatrix(fMatrixModelView);
  gRenDev->GetProjectionMatrix(fMatrixProjection);

  Matrix44 pModelView(fMatrixModelView),
            pProjection(fMatrixProjection),
            pWorldViewProj=GetTransposed44(pModelView*pProjection);

  // pass orthoprojection matrix into vertex shader
  SCGBind *pModelViewProj = vpBlur->mfGetParameterBind("ModelViewOrthoProj");
  
  if(pModelViewProj)
  {
    vpBlur->mfParameter(pModelViewProj, pWorldViewProj.GetData(), 4);
  }

  // blur texture
  for(int iBlurPasses=0; iBlurPasses<2;iBlurPasses++) 
  {
    gRenDev->DrawTriStrip(&(CVertexBuffer (pData,VERTEX_FORMAT_P3F_TEX2F)), 4);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, pTex->m_Width, pTex->m_Height);
  }

  // restore previous states
  gRenDev->Set2DMode(false, 1, 1);

  // set flags
  pTex->m_Flags &= ~FT_BUILD;
  gRenDev->m_RP.m_bDrawToTexture = false;

  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);  
  gRenDev->ResetToDefault();  
  //ClearBuffer(pTex->m_Width, pTex->m_Height, true, NULL,0);
*/
  // set flags  
  gRenDev->m_RP.m_bDrawToTexture = false; 

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** EndFlashBangMap... ***\n");
}

// ===============================================================
// Glare fx
// Last Update: 07/05/2003

void CGLTexMan::DrawToTextureForGlare(int Id)
{
  if (!iSystem)
    return;
/*
  // get data
  I3DEngine *pEng = (I3DEngine *)iSystem->GetI3DEngine();
  CREGlare *pRE = gRenDev->m_RP.m_pREGlare;
  STexPic *pTex = gRenDev->m_TexMan->m_Text_Glare;
  STexPic *pScreenTex = gRenDev->m_TexMan->m_Text_ScreenMap;  

  if((pTex->m_Flags & FT_BUILD) || (pScreenTex->m_Flags & FT_BUILD))
  {
    return;
  }

  pTex->m_Flags |= FT_BUILD;

  if(CRenderer::CV_r_glaresize != gRenDev->m_RP.m_GlareSize)
  {
    if (CRenderer::CV_r_glaresize <= 32)
      CRenderer::CV_r_glaresize = 32;
    else
    if (CRenderer::CV_r_glaresize <= 64)
      CRenderer::CV_r_glaresize = 64;
    else
    if (CRenderer::CV_r_glaresize <= 128)
      CRenderer::CV_r_glaresize = 128;
    else
      CRenderer::CV_r_glaresize = 256;

    CRenderer::CV_r_glaresize = sLimitSizeByScreenRes(CRenderer::CV_r_glaresize);
    gRenDev->m_RP.m_GlareSize = CRenderer::CV_r_glaresize;
    pTex->m_Flags &= ~FT_ALLOCATED;
    pRE->m_GlareWidth = pRE->m_GlareHeight = gRenDev->m_RP.m_GlareSize;
    pRE->mfInit();
  }

  int nTexWidth = pRE->m_GlareWidth;
  int nTexHeight = pRE->m_GlareHeight;

  // create texture if necessary
  if (!(pTex->m_Flags & FT_ALLOCATED))
  {
    pTex->m_Width = pTex->m_WidthReal = nTexWidth;
    pTex->m_Height = pTex->m_HeightReal = nTexHeight;
    pTex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    AddToHash(pTex->m_Bind, pTex);
    SetTexture(pTex->m_Bind, eTT_Base);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pTex->m_Width, pTex->m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    SetTexture(0, eTT_Base);
  }

  // get/set states
  CCamera pTempCam = gRenDev->GetCamera(); 
  CCamera pPrevCamera = pTempCam;

  // get current screen
  StartScreenTexMap(0);
  EndScreenTexMap();

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);  

  // compute screen luminosity if necessary
  if(CRenderer::CV_r_glare==2)
  {
    static int iFrameCounter=0;
    if(iFrameCounter%4==0)
    {
      gRenDev->ResetToDefault();
      // get average screen luminosity
      gRenDev->SetViewport( 0, 0, 2, 2);
      pEng->SetCamera(pTempCam);  
      CreateBufRegion(2, 2);

      // set screen texture
      SetTexture(pScreenTex->m_Bind, eTT_Rectangle); 
      glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
      glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   
      glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP); 

      // set renderstate
      gRenDev->EnableDepthWrites(false);
      gRenDev->EnableDepthTest(false);

      // screen aligned quad 
      static struct_VERTEX_FORMAT_P3F_TEX2F pScrData[]=
      {
        1, 1, 0, (float)pScreenTex->m_Width, 0,
        1, 0, 0, (float)pScreenTex->m_Width, (float)pScreenTex->m_Height,
        0, 1, 0,                          0, 0,
        0, 0, 0,                          0, (float)pScreenTex->m_Height,
      };

      // set screen mode
      gRenDev->Set2DMode(true, 1, 1);
      gRenDev->DrawTriStrip(&(CVertexBuffer (pScrData,VERTEX_FORMAT_P3F_TEX2F)), 4);  
      gRenDev->Set2DMode(false, 1, 1);
     
      // copy 4 pixels
      static unsigned char pBits[3*2*2];        
      glReadPixels(0, 0, 2, 2, GL_RGB, GL_UNSIGNED_BYTE, pBits);
                
      //// get pixels luminosity
      //float fAvgGlareAmount=0;
      //for(int iY=0; iY<3*2*2; iY++)
      //{
      //  fAvgGlareAmount+=pCurrPixel[iY];
      //}
      //fAvgGlareAmount /= 3.0f*2.0f*2.0f;
      //fAvgGlareAmount /= 255.f;

      // get pixels luminosity
      float fAvgGlareAmount=0;

      for(int iY=0; iY<2*2; iY++)
      {
        float fAvgRgb=0;

        fAvgRgb+=pBits[iY*3]; 
        fAvgRgb+=pBits[iY*3+1]; 
        fAvgRgb+=pBits[iY*3+2]; 

        fAvgRgb/=3.0f;

        fAvgGlareAmount+= fAvgRgb; 
      }

      fAvgGlareAmount /= 2.0f*2.0f;
      fAvgGlareAmount /= 255.f;

      // average pixels luminosity
      CRenderer::CV_r_glareamountdynamic=fAvgGlareAmount;   

      iFrameCounter=0;
    }

    iFrameCounter++;
  }

  gRenDev->ResetToDefault(); 
  CreateBufRegion(pTex->m_Width, pTex->m_Height);

  // resize screen to fit glare texture
  gRenDev->SetViewport( 0, 0, pTex->m_Width, pTex->m_Height );
  pEng->SetCamera(pTempCam);

  //gRenDev->ResetToDefault(); 
  // create texture ? 
  //CreateBufRegion(pTex->m_Width, pTex->m_Height);
  // clear texture ?
  //ClearBuffer(pTex->m_Width, pTex->m_Height, true, NULL, 0);

  // get renderer
  CGLRenderer *pRenderer = gcpOGL;
  // get fragment program
  CCGPShader_GL *fpGlareMap=(CCGPShader_GL *)gRenDev->m_RP.m_RCGlareMap; //CPShader::mfForName("RCGlareMap", false);   
  assert(fpGlareMap && "CGLTexMan::DrawToTextureForGlare - Invalid CPShader pointer");

  // set fragment program
  fpGlareMap->mfSet(true, 0);
  pRenderer->EF_CommitPS();

  // set constants
  float pGlareProps[]= { 0.33f, 0.61f, 0.11f, 0.0f };  
  fpGlareMap->mfParameter4f("Glare", pGlareProps);  

  // set screen texture
  SetTexture(pScreenTex->m_Bind, eTT_Rectangle); 
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // set renderstate
  gRenDev->EnableDepthWrites(false);
  gRenDev->EnableDepthTest(false);

  // screen aligned quad 
  static struct_VERTEX_FORMAT_P3F_TEX2F pScreenData[]=
  {
    800, 600, 0, (float)pScreenTex->m_Width, 0,
    800,   0, 0, (float)pScreenTex->m_Width, (float)pScreenTex->m_Height,
      0, 600, 0,                          0, 0,
      0,   0, 0,                          0, (float)pScreenTex->m_Height,
  };

  // set screen mode
  gRenDev->Set2DMode(true, 800, 600);
  gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenData,VERTEX_FORMAT_P3F_TEX2F)), 4);  
      
  // get screen
  SetTexture(pTex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, pTex->m_Width, pTex->m_Height);

  //if (CRenderer::CV_r_glare == 3) 
  //{
  //  glReadPixels (0, 0, pTex->m_Width, pTex->m_Height, GL_RGBA, GL_UNSIGNED_BYTE, pRE->m_pGlarePixels);
  //  CRenderer::CV_r_glare = 1;
  //  WriteJPG((byte *)pRE->m_pGlarePixels, pTex->m_Width, pTex->m_Height, "Glare.jpg");
  //}

  // blur texture
  
  // get vertex program
  CCGVProgram_GL *vpBlur=(CCGVProgram_GL *) gRenDev->m_RP.m_VPBlur;//(CCGVProgram_GL *) CVProgram::mfForName("CGVProgBlur", true);
  // get fragment program
  CPShader *fpBlur=gRenDev->m_RP.m_RCBlur;//CPShader::mfForName("RCBlur", false); 

  assert(pRenderer && "CGLTexMan::DrawToTextureForGlare - Invalid CGLRenderer pointer");
  assert(vpBlur && "CGLTexMan::DrawToTextureForGlare - Invalid CCGVProgram_GL pointer");
  assert(fpBlur && "CGLTexMan::DrawToTextureForGlare - Invalid CPShader pointer");

  // setup vertex/fragment program

  // set current vertex/fragment program
  vpBlur->mfSet(true, 0);
  fpBlur->mfSet(true, 0);

  // enable vertex/fragment programs
  pRenderer->EF_CommitVS();
  pRenderer->EF_CommitPS();

  // setup texture offsets, for texture neighboors sampling
  float s1=1.0f/(float) pTex->m_Width;
  float t1=1.0f/(float) pTex->m_Height;
  float s_off=0;//s1*0.5f;
  float t_off=0;//t1*0.5f;  

  float fOffset0[]={ s1*0.5f+s_off,       t1+t_off, 0.0f, 0.0f};
  float fOffset1[]={     -s1+s_off,  t1*0.5f+t_off, 0.0f, 0.0f};
  float fOffset2[]={-s1*0.5f+s_off,      -t1+t_off, 0.0f, 0.0f};
  float fOffset3[]={      s1+s_off, -t1*0.5f+t_off, 0.0f, 0.0f};

  // set vertex program consts
  vpBlur->mfParameter4f("Offset0", fOffset0);
  vpBlur->mfParameter4f("Offset1", fOffset1);
  vpBlur->mfParameter4f("Offset2", fOffset2);
  vpBlur->mfParameter4f("Offset3", fOffset3); 

  float fMinS = 0;
  float fMinT = 0;
  float fMaxS = 1.0f - fMinS;
  float fMaxT = 1.0f - fMinT;

  // screen aligned quad 
  static struct_VERTEX_FORMAT_P3F_TEX2F pData[]=
  {
    800, 600, 0, fMaxS, fMinT, 
    800,   0, 0, fMaxS, fMaxT,
      0, 600, 0, fMinS, fMinT,
      0,   0, 0, fMinS, fMaxT,
  };

  // render quad

  // set render/texture state
  gRenDev->EnableDepthWrites(false);
  gRenDev->EnableDepthTest(false); 

  pRenderer->EF_SelectTMU(0); 
  SetTexture(pTex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  pRenderer->EF_SelectTMU(1);
  SetTexture(pTex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  pRenderer->EF_SelectTMU(2);
  SetTexture(pTex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  pRenderer->EF_SelectTMU(3);
  SetTexture(pTex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
 
  // get projection/modelview matrix
  float fMatrixModelView[16];
  float fMatrixProjection[16];
  gRenDev->GetModelViewMatrix(fMatrixModelView);
  gRenDev->GetProjectionMatrix(fMatrixProjection);

  Matrix44 pModelView(fMatrixModelView),
            pProjection(fMatrixProjection),
            pWorldViewProj=GetTransposed44(pModelView*pProjection);

  // pass orthoprojection matrix into vertex shader
  SCGBind *pModelViewProj = vpBlur->mfGetParameterBind("ModelViewOrthoProj");
  if(pModelViewProj)
    vpBlur->mfParameter(pModelViewProj, pWorldViewProj.GetData(), 4);

  // blur texture
  for(int iBlurPasses=0; iBlurPasses<CRenderer::CV_r_glareboxsize;iBlurPasses++)  
  {
    gRenDev->DrawTriStrip(&(CVertexBuffer (pData,VERTEX_FORMAT_P3F_TEX2F)), 4);
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, pTex->m_Width, pTex->m_Height);
  }

  vpBlur->mfSet(false, 0);
  fpBlur->mfSet(false, 0);

  // restore previous states
  gRenDev->EnableDepthWrites(true);
  gRenDev->EnableDepthTest(true);  
//  ClearBuffer(pTex->m_Width, pTex->m_Height, true, NULL,0);    
  gRenDev->Set2DMode(false, 800, 600);  

  // restore screen...  
  gRenDev->ResetToDefault(); 
  // set screen texture
  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);
  SetTexture(pScreenTex->m_Bind, eTT_Rectangle);  
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // set renderstate
  gRenDev->EnableDepthWrites(false);
  gRenDev->EnableDepthTest(false);

  // set screen mode
  gRenDev->Set2DMode(true, 800, 600); 
  gRenDev->DrawTriStrip(&(CVertexBuffer (pScreenData,VERTEX_FORMAT_P3F_TEX2F)), 4);  
  gRenDev->Set2DMode(false, 800, 600);  

  gRenDev->SetCamera(pPrevCamera); 
  iSystem->SetViewCamera(pPrevCamera);
  
  // restore flags/states
  gRenDev->ResetToDefault();
  //SetTexture(0, eTT_Base); 
  pTex->m_Flags &= ~FT_BUILD;
  */
  gRenDev->m_RP.m_bDrawToTexture = false;

}

//==================================================================================
// Heat map

_inline void GLQuad(float wdt, float hgt)
{
  glBegin(GL_QUADS);

  glTexCoord2f(0, 0);
  glVertex3f(0, 0, 0);

  glTexCoord2f(1.0f, 0);
  glVertex3f(wdt, 0, 0);

  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(wdt, hgt, 0);

  glTexCoord2f(0, 1.0f);
  glVertex3f(0, hgt, 0);

  glEnd();
}

void CGLTexMan::EndHeatMap()
{
  gRenDev->m_RP.m_PersFlags &= ~(RBPF_DRAWHEATMAP | RBPF_NOCLEARBUF);

  STexPic *Tex = gRenDev->m_TexMan->m_Text_HeatMap;

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** End Draw scene to texture for heat ***\n");

  SetTexture(Tex->m_Bind, eTT_Base);
  if (SUPPORTS_GL_SGIS_generate_mipmap && CRenderer::CV_r_heatmapmips)
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (CRenderer::CV_r_heattype != 1)
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Tex->m_Width, Tex->m_Height);

  if (SUPPORTS_GL_SGIS_generate_mipmap && CRenderer::CV_r_heatmapmips)
  {
    if (CRenderer::CV_r_heattype != 1)
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
  }

  gRenDev->SetCamera(m_PrevCamera);
  iSystem->SetViewCamera(m_PrevCamera);

  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);

  if (CRenderer::CV_r_heattype == 1)
  {
    int i;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, gRenDev->GetWidth(), 0.0, gRenDev->GetHeight(), -20.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    gRenDev->SetCullMode(R_CULL_NONE);
    gRenDev->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
    float wdt = (float)Tex->m_Width;
    float hgt = (float)Tex->m_Height;
    gRenDev->SetState(GS_NODEPTHTEST);
    SetTexture(Tex->m_Bind, eTT_Base);

    glColor4f(1,1,1,1);

    GLQuad(wdt, hgt);

    gRenDev->EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
    for (i=0; i<4; i++)
    {
      float fOffs = (i+1)*(Tex->m_Width/1024.0f);
      float fAlpha = 0.1f;
      glColor4f(1,1,1,fAlpha);

      glPushMatrix();
      glTranslatef(fOffs,0,0);
      GLQuad(wdt, hgt);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-fOffs,0,0);
      GLQuad(wdt, hgt);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(0,fOffs,0);
      GLQuad(wdt, hgt);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(0,-fOffs,0);
      GLQuad(wdt, hgt);
      glPopMatrix();
    }

    float fRandU = RandomNum() * 2.0f;
    float fRandV = RandomNum() * 2.0f;
    gRenDev->SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_NODEPTHTEST);
    STexPic *tp = gRenDev->m_TexMan->LoadTexture("Textures/Defaults/HeatNoise", 0, 0);
    tp->Set();

    glBegin(GL_QUADS);

    glTexCoord2f(fRandU, fRandV);
    glVertex3f(0, 0, 0);

    glTexCoord2f(4.0f+fRandU, fRandV);
    glVertex3f(wdt, 0, 0);

    glTexCoord2f(4.0f+fRandU, 4.0f+fRandV);
    glVertex3f(wdt, hgt, 0);

    glTexCoord2f(fRandU, 4.0f+fRandV);
    glVertex3f(0, hgt, 0);

    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    SetTexture(Tex->m_Bind, eTT_Base);
    if (SUPPORTS_GL_SGIS_generate_mipmap && CRenderer::CV_r_heatmapmips)
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Tex->m_Width, Tex->m_Height);

    if (SUPPORTS_GL_SGIS_generate_mipmap && CRenderer::CV_r_heatmapmips)
      glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
  }

  if (CRenderer::CV_r_heatmapsave)
  {
    CRenderer::CV_r_heatmapsave = 0;
    Tex->SaveJPG("HeatMap.jpg", false);
  }

  SetTexture(0, eTT_Base);

  //ClearBuffer(Tex->m_Width, Tex->m_Height, true, NULL,0);

  Tex->m_Flags &= ~FT_BUILD;
  gRenDev->m_RP.m_bDrawToTexture = false;

  gRenDev->ResetToDefault();
}

void CGLTexMan::StartHeatMap(int Id)
{
  if (!iSystem)
    return;

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  STexPic *Tex = gRenDev->m_TexMan->m_Text_HeatMap;
  if (Tex->m_Flags & FT_BUILD)
    return;

  if (CRenderer::CV_r_heatsize != gRenDev->m_RP.m_HeatSize)
  {
    if (CRenderer::CV_r_heatsize <= 64)
      CRenderer::CV_r_heatsize = 64;
    else
    if (CRenderer::CV_r_heatsize <= 128)
      CRenderer::CV_r_heatsize = 128;
    else
    if (CRenderer::CV_r_heatsize <= 256)
      CRenderer::CV_r_heatsize = 256;
    else
    if (CRenderer::CV_r_heatsize <= 512)
      CRenderer::CV_r_heatsize = 512;

    CRenderer::CV_r_heatsize = sLimitSizeByScreenRes(CRenderer::CV_r_heatsize);
    gRenDev->m_RP.m_HeatSize = CRenderer::CV_r_heatsize;
    Tex->m_Flags &= ~FT_ALLOCATED;
  }
  int nTexWidth = gRenDev->m_RP.m_HeatSize;
  int nTexHeight = gRenDev->m_RP.m_HeatSize;

  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  Tex->m_Flags |= FT_BUILD;
  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    Tex->m_Width = nTexWidth;
    Tex->m_Height = nTexHeight;
    Tex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    AddToHash(Tex->m_Bind, Tex);
    SetTexture(Tex->m_Bind, eTT_Base);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Tex->m_Width, Tex->m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    SetTexture(0, eTT_Base);
  }

  CreateBufRegion(Tex->m_Width, Tex->m_Height);
  ClearBuffer(Tex->m_Width, Tex->m_Height, true, NULL, 0);

  CCamera tmp_cam = gRenDev->GetCamera();
  m_PrevCamera = tmp_cam;
  //tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  //tmp_cam.Update();  

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );

  eng->SetCamera(tmp_cam,false);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** Start Draw scene to texture for heat ***\n");

  gRenDev->m_RP.m_PersFlags |= RBPF_DRAWHEATMAP | RBPF_NOCLEARBUF;
  gRenDev->m_RP.m_bDrawToTexture = true;

  //int nDrawFlags = DLD_INDOORS | DLD_ADD_LIGHTSOURCES | DLD_ENTITIES | DLD_PARTICLES | DLD_STATIC_OBJECTS | DLD_FAR_SPRITES | DLD_TERRAIN_FULLRES | DLD_TERRAIN_LIGHT | DLD_TERRAIN_WATER | DLD_NEAR_OBJECTS;
  //eng->DrawLowDetail(nDrawFlags);
}

void CGLTexMan::EndNightMap()
{
  STexPic *Tex = gRenDev->m_TexMan->m_Text_NightVisMap;
  gRenDev->m_RP.m_PersFlags &= ~(RBPF_DRAWNIGHTMAP | RBPF_NOCLEARBUF);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** End Draw scene to texture for night ***\n");

  SetTexture(Tex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Tex->m_Width, Tex->m_Height);

  gRenDev->SetCamera(m_PrevCamera);
  iSystem->SetViewCamera(m_PrevCamera);

  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);

  if (CRenderer::CV_r_nighttype == 1)
  {
    int i;
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, gRenDev->GetWidth(), 0.0, gRenDev->GetHeight(), -20.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    gRenDev->SetCullMode(R_CULL_NONE);
    gRenDev->SetColorOp(eCO_MODULATE, eCO_MODULATE, DEF_TEXARG0, DEF_TEXARG0);
    float wdt = (float)Tex->m_Width;
    float hgt = (float)Tex->m_Height;
    gRenDev->EF_SetState(GS_NODEPTHTEST);
    SetTexture(Tex->m_Bind, eTT_Base);

    glColor4f(1,1,1,1);

    GLQuad(wdt, hgt);

    gRenDev->EF_SetState(GS_BLSRC_SRCALPHA | GS_BLDST_ONEMINUSSRCALPHA | GS_NODEPTHTEST);
    for (i=0; i<4; i++)
    {
      float fOffs = (i+1)*(Tex->m_Width/1024.0f);
      float fAlpha = 0.1f;
      glColor4f(1,1,1,fAlpha);

      glPushMatrix();
      glTranslatef(fOffs,0,0);
      GLQuad(wdt, hgt);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(-fOffs,0,0);
      GLQuad(wdt, hgt);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(0,fOffs,0);
      GLQuad(wdt, hgt);
      glPopMatrix();

      glPushMatrix();
      glTranslatef(0,-fOffs,0);
      GLQuad(wdt, hgt);
      glPopMatrix();
    }

    float fRandU = RandomNum() * 2.0f;
    float fRandV = RandomNum() * 2.0f;
    gRenDev->EF_SetState(GS_BLSRC_DSTCOL | GS_BLDST_SRCCOL | GS_NODEPTHTEST);
    STexPic *tp = gRenDev->m_TexMan->LoadTexture("Textures/Defaults/HeatNoise", 0, 0);
    tp->Set();

    glBegin(GL_QUADS);

    glTexCoord2f(fRandU, fRandV);
    glVertex3f(0, 0, 0);

    glTexCoord2f(4.0f+fRandU, fRandV);
    glVertex3f(wdt, 0, 0);

    glTexCoord2f(4.0f+fRandU, 4.0f+fRandV);
    glVertex3f(wdt, hgt, 0);

    glTexCoord2f(fRandU, 4.0f+fRandV);
    glVertex3f(0, hgt, 0);

    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    SetTexture(Tex->m_Bind, eTT_Base);

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Tex->m_Width, Tex->m_Height);

  }

  if (CRenderer::CV_r_nightmapsave)
  {
    CRenderer::CV_r_nightmapsave = 0;
    Tex->SaveJPG("NightMap.jpg", false);
  }

  SetTexture(0, eTT_Base);

  //ClearBuffer(Tex->m_Width, Tex->m_Height, true, NULL,0);

  Tex->m_Flags &= ~FT_BUILD;
  gRenDev->m_RP.m_bDrawToTexture = false;

  gRenDev->ResetToDefault();
}

void CGLTexMan::StartNightMap(int Id)
{
  if (!iSystem)
    return;

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  STexPic *Tex = gRenDev->m_TexMan->m_Text_NightVisMap;
  if (Tex->m_Flags & FT_BUILD)
    return;

  if (CRenderer::CV_r_nightsize != gRenDev->m_RP.m_NightSize)
  {
    if (CRenderer::CV_r_nightsize <= 64)
      CRenderer::CV_r_nightsize = 64;
    else
    if (CRenderer::CV_r_nightsize <= 128)
      CRenderer::CV_r_nightsize = 128;
    else
    if (CRenderer::CV_r_nightsize <= 256)
      CRenderer::CV_r_nightsize = 256;
    else
    if (CRenderer::CV_r_nightsize <= 512)
      CRenderer::CV_r_nightsize = 512;

    CRenderer::CV_r_nightsize = sLimitSizeByScreenRes(CRenderer::CV_r_nightsize);
    gRenDev->m_RP.m_NightSize = CRenderer::CV_r_nightsize;
    Tex->m_Flags &= ~FT_ALLOCATED;
  }
  int nTexWidth = gRenDev->m_RP.m_NightSize;
  int nTexHeight = gRenDev->m_RP.m_NightSize;

  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  Tex->m_Flags |= FT_BUILD;
  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    Tex->m_Width = nTexWidth;
    Tex->m_Height = nTexHeight;
    Tex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    AddToHash(Tex->m_Bind, Tex);
    SetTexture(Tex->m_Bind, eTT_Base);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Tex->m_Width, Tex->m_Height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    SetTexture(0, eTT_Base);
  }

  CreateBufRegion(Tex->m_Width, Tex->m_Height);
  ClearBuffer(Tex->m_Width, Tex->m_Height, true, NULL, 0);

  CCamera tmp_cam = gRenDev->GetCamera();
  m_PrevCamera = tmp_cam;
  //tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  //tmp_cam.Update();  

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );

  eng->SetCamera(tmp_cam,false);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** Start Draw scene to texture for night ***\n");

  gRenDev->m_RP.m_PersFlags |= RBPF_DRAWNIGHTMAP | RBPF_NOCLEARBUF;
  gRenDev->m_RP.m_bDrawToTexture = true;

  //int nDrawFlags = DLD_INDOORS | DLD_ADD_LIGHTSOURCES | DLD_ENTITIES | DLD_PARTICLES | DLD_STATIC_OBJECTS | DLD_FAR_SPRITES | DLD_TERRAIN_FULLRES | DLD_TERRAIN_LIGHT;
  //eng->DrawLowDetail(nDrawFlags);
}

void CGLTexMan::StartRefractMap(int Id)
{
}
void CGLTexMan::EndRefractMap()
{
}


void CGLTexMan::StartScreenMap(int Id)
{
  STexPic *tx = gRenDev->m_TexMan->m_Text_EnvScr; 

  // tiago: added
  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  int width = m_TempWidth;
  int height = m_TempHeight;

  if (!(tx->m_Flags & FT_ALLOCATED))
  {
    tx->m_Flags |= FT_ALLOCATED;
    tx->m_Flags2 |= FT2_RECTANGLE;
    tx->m_TargetType = GL_TEXTURE_RECTANGLE_NV;
    tx->m_Width = width;
    tx->m_Height = height;
    SetTexture(tx->m_Bind, eTT_Rectangle);
    glTexImage2D(tx->m_TargetType, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  }
  gRenDev->m_RP.m_bDrawToTexture = true;
  gRenDev->m_RP.m_PersFlags |= RBPF_DRAWSCREENMAP;// | RBPF_NOCLEARBUF;
}

void CGLTexMan::EndScreenMap()
{
  STexPic *tx = gRenDev->m_TexMan->m_Text_EnvScr;
  SetTexture(tx->m_Bind, eTT_Rectangle);
  glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, tx->m_Width, tx->m_Height);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  SetTexture(0, eTT_Rectangle);
  gRenDev->m_RP.m_PersFlags &= ~(RBPF_DRAWSCREENMAP);// | RBPF_NOCLEARBUF);
  gRenDev->m_RP.m_bDrawToTexture = false;
}

// ===============================================================
// ScreenTexMap - get screen sized texture, for shared use
// Last Update: 17/09/2003

// prepare screen texture
void CGLTexMan::StartScreenTexMap(int Id)
{
  if(gRenDev)
  {
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** Begin of StartScreenTexMap ***\n");
  }

  // for screen texture, only rectangular textures used, if not supported, no screen fx's
  if(!SUPPORTS_GL_NV_texture_rectangle && !SUPPORTS_GL_EXT_texture_rectangle)
  {
    return;
  }

  STexPic *tx = gRenDev->m_TexMan->m_Text_ScreenMap; 

  // tiago: added
  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  int width = m_TempWidth;
  int height = m_TempHeight;

  // recreate when necessary
  if (!(tx->m_Flags & FT_ALLOCATED))
  {
    tx->m_Flags |= FT_ALLOCATED;
    tx->m_Flags2 |= FT2_RECTANGLE;

    
    if(SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
      tx->m_TargetType = GL_TEXTURE_RECTANGLE_NV;
  
    tx->m_Width = width;
    tx->m_Height = height;    
    SetTexture(tx->m_Bind, eTT_Rectangle);
    glTexImage2D(tx->m_TargetType, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  }
  gRenDev->m_RP.m_bDrawToTexture = true;
  gRenDev->m_RP.m_PersFlags |= RBPF_DRAWSCREENTEXMAP; //| RBPF_NOCLEARBUF;

  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight); 
  //glClear(GL_DEPTH_BUFFER_BIT); 
}

// get screen texture
void CGLTexMan::EndScreenTexMap()
{ 
  if(gRenDev)
  {
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** End of StartScreenTexMap ***\n");
  }

  // for screen texture, only rectangular textures used, if not supported, no screen fx's
  if(!SUPPORTS_GL_NV_texture_rectangle && !SUPPORTS_GL_EXT_texture_rectangle)
  {
    return;
  }

  STexPic *tx = gRenDev->m_TexMan->m_Text_ScreenMap;
  SetTexture(tx->m_Bind, eTT_Rectangle);    

  unsigned int nTargetType;

  if (SUPPORTS_GL_NV_texture_rectangle || SUPPORTS_GL_EXT_texture_rectangle)
    nTargetType=GL_TEXTURE_RECTANGLE_NV;

  glCopyTexSubImage2D(nTargetType, 0, 0, 0, 0, 0, tx->m_Width, tx->m_Height);  
  glTexParameteri(nTargetType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(nTargetType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(nTargetType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(nTargetType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);      

  SetTexture(0, eTT_Rectangle);

  gRenDev->m_RP.m_PersFlags &= ~(RBPF_DRAWSCREENTEXMAP); // | RBPF_NOCLEARBUF);
  gRenDev->m_RP.m_bDrawToTexture = false;
}

//==================================================================================

void CGLTexMan::DrawToTextureForRainMap(int Id)
{
  if (!iSystem)
    return;

  I3DEngine *eng = (I3DEngine *)iSystem->GetI3DEngine();

  STexPic *Tex = gRenDev->m_TexMan->m_Text_RainMap;
  if (Tex->m_Flags & FT_BUILD)
    return;

  if (CRenderer::CV_r_rainmapsize != gRenDev->m_RP.m_RainMapSize)
  {
    if (CRenderer::CV_r_rainmapsize <= 64)
      CRenderer::CV_r_rainmapsize = 64;
    else
    if (CRenderer::CV_r_rainmapsize <= 128)
      CRenderer::CV_r_rainmapsize = 128;
    else
    if (CRenderer::CV_r_rainmapsize <= 256)
      CRenderer::CV_r_rainmapsize = 256;
    else
    if (CRenderer::CV_r_rainmapsize <= 512)
      CRenderer::CV_r_rainmapsize = 512;

    CRenderer::CV_r_rainmapsize = sLimitSizeByScreenRes(CRenderer::CV_r_rainmapsize);
    gRenDev->m_RP.m_RainMapSize = CRenderer::CV_r_rainmapsize;
    Tex->m_Flags &= ~FT_ALLOCATED;
  }
  int nTexWidth = gRenDev->m_RP.m_RainMapSize;
  int nTexHeight = gRenDev->m_RP.m_RainMapSize;

  float fMaxDist = eng->GetMaxViewDistance();
  float fMinDist = 0.25f;

  Tex->m_Flags |= FT_BUILD;
  if (!(Tex->m_Flags & FT_ALLOCATED))
  {
    Tex->m_Width = nTexWidth;
    Tex->m_Height = nTexHeight;
    Tex->m_Flags |= FT_ALLOCATED;
    // Preallocate texture
    AddToHash(Tex->m_Bind, Tex);
    SetTexture(Tex->m_Bind, eTT_Base);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Tex->m_Width, Tex->m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    SetTexture(0, eTT_Base);
  }

  CreateBufRegion(Tex->m_Width, Tex->m_Height);
  ClearBuffer(Tex->m_Width, Tex->m_Height, true, NULL, 0);

  //gRenDev->EF_SaveDLights();

  CCamera tmp_cam = gRenDev->GetCamera();
  CCamera prevCamera = tmp_cam;
  //tmp_cam.Init(Tex->m_Width, Tex->m_Height, DEFAULT_FOV, fMaxDist, 1.0f, fMinDist);
  //tmp_cam.Update();  

  gRenDev->GetViewport(&m_TempX, &m_TempY, &m_TempWidth, &m_TempHeight);
  gRenDev->SetViewport( 0, 0, Tex->m_Width, Tex->m_Height );
  gRenDev->EF_PushFog();
  gRenDev->m_RP.m_bDrawToTexture = true;

  eng->SetCamera(tmp_cam,false);

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, "*** Draw scene to texture for rainmap ***\n");

  eng->DrawRain();

  if (gRenDev->m_LogFile)
    gRenDev->Logv(SRendItem::m_RecurseLevel, ".. End DrawLowDetail .. (DrawToTextureForRainMap)\n");

  SetTexture(Tex->m_Bind, eTT_Base);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Tex->m_Width, Tex->m_Height);

  gRenDev->SetCamera(prevCamera);
  iSystem->SetViewCamera(prevCamera);
/*
  {
    byte *pic = new byte [nTexWidth * nTexHeight * 4];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pic);
    ::WriteTGA(pic,nTexWidth,nTexHeight,"RainMapAlpha.tga", 32); 
    delete [] pic;
  }
	*/
  gRenDev->SetViewport(m_TempX, m_TempY, m_TempWidth, m_TempHeight);

  SetTexture(0, eTT_Base);

  //ClearBuffer(Tex->m_Width, Tex->m_Height, true, NULL,0);

  Tex->m_Flags &= ~FT_BUILD;
  gRenDev->m_RP.m_bDrawToTexture = false;
  gRenDev->EF_PopFog();

  gRenDev->ResetToDefault();
  //gRenDev->EF_RestoreDLights();
}

//==================================================================================
extern int nTexSize;
extern int nFrameTexSize;

int TexCallback( const VOID* arg1, const VOID* arg2 )
{
  STexPic **pi1 = (STexPic **)arg1;
  STexPic **pi2 = (STexPic **)arg2;
  STexPic *ti1 = *pi1;
  STexPic *ti2 = *pi2;
  if (ti1->m_Size > ti2->m_Size)
    return -1;
  if (ti1->m_Size < ti2->m_Size)
    return 1;
  return 0;
}


void CGLTexMan::Update()
{
  CGLRenderer *rd = gcpOGL;
  int i;
  char buf[256]="";

  CheckTexLimits(NULL);

  if (CRenderer::CV_r_texresolution != m_CurTexResolution || CRenderer::CV_r_texbumpresolution != m_CurTexBumpResolution || CRenderer::CV_r_texquality != m_CurTexQuality || CRenderer::CV_r_texbumpquality != m_CurTexBumpQuality || CRenderer::CV_r_texskyquality != m_CurTexSkyQuality || CRenderer::CV_r_texskyresolution != m_CurTexSkyResolution || CRenderer::CV_r_texmaxsize != m_CurTexMaxSize || CRenderer::CV_r_texminsize != m_CurTexMinSize)
  {
    for (i=0; i<m_Textures.Num(); i++)
    {
      STexPic *tp = m_Textures[i];
      if (!tp || !tp->m_bBusy)
        continue;
      if (!(tp->m_Flags2 & FT2_WASLOADED))
        continue;
      if (tp->m_Flags & FT_NOREMOVE)
        continue;
      if (tp->m_eTT == eTT_Cubemap)
      {
        //if (tp->m_CubeSide > 0)
          continue;
        if (CRenderer::CV_r_texmaxsize != m_CurTexMaxSize || CRenderer::CV_r_texminsize != m_CurTexMinSize || CRenderer::CV_r_texresolution != m_CurTexResolution || CRenderer::CV_r_texquality != m_CurTexQuality)
        {
          gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
        }
      }
      else
      if (tp->m_eTT == eTT_Bumpmap || tp->m_eTT == eTT_DSDTBump)
      {
        if (CRenderer::CV_r_texmaxsize != m_CurTexMaxSize || CRenderer::CV_r_texminsize != m_CurTexMinSize || CRenderer::CV_r_texbumpresolution != m_CurTexBumpResolution || CRenderer::CV_r_texbumpquality != m_CurTexBumpQuality )
        {
          gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
        }
      }
      else
      {
        if (tp->m_Flags & FT_SKY)
        {
          if (CRenderer::CV_r_texmaxsize != m_CurTexMaxSize || CRenderer::CV_r_texminsize != m_CurTexMinSize || CRenderer::CV_r_texskyresolution != m_CurTexSkyResolution || CRenderer::CV_r_texskyquality != m_CurTexSkyQuality)
          {
            gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
          }
        }
        else
        {
          if (CRenderer::CV_r_texmaxsize != m_CurTexMaxSize || CRenderer::CV_r_texminsize != m_CurTexMinSize || CRenderer::CV_r_texresolution != m_CurTexResolution || CRenderer::CV_r_texquality != m_CurTexQuality)
          {
            gRenDev->EF_LoadTexture(tp->m_SearchName.c_str(), tp->m_Flags, tp->m_Flags2 | FT2_RELOAD, tp->m_eTT, tp->m_fAmount1, tp->m_fAmount2, tp->m_Id);
          }
        }
      }
    }
    m_CurTexResolution = CRenderer::CV_r_texresolution;
    m_CurTexBumpResolution = CRenderer::CV_r_texbumpresolution;
    m_CurTexQuality = CRenderer::CV_r_texquality;
    m_CurTexBumpQuality = CRenderer::CV_r_texbumpquality;
    m_CurTexSkyResolution = CRenderer::CV_r_texskyresolution;
    m_CurTexSkyQuality = CRenderer::CV_r_texskyquality;
    m_CurTexMaxSize = CRenderer::CV_r_texmaxsize;
    m_CurTexMinSize = CRenderer::CV_r_texminsize;
  }

  if (CRenderer::CV_r_logusedtextures == 1 || CRenderer::CV_r_logusedtextures == 3 || CRenderer::CV_r_logusedtextures == 4 || CRenderer::CV_r_logusedtextures == 5)
  {
    FILE *fp = NULL;
    TArray<STexPic *> Texs;
    int Size = 0;
    int PartSize = 0;

    static char *sTexType[] = 
    {
      "Base","Cubemap","AutoCubemap","Bump","DSDTBump","Rectangle"
    };
    static char *sTexFormat[] = 
    {
      "Unknown","Index8","HSV","0888","8888","RGBA","8000","0565","0555","4444","1555","DXT1","DXT3","DXT5","SIGNED_HILO16","SIGNED_HILO8","SIGNED_RGB8","RGB8","DSDT_MAG","DSDT","0088"
    };

    if (CRenderer::CV_r_logusedtextures == 1 || CRenderer::CV_r_logusedtextures == 3 || CRenderer::CV_r_logusedtextures == 5)
    {
      for (i=0; i<m_Textures.Num(); i++)
      {
        if (CRenderer::CV_r_logusedtextures == 3 && m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind == gRenDev->m_TexMan->m_Text_NoTexture->m_Bind)
        {
          if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
            Texs.AddElem(m_Textures[i]);
        }
        else
        if (CRenderer::CV_r_logusedtextures == 1 && m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && (m_Textures[i]->m_Flags2 & FT2_WASLOADED) && !(m_Textures[i]->m_Flags2 & FT2_WASUNLOADED))
        {
          if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
            Texs.AddElem(m_Textures[i]);
        }
        else
        if (CRenderer::CV_r_logusedtextures == 5 && m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && !(m_Textures[i]->m_Flags2 & FT2_WASLOADED))
        {
          if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
            Texs.AddElem(m_Textures[i]);
        }
      }
      if (CRenderer::CV_r_logusedtextures == 3)
        fp = fxopen("MissingTextures.txt", "w");
      else
      if (CRenderer::CV_r_logusedtextures == 1)
        fp = fxopen("UsedTextures.txt", "w");
      else
        fp = fxopen("UsedFuncTextures.txt", "w");
      fprintf(fp, "*** All loaded textures: ***\n");
      qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
      for (i=0; i<Texs.Num(); i++)
      {
        fprintf(fp, "%d\t\tType: %s\t\tFormat: %s\t\t(%s)\n", Texs[i]->m_Size, sTexType[Texs[i]->m_eTT], sTexFormat[Texs[i]->m_ETF], *Texs[i]->m_SearchName);
        Size += Texs[i]->m_Size;
        PartSize += Texs[i]->m_LoadedSize;
      }
      fprintf(fp, "*** Total Size: %d\n\n", Size, PartSize, PartSize);
      
      Texs.Free();
    }
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && m_Textures[i]->m_AccessFrame == rd->GetFrameID())
      {
        if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
          Texs.AddElem(m_Textures[i]);
      }
    }
    if (fp)
      fprintf(fp, "\n\n*** Textures used in current frame: ***\n");
    else
      rd->TextToScreenColor(4,13, 1,1,0,1, "*** Textures used in current frame: ***");
    int nY = 17;
    qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
    Size = 0;
    for (i=0; i<Texs.Num(); i++)
    {
      if (fp)
        fprintf(fp, "%.3fKb\t\tType: %s\t\tFormat: %s\t\t(%s)\n", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
      else
      {
        sprintf(buf, "%.3fKb  Type: %s  Format: %s  (%s)", Texs[i]->m_Size/1024.0f, sTexType[Texs[i]->m_eTT], sTexFormat[Texs[i]->m_ETF], Texs[i]->m_SearchName.c_str());
        rd->TextToScreenColor(4,nY, 0,1,0,1, buf);
        nY += 3;
      }
      Size += Texs[i]->m_Size;
    }
    if (fp)
    {
      fprintf(fp, "*** Total Size: %.3fMb\n\n", Size/(1024.0f*1024.0f));
      fclose (fp);
    }
    else
    {
      sprintf(buf, "*** Total Size: %.3fMb", Size/(1024.0f*1024.0f));
      rd->TextToScreenColor(4,nY+1, 0,1,1,1, buf);
    }
    
    if (CRenderer::CV_r_logusedtextures != 4)
      CRenderer::CV_r_logusedtextures = 0;
  }
  else
  if (CRenderer::CV_r_logusedtextures == 2)
  {
    //char *str = GetTexturesStatusText();

    TArray<STexPic *> Texs;
    TArray<STexPic *> TexsNM;
    int i;
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind)
      {
        if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
        {
          /*for (int nn = 0; nn<sTestStr.Num(); nn++)
          {
            if (sTestStr[nn] == m_Textures[i])
              break;
          }
          if (nn == sTestStr.Num())
          {
            int nnn = 0;
          }*/
          Texs.AddElem(m_Textures[i]);
          if (m_Textures[i]->m_eTT == eTT_Bumpmap || m_Textures[i]->m_eTT == eTT_DSDTBump)
            TexsNM.AddElem(m_Textures[i]);
        }
      }
    }
    int SizeNOTO = 0;
    int nNOTO = 0;
    for (i=0; i<TX_FIRSTBIND; i++)
    {
      if (BindSizes[i])
      {
        STexPic *tp = GetByID(i);
        if (!tp)
        {
          nNOTO++;
          SizeNOTO += BindSizes[i];
        }
      }
    }
    /*for (int nn = 0; nn<sTestTx.Num(); nn++)
    {
      for (i=0; i<Texs.Num(); i++)
      {
        if (sTestTx[nn] == Texs[i])
          break;
      }
      if (i == Texs.Num())
      {
        int nnn = 0;
      }
    }*/
    qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
    int AllSize = 0;
    int AllSizeNM = 0;
    int Size = 0;
    int PartSize = 0;
    int NonStrSize = 0;
    int SizeNM = 0;
    int PartSizeNM = 0;
    int nLoaded = 0;
    for (i=0; i<Texs.Num(); i++)
    {
      AllSize += Texs[i]->m_Size;
      if (!Texs[i]->IsStreamed())
        NonStrSize += Texs[i]->m_Size;
      if (!(Texs[i]->m_Flags2 & FT2_WASUNLOADED))
      {
        nLoaded++;
        Size += Texs[i]->m_Size;
        if (Texs[i]->m_LoadedSize)
          PartSize += Texs[i]->m_LoadedSize;
        else
          PartSize += Texs[i]->m_Size;
      }
    }
    for (i=0; i<TexsNM.Num(); i++)
    {
      AllSizeNM += Texs[i]->m_Size;
      if (!(Texs[i]->m_Flags2 & FT2_WASUNLOADED))
      {
        SizeNM += TexsNM[i]->m_Size;
        if (TexsNM[i]->m_LoadedSize)
          PartSizeNM += TexsNM[i]->m_LoadedSize;
        else
          PartSizeNM += TexsNM[i]->m_Size;
      }
    }
    sprintf(buf, "All API's textures: FullSize: %.3fMb (nNoTO: %d, SizeNoTO: %.3fMb)", nTexSize/(1024.0f*1024.0f), nNOTO, SizeNOTO/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,10, 1,1,0,1, buf);
    sprintf(buf, "All texture objects: %d (Size: %.3fMb, NonStreamed: %.3fMb)", Texs.Num(), AllSize/(1024.0f*1024.0f), NonStrSize/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,13, 1,1,0,1, buf);
    sprintf(buf, "All loaded texture objects: %d (All MIPS: %.3fMb, Loaded MIPS: %.3fMb)", nLoaded, Size/(1024.0f*1024.0f), PartSize/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,16, 1,1,0,1, buf);
    sprintf(buf, "All Normal Maps: %d (FullSize: %.3fMb, All MIPS: %.3fMb, Loaded MIPS: %.3fMb)", TexsNM.Num(), AllSizeNM/(1024.0f*1024.0f), SizeNM/(1024.0f*1024.0f), PartSizeNM/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,19, 1,1,0,1, buf);

    Texs.Free();
    for (i=0; i<m_Textures.Num(); i++)
    {
      if (m_Textures[i] && m_Textures[i]->m_bBusy && m_Textures[i]->m_Bind != gRenDev->m_TexMan->m_Text_NoTexture->m_Bind && m_Textures[i]->m_AccessFrame == rd->GetFrameID())
      {
        if (m_Textures[i]->m_eTT != eTT_Cubemap || !m_Textures[i]->m_CubeSide)
          Texs.AddElem(m_Textures[i]);
      }
    }
    qsort(&Texs[0], Texs.Num(), sizeof(STexPic *), TexCallback );
    Size = 0;
    PartSize = 0;
    NonStrSize = 0;
    for (i=0; i<Texs.Num(); i++)
    {
      Size += Texs[i]->m_Size;
      if (Texs[i]->m_LoadedSize)
        PartSize += Texs[i]->m_LoadedSize;
      else
        PartSize += Texs[i]->m_Size;
      if (!Texs[i]->IsStreamed())
        NonStrSize += Texs[i]->m_Size;
    }
    sprintf(buf, "Current tex. objects: %d (Size: %.3fMb, Loaded: %.3f, NonStreamed: %.3f)", Texs.Num(), Size/(1024.0f*1024.0f), PartSize/(1024.0f*1024.0f), NonStrSize/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,24, 1,0,0,1, buf);
    int n = 0;
    Size = 0;
    for (i=0; i<16384; i++)
    {
      if (BindFrame[i] == rd->GetFrameID())
      {
        if (i < TX_FIRSTBIND)
          Size += BindSizes[i];
        n++;
      }
    }
    sprintf(buf, "Current API textures: %d (Size: %.3f, SizeNoManaged: %.3f)", n, nFrameTexSize/(1024.0f*1024.0f), Size/(1024.0f*1024.0f));
    rd->TextToScreenColor(4,27, 1,0,0,1, buf);

		{ // count shadow map memory
			float nShadowMapSize = 0;
			int nShadowMapCount= 0;
			for(int i=0; i<MAX_DYNAMIC_SHADOW_MAPS_COUNT; i++)
			{
				if(((CGLRenderer*)rd)->m_ShadowTexIDBuffer[i].nTexId)
				{
					nShadowMapSize += ((CGLRenderer*)rd)->m_ShadowTexIDBuffer[i].nTexSize*((CGLRenderer*)rd)->m_ShadowTexIDBuffer[i].nTexSize*3;
					nShadowMapCount++;
				}
			}

			sprintf(buf, "Allocated shadow maps: %d (%.2f mb)", nShadowMapCount, nShadowMapSize/(1024.0f*1024.0f));
			rd->TextToScreenColor(4,30, 1,0,0,1, buf);
		}
  }
}

//==================================================================================

STexPic *CGLRenderer::EF_MakePhongTexture(int Exp)
{
  char name[128];

  sprintf(name, "$Phong_%d", Exp);
  STexPic *tp = m_TexMan->LoadTexture(name, 0, 0, eTT_Base);
  if (tp->m_Flags & FT_ALLOCATED)
    return tp;
  tp->m_Flags |= FT_ALLOCATED;
  tp->m_Bind = TX_FIRSTBIND + tp->m_Id;
  //sTestStr.AddElem(tp);
  tp->Set();
  tp->m_Flags &= ~FT_NOTFOUND;
  CGLTexMan *tm = (CGLTexMan *)m_TexMan;
  tm->MakePhongLookupTexture((float)Exp, tp);
  m_TexMan->SetTexture(0, eTT_Base);

  return tp;
}

bool CGLRenderer::EF_SetLightHole(Vec3d vPos, Vec3d vNormal, int idTex, float fScale, bool bAdditive)
{
  static const GLenum cubefaces[6] = 
  {
    GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT,
  };

  assert(idTex);

  //ITexPic *pi = EF_LoadTexture("ricochet32b", 0, 0, eTT_Base);
  //idTex = pi->GetTextureID();

  ResetToDefault();

  STexPic *Pic = m_TexMan->GetByID(idTex);
  if (!Pic)
    Pic = gRenDev->m_TexMan->m_Text_White;
  assert(Pic->m_Flags2 & FT2_WASLOADED);
  //Pic = (STexPic *)EF_LoadTexture("textures/decal/default", 0, 0, eTT_Base);

  /*static bool bFreeze;
  static float fLastTime;
  static Vec3d sPos;
  bool bGo = false;
  if ((GetAsyncKeyState('F') & 0x8000))
    bFreeze = 1;
  else
  if ((GetAsyncKeyState('U') & 0x8000))
    bFreeze = 0;
  if ((GetAsyncKeyState('G') & 0x8000) && m_RP.m_RealTime-fLastTime > 1.0f)
  {
    fLastTime = m_RP.m_RealTime;
    bGo = true;
  }
  if (!bFreeze)
    sPos = GetCamera().GetPos();
  SetMaterialColor(1,1,1,1);
  DrawBall(sPos[0], sPos[1], sPos[2], 0.05f);

  if (!bGo)
    return false;

  vPos = sPos;*/

  // Looking for opposite light source
  int i;
  //vNormal = Vec3d(0,1,0);
  for (i=0; i<m_RP.m_DLights[SRendItem::m_RecurseLevel].Num(); i++)
  {
    CDLight *dl = m_RP.m_DLights[SRendItem::m_RecurseLevel][i];
    if (!dl || !dl->m_pLightImage)
      continue;
    Vec3d p = vPos - dl->m_Origin;
    p.Normalize();
    float d = p.Dot(vNormal);
    if (d >= 0.5f)
      break;
  }
  if (i == m_RP.m_DLights[SRendItem::m_RecurseLevel].Num())
    return false;

  CDLight *pLight = m_RP.m_DLights[SRendItem::m_RecurseLevel][i];

  assert(pLight);

  /*if ((GetAsyncKeyState('R') & 0x8000) && m_RP.m_RealTime-fLastTime > 1.0f)
  {
    fLastTime = m_RP.m_RealTime;
    STexPic *pImage = (STexPic *)pLight->m_pLightImage;
    char name[128];
    strcpy(name, *pImage->m_FullName);
    pImage->Release(false);
    pLight->m_pLightImage = EF_LoadTexture(name, FT_CLAMP, FT2_FORCECUBEMAP | FT2_NODXT, eTT_Cubemap);
    bGo = false;
  }*/

  float frustMatr[16];
  float viewMatr[16];

  Vec3d PosL = pLight->m_Origin;
  Vec3d TargL = pLight->m_Origin + pLight->m_Orientation.m_vForward;
  //pLight->m_fLightFrustumAngle = 45.0f;
  makeProjectionMatrix( pLight->m_fLightFrustumAngle*2, 1, 1, 99999.0f, frustMatr);
  SGLFuncs::gluLookAt( PosL.x, PosL.y, PosL.z, TargL.x, TargL.y, TargL.z, pLight->m_Orientation.m_vUp[0], pLight->m_Orientation.m_vUp[1], pLight->m_Orientation.m_vUp[2], viewMatr);

  const int vp[4] = {0,0,1,1};
  float px, py, pz;
  SGLFuncs::gluProject(vPos.x,vPos.y,vPos.z, viewMatr, frustMatr, vp, &px, &py, &pz);
  int Side = 0;
  if (px>=0 && px<=1.0f && py>=0 && py<=1.0f)
  {
    STexPic *pImage = (STexPic *)((ITexPic*)pLight->m_pLightImage);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, m_width, 0.0, m_height, -9999.0, 9999.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    Vec3d crd[4];
    switch(Side)
    {
      case 0: //posx
        crd[0] = Vec3d(1,1,1);
        crd[1] = Vec3d(1,-1,1);
        crd[2] = Vec3d(1,-1,-1);
        crd[3] = Vec3d(1,1,-1);
        break;
      case 1: //negx
        crd[0] = Vec3d(-1,1,1);
        crd[1] = Vec3d(-1,-1,1);
        crd[2] = Vec3d(-1,-1,-1);
        crd[3] = Vec3d(-1,1,-1);
        break;
      case 2: //posy
        crd[0] = Vec3d(1,1,1);
        crd[1] = Vec3d(-1,1,1);
        crd[2] = Vec3d(-1,1,-1);
        crd[3] = Vec3d(1,1,-1);
        break;
      case 3: //negy
        crd[0] = Vec3d(1,-1,1);
        crd[1] = Vec3d(-1,-1,1);
        crd[2] = Vec3d(-1,-1,-1);
        crd[3] = Vec3d(1,-1,-1);
        break;
      case 4: //posz
        crd[0] = Vec3d(1,1,1);
        crd[1] = Vec3d(-1,1,1);
        crd[2] = Vec3d(-1,-1,1);
        crd[3] = Vec3d(1,-1,1);
        break;
      case 5: //negz
        crd[0] = Vec3d(1,1,-1);
        crd[1] = Vec3d(-1,1,-1);
        crd[2] = Vec3d(-1,-1,-1);
        crd[3] = Vec3d(1,-1,-1);
        break;
    }
    glDisable(GL_BLEND);   
	  glDisable(GL_STENCIL_TEST);
  	
    glDisable(GL_DEPTH_TEST);
    glDepthMask(1);
    glDisable(GL_CULL_FACE);

	  pImage->Set();
    glEnable(pImage->m_TargetType);

    float fWdt = (float)pImage->m_Width;
    float fHgt = (float)pImage->m_Height;

    glBegin(GL_QUADS);

	  glColor3f(1,1,1);

    glTexCoord3f(crd[0][0], crd[0][1], crd[0][2]);
	  glVertex3f(0.0f, 0.0f, 0.0f);				// Top Left

    glTexCoord3f(crd[1][0], crd[1][1], crd[1][2]);
	  glVertex3f(0.0f, fHgt, 0.0f);				// Bottom Left

    glTexCoord3f(crd[2][0], crd[2][1], crd[2][2]);
	  glVertex3f(fWdt, fHgt, 0.0f);				// Bottom Right

    glTexCoord3f(crd[3][0], crd[3][1], crd[3][2]);
	  glVertex3f(fWdt, 0.0f, 0.0f);				// Top Right

    glEnd();    

    if (pImage->m_eTT == eTT_Cubemap)
      glDisable(pImage->m_TargetType);

    float posx = (1.0f-px) * (float)pImage->m_Width;
    float posy = (1.0f-py) * (float)pImage->m_Height;

    pImage = (STexPic *)Pic;
    pImage->Set();
    float fs = (float)fWdt / 256.0f;
    float fSize = fScale / 2.0f * (float)pImage->m_Width * fs;
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glBegin(GL_QUADS);

    glTexCoord2f(0, 0);
	  glVertex3f(posx-fSize, posy-fSize, 0.0f);

    glTexCoord2f(0, 1);
	  glVertex3f(posx-fSize, posy+fSize, 0.0f);

    glTexCoord2f(1, 1);
	  glVertex3f(posx+fSize, posy+fSize, 0.0f);

    glTexCoord2f(1, 0);
	  glVertex3f(posx+fSize, posy-fSize, 0.0f);

    glEnd();

    pImage = (STexPic *)((ITexPic*)pLight->m_pLightImage);
    pImage->Set();

    glCopyTexSubImage2D(cubefaces[Side], 0, 0, 0, 0, 0, pImage->m_Width, pImage->m_Height);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (pImage->m_eTT == eTT_Cubemap)
      glDisable(pImage->m_TargetType);
    glDisable(GL_BLEND);

    m_TexMan->SetTexture(0, eTT_Cubemap);
    //pImage->SaveJPG("LightCube");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
  }

  return true;
}

float CGLTexMan::CalcFogVal(float fi, float fj)
{
  float fDelta = fabsf(fj - fi);
  if (fj > 0 && fi > 0)
    return 0;
  float fThr = -8;
  if (fj < fThr && fi < fThr)
    return 1.0f;
  float f1, f2, ff;
  if (fj > 0)
    ff = fj;
  else
  if (fi > 0)
    ff = fi;
  else
    ff = 0;
  if (fi > fj)
  {
    f1 = fi;
    f2 = fj;
  }
  else
  {
    f1 = fj;
    f2 = fi;
  }
  if (f1 > 0)
    f1 = 0;
  if (f2 < fThr)
    f2 = fThr;
  fThr = 1.0f / 8.0f;
  if (fDelta == 0)
    return -(fThr * fi);
  float fFog = (1.0f - ((f2 + f1) * fThr * -0.5f)) * (f1 - f2);
  ff = ((fDelta - ff) - fFog) / fDelta;
  float fMin = min(fi, fj) / -30.0f;
  if (fMin >= 1.0f)
    return 1;
  return (1.0f - fMin) * ff + fMin;
}

void CGLTexMan::GenerateFogMaps()
{
  int i, j;
  {
    float fdata[256];
    byte Data1[128][128][4];
    float f = 1.0f;
    for (i=0; i<256; i++)
    {
      fdata[i] = f;
      f *= 0.982f;
    }
    fdata[0] = 0;
    fdata[255] = 0;
    for (i=0; i<128; i++)
    {
      int ni = i - 64;
      for (j=0; j<128; j++)
      {
        int nj = j - 64;
        float m = (float)(nj*nj + ni*ni);
				float fsq = m ? 1.0f / cry_sqrtf(m) : 1000000; // @todo: fix devide by zero !!!
        int iIndexF = (int)((fsq * m) / 63.0f * 255.0f);
        iIndexF = CLAMP(iIndexF, 0, 255);
        int iFog = (int)((1.0f - fdata[iIndexF]) * 255.0f);
        if (!i || i==127 || !j || j==127)
          iFog = 255;
        Data1[j][i][0] = Data1[j][i][1] = Data1[j][i][2] = 255;
        Data1[j][i][3] = (byte)iFog;
      }
    }
    gRenDev->m_TexMan->m_Text_Fog = CreateTexture("$Fog", 128, 128, 1, FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT, &Data1[0][0][0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

    /*byte Data2[64][64][4];
    gcpOGL->EF_InitFogTables();
    for (i=0; i<64; i++)
    {
      for (j=0; j<64; j++)
      {
        //float fFog = CalcFogVal((float)(i-32), (float)(j-32));
        
        float fi = (float)i-63-10;
        float fj = (float)j-32;
        float fifi = fi/32.0f;
        fifi *= fifi;
        int nInd = (int)(((-fj/8.0f) * fifi) * 255.0f);
        nInd = Clamp(nInd, 0, 255);
        float fFog = SEvalFuncs::m_tFogFloats[nInd];
        int iFog = Clamp(int(fFog*255.0f), 0, 255);
        Data2[j][i][0] = Data2[j][i][1] = Data2[j][i][2] = 255;
        Data2[j][i][3] = (byte)iFog;
      }
    }*/

    //gRenDev->m_TexMan->m_Text_Fog_Enter = DownloadTexture("(FogEnter)", 64, 64, FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT, &Data2[0][0][0], eTT_Base, 0, NULL, 0, eTF_8888);
    //gRenDev->m_TexMan->m_Text_Fog_Enter->SaveTGA("FogEnter.tga", false);
    gRenDev->m_TexMan->m_Text_Fog_Enter = LoadTexture("Textures/FogEnter", FT_CLAMP | FT_NOMIPS | FT_NOREMOVE | FT_HASALPHA, FT2_NODXT);
  }
}

void CGLTexMan::GenerateFlareMap()
{
  int i, j;

  byte data[4][32][4];
  for (i=0; i<32; i++)
  {
    float f = 1.0f - ((fabsf((float)i - 15.5f) - 0.5f) / 16.0f);
    int n = (int)(f*f*255.0f);
    for (j=0; j<4; j++)
    {
      byte b = n;
      if (n < 0)
        b = 0;
      else
      if (n > 255)
        b = 255;
      data[j][i][0] = b;
      data[j][i][1] = b;
      data[j][i][2] = b;
      data[j][i][3] = 255;
    }
  }
  gRenDev->m_TexMan->m_Text_Flare = CreateTexture("$Flare", 32, 4, 1, FT_CLAMP | FT_NOREMOVE, FT2_NODXT, &data[0][0][0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);
}

void CGLTexMan::GenerateGhostMap()
{
}

#define D3DCOLOR_ARGB(a,r,g,b) \
    ((DWORD)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)

void CGLTexMan::GenerateDepthLookup()
{
  int i;

  DWORD data[2048];
  DWORD* pMap = data;
  for (i=0; i<2048; i++)
  {
    *pMap++ = D3DCOLOR_RGBA(i&0xFF, (i&0xFF00)>>3, 0, 0 );
  }
  gRenDev->m_TexMan->m_Text_DepthLookup = CreateTexture("$DepthMap", 2048, 1, 1, FT_CLAMP | FT_NOREMOVE | FT_NOMIPS | FT_PROJECTED, FT2_NODXT, (byte *)&data[0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

  DWORD data2[4][4];
  pMap = &data2[0][0];
  for (i=0; i<4*4; i++)
  {
    *pMap++ = D3DCOLOR_RGBA(0xff, 0xe0, 0, 0 );
  }
  gRenDev->m_TexMan->m_Text_Depth = CreateTexture("$Depth", 4, 4, 1, FT_CLAMP | FT_NOREMOVE | FT_HASALPHA | FT_NOMIPS, FT2_NODXT, (byte *)&data2[0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

  pMap = &data2[0][0];
  for (i=0; i<4*4; i++)
  {
    *pMap++ = D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0 );
  }
  gRenDev->m_TexMan->m_Text_WhiteShadow = CreateTexture("$WhiteShadow", 4, 4, 1, FT_CLAMP | FT_NOREMOVE | FT_HASALPHA | FT_NOMIPS, FT2_NODXT, (byte *)&data2[0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8888);

  byte data3[256];
  byte *pbMap = &data3[0];
  for (i=0; i<256; i++)
  {
    *pbMap++ = i;
  }
  gRenDev->m_TexMan->m_Text_Gradient = CreateTexture("$AlphaGradient", 256, 1, 1, FT_CLAMP | FT_NOREMOVE | FT_HASALPHA | FT_NOMIPS, FT2_NODXT, (byte *)&data3[0], eTT_Base, -1.0f, -1.0f, 0, NULL, 0, eTF_8000);
}

void CGLTexMan::GenerateFuncTextures()
{
  if (gRenDev->GetFeatures() & RFT_BUMP)
  {    
    m_Text_NormalizeCMap = LoadTexture("$NormalizeCMap", FT_NOREMOVE | FT_NOMIPS, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_NORMALIZE_CUBE_MAP);
    //SNormalizeVector norm;
    glEnable(GL_TEXTURE_CUBE_MAP_EXT);
    glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, TO_NORMALIZE_CUBE_MAP);
    //sTestStr.AddElem(m_Text_NormalizeCMap);
    //MakeCubeMap<SNormalizeVector>(norm, GL_RGB8, 256, false, m_Text_NormalizeCMap);
    MakeNormalizeVectorCubeMap(128, gRenDev->m_TexMan->m_Text_NormalizeCMap);
    glDisable(GL_TEXTURE_CUBE_MAP_EXT);    

    gRenDev->m_TexMan->m_Text_LightCMap = LoadTexture("$LightCMap", FT_NOREMOVE, FT2_NODXT, eTT_Cubemap, -1.0f, -1.0f, TO_LIGHT_CUBE_MAP);
    SSingleLight f(16);
    glEnable(GL_TEXTURE_CUBE_MAP_EXT);
    glBindTexture(GL_TEXTURE_CUBE_MAP_EXT, TO_LIGHT_CUBE_MAP);
    //sTestStr.AddElem(m_Text_LightCMap);
    MakeCubeMap<SSingleLight>(f, GL_RGB8, 64, true, gRenDev->m_TexMan->m_Text_LightCMap);
    //gRenDev->m_TexMan->m_Text_LightCMap->SaveJPG("CubeLight.jpg", false);
    glDisable(GL_TEXTURE_CUBE_MAP_EXT);    
  }

  GenerateFogMaps();
  GenerateFlareMap();
  GenerateGhostMap();
  GenerateDepthLookup();
}

void STexPic::Preload (int Flags)
{
}
