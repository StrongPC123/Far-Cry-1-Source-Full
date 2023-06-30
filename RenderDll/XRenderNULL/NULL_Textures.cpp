/*=============================================================================
  PS2_Textures.cpp : PS2 specific texture manager implementation.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Honitch Andrey

=============================================================================*/

#include "RenderPCH.h"
#include "NULL_Renderer.h"

TTextureMap CNULLTexMan::m_RefTexs;
CNULLTexUnit CNULLTexMan::m_TUState[8];
int nTexSize=0;
int nFrameTexSize=0;
int BindSizes[TX_LASTBIND];
int BindFrame[TX_LASTBIND];

//=================================================================================

byte *STexPic::GetData32()
{
  return NULL;
}

bool STexPic::SetFilter(int nFilter)
{
  return true;
}

void STexPic::SaveJPG(const char *nam, bool bMips)
{
}

void STexPic::SaveTGA(const char *nam, bool bMips)
{
}

void STexPic::ReleaseDriverTexture()
{
}

void STexPic::SetWrapping()
{
}

void STexPic::SetFilter()
{
}

STexPic *CNULLTexMan::GetByID(int Id)
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

void CNULLTexMan::RemoveFromHash(int Id, STexPic *ti)
{
  TTextureMapItor it = m_RefTexs.find(Id);
  if (it != m_RefTexs.end())
  {
    if (ti)
      assert(ti == it->second);
    m_RefTexs.erase(Id);
  }
}

STexPic *CNULLTexMan::AddToHash(int Id, STexPic *ti)
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
}

void CNULLTexMan::SetTexture(int Id, ETexType eTT)
{
}

void STexPic::Set(int nTexSlot)
{
}

int STexPic::DstFormatFromTexFormat(ETEX_Format eTF)
{
  return 0;
}
int STexPic::TexSize(int Width, int Height, int DstFormat)
{
  return 0;
}

int SShaderTexUnit::mfSetTexture(int nt)
{
  return 1;
}

bool SShaderPass::mfSetTextures()
{
  return true;
}
void SShaderPass::mfResetTextures()
{
}


CNULLTexMan::~CNULLTexMan()
{
}


STexPic *CNULLTexMan::CreateTexture()
{
#ifdef DEBUGALLOC
#undef new
#endif
  return new STexPic;
#ifdef DEBUGALLOC
#define new DEBUG_CLIENTBLOCK
#endif
}

bool CNULLTexMan::SetFilter(char *tex)  
{
  return true;
}

void CNULLTexMan::CalcMipsAndSize(STexPic *ti)
{
}

ETEX_Format CNULLTexMan::GetTexFormat(int PS2Format)
{
  return eTF_Unknown;
}

int CNULLTexMan::GetTexDstFormat(ETEX_Format eTF)
{
  return 0;
}

int CNULLTexMan::GetTexSrcFormat(ETEX_Format eTF)
{
  return 0;
}

int CNULLTexMan::TexSize(int wdt, int hgt, int mode)
{
  return 0;
}

//==================================================================================
extern int nTexSize;
extern int nFrameTexSize;

int TexCallback( const void* arg1, const void* arg2 )
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

STexPic *CNULLTexMan::CopyTexture(const char *name, STexPic *tiSrc, int CubeSide)
{
  return NULL;
}

STexPic *CNULLTexMan::CreateTexture(const char *name, int wdt, int hgt, int Depth, uint flags, uint flags2, byte *dst, ETexType eTT, float fAmount1, float fAmount2, int DXTSize, STexPic *ti, int bind, ETEX_Format eTF, const char *szSourceName)
{
  return NULL;
}



byte *CNULLTexMan::GenerateDXT_HW(STexPic *ti, EImFormat eF, byte *dst, int *numMips, int *DXTSize, bool bMips)
{
  return NULL;
}

void CNULLTexMan::UpdateTextureData(STexPic *pic, byte *data, int USize, int VSize, bool bProc, int State, bool bPal)
{
}

//=========================================================================
// Offscreen drawing functions

//=========================================================================
// Renderer interface functions

void CNULLRenderer::SetTexture(int tnum, ETexType Type)
{
}

void CNULLRenderer::SetTexture3D(int tid3d)
{
}

uint CNULLRenderer::DownLoadToVideoMemory(unsigned char *data,int w, int h, ETEX_Format eTFSrc, ETEX_Format eTFDst, int nummipmap, bool repeat, int filter, int Id, char *szCacheName, int flags)
{
  return 0;
}

//////////////////////////////////////////////////////////////////////
void CNULLRenderer::UpdateTextureInVideoMemory(uint tnum, unsigned char *newdata,int posx,int posy,int w,int h,ETEX_Format eTF)
{
}

void CNULLRenderer::RemoveTexture(ITexPic * pTexPic)
{
  if(!pTexPic)
    return;

  STexPic * pSTexPic = (STexPic *)pTexPic;
  pSTexPic->Release(false);
}

void CNULLRenderer::RemoveTexture(unsigned int nTextureId)
{
  if(nTextureId)
  {
    STexPic *tp = m_TexMan->GetByID(nTextureId);
    if (tp)
      tp->Release(false);
  }
}

///////////////////////////////////////////////////////////////////////////////////
uint CNULLRenderer::LoadTexture(const char * _filename,int *tex_type,unsigned int def_tid,bool compresstodisk,bool bWarn)
{
  return m_TexMan->m_Text_NoTexture->GetTextureID();
}

int CNULLRenderer::LoadAnimatedTexture(const char * szFileNameFormat,const int nCount)
{
  return 0;
}


void CNULLRenderer::SetTexClampMode(bool clamp)
{
}

void CNULLRenderer::DrawObjSprites (list2<CStatObjInst*> *pList, float fMaxViewDist, CObjManager *pObjMan)
{
}

uint CNULLRenderer::MakeSprite(float _object_scale, int nTexSize, float angle, IStatObj * pStatObj, uchar * _pTmpBuffer, uint def_tid)
{
  return 0;
}

uint CNULLRenderer::Make3DSprite(int nTexSize, float fAngleStep, IStatObj * pStatObj)
{
  return 0;
}

int CNULLRenderer::GenerateAlphaGlowTexture(float k)
{
  return 0;
}

bool CNULLRenderer::EF_SetLightHole(Vec3d vPos, Vec3d vNormal, int idTex, float fScale, bool bAdditive)
{
  return false;
}

STexPic *CNULLRenderer::EF_MakePhongTexture(int Exp)
{
  return NULL;
}

void STexPic::Preload (int Flags)
{
}
