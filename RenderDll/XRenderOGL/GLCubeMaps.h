#ifndef GLCUBEMAPS_H
#define GLCUBEMAPS_H

struct SSingleLight
{
	typedef GLfloat Type;
	int components;
	GLenum type;
	GLenum format;
	SSingleLight(float _power) :
	components(3), format(GL_RGB), type(GL_FLOAT), power(_power) {}
	
	void operator() (const Vec3d& v, Type * t)
	{
		float z = v[2] > 0 ? v[2] : 0;
		z = (float)cry_powf(z, power);
		t[0] = z;
		t[1] = z;
		t[2] = z;
	}
	float power;
};

struct SNormalizeVector
{
		typedef GLfloat Type;
    int components;
    GLenum type;
    GLenum format;
    SNormalizeVector() : components(3), format(GL_RGB), type(GL_FLOAT) {}
    
    void operator() (const Vec3d & v, Type * t)
    {
      Vec3d v2 = v;
      v2 *= .5;
      t[0] = v2[0] + .5f;
      t[1] = v2[1] + .5f;
      t[2] = v2[2] + .5f;
    }
};

// make a cube map from a functor
template <class FunctionOfDirection> void MakeCubeMap(FunctionOfDirection & f, GLenum internal_format, int size, bool bMips, STexPic *tp)
{
  typedef typename FunctionOfDirection::Type Type;
  int components = f.components;
  GLenum type = f.type;
  GLenum format = f.format;
  Type * ip;

  int level = 0;

  tp->Unlink();
  tp->Link(&STexPic::m_Root);
  gRenDev->m_TexMan->AddToHash(tp->m_Bind, tp);
  tp->m_Width = size;
  tp->m_Height = size;

  tp->m_nMips = 0;
  tp->m_Size = 0;
  while (true)
  {
    float offset = .5;
    float delta = 1;
    float halfsize = size/2.f;
    Vec3d v;
    Type * image  = new Type[size*size*components];
    byte * pixels  = new byte[size*size*components];
    tp->m_Size += size*size*components;
    // positive x image 
    {
      ip = image;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[2] = -(i*delta + offset - halfsize);
          v[1] = -(j*delta + offset - halfsize);
          v[0] = halfsize;
          v.Normalize();
          f(v, ip);
          pixels[3*(j*size+i) + 0] = (byte)(255.0f*ip[0]);
          pixels[3*(j*size+i) + 1] = (byte)(255.0f*ip[1]);
          pixels[3*(j*size+i) + 2] = (byte)(255.0f*ip[2]);
          ip += components;
        }
      }

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT, level, internal_format, size, size, 0, format, GL_UNSIGNED_BYTE, pixels);
    }
    // negative x image 
    {
      ip = image;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[2] = (i*delta + offset - halfsize);
          v[1] = -(j*delta + offset - halfsize);
          v[0] = -halfsize;
          v.Normalize();
          f(v, ip);
          pixels[3*(j*size+i) + 0] = (byte)(255.0f*ip[0]);
          pixels[3*(j*size+i) + 1] = (byte)(255.0f*ip[1]);
          pixels[3*(j*size+i) + 2] = (byte)(255.0f*ip[2]);
          ip += components;
        }
      }
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT, level, internal_format, size, size, 0, format, GL_UNSIGNED_BYTE, pixels);
    }

    // positive y image 
    {
      ip = image;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[0] = (i*delta + offset - halfsize);
          v[2] = (j*delta + offset - halfsize);
          v[1] = halfsize;
          v.Normalize();
          f(v, ip);
          pixels[3*(j*size+i) + 0] = (byte)(255.0f*ip[0]);
          pixels[3*(j*size+i) + 1] = (byte)(255.0f*ip[1]);
          pixels[3*(j*size+i) + 2] = (byte)(255.0f*ip[2]);
          ip += components;
        }
      }
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT, level, internal_format, size, size, 0, format, GL_UNSIGNED_BYTE, pixels);
    }
    // negative y image 
    {
      ip = image;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[0] = (i*delta + offset - halfsize);
          v[2] = -(j*delta + offset - halfsize);
          v[1] = -halfsize;
          v.Normalize();
          f(v, ip);
          pixels[3*(j*size+i) + 0] = (byte)(255.0f*ip[0]);
          pixels[3*(j*size+i) + 1] = (byte)(255.0f*ip[1]);
          pixels[3*(j*size+i) + 2] = (byte)(255.0f*ip[2]);
          ip += components;
        }
      }
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT, level, internal_format, size, size, 0, format, GL_UNSIGNED_BYTE, pixels);
    }

    // positive z image 
    {
      ip = image;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[0] = (i*delta + offset - halfsize);
          v[1] = -(j*delta + offset - halfsize);
          v[2] = halfsize;
          v.Normalize();
          f(v, ip);
          pixels[3*(j*size+i) + 0] = (byte)(255.0f*ip[0]);
          pixels[3*(j*size+i) + 1] = (byte)(255.0f*ip[1]);
          pixels[3*(j*size+i) + 2] = (byte)(255.0f*ip[2]);
          ip += components;
        }
      }
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT, level, internal_format, size, size, 0, format, GL_UNSIGNED_BYTE, pixels);
    }
    // negative z image 
    {
      ip = image;
      for(int j = 0; j < size; j++)
      {
        for(int i=0; i < size; i++)
        {
          v[0] = -(i*delta + offset - halfsize);
          v[1] = -(j*delta + offset - halfsize);
          v[2] = -halfsize;
          v.Normalize();
          f(v, ip);
          pixels[3*(j*size+i) + 0] = (byte)(255.0f*ip[0]);
          pixels[3*(j*size+i) + 1] = (byte)(255.0f*ip[1]);
          pixels[3*(j*size+i) + 2] = (byte)(255.0f*ip[2]);
          ip += components;
        }
      }
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT, level, internal_format, size, size, 0, format, GL_UNSIGNED_BYTE, pixels);
    }
    delete [] image;
    delete [] pixels;
    size >>= 1;
    level++;
    if (!bMips || !size)
      break;
  }
  tp->m_nMips = level;
  tp->m_Size *= 6;
  gRenDev->m_TexMan->m_StatsCurTexMem += tp->m_Size;

  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  if (bMips)
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  else
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  gRenDev->m_TexMan->CheckTexLimits(NULL);
}

#endif
