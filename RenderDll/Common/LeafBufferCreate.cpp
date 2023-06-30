////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Make vert buffers
//
////////////////////////////////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"
#include "MeshIdx.h"

#include "float.h"
#include "i3dengine.h"

#include <CryCompiledFile.h>
#include <VertexBufferSource.h>

CLeafBuffer CLeafBuffer::m_Root("Root");
CLeafBuffer CLeafBuffer::m_RootGlobal("RootGlobal");

void CLeafBuffer::CopyTo(CLeafBuffer *pDst, bool bUseSysBuf)
{
	list2<CMatInfo>& arrSrcMats = *(m_pMats);
  list2<CMatInfo>& arrNewMats = *(pDst->m_pMats = new list2<CMatInfo>);
  pDst->m_bMaterialsWasCreatedInRenderer  = true;
	arrNewMats.resize (arrSrcMats.Size());
	unsigned i;
  for (i = 0; i < arrSrcMats.size(); ++i)
  {
		CMatInfo& rSrcMat = arrSrcMats[i]; 
    CMatInfo& rNewMat = arrNewMats[i];
		rNewMat = rSrcMat;
    SShaderItem Sh = rNewMat.GetShaderItem();
    if (Sh.m_pShader)
      Sh.m_pShader->AddRef();
    if (Sh.m_pShaderResources)
      Sh.m_pShaderResources->AddRef();
    rNewMat.m_pPrimitiveGroups = NULL;
    CREOcLeaf *re = rSrcMat.pRE;
    if (re)
    {
      rNewMat.pRE = (CREOcLeaf *)gRenDev->EF_CreateRE(eDATA_OcLeaf);
      CRendElement *pNext = rNewMat.pRE->m_NextGlobal;
      CRendElement *pPrev = rNewMat.pRE->m_PrevGlobal;
      *rNewMat.pRE = *re;
      rNewMat.pRE->m_NextGlobal = pNext;
      rNewMat.pRE->m_PrevGlobal = pPrev;
      rNewMat.pRE->m_LIndicies = NULL;
      rNewMat.pRE->m_Faces = NULL;
      rNewMat.pRE->m_pBuffer = pDst;
		}
  }
  pDst->m_Indices.Reset();
  pDst->m_SecIndices.Copy(m_SecIndices);
  pDst->m_NumIndices = m_NumIndices;
  pDst->InvalidateVideoBuffer(-1);
	pDst->m_SecVertCount = m_SecVertCount;
  if (!bUseSysBuf)
    pDst->m_bOnlyVideoBuffer=true;
  else
  {
    pDst->AllocateSystemBuffer(m_SecVertCount);
		CVertexBuffer* pSecVertBuffer  = pDst->m_pSecVertBuffer;
    void * pTmp = pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
    void * pTmpTangs = pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
    cryMemcpy(pSecVertBuffer, m_pSecVertBuffer, sizeof(*pSecVertBuffer));
    pSecVertBuffer->m_VS[VSF_GENERAL].m_VData=pTmp;
    pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData=pTmpTangs;
    cryMemcpy(pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, m_VertexSize[m_pSecVertBuffer->m_vertexformat]*m_SecVertCount);
    pDst->CalcFaceNormals();
  }
  
  if (CRenderer::CV_r_precachemesh)
  {
    int nVertFormat = -1;
    int Flags = 0;
	  for (int i=0; i<(*pDst->m_pMats).Count(); i++)
	  {
		  CMatInfo *mi = &(*pDst->m_pMats)[i];
		  if (!mi->pRE)
			  continue;
      SShaderItem Sh = mi->GetShaderItem();
      if (Sh.m_pShader)
      {
        IShader *pSH = Sh.m_pShader->GetTemplate(-1);
        int nShVertFormat = pSH->GetVertexFormat();
        if (nVertFormat < 0)
          nVertFormat = nShVertFormat;
        else
          nVertFormat = gRenDev->m_RP.m_VFormatsMerge[nShVertFormat][nVertFormat];
        if (pSH->GetFlags() & EF_NEEDTANGENTS)
          Flags |= SHPF_TANGENTS;
      }
    }
    if (nVertFormat >= 0)
      pDst->CheckUpdate(nVertFormat, Flags, false);
  }
}

void CLeafBuffer::AllocateSystemBuffer( int nVertCount )
{
  m_pSecVertBuffer = new CVertexBuffer;
  m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[nVertCount];
}

bool CLeafBuffer::UpdateTangBuffer(SPipTangents *pBasis)
{
	SAFE_DELETE_ARRAY(m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData);
	if (!pBasis)
		return false;

	m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData = new SPipTangents[m_SecVertCount];
	SPipTangents *tn = (SPipTangents *)m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
	for (int i=0; i<m_SecVertCount; i++)
	{
		tn[i] = pBasis[i];
	}

	return true;
}

//===================================================================

#include "TangentSpaceCalculation.h"

class CMeshInputProxy
{

  // helper to get order for CVertexLoadHelper
  struct NormalCompare: public std::binary_function<Vec3, Vec3, bool>
  {
    bool operator() ( const Vec3 &a, const Vec3 &b ) const
    {
      // first sort by x
      if(a.x<b.x)
        return(true);
      if(a.x>b.x)
        return(false);

      // then by y
      if(a.y<b.y)
        return(true);
      if(a.y>b.y)
        return(false);

      // then by z
      if(a.z<b.z)
        return(true);
      if(a.z>b.z)
        return(false);

      return(false);
    }
  };

public:

  //! constructor
  //! /param inpMesh must not be 0
  CMeshInputProxy(CIndexedMesh &inData)
  {
    m_pData = &inData;

    // remap the normals (weld them)
    DWORD dwFaceCount = GetTriangleCount();

    m_NormIndx.reserve(dwFaceCount);

    std::map<Vec3,DWORD,NormalCompare>  mapNormalsToNumber;
    DWORD dwmapSize=0;

    // for every triangle
    for(DWORD i=0;i<dwFaceCount;i++)
    {
      CTriNormIndex idx;

      // for every vertex of the triangle
      for(DWORD e=0;e<3;e++)
      {
        int iNorm = m_pData->m_pFaces[i].n[e];
        Vec3 &vNorm = m_pData->m_pNorms[iNorm];

        std::map<Vec3,DWORD,NormalCompare>::iterator iFind = mapNormalsToNumber.find(vNorm);

        if(iFind == mapNormalsToNumber.end())       // not found
        {
          idx.p[e] = dwmapSize;
          mapNormalsToNumber[vNorm] = dwmapSize;
          dwmapSize++;
        }
        else
          idx.p[e] = (*iFind).second;
      }

      m_NormIndx.push_back(idx);
    }
  }

  //! /return 0..
  DWORD GetTriangleCount( void ) const
  {
    return m_pData->m_nFaceCount;
  }

  //! /param indwTriNo 0..
  //! /param outdwPos
  //! /param outdwNorm
  //! /param outdwUV
  void GetTriangleIndices( const DWORD indwTriNo, DWORD outdwPos[3], DWORD outdwNorm[3], DWORD outdwUV[3] ) const
  {
    const ushort *pIndsP = &m_pData->m_pFaces[indwTriNo].v[0];
    const ushort *pIndsUV = &m_pData->m_pFaces[indwTriNo].t[0];
    const CTriNormIndex &norm = m_NormIndx[indwTriNo];

    for(int i=0; i<3; i++)
    {
      outdwPos[i] = pIndsP[i];
      outdwUV[i] = pIndsUV[i];
      outdwNorm[i] = norm.p[i];
    }
  }

  //! /param indwPos 0..
  //! /param outfPos
  void GetPos( const DWORD indwPos, float outfPos[3] ) const
  {
    assert(indwPos < m_pData->m_nVertCount);
    Vec3 &ref = m_pData->m_pVerts[indwPos];
    outfPos[0] = ref.x;
    outfPos[1] = ref.y;
    outfPos[2] = ref.z;
  }

  //! /param indwPos 0..
  //! /param outfUV 
  void GetUV( const DWORD indwPos, float outfUV[2] ) const
  {
		assert(indwPos < m_pData->m_nCoorCount);
    if(indwPos < m_pData->m_nCoorCount)
		{
			TexCoord &ref = m_pData->m_pCoors[indwPos];
			outfUV[0] = ref.s;
			outfUV[1] = ref.t;
		}
		else
		{
			outfUV[0] = 0;
			outfUV[1] = 0;
		}
  }

private:

  class CTriNormIndex
  {
  public:
    DWORD p[3];                          //!< index in m_BaseVectors
  };

  std::vector<CTriNormIndex>  m_NormIndx;     //!< normal indices for each triangle
  CIndexedMesh *m_pData;   //!< must not be 0
};

struct SBasisFace
{
  ushort v[3];
};

//////////////////////////////////////////////////////////////////////////
// Creates the leaf buffer out of the given indexed mesh.
// IMPLEMENTATION:
// . Sort|Group faces by materials
// . Create vertex buffer with sequence of (possibly non-unique) vertices, 3 verts per face
// . Make Render ELements for each material
// . For each (non-unique) vertex calculate the tangent base
// . Index the mesh (Compact Vertices): detect and delete duplicate vertices
// . Remove degenerated triangles in the generated mesh (GetIndices())
// . Sort vertices and indices for GPU cache
void CLeafBuffer::CreateBuffer( CIndexedMesh * pTriData, bool bStripifyAndShareVerts, bool bRemoveNoDrawFaces, bool bKeepRemapTable)
{
	int max_vert_num = pTriData->m_nFaceCount*3;

  //bStripifyAndShareVerts = true;

	int i;
	struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * pVBuff = new struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F[max_vert_num];
  SPipTangents *pTBuff = new SPipTangents[max_vert_num];
	int buff_vert_count = 0;
  uint *uiInfo = new uint[max_vert_num];
  m_nVertexFormat = VERTEX_FORMAT_P3F_N_COL4UB_TEX2F;

	uint *piVtxIdx = 0; // [Anton] need this to build the mapping table leafbuffer vtx idx->original vtx idx
	if (bKeepRemapTable)
		piVtxIdx = new uint[max_vert_num];
	m_arrVtxMap = 0;

	// . Sort|Group faces by materials
	// For each shader (designated by shader_id of an element of m_pFaces)
	// there is one list2 in this table. In this list, there will be a sorted
	// array of faces belonging to this shader
	list2<CObjFace*>   _vhash_table[512];
	list2<SBasisFace> _thash_table[512];
  bool bShareVertsArr[512];
  SPipTangents *pBasises = NULL;
  ushort *pBasisIndices = NULL;

  {
    // Generate tangent basis vectors before indexing per-material
    CMeshInputProxy Input(*pTriData);
    CTangentSpaceCalculation<CMeshInputProxy> tangents;

    // calculate the base matrices
    tangents.CalculateTangentSpace(Input);

    DWORD dwCnt = tangents.GetBaseCount();
    // for every triangle
    DWORD dwTris = Input.GetTriangleCount();

    pBasises = new SPipTangents[dwCnt];
    pBasisIndices = new ushort[dwTris*3];

    for(DWORD dwTri=0; dwTri<dwTris; dwTri++)
    {
      DWORD dwBaseIndx[3];

      tangents.GetTriangleBaseIndices(dwTri, dwBaseIndx);

      assert(dwBaseIndx[0]<dwCnt);
      assert(dwBaseIndx[1]<dwCnt);
      assert(dwBaseIndx[2]<dwCnt);

      // for every vertex of the triangle
      for(i=0; i<3; i++)
      {
        pBasisIndices[dwTri*3+i] = dwBaseIndx[i];		// set the base vector
      }
    }

    //
    for(i=0; i<dwCnt; i++)
    {
      tangents.GetBase(i, &pBasises[i].m_Tangent[0], &pBasises[i].m_Binormal[0], &pBasises[i].m_TNormal[0]);
    }
  }

  static int a=0,b=0;

	{ // fill the table: one list of faces per one shader
		for(int i=0; i<pTriData->m_nFaceCount; i++)
		{
			CObjFace *pFace =  &pTriData->m_pFaces[i];   

      IShader *ef = (m_pMats->GetAt(pFace->shader_id).shaderItem.m_pShader)->GetTemplate(-1);
      a++;
      if(ef->GetFlags3() & EF3_NODRAW)
      {
        b++;
        if(bRemoveNoDrawFaces)
          continue;
      }
	    
			assert(pFace->shader_id>=0 && pFace->shader_id<512);
	    
			if(pFace->shader_id>=m_pMats->Count())
			{
				pFace->shader_id=0;
				iLog->Log("CLeafBuffer::CreateBuffer shader_id of face is out of range");
			}
      SBasisFace fc;
      ushort *v = &pBasisIndices[i*3];
      fc.v[0] = v[0];
      fc.v[1] = v[1];
      fc.v[2] = v[2];

			_vhash_table[pFace->shader_id].Add(pFace);
			_thash_table[pFace->shader_id].Add(fc);
		}
	}

//  m_pDoubleSideLighting = new uchar[max_vert_num];
	
	// . Create vertex buffer with sequence of (possibly non-unique) vertices, 3 verts per face
	// for each shader..
	for (int t=0; t<m_pMats->Count(); t++) 
	{
		// memorize the starting index of this material's face range
		(*m_pMats)[t].nFirstIndexId = buff_vert_count;

		// scan through all the faces using the shader #t
		for(int i=0; i<_vhash_table[t].Count(); ++i)
		{
			CObjFace *pFace = _vhash_table[t][i];
      SBasisFace *pTFace = &_thash_table[t][i];
	    
			assert(pFace->shader_id == t);

			for (int v=0; v<3; ++v)
			{
        pTBuff[buff_vert_count] = pBasises[pTFace->v[v]];

				if(pTriData->m_pColor)
				{ // if color exported - copy from pTriData

					pVBuff[buff_vert_count].color.bcolor[0] = pTriData->m_pColor[pFace->v[v]].r;
					pVBuff[buff_vert_count].color.bcolor[1] = pTriData->m_pColor[pFace->v[v]].g;
					pVBuff[buff_vert_count].color.bcolor[2] = pTriData->m_pColor[pFace->v[v]].b;
					pVBuff[buff_vert_count].color.bcolor[3] = pTriData->m_pColor[pFace->v[v]].a;
				}
				else
				{
					pVBuff[buff_vert_count].color.dcolor = -1;
				}
	  
				// base tex coord
				int tid = pFace->t[v];
				if(tid>=0 && tid<pTriData->m_nCoorCount)
				{
					pVBuff[buff_vert_count].st[0] = pTriData->m_pCoors[pFace->t[v]].s;
					pVBuff[buff_vert_count].st[1] = pTriData->m_pCoors[pFace->t[v]].t;
				}
				else
				{ 
					pVBuff[buff_vert_count].st[0] = 0;
          pVBuff[buff_vert_count].st[1] = 0;
				}						

				// normal
				assert(pTriData->m_pNorms);
				pVBuff[buff_vert_count].normal = pTriData->m_pNorms[pFace->n[v]];
				uiInfo[buff_vert_count] = 0;

				// position
				pVBuff[buff_vert_count].xyz = pTriData->m_pVerts[pFace->v[v]];
				
				// store shader id to prevent vertex sharing between materials during recompacting
				uiInfo[buff_vert_count] |= pFace->shader_id;

				// [Anton] keep index list to build mapping table later
				if (piVtxIdx)
					piVtxIdx[buff_vert_count] = pFace->v[v];

				// store cull flag for UpdateCustomLighting()
				// removed to make it work as in resource compiler
/*				IShader * ef = (m_pMats->GetAt(pFace->shader_id).shaderItem.m_pShader)->GetTemplate(-1);
        bool bTwoSided =  ef && (ef->GetCull() == e CULL_None);
				uiInfo[buff_vert_count] |= bTwoSided << 8;
        bShareVertsArr[pFace->shader_id] = ((ef->GetFlags3() & EF3_SHAREVERTS) != 0);
        if (!bShareVertsArr[pFace->shader_id])*/
          bShareVertsArr[pFace->shader_id] = bStripifyAndShareVerts;
	      
				buff_vert_count++;
			}
		}

		// there are faces belonging to this material(shader) #t, if number of indices > 0
		(*m_pMats)[t].nNumIndices = buff_vert_count - (*m_pMats)[t].nFirstIndexId;
		_vhash_table[t].Reset();
		_thash_table[t].Reset();
	}

	// make REs
	for (i=0; i<(*m_pMats).Count(); i++)
	{		
		if((*m_pMats)[i].nNumIndices)
		{
			CREOcLeaf *re = (CREOcLeaf *)gRenDev->EF_CreateRE(eDATA_OcLeaf);
			re->m_pChunk = &(*m_pMats)[i];
			re->m_pBuffer = this;

			//assert (re->m_pChunk->nNumIndices < 60000);

			re->m_pChunk->pRE = re;

			// always enable sharing if there is 'flareproc' in shader/material name
			if (!bShareVertsArr[i] && (*m_pMats)[i].nNumIndices == 6)
			{
				IShader * ef = (*m_pMats)[i].shaderItem.m_pShader->GetTemplate(-1);
				char nameSh[128];
				strncpy(nameSh, ef->GetTemplate(-1)->GetName(),sizeof(nameSh));
				strlwr(nameSh);
				bShareVertsArr[i] = strstr(nameSh, "flareproc")!=0;
			}

//      IShader * ef = (*m_pMats)[i].shaderItem.m_pShader->GetTemplate(-1);
  //    bool bTwoSided =  ef && (ef->GetCull() == eCULL_None);
			bool bTwoSided =  ((*m_pMats)[i].shaderItem.m_pShaderResources->m_ResFlags & MTLFLAG_2SIDED)!=0;

      re->m_SortId = i + 2*(!bTwoSided); // render double sided leafs last
		}
	}


	// Index the mesh (Compact Vertices): detect and delete duplicate vertices

	CompactBuffer(pVBuff, pTBuff, &buff_vert_count, &m_SecIndices, bShareVertsArr, uiInfo);

  SAFE_DELETE_ARRAY (uiInfo);
  SAFE_DELETE_ARRAY (pBasises);
  SAFE_DELETE_ARRAY (pBasisIndices);

  /*if (buff_vert_count == 12288)
  {
    FILE *fp = fopen("verts.fur", "r");
    int nVerts = 0;
    while (true)
    {
      Vec3 pos, norm;
      float tc[2];
      int n = fscanf(fp, "%f %f %f %f %f %f %f %f\n", &pos.x, &pos.y, &pos.z, &norm.x, &norm.y, &norm.z, &tc[0], &tc[1]);
      if (n != 8)
        break;
      pVBuff[nVerts].xyz = pos;
      pVBuff[nVerts].normal = norm;
      pVBuff[nVerts].st[0] = tc[0];
      pVBuff[nVerts].st[1] = tc[1];
      nVerts++;
    }
    fclose(fp);
    buff_vert_count = nVerts;

    fp = fopen("inds.fur", "r");
    int nInds = 0;
    m_SecIndices.Free();
    while (true)
    {
      int ind;
      int n = fscanf(fp, "%d\n", &ind);
      if (n != 1)
        break;
      m_SecIndices.AddElem(ind);
      nInds++;
    }
    fclose(fp);
  	CMatInfo *mi = &(*m_pMats)[0];
    mi->nFirstIndexId = 0;
    mi->nNumIndices = nInds;
    mi->nFirstVertId = 0;
    mi->nNumVerts = nVerts;
  }*/

	for( i=0; i<m_SecIndices.Num(); i++ )
	{
		if(m_SecIndices[i]<buff_vert_count)
			continue;

		iConsole->Exit("CLeafBuffer::CreateBuffer: Indices out of range");
	}

	if (bKeepRemapTable) // [Anton] build the mapping table leaf buffer vertex index -> original vertex index
	{
		m_arrVtxMap = new uint[buff_vert_count];
		for( i=0; i<m_SecIndices.Num(); i++ )
			m_arrVtxMap[m_SecIndices[i]] = piVtxIdx[i];
		delete[] piVtxIdx;
	}

  m_Indices.m_bDynamic = m_bDynamic;

	if(buff_vert_count>65535)
		iConsole->Exit("CLeafBuffer::CreateBuffer: Number of vertices in object is more than 65535");

	// . Remove degenerated triangles in the generated mesh (GetIndices())
	// FIXME: For some reason this optimization doesn't work for Animated objects (Assertion in CryModelState::GenerateRenderArrays)
	if (!m_sSource || strcmp(m_sSource, "CryModelArray") != 0)
	{
		// Remove degenerated triangles
    TArray<ushort> NewIndexes;
		for (i=0; i<(*m_pMats).Count(); i++)	// each material..
		{
			CMatInfo *mi = &(*m_pMats)[i];
			if (!mi->pRE)
				continue;
			int nFirstInd = NewIndexes.Num();
			for (int j=mi->nFirstIndexId; j<mi->nFirstIndexId+mi->nNumIndices; j+=3)
			{
				// the face in material #i consists of vertices i0,i1,i2:
				int i0 = m_SecIndices[j+0];
				int i1 = m_SecIndices[j+1];
				int i2 = m_SecIndices[j+2];
        assert (i0<65536 && i1<65536 && i2<65536);
				// if the face is not degenerated, then add it; otherwise skip and finally it'll be deleted
				if (i0!=i1 && i0!=i2 && i1!=i2)
				{
					NewIndexes.AddElem(i0);
					NewIndexes.AddElem(i1);
					NewIndexes.AddElem(i2);
				}
			}
			mi->nFirstIndexId = nFirstInd;
			mi->nNumIndices = NewIndexes.Num() - nFirstInd;
			if (!mi->nNumIndices)
			{
				mi->pRE->Release();
				mi->pRE = NULL;
			}
		}
    m_SecIndices.Free();
    m_SecIndices.Copy(NewIndexes);
    m_NumIndices = m_SecIndices.Num();
		NewIndexes.Free();
	}
  else
    m_NumIndices = m_SecIndices.Num();
  InvalidateVideoBuffer(-1);

  // Lock the index buffer and get the pointer
  ushort *pInds = GetIndices(NULL);

  int nVertFormat = -1;
  int nFlags = 0;

	// . Find vertex range (both index and spacial ranges) for each material (needed for rendering)
	for (i=0; i<(*m_pMats).Count(); i++)
	{
		CMatInfo *mi = &(*m_pMats)[i];
		if (!mi->pRE)
			continue;
		if (mi->nNumIndices+mi->nFirstIndexId > m_NumIndices)
		{
      assert(0);
      continue;
    }
    SShaderItem Sh = mi->GetShaderItem();
    if (Sh.m_pShader)
    {
      IShader *pSH = Sh.m_pShader->GetTemplate(-1);
      int nShVertFormat = pSH->GetVertexFormat();
      if (nVertFormat < 0)
        nVertFormat = nShVertFormat;
      else
        nVertFormat = gRenDev->m_RP.m_VFormatsMerge[nShVertFormat][nVertFormat];
      if ((pSH->GetFlags() & EF_NEEDTANGENTS) && !(pSH->GetFlags3() & EF3_HASVCOLORS))
        nFlags |= SHPF_TANGENTS;
    }
		int nMin =  999999;
		int nMax = -999999;
		Vec3d vMin;
		Vec3d vMax;
		vMin=SetMaxBB();
		vMax=SetMinBB();
		for (int j=mi->nFirstIndexId; j<mi->nNumIndices+mi->nFirstIndexId; j++)
		{
			int ind = pInds[j];
			struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *pV = &pVBuff[ind];
			Vec3d v = pV->xyz;
			vMin.CheckMin(v);
			vMax.CheckMax(v);
			nMin = min(nMin, ind);
			nMax = max(nMax, ind);
		}
		mi->m_vCenter = (vMin + vMax) * 0.5f;
		mi->m_fRadius = (vMin - mi->m_vCenter).GetLength();
		mi->nFirstVertId = nMin;
		mi->nNumVerts = nMax-nMin+1;
    if (mi->nNumVerts <= 1000 && !m_bOnlyVideoBuffer)
    {
      mi->pRE->mfUpdateFlags(FCEF_MERGABLE);
    }
	}
  {
    m_fMinU = m_fMinV = 9999;
    m_fMaxU = m_fMaxV = -9999;
    for (int i=0; i<buff_vert_count; i++)
    {
      if (pVBuff[i].st[0] < m_fMinU)
        m_fMinU = pVBuff[i].st[0];
      if (pVBuff[i].st[0] > m_fMaxU)
        m_fMaxU = pVBuff[i].st[0];
      if (pVBuff[i].st[1] < m_fMinV)
        m_fMinV = pVBuff[i].st[1];
      if (pVBuff[i].st[1] > m_fMaxV)
        m_fMaxV = pVBuff[i].st[1];
    }
  }

  /*if (bStripifyAndShareVerts)
  {
    FILE *fp = fopen("vertanim.txt", "w");
    for (i=0; i<(*m_pMats).Count(); i++)	// each material..
    {
      CMatInfo *mi = &(*m_pMats)[i];
      if (!mi->pRE)
        continue;
      fprintf(fp, "\nMaterial: %s\n", mi->shaderItem.m_pShader->GetName());
      for (int j=0; j<mi->nNumVerts; j++)
      {
        int ind = j+mi->nFirstVertId;
        struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *pV = &pVBuff[ind];
        fprintf(fp, "%.3f, %.3f, %.3f (%.3f, %.3f, %.3f) (%.3f, %.3f)\n", pV->x, pV->y, pV->z, pV->nx, pV->ny, pV->nz, pV->s, pV->t);
        for (int n=j+1; n<mi->nNumVerts; n++)
        {
          int ind = n+mi->nFirstVertId;
          struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *pV1 = &pVBuff[ind];
          if (*pV1 == *pV)
          {
            int nnn = 0;
          }
        }
      }
    }
    fclose(fp);
  }*/

	// store resulting vertex buffer in system memory
	m_SecVertCount = buff_vert_count;
	m_pSecVertBuffer = new CVertexBuffer;
	m_pSecVertBuffer->m_vertexformat = VERTEX_FORMAT_P3F_N_COL4UB_TEX2F;
	m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = new struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F[m_SecVertCount];
	cryMemcpy(m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, pVBuff, m_SecVertCount*sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F));

  m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData = new SPipTangents[m_SecVertCount];
  cryMemcpy(m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData, pTBuff, m_SecVertCount*sizeof(SPipTangents));

	SAFE_DELETE_ARRAY (pVBuff);
  SAFE_DELETE_ARRAY (pTBuff);

  SAFE_DELETE(m_pIndicesPreStrip);
	m_pIndicesPreStrip = new list2<ushort>;
  m_pIndicesPreStrip->AddList((ushort *)&m_SecIndices[0], m_SecIndices.Num());

	// . Stripify the indices
#if !defined(LINUX)
#if !defined(_DEBUG) 
	if (bStripifyAndShareVerts && !bKeepRemapTable)
    StripifyMesh(STRIPTYPE_ONLYLISTS);
#endif
#endif//LINUX

  CalcFaceNormals();
	//CreateTangBuffer();

	// if colors was loaded - remember for later use
	if(pTriData->m_pColor)
	{
		struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * pSecBuff = (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *)m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
		m_pLoadedColors = new Vec3d[m_SecVertCount];
		for(int i=0; i<m_SecVertCount; i++)
		{
			m_pLoadedColors[i].x = pSecBuff[i].color.bcolor[0];
			m_pLoadedColors[i].y = pSecBuff[i].color.bcolor[1];
			m_pLoadedColors[i].z = pSecBuff[i].color.bcolor[2];
		}
	}

  if (nVertFormat >= 0 && CRenderer::CV_r_precachemesh)
    CheckUpdate(nVertFormat, nFlags, false);

  m_vBoxMin = pTriData->m_vBoxMin;
	m_vBoxMax = pTriData->m_vBoxMax;
}

//================================================================

class CTriangleInputProxyLB
{
  // helper to get order for CVertexLoadHelper
  struct NormalCompare: public std::binary_function<Vec3, Vec3, bool>
  {
    bool operator() ( const Vec3 &a, const Vec3 &b ) const
    {
      // first sort by x
      if(a.x<b.x)
        return(true);
      if(a.x>b.x)
        return(false);

      // then by y
      if(a.y<b.y)
        return(true);
      if(a.y>b.y)
        return(false);

      // then by z
      if(a.z<b.z)
        return(true);
      if(a.z>b.z)
        return(false);

      return(false);
    }
  };

public:

  //! constructor
  //! /param inpMesh must not be 0
  CTriangleInputProxyLB(CLeafBuffer &inData)
  {
    m_pLB = &inData;

    byte *pD = (byte *)inData.m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
    SBufInfoTable *pOffs = &gBufInfoTable[inData.m_pSecVertBuffer->m_vertexformat];
    int Stride = m_VertexSize[inData.m_pSecVertBuffer->m_vertexformat]; // position stride

    // Get pointers to positions, TexCoords and Normals
    m_pInPos = (Vec3 *)pD;
    m_nStridePos = Stride;

    if (pOffs->OffsNormal)
    {
      m_pInN = (Vec3 *)&pD[pOffs->OffsNormal];
      m_nStrideN = Stride;
    }
    else
    {
      m_pInN = inData.m_TempNormals;
      m_nStrideN = sizeof(Vec3);
    }

    if (pOffs->OffsTC)
    {
      m_pInUV = (Vec2 *)&pD[pOffs->OffsTC];
      m_nStrideUV = Stride;
    }
    else
    {
      m_pInUV = (Vec2 *)inData.m_TempTexCoords;
      m_nStrideUV = sizeof(Vec2);
    }

    m_pInds = inData.GetIndices(&m_nInds);

    // remap the normals (weld them)
    DWORD dwFaceCount=GetTriangleCount();

    m_NormIndx.reserve(dwFaceCount);

    std::map<Vec3,DWORD,NormalCompare>  mapNormalsToNumber;
    DWORD dwmapSize=0;

    // for every triangle
    for(DWORD i=0;i<dwFaceCount;i++)
    {
      CTriNormIndex idx;

      // for every vertex of the triangle
      for(DWORD e=0;e<3;e++)
      {
        int iNorm = m_pInds[i*3+e];
        Vec3 &vNorm = m_pInN[iNorm];

        std::map<Vec3,DWORD,NormalCompare>::iterator iFind = mapNormalsToNumber.find(vNorm);

        if(iFind==mapNormalsToNumber.end())       // not found
        {
          idx.p[e] = dwmapSize;
          mapNormalsToNumber[vNorm] = dwmapSize;
          dwmapSize++;
        }
        else
          idx.p[e] = (*iFind).second;
      }

      m_NormIndx.push_back(idx);
    }
  }

  //! /return 0..
  DWORD GetTriangleCount( void ) const
  {
    return m_nInds / 3;
  }

  //! /param indwTriNo 0..
  //! /param outdwPos
  //! /param outdwNorm
  //! /param outdwUV
  void GetTriangleIndices( const DWORD indwTriNo, DWORD outdwPos[3], DWORD outdwNorm[3], DWORD outdwUV[3] ) const
  {
    ushort *pInds = &m_pInds[indwTriNo*3];
    const CTriNormIndex &norm = m_NormIndx[indwTriNo];

    for(int i=0;i<3;i++)
    {
      outdwPos[i] = pInds[i];
      outdwUV[i] = pInds[i];
      outdwNorm[i] = norm.p[i];
    }
  }

  //! /param indwPos 0..
  //! /param outfPos
  void GetPos( const DWORD indwPos, float outfPos[3] ) const
  {
    assert(indwPos < m_pLB->m_SecVertCount);
    byte *b = (byte *)m_pInPos+indwPos*m_nStridePos;
    Vec3 *ref = (Vec3 *)b;
    outfPos[0] = ref->x;
    outfPos[1] = ref->y;
    outfPos[2] = ref->z;
  }

  //! /param indwPos 0..
  //! /param outfUV 
  void GetUV( const DWORD indwPos, float outfUV[2] ) const
  {
    assert(indwPos < m_pLB->m_SecVertCount);
    byte *b = (byte *)m_pInUV+indwPos*m_nStrideUV;
    Vec2 *ref = (Vec2 *)b;
    outfUV[0] = ref->x;
    outfUV[1] = ref->y;
  }

private:

  class CTriNormIndex
  {
    public:
      DWORD p[3];                          //!< index in m_BaseVectors
  };

  std::vector<CTriNormIndex>  m_NormIndx;     //!< normal indices for each triangle
  CLeafBuffer *m_pLB;                  //!< must not be 0

  int m_nStrideN;
  int m_nStridePos;
  Vec3 *m_pInPos;
  int m_nStrideUV;
  int m_nInds;
public:
  Vec3 *m_pInN;
  Vec2 *m_pInUV;
  ushort *m_pInds;
};

bool CLeafBuffer::CreateTangBuffer()
{
  return false;
  assert(m_pSecVertBuffer);
  if (!m_pSecVertBuffer)
    return false;
  assert(m_SecIndices.Num());
  if (!m_SecIndices.Num())
    return false;

  int i;
  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *p = (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *)m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  CTangentSpaceCalculation<CTriangleInputProxyLB> tangents;
  CTriangleInputProxyLB Input(*this);
  if (!Input.m_pInUV)
    return false;
  if (!Input.m_pInN)
    return false;
  if (!Input.m_pInds)
    return false;

  // calculate the base matrices
  tangents.CalculateTangentSpace(Input);

  DWORD dwCnt = tangents.GetBaseCount();
  // for every triangle
  DWORD dwTris = Input.GetTriangleCount();

  SPipTangents *pBasises = new SPipTangents[dwCnt];
  ushort *pBasisIndices = new ushort[dwTris*3];

  for(DWORD dwTri=0; dwTri<dwTris; dwTri++)
  {
    DWORD dwBaseIndx[3];

    tangents.GetTriangleBaseIndices(dwTri, dwBaseIndx);

    assert(dwBaseIndx[0]<dwCnt);
    assert(dwBaseIndx[1]<dwCnt);
    assert(dwBaseIndx[2]<dwCnt);

    // for every vertex of the triangle
    for(int i=0; i<3; i++)
    {
      pBasisIndices[dwTri*3+i] = dwBaseIndx[i];		// set the base vector
    }
  }

  //
  for(i=0; i<dwCnt; i++)
  {
    tangents.GetBase(i, &pBasises[i].m_Tangent[0], &pBasises[i].m_Binormal[0], &pBasises[i].m_TNormal[0]);
  }
  assert(dwTris*3 == m_SecIndices.Num());
  SAFE_DELETE_ARRAY(m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData);
  m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData = new SPipTangents[m_SecVertCount];
  SPipTangents *tn = (SPipTangents *)m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
  for (i=0; i<m_SecIndices.Num(); i++)
  {
    tn[m_SecIndices[i]] = pBasises[pBasisIndices[i]];
  }
  SAFE_DELETE_ARRAY(pBasises);
  SAFE_DELETE_ARRAY(pBasisIndices);


  return true;
}

//======================================================================

bool CLeafBuffer::CreateBuffer( struct VertexBufferSource* pSource )
{
	//struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F * pVBuff = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[pSource->numVertices];
	//memset (pVBuff, 0, sizeof(struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F)*pSource->numVertices);
	
	if (!m_pMats)
	{
		if (pSource->pMats)
		{
			m_pMats = pSource->pMats;
			pSource->pMats = NULL;
		}
		else
		{
			m_pMats = new list2<CMatInfo>;
			m_pMats->resize(pSource->numPrimGroups);
			memset (&(*m_pMats)[0], 0, pSource->numPrimGroups*sizeof(CMatInfo));
		}
	}
	else
		assert (0); // this means we already have someone allocated the materials for us - not good!
	bool bBoxInited = false;
	for (unsigned t = 0; t < pSource->numPrimGroups; ++t)
	{
		const CCFMaterialGroup& pg = pSource->pPrimGroups[t];
		assert (pg.nMaterial < pSource->numMaterials);
		CMatInfo& mi = (*m_pMats)[pg.nMaterial];

		mi.nFirstIndexId = pg.nIndexBase;
		mi.nNumIndices   = pg.numIndices;

		CREOcLeaf *re = (CREOcLeaf *)gRenDev->EF_CreateRE(eDATA_OcLeaf);
		re->m_pBuffer = this;
		re->m_pChunk = &mi;
		mi.pRE = re;
		// fast creation of placeholder for the normal/centroid info
		/*
		if (!re->m_Faces)
			re->m_Faces = new TArray<SMeshFace>;
		re->m_Faces->Free();
		re->m_Faces->Alloc (pg.numIndices / 3);
		*/

		mi.m_Id = pg.nMaterial;
		if (pSource->pShaders)
			mi.shaderItem = pSource->pShaders[pg.nMaterial];
		re->m_Flags |= pSource->nREFlags;

		// correct the material flags according to incoming correction arrays
		if (pSource->pOrFlags)
			mi.m_Flags |= pSource->pOrFlags[pg.nMaterial];
		if (pSource->pAndFlags)
			mi.m_Flags &= pSource->pAndFlags[pg.nMaterial];

		// calculate the vertex index and coordinate ranges for this material
		unsigned nIndexEnd = pg.nIndexBase+pg.numIndices;
		if (pg.numIndices)
		{
			Vec3d vMinVtx = pSource->pVertices[pSource->pIndices[pg.nIndexBase]];
			Vec3d vMaxVtx = vMinVtx;
			if (!bBoxInited)
			{
				m_vBoxMin = vMinVtx;
				m_vBoxMax = vMaxVtx;
				bBoxInited = true;
			}
			else
			{
				m_vBoxMin.CheckMin(vMinVtx);
				m_vBoxMax.CheckMax(vMaxVtx);
			}
			unsigned short nMinVtx = pSource->pIndices[pg.nIndexBase];
			unsigned short nMaxVtx = nMinVtx;
			for (unsigned nIndex = pg.nIndexBase+1; nIndex < nIndexEnd; ++nIndex)
			{
				unsigned short nVertex = pSource->pIndices[nIndex];
				const Vec3d& vVertex = pSource->pVertices[nVertex];
				vMinVtx.CheckMin(vVertex);
				vMaxVtx.CheckMax(vVertex);
				nMinVtx = min (nMinVtx, nVertex);
				nMaxVtx = max (nMaxVtx, nVertex);
			}
			mi.m_vCenter = (vMinVtx+vMaxVtx) * 0.5f;
			mi.m_fRadius = (vMinVtx - mi.m_vCenter).GetLength();
			mi.nFirstVertId = nMinVtx;
			mi.nNumVerts = nMaxVtx-nMinVtx+1;
		}
		else
		{
			mi.m_vCenter = Vec3d(0,0,0);
			mi.m_fRadius = 0;
			mi.nFirstVertId = 0;
			mi.nNumVerts = 0;
		}
	}

  UpdateSysIndices(pSource->pIndices, pSource->numIndices);

	m_SecVertCount = pSource->numVertices;

	// TODO: get rid of the following
	/*
	m_pSecVertBuffer = new CVertexBuffer;
	m_pSecVertBuffer->m_vertexformat = VERTEX_FORMAT_P3F_COL4UB_TEX2F;
	struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F* pVBuf = new struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F[m_SecVertCount];
	m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = pVBuf;
	for (unsigned i = 0; i < m_SecVertCount; ++i)
	{
		*(Vec3d*)(&pVBuf[i].x) = pSource->pVertices[i];
		*(CryUV*)(&pVBuf[i].s) = pSource->pUVs[i];
	}
	

	CalcFaceNormals();
	CreateTangBuffer();
	*/
	return false;
}

#include "NvTriStrip/NVTriStrip.h"

void CLeafBuffer::StripifyMesh(int StripType)
{
  int i;
  unsigned int n;

  //iLog->Log("Stripify mesh...");
  
  ////////////////////////////////////////////////////////////////////////////////////////
  // Stripping stuff

  if (StripType == STRIPTYPE_DEFAULT)
    StripType = CRenderer::CV_r_stripmesh;

  if (StripType == STRIPTYPE_NONE)
    return;

  CryLogComment("  Sorting vertices for GPU cache ... ");

  int nGPU = gRenDev->GetFeatures() & RFT_HW_MASK;
  if (nGPU == RFT_HW_GF3 || nGPU == RFT_HW_GFFX || nGPU == RFT_HW_NV4X)
    SetCacheSize(CACHESIZE_GEFORCE3);
  else
    SetCacheSize(CACHESIZE_GEFORCE1_2);
  if (StripType == STRIPTYPE_SINGLESTRIP)
    SetStitchStrips(true);
  else
    SetStitchStrips(false);
  SetMinStripSize(0);
  if (StripType == STRIPTYPE_ONLYLISTS)
  {
    SetListsOnly(true);
    SetStitchStrips(false);
  }
  else
    SetListsOnly(false);

  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *pVBOld = (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *)m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *pVBNew = new struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F [m_SecVertCount];
  m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = pVBNew;

  SPipTangents *pTBOld = (SPipTangents *)m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
  SPipTangents *pTBNew = new SPipTangents [m_SecVertCount];
  m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData = pTBNew;
  
  // remember remapping
  m_arrVertStripMap = new uint [m_SecVertCount];
  memset(m_arrVertStripMap,256,sizeof(uint)*m_SecVertCount);

  int vertFirst = 0;

  list2<ushort> NewIndexes;

  //stripify!
  for (i=0; i<(*m_pMats).Count(); i++)
  {
    CMatInfo *mi = &(*m_pMats)[i];
    if (!mi->pRE)
      continue;
    PrimitiveGroup* pOldPG;
    GenerateStrips(&GetIndices(NULL)[mi->nFirstIndexId], mi->nNumIndices, &pOldPG, (unsigned short*)&mi->m_dwNumSections);

    //remap!
    PrimitiveGroup *pg;
		if ((int)mi->m_dwNumSections < 0)
			iLog->Log("\001CLeafBuffer::CreateBuffer m_dwNumSections out of range (%d), crash is very likely", mi->m_dwNumSections);

    RemapIndices(pOldPG, mi->m_dwNumSections, m_SecVertCount, &pg);
    mi->m_pPrimitiveGroups = new SPrimitiveGroup[mi->m_dwNumSections];

    int nMin =  999999;
    int nMax = -999999;
    
    //loop through all indices, copying from oldVB -> newVB
    //note that this will do numIndices copies, instead of numVerts copies,
    // which is extraneous.  Deal with it! ;-)
    int nFirstIndex = 0;
    mi->nFirstIndexId = NewIndexes.Count();
    for(int groupCtr = 0; groupCtr < mi->m_dwNumSections; groupCtr++)
    {
      mi->m_pPrimitiveGroups[groupCtr].type = pg[groupCtr].type;
      mi->m_pPrimitiveGroups[groupCtr].numIndices = pg[groupCtr].numIndices;
      mi->m_pPrimitiveGroups[groupCtr].offsIndex = nFirstIndex;
      mi->m_pPrimitiveGroups[groupCtr].numTris = 0;
      for(unsigned int indexCtr = 0; indexCtr < mi->m_pPrimitiveGroups[groupCtr].numIndices; indexCtr++)
      {
        //grab old index
        int oldVertex = pOldPG[groupCtr].indices[indexCtr];
        
        //grab new index
        int newVertex = pg[groupCtr].indices[indexCtr] + vertFirst;
        NewIndexes.Add(newVertex);

        nMin = min(nMin, newVertex);
        nMax = max(nMax, newVertex);

        //copy from old -> new vertex buffer
        memcpy(&pVBNew[newVertex], &pVBOld[oldVertex], sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F));
        memcpy(&pTBNew[newVertex], &pTBOld[oldVertex], sizeof(SPipTangents));

        // remember remaping
        m_arrVertStripMap[oldVertex] = newVertex;
      }
      nFirstIndex += mi->m_pPrimitiveGroups[groupCtr].numIndices;

      SPrimitiveGroup *pgn = &mi->m_pPrimitiveGroups[groupCtr];
      int incr;
      switch (pgn->type)
      {
        case PT_LIST:
          incr = 3;
          break;
        case PT_STRIP:
        case PT_FAN:
          incr = 1;
          break;
      }
      int offs = pgn->offsIndex;
      for (n=0; n<pgn->numIndices-2; n+=incr)
      {
        int i0, i1, i2;
        switch (pgn->type)
        {
          case PT_LIST:
            i0 = pg[groupCtr].indices[offs+n];
            i1 = pg[groupCtr].indices[offs+n+1];
            i2 = pg[groupCtr].indices[offs+n+2];
            break;
          case PT_STRIP:
            i0 = pg[groupCtr].indices[offs+n];
            i1 = pg[groupCtr].indices[offs+n+1];
            i2 = pg[groupCtr].indices[offs+n+2];
            break;
          case PT_FAN:
            i0 = pg[groupCtr].indices[offs+0];
            i1 = pg[groupCtr].indices[offs+n+1];
            i2 = pg[groupCtr].indices[offs+n+2];
            break;
        }
        // ignore degenerate triangle
        if (i0==i1 || i0==i2 || i1==i2)
          continue;
        pgn->numTris++;
      }
    }
    mi->nNumIndices = nFirstIndex;
    mi->nFirstVertId = nMin;
    mi->nNumVerts = nMax-nMin+1;
    vertFirst += mi->nNumVerts;
  }
  m_nPrimetiveType = R_PRIMV_MULTI_GROUPS;
  
  UpdateSysIndices(NewIndexes.GetElements(), NewIndexes.Count());
  
  SAFE_DELETE_ARRAY(pVBOld);
  SAFE_DELETE_ARRAY(pTBOld);
 
  //iLog->LogPlus("Ok");
}

void CLeafBuffer::CalcFaceNormals()
{
  int i, j;

  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *pV = (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *)m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  ushort *pInds = GetIndices(NULL);

  if (m_nPrimetiveType != R_PRIMV_MULTI_GROUPS)
  {
    for (i=0; i<m_pMats->Count(); i++)
    {
      CMatInfo *mi = m_pMats->Get(i);
      CREOcLeaf *re = mi->pRE;
      if (!re)
        continue;
      if (!re->m_Faces)
        re->m_Faces = new TArray<SMeshFace>;
      re->m_Faces->Free();
      int nOffs = mi->nFirstIndexId;
      for(j=0; j<mi->nNumIndices-2; j+=3)
	    {
		    unsigned short * face = &pInds[j+nOffs];

        struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *p0 = &pV[face[0]];
        struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *p1 = &pV[face[1]];
        struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *p2 = &pV[face[2]];
        Vec3d v0 = p0->xyz;
        Vec3d v1 = p1->xyz;
        Vec3d v2 = p2->xyz;
        Vec3d face_normal = (v0-v1) ^ (v0-v2);
		    face_normal.Normalize();
        SMeshFace fn;
        fn.m_Normal = face_normal;
        fn.m_Middle = (v0 + v1 + v2) / 3.0f;
        re->m_Faces->AddElem(fn);
      }
    }
  }
  else
  {
    unsigned int n;
    for (i=0; i<m_pMats->Count(); i++)
    {
      CMatInfo *mi = m_pMats->Get(i);
      CREOcLeaf *re = mi->pRE;
      if (!re)
        continue;
      if (!re->m_Faces)
        re->m_Faces = new TArray<SMeshFace>;
      re->m_Faces->Free();
      int nOffs = mi->nFirstIndexId;
      for (j=0; j<mi->m_dwNumSections; j++)
      {
        SPrimitiveGroup *g = &mi->m_pPrimitiveGroups[j];
        g->nFirstFace = re->m_Faces->Num();
        int incr;
        switch (g->type)
        {
          case PT_LIST:
            incr = 3;
            break;
          case PT_STRIP:
          case PT_FAN:
            incr = 1;
            break;
        }
        int offs = g->offsIndex + nOffs;
        for (n=0; n<g->numIndices-2; n+=incr)
        {
          int i0, i1, i2;
          switch (g->type)
          {
            case PT_LIST:
              i0 = pInds[offs+n];
              i1 = pInds[offs+n+1];
              i2 = pInds[offs+n+2];
              break;
            case PT_STRIP:
              i0 = pInds[offs+n];
              i1 = pInds[offs+n+1];
              i2 = pInds[offs+n+2];
              break;
            case PT_FAN:
              i0 = pInds[offs+0];
              i1 = pInds[offs+n+1];
              i2 = pInds[offs+n+2];
              break;
          }
          // ignore degenerate triangle
          if (i0==i1 || i0==i2 || i1==i2)
            continue;

          struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *p0 = &pV[i0];
          struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *p1 = &pV[i1];
          struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *p2 = &pV[i2];
          Vec3d v0 = p0->xyz;
          Vec3d v1 = p1->xyz;
          Vec3d v2 = p2->xyz;
          Vec3d face_normal = (v0-v1) ^ (v0-v2);
          face_normal.Normalize();

          SMeshFace fn;
          fn.m_Normal = face_normal;
          fn.m_Middle = (v0 + v1 + v2) / 3.0f;
          re->m_Faces->AddElem(fn);
        }
      }
    }
  }
}

void CLeafBuffer::SaveColors(byte *pD, SBufInfoTable *pOffs, int Size)
{
  if (!pOffs->OffsColor)
    return;
  if (!m_TempColors)
  {
    m_TempColors = new UCol[m_SecVertCount];
    for (int i=0; i<m_SecVertCount; i++, pD+=Size)
    {
      m_TempColors[i].dcolor = *(uint *)&pD[pOffs->OffsColor];
    }
  }
}
void CLeafBuffer::SaveSecColors(byte *pD, SBufInfoTable *pOffs, int Size)
{
  if (!pOffs->OffsSecColor)
    return;
  if (!m_TempSecColors)
  {
    m_TempSecColors = new UCol[m_SecVertCount];
    for (int i=0; i<m_SecVertCount; i++, pD+=Size)
    {
      m_TempSecColors[i].dcolor = *(uint *)&pD[pOffs->OffsSecColor];
    }
  }
}

void CLeafBuffer::SaveTexCoords(byte *pD, SBufInfoTable *pOffs, int Size)
{
  if (!pOffs->OffsTC)
    return;
  if (!m_TempTexCoords)
  {
    m_TempTexCoords = new SMRendTexVert[m_SecVertCount];
    for (int i=0; i<m_SecVertCount; i++, pD+=Size)
    {
      m_TempTexCoords[i].vert[0] = *(float *)&pD[pOffs->OffsTC];
      m_TempTexCoords[i].vert[1] = *(float *)&pD[pOffs->OffsTC+4];
    }
  }
}

void CLeafBuffer::SaveNormals(byte *pD, SBufInfoTable *pOffs, int Size)
{
  if (!pOffs->OffsNormal)
    return;
  if (!m_TempNormals)
  {
    m_TempNormals = new Vec3d[m_SecVertCount];
    for (int i=0; i<m_SecVertCount; i++, pD+=Size)
    {
      m_TempNormals[i].x = *(float *)&pD[pOffs->OffsNormal];
      m_TempNormals[i].y = *(float *)&pD[pOffs->OffsNormal+4];
      m_TempNormals[i].z = *(float *)&pD[pOffs->OffsNormal+8];
    }
  }
}

bool CLeafBuffer::ReCreateSystemBuffer(int VertFormat)
{
  SBufInfoTable *pOffsOld, *pOffsNew;
  byte *pOld = (byte *)m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  int nFormatOld = m_pSecVertBuffer->m_vertexformat;
  pOffsOld = &gBufInfoTable[nFormatOld];
  int SizeOld = m_VertexSize[nFormatOld];
  byte *pNew = NULL;
  m_pSecVertBuffer->m_vertexformat = VertFormat;
  pOffsNew = &gBufInfoTable[VertFormat];
  int SizeNew = m_VertexSize[VertFormat];

  bool bRes = true;

  if (pOld)
  {
    byte *p = (byte *)CreateVertexBuffer(VertFormat, m_SecVertCount);
    pNew = (byte *)p;
    byte *pS = pOld;
    byte *pD = pNew;
    for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew, pS+=SizeOld)
    {
      *(Vec3 *)pD = *(Vec3 *)pS;
    }
    if (pOffsNew->OffsColor)
    {
      // Restore colors
      pD = &pNew[pOffsNew->OffsColor];
      if (pOffsOld->OffsColor)
      {
        pS = &pOld[pOffsOld->OffsColor];
        for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew, pS+=SizeOld)
        {
          *(DWORD *)pD = *(DWORD *)pS;
        }
      }
      else
      if (m_TempColors)
      {
        for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew)
        {
          *(DWORD *)pD = m_TempColors[i].dcolor;
        }
        SAFE_DELETE_ARRAY(m_TempColors);
      }
      else
      {
        if (gRenDev->m_RP.m_pShader)
          iLog->Log("Warning: we lost colors for shader '%s'\n", gRenDev->m_RP.m_pShader->m_Name.c_str());
        bRes = false;
      }
    }
    else
      SaveColors(pOld, pOffsOld, SizeOld);

    if (pOffsNew->OffsSecColor)
    {
      // Restore colors
      pD = &pNew[pOffsNew->OffsSecColor];
      if (pOffsOld->OffsSecColor)
      {
        pS = &pOld[pOffsOld->OffsSecColor];
        for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew, pS+=SizeOld)
        {
          *(DWORD *)pD = *(DWORD *)pS;
        }
      }
      else
      if (m_TempSecColors)
      {
        for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew)
        {
          *(DWORD *)pD = m_TempSecColors[i].dcolor;
        }
        SAFE_DELETE_ARRAY(m_TempSecColors);
      }
      else
      {
        if (gRenDev->m_RP.m_pShader)
          iLog->Log("Warning: we lost secondary colors for shader '%s'\n", gRenDev->m_RP.m_pShader->m_Name.c_str());
        bRes = false;
      }
    }
    else
      SaveSecColors(pOld, pOffsOld, SizeOld);

    if (pOffsNew->OffsTC)
    {
      // Restore colors
      pD = &pNew[pOffsNew->OffsTC];
      if (pOffsOld->OffsTC)
      {
        pS = &pOld[pOffsOld->OffsTC];
        for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew, pS+=SizeOld)
        {
          *(float *)pD = *(float *)pS;
          *(float *)&pD[4] = *(float *)&pS[4];
        }
      }
      else
      if (m_TempTexCoords)
      {
        for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew)
        {
          *(float *)pD = m_TempTexCoords[i].vert[0];
          *(float *)&pD[4] = m_TempTexCoords[i].vert[1];
        }
        SAFE_DELETE_ARRAY(m_TempTexCoords);
      }
      else
      {
        if (gRenDev->m_RP.m_pShader)
          iLog->Log("Warning: we lost texture coords for shader '%s'\n", gRenDev->m_RP.m_pShader->m_Name.c_str());
        bRes = false;
      }
    }
    else
      SaveTexCoords(pOld, pOffsOld, SizeOld);

    if (pOffsNew->OffsNormal)
    {
      // Restore colors
      pD = &pNew[pOffsNew->OffsNormal];
      if (pOffsOld->OffsNormal)
      {
        pS = &pOld[pOffsOld->OffsNormal];
        for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew, pS+=SizeOld)
        {
          *(Vec3 *)pD = *(Vec3 *)pS;
        }
      }
      else
      if (m_TempNormals)
      {
        for (int i=0; i<m_SecVertCount; i++, pD+=SizeNew)
        {
          *(Vec3 *)pD = m_TempNormals[i];
        }
        SAFE_DELETE_ARRAY(m_TempNormals);
      }
      else
      {
        if (gRenDev->m_RP.m_pShader)
          iLog->Log("Warning: we lost normals for shader '%s'\n", gRenDev->m_RP.m_pShader->m_Name.c_str());
        bRes = false;
      }
    }
    else
      SaveNormals(pOld, pOffsOld, SizeOld);
  }

  delete [] pOld;
  m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = pNew;
  if (!pNew)
    return false;

  return bRes;
}


void CLeafBuffer::Unload()
{
  Unlink();
  if (gRenDev)
  {
    assert(!m_pVertexBuffer || m_pVertexBuffer->m_NumVerts == m_SecVertCount);
    if (m_pVertexBuffer)
      gRenDev->ReleaseBuffer(m_pVertexBuffer);
    gRenDev->ReleaseIndexBuffer(&m_Indices);
  }
  m_pVertexBuffer = NULL;
}

void CLeafBuffer::UpdateDynBufPtr(int VertFormat)
{
  if (m_pVertexBuffer && !m_pVertexBuffer->m_bFenceSet)
  {
    gRenDev->ReleaseBuffer(m_pVertexBuffer);
    m_pVertexBuffer = NULL;
  }
  if (!m_pVertexBuffer)
    m_pVertexBuffer = new CVertexBuffer;
  if (m_pVertexBuffer)
  {
    void *vBufTangs, *vBufGen;
    int nOffsTangs, nOffsGen;
    vBufGen = gRenDev->GetDynVBPtr(m_SecVertCount, nOffsGen, 1);
    vBufTangs = gRenDev->GetDynVBPtr(m_SecVertCount, nOffsTangs, 2);
    assert (vBufTangs && vBufGen && nOffsTangs == nOffsGen);
    m_pVertexBuffer->m_fence = nOffsTangs;
    m_pVertexBuffer->m_bFenceSet = true;
    m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData = vBufGen;
    m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData = vBufTangs;
    m_pVertexBuffer->m_NumVerts = m_SecVertCount;
    m_pVertexBuffer->m_VS[VSF_GENERAL].m_nItems = m_SecVertCount;
    m_pVertexBuffer->m_VS[VSF_TANGENTS].m_nItems = m_SecVertCount;
    m_pVertexBuffer->m_VS[VSF_GENERAL].m_bLocked = true;
    m_pVertexBuffer->m_VS[VSF_TANGENTS].m_bLocked = true;
    m_pVertexBuffer->m_vertexformat = VERTEX_FORMAT_P3F_TEX2F;
    Unlink();
  }
}


struct SSortTri
{
  int nTri;
  float fDist;
};

static TArray<SSortTri> sSortTris;

static _inline int Compare(SSortTri &a, SSortTri &b)
{
  float fDistA = a.fDist;
  float fDistB = b.fDist;
  if (fDistA+0.01f < fDistB)
    return 1;
  else
  if (fDistA > fDistB+0.01f)
    return -1;
  else
    return 0;
}

void CLeafBuffer::SortTris()
{
  int i, n;

  CRenderer *rd = gRenDev;
  if (m_SortFrame == rd->m_RP.m_TransformFrame)
    return;
  m_SortFrame = rd->m_RP.m_TransformFrame;
  ushort *pInds = GetIndices(NULL);
  CCObject *pObj = rd->m_RP.m_pCurObject;
  Vec3d vCam = pObj->GetInvMatrix().TransformPointOLD(rd->m_RP.m_ViewOrg);
  ushort *pDst = NULL;
  int nPosPtr;
  byte *pPosPtr = GetPosPtr(nPosPtr);
  bool bGlobalTransp = false;
  if (rd->m_RP.m_pShaderResources && rd->m_RP.m_pShaderResources->m_Opacity != 1.0f)
    bGlobalTransp = true;

  for(i=0; i<m_pMats->Count(); i++)
  {
    CMatInfo *pMI = m_pMats->Get(i);

    if (!pMI->pRE)
      continue;

    IShader *pSH = pMI->shaderItem.m_pShader->GetTemplate(-1);
    if (!bGlobalTransp)
    {
      if (!(pSH->GetFlags3() & EF3_HASALPHATEST))
      {
        if (!pMI->shaderItem.IsTransparent())
          continue;
      }
    }
    if (!m_Indices.m_bLocked)
    {
      rd->UpdateIndexBuffer(&m_Indices, NULL, 0, false);
      pDst = (ushort *)m_Indices.m_VData;
    }
    if (m_nPrimetiveType != R_PRIMV_MULTI_GROUPS)
    {
      int nStart = pMI->nFirstIndexId;
      int nEnd   = nStart + pMI->nNumIndices;
      int nTris = pMI->nNumIndices/3;

      assert(nEnd <= m_NumIndices);

      sSortTris.SetUse(nStart);
      for(n=nStart; n<nEnd; n+=3)
      {
        Vec3 *v0 = (Vec3 *)&pPosPtr[pInds[n+0]*nPosPtr];
        Vec3 *v1 = (Vec3 *)&pPosPtr[pInds[n+1]*nPosPtr];
        Vec3 *v2 = (Vec3 *)&pPosPtr[pInds[n+2]*nPosPtr];
        float d0 = GetSquaredDistance(*v0, vCam);
        float d1 = GetSquaredDistance(*v1, vCam);
        float d2 = GetSquaredDistance(*v2, vCam);
        SSortTri st;
        st.nTri = n;
        st.fDist = max(d2, max(d1, d0));
        sSortTris.AddElem(st);
      }
      ::Sort(&sSortTris[nStart], nTris);
      for(n=0; n<nTris; n++)
      {
        int m = sSortTris[n].nTri;
        pDst[nStart+n*3+0] = pInds[m+0];
        pDst[nStart+n*3+1] = pInds[m+1];
        pDst[nStart+n*3+2] = pInds[m+2];
      }
    }
    else
    {
    }
  }
  if (m_Indices.m_bLocked)
    rd->UpdateIndexBuffer(&m_Indices, NULL, 0, true);
}

bool CLeafBuffer::CheckUpdate(int VertFormat, int Flags, bool bNeedAddNormals)
{
  CLeafBuffer *lb = GetVertexContainer();

  lb->Unlink();
  lb->Link(&CLeafBuffer::m_Root);

  bool bWasReleased = false;

  if (lb->m_sSource)
  {
    /*if (!stricmp(lb->m_sSource, "WaterVolume"))
    {
      int nnn = 0;
    }*/
  }
  /*if (Flags & FHF_FORANIM)
  {
    int RequestedVertFormat = VertFormat;
    if (lb->m_pVertexBuffer)
      RequestedVertFormat = gRenDev->m_RP.m_VFormatsMerge[lb->m_pVertexBuffer->m_vertexformat][RequestedVertFormat];
    if (RequestedVertFormat == VERTEX_FORMAT_P3F_TEX2F && (Flags & FHF_TANGENTSUSED))
    {
      lb->UpdateDynBufPtr(RequestedVertFormat);
    }
    else
    {
      int nnn = 0;
    }
  }*/
  CRenderer *rd = gRenDev;

  if (bNeedAddNormals || !lb->m_pVertexBuffer || (lb->m_UpdateVBufferMask & 0x101) || lb->m_pVertexBuffer->m_vertexformat != VertFormat)
  {
    int RequestedVertFormat;
    RequestedVertFormat = VertFormat;
    if (bNeedAddNormals)
    {
      SVertBufComps Comps;
      GetVertBufComps(&Comps, RequestedVertFormat);
      RequestedVertFormat = VertFormatForComponents(Comps.m_bHasColors, Comps.m_bHasSecColors, true, Comps.m_bHasTC);
    }

    // Create the video buffer
    bool bCreate = true;
    if (lb->m_pVertexBuffer)
    {
      RequestedVertFormat = rd->m_RP.m_VFormatsMerge[lb->m_pVertexBuffer->m_vertexformat][RequestedVertFormat];
      if (lb->m_pVertexBuffer->m_vertexformat == RequestedVertFormat)
        bCreate = false;
      else
      {
        rd->ReleaseBuffer(lb->m_pVertexBuffer);
        bWasReleased = true;
        lb->m_pVertexBuffer = 0;  // M.M. test
      }
    }
    if (bCreate || (lb->m_UpdateVBufferMask & 0x101))
    {
      lb->m_UpdateFrame = gRenDev->GetFrameID();
      if (bCreate)
        lb->m_UpdateVBufferMask |= 1;
      if (lb->PrepareBufferCallback)
      {
        int nnn = 0;
      }
      if (!lb->m_pVertexBuffer)
      {
        PROFILE_FRAME(Mesh_CheckUpdateCreateGBuf);
        lb->CreateVidVertices(lb->m_SecVertCount, RequestedVertFormat);
        lb->m_nVertexFormat = RequestedVertFormat;
      }
      if (lb->PrepareBufferCallback)
      {
        PROFILE_FRAME(Mesh_CheckUpdateCallback);
        if (!lb->PrepareBufferCallback(lb, (Flags & SHPF_TANGENTS) != 0))
          return false;
        bWasReleased = true;
        lb->m_UpdateVBufferMask &= ~0x100;
      }
      else
      if (!lb->m_bOnlyVideoBuffer)
      {
        assert(lb->m_pSecVertBuffer);
        if (lb->m_pSecVertBuffer->m_vertexformat != RequestedVertFormat)
        {
          PROFILE_FRAME(Mesh_CheckUpdateRecreateSystem);
          lb->ReCreateSystemBuffer(RequestedVertFormat);
        }
      }
      if (!lb->m_pVertexBuffer)
        return false;
    } 
    if ((lb->m_UpdateVBufferMask & 1) && lb->m_pSecVertBuffer && lb->m_pVertexBuffer)
    {
      PROFILE_FRAME(Mesh_CheckUpdateUpdateGBuf);
      rd->UpdateBuffer(lb->m_pVertexBuffer, lb->m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, lb->m_SecVertCount, true, 0);
    }
    lb->m_UpdateVBufferMask &= ~1;
  }

  // If we need Tangent Vectors for shader and Tangents stream doesn't exist create and fill it
  if ((Flags & SHPF_TANGENTS) && (!lb->m_pVertexBuffer->GetStream(VSF_TANGENTS, NULL) || (lb->m_UpdateVBufferMask & 2)))
  {
    lb->m_UpdateFrame = gRenDev->GetFrameID();
    lb->m_UpdateVBufferMask |= 2;
    if (!lb->m_bOnlyVideoBuffer && lb->m_pSecVertBuffer)
    {
      if (!lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData)
      {
        PROFILE_FRAME(Mesh_CheckUpdateCreateSysTang);
        lb->CreateTangBuffer();
      }
      assert(lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData);
      if (lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData)
      {
        if (!lb->m_pVertexBuffer->GetStream(VSF_TANGENTS, NULL))
        {
          PROFILE_FRAME(Mesh_CheckUpdateCreateTBuf);
          gRenDev->CreateBuffer(lb->m_SecVertCount*sizeof(SPipTangents), 0, lb->m_pVertexBuffer, VSF_TANGENTS, "LeafBuffer tangents");
        }
      }
    }
    else
    if (!lb->m_pVertexBuffer->GetStream(VSF_TANGENTS, NULL))
    {
      PROFILE_FRAME(Mesh_CheckUpdateCreateTBuf);
      gRenDev->CreateBuffer(lb->m_SecVertCount*sizeof(SPipTangents), 0, lb->m_pVertexBuffer, VSF_TANGENTS, "LeafBuffer tangents");
    }
    if (!lb->m_pVertexBuffer->GetStream(VSF_TANGENTS, NULL))
      return false;
    if ((lb->m_UpdateVBufferMask & 2) && lb->m_pSecVertBuffer && lb->m_pVertexBuffer)
    {
      PROFILE_FRAME(Mesh_CheckUpdateUpdateTBuf);
      gRenDev->UpdateBuffer(lb->m_pVertexBuffer, lb->m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData, lb->m_SecVertCount, true, 0, VSF_TANGENTS);
    }
    lb->m_UpdateVBufferMask &= ~2;
  }

  if (!m_Indices.m_VData || (m_UpdateVBufferMask & 0x100))
  {
    PROFILE_FRAME(Mesh_CheckUpdateUpdateInds);
    UpdateVidIndices(&m_SecIndices[0], m_NumIndices);
  }
  if (rd->m_RP.m_ObjFlags & FOB_SORTPOLYS)
    SortTris();
  //else
  //if (rd->m_RP.m_pShaderResources && rd->m_RP.m_pShaderResources->m_Opacity != 1.0f)
  //  SortTris();

  return bWasReleased;
}

int CLeafBuffer::GetAllocatedBytes(bool bVideoBuf)
{
  if (bVideoBuf)
  {
    int size = 0;
    if (m_pVertexBuffer)
    {
      size += m_VertexSize[m_pVertexBuffer->m_vertexformat];
      if (m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData)
        size += m_SecVertCount * sizeof(SPipTangents);
    }
    return size;
  }
  int size = sizeof(*this);
  
  if(m_pSecVertBuffer)
    size += m_VertexSize[m_pSecVertBuffer->m_vertexformat]*m_SecVertCount;

  return size;
}

void CLeafBuffer::ReleaseShaders()
{
	if(!m_pMats)
		return;

  for(int i=0; i<m_pMats->Count(); i++)
  {
    if(m_pMats->Get(i)->shaderItem.m_pShader)
    {
      m_pMats->Get(i)->shaderItem.m_pShader->Release();
      m_pMats->Get(i)->shaderItem.m_pShader = NULL;
    }
  }
}

CLeafBuffer::~CLeafBuffer()
{
  UnlinkGlobal();
  if(m_bMaterialsWasCreatedInRenderer && m_pMats)
  {
    for(int i=0; i<m_pMats->Count(); i++)
    {
      CMatInfo *mi = m_pMats->Get(i);
      if(mi->pMatEnt)
      { // pMatEnt can present only if file was loaded from co
        delete mi->pMatEnt;
        mi->pMatEnt=0;

        if(mi->m_dwNumSections)
          delete [] mi->m_pPrimitiveGroups;
        mi->m_pPrimitiveGroups = 0;
      }

      if (mi->shaderItem.m_pShader)
        mi->shaderItem.m_pShader->Release();
      if (mi->shaderItem.m_pShaderResources)
        mi->shaderItem.m_pShaderResources->Release();

      if(m_pMats->Get(i)->pRE)
      {
        m_pMats->Get(i)->pRE->Release();
      }
    }
    delete m_pMats;
    m_pMats=0;
  }

  FreeSystemBuffer();

  if(m_pLoadedColors)
    delete [] m_pLoadedColors;
  m_pLoadedColors=0;

  Unload();

  DestroyIndices();
  SAFE_DELETE(m_pIndicesPreStrip);

  delete [] m_arrVertStripMap;
  m_arrVertStripMap=0;
}

void CLeafBuffer::FreeSystemBuffer()
{
  if (m_pSecVertBuffer) 
  {
#ifdef PS2
    SAFE_DELETE_ARRAY((PipVertex*)m_pSecVertBuffer->m_VData);
#else  
    SAFE_DELETE_ARRAY(m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData);
#endif    
    SAFE_DELETE_ARRAY(m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData); 
    SAFE_DELETE(m_pSecVertBuffer);
  }

  SAFE_DELETE_ARRAY(m_TempNormals);
  SAFE_DELETE_ARRAY(m_TempTexCoords);
  SAFE_DELETE_ARRAY(m_TempColors);
  SAFE_DELETE_ARRAY(m_TempSecColors);

	SAFE_DELETE_ARRAY(m_arrVtxMap);
}

unsigned short *CLeafBuffer::GetIndices(int * pIndicesCount)
{
  if (pIndicesCount)
    *pIndicesCount = m_NumIndices;
  return &m_SecIndices[0];
}
void CLeafBuffer::DestroyIndices()
{
  if (gRenDev)
    gRenDev->ReleaseIndexBuffer(&m_Indices);
  m_SecIndices.Free();
}

void CLeafBuffer::UpdateVidIndices(const ushort *pNewInds, int nInds)
{
  gRenDev->UpdateIndexBuffer(&m_Indices, pNewInds, nInds, true);
  m_UpdateVBufferMask &= ~0x100;
}
void CLeafBuffer::UpdateSysIndices(const ushort *pNewInds, int nInds)
{
  m_NumIndices = nInds;
  if (m_SecIndices.Num() != nInds)
  {
    m_SecIndices.Free();
    m_SecIndices.Reserve(nInds);
  }
  cryMemcpy(&m_SecIndices[0], pNewInds, nInds*2);
  InvalidateVideoBuffer(0x100);
}

void CLeafBuffer::UpdateSysVertices(void * pNewVertices, int nNewVerticesCount)
{
  if (!m_pSecVertBuffer)
    CreateSysVertices(nNewVerticesCount, m_nVertexFormat);
  gRenDev->UpdateBuffer(m_pSecVertBuffer,pNewVertices, nNewVerticesCount,true);
  if(m_pVertexBuffer)
  {
    if (m_pVertexBuffer->m_NumVerts != m_pSecVertBuffer->m_NumVerts)
    {
      gRenDev->ReleaseBuffer(m_pVertexBuffer);
      m_pVertexBuffer = NULL;
    }
  }
  InvalidateVideoBuffer(1);
}
void CLeafBuffer::UpdateVidVertices(void * pNewVertices, int nNewVerticesCount)
{
  if (!m_pVertexBuffer)
    CreateVidVertices(nNewVerticesCount, m_nVertexFormat);
  else
  if (m_pVertexBuffer->m_NumVerts != nNewVerticesCount)
  {
    gRenDev->ReleaseBuffer(m_pVertexBuffer);
    m_pVertexBuffer = NULL;
    m_SecVertCount = nNewVerticesCount;
    CreateVidVertices(nNewVerticesCount, m_nVertexFormat);
  }

  gRenDev->UpdateBuffer(m_pVertexBuffer,pNewVertices, nNewVerticesCount,true);
}

void CLeafBuffer::CreateVidVertices(int nVerts, int VertFormat)
{
//	assert(!m_Next && !m_Prev);
	Unlink();
  assert(!m_pVertexBuffer);
  m_pVertexBuffer = gRenDev->CreateBuffer(nVerts, VertFormat, "LeafBuffer", m_bDynamic);
  if (!m_pVertexBuffer)
    return;
  assert(m_SecVertCount == m_pVertexBuffer->m_NumVerts);
  Link(&m_Root);
}

bool CLeafBuffer::CreateSysVertices(int nVerts, int VertFormat)
{
  if (!m_pSecVertBuffer)
    m_pSecVertBuffer = new CVertexBuffer;
  else
  if (m_SecVertCount != nVerts)
  {
    SAFE_DELETE(m_pSecVertBuffer);
    m_pSecVertBuffer = new CVertexBuffer;
  }
  m_SecVertCount = nVerts;
  m_pSecVertBuffer->m_vertexformat = VertFormat;
  int Size = m_VertexSize[VertFormat];
  m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = new byte[m_SecVertCount*Size];
  memset(m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, 0, m_SecVertCount*Size);

  return true;
}


void * CLeafBuffer::GetSecVerticesPtr(int * pVerticesCount) 
{ 
  *pVerticesCount = m_pSecVertBuffer->m_NumVerts;
//  return m_pSecVertBuffer->m_data; 

  return m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
}

// add new chunk
void CLeafBuffer::SetChunk( IShader * pShader, 
                            int nFirstVertId, int nVertCount, 
                            int nFirstIndexId, int nIndexCount, int nMatID, 
														bool bForceInitChunk )
{
  if(!nIndexCount || !nVertCount)
    return;

  CMatInfo * pMat = 0;

  if(nMatID < 0 || nMatID >= m_pMats->Count())
  {
    // add new chunk
    CMatInfo matinfo;
    m_pMats->Add(matinfo);
    pMat = &m_pMats->Last();

    if(m_pMats->Count()>1 && !bForceInitChunk)
    {
      pMat->shaderItem.m_pShader = 0;
      pMat->pRE = 0;
    }
    else
    {
      pMat->shaderItem.m_pShader = pShader;//gRenDev->EF_LoadShader((char*)szEfName, -1, eEF_World, 0);
      pMat->pRE = (CREOcLeaf*)gRenDev->EF_CreateRE(eDATA_OcLeaf);
      pMat->pRE->m_CustomTexBind[0] = m_nClientTextureBindID;
    }
  }
  else
  {
    // use present chunk
    pMat = m_pMats->Get(nMatID);
  }

  // update chunk
  if(pMat->pRE)
  {
    pMat->pRE->m_pChunk = pMat;
    pMat->pRE->m_pBuffer = this;
    pMat->shaderItem.m_pShader = pShader;
  }

  assert(!pMat->pRE ||pMat->pRE->m_pChunk->nFirstIndexId<60000);

  pMat->nFirstIndexId = nFirstIndexId;
  pMat->nNumIndices  = max(nIndexCount,0);
  pMat->nFirstVertId = nFirstVertId;
  pMat->nNumVerts = max(nVertCount,0);

/*#ifdef _DEBUG
	ushort *pInds = (ushort *)m_Indices.m_VData;
	if (pInds)
	{
		for(int i=pMat->nFirstIndexId; i<pMat->nFirstIndexId+pMat->nNumIndices; i++)
		{
			int id = pInds[i];
			assert(id>=pMat->nFirstVertId && id<(pMat->nFirstVertId+pMat->nNumVerts));
		}
	}
#endif // _DEBUG*/
}

// set effector for all chunks
void CLeafBuffer::SetShader( IShader * pShader, int nCustomTID )
{
  if (m_pMats->Count())
  {
    CMatInfo *mi = m_pMats->Get(0);
    if (mi->shaderItem.m_pShader == pShader && mi->pRE && mi->pRE->m_CustomTexBind[0] == nCustomTID)
      return;
  }
  for(int i=0; i<m_pMats->Count(); i++)
  {
    CMatInfo *mi = m_pMats->Get(i);
    mi->shaderItem.m_pShader = pShader;
    if(mi->pRE)
      mi->pRE->m_CustomTexBind[0] = nCustomTID;
  }
}

void CLeafBuffer::SetRECustomData(float * pfCustomData, float fFogScale, float fAlpha)
{
  for(int i=0; i<m_pMats->Count(); i++)
  {
    if(m_pMats->Get(i)->pRE)
    {
      m_pMats->Get(i)->pRE->m_CustomData = pfCustomData;
      m_pMats->Get(i)->pRE->m_fFogScale  = fFogScale;
			m_pMats->Get(i)->pRE->m_Color.a		 = fAlpha;
    }
  }
}

int CVertexBuffer::Size(int Flags, int nVerts)
{
  int nSize = sizeof(*this);

  if (m_VS[VSF_GENERAL].m_VData)
    nSize += nVerts * m_VertexSize[m_vertexformat];
  if (m_VS[VSF_TANGENTS].m_VData)
    nSize += nVerts * sizeof(SPipTangents);

  return nSize;
}

int CMatInfo::Size()
{
  int nSize = sizeof(*this);
  if (m_pPrimitiveGroups)
    nSize += sizeof(SPrimitiveGroup)*m_dwNumSections;

  return nSize;
}

int CLeafBuffer::Size(int Flags)
{
  int nSize;
  if (!Flags)
  {
    nSize = sizeof(*this);
    if (m_pSecVertBuffer)
      nSize += m_pSecVertBuffer->Size(Flags, m_SecVertCount);
    if (m_TempNormals)
      nSize += sizeof(Vec3d) * m_SecVertCount;
    if (m_TempTexCoords)
      nSize += sizeof(SMRendTexVert) * m_SecVertCount;
    if (m_TempColors)
      nSize += sizeof(UCol) * m_SecVertCount;
    if (m_TempSecColors)
      nSize += sizeof(UCol) * m_SecVertCount;
    if (m_SecIndices.Num())
      nSize += m_SecIndices.GetMemoryUsage();
    if (m_pIndicesPreStrip && m_pIndicesPreStrip->Count())
      nSize += m_pIndicesPreStrip->GetMemoryUsage();
    if (m_pMats)
    {
      for (int i=0; i<(int)m_pMats->capacity(); i++)
      {
        if (i < m_pMats->Count())
          nSize += m_pMats->Get(i)->Size();
        else
          nSize += sizeof(CMatInfo);
      }
    }
    if (m_pLoadedColors)
      nSize += sizeof(Vec3d) * m_SecVertCount;
  }
  if (Flags & 1)
  {
    nSize = 0;
    if (m_pSecVertBuffer)
      nSize += m_pSecVertBuffer->Size(0, m_SecVertCount);
  }
  if (Flags & 2)
  {
    nSize = 0;
    if (m_SecIndices.Num())
      nSize += m_SecIndices.GetMemoryUsage();
  }

  return nSize;
}

CLeafBuffer::CLeafBuffer(const char *szSource)
{
  m_Indices.Reset();
  m_pIndicesPreStrip = NULL;

  m_sSource = (char *)szSource;

	m_SecVertCount=0;
	m_pSecVertBuffer=NULL;
	m_pVertexBuffer=NULL;

	m_pMats = NULL;
  m_nPrimetiveType = R_PRIMV_TRIANGLES;

  m_TempTexCoords = NULL;
  m_TempColors = NULL;
  m_TempSecColors = NULL;
  m_TempNormals = NULL;

  m_nClientTextureBindID = 0;
  m_bMaterialsWasCreatedInRenderer = 0;
  m_bOnlyVideoBuffer = 0;
  m_bDynamic = 0;
  m_Next = NULL;
  m_Prev = NULL;
  m_NextGlobal = NULL;
  m_PrevGlobal = NULL;
  m_pVertexContainer = NULL;
  m_UpdateVBufferMask = -1;
  m_UpdateFrame = 0;
  if (!m_Root.m_Next)
  {
    m_Root.m_Next = &m_Root;
    m_Root.m_Prev = &m_Root;
  }
  if (!m_RootGlobal.m_NextGlobal)
  {
    m_RootGlobal.m_NextGlobal = &m_RootGlobal;
    m_RootGlobal.m_PrevGlobal = &m_RootGlobal;
  }
  m_vBoxMin = m_vBoxMax = Vec3d(0,0,0);//used for hw occlusion test
  m_pLoadedColors = 0;
  m_arrVertStripMap = 0;
  PrepareBufferCallback = NULL;
  if (this != &m_RootGlobal && this != &m_Root)
    LinkGlobal(&m_RootGlobal);
	m_arrVtxMap = 0;
}
