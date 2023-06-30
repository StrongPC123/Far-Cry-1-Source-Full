/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	08/28/2002 - Created by Sergiy Migdalskiy <sergiy@crytek.de>
//
//  Contains:
//  Implementation of CryGeometryInfo, a structure holding geometry in the form that is  to
//  the data structure of CGF, and supporting subclass(es)
/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <I3DEngine.h>
#include <IEdgeConnectivityBuilder.h>									// IEdgeConnectivityBuilder
#include <StlUtils.h>
#include <StringUtils.h>
#include "MeshIdx.h"
#include "VertexBufferArrayDrivers.h"
#include "CryGeometryInfo.h"
#include "ChunkFileReader.h"
#include "CrySkinBuilder.h"
#include "CrySkinBasisBuilder.h"
#include "CrySkinRigidBasis.h"
#include "CrySkinFull.h"
#include "CryBone.h"
#include "RenderUtils.h"
#include "CVars.h"
#include "CryCompiledFile.h"

CryGeometryInfo::CryGeometryInfo():
	m_pStencilShadowConnectivity(NULL),
	m_numUsedVertices(0),
	m_arrVertBuf ("CryGeometryInfo.VertBuf"),
	m_nVertBufFormat (-1),
	m_arrExtTangents ("CryGeometryInfo.ExtTangents"),
	m_arrExtToIntMap ("CryGeometryInfo.ExtToIntMap"),
	m_arrExtUVs ("CryGeometryInfo.ExtUVs")
	, m_arrVertices    ("CryGeometryInfo.Vertices")
	, m_arrNormals     ("CryGeometryInfo.Normals")
	, m_arrTexFaces    ("CryGeometryInfo.TexFaces")
	, m_arrUVs         ("CryGeometryInfo.UVs")
	, m_arrLinks       ("CryGeometryInfo.Links")
	, m_arrIntToExtMap ("CryGeometryInfo.IntToExtMap")
{
}


CryGeometryInfo::~CryGeometryInfo() 
{
	if (m_pStencilShadowConnectivity)
		m_pStencilShadowConnectivity->Release();
}


void CryGeometryInfo::buildGeomSkins(const CryBoneInfo* pBoneInfo, unsigned numBoneInfos)
{
	assert(m_SkinGeom.empty());

	// build the skinner that will skin the vertices
	{
		CrySkinVertexSource SkinVertexSource (this);
		CrySkinBuilder builder (&SkinVertexSource);
		builder.initSkinFull(&m_SkinGeom);
	}

	// build a separate skinner for normals. The skinner itself
	// actually doesn't even know that those are normals, it skins them just like normal vertices
	{
		CrySkinNormalSource SkinNormalSource (this, pBoneInfo);
		CrySkinBuilder builder (&SkinNormalSource);
		builder.initSkinFull(&m_SkinNormal);
	}
}


bool CryGeometryInfo::loadVertexSkin (const void* pData, unsigned nSize)
{
	return m_SkinGeom.Serialize_PC(false, (void*)pData, nSize) != 0;
}

bool CryGeometryInfo::loadNormalSkin (const void* pData, unsigned nSize)
{
	return m_SkinNormal.Serialize_PC(false, (void*)pData, nSize) != 0;
}

bool CryGeometryInfo::loadTangSkin (const void* pData, unsigned nSize)
{
	return m_TangSkin.Serialize(false, (void*)pData, nSize) != 0;
}


// creates (allocates with new[]) an array of bindings for each normal
// there are numVertices() elements in this array
CryVertexBinding* CryGeometryInfo::newNormalLinks (const CryBoneInfo* pBoneInfo)
{
	CryVertexBinding* pNormals = new CryVertexBinding[numVertices()];

	for (unsigned i = 0; i < numVertices(); ++i)
	{
		pNormals[i].resize (1);
		unsigned nBone = pNormals[i][0].BoneID   = getLink(i)[0].BoneID;
		pNormals[i][0].Blending	= 1;//getLink(0)[0].Blending;
		//CHANGED_BY_IVO
		pNormals[i][0].offset   = pBoneInfo[nBone].getInvDefGlobal().TransformVectorOLD(getNormal (i));
		// Temporary fixed by Sergiy. TransformVector is not the same as TransformPoint
		//pNormals[i][0].offset   = GetTransposed44(pBoneInfo[nBone].getInvDefGlobal())*( getNormal(i) );
	}

	return pNormals;
}

bool CryGeometryInfo::hasGeomSkin()const
{
	return !m_SkinGeom.empty();
}

CrySkinFull* CryGeometryInfo::getGeomSkin()
{
	assert(!m_SkinGeom.empty());
	return &m_SkinGeom;
}

CrySkinFull* CryGeometryInfo::getNormalSkin()
{
	assert(!m_SkinNormal.empty());
	return &m_SkinNormal;
}

// computes, caches and returns the connectivity object for stencil shadows
// (or just returns, if the object is already cached)
IStencilShadowConnectivity* CryGeometryInfo::getStencilShadowConnectivity (const std::vector<MAT_ENTITY>&arrMaterials)
{
	if (!m_pStencilShadowConnectivity)
		buildStencilShadowConnectivity(arrMaterials);
	return m_pStencilShadowConnectivity;
}


// deserializes the stencil shadow connectivity object
void CryGeometryInfo::setStencilShadowConnectivity (IStencilShadowConnectivity* pConnectivity)
{
	if (m_pStencilShadowConnectivity)
		m_pStencilShadowConnectivity->Release();

	m_pStencilShadowConnectivity = pConnectivity;
}


void CryGeometryInfo::OutputOrphanedEdgeWarning(class IEdgeConnectivityBuilder *iEdgeBuilder, const std::vector<MAT_ENTITY>&arrMaterials)
{
	unsigned numOrphanedEdges = iEdgeBuilder->numOrphanedEdges();
	if (numOrphanedEdges > 0)
	{
		// calculate the set of materials of faces which have open edges
		std::vector<unsigned> arrFaces; // the faces that have open edges
		arrFaces.resize (numOrphanedEdges);
		iEdgeBuilder->getOrphanedEdgeFaces(&arrFaces[0]);
		std::set<unsigned> setShaders; // the shaders to which the faces belong
		for (unsigned i = 0; i < numOrphanedEdges; ++i)
		{
			assert (arrFaces[i] < m_arrFaceMtl.size());
			setShaders.insert (m_arrFaceMtl[arrFaces[i]]);
		}

		// the set of shader names
		std::set<string> setShaderNames;
		string strShaderNameList;
		for (std::set<unsigned>::iterator it = setShaders.begin(); it != setShaders.end(); ++it)
		{
			unsigned nShader = *it;
			if (nShader < arrMaterials.size())
				setShaderNames.insert (arrMaterials[nShader].name);
			else
			{
				char szBuf[64];
				sprintf (szBuf, "#Unknown#%d#", nShader);
			}
		}

		strShaderNameList = "Shaders:" + CryStringUtils::toString (setShaderNames);
		g_GetLog()->LogWarning ("\003Shadow Connectivity Builder: %u orphaned edges discovered in the model %s", iEdgeBuilder->numOrphanedEdges(), strShaderNameList.c_str());
	}
}

//////////////////////////////////////////////////////////////////////////
// builds the connectivity object for stencil shadows
// PARAMETERS:
//  iEdgeBuilder - the builder to use to create the connectivity info
//  pbCastShadow - the array of flags, 1 flag per 1 material, if the flag is true, then this material casts shadow, otherwise not
//  numMaterials - number of items in the pbCastShadow array
void CryGeometryInfo::buildStencilShadowConnectivity (const std::vector<MAT_ENTITY>&arrMaterials)
{
	IEdgeConnectivityBuilder *iEdgeBuilder = Get3DEngine()->GetNewConnectivityBuilder();
	assert(iEdgeBuilder);						// should be always there
#ifdef _DEBUG
	if (m_pStencilShadowConnectivity && g_GetCVars()->ca_DebugRebuildShadowVolumes())
	{
		m_pStencilShadowConnectivity->Release();
		m_pStencilShadowConnectivity = NULL;
	}
#endif

	if (!m_pStencilShadowConnectivity)
	{
		unsigned numFaces = (unsigned)this->numFaces();
		iEdgeBuilder->ReserveForTriangles(numFaces,numUsedVertices());

		for (unsigned i = 0; i < numFaces; ++i)
		{
			GeomFace Face = getFace(i);
			GeomMtlID nMaterial = getFaceMtl(i);
			assert (nMaterial < arrMaterials.size());
			if (nMaterial < arrMaterials.size() && arrMaterials[nMaterial].Dyn_StaticFriction == 1)
				continue;

			// with welding
			iEdgeBuilder->AddTriangleWelded (Face[0], Face[1], Face[2], m_arrVertices[Face[0]], m_arrVertices[Face[1]], m_arrVertices[Face[2]]);
		}

		OutputOrphanedEdgeWarning(iEdgeBuilder, arrMaterials);
		//if (iEdgeBuilder->numOrphanedEdges() > 0)
		//	GetLog()->Log ("Shadow Connectivity Builder: %u orphaned edges discovered in the model", iEdgeBuilder->numOrphanedEdges());

		m_pStencilShadowConnectivity=iEdgeBuilder->ConstructConnectivity();

#ifdef _DEBUG
		if(m_pStencilShadowConnectivity)
		{
			char str[256];
			DWORD dwVertCount,dwTriCount;

			m_pStencilShadowConnectivity->GetStats(dwVertCount,dwTriCount);

			sprintf(str,"StencilEdgeConnectivity Indoor Stats: %d/%d Vertices %d/%d Faces\n",dwVertCount,numUsedVertices(),dwTriCount,numFaces);
		#ifdef WIN32
			OutputDebugString(str);
		#endif
	
		#ifdef GAMECUBE
			OSReport(str);
		#endif

		}
#endif

		clearConstructionData();
	}
}

// Allocates arrays that keep the vertices and tangents
void CryGeometryInfo::PrepareVertices (unsigned numVertices)
{
	m_numUsedVertices =numVertices;
	m_arrVertices.reinit (numVertices);
	m_arrNormals.reinit (numVertices);
}

// Allocattes the texture coordinate array
void CryGeometryInfo::PrepareUVs (int numUVs)
{
	m_arrUVs.reinit(numUVs);
}


// Allocates texture face array
void CryGeometryInfo::PrepareTexFaces (int numTexFaces)
{
	assert(numFaces() == numTexFaces);
	m_arrTexFaces.reinit(numTexFaces);
}

// Allocates the external to internal map entries
void CryGeometryInfo::PrepareExtToIntMap (int numExtVertices)
{
	m_arrExtToIntMap.reinit (numExtVertices, 0);
	m_arrExtTangents.reinit (numExtVertices);
}

// Create the Internal-to-external map
void CryGeometryInfo::CreateIntToExtMap ()
{
	m_arrIntToExtMap.reinit (numUsedVertices(), 0);
	unsigned numExternalVertices = (unsigned)m_arrExtToIntMap.size();
	for (unsigned nExternalVertex = 0; nExternalVertex < numExternalVertices; ++nExternalVertex)
	{
		m_arrIntToExtMap[m_arrExtToIntMap[nExternalVertex]] = nExternalVertex;
	}
}


// returns the number of texture faces, which is either NULL,
// or the same as the number of geometry faces
unsigned CryGeometryInfo::numTexFaces() const
{
	return m_arrTexFaces.empty()? 0 : (unsigned)numFaces();
}


void CryGeometryInfo::PrepareLinks (int numVertices)
{
	assert (
		numVertices == this->numVertices()
	);
	m_arrLinks.reinit (numVertices);
}

// returns the number of links, link array is either NULL or an array with numVertices() elements
unsigned CryGeometryInfo::numLinks() const
{
	return m_arrLinks.empty()? 0 : numUsedVertices();
}


void CryGeometryInfo::MakeIntFaces(const CryFace* pCryFaces, unsigned numFaces)
{
	m_arrFaces.resize (numFaces);
	m_arrFaceMtl.resize (numFaces);
	for (unsigned nFace = 0; nFace < numFaces; ++nFace)
		for (unsigned v = 0; v < 3; ++v)
		{
			m_arrFaces[nFace][v] = (unsigned short)pCryFaces[nFace][v];
			m_arrFaceMtl[nFace] = (GeomMtlID)pCryFaces[nFace].MatID;
		}
}


//////////////////////////////////////////////////////////////////////////
// sorts the faces by the material indices.
// in the given array, sets elements corresponding to used materials to true; doesn't touch the other elements
// (i.e. the array elements must be set to false before calling this function)
// IMPORTANT: Keeps the original order of faces in each material group
void CryGeometryInfo::sortFacesByMaterial (std::vector<bool>& arrMtlUsage)
{
	// for each material, construct an array of CryFaces and keep the faces (in the original order) in there
	typedef std::vector<GeomFace> GeomFaceArray;
	typedef std::map<GeomMtlID, GeomFaceArray> IntFaceMtlMap;
	IntFaceMtlMap mapMtlFace;
	unsigned i;
	
	// put each face into its material group;
	// the corresponding groups CryFaceArray will be created by the map automatically
	// upon the first request of that group
	//
  for (i = 0; i < numFaces(); ++i)
	{
		GeomMtlID nMaterial = getFaceMtl (i);
    
		mapMtlFace[nMaterial].push_back(getFace(i));
		
		if ((unsigned)nMaterial < arrMtlUsage.size())
			arrMtlUsage[nMaterial] = true;
		else
			assert (0); // material is out of range
	}

	// Scan through all the faces and put the sorted face infos back into the original array
	// scan through each material group sequentally, so that the material sort order is maintained
	unsigned nNewFace = 0;
	for (IntFaceMtlMap::iterator itMtl = mapMtlFace.begin(); itMtl != mapMtlFace.end(); ++itMtl)
  {
		GeomFaceArray& arrFaces = itMtl->second;
		for (GeomFaceArray::iterator itFace = arrFaces.begin(); itFace != arrFaces.end(); ++itFace)
		{
			m_arrFaces[nNewFace] = *itFace;
			m_arrFaceMtl[nNewFace] = itMtl->first;
			++nNewFace;
		}
  }
	assert (nNewFace == numFaces());
}


//////////////////////////////////////////////////////////////////////////
// creates an instance of the CIndexedMesh with the info from this geometry info.
// this is needed to create the leaf buffers (while GenerateRenderArrays() execution)
// The client is responsible for releasing (by delete operator) the created instance.
CIndexedMesh* CryGeometryInfo::new3DEngineIndexedMesh()const
{
	CryFace* pCryFaces = newCryFaces();

	CIndexedMesh* pMesh = new CIndexedMesh(
			(unsigned)numFaces(), pCryFaces, getTexFaces(),
			(unsigned)numVertices(), getVertices(), getNormals(),
			(unsigned)numUVs(), getUVs()
		);
	delete[]pCryFaces;
	return pMesh;
}

// creates a fake array of faces
CryFace* CryGeometryInfo::newCryFaces()const
{
	CryFace* pCryFaces = new CryFace[numFaces()];
	const GeomFace* pFaces = getFaces();
	for (unsigned nFace = 0; nFace < numFaces(); ++nFace)
	{
		pCryFaces[nFace].v0 = pFaces[nFace][0];
		pCryFaces[nFace].v1 = pFaces[nFace][1];
		pCryFaces[nFace].v2 = pFaces[nFace][2];
		pCryFaces[nFace].SmGroup = 1;
		pCryFaces[nFace].MatID = getFaceMtl(nFace);
	}
	return pCryFaces;
}

// calculates the bounding box for the object
// if the bbox can't be calculated (e.g. the skin hasn't been yet loaded), returns false
bool CryGeometryInfo::computeBBox (CryAABB& bb)
{
	if (m_arrVertices.empty())
	{
		// set the (0,0,0)-(0,0,0) empty bounding box
		bb = CryAABB( Vec3d(0,0,0), Vec3d(0,0,0) );
		return false;
	}

	Vec3dArray::iterator it = m_arrVertices.begin(), itEnd = it + numVertices();
	bb = CryAABB(*it,*it);
	++it;
	for (; it != itEnd; ++it)
	{
		bb.include(*it);
	}
	return true;
}


DWORD toDword(const CryIRGB color)
{
	return ((DWORD)(((BYTE)(color.r)|((WORD)((BYTE)(color.g))<<8))|(((DWORD)(BYTE)(color.b))<<16)))| 0xFF000000;
}


//////////////////////////////////////////////////////////////////////////
// Loads the given geometry into this geometry info.
// The geometry is given as the part of the CGF file
// PARAMETERS:
//   nLOD       - treat the geometry as the specified LOD
//   pChunk     - pointer to the chunk
//   nChunkSize - chunk size, in bytes
// RETURNS:
//   the number of bytes read; 0 if the chunk is useless
unsigned CryGeometryInfo::Load (unsigned nLOD, const MESH_CHUNK_DESC_0744* pChunk, unsigned nChunkSize)
{
	const void *pRawData = pChunk+1;

	if (nChunkSize < sizeof(*pChunk))
		return 0;

	nChunkSize -= sizeof(*pChunk);

  if(pChunk->nVerts<=0 || pChunk->nVerts>60000)
    return 0;

	PrepareVertices (pChunk->nVerts);
	assert (pChunk->nVerts);

	const CryVertex* pSrcVertices = (const CryVertex*)pRawData;
	if ((const char*)(pSrcVertices + pChunk->nVerts) > (const char*)pRawData + nChunkSize)
		return 0; // the cry vertex info is truncated
	for (int i = 0; i < pChunk->nVerts; ++i)
	{
		m_arrVertices[i] = pSrcVertices[i].p;
		m_arrNormals[i] = pSrcVertices[i].n;
	}

	pRawData = pSrcVertices + pChunk->nVerts;
 
  //read faces
	std::vector<CryFace> arrCryFaces;
	arrCryFaces.resize(pChunk->nFaces);
  
  if (!EatRawData (&arrCryFaces[0], pChunk->nFaces, pRawData, nChunkSize))
		return 0;

	MakeIntFaces(&arrCryFaces[0], pChunk->nFaces);
	// convert the CryFace's into int faces
	arrCryFaces.clear();

	if (!ValidateFaceIndices())
		g_GetConsole()->Exit("CryGeometryInfo::Load: Indices out of range");

  //read tverts
  if(pChunk->nTVerts)
  {
    PrepareUVs(pChunk->nTVerts);
		if (!EatRawData(getUVs(), pChunk->nTVerts, pRawData, nChunkSize))
			return 0;

    // flip tex coords (since it was flipped in max?)
    for (unsigned t = 0; t < (unsigned)pChunk->nTVerts; ++t)
      getUV(t).v = 1.f-getUV(t).v;

    //read Tfaces
    PrepareTexFaces(pChunk->nFaces);
    if (!EatRawData (getTexFaces(), pChunk->nFaces, pRawData, nChunkSize))
			return 0;

		if (!ValidateTexFaceIndices())
			g_GetConsole()->Exit("CryGeometryInfo::Load: Texture vertex Indices out of range");
  }

  //read Links if there is the bone info.
	// NOTE: we allow the bone info to be absent if it's not needed, even if the HasBoneInfo flag is set.
	// this is due to the bug in the exporter utility that can set the flag but not export the actual bones
	bool bHasBoneInfo = pChunk->HasBoneInfo;
	bool bNoLinksWarning = false;
	if (nChunkSize == 0)
		bHasBoneInfo = false; // ignore the flag if the chunk doesn't actually contain the bone info

	if(bHasBoneInfo)
  {
		PrepareLinks(pChunk->nVerts);

    for (int i=0; i < pChunk->nVerts; i++)
    {
			// the links for the current vertex in the geometry info structure
			CryVertexBinding& arrLinks = getLink(i);
			
			// read the number of links and initialize the link array
			{
				// the number of links for the current vertex
				unsigned numLinks; 
				if (!EatRawData(&numLinks,1,pRawData, nChunkSize))
					return 0;

				if(numLinks > 32u)
				{ 
					g_GetLog()->LogError ("\001CryGeometryInfo::Load: Number of links for vertex is invalid: %u", numLinks);
					return 0;
				}

				arrLinks.resize(numLinks);
			}

			if (arrLinks.empty())
			{
				if (!bNoLinksWarning)
				{
					g_GetLog()->LogError ("\003file contains unbound vertices");
					bNoLinksWarning = true;
				}
				arrLinks.resize(1);
				arrLinks[0].Blending = 1;
				arrLinks[0].BoneID = 0;
				arrLinks[0].offset = Vec3d (0,0,0);
				continue;
			}
			else
	      if (!EatRawData(&arrLinks[0], (unsigned)arrLinks.size(), pRawData, nChunkSize))
					return 0;

			arrLinks.sortByBlendingDescending();

      // limit links number
      if(arrLinks.size() > 3)
        arrLinks.resize(3);

			arrLinks.normalizeBlendWeights();
			arrLinks.pruneSmallWeights (g_GetCVars()->ca_MinVertexWeightLOD(nLOD));
			arrLinks.normalizeBlendWeights();
    }
		// optimize the vertex order for skinning - will interfere with morph targets
		//sortVertices();
  }


  //read Vertex Colors
  if(pChunk->HasVertexCol)
  {
		if (g_GetCVars()->ca_AnimWarningLevel()>2)
			g_GetLog()->LogWarning ("\004Vertex colors are found in mesh");

		TElementaryArray<CryIRGB> arrVColors;
		arrVColors.reinit (pChunk->nVerts);
    if (EatRawData (&arrVColors[0], pChunk->nVerts, pRawData, nChunkSize))
		{
			m_arrVColors.resize (pChunk->nVerts);
			for (int i = 0; i < pChunk->nVerts; ++i)
				m_arrVColors[i] = toDword (arrVColors[i]);
		}

		//nChunkSize -= sizeof(CryIRGB) * pChunk->nVerts;
		//pRawData = (CryIRGB*)pRawData + pChunk->nVerts;
  }

  return ((byte*)pRawData) - ((byte*)pChunk);
}


// scales the model. Multiplies all vertex coordinates by the given factor
void CryGeometryInfo::Scale (float fScale)
{
	Vec3dArray::iterator itVertex = m_arrVertices.begin(), itVertexEnd = itVertex + numVertices();
	for (; itVertex != itVertexEnd; ++itVertex)
		*itVertex *= fScale;

	VertexBindingArray::iterator itLink = m_arrLinks.begin(), itLinkEnd = itLink + numLinks();
	for (; itLink != itLinkEnd; ++itLink)
		itLink->scaleOffsets(fScale);
}


// Checks the face indices, if they are in range (0..numVertices())
// returns true upon successful validation, false means there are indices out of range
bool CryGeometryInfo::ValidateFaceIndices ()
{
  // prepare (cache) lookup values
	unsigned numVertices = this->numUsedVertices();
	FaceArray::iterator it, itEnd = m_arrFaces.end();

	// check indices of each face
  for (it = m_arrFaces.begin(); it != itEnd; ++it)
  {
		for (int v = 0; v < 3; ++v)
			if ((unsigned)((*it)[v]) >= numVertices)
				return false;
  }
	return true; // validation passed successfully
}


// Checks the texture face indices, if they are in range (0..numVertices())
// returns true upon successful validation, false means there are indices out of range
bool CryGeometryInfo::ValidateTexFaceIndices ()
{
	// prepare (cache) lookup values
	int numUVs = this->numUVs();
	TexFaceArray::iterator
		it = m_arrTexFaces.begin(),
		itEnd = it + numTexFaces();

	// check indices of each face
	for (; it != itEnd; ++it)
	{
		if ( it->t0 >= numUVs || it->t0 < 0
			|| it->t1 >= numUVs || it->t1 < 0
			|| it->t2 >= numUVs || it->t2 < 0 )
			return false;
	}
	return true; // validation passed successfully
}


// forces the material indices to be in the range [0..numMaterials]
void CryGeometryInfo::limitMaterialIDs(unsigned numMaterials)
{
	for (unsigned i = 0; i < numFaces(); ++i)
	{
		if ((unsigned)m_arrFaceMtl[i] >= numMaterials)
			m_arrFaceMtl[i] = 0;
	}
}

// recalculates all the normals, assuming smooth geometry (non-smooth vertices were already split in the exporter)
void CryGeometryInfo::recalculateNormals()
{
	unsigned i, nActiveVerts = numVertices();
#ifdef _DEBUG
	Vec3dElementaryArray arrOldNormals(nActiveVerts);
	for (i = 0; i < nActiveVerts; ++i)
		arrOldNormals[i] = getNormal(i);
#endif

  for (i=0; i<nActiveVerts; i++)
		getNormal(i)(0,0,0);

  for (unsigned n = 0; n < numFaces(); ++n)
  {
    const GeomFace& fc = getFace(n);
		Vec3d v1, v2, vTmpNormal;
		v1 = getVertex(fc[0]) - getVertex(fc[1]);
		v2 = getVertex(fc[0]) - getVertex(fc[2]);
		vTmpNormal = v1 ^ v2;
		//vTmpNormal.Normalize();

		for (int v = 0; v < 3; ++v)
		{
			Vec3d& vNormal = getNormal(fc[v]);
			Vec3d vNewNormal = vTmpNormal + vNormal;
			//assert (vNewNormal.Length() > 1e-3);
			vNormal = vNewNormal;
		}
  }

	for (i=0; i<nActiveVerts; i++)
	{
		Vec3d& vN = getNormal(i);
		float fLength = vN.Length();
		// the threshold must be very low because the areas of rectangles
		// are very small (in square meters), it can easily be 1e-4 (square mm) or even 1e-6 (square cm)
		// and we don't loose anything if we set some random (but not NaN) normal instead of 0,1,0 which is also random
		if (fLength < 1e-12)
			vN = Vec3d(0,1,0);
		else
			vN /= fLength;
	}
/*#ifdef _DEBUG
	for (i = 0; i < nActiveVerts; ++i)
	{
		Vec3d vOld = arrOldNormals[i];
		Vec3d vNew = getNormal(i);
		assert (GetDistance(vOld, vNew) < 0.5);
	}
#endif*/
}

// returns true if the geometry is degraded (e.g. no vertices or no faces)
bool CryGeometryInfo::empty()
{
	return numVertices() == 0 || numFaces () == 0;
}

// remaps the ids of the bones in the CryLink structures
void CryGeometryInfo::remapBoneIds (const unsigned* arrBoneIdMap, unsigned numBoneIds)
{
	VertexBindingArray::iterator it = m_arrLinks.begin(), itEnd = it + numLinks();
	for (; it != itEnd; ++it)
	{
		it->remapBoneIds (arrBoneIdMap, numBoneIds);
	}
}

// remaps the ids of materials. arrMtlIdMap[nOldMtlId] == nNewMtlId
void CryGeometryInfo::remapMtlIds (const unsigned* arrMtlIdMap, unsigned numMtlIds)
{
	for (unsigned nFace = 0; nFace < numFaces(); ++nFace)
	{
		GeomMtlID nOldMtlId = getFaceMtl (nFace);
		GeomMtlID nNewMtlId = 0;
		if (nOldMtlId < numMtlIds)
      nNewMtlId = arrMtlIdMap[nOldMtlId];			
		m_arrFaceMtl[nFace] = nNewMtlId;
	}
}


// rotates the model; transforms each vector with the specified matrix, using only its
// ROTATIONAL components (no translation)
void CryGeometryInfo::rotate (const Matrix44& tm)
{
	Vec3dArray::iterator it = m_arrVertices.begin(), itEnd = it + numVertices();
	for (; it != itEnd; ++it)
		//CHANGED_BY_IVO - INVALID CHANGE, PLEASE REVISE
		*it = tm.TransformVectorOLD(*it);
		//*it = GetTransposed44(tm)*(*it);
}

// transforms the model with the given transform matrix
void CryGeometryInfo::transform (const Matrix44& tm)
{
	Vec3dArray::iterator it = m_arrVertices.begin(), itEnd = it + numVertices();
	for (; it != itEnd; ++it)
		*it = tm.TransformPointOLD(*it);
}

// destroys the UV array
void CryGeometryInfo::removeUVs()
{
	m_arrUVs.clear();
}

// destroys the texture faces
void CryGeometryInfo::removeTexFaces()
{
	m_arrTexFaces.clear();
}


void CryGeometryInfo::initExtUVs (const CVertexBufferUVArrayDriver& pUVs)
{
	unsigned i, numExtUVs = numExtToIntMapEntries();
	m_arrExtUVs.reinit (numExtUVs);
	for (i = 0; i < numExtUVs; ++i)
	{
		m_arrExtUVs[i] = pUVs[i];
	}
}

#if 0
// after strip-generation of the mesh, rearranges the geometry info to match the sequence of vertices
// in the stripified mesh
void CryGeometryInfo::remapVerticesToExt ()
{
	// reorder: Vertices, Normals, Face Indices, Links
	// then destroy the ext<->int mappings

	GetLog()->LogToFile ("\005CryGeometryInfo::remapVerticesToExt:%d->%d", numVertices(), numExtToIntMapEntries());

	unsigned numExtVerts = numExtToIntMapEntries();

	Vec3dArray arrExtVertices, arrExtNormals;
	arrExtVertices.reinit (numExtVerts);
	arrExtNormals.reinit (numExtVerts);

	VertexBindingArray arrExtLinks;
	arrExtLinks.reinit (numExtVerts);

	for (unsigned nExtVert = 0; nExtVert < numExtVerts; ++nExtVert)
	{
		unsigned nIntVert = getExtToIntMapEntry(nExtVert);
		arrExtVertices[nExtVert] = m_arrVertices[nIntVert];
		arrExtNormals[nExtVert]  = m_arrNormals[nIntVert];
		arrExtLinks[nExtVert]    = m_arrLinks[nIntVert];
	}

	m_arrLinks.swap (arrExtLinks);
	m_arrVertices.swap (arrExtVertices);
	m_arrNormals.swap (arrExtNormals);

	for (unsigned nFace = 0; nFace < numFaces(); ++nFace)
	{
		CryFace& face = m_arrFaces[nFace];
		face.v0 = m_arrIntToExtMap[face.v0];
		face.v1 = m_arrIntToExtMap[face.v1];
		face.v2 = m_arrIntToExtMap[face.v2];
	}

	m_arrExtToIntMap.clear();
	m_arrIntToExtMap.clear();
}
#endif

class CSkinVertexSorter
{
public:
	CSkinVertexSorter (CryGeometryInfo* pGeom):
		m_pGeom(pGeom)
	{}

	bool operator() (unsigned nLeft, unsigned nRight)const
	{
		CryVertexBinding& rLeftBind = m_pGeom->getLink(nLeft);
		CryVertexBinding& rRightBind = m_pGeom->getLink(nRight);
		const CryLink& rLeftLink = rLeftBind[0];
		const CryLink& rRightLink = rRightBind[0];
		if (rLeftBind.size() == 1 && rRightBind.size() > 1)
			return true;
		if (rLeftBind.size() > 1 && rRightBind.size() == 1)
			return false;
		if (rLeftLink.BoneID < rRightLink.BoneID)
			return true;
		if (rLeftLink.BoneID > rRightLink.BoneID)
			return false;
		return rLeftLink.Blending < rRightLink.Blending;
	}
protected:
	CryGeometryInfo* m_pGeom;
};

// optimize the vertex order for skinning
// WARNING: will interfere with morph targets
void CryGeometryInfo::sortVertices()
{
	// sort the vertices by the most influential bone index
	CSkinVertexSorter sorter (this);
	TElementaryArray<unsigned> arrMap, arrUnmap;
	arrMap.reinit (numVertices());
	unsigned i;
	for (i = 0; i < numVertices(); ++i)
		arrMap[i] = i;
	std::sort (arrMap.begin(), arrMap.begin() + numVertices(), sorter);
	
	arrUnmap.reinit (numVertices());
	for (i = 0; i < numVertices(); ++i)
		arrUnmap[arrMap[i]] = i;

	// swap vertices and normals
	// face vertex indices and links
	Vec3dElementaryArray arrNewVertices("sortVertices"), arrNewNormals("sortVertices");
	arrNewVertices.reinit (numVertices());
	arrNewNormals.reinit (numVertices());

	VertexBindingArray arrNewLinks ("sortVertices");
	arrNewLinks.reinit (numVertices());

	// sort the vertices, normals and links
	for (i = 0; i < numVertices(); ++i)
	{
		arrNewVertices[i] = m_arrVertices[arrMap[i]];
		arrNewNormals[i]  = m_arrNormals[arrMap[i]];
		arrNewLinks[i]    = m_arrLinks[arrMap[i]];
	}

	m_arrVertices.swap (arrNewVertices);
	m_arrNormals.swap (arrNewNormals);
	m_arrLinks.swap (arrNewLinks);

	for (unsigned nFace = 0; nFace < numFaces(); ++nFace)
	{
		GeomFace& face = m_arrFaces[nFace];
		for (int v = 0; v < 3; ++v)
			face[v] = arrUnmap[face[v]];
	}
}



void CryGeometryInfo::buildTangSkins (const CryBoneInfo* pBoneInfo, unsigned numBoneInfos)
{
	CrySkinVertexSource VertexSource(this);
	// form the array of inverse-default-global matrices for the bones
	std::vector<Matrix44> arrMatInvDef;
	arrMatInvDef.resize (numBoneInfos);
	for (unsigned i = 0; i < numBoneInfos; ++i)
		arrMatInvDef[i] = pBoneInfo[i].getInvDefGlobal();
	CrySkinBasisBuilder builder (&VertexSource, &arrMatInvDef[0], numBoneInfos);
	builder.setDestinationInterval(0,0xFFFFFFFF);
	builder.initRigidBasisSkin (&m_TangSkin);
}



// the tangent bases are calculated in pieces, each piece calculating some number of vertices
// the piece
class CrySkinRigidBasis* CryGeometryInfo::getTangSkin()
{
	return &m_TangSkin;
}


// after constructing the skins etc. some data is not ever more used
// this data can be cleared here. This should be called after all the initialization has passed
void CryGeometryInfo::clearConstructionData()
{
	if (!g_GetCVars()->ca_ZDeleteConstructionData())
		return;
	m_arrNormals.clear();
	m_arrUVs.clear();
	m_arrTexFaces.clear();

	// we may need faces for decals and stencil shadows

	if (!m_arrLinks.empty())
	{
		// if we don't have links, therefore we don't have bones, and so
		// we need the vertices for morphing etc.
		// The same applies to tangents
		m_arrLinks.clear();
		if (g_GetCVars()->ca_EnableTangentSkinning())
			m_arrExtTangents.clear();
#ifdef _DEBUG
		if (!g_GetCVars()->ca_DebugRebuildShadowVolumes())
#endif
		if (m_pStencilShadowConnectivity) // if there's no shadow connectivity yet, we'll need vertices for construction (HACK: we could get rid of it if we didn't split the vertices because of different normals per one vertex)
			m_arrVertices.clear();
	}
	m_arrIntToExtMap.clear();
}

// this is the number of vertices that are really used by the faces
unsigned CryGeometryInfo::numUsedVertices()const
{
	return m_numUsedVertices;
}

bool CryGeometryInfo::hasSkin()const
{
	return !m_TangSkin.empty();
}


void CryGeometryInfo::GetSize (ICrySizer* pSizer)const
{
	SIZER_SUBCOMPONENT_NAME(pSizer, "Geometry");
	unsigned nSize = sizeof(*this);
	unsigned i;

	nSize += sizeofArray (m_arrExtTangents);
	nSize += sizeofArray (m_arrExtToIntMap);
	nSize += sizeofArray (m_arrExtUVs, numExtUVs());
	nSize += sizeofArray (m_arrIntToExtMap, numVertices());
	nSize += sizeofArray (m_arrPrimGroups);
	nSize += sizeofArray (m_arrIndices);
	nSize += sizeofArray (m_arrFaces);
	nSize += sizeofArray (m_arrVColors);

	if (!m_arrLinks.empty())
	{
		nSize += sizeofArray (m_arrLinks, numLinks());
		for (i = 0; i < numLinks(); ++i)
			nSize += sizeofArray(m_arrLinks[i]);
	}

	nSize += sizeofArray (m_arrTexFaces, numTexFaces());
	nSize += sizeofArray (m_arrUVs);
	nSize += sizeofArray (m_arrVertices, numVertices());
	nSize += sizeofArray (m_arrNormals, numVertices());
	
	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "Meshes");
		if (!pSizer->AddObject (this, nSize))
			return;
	}

	{
		// calculate the skin memory separately, don't take the objects into accout
		// since they were already calculated in the sizeof(*this) above
		SIZER_SUBCOMPONENT_NAME(pSizer, "Skins");
		nSize = m_TangSkin.sizeofThis() - sizeof(m_TangSkin);
		nSize += m_SkinGeom.sizeofThis() - sizeof(m_SkinGeom);
		nSize += m_SkinNormal.sizeofThis() - sizeof(m_SkinNormal);
		// NOTE: if this call will lead to assert, we should find some other id for these subcomponents
		pSizer->AddObject(&m_TangSkin, nSize);
	}

	if (m_pStencilShadowConnectivity)
	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "Shadow Connectivity");
		m_pStencilShadowConnectivity->GetMemoryUsage(pSizer);
	}

	if (!m_arrVertBuf.empty())
	{
		SIZER_SUBCOMPONENT_NAME(pSizer, "System Buffers");
		pSizer->Add (&m_arrVertBuf[0], numExtVertices()*m_VertexSize[m_nVertBufFormat]);
	}
}


// returns the vertex buffer of the given format; in this buffer, UVs will
// already be set up properly and extra fields like color will be NULLed
// Will clean the buffer and return NULL when the vertex format is < 0
char* CryGeometryInfo::getVertBuf (int nVertFormat)
{
	if (nVertFormat == m_nVertBufFormat)
		return &m_arrVertBuf[0];

	// clean the buffer and return NULL if invalid vertex format
	if (nVertFormat <= 0 || nVertFormat >= VERTEX_FORMAT_NUMS)
	{
		m_arrVertBuf.clear();
		return NULL;
	}

	m_nVertBufFormat = nVertFormat;
	unsigned nVertSize = m_VertexSize[nVertFormat];
	m_arrVertBuf.reinit (nVertSize * numExtVertices());

	// zero the memory for some formats
	switch (nVertFormat)
	{
	case VERTEX_FORMAT_P3F:
	case VERTEX_FORMAT_P3F_TEX2F:
		// no need to zero since the format is tightly packed and we'll fill it
		// with actual data
		break;
	default:
		memset (m_arrVertBuf.begin(), 0, nVertSize*numExtVertices());
		break;
	}

	// fill the UVs in
	int nUVOffset = g_VertFormatUVOffsets[nVertFormat];
	if (nUVOffset >= 0)
	{
		for (unsigned i = 0; i < numExtVertices(); ++i)
			*(CryUV*)(m_arrVertBuf.begin() + nUVOffset + nVertSize * i) = getExtUV (i);
	}

	int nColorOffset = g_VertFormatRGBAOffsets[nVertFormat];
	if (nColorOffset >= 0)
	{
		unsigned i;
		if (m_arrVColors.empty() || m_arrExtToIntMap.empty() || !g_GetCVars()->ca_EnableVertexColors())
			for (i = 0; i < numExtVertices(); ++i)
				*(DWORD*)(m_arrVertBuf.begin() + nColorOffset + nVertSize * i) = 0xFFFFFFFF;
		else
			for (i = 0; i < numExtVertices(); ++i)
				*(DWORD*)(m_arrVertBuf.begin() + nColorOffset + nVertSize * i) = m_arrVColors[m_arrExtToIntMap[i]];
	}

	return m_arrVertBuf.begin();
}

/*
#ifdef _DEBUG
void CryGeometryInfo::selfValidate()
{
	for (unsigned i = 0; i < numNormals(); ++i)
		assert (getNormal(i).Length2() > 0.9);
}
#endif
*/

// initializes the ext-to-int map
void CryGeometryInfo::initExtToIntMap (const unsigned short* pExtToIntMap, unsigned numEntries)
{
	m_arrExtToIntMap.resize (numEntries);
	for (unsigned i = 0; i < numEntries; ++i)
		m_arrExtToIntMap[i] = pExtToIntMap[i];
}

// creates and initializes the array of extern UVs
void CryGeometryInfo::initExtUVs (const CryUV* pUVs, unsigned numUVs)
{
	m_arrExtUVs.reinit (numUVs);
	memcpy (&m_arrExtUVs[0], pUVs, sizeof(CryUV)*numUVs);
}

// creates and copies the data into the new face array
void CryGeometryInfo::initFaces (unsigned numFaces, const CCFIntFace* pFaces, const void* pFaceMtls)
{
	unsigned nFace;
	m_arrFaces.resize (numFaces);
	for (nFace = 0; nFace < numFaces; ++nFace)
		m_arrFaces[nFace] = pFaces[nFace];

	m_arrFaceMtl.resize (numFaces);
	for (nFace = 0; nFace < numFaces; ++nFace)
		m_arrFaceMtl[nFace] = ((const CCFIntFaceMtlID*)pFaceMtls)[nFace];
}

void CryGeometryInfo::exportASC (FILE* f)
{
	// vertices of the geometry (first morphed, then skinned)
	const CryUV* pExtUVs = getExtUVs();
	TangData * pExtTangents = getExtTangents();
	unsigned numExtTangents = this->numExtTangents();
	Vec3d* pVerts = getVertices();
	if (!pVerts || !pExtTangents)
	{
		fprintf (f, "Error: no vertices or ext tangents. Please restart with ca_ZDeleteConstructionData = 0, don't use stencil shadows and compiled cgfs\n");
		return;
	}
	if (!pExtUVs)
	{
		fprintf (f, "No externals UVs. Cannot export.\n");
		return;
	}

	fprintf (f, "Vertex \t binormal \t tangent \t normal \t UV\n");
	for (unsigned i = 0; i < numExtTangents; ++i)
	{
#define V(v) v.x, v.y, v.z

		fprintf (f, "%12.9ff,%12.9ff,%12.9ff, %12.9ff,%12.9ff,%12.9ff, %12.9ff,%12.9ff,%12.9ff, %12.9ff,%12.9ff,%12.9ff, %12.9ff,%12.9ff,  //0x%04x \n",
			V(pVerts[m_arrExtToIntMap[i]]), V(pExtTangents[i].tangent), V(pExtTangents[i].binormal),  V(pExtTangents[i].tnormal), pExtUVs[i].u, pExtUVs[i].v, i );

#undef V
	}
}

// vlad: I placed it temporary here because of Timur chandes in rc.exe
CIndexedMesh::~CIndexedMesh()
{
	if(m_pFaces)
		free(m_pFaces);
	if(m_pVerts)
		free(m_pVerts);
	if(m_pCoors)
		free(m_pCoors);
	if(m_pNorms)
		free(m_pNorms);
	if(m_pColor)
		free(m_pColor);

	for(int g=0;  g<m_lstGeomNames.Count(); g++)
		delete [] m_lstGeomNames[g];
	int i;
	for (i=0; i<m_lstMatTable.Count(); i++)
	{
		CMatInfo *mi = &m_lstMatTable[i];
		if (mi->pRE)
			mi->pRE->Release();
	}
	for (i=0; i<m_lstLSources.Count(); i++)
	{
		/*      if (m_lstLSources[i]->m_Name)
		delete [] m_lstLSources[i]->m_Name;
		if (m_lstLSources[i]->m_TargetName)
		delete [] m_lstLSources[i]->m_TargetName;
		if(m_lstLSources[i]->m_OrigLight)
		delete m_lstLSources[i]->m_OrigLight;*/
		delete m_lstLSources[i];
	}
	// We don't need to release target light sources because they're released before (during releasing of main lights in destructor)
	/*for (i=0; i<m_tgtLSources.Count(); i++)
	{
	if (m_tgtLSources[i]->m_Name)
	delete [] m_tgtLSources[i]->m_Name;
	if (m_tgtLSources[i]->m_TargetName)
	delete [] m_tgtLSources[i]->m_TargetName;
	delete m_tgtLSources[i];
	}*/
}
