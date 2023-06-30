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

#include "RenderPCH.h"
#include "cryheaders.h"
#include "nvtristrip/nvtristrip.h"
#include "serializebuffer.h"
#include "MakMatInfoFromMAT_ENTITY.h"

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

int CLeafBuffer__SetTexType(TextureMap3 *tm)
{
  if (tm->type == TEXMAP_CUBIC)
    return eTT_Cubemap;
  else
  if (tm->type == TEXMAP_AUTOCUBIC)
    return eTT_AutoCubemap;
  return eTT_Base;
}

bool CLeafBuffer::Serialize(int & nPos, uchar * pSerBuf, bool bSave, char * _szFolderName, char * _szFileName, double & dCIndexedMesh__LoadMaterial)
{ // Load
  int i;
  char szSignature[16]="";
  if(!LoadBuffer(szSignature, 11, pSerBuf, nPos) || strcmp(szSignature,"LeafBuffer"))
    return false;

  LoadBuffer(&m_SecVertCount, sizeof(m_SecVertCount), pSerBuf, nPos);

  if((*((int*)&pSerBuf[nPos])))
    m_arrVertStripMap = new uint[(*((int*)&pSerBuf[nPos]))];
  LoadBuffer( m_arrVertStripMap, m_SecVertCount*sizeof(m_arrVertStripMap[0]), pSerBuf, nPos);

  m_SecIndices.LoadFromBuffer(pSerBuf, nPos);
	m_NumIndices = m_SecIndices.Num();

  if((*((int*)&pSerBuf[nPos])) && !m_pIndicesPreStrip)
    m_pIndicesPreStrip = new list2<unsigned short>;

  m_pIndicesPreStrip->LoadFromBuffer(pSerBuf, nPos);
  LoadBuffer(&m_nPrimetiveType,sizeof(m_nPrimetiveType), pSerBuf, nPos);

  if((*((int*)&pSerBuf[nPos])))
  {
    m_pLoadedColors = new Vec3d[(*((int*)&pSerBuf[nPos]))/sizeof(m_pLoadedColors[0])];
    assert((*((int*)&pSerBuf[nPos]))/sizeof(m_pLoadedColors[0]) == m_SecVertCount);
  }
  LoadBuffer( m_pLoadedColors, m_SecVertCount*sizeof(m_pLoadedColors[0]), pSerBuf, nPos);

  // load mat info
  m_pMats = new list2<CMatInfo>;
  m_bMaterialsWasCreatedInRenderer = true;
  m_pMats->LoadFromBuffer(pSerBuf, nPos);

  // create materaial
  for (i=0; i<m_pMats->Count(); i++)
  {
    m_pMats->GetAt(i).pRE = 0;

    MAT_ENTITY * pMatEnt = new MAT_ENTITY;
    LoadBuffer(pMatEnt, sizeof(*pMatEnt), pSerBuf, nPos);
    m_pMats->Get(i)->pMatEnt = pMatEnt;

    // skip loading of fake mats
    if(m_pMats->Get(i)->nNumIndices && m_pMats->Get(i)->nNumVerts)
      if(_szFileName && _szFolderName)
			{
				AUTO_PROFILE_SECTION(iSystem->GetITimer(), dCIndexedMesh__LoadMaterial);
        CIndexedMesh__LoadMaterial(_szFileName, _szFolderName, m_pMats->GetAt(i), gRenDev, pMatEnt);
			}

    delete m_pMats->Get(i)->pMatEnt;
    m_pMats->Get(i)->pMatEnt=0;

    if(m_pMats->GetAt(i).m_dwNumSections)
    {
      m_pMats->GetAt(i).m_pPrimitiveGroups = new SPrimitiveGroup[m_pMats->GetAt(i).m_dwNumSections];
      LoadBuffer((void*)m_pMats->GetAt(i).m_pPrimitiveGroups, 
      sizeof(SPrimitiveGroup)*m_pMats->GetAt(i).m_dwNumSections, pSerBuf, nPos);
    }
  }

  // make system vert buffer
  m_pSecVertBuffer = new CVertexBuffer();
  LoadBuffer( m_pSecVertBuffer, sizeof(CVertexBuffer), pSerBuf, nPos); // need to restore

  // load positions
  if(*((int*)&pSerBuf[nPos]))
  {
    assert(m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData); // should not be zero before saving
    m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData = new uchar[m_SecVertCount*m_VertexSize[m_pSecVertBuffer->m_vertexformat]];
  }
  else
    assert(!m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData);
  assert(m_SecVertCount*m_VertexSize[m_pSecVertBuffer->m_vertexformat] == (*((int*)&pSerBuf[nPos])));
  LoadBuffer( m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData, m_SecVertCount*m_VertexSize[m_pSecVertBuffer->m_vertexformat], pSerBuf, nPos);

  // load tangents
  if(*((int*)&pSerBuf[nPos]))
  {
    assert(m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData); // should not be zero before saving
    m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData = new SPipTangents[m_SecVertCount];
  }
  else
    assert(!m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData);
  assert(m_SecVertCount*sizeof(SPipTangents) == (*((int*)&pSerBuf[nPos])));
  LoadBuffer( m_pSecVertBuffer->m_VS[VSF_TANGENTS].m_VData, m_SecVertCount*sizeof(SPipTangents), pSerBuf, nPos);

  assert(!m_pVertexBuffer); // not needed
  LoadBuffer(&m_vBoxMax,       sizeof(m_vBoxMax),           pSerBuf, nPos);
  LoadBuffer(&m_vBoxMin,       sizeof(m_vBoxMin),           pSerBuf, nPos);

	int bHasVtxMap = 0;
	LoadBuffer(&bHasVtxMap, sizeof(bHasVtxMap), pSerBuf,nPos);
	if (bHasVtxMap)
	{
		m_arrVtxMap = new uint[m_SecVertCount];
		LoadBuffer(m_arrVtxMap, sizeof(uint)*m_SecVertCount, pSerBuf,nPos);
	}

  // make REs
  for (i=0; i<(*m_pMats).Count(); i++)
  {		
    if(m_pMats->Get(i)->nNumIndices && m_pMats->Get(i)->nNumVerts)
    {			
      CREOcLeaf *re = (CREOcLeaf *)gRenDev->EF_CreateRE(eDATA_OcLeaf);
      re->m_pChunk = &(*m_pMats)[i];
      re->m_pBuffer = this;
      assert (re->m_pChunk->nNumIndices < 60000);
      re->m_pChunk->pRE = re;			

			// set sort offset
//			IShader * ef = m_pMats->Get(i)->shaderItem.m_pShader->GetTemplate(-1);
	//		bool bTwoSided =  ef && (ef->GetCull() == eCULL_None);
			bool bTwoSided =  (m_pMats->Get(i)->shaderItem.m_pShaderResources->m_ResFlags & MTLFLAG_2SIDED)!=0;//ef && (ef->GetCull() == e CULL_None);
			re->m_SortId = i + 2*(!bTwoSided); // render double sided leafs last
		}
    else
      (*m_pMats)[i].pRE = 0;
  }

  return 0;
}

bool CLeafBuffer::LoadMaterial(int m, 
                                const char *szFileName, const char *szFolderName, 
                                list2<CMatInfo> & lstMatTable, IRenderer * pRenderer,
                                MAT_ENTITY * me, bool bFake)
{
  if(m<0 || bFake)//!pCGF->m_lstMaterials[m].IsStdMat)
  {
    CMatInfo fake;
    lstMatTable.Add(fake);
    return false;
  }

//  assert(strlen(szFolderName)+strlen(pCGF->m_lstMaterials[m].map_d.name)<1024);
//    m_pSystem->GetIConsole()->Exit("LoadCGF: texture path len error");

  SInputShaderResources Res;
  memset(&Res, 0, sizeof(Res));
  SLightMaterial LMs;
//  MAT_ENTITY *me = &pCGF->m_lstMaterials[m];
  if (me->m_New && (me->col_d.r>5 || me->col_d.g>5 || me->col_d.b>5 || me->col_s.r>5 || me->col_s.g>5 || me->col_s.b>5))
  {
    Res.m_LMaterial = &LMs;
    Res.m_LMaterial->Front.m_Ambient  = CFColor(me->col_a.r/255.f,me->col_a.g/255.f,me->col_a.b/255.f);
    Res.m_LMaterial->Front.m_Diffuse  = CFColor(me->col_d.r/255.f,me->col_d.g/255.f,me->col_d.b/255.f);
    Res.m_LMaterial->Front.m_Specular = CFColor(me->col_s.r/255.f,me->col_s.g/255.f,me->col_s.b/255.f);
		Res.m_LMaterial->Front.m_Specular *= me->specLevel;
		Res.m_LMaterial->Front.m_Specular.Clamp();
		Res.m_LMaterial->Front.m_Emission = Res.m_LMaterial->Front.m_Diffuse * me->selfIllum; 
    Res.m_LMaterial->Front.m_SpecShininess = me->specShininess;
  }

  char diffuse[256]="";
  strcpy(diffuse, me->map_d.name);

  char bump[256]="";
  strcpy(bump, me->map_b.name);

  char normalmap[256]="";
  if(me->map_displ.name[0] && (me->flags & MTLFLAG_CRYSHADER))
    strcpy(normalmap, me->map_displ.name);

  char opacity[256]="";
  char decal[256]="";
  if(me->map_o.name[0])
  {
    if (me->flags & MTLFLAG_CRYSHADER)
      strcpy(decal, me->map_o.name);
    else
      strcpy(opacity, me->map_o.name);
  }

  char gloss[256]="";
  if(me->map_g.name[0])
    strcpy(gloss, me->map_g.name);

  char cubemap[256]="";

  char env[256]="";
  if(me->map_e.name[0])
    strcpy(env, me->map_e.name);

  char spec[256]="";
  if(me->map_s.name[0])
    strcpy(spec, me->map_s.name);

  char det[256]="";
  if(me->map_detail.name[0])
    strcpy(det, me->map_detail.name);

  char subsurf[256]="";
  if(me->map_subsurf.name[0])
    strcpy(subsurf, me->map_subsurf.name);

  char refl[256]="";
  if(me->map_e.name[0])
    strcpy(refl, me->map_e.name);

  char * mat_name = me->name;
  
  // fill MatInfo struct
  CMatInfo & newMat = lstMatTable[m];
//  strcpy(newMat.szDiffuse, diffuse);

  if (me->Dyn_Bounce == 1.0f)
    newMat.m_Flags |= MIF_PHYSIC;
  
  if (me->Dyn_StaticFriction == 1.0f)
    newMat.m_Flags |= MIF_NOCASTSHADOWS;

/*  if (nLM > 0)
  {
    Res.m_Textures[EFTT_LIGHTMAP].m_TU.m_ITexPic = m_pSystem->GetIRenderer()->EF_GetTextureByID(nLM);
    Res.m_Textures[EFTT_LIGHTMAP].m_Name = Res.m_Textures[EFTT_LIGHTMAP].m_TU.m_ITexPic->GetName();
  }
  if (nLM_LD > 0)
  {
    Res.m_Textures[EFTT_LIGHTMAP_DIR].m_TU.m_ITexPic = m_pSystem->GetIRenderer()->EF_GetTextureByID(nLM_LD);
    Res.m_Textures[EFTT_LIGHTMAP_DIR].m_Name = Res.m_Textures[EFTT_LIGHTMAP].m_TU.m_ITexPic->GetName();
  }*/

  Res.m_TexturePath = szFolderName;
  Res.m_Textures[EFTT_DIFFUSE].m_Name = diffuse;
  Res.m_Textures[EFTT_GLOSS].m_Name = gloss;
  Res.m_Textures[EFTT_SUBSURFACE].m_Name = subsurf;
  Res.m_Textures[EFTT_BUMP].m_Name = bump;
  Res.m_Textures[EFTT_NORMALMAP].m_Name = normalmap;
  Res.m_Textures[EFTT_CUBEMAP].m_Name = cubemap[0] ? cubemap : env;
  Res.m_Textures[EFTT_SPECULAR].m_Name = spec;
  Res.m_Textures[EFTT_DETAIL_OVERLAY].m_Name = det;
  Res.m_Textures[EFTT_OPACITY].m_Name = opacity;
  Res.m_Textures[EFTT_DECAL_OVERLAY].m_Name = decal;
  Res.m_Textures[EFTT_SUBSURFACE].m_Name = subsurf;
  Res.m_Textures[EFTT_REFLECTION].m_Name = refl;
  Res.m_Textures[EFTT_REFLECTION].m_TU.m_eTexType = (ETexType)SetTexType(&me->map_e);
  Res.m_Textures[EFTT_CUBEMAP].m_TU.m_eTexType = (ETexType)SetTexType(&me->map_e);
  Res.m_Textures[EFTT_SUBSURFACE].m_TU.m_eTexType = (ETexType)SetTexType(&me->map_subsurf);
	Res.m_Textures[EFTT_BUMP].m_TU.m_eTexType = eTT_Bumpmap;
  Res.m_ResFlags = me->flags;
  Res.m_Opacity = me->opacity;
  Res.m_AlphaRef = me->Dyn_SlidingFriction;
  if (me->flags & MTLFLAG_CRYSHADER)
    Res.m_AlphaRef = me->alpharef;

  if (decal[0])
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_Amount = me->map_o.Amount;
  Res.m_Textures[EFTT_DIFFUSE].m_Amount = me->map_d.Amount;
  Res.m_Textures[EFTT_BUMP].m_Amount = me->map_b.Amount;
  if (me->flags & MTLFLAG_CRYSHADER)
    Res.m_Textures[EFTT_NORMALMAP].m_Amount = me->map_displ.Amount;
  Res.m_Textures[EFTT_OPACITY].m_Amount = me->map_o.Amount;
  Res.m_Textures[EFTT_REFLECTION].m_Amount = me->map_e.Amount;
  Res.m_Textures[EFTT_SUBSURFACE].m_Amount = me->map_subsurf.Amount;

  Res.m_Textures[EFTT_DIFFUSE].m_TexFlags = me->map_d.flags;
  //Res.m_Textures[EFTT_SUBSURFACE].m_TexFlags = me->map_d.flags;
  Res.m_Textures[EFTT_GLOSS].m_TexFlags = me->map_g.flags;
  Res.m_Textures[EFTT_BUMP].m_TexFlags = me->map_b.flags;
  Res.m_Textures[EFTT_CUBEMAP].m_TexFlags = me->map_e.flags;
  Res.m_Textures[EFTT_SPECULAR].m_TexFlags = me->map_s.flags;
  Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexFlags = me->map_detail.flags;
  Res.m_Textures[EFTT_SUBSURFACE].m_TexFlags = me->map_subsurf.flags;

  if (!Res.m_Textures[EFTT_DETAIL_OVERLAY].m_Name.empty())
  {
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexModificator.m_Tiling[0] = me->map_detail.uscl_val;
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexModificator.m_Tiling[1] = me->map_detail.vscl_val;
  }
  if (!Res.m_Textures[EFTT_OPACITY].m_Name.empty())
  {
    Res.m_Textures[EFTT_OPACITY].m_TexModificator.m_Tiling[0] = me->map_o.uscl_val;
    Res.m_Textures[EFTT_OPACITY].m_TexModificator.m_Tiling[1] = me->map_o.vscl_val;
    Res.m_Textures[EFTT_OPACITY].m_TexModificator.m_Rot[0] = Degr2Word(me->map_o.urot_val);
    Res.m_Textures[EFTT_OPACITY].m_TexModificator.m_Rot[1] = Degr2Word(me->map_o.vrot_val);
    Res.m_Textures[EFTT_OPACITY].m_TexModificator.m_Offs[0] = me->map_o.uoff_val;
    Res.m_Textures[EFTT_OPACITY].m_TexModificator.m_Offs[1] = me->map_o.voff_val;
    Res.m_Textures[EFTT_OPACITY].m_bUTile = me->map_o.utile;
    Res.m_Textures[EFTT_OPACITY].m_bVTile = me->map_o.vtile;
  }
  if (!Res.m_Textures[EFTT_DECAL_OVERLAY].m_Name.empty())
  {
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_TexModificator.m_Tiling[0] = me->map_o.uscl_val;
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_TexModificator.m_Tiling[1] = me->map_o.vscl_val;
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_TexModificator.m_Rot[0] = Degr2Word(me->map_o.urot_val);
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_TexModificator.m_Rot[1] = Degr2Word(me->map_o.vrot_val);
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_TexModificator.m_Offs[0] = me->map_o.uoff_val;
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_TexModificator.m_Offs[1] = me->map_o.voff_val;
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_bUTile = me->map_o.utile;
    Res.m_Textures[EFTT_DECAL_OVERLAY].m_bVTile = me->map_o.vtile;
  }

  char mName[128];
  strcpy(mName, mat_name);
  char *str = strchr(mat_name, '/');
  if (str)
  {
    mName[str-mat_name] = 0;
    strncpy(newMat.sScriptMaterial, &str[1], sizeof(newMat.sScriptMaterial));
    newMat.sScriptMaterial[sizeof(newMat.sScriptMaterial)-1]=0;
  }
  else
  {
    newMat.sScriptMaterial[0] = 0;
  }

  char *templName = NULL;;
  if(strnicmp(mName, "$s_",3)==0)
  {
    templName = &mName[3];
  }
  else
  if(str=strchr(mName, '('))
  {
    mName[str-mName] = 0;
    templName = &mName[str-mName+1];
    if(str=strchr(templName, ')'))
      templName[str-templName] = 0;
  }

  uchar nInvert = 0;
  if (templName && templName[0] == '#')
  {
    templName++;
    nInvert = 1;
  }

//  strcpy(newMat.szMatName, mName);
  
    // load shader
	  if(mName[0]==0)
		  strcpy(mName,"nodraw");


    newMat.shaderItem = pRenderer->EF_LoadShaderItem(mName, eSH_World, true, templName, 0, &Res);

    //newMat.shaderItem->m_pShader->AdjustResources(newMat.shaderItem->m_pShaderResources);

    //IShader * ef = newMat.shaderItem.m_pShader->GetTemplate(-1);
    //if(ef->GetPhysMaterialFlags() & MATF_NOCLIP)
    //  newMat.m_Flags &= ~MIF_PHYSIC;
  //	strcpy(newMat.szFolderName, szFolderName);
      // remember polybump state
    newMat.m_Flags &= ~MIF_POLYBUMP;

  //  pFace->shader_id = m_lstMatTable.Count();

    newMat.fAlpha = me->opacity;

  return true;
}

int CLeafBuffer::SetTexType(TextureMap3 *tm)
{
  if (tm->type == TEXMAP_CUBIC)
    return eTT_Cubemap;
  else
  if (tm->type == TEXMAP_AUTOCUBIC)
    return eTT_AutoCubemap;
  return eTT_Base;
}
