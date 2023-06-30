////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   crystaticmodel.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: cgf file loader
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef CryStaticModel_H
#define CryStaticModel_H

class CBaseObj;
class CGeom;

#include "../Cry3DEngine/Cry3DEngineBase.h"

struct LightInstance
{
	LIGHT_CHUNK_DESC Chunk;
  Vec3d vPos;
  char szName[64];
  struct ITexPic * pLightImage;
};

struct HelperInstance
{
	HELPER_CHUNK_DESC Chunk;
//  Vec3d vPos; 
//	CryQuat	qRot;
  char szName[64];
	Matrix44 tMat;
};

class CXFile;

struct CryStaticModel : public Cry3DEngineBase
{
	CryStaticModel();	
	~CryStaticModel();

  char m_FileName[256];

  list2<CGeom*>          m_lstGeoms;
  list2<MAT_ENTITY>      m_lstMaterials;	  
	list2<NAME_ENTITY>     m_lstGeomNames;
  list2<LightInstance>   m_lstLights;	
  list2<HelperInstance>  m_lstHelpers;	

  bool OnLoadgeom(char * filename, const char * geom_name, bool bLoadMats, bool bKeepInLocalSpace);

  float m_fBoundingRadius;
  float m_fCenterZ;

  void LoadMaterials(CXFile*f, int pos);
	int m_nNewObjs;
  CBaseObj ** m_ppNewObjs;
///  static CXFile * m_pXFile;
};

// timers that are used for precision very low cost profiling of load times
// this timer measures the time spent in the CGF Loader
//extern double g_dTimeLoadCGF;

#endif // CryStaticModel_H
