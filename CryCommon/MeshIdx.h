////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   meshidx.h
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: prepare shaders for object
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef IDX_MESH_H
#define IDX_MESH_H

#include "list2.h"

#define CCGF_CACHE_DIR_NAME "CCGF_CACHE"
#define CCGF_FILE_VERSION "CCGF_0.6"

#define CCGFHF_PHYSICS_EXIST 1

struct CCGFHeader
{
	CCGFHeader() 
	{ 
		memset(this,0,sizeof(*this)); 
		nStructuresCheckSummm = GetStructuresCheckSummm();
	}

	static int GetStructuresCheckSummm() 
	{ 
		return sizeof(CDLight)+sizeof(list2<int>)+sizeof(CMatInfo)+//sizeof(MAT_ENTITY)+
			sizeof(CVertexBuffer)+sizeof(SPipTangents)+m_VertexSize[0]+
			sizeof(uint); // [Anton] - for m_arrVtxMap in CLeafBuffer
	}

	int nDataSize;
	char szVersion[16];
	int nStructuresCheckSummm;
	FILETIME SourceFileTime;
	Vec3d vBoxMin, vBoxMax;
	int nFacesInCGFNum;
	int dwFlags;
};

struct TexCoord
{
  float s,t;

  bool operator == (TexCoord & other)
  {
    if(s == other.s)
    if(t == other.t)
      return 1;

    return 0;
  }
};

struct UColor
{
  uchar r,g,b,a;
};

#include "CryHeaders.h"

struct TextureMap2;
struct CryStaticModel;

struct StatHelperInfo
{
  StatHelperInfo(char * new_name, int new_type, IShader *pShader, const Matrix44 & newMat)
  {
//    vPos = new_pos;
//    qRot = new_rot;
    strncpy(sName,new_name,64);
    nType = new_type;
    m_pShader = pShader;
  //  m_pObject = NULL;
		tMat = newMat;
  }

  //Vec3d vPos; CryQuat qRot;
  int nType;
  char sName[64];
  IShader *m_pShader;
//  CCObject *m_pObject;
	Matrix44 tMat;
};

#define OBJFACE_FLAG_REMOVE 1

class CIndexedMesh
{
public:

  // geometry data
  CObjFace * m_pFaces;
  Vec3d * m_pVerts;
  TexCoord * m_pCoors;
	struct CBasis * m_pTangBasis;
	int * m_pVertMats;
  
  Vec3d * m_pNorms;
	UColor* m_pColor;
	UColor* m_pColorSec;
  int m_nFaceCount;
  int m_nVertCount;
  int m_nCoorCount; // number of texture coordinates in m_pCoors array
  int m_nNormCount;

  // bbox
  Vec3d m_vBoxMin, m_vBoxMax;

  // materials table
  list2<CMatInfo> m_lstMatTable;

  // list of geom names from cgf file
  list2<char*> m_lstGeomNames;

	FILETIME  m_fileTime;

   // constructor
	CIndexedMesh(){}

	// [Sergiy] constructs the CINdexedMesh object out of the given data,
	// [Sergiy] required by the CryAnimation module in order to create leaf buffers (generate render arrays)
	// [Sergiy] This code should be here, it deals with internal structure of the CIndexedMesh.
	// [Sergiy] Originally this was done externally in the GenerateRenderArrays method of CryModelState
	// PARAMETERS:
	// nFaces, pFaces, pTexFaces   - number of faces and texture faces and their arrays
	// nVertices, pVertices        - number of vertices and the array of vertices, in object coordinate system
	// nUVs, pUVs                  - number of UVs and the UVs themselves
	CIndexedMesh (
		unsigned nFaces, const CryFace* pFaces, const CryTexFace* pTexFaces,
		unsigned nVertices, const Vec3d* pVertices, const Vec3d* pNormals,
		unsigned nUVs, const CryUV* pUVs
		):
		m_pColor (NULL), m_pColorSec (NULL) // not initialized
	{
		m_pTangBasis=0;
		m_nFaceCount = nFaces;
		m_pFaces = (CObjFace *)malloc(sizeof(CObjFace)*nFaces);
		unsigned i=0;
		for (i=0; i<nFaces; i++)
		{
			const CryFace& face = pFaces[i];			

			m_pFaces[i].v[0] = face.v0;//flip
			m_pFaces[i].v[1] = face.v1;
			m_pFaces[i].v[2] = face.v2;
			m_pFaces[i].n[0] = face.v0;//flip
			m_pFaces[i].n[1] = face.v1;
			m_pFaces[i].n[2] = face.v2;
			m_pFaces[i].shader_id = face.MatID;
//			m_pFaces[i].m_lInfo = NULL;

			if (pTexFaces)
			{
				const CryTexFace& texface = pTexFaces[i];
				m_pFaces[i].t[0] = texface.t0;//flip
				m_pFaces[i].t[1] = texface.t1;
				m_pFaces[i].t[2] = texface.t2;
			}

			/*// Exit if the vertex index is out of range
			if(m_pFaces[i].v[0] >= nVertices
				|| m_pFaces[i].v[1] >= nVertices
				|| m_pFaces[i].v[2] >= nVertices)
				GetConsole()->Exit("CIndexedMesh: indices out of range");
				*/
		}

		m_pCoors = (TexCoord *)malloc(sizeof(TexCoord)*nUVs);
		m_nCoorCount = nUVs;
		for (i=0; i<nUVs; i++)
		{
			m_pCoors[i].s = pUVs[i].u;
			m_pCoors[i].t = pUVs[i].v;
		}

		m_pNorms = (Vec3d *)malloc(sizeof(Vec3d)*nVertices);
		m_nNormCount = nVertices;
		m_pVerts = (Vec3d *)malloc(sizeof(Vec3d)*nVertices);
		m_nVertCount = nVertices;

		for (i=0; i<nVertices; i++)
		{
			m_pNorms[i].x = pNormals[i].x;
			m_pNorms[i].y = pNormals[i].y;
			m_pNorms[i].z = pNormals[i].z;

			m_pVerts[i].x = pVertices[i].x;
			m_pVerts[i].y = pVertices[i].y;
			m_pVerts[i].z = pVertices[i].z;
		}
	}


	CIndexedMesh(ISystem	* pSystem,
    const char*szFileName,const char*szGeomName,//Vec3d * pvAngles,
    int*_count, bool bLoadAdditinalInfo, bool bKeepInLocalSpace, bool bIgnoreFakeMats = false); 

  ~CIndexedMesh();
  void FreeLMInfo();

  bool LoadCGF(const char*szFileName, const char * szGeomName, bool bLoadAdditinalInfo, bool bKeepInLocalSpace, bool bIgnoreFakeMats);
  static bool LoadMaterial(const char *szFileName, const char *szFolderName, 
                                CMatInfo & MatInfo, IRenderer * pRenderer, MAT_ENTITY * me);

  void SetMinMax();

  int GetAllocatedBytes();

  void * ReAllocElements(void * old_ptr, int old_elem_num, int new_elem_num, int size_of_element);

  void MakeLightSources(CryStaticModel * pStaticCGF);

	list2<StatHelperInfo> * GetHelpers() { return &m_lstHelpers; }
	list2<CDLight*> * GetLightSourcesList() { return &m_lstLSources; }

	size_t GetMemoryUsage();
	bool CalcTangentSpace();
	void DoBoxTexGen(float fScale, Vec3d * pvNormal = NULL);
	void SmoothMesh(bool bSmoothVerts, bool bSmoothNorms, Vec3d * pvBoxMin = NULL, Vec3d * pvBoxMax = NULL, bool bMarkInvalidFaces = false);
	void SimplifyMesh(float fMaxEdgeLen, Vec3d vBoundaryMin, Vec3d vBoundaryMax);
	void FixMesh();
	float GetEdgeError(struct TriEdge & tEdge, Vec3d * pVertsShared, struct VertInfo * pVertsInfo, list2<int> & lstIndices, UColor * pColorShared);
	void RebuildMesh(Vec3d * pVertsShared, TexCoord * pCoordShared,Vec3d * pNormsShared, UColor * pColorShared, list2<int> & lstIndices, int nVertCountShared, int * pVertMatsShared);
	bool RemoveEdge(Vec3d * pVertsShared, struct VertInfo * pVertsInfo, TexCoord * pCoordShared,Vec3d * pNormsShared, UColor * pColorShared, UColor * pColorSharedSec, list2<int> & lstIndices, int nVertCountShared,
		list2<struct TriEdge> & lstTriEdges, int nEdgeForRemoveId, const Vec3d & vBoundaryMin, const Vec3d & vBoundaryMax);
	void RemoveDuplicatedEdges(list2<struct TriEdge> & lstTriEdges);
	bool TestEdges(int nBeginWith, list2<TriEdge> & lstTriEdges, list2<int> & lstIndices, TriEdge * pEdgeForDelete);
	void MergeVertexFaceLists(int v0, int v1, struct VertInfo * pVertsInfo);
	bool InitEdgeFaces(TriEdge & e, VertInfo * pVertsInfo, Vec3d * pVertsShared, TriEdge * pe0, const Vec3d & vBoundaryMin, const Vec3d & vBoundaryMax);
	void MergeVertexComponents(TriEdge & e0, Vec3d * pVertsShared, struct VertInfo * pVertsInfo, 
		TexCoord * pCoordShared,Vec3d * pNormsShared, UColor * pColorShared, UColor * pColorSharedSec);
	CIndexedMesh * MakeMaterialMesh(int nMatId, uchar ucAxis);

	struct ISystem * m_pSystem;
	Vec3d m_vOrigin;

protected:
	list2<StatHelperInfo> m_lstHelpers;	

	list2<CDLight *> m_tgtLSources;
  list2<CDLight *> m_lstLSources;
};

#endif // IDX_MESH_H
