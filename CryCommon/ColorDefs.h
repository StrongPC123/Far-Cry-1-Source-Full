/*=============================================================================
  ColorDefs.h : Colors declarations.
  Copyright (c) 2001 Crytek Studios. All Rights Reserved.

  Revision history:
    * Created by Khonich Andrey

=============================================================================*/

#ifndef __COLORDEFS_H__
#define __COLORDEFS_H__

//#pragma warning(disable:4018) // signed/unsigned mismatch
//#pragma warning(disable:4244) // int to float loose of data.

//=========================================================

///#include "CryHeaders.h"

_inline float FClamp( float X, float Min, float Max )
{
  return X<Min ? Min : X<Max ? X : Max;
}

class CFColor
{
public:
  float r, g, b, a;

public:
  CFColor() {}
  CFColor(const Vec3 & vVec)
  {
    r = vVec.x;
    g = vVec.y;
    b = vVec.z;
    a = 1.f;
  }
  CFColor(float lr, float lg, float lb, float la = 1.0f)
  {
    r = lr;
    g = lg;
    b = lb;
    a = la;
  }
  CFColor(float *cols)
  {
    r = cols[0];
    g = cols[1];
    b = cols[2];
    a = cols[3];
  }
  CFColor(unsigned int c)
  {
    r = (c & 0xff) / 255.0f;
    g = ((c>>8) & 0xff) / 255.0f;
    b = ((c>>16) & 0xff) / 255.0f;
    a = ((c>>24) & 0xff) / 255.0f;
  }
  CFColor(unsigned char c[4])
  {
    r = c[0] / 255.0f;
    g = c[1] / 255.0f;
    b = c[2] / 255.0f;
    a = c[3] / 255.0f;
  }
/*  CFColor(CryIRGB *c)
  {
    r = c->r / 255.0f;
    g = c->g / 255.0f;
    b = c->b / 255.0f;
    a = 1.0f;
  }*/

  CFColor(float c)
  {
    r = g = b = a = c;
  }

	void	Set(float fR,float fG,float fB,float fA=1.0f)
	{
		r=fR;g=fG;b=fB;a=fA;
	}

  // Binary math operators.
  friend CFColor operator*( float Scale, const CFColor& C )
  {
    return CFColor( C.r * Scale, C.g * Scale, C.b * Scale, C.a * Scale );
  }
  CFColor operator+( const CFColor& C ) const
  {
    return CFColor( r + C.r, g + C.g, b + C.b, a + C.a );
  }
  CFColor operator-( const CFColor& C ) const
  {
    return CFColor( r - C.r, g - C.g, b - C.b, a - C.a );
  }
  CFColor operator*( float Scale ) const
  {
    return CFColor( r * Scale, g * Scale, b * Scale, a * Scale );
  }
  CFColor operator/( float Scale ) const
  {
    float IScale = 1.0f/Scale;
    return CFColor( r * IScale, g * IScale, b * IScale, a * IScale );
  }
  CFColor operator*( const CFColor& C ) const
  {
    return CFColor( r * C.r, g * C.g, b * C.b, a * C.a );
  }

  // Binary comparison operators.
  bool operator==( const CFColor& C ) const
  {
    return r==C.r && g==C.g && b==C.b && a==C.a;
  }
  bool operator!=( const CFColor& C ) const
  {
    return r!=C.r || g!=C.g || b!=C.b || a!=C.a;
  }

  // Assignment operators.
  CFColor operator=( const float c )
  {
    r = g = b = a = c;
    return *this;
  }
  CFColor operator=( const CFColor& C )
  {
    r = C.r;
    g = C.g;
    b = C.b;
    a = C.a;
    return *this;
  }
  CFColor operator+=( const CFColor& C )
  {
    r += C.r; g += C.g; b += C.b; a += C.a;
    return *this;
  }
  CFColor operator-=( const CFColor& C )
  {
    r -= C.r; g -= C.g; b -= C.b; a -= C.a;
    return *this;
  }
  CFColor operator*=( float Scale )
  {
    r *= Scale; g *= Scale; b *= Scale; a *= Scale;
    return *this;
  }
  CFColor operator/=( float V )
  {
    float IV = 1.0f/V;
    r *= IV; g *= IV; b *= IV; a *= IV;
    return *this;
  }
  CFColor operator*=( const CFColor& C )
  {
    r *= C.r; g *= C.g; b *= C.b; a *= C.a;
    return *this;
  }
  CFColor operator/=( const CFColor& C )
  {
    r /= C.r; g /= C.g; b /= C.b; a /= C.a;
    return *this;
  }

  _inline const float& operator[](int i) const { return (&r)[i]; }

  _inline float& operator[](int i) { return (&r)[i]; }

  float* operator * ()                 { return (&r); }

  unsigned int GetTrue()
  {
    union
    {
      struct
      {
        unsigned char R, G, B, A;
      };
      unsigned int D;
    }C;
    C.R = uchar(r * 255.0f); 
    C.G = uchar(g * 255.0f); 
    C.B = uchar(b * 255.0f); 
    C.A = uchar(a * 255.0f);
    return C.D;
  }

  unsigned int GetTrueInv()
  {
    union
    {
      struct
      {
        unsigned char R, G, B, A;
      };
      unsigned int D;
    }C;
    C.R = uchar(255.0f-r*255.0f); 
    C.G = uchar(255.0f-g*255.0f); 
    C.B = uchar(255.0f-b*255.0f); 
    C.A = uchar(255.0f-a*255.0f);
    return C.D;
  }

  unsigned int GetTrueCol()
  {
    union
    {
      struct
      {
        unsigned char R, G, B, A;
      };
      unsigned int D;
    }C;
//    C.R = r * 255.0f; C.G = g * 255.0f; C.B = b * 255.0f; C.A = 255;
    C.R = uchar(r * 255.0f); 
    C.G = uchar(g * 255.0f); 
    C.B = uchar(b * 255.0f); 
    C.A = uchar(255.0f);
    return C.D;
  }

  void ScaleCol (float f)
  {
    r *= f; g *= f; b *= f;
  }

  float NormalizeCol (CFColor& out)
  {
    float max;

    max = r;
    if (g > max)
      max = g;
    if (b > max)
      max = b;

    if (max == 0)
      return 0;

    out = *this / max;

    return max;
  }

  float Normalize (CFColor& out)
  {
    float max;

    max = r;
    if (g > max)
      max = g;
    if (b > max)
      max = b;
    if (a > max)
      max = a;

    if (max == 0)
      return 0;

    out = *this / max;

    return max;
  }

  void Clamp(float Min = 0, float Max = 1.0f)
  {
    r = ::FClamp(r, Min, Max);
    g = ::FClamp(g, Min, Max);
    b = ::FClamp(b, Min, Max);
    a = ::FClamp(a, Min, Max);
  }
  void ClampCol(float Min = 0, float Max = 1.0f)
  {
    r = ::FClamp(r, Min, Max);
    g = ::FClamp(g, Min, Max);
    b = ::FClamp(b, Min, Max);
  }
  void ClampAlpha(float Min = 0, float Max = 1.0f)
  {
    a = ::FClamp(a, Min, Max);
  }
};

#define Col_Aquamarine		CFColor (0.439216f, 0.858824f, 0.576471f)
#define Col_Black		CFColor (0.0f, 0.0f, 0.0f)
#define Col_Blue		CFColor (0.0f, 0.0f, 1.0f)
#define Col_BlueViolet		CFColor (0.623529f, 0.372549f, 0.623529f)
#define Col_Brown		CFColor (0.647059f, 0.164706f, 0.164706f)
#define Col_CadetBlue		CFColor (0.372549f, 0.623529f, 0.623529f)
#define Col_Coral		CFColor (1.0f, 0.498039f, 0.0f)
#define Col_CornflowerBlue	CFColor (0.258824f, 0.258824f, 0.435294f)
#define Col_Cyan		CFColor (0.0f, 1.0f, 1.0f)
#define Col_DarkGreen		CFColor (0.184314f, 0.309804f, 0.184314f)
#define Col_DarkOliveGreen	CFColor (0.309804f, 0.309804f, 0.184314f)
#define Col_DarkOrchild		CFColor (0.6f, 0.196078f, 0.8f)
#define Col_DarkSlateBlue	CFColor (0.419608f, 0.137255f, 0.556863f)
#define Col_DarkSlateGray	CFColor (0.184314f, 0.309804f, 0.309804f)
#define Col_DarkSlateGrey	CFColor (0.184314f, 0.309804f, 0.309804f)
#define Col_DarkTurquoise	CFColor (0.439216f, 0.576471f, 0.858824f)
#define Col_DarkWood		CFColor (0.05f, 0.01f, 0.005f)
#define Col_DimGray		CFColor (0.329412f, 0.329412f, 0.329412f)
#define Col_DimGrey		CFColor (0.329412f, 0.329412f, 0.329412f)
#define Col_FireBrick		CFColor (0.9f, 0.4f, 0.3f)
#define Col_ForestGreen		CFColor (0.137255f, 0.556863f, 0.137255f)
#define Col_Gold		CFColor (0.8f, 0.498039f, 0.196078f)
#define Col_Goldenrod		CFColor (0.858824f, 0.858824f, 0.439216f)
#define Col_Gray		CFColor (0.752941f, 0.752941f, 0.752941f)
#define Col_Green		CFColor (0.0f, 1.0f, 0.0f)
#define Col_GreenYellow		CFColor (0.576471f, 0.858824f, 0.439216f)
#define Col_Grey		CFColor (0.752941f, 0.752941f, 0.752941f)
#define Col_IndianRed		CFColor (0.309804f, 0.184314f, 0.184314f)
#define Col_Khaki		CFColor (0.623529f, 0.623529f, 0.372549f)
#define Col_LightBlue		CFColor (0.74902f, 0.847059f, 0.847059f)
#define Col_LightGray		CFColor (0.658824f, 0.658824f, 0.658824f)
#define Col_LightGrey		CFColor (0.658824f, 0.658824f, 0.658824f)
#define Col_LightSteelBlue	CFColor (0.560784f, 0.560784f, 0.737255f)
#define Col_LightWood		CFColor (0.6f, 0.24f, 0.1f)
#define Col_LimeGreen		CFColor (0.196078f, 0.8f, 0.196078f)
#define Col_Magenta		CFColor (1.0f, 0.0f, 1.0f)
#define Col_Maroon		CFColor (0.556863f, 0.137255f, 0.419608f)
#define Col_MedianWood		CFColor (0.3f, 0.12f, 0.03f)
#define Col_MediumAquamarine	CFColor (0.196078f, 0.8f, 0.6f)
#define Col_MediumBlue		CFColor (0.196078f, 0.196078f, 0.8f)
#define Col_MediumForestGreen	CFColor (0.419608f, 0.556863f, 0.137255f)
#define Col_MediumGoldenrod	CFColor (0.917647f, 0.917647f, 0.678431f)
#define Col_MediumOrchid	CFColor (0.576471f, 0.439216f, 0.858824f)
#define Col_MediumSeaGreen	CFColor (0.258824f, 0.435294f, 0.258824f)
#define Col_MediumSlateBlue	CFColor (0.498039f, 0.0f, 1.0f)
#define Col_MediumSpringGreen	CFColor (0.498039f, 1.0f, 0.0f)
#define Col_MediumTurquoise	CFColor (0.439216f, 0.858824f, 0.858824f)
#define Col_MediumVioletRed	CFColor (0.858824f, 0.439216f, 0.576471f)
#define Col_MidnightBlue	CFColor (0.184314f, 0.184314f, 0.309804f)
#define Col_Navy		CFColor (0.137255f, 0.137255f, 0.556863f)
#define Col_NavyBlue		CFColor (0.137255f, 0.137255f, 0.556863f)
#define Col_Orange		CFColor (0.8f, 0.196078f, 0.196078f)
#define Col_OrangeRed		CFColor (0.0f, 0.0f, 0.498039f)
#define Col_Orchid		CFColor (0.858824f, 0.439216f, 0.858824f)
#define Col_PaleGreen		CFColor (0.560784f, 0.737255f, 0.560784f)
#define Col_Pink		CFColor (0.737255f, 0.560784f, 0.560784f)
#define Col_Plum		CFColor (0.917647f, 0.678431f, 0.917647f)
#define Col_Red			CFColor (1.0f, 0.0f, 0.0f)
#define Col_Salmon		CFColor (0.435294f, 0.258824f, 0.258824f)
#define Col_SeaGreen		CFColor (0.137255f, 0.556863f, 0.419608f)
#define Col_Sienna		CFColor (0.556863f, 0.419608f, 0.137255f)
#define Col_SkyBlue		CFColor (0.196078f, 0.6f, 0.8f)
#define Col_SlateBlue		CFColor (0.0f, 0.498039f, 1.0f)
#define Col_SpringGreen		CFColor (0.0f, 1.0f, 0.498039f)
#define Col_SteelBlue		CFColor (0.137255f, 0.419608f, 0.556863f)
#define Col_Tan			CFColor (0.858824f, 0.576471f, 0.439216f)
#define Col_Thistle		CFColor (0.847059f, 0.74902f, 0.847059f)
#define Col_Turquoise		CFColor (0.678431f, 0.917647f, 0.917647f)
#define Col_Violet		CFColor (0.309804f, 0.184314f, 0.309804f)
#define Col_VioletRed		CFColor (0.8f, 0.196078f, 0.6f)
#define Col_Wheat		CFColor (0.847059f, 0.847059f, 0.74902f)
#define Col_White		CFColor (1.0f, 1.0f, 1.0f)
#define Col_Yellow		CFColor (1.0f, 1.0f, 0.0f)
#define Col_YellowGreen		CFColor (0.6f, 0.8f, 0.196078f)

//=========================================================

#endif

