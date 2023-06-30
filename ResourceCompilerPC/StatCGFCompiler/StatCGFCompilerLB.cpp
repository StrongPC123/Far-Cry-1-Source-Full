////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   StatCGFCompiler.cpp
//  Version:     v1.00
//  Created:     5/11/2002 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "Dbghelp.h"
#include "FileUtil.h"
#include "PathUtil.h"
#include "..\ResourceCompilerPC.h"

#include "StatCGFCompiler.h"
#include "CryChunkedFile.h"
#include "CryHeaders.h"

#include "NvTriStrip\NvTriStrip.h"

#include "meshidx.h"
#include <IShader.h>
#include "Cry_Geo.h"

CSimpleLeafBuffer::CSimpleLeafBuffer(IRCLog * pLog, CIndexedMesh * pIndexedMesh, bool bStripifyAndShareVerts,bool bKeepRemapTable)
{ 
	memset(this,0,sizeof(CSimpleLeafBuffer)); 
  m_pIndices = new list2<unsigned short>;
  m_pIndicesPreStrip = new list2<unsigned short>;
	m_pLog = pLog;

  CreateLeafBuffer(pIndexedMesh,bStripifyAndShareVerts,bStripifyAndShareVerts,bKeepRemapTable);
}

CSimpleLeafBuffer::~CSimpleLeafBuffer()
{
	for (int i=0; m_pMats && i<m_pMats->Count(); i++)
	{
		delete (CSimpleREOcLeaf*)(m_pMats->Get(i)->pRE);
		m_pMats->Get(i)->pRE = 0;
		delete m_pMats->Get(i)->pMatEnt;
		m_pMats->Get(i)->pMatEnt=0;
		delete [] m_pMats->Get(i)->m_pPrimitiveGroups;
		m_pMats->Get(i)->m_pPrimitiveGroups=0;
	}

	delete m_pIndices;
	delete m_pIndicesPreStrip;

	delete m_TempNormals;
	delete m_TempTexCoords;
	delete m_TempColors;
	delete m_pMats;
	
	delete m_pD3DIndBuf;
	delete m_pLoadedColors;
	delete m_arrVertStripMap;
	
	if (m_pVertexBuffer)
	{
		delete m_pVertexBuffer->m_VS[VSF_GENERAL].m_VData;
		delete m_pVertexBuffer->m_VS[VSF_TANGENTS].m_VData;
	}

	if (m_pSecVertBuffer)
	{
		delete m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
		delete m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
	}

	delete m_pVertexContainer;
	delete m_pVertexBuffer;
	delete m_pSecVertBuffer;

	if (m_arrVtxMap)
		delete[] m_arrVtxMap;
}


void CSimpleLeafBuffer::CreateLeafBuffer( CIndexedMesh * pTriData, int Stripify, bool bShareVerts, bool bKeepRemapTable )
{
	m_pLog->Log("Processing geometry");

	int max_vert_num = pTriData->m_nFaceCount*3;

	int i;
	struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * pVBuff = new struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F[max_vert_num];
	CBasis * pTmpTangBasis = new CBasis[max_vert_num];

	int buff_vert_count = 0;
  uint *uiInfo = new uint[max_vert_num];

	uint *piVtxIdx = 0; // [Anton] need this to build the mapping table leafbuffer vtx idx->original vtx idx
	if (bKeepRemapTable)
		piVtxIdx = new uint[max_vert_num];
	m_arrVtxMap = 0;

	// . Sort|Group faces by materials
	// For each shader (designated by shader_id of an element of m_pFaces)
	// there is one list2 in this table. Each list will contain
	// set of faces belonging to this shader.
	list2<CObjFace*> _hash_table[512];
  bool bShareVertsArr[512];

	m_pMats = new list2<CMatInfo>;
	m_pMats->PreAllocate(pTriData->m_lstMatTable.Count(),pTriData->m_lstMatTable.Count());
	*m_pMats = pTriData->m_lstMatTable;

	{ // fill the table: one list of faces per one shader
		for(int i=0; i<pTriData->m_nFaceCount; i++)
		{
			CObjFace * pFace =  &pTriData->m_pFaces[i];   

			char szMatName[128];
			strncpy(szMatName, m_pMats->GetAt(pFace->shader_id).pMatEnt->name, 128);
			strlwr(szMatName);
			if(strstr(szMatName,"(nodraw)") || strstr(szMatName,"(no_draw)"))
				continue;

			assert(pFace->shader_id>=0 && pFace->shader_id<512);
	    
			if(pFace->shader_id>=m_pMats->Count())
			{
				pFace->shader_id=0;
				m_pLog->Log("CLeafBuffer::CreateBuffer shader_id of face is out of range");
			}

			_hash_table[pFace->shader_id].Add(pFace);
		}
	}

	// . Create vertex buffer with sequence of (possibly non-unique) vertices, 3 verts per face
	// for each shader..
	for (int t = 0; t < m_pMats->Count(); t++) 
	{
		// memorize the starting index of this material's face range
		(*m_pMats)[t].nFirstIndexId = buff_vert_count;

		// scan through all the faces using the shader #t
		for(int i=0; i<_hash_table[t].Count(); ++i)
		{
			CObjFace * pFace = _hash_table[t][i];
	    
			assert(pFace->shader_id == t);

			for (int v = 0; v < 3; ++v)
			{
				if(pTriData->m_pColor)
				{ // if color exported - copy from pTriData
					pVBuff[buff_vert_count].color.bcolor[0] = pTriData->m_pColor[pFace->v[v]].r;
					pVBuff[buff_vert_count].color.bcolor[1] = pTriData->m_pColor[pFace->v[v]].g;
					pVBuff[buff_vert_count].color.bcolor[2] = pTriData->m_pColor[pFace->v[v]].b;
					pVBuff[buff_vert_count].color.bcolor[3] = 255;
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
        pVBuff[buff_vert_count].normal = pTriData->m_pNorms[pFace->n[v]];
        uiInfo[buff_vert_count] = 0;

				// position
        pVBuff[buff_vert_count].xyz = pTriData->m_pVerts[pFace->v[v]];
				
				// remember shader id per face to prevent vertex sharing between materials during recompacting
        uiInfo[buff_vert_count] |= pFace->shader_id;
        bShareVertsArr[pFace->shader_id] = bShareVerts;

				// [Anton] keep index list to build mapping table later
				if (piVtxIdx)
					piVtxIdx[buff_vert_count] = pFace->v[v];

				// tang basis
				pTmpTangBasis[buff_vert_count] = pTriData->m_pTangBasis[pFace->b[v]];

				buff_vert_count++;
			}
		}

		// there are faces belonging to this material(shader) #t, if number of indices > 0
		(*m_pMats)[t].nNumIndices = buff_vert_count - (*m_pMats)[t].nFirstIndexId;
		_hash_table[t].Reset();
	}

	// make REs
	for (i=0; i<(*m_pMats).Count(); i++)
	{		
		if((*m_pMats)[i].nNumIndices)
		{			
			CSimpleREOcLeaf *re = new CSimpleREOcLeaf; // (CRE OcLeaf *)gRenDev->EF_CreateRE(eDATA_OcLeaf);
			re->m_pChunk = &(*m_pMats)[i];
			re->m_pBuffer = this;
			assert (re->m_pChunk->nNumIndices < 60000);
			re->m_pChunk->pRE = (CREOcLeaf*)re;			

			// always enable sharing if there is 'flareproc' in shader/material name
			if (!bShareVertsArr[i] && (*m_pMats)[i].nNumIndices == 6)
			{
				char nameSh[128];
				strncpy(nameSh, (*m_pMats)[i].pMatEnt->name,sizeof(nameSh));
				strlwr(nameSh);
				bShareVertsArr[i] = strstr(nameSh, "flareproc")!=0;
			}
		}
	}

	// . For each (non-unique) vertex calculate the tangent base
/*	m_pBasises = buff_vert_count ? new CBasis[buff_vert_count] : 0;
	for (int n=0; n<buff_vert_count; n+=3)
	{
    Vec3d *vN0 = (Vec3d *)(&pVBuff[n+0].nx);
    Vec3d *vN1 = (Vec3d *)(&pVBuff[n+1].nx);
    Vec3d *vN2 = (Vec3d *)(&pVBuff[n+2].nx);
    Vec3d vFaceNormal = *vN0 + *vN1 + *vN2;
    vFaceNormal.Normalize();

		float *v[3] = 
		{
			(float *)&pVBuff[n+0].x,
			(float *)&pVBuff[n+1].x,
			(float *)&pVBuff[n+2].x,
		};

		float *tc[3] =
		{
			(float *)&pVBuff[n+0].s,
			(float *)&pVBuff[n+1].s,
			(float *)&pVBuff[n+2].s,
		};

		compute_tangent(v[0], v[1], v[2], tc[0], tc[1], tc[2], m_pBasises[n+0].tangent, m_pBasises[n+0].binormal, m_pBasises[n+0].tnormal, vFaceNormal);
		compute_tangent(v[1], v[2], v[0], tc[1], tc[2], tc[0], m_pBasises[n+1].tangent, m_pBasises[n+1].binormal, m_pBasises[n+1].tnormal, vFaceNormal);
		compute_tangent(v[2], v[0], v[1], tc[2], tc[0], tc[1], m_pBasises[n+2].tangent, m_pBasises[n+2].binormal, m_pBasises[n+2].tnormal, vFaceNormal);
	}*/

	// . Index the mesh (Compact Vertices): detect and delete duplicate vertices
	// remove duplicates
	if(buff_vert_count)
		CompactBuffer(pVBuff, &buff_vert_count, &GetIndices(), bShareVertsArr, uiInfo, pTmpTangBasis	);

  delete [] uiInfo;
  uiInfo=0;

	for( i=0; i<GetIndices().Count(); i++ )
	{
		if(GetIndices()[i]<buff_vert_count)
			continue;

		m_pLog->ThrowError("CLeafBuffer::CreateBuffer: Indices out of range");
	}

	if (bKeepRemapTable) // [Anton] build the mapping table leaf buffer vertex index -> original vertex index
	{
		m_arrVtxMap = new uint[buff_vert_count];
		for( i=0; i<GetIndices().Count(); i++ )
			m_arrVtxMap[GetIndices()[i]] = piVtxIdx[i];
		delete[] piVtxIdx;
	}

	if(buff_vert_count>65535)
		m_pLog->ThrowError("CLeafBuffer::CreateBuffer: Number of vertices in object is more than 65535");

	// . Remove degenerated triangles in the generated mesh (GetIndices())
	// FIXME: For some reason this optimization doesn't work for Animated objects (Assertion in CryModelState::GenerateRenderArrays)
	/*if (!m_sSource || strcmp(m_sSource, "CryModelArray") != 0)
	{
		// Remove degenerated triangles
		list2<ushort> NewIndexes;
		for (i=0; i<(*m_pMats).Count(); i++)	// each material..
		{
			CMatInfo *mi = &(*m_pMats)[i];
			if (!mi->pRE)
				continue;
			int nFirstInd = NewIndexes.Count();
			for (int j=mi->nFirstIndexId; j<mi->nFirstIndexId+mi->nNumIndices; j+=3)
			{
				// the face in material #i consists of vertices i0,i1,i2:
				int i0 = GetIndices()[j+0];
				int i1 = GetIndices()[j+1];
				int i2 = GetIndices()[j+2];
				// if the face is not degenerated, then add it; otherwise skip and finally it'll be deleted
				if (i0!=i1 && i0!=i2 && i1!=i2)
				{
					NewIndexes.Add(i0);
					NewIndexes.Add(i1);
					NewIndexes.Add(i2);
				}
			}
			mi->nFirstIndexId = nFirstInd;
			mi->nNumIndices = NewIndexes.Count() - nFirstInd;
			if (!mi->nNumIndices)
			{
				mi->pRE->Release();
				mi->pRE = NULL;
			}
		}
		GetIndices().Free();
		GetIndices().AddList(NewIndexes);
		NewIndexes.Free();
	}*/

	// . Find vertex range (both index and spacial ranges) for each material (needed for rendering)
	for (i=0; i<(*m_pMats).Count(); i++)
	{
		CMatInfo *mi = &(*m_pMats)[i];
		if (!mi->pRE)
			continue;
		if (mi->nNumIndices+mi->nFirstIndexId > GetIndices().Count())
		{ assert(0); continue; }
		int nMin =  999999;
		int nMax = -999999;
		Vec3d vMin;
		Vec3d vMax;
		vMin=SetMaxBB();
		vMax=SetMinBB();
		for (int j=mi->nFirstIndexId; j<mi->nNumIndices+mi->nFirstIndexId; j++)
		{
			int ind = GetIndices()[j];
			struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *pV = &pVBuff[ind];
			Vec3d v = pV->xyz;
			vMin.CheckMin(v);
			vMax.CheckMax(v);
			nMin = min(nMin, ind);
			nMax = max(nMax, ind);
		}
		mi->m_vCenter = (vMin + vMax) * 0.5f;
		mi->m_fRadius = (vMin - mi->m_vCenter).Length();
		mi->nFirstVertId = nMin;
		mi->nNumVerts = nMax-nMin+1;
	}
	
	// store resulting vertex buffer in system memory
	m_SecVertCount = buff_vert_count;
	m_pSecVertBuffer = new CVertexBuffer;
	m_pSecVertBuffer->m_vertexformat = VERTEX_FORMAT_P3F_N_COL4UB_TEX2F;
	if(m_SecVertCount)
		m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = new struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F[m_SecVertCount];
	memcpy(m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, pVBuff, m_SecVertCount*sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F));

	delete [] pVBuff;
	pVBuff=0;

	*m_pIndicesPreStrip = GetIndices();

	if(m_SecVertCount)
	{
		if (Stripify!=STRIPTYPE_NONE && !bKeepRemapTable)
			StripifyMesh(Stripify,pTmpTangBasis);
		CalcFaceNormals();
		CreateTangBuffer(pTmpTangBasis);
	}

	delete [] pTmpTangBasis;
	pTmpTangBasis=0;

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
	m_vBoxMin = pTriData->m_vBoxMin;
	m_vBoxMax = pTriData->m_vBoxMax;
}


void CSimpleLeafBuffer::CompactBuffer(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * _vbuff, int * _vcount, 
																			list2<unsigned short> * pindices, bool bShareVerts[128], uint *uiInfo,
																			CBasis * pBasises)
{
  assert(*_vcount);
  if(!*_vcount)
    m_pLog->ThrowError("CLeafBuffer::CompactBuffer error");
  
  int vert_num_before = *_vcount;

  CBasis *tmp_basis = new CBasis[*_vcount];
  SMRendTexVert *tmp_lmtc = NULL;
  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * tmp_buff = new struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F[*_vcount];
  unsigned int tmp_count = 0;
  pindices->Clear();
	TArray<uint> ShareNewInfo;

  list2<unsigned short> hash_table[256];//[256];

  for(unsigned int v=0; v<(unsigned int)(*_vcount); v++)
  {
    int nHashInd = (unsigned char)(_vbuff[v].xyz.x*100);
    uint nMInfo = uiInfo[v];
    uint nMatId = nMInfo & 255;
    int find = bShareVerts[nMatId] ? FindInBuffer( _vbuff[v], pBasises[v], nMInfo, uiInfo, tmp_buff, tmp_basis, tmp_count, &hash_table[nHashInd], ShareNewInfo/*[(unsigned char)(_vbuff[v].pos.y*100)]*/) : -1;
    if(find<0)
    { // not found
      tmp_buff[tmp_count] = _vbuff[v];
      tmp_basis[tmp_count] = pBasises[v];
      pindices->Add(tmp_count);
			ShareNewInfo.AddElem(uiInfo[v]);

      hash_table[(unsigned char)(_vbuff[v].xyz.x*100)]/*[(unsigned char)(_vbuff[v].pos.y*100)]*/.Add(tmp_count);

      tmp_count++;
    }
    else
    { // found
      pindices->Add(find);
    }
  }

  * _vcount = tmp_count;
  memcpy( _vbuff, tmp_buff, tmp_count*sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F));
  delete [] tmp_buff;

	// pBasises will contain recompacted tangents now
	memcpy(	pBasises, tmp_basis, tmp_count*sizeof(CBasis) );
  delete [] tmp_basis;

//  SAFE_DELETE_ARRAY(pBasises);

  int ratio = 100*(*_vcount)/vert_num_before;
  m_pLog->Log("  Vert buffer size after compression = %d %s ( %d -> %d )", ratio, "%", vert_num_before, *_vcount); 
}

#include "NvTriStrip/NVTriStrip.h"

void CSimpleLeafBuffer::StripifyMesh(int StripType, CBasis *pTangNonStrip)
{
  int i;
  unsigned int n;

  //Log("Stripify mesh...");
  
  ////////////////////////////////////////////////////////////////////////////////////////
  // Stripping stuff

  if (StripType == STRIPTYPE_DEFAULT)
    StripType = STRIPTYPE_ONLYLISTS;//CRenderer::CV_r_stripmesh;

  if (StripType == STRIPTYPE_NONE)
    return;

  m_pLog->Log("  Sorting vertices for GPU cache");

//  if (gRenDev->GetFeatures() & RFT_HW_GF3)
    SetCacheSize(CACHESIZE_GEFORCE3);
//  else
  //  SetCacheSize(CACHESIZE_GEFORCE1_2);
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

	CBasis *pTangOld = pTangNonStrip;
	CBasis *pTangNew = new CBasis[m_SecVertCount];

  m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = pVBNew;
  
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
    GenerateStrips(&GetIndices()[mi->nFirstIndexId], mi->nNumIndices, &pOldPG, (unsigned short*)&mi->m_dwNumSections);

    //remap!
    PrimitiveGroup *pg;
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

				//copy from old -> new tang buffer
				memcpy(&pTangNew[newVertex], &pTangOld[oldVertex], sizeof(CBasis));
//				if (pVBLMOld)
	//				memcpy(&pVBLMNew[newVertex], &pVBLMOld[oldVertex], sizeof(SMRendTexVert));

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
  
  GetIndices().Free();
  GetIndices().AddList(NewIndexes);
  
  delete [] pVBOld;

	memcpy(pTangOld,pTangNew,sizeof(CBasis)*m_SecVertCount);
	delete [] pTangNew;
}

void CSimpleLeafBuffer::CalcFaceNormals()
{
  int i, j;

  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *pV = (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F *)m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;

  if (m_nPrimetiveType != R_PRIMV_MULTI_GROUPS)
  {
    for (i=0; i<m_pMats->Count(); i++)
    {
      CMatInfo *mi = m_pMats->Get(i);
      CSimpleREOcLeaf * re = (CSimpleREOcLeaf *)mi->pRE;
      if (!re)
        continue;
      if (!re->m_Faces)
        re->m_Faces = new TArray<SMeshFace>;
      re->m_Faces->Free();
      int nOffs = mi->nFirstIndexId;
      for(j=0; j<mi->nNumIndices-2; j+=3)
	    {
		    unsigned short * face = &GetIndices()[j+nOffs];

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
//    assert(0);
    
    unsigned int n;
    for (i=0; i<m_pMats->Count(); i++)
    {
      CMatInfo *mi = m_pMats->Get(i);
      CSimpleREOcLeaf *re = (CSimpleREOcLeaf*)mi->pRE;
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
              i0 = GetIndices()[offs+n];
              i1 = GetIndices()[offs+n+1];
              i2 = GetIndices()[offs+n+2];
              break;
            case PT_STRIP:
              i0 = GetIndices()[offs+n];
              i1 = GetIndices()[offs+n+1];
              i2 = GetIndices()[offs+n+2];
              break;
            case PT_FAN:
              i0 = GetIndices()[offs+0];
              i1 = GetIndices()[offs+n+1];
              i2 = GetIndices()[offs+n+2];
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

bool CSimpleLeafBuffer::CreateTangBuffer(CBasis * pBasises)
{
//  if (!m_pBasises)
  //  PrepareTexSpaceBasis();
	assert(pBasises);

  SAFE_DELETE_ARRAY(m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData);
//  if (!m_pBasises)
  //  return false;
  m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData = new SPipTangents[m_SecVertCount];
  SPipTangents *tn = (SPipTangents *)m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData;
  for (int i=0; i<m_SecVertCount; i++)
  {
    tn[i].m_Tangent[0] = pBasises[i].tangent[0];
    tn[i].m_Tangent[1] = pBasises[i].tangent[1];
    tn[i].m_Tangent[2] = pBasises[i].tangent[2];

    tn[i].m_Binormal[0] = pBasises[i].binormal[0];
    tn[i].m_Binormal[1] = pBasises[i].binormal[1];
    tn[i].m_Binormal[2] = pBasises[i].binormal[2];

    tn[i].m_TNormal[0] = pBasises[i].tnormal[0];
    tn[i].m_TNormal[1] = pBasises[i].tnormal[1];
    tn[i].m_TNormal[2] = pBasises[i].tnormal[2];
  }

//  CorrectTangentBasisesForPolyBump();

  // Temporary basis vectors 
//  delete [] m_pBasises;
//  m_pBasises = NULL;
  return true;
}

#define PIP_TEX_EPS 0.001f
#define PIP_VER_EPS 0.001f

bool struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F::operator == (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & other)
{
  assert(this != &other);

  return fabs(xyz.x-other.xyz.x)<PIP_VER_EPS && fabs(xyz.y-other.xyz.y)<PIP_VER_EPS && fabs(xyz.z-other.xyz.z)<PIP_VER_EPS &&
    fabs(normal.x-other.normal.x)<PIP_VER_EPS && fabs(normal.y-other.normal.y)<PIP_VER_EPS && fabs(normal.z-other.normal.z)<PIP_VER_EPS &&
    fabs(st[0]-other.st[0])<PIP_TEX_EPS && fabs(st[1]-other.st[1])<PIP_TEX_EPS &&
    (color.dcolor&0xffffff) == (other.color.dcolor&0xffffff);
}

int CSimpleLeafBuffer::FindInBuffer(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F &opt, CBasis &origBasis, uint nMatInfo, uint *uiInfo, struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F* _vbuff, CBasis *_vbasis, int _vcount, list2<unsigned short> * pHash, TArray<uint>& ShareNewInfo)
{
  for(int i=0; i<pHash->Count(); i++) 
  {
    int id = (*pHash)[i];
    if(_vbuff[id] == opt) 
    {
			if (ShareNewInfo[id] != nMatInfo)
				continue;
      if (origBasis.binormal.Dot(_vbasis[id].binormal) > 0.5f && origBasis.tangent.Dot(_vbasis[id].tangent) > 0.5f)
        return (*pHash)[i];  
    }
  }

  return -1;
}

void CSimpleLeafBuffer::CorrectTangentBasisesForPolyBump( TangData * pDuplTangData )
{
  TArray <bool> bUsedVerts;

  int nBinormalStride=0, nTangentStride=0, nTNormalStride=0;
  byte * pBinormal = GetBinormalPtr(nBinormalStride, 0, true);
  byte * pTangent  = GetTangentPtr(nTangentStride, 0, true);
  byte * pTNormal  = GetTNormalPtr(nTNormalStride, 0, true);

  bUsedVerts.Reserve(GetIndices().Count());

  for(int m=0; m<m_pMats->Count(); m++)
  {
    CMatInfo *pMI = m_pMats->Get(m);
    if(!(pMI->m_Flags & MIF_POLYBUMP))
      continue; // not polybump

    bool bCloneSpace = false;
    if (pMI->shaderItem.m_pShaderResources)
    {
      SRenderShaderResources *sr = pMI->shaderItem.m_pShaderResources;
      if (sr->m_Textures[EFTT_BUMP] && sr->m_Textures[EFTT_BUMP]->m_TU.m_ITexPic && bCloneSpace)
        continue;
    }

    if (m_nPrimetiveType != R_PRIMV_MULTI_GROUPS)
    {
      int nStart = pMI->nFirstIndexId;
      int nEnd   = nStart + pMI->nNumIndices;

      assert(nEnd <= GetIndices().Count());

      for( int i = nStart; i<nEnd; i++ )
      {
        int nVertId = GetIndices()[i];
        if (bUsedVerts[nVertId])
          continue;
        bUsedVerts[nVertId] = true;
      
        Vec3d vBin, vTan, vTnor;
        if(pMI->m_Flags & MIF_INVPOLYBUMP)
        {
          vTan = Vec3d(1,0,0);
          vBin = Vec3d(0,1,0); 
          vTnor = vTan.Cross(vBin);
        }
        else
        {
          vTan = Vec3d(-1,0,0);
          vBin = Vec3d(0,1,0);    
          vTnor = vBin.Cross(vTan);
        }
        if ((UINT_PTR)pBinormal>256 && (UINT_PTR)pTangent>256)
        {
          Vec3d * vBinorm = (Vec3d *)&pBinormal[nBinormalStride*nVertId];
          Vec3d * vTang   = (Vec3d *)&pTangent [nTangentStride*nVertId];
          Vec3d * vTNormal   = (Vec3d *)&pTNormal [nTNormalStride*nVertId];

          *vBinorm  = vBin;
          *vTang    = vTan;    
          *vTNormal = vTnor;    
        }

        if (pDuplTangData)
        {
  //        int sn = pGeomInfo->m_rDupVertToNorVert[nVertId];
  //        assert(nVertId<pGeomInfo->m_nAllocatedTangNum);
          pDuplTangData[nVertId].binormal = vBin;
          pDuplTangData[nVertId].tangent  = vTan;
          pDuplTangData[nVertId].tnormal  = vTnor;
        }
      }
    }
    else
    {
      for (int j=0; j<pMI->m_dwNumSections; j++)
      {
        SPrimitiveGroup *g = &pMI->m_pPrimitiveGroups[j];
        int offs = g->offsIndex+pMI->nFirstIndexId;
        for (uint n=0; n<g->numIndices; n++)
        {
          int nVertId = GetIndices()[n+offs];
          if (bUsedVerts[nVertId])
            continue;
          bUsedVerts[nVertId] = true;
        
          Vec3d vBin, vTan, vTnor;
          if(pMI->m_Flags & MIF_INVPOLYBUMP)
          {
            vTan = Vec3d(1,0,0);
            vBin = Vec3d(0,1,0);    
            vTnor = vTan.Cross(vBin);
          }
          else
          {
            vTan = Vec3d(-1,0,0);
            vBin = Vec3d(0,1,0);    
            vTnor = vBin.Cross(vTan);
          }
          if ((UINT_PTR)pBinormal>256 && (UINT_PTR)pTangent>256)
          {
            Vec3d * vBinorm = (Vec3d *)&pBinormal[nBinormalStride*nVertId];
            Vec3d * vTang   = (Vec3d *)&pTangent [nTangentStride*nVertId];
            Vec3d * vTNormal   = (Vec3d *)&pTNormal [nTNormalStride*nVertId];

            *vBinorm  = vBin;
            *vTang    = vTan;    
            *vTNormal = vTnor;    
          }

          if (pDuplTangData)
          {
    //        int sn = pGeomInfo->m_rDupVertToNorVert[nVertId];
    //        assert(nVertId<pGeomInfo->m_nAllocatedTangNum);
            pDuplTangData[nVertId].binormal = vBin;
            pDuplTangData[nVertId].tangent  = vTan;
            pDuplTangData[nVertId].tnormal  = vTnor;
          }
        }
      }
    }
  }
  bUsedVerts.Free();
}
