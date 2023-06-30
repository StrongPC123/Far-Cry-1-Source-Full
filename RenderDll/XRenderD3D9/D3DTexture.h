#ifndef __D3DTEXTURE_H
#define __D3DTEXTURE_H

class D3DDataFormat
{
public:
  virtual void GetData(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, DWORD& dwValue) { assert(0); };
  virtual void SetData(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, DWORD dwValue) { assert(0); };

  virtual void GetColors(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, float& fRed, float& fGreen, float& fBlue, float& fAlpha) { assert(0); };
  virtual void SetColors(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, float fRed, float fGreen, float fBlue, float fAlpha)  { assert(0); };

  virtual void GetLuminance(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, float& Luminance) { assert(0); };
  virtual void SetLuminance(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, float Luminance) { assert(0); };
  
  virtual void GetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, D3DXVECTOR3& inVector) { assert(0); };
  virtual void SetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, const D3DXVECTOR3& inVector) { assert(0); };
};

class D3DDataFormat_32Bit : public D3DDataFormat
{
public:
  virtual void GetData(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, DWORD& dwValue)
  {
    dwValue = (*(DWORD*) (((BYTE*)LockData.pBits) + (j * LockData.Pitch) + (i * sizeof(DWORD))) );
  }

  virtual void SetData(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, DWORD dwValue)
  {
    *(DWORD*)(((BYTE*)LockData.pBits) + (j * LockData.Pitch) + (i * sizeof(DWORD))) = dwValue;
  }
};

class D3DDataFormat_16Bit : public D3DDataFormat
{
public:
  virtual void GetData(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, DWORD& dwValue)
  {
    dwValue = ((DWORD)*(WORD*)(((BYTE*)LockData.pBits) + (j * LockData.Pitch) + (i * sizeof(WORD))) );
  }

  virtual void SetData(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, DWORD dwValue)
  {
    *(WORD*)(((BYTE*)LockData.pBits) + (j * LockData.Pitch) + (i * sizeof(WORD))) = (WORD)dwValue;
  }
};

class D3DDataFormat_8Bit : public D3DDataFormat
{
public:
  virtual void GetData(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, DWORD& dwValue)
  {
    dwValue = ((DWORD)*(BYTE*)(((BYTE*)LockData.pBits) + (j * LockData.Pitch) + (i * sizeof(BYTE))) );
  }

  virtual void SetData(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, DWORD dwValue)
  {
    *(BYTE*)(((BYTE*)LockData.pBits) + (j * LockData.Pitch) + (i * sizeof(WORD))) = (BYTE)dwValue;
  }
};

class D3DDataFormat_A8R8G8B8 : public D3DDataFormat_32Bit
{ 
public:
  virtual void GetColors(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, float& fRed, float& fGreen, float& fBlue, float& fAlpha)
  {
    DWORD dwValue;
    GetData(LockData, i, j, dwValue);
    fBlue = ((float)(dwValue & 0xFF))/ 255.0f;
    fGreen = ((float)((dwValue >> 8) & 0xFF)) / 255.0f;
    fRed = ((float)((dwValue >> 16) & 0xFF)) / 255.0f;
    fAlpha = ((float)((dwValue >> 24) & 0xFF)) / 255.0f;
  }
  
  virtual void GetLuminance(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, float& Luminance)
  {
    float fRed, fGreen, fBlue, fAlpha;
    GetColors(LockData, i, j, fRed, fGreen, fBlue, fAlpha);
    Luminance = ((fRed * 0.3f) + (fGreen * 0.59f) + (fBlue * 0.11f));
  }

  virtual void SetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, const D3DXVECTOR3& inVector)
  {
    D3DXVECTOR3 vecScaled = (inVector + D3DXVECTOR3(1.0f, 1.0f, 1.0f)) * 127.5f;
    BYTE red   = (BYTE)vecScaled.x;
    BYTE green = (BYTE)vecScaled.y;
    BYTE blue  = (BYTE)vecScaled.z;
    BYTE alpha = 0xFF;
    DWORD dwData = (DWORD)( ( (DWORD)alpha << 24 ) | ( (DWORD)red << 16 ) | ( (DWORD)green << 8 ) | ( (DWORD)blue << 0) );
    SetData(LockData, i, j, dwData);
  }

  virtual void GetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, D3DXVECTOR3& outVector)
  {
    DWORD dwData;
    GetData(LockData, i, j, dwData);

    outVector.x = (float)((dwData >> 16) & 0xFF);
    outVector.y = (float)((dwData >> 8) & 0xFF);
    outVector.z = (float)((dwData) & 0xFF);
    outVector /= 127.5f;
    outVector -= D3DXVECTOR3(1.0f, 1.0f, 1.0f);
  }

};

class D3DDataFormat_X8R8G8B8 : public D3DDataFormat_32Bit
{
public:
  virtual void GetColors(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, float& fRed, float& fGreen, float& fBlue, float& fAlpha)
  {
    DWORD dwValue;
    GetData(LockData, i, j, dwValue);
    fBlue = ((float)(dwValue & 0xFF))/ 255.0f;
    fGreen = ((float)((dwValue >> 8) & 0xFF)) / 255.0f;
    fRed = ((float)((dwValue >> 16) & 0xFF)) / 255.0f;
    fAlpha = 1.0f;
  }
  
  virtual void GetLuminance(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, float& Luminance)
  {
    float fRed, fGreen, fBlue, fAlpha;
    GetColors(LockData, i, j, fRed, fGreen, fBlue, fAlpha);
    Luminance = ((fRed * 0.3f) + (fGreen * 0.59f) + (fBlue * 0.11f));
  }

  virtual void SetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, const D3DXVECTOR3& inVector)
  {
    D3DXVECTOR3 vecScaled = (inVector + D3DXVECTOR3(1.0f, 1.0f, 1.0f)) * 127.5f;
    BYTE red   = (BYTE)vecScaled.x;
    BYTE green = (BYTE)vecScaled.y;
    BYTE blue  = (BYTE)vecScaled.z;
    BYTE alpha = 0xFF;
    DWORD dwData = (DWORD)( ( (DWORD)alpha << 24 ) | ( (DWORD)red << 16 ) | ( (DWORD)green << 8 ) | ( (DWORD)blue << 0) );
    SetData(LockData, i, j, dwData);
  }

  virtual void GetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, D3DXVECTOR3& outVector)
  {
    DWORD dwData;
    GetData(LockData, i, j, dwData);

    outVector.x = (float)((dwData >> 16) & 0xFF);
    outVector.y = (float)((dwData >> 8) & 0xFF);
    outVector.z = (float)((dwData) & 0xFF);
    outVector /= 127.5f;
    outVector -= D3DXVECTOR3(1.0f, 1.0f, 1.0f);
  }

};

class D3DDataFormat_Q8W8V8U8 : public D3DDataFormat_32Bit
{
  virtual void SetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, const D3DXVECTOR3& inVector)
  {
    D3DXVECTOR3 vecScaled = inVector * 127.5f;
    signed char red   = (signed char)vecScaled.x;
    signed char green = (signed char)vecScaled.y;
    signed char blue  = (signed char)vecScaled.z;
    signed char alpha = 0;
    DWORD dwData = (DWORD)( ( (DWORD)(unsigned char)alpha << 24 ) | ( (DWORD)(unsigned char)blue << 16 ) | ( (DWORD)(unsigned char)green << 8 ) | ( (DWORD)(unsigned char)red << 0) );
    SetData(LockData, i, j, dwData);
  }

  virtual void GetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, D3DXVECTOR3& outVector)
  {
    DWORD dwData;
    GetData(LockData, i, j, dwData);

    outVector.x = (float)(signed char)((dwData) & 0xFF);
    outVector.y = (float)(signed char)((dwData >> 8) & 0xFF);
    outVector.z = (float)(signed char)((dwData >> 16) & 0xFF);
    outVector /= 127.5f;
  }
};

class D3DDataFormat_X8L8V8U8 : public D3DDataFormat_32Bit
{
  virtual void SetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, const D3DXVECTOR3& inVector)
  {
    D3DXVECTOR3 vecScaled = inVector * 127.5f;
    signed char red   = (signed char)vecScaled.x;
    signed char green = (signed char)vecScaled.y;
    signed char blue  = (signed char)vecScaled.z;
    signed char alpha = 0;
    DWORD dwData = (DWORD)( ( (DWORD)(unsigned char)alpha << 24 ) | ( (DWORD)(unsigned char)blue << 16 ) | ( (DWORD)(unsigned char)green << 8 ) | ( (DWORD)(unsigned char)red << 0) );
    SetData(LockData, i, j, dwData);
  }

  virtual void GetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, D3DXVECTOR3& outVector)
  {
    DWORD dwData;
    GetData(LockData, i, j, dwData);

    outVector.x = (float)(signed char)((dwData) & 0xFF);
    outVector.y = (float)(signed char)((dwData >> 8) & 0xFF);
    outVector.z = (float)(signed char)((dwData >> 16) & 0xFF);
    outVector /= 127.5f;
  }
};

class D3DDataFormat_D3DHS : public D3DDataFormat_32Bit
{
  virtual void SetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, const D3DXVECTOR3& inVector)
  {
    D3DXVECTOR3 vecScaled = inVector * 32767.0f;
      signed short h = (signed short)vecScaled.x;
    signed short l = (signed short)vecScaled.y;
    DWORD dwData = ( ( (DWORD)(unsigned short)h << 16 ) | ( (DWORD)(unsigned short)l << 0 ) );
    SetData(LockData, i, j, dwData);
  }

  virtual void GetVector(const D3DLOCKED_RECT& LockData, DWORD i, DWORD j, D3DXVECTOR3& outVector)
  {
    DWORD dwData;
    GetData(LockData, i, j, dwData);

    outVector.x = (float)(unsigned short)((dwData >> 16) & 0xFFFF);
    outVector.y = (float)(unsigned short)((dwData) & 0xFFFF);
    outVector.z = 1.0f;
    outVector /= 32767.0f;
  }

};
  
class D3D2DTextureLocker
{
public:
  enum
  {
    MAX_LOCK_LEVELS = 12
  };

  D3D2DTextureLocker(LPDIRECT3DTEXTURE9 pTexture)
    : m_pTexture(pTexture),
    m_pDataFormat(NULL)
  {
    m_pTexture->AddRef();
    for (DWORD i=0; i < MAX_LOCK_LEVELS; i++)
    {
      m_bLocked[i] = false;
    }
    m_dwLevels = m_pTexture->GetLevelCount();

    D3DSURFACE_DESC LevelDesc;
    m_pTexture->GetLevelDesc(0, &LevelDesc);

    switch(LevelDesc.Format)
    {
      case D3DFMT_UNKNOWN:
      case D3DFMT_VERTEXDATA:
      case D3DFMT_INDEX16:
#ifndef _XBOX
      case D3DFMT_INDEX32:
#endif
      case D3DFMT_DXT1:
      case D3DFMT_DXT2:
#ifndef _XBOX
      case D3DFMT_DXT3:
#endif
      case D3DFMT_DXT4:
#ifndef _XBOX
      case D3DFMT_DXT5:
      default:
        assert(!"Don't understand surface format");
        break;

      case D3DFMT_R8G8B8:
        assert(!"Don't handle 24 bit surfaces");
        break;

      case D3DFMT_X8L8V8U8:    
        m_pDataFormat = new D3DDataFormat_X8L8V8U8;
        break;
#endif
      case D3DFMT_A8R8G8B8:
        m_pDataFormat = new D3DDataFormat_A8R8G8B8;
        break;

      case D3DFMT_X8R8G8B8:
        m_pDataFormat = new D3DDataFormat_X8R8G8B8;
        break;

      case D3DFMT_Q8W8V8U8:    
        m_pDataFormat = new D3DDataFormat_Q8W8V8U8;
        break;

      case MAKEFOURCC('N', 'V', 'H', 'S'):
        m_pDataFormat = new D3DDataFormat_D3DHS;
        break;
    }

    assert(m_pDataFormat);
  }

  virtual ~D3D2DTextureLocker()
  {
    for (DWORD i = 0; i < MAX_LOCK_LEVELS; i++)
    {
      if (m_bLocked[i])
      {
        Unlock(i);
      }
    }
    SAFE_RELEASE(m_pTexture);
    SAFE_DELETE(m_pDataFormat);
  }


  bool Lock(DWORD dwLevel)
  {
    HRESULT hr;
    assert(dwLevel < m_dwLevels);
    assert(!m_bLocked[dwLevel]);

    m_bLocked[dwLevel] = true;
    hr = m_pTexture->LockRect(dwLevel, &m_LockData[dwLevel], NULL, 0);
    m_pTexture->GetLevelDesc(dwLevel, &m_LevelDesc[dwLevel]);

    if (FAILED(hr))
      return false;

    return true;
  }

  bool Unlock(DWORD dwLevel)
  {
    HRESULT hr;

    assert(dwLevel < m_dwLevels);
    assert(m_bLocked[dwLevel]);

    m_bLocked[dwLevel] = false;
    hr = m_pTexture->UnlockRect(dwLevel);
    if (FAILED(hr))
      return false;

    return true;
  }

  void WrapAddress(DWORD dwLevel, DWORD& i, DWORD& j)
  {
    if (i >= 0)
    {
      i = (i % m_LevelDesc[dwLevel].Width);
    }
    else
    {
      i = (m_LevelDesc[dwLevel].Width - 1) + (i % m_LevelDesc[dwLevel].Width);
    }
    
    if (j >= 0)
    {
      j = (j % m_LevelDesc[dwLevel].Height);
    }
    else
    {
      j = (m_LevelDesc[dwLevel].Height - 1) + (j % m_LevelDesc[dwLevel].Height);
    }
    assert(i >= 0);
    assert(j >= 0);
    assert(i < m_LevelDesc[dwLevel].Width);
    assert(j < m_LevelDesc[dwLevel].Height);
  }

  void GetMapData(DWORD dwLevel, DWORD i, DWORD j, DWORD& dwValue)
  {
    assert(m_bLocked[dwLevel]);
    WrapAddress(dwLevel, i, j);
    m_pDataFormat->GetData(m_LockData[dwLevel], i, j, dwValue);
  }

  void SetMapData(DWORD dwLevel, DWORD i, DWORD j, DWORD dwValue)
  {
    assert(m_bLocked[dwLevel]);
    WrapAddress(dwLevel, i, j);
    m_pDataFormat->SetData(m_LockData[dwLevel], i, j, dwValue);
  }


  void GetMapColors(DWORD dwLevel, DWORD i, DWORD j, float& fRed, float& fGreen, float& fBlue, float& fAlpha)
  {
    assert(m_bLocked[dwLevel]);
    WrapAddress(dwLevel, i, j);
    m_pDataFormat->GetColors(m_LockData[dwLevel], i, j, fRed, fGreen, fBlue, fAlpha);
  }

  void GetMapLuminance(DWORD dwLevel, DWORD i, DWORD j, float& Luminance)
  {
    assert(m_bLocked[dwLevel]);
    WrapAddress(dwLevel, i, j);
    m_pDataFormat->GetLuminance(m_LockData[dwLevel], i, j, Luminance);
  }
    
  void SetMapVector(DWORD dwLevel, DWORD i, DWORD j, const D3DXVECTOR3& inVector)
  {
    assert(m_bLocked[dwLevel]);
    WrapAddress(dwLevel, i, j);
    m_pDataFormat->SetVector(m_LockData[dwLevel], i, j, inVector);
  }

  void GetMapVector(DWORD dwLevel, DWORD i, DWORD j, D3DXVECTOR3& inVector)
  {
    assert(m_bLocked[dwLevel]);
    WrapAddress(dwLevel, i, j);
    m_pDataFormat->GetVector(m_LockData[dwLevel], i, j, inVector);
  }

private:
  D3DDataFormat* m_pDataFormat;
  DWORD m_dwLevels;
  bool m_bLocked[MAX_LOCK_LEVELS];
  D3DLOCKED_RECT m_LockData[MAX_LOCK_LEVELS];
  D3DSURFACE_DESC m_LevelDesc[MAX_LOCK_LEVELS];

  
  LPDIRECT3DTEXTURE9 m_pTexture;

};

class D3D2DSurfaceLocker
{
public:

  D3D2DSurfaceLocker(LPDIRECT3DSURFACE9 pSurface)
    : m_pSurface(pSurface),
    m_pDataFormat(NULL)
  {
    m_pSurface->AddRef();
    m_bLocked = false;

    m_pSurface->GetDesc(&m_LevelDesc);

    switch(m_LevelDesc.Format)
    {
      case D3DFMT_UNKNOWN:
      case D3DFMT_VERTEXDATA:
      case D3DFMT_INDEX16:
#ifndef _XBOX
      case D3DFMT_INDEX32:
#endif
#ifndef _XBOX		  
      case D3DFMT_DXT1:
      case D3DFMT_DXT2:
      case D3DFMT_DXT3:
      case D3DFMT_DXT4:
      case D3DFMT_DXT5:
      default:
        assert(!"Don't understand surface format");
        break;

      case D3DFMT_R8G8B8:
        assert(!"Don't handle 24 bit surfaces");
        break;

      case D3DFMT_X8L8V8U8:    
        m_pDataFormat = new D3DDataFormat_X8L8V8U8;
        break;

#endif
      case D3DFMT_A8R8G8B8:
        m_pDataFormat = new D3DDataFormat_A8R8G8B8;
        break;

      case D3DFMT_X8R8G8B8:
        m_pDataFormat = new D3DDataFormat_X8R8G8B8;
        break;

      case D3DFMT_Q8W8V8U8:    
        m_pDataFormat = new D3DDataFormat_Q8W8V8U8;
        break;

      case MAKEFOURCC('N', 'V', 'H', 'S'):
        m_pDataFormat = new D3DDataFormat_D3DHS;
        break;
    }

    assert(m_pDataFormat);
  }

  virtual ~D3D2DSurfaceLocker()
  {
    if (m_bLocked)
    {
      Unlock();
    }
    SAFE_RELEASE(m_pSurface);
    SAFE_DELETE(m_pDataFormat);
  }


  bool Lock()
  {
    HRESULT hr;
    assert(!m_bLocked);

    m_bLocked = true;
    hr = m_pSurface->LockRect(&m_LockData, NULL, 0);
    if (FAILED(hr))
      return false;

    return true;
  }

  bool Unlock()
  {
    HRESULT hr;
    assert(m_bLocked);

    m_bLocked = false;
    hr = m_pSurface->UnlockRect();
    if (FAILED(hr))
      return false;

    return true;
  }

  void WrapAddress(DWORD& i, DWORD& j)
  {
    if (i >= 0)
    {
      i = (i % m_LevelDesc.Width);
    }
    else
    {
      i = (m_LevelDesc.Width - 1) + (i % m_LevelDesc.Width);
    }
    
    if (j >= 0)
    {
      j = (j % m_LevelDesc.Height);
    }
    else
    {
      j = (m_LevelDesc.Height - 1) + (j % m_LevelDesc.Height);
    }
    assert(i >= 0);
    assert(j >= 0);
    assert(i < m_LevelDesc.Width);
    assert(j < m_LevelDesc.Height);
  }

  void GetMapData(DWORD i, DWORD j, DWORD& dwValue)
  {
    assert(m_bLocked);
    WrapAddress(i, j);
    m_pDataFormat->GetData(m_LockData, i, j, dwValue);
  }

  void SetMapData(DWORD i, DWORD j, DWORD dwValue)
  {
    assert(m_bLocked);
    WrapAddress(i, j);
    m_pDataFormat->SetData(m_LockData, i, j, dwValue);
  }


  void GetMapColors(DWORD i, DWORD j, float& fRed, float& fGreen, float& fBlue, float& fAlpha)
  {
    assert(m_bLocked);
    WrapAddress(i, j);
    m_pDataFormat->GetColors(m_LockData, i, j, fRed, fGreen, fBlue, fAlpha);
  }

  void GetMapLuminance(DWORD i, DWORD j, float& Luminance)
  {
    assert(m_bLocked);
    WrapAddress(i, j);
    m_pDataFormat->GetLuminance(m_LockData, i, j, Luminance);
  }
    
  void SetMapVector(DWORD i, DWORD j, const D3DXVECTOR3& inVector)
  {
    assert(m_bLocked);
    WrapAddress(i, j);
    m_pDataFormat->SetVector(m_LockData, i, j, inVector);
  }

  void GetMapVector(DWORD i, DWORD j, D3DXVECTOR3& inVector)
  {
    assert(m_bLocked);
    WrapAddress(i, j);
    m_pDataFormat->GetVector(m_LockData, i, j, inVector);
  }

private:
  D3DDataFormat* m_pDataFormat;
  bool m_bLocked;
  D3DSURFACE_DESC m_LevelDesc;
  D3DLOCKED_RECT m_LockData;
  
  LPDIRECT3DSURFACE9 m_pSurface;

};

class CD3DTexture
{
public:

  // Gets height from luminance value
  static LPDIRECT3DTEXTURE9 CreateNormalMap(LPDIRECT3DDEVICE9 pD3DDev, LPDIRECT3DTEXTURE9 pSource, STexPic *ti, bool bMips, D3DXVECTOR3 Scale, D3DFORMAT Format = D3DFMT_Q8W8V8U8, D3DPOOL Pool = D3DPOOL_MANAGED)
  {
    LPDIRECT3DTEXTURE9 pNormalMap = NULL;
    D3DSURFACE_DESC ddsdDescDest;
    D3DSURFACE_DESC ddsdDescSource;
    D3DXVECTOR3 Normal;
    HRESULT hr;
    DWORD i, j;
    LPDIRECT3DTEXTURE9 pNewTex = NULL;

    assert(pSource && pSource->GetType() == D3DRTYPE_TEXTURE);
    if (!pSource)
      return NULL;

    (pSource)->GetLevelDesc(0, &ddsdDescSource);
    
    // Handle conversion from compressed to specified format
    switch(ddsdDescSource.Format)
    {
      default:
        break;
      case D3DFMT_DXT1:
      case D3DFMT_DXT3:
      case D3DFMT_DXT5:
      {
        hr = D3DXCreateTexture(pD3DDev, ddsdDescSource.Width, ddsdDescSource.Height, bMips ? D3DX_DEFAULT : 1, 0, Format, D3DPOOL_SYSTEMMEM, &pNewTex);
        if (FAILED(hr))
          return NULL;

        // Copy the levels to RGB textures
        for (i = 0; i < 1; i++)
        {
          LPDIRECT3DSURFACE9 pDestSurf;
          LPDIRECT3DSURFACE9 pSourceSurf;
          pNewTex->GetSurfaceLevel(i, &pDestSurf);
          pSource->GetSurfaceLevel(i, &pSourceSurf);
          D3DXLoadSurfaceFromSurface(pDestSurf, NULL, NULL, pSourceSurf, NULL, NULL,  D3DX_FILTER_NONE, 0);
          SAFE_RELEASE(pDestSurf);
          SAFE_RELEASE(pSourceSurf);
        }
              
        pSource = pNewTex;
      }
      break;
    }

    hr = D3DXCreateTexture(pD3DDev, ddsdDescSource.Width, ddsdDescSource.Height, bMips ? D3DX_DEFAULT : 1, 0, Format, Pool, &pNormalMap );

    if (FAILED(hr))
    {
      SAFE_RELEASE(pNewTex);
      return NULL;
    }

    D3D2DTextureLocker SourceLocker(pSource);
    D3D2DTextureLocker DestLocker(pNormalMap);

    for (DWORD Level = 0; Level < 1; Level++)
    {
      pNormalMap->GetLevelDesc(Level, &ddsdDescDest);

      DWORD dwWidth = ddsdDescDest.Width;
      DWORD dwHeight = ddsdDescDest.Height;

      SourceLocker.Lock(Level);
      DestLocker.Lock(Level);

      for(i=0; i < dwWidth; i++)
      {
        for(j = 0; j < dwHeight; j++)
        {
          float fRight, fLeft, fUp, fDown;

          SourceLocker.GetMapLuminance(Level, i + 1, j, fRight);
          SourceLocker.GetMapLuminance(Level, i - 1, j, fLeft);
          SourceLocker.GetMapLuminance(Level, i, j - 1, fUp);
          SourceLocker.GetMapLuminance(Level, i, j + 1, fDown);

          D3DXVECTOR3 dfdi(2.f, 0.f, fRight - fLeft);
          D3DXVECTOR3 dfdj(0.f, 2.f, fDown - fUp);

          D3DXVec3Cross(&Normal, &dfdi, &dfdj);
          Normal[0] *= Scale[0];
          Normal[1] *= Scale[1];
          Normal[2] *= Scale[2];
          D3DXVec3Normalize(&Normal, &Normal);
          if (ti->m_eTT == eTT_DSDTBump)
            Normal[2] = 0.5f;

          DestLocker.SetMapVector(Level, i, j, Normal);
        }
      }

      SourceLocker.Unlock(Level);
      DestLocker.Unlock(Level);
    }

    SAFE_RELEASE(pNewTex);

    return pNormalMap;
  }

  static void FilterNormalMap(LPDIRECT3DDEVICE9 pD3DDev, LPDIRECT3DTEXTURE9 pNormalMap)
  {
    D3DSURFACE_DESC ddsdDescSource;
    D3DXVECTOR3 Normal;
    DWORD i, j;

    D3D2DTextureLocker SurfaceLocker(pNormalMap);

    for (DWORD Level = 0; Level < pNormalMap->GetLevelCount() - 1; Level++)
    {
      SurfaceLocker.Lock(Level);
      SurfaceLocker.Lock(Level + 1);

      pNormalMap->GetLevelDesc(Level, &ddsdDescSource);
      for(i=0; i < ddsdDescSource.Width; i+=2)
      {
        for(j = 0; j < ddsdDescSource.Height; j+=2)
        {
          D3DXVECTOR3 Vectors[4];
          SurfaceLocker.GetMapVector(Level, i, j, Vectors[0]);
          SurfaceLocker.GetMapVector(Level, i+1, j, Vectors[1]);
          SurfaceLocker.GetMapVector(Level, i+1, j+1, Vectors[2]);
          SurfaceLocker.GetMapVector(Level, i+1, j+1, Vectors[3]);

          D3DXVECTOR3 Normal = Vectors[0] + Vectors[1] + Vectors[2] + Vectors[3];
          D3DXVec3Normalize(&Normal, &Normal);

          SurfaceLocker.SetMapVector(Level + 1, i / 2, j / 2, Normal);
        }
      }

      SurfaceLocker.Unlock(Level);
      SurfaceLocker.Unlock(Level + 1);
    }
  }

  static LPDIRECT3DCUBETEXTURE9 CreateNormalizationCubeMap(LPDIRECT3DDEVICE9 pD3DDev, DWORD dwWidth, DWORD dwMipmaps = 0, D3DPOOL Pool = D3DPOOL_MANAGED)
  {
    HRESULT hr;
    LPDIRECT3DCUBETEXTURE9 pCubeTexture;

    hr = D3DXCreateCubeTexture(pD3DDev, dwWidth, dwMipmaps, 0, D3DFMT_X8R8G8B8, Pool, &pCubeTexture);
    if(FAILED(hr))
    {
      return NULL;
    }

    if (dwMipmaps == 0)
      dwMipmaps = pCubeTexture->GetLevelCount();

    for (DWORD dwLevel = 0; dwLevel < dwMipmaps; dwLevel++)
    {
      for (int i = 0; i < 6; i++)
      {
        D3DLOCKED_RECT Locked;
        D3DXVECTOR3 Normal;
        float w,h;
        D3DSURFACE_DESC ddsdDesc;
        
        pCubeTexture->GetLevelDesc(dwLevel, &ddsdDesc);

        pCubeTexture->LockRect((D3DCUBEMAP_FACES)i, dwLevel, &Locked, NULL, 0);

        for (unsigned int y = 0; y < ddsdDesc.Height; y++)
        {
          h = (float)y / ((float)(ddsdDesc.Height - 1));
          h *= 2.0f;
          h -= 1.0f;

          for (unsigned int x = 0; x < ddsdDesc.Width; x++)
          {
            w = (float)x / ((float)(ddsdDesc.Width - 1));
            w *= 2.0f;
            w -= 1.0f;

            DWORD* pBits = (DWORD*)((BYTE*)Locked.pBits + (y * Locked.Pitch));
            pBits += x;

            switch((D3DCUBEMAP_FACES)i)
            {
              case D3DCUBEMAP_FACE_POSITIVE_X:
                Normal = D3DXVECTOR3(1.0f, -h, -w);
                break;
              case D3DCUBEMAP_FACE_NEGATIVE_X:
                Normal = D3DXVECTOR3(-1.0f, -h, w);
                break;
              case D3DCUBEMAP_FACE_POSITIVE_Y:
                Normal = D3DXVECTOR3(w, 1.0f, h);
                break;
              case D3DCUBEMAP_FACE_NEGATIVE_Y:
                Normal = D3DXVECTOR3(w, -1.0f, -h);
                break;
              case D3DCUBEMAP_FACE_POSITIVE_Z:
                Normal = D3DXVECTOR3(w, -h, 1.0f);
                break;
              case D3DCUBEMAP_FACE_NEGATIVE_Z:
                Normal = D3DXVECTOR3(-w, -h, -1.0f);
                break;
              default:
                assert(0);
                break;
            }

            D3DXVec3Normalize(&Normal, &Normal);

            // Scale to be a color from 0 to 255 (127 is 0)
            Normal += D3DXVECTOR3(1.0f, 1.0f, 1.0f);
            Normal *= 127.0f;

            // Store the color
            *pBits = (DWORD)(((DWORD)Normal.x << 16) | ((DWORD)Normal.y << 8) | ((DWORD)Normal.z << 0));

          }
        }
        pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)i, 0);
      }
    }

    return pCubeTexture;
  }
};

//vlad123

#endif __D3DTEXTURE_H
