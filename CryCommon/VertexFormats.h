
//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:VertexFormats.h - 
//
//	History:
//	-Feb 23,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef VERTEXFORMATS_H
#define VERTEXFORMATS_H

#if _MSC_VER > 1000
# pragma once
#endif

// If you change this you also have to change gBufOffsTable in CRendElement.cpp
enum eVertexFormat
{
	VERTEX_FORMAT_P3F=1,                // shadow volumes (12 bytes)
  VERTEX_FORMAT_P3F_COL4UB=2,         // usually terrain (16 bytes)
  VERTEX_FORMAT_P3F_TEX2F=3,          // everything else (20 bytes)
	VERTEX_FORMAT_P3F_COL4UB_TEX2F=4,   // usually plants (24 bytes)
  VERTEX_FORMAT_TRP3F_COL4UB_TEX2F=5, // fonts (28 bytes)
	VERTEX_FORMAT_P3F_COL4UB_COL4UB=6,  // terrain with detail layers (20 bytes)
  VERTEX_FORMAT_P3F_N=7,              // (24 bytes)
  VERTEX_FORMAT_P3F_N_COL4UB=8,       // (28 bytes)
  VERTEX_FORMAT_P3F_N_TEX2F=9,        // (32 bytes)
	VERTEX_FORMAT_P3F_N_COL4UB_TEX2F=10, // (36 bytes)
  VERTEX_FORMAT_P3F_N_COL4UB_COL4UB=11,  // terrain with detail layers (32 bytes)
  VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F=12,  // usually plants (28 bytes)
  VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F=13,  // usually plants (40 bytes)
	VERTEX_FORMAT_T3F_B3F_N3F=14,        // tangent space (36 bytes)
  VERTEX_FORMAT_TEX2F=15,              // light maps TC (8 bytes)
  VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F=16,  // used for multitextured drawing
  VERTEX_FORMAT_NUMS=17,              // number of vertex formats
};

_inline int VertFormatForComponents(bool bNeedCol, bool bNeedSecCol, bool bNeedNormals, bool bHasTC)
{
  int RequestedVertFormat;

  if (!bNeedCol && !bHasTC && !bNeedNormals)
    RequestedVertFormat = VERTEX_FORMAT_P3F;
  else
  if (!bNeedCol && !bHasTC && bNeedNormals)
    RequestedVertFormat = VERTEX_FORMAT_P3F_N;
  else
  if (bNeedCol && bNeedSecCol && bNeedNormals && !bHasTC)
    RequestedVertFormat = VERTEX_FORMAT_P3F_N_COL4UB_COL4UB;
  else
  if (bNeedCol && bNeedSecCol && bNeedNormals && bHasTC)
    RequestedVertFormat = VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F;
  else
  if (bNeedCol && bNeedSecCol && !bNeedNormals && bHasTC)
    RequestedVertFormat = VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F;
  else
  if (bNeedCol && bNeedSecCol && !bHasTC)
    RequestedVertFormat = VERTEX_FORMAT_P3F_COL4UB_COL4UB;
  else
  if (bNeedCol && !bHasTC && !bNeedNormals)
    RequestedVertFormat = VERTEX_FORMAT_P3F_COL4UB;
  else
  if (bNeedCol && !bHasTC && bNeedNormals)
    RequestedVertFormat = VERTEX_FORMAT_P3F_N_COL4UB;
  else
  if (!bNeedCol && bHasTC && !bNeedNormals)
    RequestedVertFormat = VERTEX_FORMAT_P3F_TEX2F;
  else
  if (bNeedCol && bHasTC && !bNeedNormals)
    RequestedVertFormat = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
  else
  if (!bNeedCol && bHasTC && bNeedNormals)
    RequestedVertFormat = VERTEX_FORMAT_P3F_N_TEX2F;
  else
    RequestedVertFormat = VERTEX_FORMAT_P3F_N_COL4UB_TEX2F;

  return RequestedVertFormat;
}

struct SBufInfoTable
{
  int OffsTC;
  int OffsColor;
  int OffsSecColor;
  int OffsNormal;
};


struct struct_VERTEX_FORMAT_P3F // 12 bytes
{
  Vec3 xyz;
};
struct struct_VERTEX_FORMAT_P3F_COL4UB
{
  Vec3 xyz;
  UCol color;
};
struct struct_VERTEX_FORMAT_P3F_N // 24 bytes
{
  Vec3 xyz;
  Vec3 normal;
};
struct struct_VERTEX_FORMAT_TEX2F // 8 bytes
{
  float st[2];
};

struct struct_VERTEX_FORMAT_P3F_N_COL4UB
{
  Vec3 xyz;
  Vec3 normal;
  UCol color;
};

struct struct_VERTEX_FORMAT_P3F_TEX2F
{
  Vec3 xyz;
  float st[2];
};
struct struct_VERTEX_FORMAT_P3F_N_TEX2F
{
  Vec3 xyz;
  Vec3 normal;
  float st[2];
};

struct struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F
{
  Vec3 xyz;
  UCol color;
  float st[2];
};
struct struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F
{
	Vec3 xyz;
  UCol color;
  UCol seccolor;
  float st[2];
};
struct struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F
{
  Vec3 xyz;
  Vec3 normal;
  UCol color;
  UCol seccolor;
  float st[2];
};
struct struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F
{
  Vec3 xyz;
  UCol color;
  float st0[2];
  float st1[2];
};

struct struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F
{
  Vec3 xyz;
  Vec3 normal;
  UCol color;
  float st[2];

  bool operator == (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & other);
};

struct struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F
{
  float x,y,z,rhw;
  UCol color;
  float st[2];
};

struct struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB
{
  Vec3 xyz;
  UCol color;
  UCol seccolor;
};

struct struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB
{
  Vec3 xyz;
  Vec3 normal;
  UCol color;
  UCol seccolor;
};

struct SPipTangents
{
  Vec3 m_Tangent;
  Vec3 m_Binormal;
  Vec3 m_TNormal;
};

_inline void *CreateVertexBuffer(int nFormat, int nVerts)
{
  switch(nFormat)
  {
    case VERTEX_FORMAT_P3F:
      return new struct_VERTEX_FORMAT_P3F[nVerts];

    case VERTEX_FORMAT_P3F_N:
      return new struct_VERTEX_FORMAT_P3F_N[nVerts];

    case VERTEX_FORMAT_P3F_COL4UB_TEX2F:
      return new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[nVerts];

    case VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F:
      return new struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F[nVerts];

    case VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F:
      return new struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F[nVerts];

    case VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F:
      return new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F[nVerts];

    case VERTEX_FORMAT_P3F_N_COL4UB_TEX2F:
      return new struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F[nVerts];

    case VERTEX_FORMAT_P3F_TEX2F:
      return new struct_VERTEX_FORMAT_P3F_TEX2F[nVerts];

    case VERTEX_FORMAT_P3F_N_TEX2F:
      return new struct_VERTEX_FORMAT_P3F_N_TEX2F[nVerts];

    case VERTEX_FORMAT_P3F_COL4UB:
      return new struct_VERTEX_FORMAT_P3F_COL4UB[nVerts];

    case VERTEX_FORMAT_P3F_COL4UB_COL4UB:
      return new struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB[nVerts];

    case VERTEX_FORMAT_P3F_N_COL4UB_COL4UB:
      return new struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB[nVerts];

    case VERTEX_FORMAT_P3F_N_COL4UB:
      return new struct_VERTEX_FORMAT_P3F_N_COL4UB[nVerts];

    case VERTEX_FORMAT_TRP3F_COL4UB_TEX2F:
      return new struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F[nVerts];

    case VERTEX_FORMAT_TEX2F:
      return new struct_VERTEX_FORMAT_TEX2F[nVerts];

    default:
      assert(0);
  }
  return NULL;
}

#ifdef WIN64
// we don't care about truncation of the struct member offset, because
// it's a very small integer (even fits into a signed byte)
#pragma warning(push)
#pragma warning(disable:4311)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Sizes
const int m_VertexSize[]=
{
  0,
  sizeof(struct_VERTEX_FORMAT_P3F),	
  sizeof(struct_VERTEX_FORMAT_P3F_COL4UB),
  sizeof(struct_VERTEX_FORMAT_P3F_TEX2F),
  sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F),
  sizeof(struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F),
  sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB),
  sizeof(struct_VERTEX_FORMAT_P3F_N),	
  sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB),
  sizeof(struct_VERTEX_FORMAT_P3F_N_TEX2F),
  sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F),
  sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB),
  sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F),
  sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F),
  sizeof(SPipTangents),
  sizeof(struct_VERTEX_FORMAT_TEX2F),
  sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F_TEX2F),
};



// this is the table of offsets of UVs relative to the start of the structure
// -1 means there's no UVs in this format
// This is required by Animation and must be kept in tact with the vertex format enumeration
const int g_VertFormatUVOffsets[] = 
{
	-1, // no UVs in this format - invalid format
	-1, // VERTEX_FORMAT_P3F=1,                // shadow volumes (12 bytes)
	-1, // VERTEX_FORMAT_P3F_COL4UB=2,         // usually terrain (16 bytes)
	(int)&(((struct_VERTEX_FORMAT_P3F_TEX2F*)0)->st[0]), // VERTEX_FORMAT_P3F_TEX2F=3,          // everything else (20 bytes)
	(int)&(((struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*)0)->st[0]), // VERTEX_FORMAT_P3F_COL4UB_TEX2F=4,   // usually plants (24 bytes)
	(int)&(((struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F*)0)->st[0]),// VERTEX_FORMAT_TRP3F_COL4UB_TEX2F=5, // fonts (28 bytes)
	-1, // VERTEX_FORMAT_P3F_COL4UB_COL4UB=1,     
	-1, // VERTEX_FORMAT_P3F_N=1,                
  -1, // VERTEX_FORMAT_P3F_N_COL4UB=1,                
	(int)&(((struct_VERTEX_FORMAT_P3F_N_TEX2F*)0)->st[0]), // VERTEX_FORMAT_P3F_N_TEX2F=3,          // everything else (20 bytes)
	(int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F*)0)->st[0]) // VERTEX_FORMAT_P3F_N_COL4UB_TEX2F=4,   // usually plants (24 bytes)
  -1, // VERTEX_FORMAT_P3F_N_COL4UB_COL4UB=1,                
  (int)&(((struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F*)0)->st[0]), // VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F=4,   // usually plants (24 bytes)
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F*)0)->st[0]), // VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F=4,   // usually plants (24 bytes)
};


// this is the table of offsets of colors relative to the start of the structure
// -1 means there's no colors in this format
// This is required by Animation and must be kept in tact with the vertex format enumeration
const int g_VertFormatRGBAOffsets[] = 
{
	-1, // invalid format
	-1,
	(int)&(((struct_VERTEX_FORMAT_P3F_COL4UB*)0)->color.dcolor),
	-1,
	(int)&(((struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F*)0)->color.dcolor),
	(int)&(((struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F*)0)->color.dcolor),
  (int)&(((struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB*)0)->color.dcolor),
  -1,
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB*)0)->color.dcolor),
  -1,
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F*)0)->color.dcolor),
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB*)0)->color.dcolor),
  (int)&(((struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F*)0)->color.dcolor),
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F*)0)->color.dcolor),
};

// this is the table of offsets of normals relative to the start of the structure
// -1 means there's no colors in this format
// This is required by Animation and must be kept in tact with the vertex format enumeration
const int g_VertFormatNormalOffsets[] = 
{
	-1, // invalid format
	-1, // VERTEX_FORMAT_P3F=1
	-1, // VERTEX_FORMAT_P3F_COL4UB=2,
	-1, // VERTEX_FORMAT_P3F_TEX2F=3,
	-1, // VERTEX_FORMAT_P3F_COL4UB_TEX2F=4,
	-1, // VERTEX_FORMAT_TRP3F_COL4UB_TEX2F=5,
  -1, // VERTEX_FORMAT_P3F_COL4UB_COL4UB,
  (int)&(((struct_VERTEX_FORMAT_P3F_N*)0)->normal), // VERTEX_FORMAT_P3F_N=1,                
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB*)0)->normal), // VERTEX_FORMAT_P3F_N_COL4UB=1,                
  (int)&(((struct_VERTEX_FORMAT_P3F_N_TEX2F*)0)->normal), // VERTEX_FORMAT_P3F_N_TEX2F=3,          // everything else (20 bytes)
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F*)0)->normal), // VERTEX_FORMAT_P3F_N_COL4UB_TEX2F=4,   // usually plants (24 bytes)
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB*)0)->normal),
  -1, 
  (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F*)0)->normal),
};

static struct SBufInfoTable gBufInfoTable[] = 
{
  {
    0
  },
  {  //VERTEX_FORMAT_P3F
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F *)0)->x)  
    0
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_COL4UB
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_COL4UB *)0)->x)  
    0,
    OOFS(color.dcolor),
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_TEX2F
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_TEX2F *)0)->x)  
    OOFS(st[0])
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_COL4UB_TEX2F
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)0)->x)  
    OOFS(st[0]),
    OOFS(color.dcolor)
#undef OOFS
  },
  {  //VERTEX_FORMAT_TRP3F_COL4UB_TEX2F
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_TRP3F_COL4UB_TEX2F *)0)->x)  
    OOFS(st[0]),
    OOFS(color.dcolor),
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_COL4UB_COL4UB
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB *)0)->x)  
    0,
    OOFS(color.dcolor),
    OOFS(seccolor.dcolor),
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_N
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_N *)0)->x)  
    0,
    0,
    0,
    OOFS(normal.x),
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_N_COL4UB
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB *)0)->x)  
    0,
    OOFS(color.dcolor),
    0,
    OOFS(normal.x),
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_N_TEX2F
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_N_TEX2F *)0)->x)  
    OOFS(st[0]),
    0,
    0,
    OOFS(normal.x),
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_N_COL4UB_TEX2F
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *)0)->x)  
    OOFS(st[0]),
    OOFS(color.dcolor),
    0,
    OOFS(normal.x),
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_N_COL4UB_COL4UB
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB *)0)->x)  
    0,
    OOFS(color.dcolor),
    OOFS(seccolor.dcolor),
    OOFS(normal.x),
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_COL4UB_COL4UB_TEX2F *)0)->x)  
    OOFS(st[0]),
    OOFS(color.dcolor),
    OOFS(seccolor.dcolor),
    0,
#undef OOFS
  },
  {  //VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F
#define OOFS(x) (int)&(((struct_VERTEX_FORMAT_P3F_N_COL4UB_COL4UB_TEX2F *)0)->x)  
    OOFS(st[0]),
    OOFS(color.dcolor),
    OOFS(seccolor.dcolor),
    OOFS(normal.x),
#undef OOFS
  },
};

#ifdef WIN64
#pragma warning(pop)
#endif

_inline void GetVertBufComps(SVertBufComps *Comps, int Format)
{
  memset(Comps, 0, sizeof(SVertBufComps));
  if (gBufInfoTable[Format].OffsTC)
    Comps->m_bHasTC = true;
  if (gBufInfoTable[Format].OffsColor)
    Comps->m_bHasColors = true;
  if (gBufInfoTable[Format].OffsSecColor)
    Comps->m_bHasSecColors = true;
  if (gBufInfoTable[Format].OffsNormal)
    Comps->m_bHasNormals = true;
}



#endif