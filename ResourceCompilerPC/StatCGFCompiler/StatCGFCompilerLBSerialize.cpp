////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   LeafBufferSerialize.cpp
//  Version:     v1.00
//  Created:     28/8/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: LeafBuffer serialization
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

#include "meshidx.h"
#include "IShader.h"
#include "NvTriStrip\NvTriStrip.h"

#include "SerializeBuffer.h"

CryIRGB CF2IRGB(CFColor in)
{
  CryIRGB out;
  out.r = uchar(in.r*255);
  out.g = uchar(in.g*255);
  out.b = uchar(in.b*255);
  return out;
}

char *SkipPath (char *pathname)
{
  char  *last;

  last = pathname;
  while (*pathname)
  {
    if (*pathname=='/' || *pathname=='\\')
      last = pathname+1;
    pathname++;
  }
  return last;
}

int CSimpleLeafBuffer__SetTexType(TextureMap3 *tm)
{
  if (tm->type == TEXMAP_CUBIC)
    return eTT_Cubemap;
  else
  if (tm->type == TEXMAP_AUTOCUBIC)
    return eTT_AutoCubemap;
  return eTT_Base;
}

bool CSimpleLeafBuffer::Serialize(int & nPos, uchar * pSerBuf, bool bSave, const char * szFolderName)
{
  assert(bSave);

  SaveBuffer("LeafBuffer", 11, pSerBuf, nPos);

  SaveBuffer(&m_SecVertCount,                   sizeof(m_SecVertCount),                                        pSerBuf, nPos);

  SaveBuffer( m_arrVertStripMap,                m_SecVertCount*sizeof(m_arrVertStripMap[0]),                   pSerBuf, nPos);
  GetIndices().SaveToBuffer(pSerBuf, nPos);
  m_pIndicesPreStrip->SaveToBuffer(pSerBuf, nPos);
  SaveBuffer(&m_nPrimetiveType,                 sizeof(m_nPrimetiveType),                                      pSerBuf, nPos);
//  assert(m_pBasises==0); // not needed
  SaveBuffer( m_pLoadedColors,                  m_SecVertCount*sizeof(m_pLoadedColors[0]),                     pSerBuf, nPos);
  m_pMats->SaveToBuffer(pSerBuf, nPos); // need to restore

  // save shader info
  for (int i=0; i<m_pMats->Count(); i++)
  {
    MAT_ENTITY * pMatEnt = m_pMats->Get(i)->pMatEnt;
    SaveBuffer(pMatEnt, sizeof(*pMatEnt), pSerBuf, nPos);

    if(m_pMats->GetAt(i).pRE)
    {
      assert(((CSimpleREOcLeaf*)m_pMats->GetAt(i).pRE)->m_pChunk->nNumIndices);
      assert(((CSimpleREOcLeaf*)m_pMats->GetAt(i).pRE)->m_pChunk == m_pMats->Get(i));
    }

		// save primitive groups
    if(m_pMats->GetAt(i).m_dwNumSections)
			SaveBuffer((void*)m_pMats->GetAt(i).m_pPrimitiveGroups, 
      sizeof(SPrimitiveGroup)*m_pMats->GetAt(i).m_dwNumSections, 
      pSerBuf, nPos);
  }

  SaveBuffer( m_pSecVertBuffer,                 sizeof(*m_pSecVertBuffer),                                     pSerBuf, nPos); // need to restore
  SaveBuffer( m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData,      m_SecVertCount*m_VertexSize[m_pSecVertBuffer->m_vertexformat], pSerBuf, nPos);
  SaveBuffer( m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData,     m_SecVertCount*sizeof(SPipTangents),                           pSerBuf, nPos);

  assert(!m_pVertexBuffer); // not needed
  SaveBuffer(&m_vBoxMax,                        sizeof(m_vBoxMax),                                             pSerBuf, nPos);
  SaveBuffer(&m_vBoxMin,                        sizeof(m_vBoxMin),                                             pSerBuf, nPos);

	// [Anton] m_arrVtxMap serialization
	int bHasVtxMap;
	if (m_arrVtxMap)
	{
		SaveBuffer(&(bHasVtxMap=1),sizeof(bHasVtxMap), pSerBuf,nPos);
		SaveBuffer(m_arrVtxMap,sizeof(uint)*m_SecVertCount, pSerBuf,nPos);
	}
	else
		SaveBuffer(&(bHasVtxMap=0),sizeof(bHasVtxMap), pSerBuf,nPos);
	
  return 0;
}
