#ifndef D3DCUBEMAPS_H
#define D3DCUBEMAPS_H

struct SSingleLight
{
	typedef byte Type;
	int components;
	int format;
	SSingleLight(float _power) :
	components(3), format(D3DFMT_X8R8G8B8), power(_power) {}
	
	void operator() (const Vec3d& v, Type * t)
	{
		float z = v[2] > 0 ? v[2] : 0;
		z = cry_powf(z, power);
		t[0] = (Type)(z * 255.0f);
		t[1] = (Type)(z * 255.0f);
		t[2] = (Type)(z * 255.0f);
	}
	float power;
};

struct SNormalizeVector
{
		typedef byte Type;
    int components;
    int format;
    SNormalizeVector() : components(4), format(D3DFMT_X8R8G8B8) {}
    
    void operator() (const Vec3d & v, Type * t)
    {
      Vec3d v2 = v;
      v2 += Vec3d(1.0f,1.0f,1.0f);
      t[2] = (Type)(v2[0] * 127.0f);
      t[1] = (Type)(v2[1] * 127.0f);
      t[0] = (Type)(v2[2] * 127.0f);
    }
};

// make a cube map from a functor
template <class FunctionOfDirection> void MakeCubeMap(FunctionOfDirection & f, LPDIRECT3DCUBETEXTURE9 pCubeTexture, int size, bool bMips)
{
  typedef typename FunctionOfDirection::Type Type;
  int components = f.components;
  int format = f.format;
  Type * ip;

  int dwMipmaps = pCubeTexture->GetLevelCount();

  int dwLevel = 0;

  while (true)
  {
    float offset = .5;
    float delta = 1;
    float halfsize = size/2.f;
    Vec3d v;

		D3DLOCKED_RECT Locked;
    D3DXVECTOR3 Normal;
    D3DSURFACE_DESC ddsdDesc;
    
    // positive x image 
    {
      pCubeTexture->GetLevelDesc(dwLevel, &ddsdDesc);      
      pCubeTexture->LockRect((D3DCUBEMAP_FACES)0, dwLevel, &Locked, NULL, 0);
			Type* pBits = (Type*)(Locked.pBits);      
      ip = pBits;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[2] = -(i*delta + offset - halfsize);
          v[1] = -(j*delta + offset - halfsize);
          v[0] = halfsize;
          v.Normalize();
          f(v, ip);
          ip += components;
        }
      }
      pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)0, dwLevel);
    }
    // negative x image 
    {
      pCubeTexture->GetLevelDesc(dwLevel, &ddsdDesc);      
      pCubeTexture->LockRect((D3DCUBEMAP_FACES)1, dwLevel, &Locked, NULL, 0);
      Type* pBits = (Type*)(Locked.pBits);
      ip = pBits;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[2] = (i*delta + offset - halfsize);
          v[1] = -(j*delta + offset - halfsize);
          v[0] = -halfsize;
          v.Normalize();
          f(v, ip);
          ip += components;
        }
      }
      pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)1, dwLevel);
    }

    // positive y image 
    {
      pCubeTexture->GetLevelDesc(dwLevel, &ddsdDesc);      
      pCubeTexture->LockRect((D3DCUBEMAP_FACES)2, dwLevel, &Locked, NULL, 0);
      Type* pBits = (Type*)(Locked.pBits);
      ip = pBits;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[0] = (i*delta + offset - halfsize);
          v[2] = (j*delta + offset - halfsize);
          v[1] = halfsize;
          v.Normalize();
          f(v, ip);
          ip += components;
        }
      }
      pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)2, dwLevel);
    }
    // negative y image 
    {
      pCubeTexture->GetLevelDesc(dwLevel, &ddsdDesc);      
      pCubeTexture->LockRect((D3DCUBEMAP_FACES)3, dwLevel, &Locked, NULL, 0);
      Type* pBits = (Type*)(Locked.pBits);
      ip = pBits;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[0] = (i*delta + offset - halfsize);
          v[2] = -(j*delta + offset - halfsize);
          v[1] = -halfsize;
          v.Normalize();
          f(v, ip);
          ip += components;
        }
      }
      pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)3, dwLevel);
    }

    // positive z image 
    {
      pCubeTexture->GetLevelDesc(dwLevel, &ddsdDesc);      
      pCubeTexture->LockRect((D3DCUBEMAP_FACES)4, dwLevel, &Locked, NULL, 0);
      Type* pBits = (Type*)(Locked.pBits);
      ip = pBits;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[0] = (i*delta + offset - halfsize);
          v[1] = -(j*delta + offset - halfsize);
          v[2] = halfsize;
          v.Normalize();
          f(v, ip);
          ip += components;
        }
      }
      pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)4, dwLevel);
    }
    // negative z image 
    {
      pCubeTexture->GetLevelDesc(dwLevel, &ddsdDesc);      
      pCubeTexture->LockRect((D3DCUBEMAP_FACES)5, dwLevel, &Locked, NULL, 0);
      Type* pBits = (Type*)(Locked.pBits);
      ip = pBits;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[0] = -(i*delta + offset - halfsize);
          v[1] = -(j*delta + offset - halfsize);
          v[2] = -halfsize;
          v.Normalize();
          f(v, ip);
          ip += components;
        }
      }
      pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)5, dwLevel);
    }
    size >>= 1;
    if (!bMips || !size)
      break;
    dwLevel++;
    if (dwLevel >= dwMipmaps)
      break;
  }
}

#endif
