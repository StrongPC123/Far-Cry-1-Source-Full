////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjconstr.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: loading
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "MeshIdx.h"
#include "serializebuffer.h"
#include "objman.h"

bool CStatObj::Serialize(int & nPos, uchar * pSerBuf, bool bSave, char * szFolderName)
{
  char szSignature[16];
  if(!LoadBuffer(szSignature, 8, pSerBuf, nPos) || strcmp(szSignature,"StatObj"))
    return false;

  for(int i=0; i<3; i++)
  {
    m_lstProxyVerts[i].LoadFromBuffer(pSerBuf,nPos);
    m_lstProxyInds[i].LoadFromBuffer(pSerBuf,nPos);
    LoadBuffer(m_vPhysBoxMin[i], sizeof(m_vPhysBoxMin[i]), pSerBuf, nPos);
    LoadBuffer(m_vPhysBoxMax[i], sizeof(m_vPhysBoxMax[i]), pSerBuf, nPos);
    m_lstProxyFaceMaterials[i].LoadFromBuffer(pSerBuf,nPos);
  }

  LoadBuffer(&m_vBoxMin, sizeof(m_vBoxMin), pSerBuf, nPos);
  LoadBuffer(&m_vBoxMax, sizeof(m_vBoxMax), pSerBuf, nPos);

	assert(m_pSvObj	== 0x00000000);

  m_lstHelpers.LoadFromBuffer(pSerBuf,nPos);
  m_lstLSources.LoadFromBuffer(pSerBuf,nPos);

	// parse name and and load light shader if needed
	for(int i=0; i<m_lstLSources.Count(); i++)
		InitCompiledLightSource(&m_lstLSources[i]);

  m_pLeafBuffer = GetRenderer()->CreateLeafBuffer(eBT_Static,"CompiledStatObj");
  m_pLeafBuffer->Serialize(nPos, pSerBuf, bSave, m_szFolderName, m_szFileName, CObjManager::m_dCIndexedMesh__LoadMaterial);

  SetShadowVolume(CStatObj::MakeConnectivityInfoFromCompiledData(pSerBuf, nPos, this));

	return true;
}

void CStatObj::InitCompiledLightSource(CDLight * pDLight)
{
	char *shName = NULL;
	char *str;
	char name[128];
	char nameTgt[128];
	strncpy(name, pDLight->m_Name, sizeof(name));
	str = strstr(name, "->");
	if (str)
	{
		name[str-name] = 0;
		strncpy(nameTgt, &str[2], sizeof(nameTgt));
	}
	else
		nameTgt[0] = 0;
	if(str=strchr(name, '('))
	{
		name[str-name] = 0;
		shName = &name[str-name+1];
		if(str=strchr(shName, ')'))
			shName[str-shName] = 0;
	}

	//	strcpy(pDLight->m_Name, name);

	if (nameTgt[0])
	{
		//      strcpy(pDLight->m_TargetName, nameTgt);
	}
	pDLight->m_Flags &= ~(DLF_LIGHTSOURCE | DLF_HEATSOURCE);
	if (!pDLight->Parse())
		pDLight->m_Flags |= DLF_LIGHTSOURCE;
	else
	{
		if (!strncmp(pDLight->m_Name, "local_hs", 8))
		{
			pDLight->m_fDirectFactor = 0;
			pDLight->m_Flags |= DLF_LOCAL;
		}
	}
	if (shName)
	{
		pDLight->m_pShader = GetRenderer()->EF_LoadShader(shName, eSH_Misc);
		if (pDLight->m_pShader!=0 && (pDLight->m_pShader->GetFlags() & EF_NOTFOUND))
		{
			pDLight->m_pShader->Release();
			pDLight->m_pShader = NULL;
			pDLight->m_Flags |= DLF_FAKE;
#if !defined(LINUX)
			Warning(0,m_szFileName,
				"Error: CIndexedMesh::MakeLightSources: Shader %s not found for lsource %s", 
				shName, pDLight->m_Name);     
#endif
		}

		if (pDLight->m_pShader!=0 && (pDLight->m_pShader->GetLFlags() & LMF_DISABLE))
			pDLight->m_Flags |= DLF_FAKE;
	}

	pDLight->MakeBaseParams();
}
