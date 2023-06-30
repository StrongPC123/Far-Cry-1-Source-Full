////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   meshidx.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: prepare shaders for cfg object
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "geom.h"
#include "CryStaticModel.h"
#include "StatCGFCompiler.h"

#define ISystem IRCLog

#include "meshidx.h"
#include "MakMatInfoFromMAT_ENTITY.h"

static void StripExtension (const char *in, char *out)
{
  char c;

  while (*in)
  {
    if (*in=='.')
    {
      c = in[1];
      if (c!='.' && c!='/' && c!='\\')
        break;
    }
    *out++ = *in++;
  }
  *out = 0;
}

static char *SkipPath (char *pathname)
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

//////////////////////////////////////////////////////////////////////////
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
		mi->pRE=0;
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
	delete m_pTangBasis;
}

bool CIndexedMesh::LoadMaterial(const char *szFileName, const char *szFolderName, 
                                CMatInfo & newMat, IRenderer * pRenderer, MAT_ENTITY * me)
{
  return CIndexedMesh__LoadMaterial(szFileName, szFolderName, newMat, pRenderer, me);
}
/*
bool CIndexedMesh::SearchTargetLight(CDLight *l)
{
  int j;

  if (l->m_TargetName[0])
  {
    for (j=0; j<m_tgtLSources.Count(); j++)
    {
      if (l == m_tgtLSources[j] || !m_tgtLSources[j]->m_Name[0])
        continue;
      if (!stricmp(l->m_TargetName, m_tgtLSources[j]->m_Name))
      {
        l->m_NextLight = m_tgtLSources[j];
        break;
      }
    }
    if (j == m_tgtLSources.Count())
    {
      for (j=0; j<m_lstLSources.Count(); j++)
      {
        if (l == m_lstLSources[j] || !m_lstLSources[j]->m_Name[0])
          continue;
        if (!stricmp(l->m_TargetName, m_lstLSources[j]->m_Name))
        {
          l->m_NextLight = m_lstLSources[j];
          break;
        }
      }
      if (j == m_lstLSources.Count())
      {
        m_pSystem->Log("Warning: Couldn't find target light '%s' for light source '%s'", l->m_TargetName, l->m_Name);
        return false;
      }
    }
  }
  return true;
}*/

//int nLoadedObjNum=0;

bool CIndexedMesh::LoadCGF(const char*szFileName, const char * szGeomName, bool bLoadAdditinalInfo, bool bKeepInLocalSpace, bool bIgnoreFakeMats)
{
  //reset
  m_pFaces=0;
  m_pVerts=0;
  m_pCoors=0;
  m_pNorms=0;
  m_pColor=0;	

  m_nFaceCount = m_nVertCount = m_nCoorCount = m_nNormCount  = 0;

  // load data from file into CryStaticModel  
  bool bReset = false;
  //  if(true || strcmp(m_szStaticLoadedFileName,szFileName) || m_bKeepInLocalSpace != bKeepInLocalSpace)
  //  { // if we load new object - reload file and materials
  //    strcpy(m_szStaticLoadedFileName,szFileName);
  //	m_bKeepInLocalSpace = bKeepInLocalSpace;

  bReset = true;
  //    m_LoadedShaders.Reset();

  //    delete pStaticCGF;
  CryStaticModel * pStaticCGF = new CryStaticModel();
  pStaticCGF->m_pLog = m_pSystem;
  if(!pStaticCGF->OnLoadgeom((char*)szFileName, 0, bReset, bKeepInLocalSpace))
  {
    delete pStaticCGF;
    pStaticCGF=0;
    //			m_szStaticLoadedFileName[0]=0;
    return 0;
  }
  //  }
  /*	else
  {
  int i=0;
  }*/

  if(bLoadAdditinalInfo)
  {
    MakeLightSources(pStaticCGF);

    // get helpers
    for(int h=0; h<pStaticCGF->m_lstHelpers.Count(); h++)
    {
      char *szName = pStaticCGF->m_lstHelpers[h].szName;
      char *str;
      char *szShadName = NULL;
      if(str=strchr(szName, '('))
      {
        szName[str-szName] = 0;
        szShadName = &szName[str-szName+1];
        if(str=strchr(szShadName, ')'))
          szShadName[str-szShadName] = 0;
      }
      IShader *pShader = NULL;
      if (szShadName)
        pShader ;//= m_pSystem->GetIRenderer()->EF_LoadShader(szShadName, eSH_World, 0);

      StatHelperInfo hi = StatHelperInfo(//pStaticCGF->m_lstHelpers[h].vPos,
        //pStaticCGF->m_lstHelpers[h].qRot,
        pStaticCGF->m_lstHelpers[h].szName,
        pStaticCGF->m_lstHelpers[h].Chunk.type,
        pShader, pStaticCGF->m_lstHelpers[h].tMat);

      m_lstHelpers.Add(hi);
    }

    // get geom names
    int g;
    for(g=0;  g<pStaticCGF->m_lstGeomNames.Count(); g++)
    {
      char * str = new char[strlen(pStaticCGF->m_lstGeomNames[g].name)+1];
      strcpy(str, (const char *)pStaticCGF->m_lstGeomNames[g].name);
      m_lstGeomNames.Add(str);
    }
  }

  if(pStaticCGF->m_lstMaterials.Count()==0)
    m_pSystem->Log("Warning: Materials not found in cgf file: %s", szFileName);

  bool bColorsFound = false;


  char szFolderName[1024];
  { // make folder name to create full path    
    strncpy(szFolderName, szFileName, sizeof(szFolderName));
    size_t nStringLast = strlen(szFolderName)-1;
    while(szFolderName[0])
    {
      if (szFolderName[nStringLast] == '\\' || szFolderName[nStringLast] == '/')
        break;
      else
      {
        szFolderName[nStringLast]='\0';
        nStringLast--;
      }        
    }
  }

  bool bFacesWithInvalidMatIdFoundInThisFile = false;

  // merge geoms
  for( int g=0; g<pStaticCGF->m_lstGeoms.Count(); g++)
  {
    CGeom * geom = pStaticCGF->m_lstGeoms[g];

    if(szGeomName && !geom)
      continue; // load only requested geom

    if(!geom)
    {  m_pSystem->Log("CIndexedMesh::LoadCGF error"); break; }

    if(szGeomName && strcmp(pStaticCGF->m_lstGeomNames[g].name,szGeomName))
      continue;

    MESH_CHUNK_DESC	* chunk = geom->GetChunk();

#ifdef _DEBUG
    { // data valid check
      bool error = 0;
      for( int c=0; c<chunk->nFaces && geom->m_pTexFaces; c++)
      {
        if(geom->m_pTexFaces[c].t0<geom->m_Chunk.nTVerts)
          if(geom->m_pTexFaces[c].t1<geom->m_Chunk.nTVerts)
            if(geom->m_pTexFaces[c].t2<geom->m_Chunk.nTVerts)
              continue;

        m_pSystem->Log("Error in geometry data %s (%s)", pStaticCGF->m_lstGeomNames.Count() ? pStaticCGF->m_lstGeomNames[g].name : "noname", szGeomName);
        error = true;
        break;
      }
      if(error)
        continue;
    }
#endif

    if(chunk->nTVerts)
    { // realloc texcoords
      m_pCoors = (TexCoord*)ReAllocElements(m_pCoors, m_nCoorCount, m_nCoorCount+chunk->nTVerts, sizeof(TexCoord));

      for (int c=0; c<chunk->nTVerts; c++)
      {
        m_pCoors[c+m_nCoorCount].s = geom->m_pUVs[c].u;
        m_pCoors[c+m_nCoorCount].t = geom->m_pUVs[c].v;
      } //c
    } //texcoords

    { // realloc verts and normals
      m_pVerts = (Vec3d*)ReAllocElements(m_pVerts, m_nVertCount, m_nVertCount+chunk->nVerts, sizeof(Vec3d));
      m_pNorms = (Vec3d*)ReAllocElements(m_pNorms, m_nNormCount, m_nNormCount+chunk->nVerts, sizeof(Vec3d));

      //      if(geom->m_pVcols)
      m_pColor = (UColor*)ReAllocElements(m_pColor, m_nVertCount, m_nVertCount+chunk->nVerts, sizeof(UColor));

      for( int c=0; c<chunk->nVerts; c++)
      {
        m_pVerts[c+m_nVertCount].x = geom->m_pVertices[c].p.x;
        m_pVerts[c+m_nVertCount].y = geom->m_pVertices[c].p.y;
        m_pVerts[c+m_nVertCount].z = geom->m_pVertices[c].p.z;

        m_pNorms[c+m_nVertCount].x = geom->m_pVertices[c].n.x;
        m_pNorms[c+m_nVertCount].y = geom->m_pVertices[c].n.y;
        m_pNorms[c+m_nVertCount].z = geom->m_pVertices[c].n.z;        

        if(geom->m_pVcols)
        {
          m_pColor[c+m_nVertCount].r = geom->m_pVcols[c].r;
          m_pColor[c+m_nVertCount].g = geom->m_pVcols[c].g;
          m_pColor[c+m_nVertCount].b = geom->m_pVcols[c].b;
          m_pColor[c+m_nVertCount].a = 255;
          bColorsFound=true;
        }
        else
        {
          m_pColor[c+m_nVertCount].r = 255;
          m_pColor[c+m_nVertCount].g = 255;
          m_pColor[c+m_nVertCount].b = 255;
          m_pColor[c+m_nVertCount].a = 255;
        }
      }
    }

    { // realloc faces
      m_pFaces = (CObjFace*)ReAllocElements(m_pFaces, m_nFaceCount, m_nFaceCount+chunk->nFaces, sizeof(CObjFace));

      //			CXFile * fp = NULL;
      //			std::vector<int> lstLMapsId;
      //		std::vector<int> lstLMapsId_LD;

      //try to load lightmaps only if there is an
      //object file specified AND lightmaps are enabled
      /*if (0 && szGeomName && bLoadLightmaps)
      {	
      // Create subdirectory path with name of the building
      char szLMFolderName[256];
      StripExtension(szFileName, szLMFolderName);
      UINT iLen = strlen(szLMFolderName);
      szLMFolderName[iLen] = '\\';
      szLMFolderName[iLen + 1] = '\0';

      //check if a lightmap file does exist			
      //TODO: Make this file check per .CGF file,store all infos for each object in one file,
      //not on a per-object basis

      fp = new CXFile;
      char szTempFilename[512];
      snprintf(szTempFilename, sizeof(szTempFilename), "%s\\-%s",szLMFolderName,szGeomName+1);

      // preload all data
      if (!fp->FLoad(szTempFilename))
      {
      delete fp;
      fp = NULL;
      }

      if (fp)
      { 
      //read number of faces
      int nNumFaces;
      fp->FRead(&nNumFaces,sizeof(int),1);
      if (nNumFaces!=chunk->nFaces)
      {
      m_pSystem->Log("Error: face count stored in %s is %d, real size=%d",szTempFilename,nNumFaces,m_nFaceCount);
      fp->FClose();
      delete fp;
      fp=NULL;
      }
      else
      {
      //load for every object light texture infos...						
      snprintf(szTempFilename, sizeof(szTempFilename), "%s\\light.dat",szLMFolderName,szGeomName);						
      FILE * fp2 = f xopen(szTempFilename,"rb");
      if (fp2)
      {
      int nLmapNum=0;
      f read(&nLmapNum,sizeof(int),1,fp2); //number of textures
      if (!nLmapNum)
      {
      //if the texture information doesn't exist, close the previous file							
      fp->FClose();
      delete fp;
      fp=NULL;								
      }
      else
      {
      lstLMapsId.reserve(nLmapNum);
      lstLMapsId_LD.reserve(nLmapNum);
      char szTextName[256];
      for (int k=0;k<nLmapNum;k++)
      {
      f read(szTextName,sizeof(char),256,fp2);									
      //Marco's NOTE: Andrey add here the function to get lm id
      int nId = m_pSystem->GetIRenderer()->EF_LoadLightmap(szTextName);
      lstLMapsId.push_back(nId);
      char szNewTexName[256];
      StripExtension(szTextName, szNewTexName);
      strcat(szNewTexName, "_ld.tga");
      nId = m_pSystem->GetIRenderer()->EF_LoadLightmap(szNewTexName);
      lstLMapsId_LD.push_back(nId);
      } //k
      }							

      fc lose(fp2);
      }
      else
      {
      //if the texture file information doesn't exist, close the previous file							
      fp->FClose();
      delete fp;
      fp=NULL;
      }
      }	//numfaces
      } //fp
      } //lmaps*/

      for( int c=0; c<chunk->nFaces; c++)
      {
        memset(&m_pFaces[c+m_nFaceCount],0,sizeof(CObjFace));

        CObjFace * pFace = &m_pFaces[c+m_nFaceCount];

        /*if (fp) 
        {
        //if lightmap file does exist, allocate temporary space for 
        //lightmaps texture coords and texture id					
        pFace->m_lInfo=new tLmInfo;	
        pFace->m_lInfo->nTextureIdLM_LD = 0;

        //read datas from memory 					
        fp->FRead(&pFace->m_lInfo->nTextureIdLM,sizeof(unsigned short),1);
        //convert to the correct one
        if (pFace->m_lInfo->nTextureIdLM!=65535)
        {
        int n = pFace->m_lInfo->nTextureIdLM;
        pFace->m_lInfo->nTextureIdLM=lstLMapsId[n];
        pFace->m_lInfo->nTextureIdLM_LD=lstLMapsId_LD[n];
        }

        for (int v=0;v<3;v++)
        {
        fp->FRead(&pFace->m_lInfo->fS[v],sizeof(float),1);
        fp->FRead(&pFace->m_lInfo->fT[v],sizeof(float),1);
        }

        if (pFace->m_lInfo->nTextureIdLM==65535) //no lightmap
        {
        delete pFace->m_lInfo;
        pFace->m_lInfo=NULL;
        }
        }
        else*/
//        pFace->m_lInfo=NULL;

        m_pFaces[c+m_nFaceCount].v[0] = m_pFaces[c+m_nFaceCount].n[0] = geom->m_pFaces[c].v0+m_nVertCount;
        m_pFaces[c+m_nFaceCount].v[1] = m_pFaces[c+m_nFaceCount].n[1] = geom->m_pFaces[c].v1+m_nVertCount;
        m_pFaces[c+m_nFaceCount].v[2] = m_pFaces[c+m_nFaceCount].n[2] = geom->m_pFaces[c].v2+m_nVertCount;

        if(geom->m_pTexFaces)
        {
          assert(
            geom->m_pTexFaces[c].t0<geom->m_Chunk.nTVerts &&
            geom->m_pTexFaces[c].t1<geom->m_Chunk.nTVerts &&
            geom->m_pTexFaces[c].t2<geom->m_Chunk.nTVerts);
        }

        m_pFaces[c+m_nFaceCount].t[0] = geom->m_pTexFaces ? geom->m_pTexFaces[c].t0+m_nCoorCount : (geom->m_pFaces[c].v0+m_nCoorCount);
        m_pFaces[c+m_nFaceCount].t[1] = geom->m_pTexFaces ? geom->m_pTexFaces[c].t1+m_nCoorCount : (geom->m_pFaces[c].v1+m_nCoorCount);
        m_pFaces[c+m_nFaceCount].t[2] = geom->m_pTexFaces ? geom->m_pTexFaces[c].t2+m_nCoorCount : (geom->m_pFaces[c].v2+m_nCoorCount);

        m_pFaces[c+m_nFaceCount].shader_id = geom->m_pFaces[c].MatID;

        if(m_pFaces[c+m_nFaceCount].shader_id>=pStaticCGF->m_lstMaterials.Count() && pStaticCGF->m_lstMaterials.Count())
        {
          if(!bFacesWithInvalidMatIdFoundInThisFile)
          {
            if(m_pFaces[c+m_nFaceCount].shader_id == (unsigned short)-1)
            {
              m_pSystem->Log(
                "Warning: CIndexedMesh::LoadCGF: Some faces has undefined material."
                " File name is %s%s%s.",
                szFileName, 
                szGeomName ? ", object name is " : "",  
                szGeomName ? szGeomName : "");
            }
            else
            {
              m_pSystem->Log(
                "Warning: CIndexedMesh::LoadCGF: Some faces are referencing to material which is not present in the file."
                " File name is %s%s%s."
                " Face material id is %d, number of materials in file is %d.",

                szFileName, 
                szGeomName ? ", object name is " : "",  
                szGeomName ? szGeomName : "",

                m_pFaces[c+m_nFaceCount].shader_id, 
                pStaticCGF->m_lstMaterials.Count());
            }

            bFacesWithInvalidMatIdFoundInThisFile = true;
          }
          m_pFaces[c+m_nFaceCount].shader_id = 0;
        }

        if(m_pFaces[c+m_nFaceCount].shader_id > 1000)
        {
          m_pSystem->Log("Warning: CIndexedMesh::LoadCGF: Face has invalid shader_id");
          m_pFaces[c+m_nFaceCount].shader_id = 0;
        }

        /*if (pStaticCGF->m_lstMaterials.Count())
        {
        int nLM = pFace->m_lInfo ? pFace->m_lInfo->nTextureIdLM : -1; 
        int nLM_LD = pFace->m_lInfo ? pFace->m_lInfo->nTextureIdLM_LD : -1; 
        int ns;
        for (ns=0; ns<m_LoadedShaders.Num(); ns++)
        {
        if (m_LoadedShaders[ns].nShaderID == pFace->shader_id )//&& 
        //                m_LoadedShaders[ns].nLMTexID == nLM &&
        //              m_LoadedShaders[ns].nLMTexID_LD == nLM_LD)
        break;
        }
        if (ns == m_LoadedShaders.Num())
        {
        SLoadedShader ls;
        ls.pMesh = this;
        ls.nShaderID = pFace->shader_id;
        //            ls.nLMTexID = nLM;
        //          ls.nLMTexID_LD = nLM_LD;
        if(!LoadShaderForFace(pFace, pStaticCGF, szFileName, szFolderName))
        m_pSystem->Log("Error: CIndexedMesh::LoadCGF: LoadShaderForFace returns false");
        ls.nNewShaderID = pFace->shader_id;
        ls.MatInfo = m_lstMatTable.Last();
        m_LoadedShaders.AddElem(ls);
        }
        else
        {                                         // happends if we load same object second time
        if(m_LoadedShaders[ns].pMesh == this && m_LoadedShaders[ns].nNewShaderID<m_lstMatTable.Count())
        { // if we still in the same object
        pFace->shader_id = m_LoadedShaders[ns].nNewShaderID;
        assert(pFace->shader_id<m_lstMatTable.Count());
        }
        else
        { // try to find id of such already registered material
        int f=0;
        for(f=0; f<m_lstMatTable.Count(); f++)
        {
        if(memcmp(&m_lstMatTable[f],&m_LoadedShaders[ns].MatInfo,sizeof(CMatInfo))==0)
        break;
        }

        if(f<m_lstMatTable.Count())
        pFace->shader_id = f;
        else
        { // add new if not found
        pFace->shader_id = m_lstMatTable.Count();
        m_lstMatTable.Add(m_LoadedShaders[ns].MatInfo);
        }
        }
        }
        }*/

        // check vertex id
        assert(m_pFaces[c+m_nFaceCount].v[0] < m_nVertCount+chunk->nVerts);
        assert(m_pFaces[c+m_nFaceCount].v[1] < m_nVertCount+chunk->nVerts);
        assert(m_pFaces[c+m_nFaceCount].v[2] < m_nVertCount+chunk->nVerts);
      }

      //			lstLMapsId.clear();
      //		lstLMapsId_LD.clear();
      /*			if (fp)
      {
      fp->FClose();
      delete fp;
      fp=NULL; //for the sake of clarity
      }*/
    }

    // sum number of elements
    m_nFaceCount  += chunk->nFaces;
    m_nVertCount  += chunk->nVerts;
    m_nCoorCount  += chunk->nTVerts;
    m_nNormCount  += chunk->nVerts;
  }

  if(!bColorsFound)
  {
    free(m_pColor);
    m_pColor=0;
  }

  // mark really used materials
  byte arrMats[128];
  memset(arrMats,0,sizeof(arrMats));
  for(int f=0; f<m_nFaceCount; f++)
    arrMats[m_pFaces[f].shader_id&127] = true;

  for(int nMat=0; nMat<pStaticCGF->m_lstMaterials.Count(); nMat++)
  {
    CMatInfo MatInfo;

    if(arrMats[nMat&127] && pStaticCGF->m_lstMaterials[nMat].IsStdMat)
    {
      LoadMaterial( szFileName, szFolderName, MatInfo, NULL, &pStaticCGF->m_lstMaterials[nMat]);
			
			// this object contains all info about material
			// The Martin
			//pStaticCGF->m_lstMaterials[nMat].
    }

    m_lstMatTable.Add(MatInfo);
  }

  if(!szGeomName)
  {
    m_pSystem->Log("  %d faces, %d verts, %d texcoord", m_nFaceCount, m_nVertCount, m_nCoorCount);
    m_pSystem->Log("  %d geoms, %d mats", pStaticCGF->m_lstGeoms.Count(), m_lstMatTable.Count());

    //    if(m_lstMatTable.Count()>8)
    //    m_pSystem->Log("Warning: number of materials is %d", m_lstMatTable.Count());
  }

  if(!szGeomName && m_lstLSources.Count())
    m_pSystem->Log("  %d light sources", m_lstLSources.Count());

  if(m_lstMatTable.Count()==0)
  {
    for(int fc=0; fc<m_nFaceCount; fc++)
      m_pFaces[fc].shader_id = 0;

    CMatInfo newMat;
    newMat.shaderItem.m_pShader;// = m_pSystem->GetIRenderer()->EF_LoadShader("default", eSH_World);
    m_lstMatTable.Add(newMat);
  }

	// Free memory.
	delete pStaticCGF;

  return true;
}

void CIndexedMesh::MakeLightSources(CryStaticModel * pStaticCGF)
{ // get light sources
  m_lstLSources.Clear();
  m_tgtLSources.Clear();
  for(int l=0; l<pStaticCGF->m_lstLights.Count(); l++)
  {
    CDLight *lsource;
    lsource = new CDLight;

    //lsource->m_Origin.Set(&pStaticCGF->m_lstLights[l].vPos.x);
    lsource->m_Origin.Set( pStaticCGF->m_lstLights[l].vPos[0], pStaticCGF->m_lstLights[l].vPos[1], pStaticCGF->m_lstLights[l].vPos[2]);

    lsource->m_vObjectSpacePos = lsource->m_Origin;

    lsource->m_Color.r = pStaticCGF->m_lstLights[l].Chunk.color.r / 255.0f;
    lsource->m_Color.g = pStaticCGF->m_lstLights[l].Chunk.color.g / 255.0f;
    lsource->m_Color.b = pStaticCGF->m_lstLights[l].Chunk.color.b / 255.0f;
    lsource->m_Color.a = 1.0f;
    lsource->m_SpecColor = lsource->m_Color;

    lsource->m_fRadius = pStaticCGF->m_lstLights[l].Chunk.attenEnd;
    // all light sources should have a radius, to be able to cull objects with
    if (lsource->m_fRadius<=0)
      lsource->m_fRadius=10;

    /*		if (pStaticCGF->m_lstLights[l].Chunk.szLightImage[0]!=0)
    {
    lsource->m_pLightImage = GetRenderer()->EF_LoadTexture(pStaticCGF->m_lstLights[l].Chunk.szLightImage, FT_CLAMP, FT2_NODXT | FT2_FORCECUBEMAP, eTT_Cubemap); 
    if (!lsource->m_pLightImage->IsTextureLoaded())
    lsource->m_pLightImage = NULL;
    }
    else				
    lsource->m_pLightImage = NULL;*/

    //calc the light orientation
    Vec3d Angs = pStaticCGF->m_lstLights[l].Chunk.vDirection / gf_PI * 180.0f;
    Vec3d a;
    a[0] =   Angs[1]+90.0f;
    a[1] = -(Angs[0]-90.0f);
    a[2] =   Angs[2]+90.0f;
    lsource->m_ProjAngles = a;
    lsource->m_Orientation.rotate(Vec3d(1,0,0), a[0]);
    lsource->m_Orientation.rotate(Vec3d(0,1,0), a[1]);
    lsource->m_Orientation.rotate(Vec3d(0,0,1), a[2]);

    lsource->m_pLightImage = pStaticCGF->m_lstLights[l].pLightImage;

    //check if the light has a proj text
    if (lsource->m_pLightImage)
    {
      lsource->m_Flags |= DLF_PROJECT;
    }
    else
      if (pStaticCGF->m_lstLights[l].Chunk.type == LT_OMNI)
      {
        if (pStaticCGF->m_lstLights[l].Chunk.useAtten)
          lsource->m_Flags |= DLF_POINT;
        else
          lsource->m_Flags |= DLF_DIRECTIONAL;
      }
      else
        if (pStaticCGF->m_lstLights[l].Chunk.type == LT_DIRECT)
          lsource->m_Flags |= DLF_DYNAMIC | DLF_DIRECTIONAL;
        else
          lsource->m_Flags |= DLF_DIRECTIONAL; //should never happen
    //if (lsource->m_Flags & DLF_DIRECTIONAL)
    //  lsource->m_fRadius *= 1000.0f;

    lsource->m_fLightFrustumAngle = pStaticCGF->m_lstLights[l].Chunk.fallsize;

    //the following adjustment to the frustum angle is done to match 3dsMax settings		
    lsource->m_fLightFrustumAngle-=6.0f;

    //doesnt make sense less than 1
    if (lsource->m_fLightFrustumAngle < 1.0f)
      lsource->m_fLightFrustumAngle = 1.0f;
    //the texture will be stretched too much
    if (lsource->m_fLightFrustumAngle > 88.0f)
      lsource->m_fLightFrustumAngle = 88.0f;

		// parse this name in engine
		strcpy(lsource->m_Name, pStaticCGF->m_lstLights[l].szName);

/*
    char *shName = NULL;
    char *str;
    char name[128];
    char nameTgt[128];
    strcpy(name, pStaticCGF->m_lstLights[l].szName);
    str = strstr(name, "->");
    if (str)
    {
      name[str-name] = 0;
      strcpy(nameTgt, &str[2]);
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
    strcpy(lsource->m_Name, name);
    if (nameTgt[0])
    {
//      strcpy(lsource->m_TargetName, nameTgt);
    }
    lsource->m_Flags &= ~(DLF_LIGHTSOURCE | DLF_HEATSOURCE);
    if (!lsource->Parse())
      lsource->m_Flags |= DLF_LIGHTSOURCE;
    else
    {
      if (!strncmp(lsource->m_Name, "local_hs", 8))
      {
        lsource->m_fDirectFactor = 0;
        lsource->m_Flags |= DLF_LOCAL;
      }
    }
    if (shName)
    {
      lsource->m_pShader=0;// = m_pSystem->GetIRenderer()->EF_LoadShader(shName, eSH_Misc);
      if (lsource->m_pShader && (lsource->m_pShader->GetFlags() & EF_NOTFOUND))
      {
        lsource->m_pShader->Release();
        lsource->m_pShader = NULL;
        lsource->m_Flags |= DLF_FAKE;
        m_pSystem->Log("Error: CIndexedMesh::MakeLightSources: Shader %s not found for lsource %s", shName, pStaticCGF->m_lstLights[l].szName);     
        continue;
      }
      if (lsource->m_pShader && (lsource->m_pShader->GetLFlags() & LMF_DISABLE))
        lsource->m_Flags |= DLF_FAKE;
    }
*/
    if (!strnicmp(lsource->m_Name, "tgt", 3))
      m_tgtLSources.Add(lsource);
    else
    {
      CDLight * dlOrig = new CDLight;
      *dlOrig = *lsource;
//      lsource->m_OrigLight = dlOrig;
      m_lstLSources.Add(lsource);
    }
  }

//  int i;
/*  for (i=0; i<m_lstLSources.Count(); i++)
  {
    CDLight *l = m_lstLSources[i];
    if (l->m_TargetName[0])
      SearchTargetLight(l);
  }
  for (i=0; i<m_tgtLSources.Count(); i++)
  {
    CDLight *l = m_tgtLSources[i];
    if (l->m_TargetName[0])
      SearchTargetLight(l);
  }*/
/*  for (i=0; i<m_lstLSources.Count(); i++)
  {
    CDLight *l = m_lstLSources[i];
    CDLight *dl = l;
    int n = 0;
    while (dl)
    {
      dl = dl->m_NextLight;
      n++;
      if (dl == l)
        break;
    }
    l->m_NumLights = n;
  }*/
}

void * CIndexedMesh::ReAllocElements(void * old_ptr, int old_elem_num, int new_elem_num, int size_of_element)
{
  //  return realloc(old_ptr,new_elem_num*size_of_element);
  void * new_ptr = malloc(new_elem_num*size_of_element); // allign data
  memcpy(new_ptr,old_ptr,old_elem_num*size_of_element);
  free(old_ptr);
  return new_ptr;
}

#define FLAG_SKIP_SHADOWVOLUME 1 // todo: share this flag

CIndexedMesh::CIndexedMesh(IRCLog	* pLog, 
                           const char*szFileName, const char*szGeomName, 
                           int*_count, bool bLoadAdditinalInfo, bool bKeepInLocalSpace, bool bIgnoreFakeMats ) 
{
  //	memset( this,0,sizeof(*this) );
	m_pTangBasis=0;
  m_pFaces=0;
  m_pVerts=0;
  m_pCoors=0;
  m_pNorms=0;
  m_pColor=0;
  m_nFaceCount=0;
  m_nVertCount=0;
  m_nCoorCount=0;
  m_nNormCount=0;
  m_vBoxMin(0,0,0);
  m_vBoxMax(0,0,0);

  m_pSystem = pLog;

  if(!szGeomName)
  {
    /*    if(pvAngles && *pvAngles!=Vec3d(0,0,0))
    GetLog()->UpdateLoadingScreen("Loading %s (Angles=%.2f,%.2f,%.2f)",szFileName,pvAngles->x,pvAngles->y,pvAngles->z);
    else*/
//    m_pSystem->Log(" Loading %s",szFileName);
    m_pSystem->Log("Loading static CGF");		// name is already printed
  }

  LoadCGF(szFileName, szGeomName, bLoadAdditinalInfo, bKeepInLocalSpace, bIgnoreFakeMats);

	// no shadow volumes if TemplPlants shader found
	bool bTemplPlantsFound = false;
	for(int m=0; m<m_lstMatTable.Count(); m++)
	{
		CMatInfo * pMatInfo = &m_lstMatTable[m];
		if(pMatInfo->pMatEnt)
		{
			char szMatName[128];
			strncpy(szMatName, pMatInfo->pMatEnt->name, sizeof(szMatName)); szMatName[sizeof(szMatName)-1]=0;
			strlwr(szMatName);
			if(strstr(szMatName,"templplants"))
			{
				bTemplPlantsFound = true;
				break;
			}
		}
	}

	// set cast shadow flag per face
  for(int i=0; i<m_nFaceCount; i++)
  {
    CMatInfo * pMatInfo = &m_lstMatTable[m_pFaces[i].shader_id];
    if( pMatInfo->m_Flags & MIF_NOCASTSHADOWS || bTemplPlantsFound)
      m_pFaces[i].m_dwFlags |= FLAG_SKIP_SHADOWVOLUME;
  }

  *_count = m_nFaceCount;

  if(m_nFaceCount == 0)
  { 
    if(!szGeomName)
      m_pSystem->Log("Error loading file");     // name is already printed

    return; 
  }

  /*  if(pvAngles && *pvAngles != Vec3d(0,0,0))
  { // rotate object
  //    Matrix m;
  //  m.Identity();
  //m.RotateMatrix(*pvAngles);


  // calculate obj rotation matrix
  GetRenderer()->PushMatrix();
  GetRenderer()->LoadMatrix(0);
  GetRenderer()->RotateMatrix(pvAngles->z,0,0,1);
  GetRenderer()->RotateMatrix(pvAngles->y,0,1,0);
  GetRenderer()->RotateMatrix(pvAngles->x,1,0,0);

  // get matrix
  float arrMat[16];
  GetRenderer()->GetModelViewMatrix(arrMat);            
  GetRenderer()->PopMatrix();

  Matrix m(arrMat);

  for(int i=0; i<m_nVertCount; i++)
  {
  Vec3d vert; vert.Set(&m_pVerts[i].x);
  Vec3d norm; norm.Set(&m_pNorms[i].x);

  vert = m.TransformPoint(vert);
  norm = m.TransformVector(norm);

  m_pVerts[i].x = vert.x;
  m_pVerts[i].y = vert.y;
  m_pVerts[i].z = vert.z;

  m_pNorms[i].x = norm.x;
  m_pNorms[i].y = norm.y;
  m_pNorms[i].z = norm.z;
  }

  int l;
  for(l=0; l<m_lstLSources.Count(); l++)
  {
  m_lstLSources[l]->m_Origin = m.TransformPoint(m_lstLSources[l]->m_Origin);
  }
  for(l=0; l<m_tgtLSources.Count(); l++)
  {
  m_tgtLSources[l]->m_Origin = m.TransformPoint(m_tgtLSources[l]->m_Origin);
  }

  for(int h=0; h<m_lstHelpers.Count(); h++)
  {
  m_lstHelpers[h].vPos = m.TransformPoint(m_lstHelpers[h].vPos);
  //      m_lstHelpers[h].qRot += *pvAngles;
  }
  }*/

  SetMinMax();

  for(int n=0; n<m_nVertCount; n++)
    m_pNorms[n].Normalize();

  { // convert from sm into meters
    for(int i=0; i<m_nVertCount; i++)
    {
      m_pVerts[i].x*=0.01f;
      m_pVerts[i].y*=0.01f;
      m_pVerts[i].z*=0.01f;
    }

    for(int l=0; l<m_lstLSources.Count(); l++)
    {
      m_lstLSources[l]->m_Origin *=0.01f;
      m_lstLSources[l]->m_vObjectSpacePos	*=0.01f;
      m_lstLSources[l]->m_fRadius*=0.01f;

/*      if (m_lstLSources[l]->m_OrigLight)
      {
        m_lstLSources[l]->m_OrigLight->m_Origin *=0.01f;
        m_lstLSources[l]->m_OrigLight->m_vObjectSpacePos *=0.01f;
        m_lstLSources[l]->m_OrigLight->m_fRadius*=0.01f;
      }*/
    }

    for(int l=0; l<m_tgtLSources.Count(); l++)
    {
      m_tgtLSources[l]->m_Origin *=0.01f;
      m_tgtLSources[l]->m_vObjectSpacePos *=0.01f;
      m_tgtLSources[l]->m_fRadius*=0.01f;
    }

    for(int h=0; h<m_lstHelpers.Count(); h++)
    {
      //T_CHANGED_BY_IVO
      //m_lstHelpers[h].tMat.m_values[3][0]*=0.01f;
      //m_lstHelpers[h].tMat.m_values[3][1]*=0.01f;
      //m_lstHelpers[h].tMat.m_values[3][2]*=0.01f;

      Vec3d temp = m_lstHelpers[h].tMat.GetTranslationOLD();
      temp = temp * 0.01f;
      m_lstHelpers[h].tMat.SetTranslationOLD(temp);

    }

    m_vBoxMin*=0.01f;
    m_vBoxMax*=0.01f;
  }
}

// find min max
void CIndexedMesh::SetMinMax()
{
  if(!m_nFaceCount || !m_nVertCount)
    return;

  m_vBoxMax = m_vBoxMin = Vec3d(m_pVerts[0].x,m_pVerts[0].y,m_pVerts[0].z);

  for(int i=0; i<m_nVertCount; i++)
  {
    m_vBoxMin.CheckMin(Vec3d(m_pVerts[i].x,m_pVerts[i].y,m_pVerts[i].z));
    m_vBoxMax.CheckMax(Vec3d(m_pVerts[i].x,m_pVerts[i].y,m_pVerts[i].z));
  }
}
/*
// center objects
void CIndexedMesh::Center( float new_size )
{
Vec3d center = (m_vBoxMax+m_vBoxMin)/2;

Vec3d maxVec = Vec3d(m_vBoxMax-m_vBoxMin);

float max_size = max(max(maxVec.x,maxVec.y),maxVec.z);
float K = (float)(fabs(new_size)/max_size);

for(int i=0; i<m_nVertCount; i++)
{
m_pVerts[i].x-=center.x;
m_pVerts[i].y-=center.y;
if(new_size<0)
m_pVerts[i].z-=center.z;

m_pVerts[i].x*=K;
m_pVerts[i].y*=K;
m_pVerts[i].z*=K;
}

for(int l=0; l<m_lstLSources.Count(); l++)
{
m_lstLSources[l].m_Origin.x-=center.x;
m_lstLSources[l].m_Origin.y-=center.y;
if(new_size<0)
m_lstLSources[l].m_Origin.z-=center.z;

m_lstLSources[l].m_Origin.x*=K;
m_lstLSources[l].m_Origin.y*=K;
m_lstLSources[l].m_Origin.z*=K;
m_lstLSources[l].m_fRadius*=K;      
}

for(int h=0; h<m_Helpers.Count(); h++)
{
m_Helpers[h].pos.x-=center.x;
m_Helpers[h].pos.y-=center.y;
if(new_size<0)
m_Helpers[h].pos.z-=center.z;

m_Helpers[h].pos.x*=K;
m_Helpers[h].pos.y*=K;
m_Helpers[h].pos.z*=K;
}
}*/

int CIndexedMesh::GetAllocatedBytes()
{
  return
    sizeof(*this) + 
    sizeof(CObjFace) * m_nFaceCount +
    sizeof(Vec3d) * m_nVertCount +
    sizeof(TexCoord) * m_nCoorCount +
    sizeof(Vec3d) * m_nNormCount +
    sizeof(CDLight)  * m_lstLSources.Count() +
    sizeof(CDLight)  * m_tgtLSources.Count();
}

void CIndexedMesh::FreeLMInfo()
{
/*  for( int c=0; c<m_nFaceCount; c++)
  {
    delete m_pFaces[c].m_lInfo;	
    m_pFaces[c].m_lInfo=0;
  }*/
}

size_t CIndexedMesh::GetMemoryUsage()
{
  size_t nSize = 0;

  for(int i=0; i<m_lstGeomNames.Count(); i++ )
    nSize += strlen(m_lstGeomNames[i])+1;
  nSize += m_lstHelpers.GetMemoryUsage();
  nSize += m_lstLSources.Count() * sizeof(CDLight) + m_lstLSources.GetMemoryUsage();
  nSize += m_lstMatTable.GetMemoryUsage();
  nSize += m_nCoorCount * sizeof(TexCoord);
  nSize += m_nFaceCount * sizeof(CObjFace);
  nSize += m_nNormCount * sizeof(Vec3d);
  nSize += m_nVertCount * sizeof(Vec3d); // vert
  nSize += m_nVertCount * sizeof(Vec3d); // color
  nSize += m_tgtLSources.Count() * sizeof(CDLight) + m_tgtLSources.GetMemoryUsage();

  return nSize;
}

#include "..\TangentSpaceCalculation.h"

// a proxy structure that gets passed to the tangent space calculation algorithm
struct IndexedMeshProxy
{
public:
	IndexedMeshProxy(CIndexedMesh * pIndexedMesh)
	{
		m_pIndexedMesh = pIndexedMesh;
	}

	void init ()
	{
	}

	DWORD GetTriangleCount( void ) const
	{
		return m_pIndexedMesh->m_nFaceCount;
	}

	void GetTriangleIndices( const DWORD indwTriNo, DWORD outdwPos[3], DWORD outdwNorm[3], DWORD outdwUV[3] ) const
	{
		const CObjFace & rFace = m_pIndexedMesh->m_pFaces[indwTriNo];
		outdwNorm[0] = outdwPos[0] = rFace.v[0];
		outdwNorm[1] = outdwPos[1] = rFace.v[1];
		outdwNorm[2] = outdwPos[2] = rFace.v[2];

		if(m_pIndexedMesh->m_pCoors)
		{
			outdwUV[0] = rFace.t[0];
			outdwUV[1] = rFace.t[1];
			outdwUV[2] = rFace.t[2];
		}
		else
		{
			outdwUV[0] = outdwUV[1] = outdwUV[2] = 0;
		}
	}

	void GetPos( const DWORD indwPos, float outfPos[3] ) const
	{
		const Vec3d& ptPos = m_pIndexedMesh->m_pVerts[indwPos];
		outfPos[0] = ptPos.x;
		outfPos[1] = ptPos.y;
		outfPos[2] = ptPos.z;
	}

	void GetUV ( const DWORD indwPos, float outfUV[2] ) const
	{
		if(m_pIndexedMesh->m_pCoors)
		{
			const TexCoord& uv = m_pIndexedMesh->m_pCoors[indwPos];
			outfUV[0] = uv.s;
			outfUV[1] = uv.t;
		}
		else
		{
			outfUV[0] = outfUV[1] = 0;
		}
	}

protected:
	CIndexedMesh * m_pIndexedMesh;
};

bool CIndexedMesh::CalcTangentSpace()
{
	CTangentSpaceCalculation<IndexedMeshProxy> TangBaseBuilder;

	IndexedMeshProxy Proxy(this);
	TangBaseBuilder.CalculateTangentSpace (Proxy);

	int nBaseCount = (int)TangBaseBuilder.GetBaseCount();

	for(int i=0; i<m_nFaceCount; i++)
	{
		CObjFace & rFace = m_pFaces[i];
		DWORD outdwBase[3]={0,0,0};
		TangBaseBuilder.GetTriangleBaseIndices( i, outdwBase );
		assert((int)outdwBase[0]<nBaseCount);
		assert((int)outdwBase[1]<nBaseCount);
		assert((int)outdwBase[2]<nBaseCount);
		rFace.b[0] = ushort(outdwBase[0]);
		rFace.b[1] = ushort(outdwBase[1]);
		rFace.b[2] = ushort(outdwBase[2]);
	}

	m_pTangBasis = new CBasis[nBaseCount];
	for(int i=0; i<nBaseCount; i++)
	{
		float outU[3], outV[3], outN[3];

		TangBaseBuilder.GetBase( i, outU, outV, outN );

		assert (outU[0]>=-1 && outU[0]<=1);
		assert (outU[1]>=-1 && outU[1]<=1);
		assert (outU[2]>=-1 && outU[2]<=1);
		assert (outV[0]>=-1 && outV[0]<=1);
		assert (outV[1]>=-1 && outV[1]<=1);
		assert (outV[2]>=-1 && outV[2]<=1);
		assert (outN[0]>=-1 && outN[0]<=1);
		assert (outN[1]>=-1 && outN[1]<=1);
		assert (outN[2]>=-1 && outN[2]<=1);

		m_pTangBasis[i].tangent  = Vec3d(outU);
		m_pTangBasis[i].binormal = Vec3d(outV);
		m_pTangBasis[i].tnormal  = Vec3d(outN);
	}

	return true;
}
