////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   crystaticmodel.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: load cgf file
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "CryStaticModel.h"

#include "baseobj.h"
#include "node.h"
#include "geom.h"
#include "helper.h"
#include "light.h"

#include "file.h"

CryStaticModel::CryStaticModel()
{
	memset( this,0,sizeof(*this) );
}

CryStaticModel::~CryStaticModel()
{
  for( int i=0; i<m_nNewObjs; i++)
  {
    CNodeCGF * pNode = (CNodeCGF*)m_ppNewObjs[i];
    delete pNode;
  }

  if(m_ppNewObjs)
    free(m_ppNewObjs);
  m_ppNewObjs=0;
}

void CryStaticModel::LoadMaterials(CXFile*f, int pos)
{
  if(f->FSeek(pos,SEEK_SET))
    return;

  CHUNK_HEADER ch;
  int res = f->FRead(&ch,1,sizeof(ch));
	if (ch.ChunkVersion == MTL_CHUNK_DESC_0746::VERSION)
  {
    f->FSeek(pos,SEEK_SET);
    MTL_CHUNK_DESC_0746 chunk;
    int res=f->FRead(&chunk,1,sizeof(chunk));
    if(res!=sizeof(chunk))
      return;
    
    MAT_ENTITY me;
    memset(&me, 0, sizeof(MAT_ENTITY));
    me.opacity = 1.0f;
    me.alpharef = 0;
    me.m_New = 2;
    strcpy(me.name, chunk.name);
    switch (chunk.MtlType)
    {
      case MTL_STANDARD:
        me.IsStdMat = true;
        me.col_d = chunk.col_d;
        me.col_a = chunk.col_a;
        me.col_s = chunk.col_s;
        
        me.specLevel = chunk.specLevel;
        me.specShininess = chunk.specShininess*100;
        me.opacity = chunk.opacity;
        me.selfIllum = chunk.selfIllum;
        me.flags = chunk.flags;
        if (me.flags & MTLFLAG_CRYSHADER)
          me.alpharef = chunk.alphaTest;
        
        me.Dyn_Bounce = chunk.Dyn_Bounce;
        me.Dyn_StaticFriction = chunk.Dyn_StaticFriction;
        me.Dyn_SlidingFriction = chunk.Dyn_SlidingFriction;
        /* //Timur[10/24/2001] 
        strcpy(me.map_a, chunk.tex_a.name);
        strcpy(me.map_d, chunk.tex_d.name);
        strcpy(me.map_o, chunk.tex_o.name);
        strcpy(me.map_b, chunk.tex_b.name);
        strcpy(me.map_s, chunk.tex_s.name);
        strcpy(me.map_g, chunk.tex_g.name);
        strcpy(me.map_c, chunk.tex_c.name);
        strcpy(me.map_e, chunk.tex_rl.name);
        strcpy(me.map_rr, chunk.tex_rr.name);
        strcpy(me.map_det, chunk.tex_det.name);
        */
        me.map_a = chunk.tex_a;
        me.map_d = chunk.tex_d;
        me.map_o = chunk.tex_o;
        me.map_b = chunk.tex_b;
        me.map_s = chunk.tex_s;
        me.map_g = chunk.tex_g;
        me.map_detail = chunk.tex_fl;
        me.map_e = chunk.tex_rl;
        me.map_subsurf = chunk.tex_subsurf;
        me.map_displ = chunk.tex_det;
        
        me.nChildren = chunk.nChildren;
        
        m_lstMaterials.Add(me);
        break;
        
        /*      case MTL_MULTI:
        me.IsStdMat = 0;
        me.nChildren = chunk.nChildren;
        me.children = new int [chunk.nChildren];
        int res=f->FRead(me.children,sizeof(int),chunk.nChildren);
        if (res != chunk.nChildren)
        return;*/
    }
  }
  else
  if (ch.ChunkVersion == MTL_CHUNK_DESC_0745::VERSION)
  {
    f->FSeek(pos,SEEK_SET);
    MTL_CHUNK_DESC_0745 chunk;
    int res=f->FRead(&chunk,1,sizeof(chunk));
    if(res!=sizeof(chunk))
      return;
    
    MAT_ENTITY me;
    memset(&me, 0, sizeof(MAT_ENTITY));
    me.opacity = 1.0f;
    me.alpharef = 0;
    me.m_New = 1;
    strcpy(me.name, chunk.name);
    switch (chunk.MtlType)
    {
      case MTL_STANDARD:
        me.IsStdMat = true;
        me.col_d = chunk.col_d;
        me.col_a = chunk.col_a;
        me.col_s = chunk.col_s;
        
        me.specLevel = chunk.specLevel;
        me.specShininess = chunk.specShininess*100;
        me.opacity = chunk.opacity;
        me.selfIllum = chunk.selfIllum;
        me.flags = chunk.flags;
        
        me.Dyn_Bounce = chunk.Dyn_Bounce;
        me.Dyn_StaticFriction = chunk.Dyn_StaticFriction;
        me.Dyn_SlidingFriction = chunk.Dyn_SlidingFriction;
				/* //Timur[10/24/2001] 
        strcpy(me.map_a, chunk.tex_a.name);
        strcpy(me.map_d, chunk.tex_d.name);
        strcpy(me.map_o, chunk.tex_o.name);
        strcpy(me.map_b, chunk.tex_b.name);
        strcpy(me.map_s, chunk.tex_s.name);
        strcpy(me.map_g, chunk.tex_g.name);
        strcpy(me.map_c, chunk.tex_c.name);
        strcpy(me.map_e, chunk.tex_rl.name);
        strcpy(me.map_rr, chunk.tex_rr.name);
        strcpy(me.map_det, chunk.tex_det.name);
				*/
				me.map_a = chunk.tex_a;
        me.map_d = chunk.tex_d;
        me.map_o = chunk.tex_o;
        me.map_b = chunk.tex_b;
        me.map_s = chunk.tex_s;
        me.map_g = chunk.tex_g;
        me.map_detail = chunk.tex_c;
        me.map_e = chunk.tex_rl;
        me.map_subsurf = chunk.tex_subsurf;
        me.map_displ = chunk.tex_det;

        me.nChildren = chunk.nChildren;
        
        m_lstMaterials.Add(me);
        break;
        
/*      case MTL_MULTI:
        me.IsStdMat = 0;
        me.nChildren = chunk.nChildren;
        me.children = new int [chunk.nChildren];
        int res=f->FRead(me.children,sizeof(int),chunk.nChildren);
        if (res != chunk.nChildren)
          return;*/
    }
  }
  else
  if (ch.ChunkVersion == MTL_CHUNK_DESC_0744::VERSION)
  {
    f->FSeek(pos,SEEK_SET);
    MTL_CHUNK_DESC_0744 chunk;
    int res=f->FRead(&chunk,1,sizeof(chunk));
    if(res!=sizeof(chunk))
      return;

    MAT_ENTITY me;
    memset(&me, 0, sizeof(MAT_ENTITY));
    me.opacity = 1.0f;
    me.alpharef = 0;
    strcpy(me.name, chunk.name);
    switch (chunk.MtlType)
    {
      case MTL_STANDARD:
        me.IsStdMat = true;
        me.col_d = chunk.col_d;
        me.col_a = chunk.col_a;
        me.col_s = chunk.col_s;
        me.Dyn_Bounce = chunk.Dyn_Bounce;
        me.Dyn_StaticFriction = chunk.Dyn_StaticFriction;
        me.Dyn_SlidingFriction = chunk.Dyn_SlidingFriction;
        strcpy(me.map_d.name, chunk.tex_d.name);
        strcpy(me.map_o.name, chunk.tex_o.name);
        strcpy(me.map_b.name, chunk.tex_b.name);
        me.nChildren = chunk.nChildren;

        m_lstMaterials.Add(me);
        break;

      case MTL_MULTI:
        me.IsStdMat = 0;
        me.nChildren = chunk.nChildren;
        me.m_pMaterialChildren = new int [chunk.nChildren];//leak
        int res=f->FRead(me.m_pMaterialChildren,sizeof(int),chunk.nChildren);
        if (res != chunk.nChildren)
          return;
    }
  }
}

bool CryStaticModel::OnLoadgeom(char * FileName, const char * szGeomName, bool bLoadMats, bool bKeepInLocalSpace) 
{
  CXFile * f = new CXFile(GetSystem()->GetIPak());

  // preload all data
  if(!f->FLoad(FileName))
  { 
    delete f; return 0; 
  }

	//read the file header
	FILE_HEADER fh;
	int res = f->FRead(&fh,sizeof(fh),1);
	if(res!=1) 
    return 0;

	if(fh.Version != GeomFileVersion) 
	{
		f->FClose(); delete f; f=0;
		Warning(0,FileName,"CGF file version error: %s", FileName);
		return 0;
	}

	if(fh.FileType != FileType_Geom) 
	{
		f->FClose(); delete f; f=0;
    Warning(0,FileName,"CGF file version error: %s", FileName);
		return 0;
	}

	//read the chunk table
	f->FSeek(fh.ChunkTableOffset,SEEK_SET);
	int n_chunks=100000;
	res = f->FRead(&n_chunks,sizeof(n_chunks),1);
	if(res!=1) 
    return 0;

  if(n_chunks>=100000)
	{
		f->FClose(); delete f; f=0;
		Warning(0,FileName,"CGF File corrupted: %s, (n_chunks>=100000)", FileName);
		return 0;
	}

	CHUNK_HEADER * pChunks;
	pChunks=(CHUNK_HEADER *)malloc(sizeof(CHUNK_HEADER)*n_chunks);
	assert(pChunks);
	res = f->FRead(pChunks,sizeof(CHUNK_HEADER),n_chunks);
	if(res!=n_chunks) 
    return 0;

/////////////////////////////////////////////////////////////////////////////
// Create and load objects
/////////////////////////////////////////////////////////////////////////////

	m_ppNewObjs = (CBaseObj **)malloc(n_chunks*sizeof(CBaseObj*));
  memset(m_ppNewObjs,0,n_chunks*sizeof(CBaseObj*));

	assert(m_ppNewObjs);
	m_nNewObjs=0;

  int nGeomToLoadID = -1;

  int i;
	for(i=0;i<n_chunks;i++)
	{
		switch(pChunks[i].ChunkType)
		{
		case ChunkType_Node:
			m_ppNewObjs[m_nNewObjs]=new CNodeCGF();
			break;

		case ChunkType_Mesh:
      if(!szGeomName || nGeomToLoadID == i)
			  m_ppNewObjs[m_nNewObjs]=new CGeom();
			break;

		case ChunkType_Helper:
			m_ppNewObjs[m_nNewObjs]=new CHelper();
			break;

		case ChunkType_Light:
			m_ppNewObjs[m_nNewObjs]=new CLight();
			break;
    
    case ChunkType_Mtl:
      if(bLoadMats)
        LoadMaterials(f,pChunks[i].FileOffset);
      break;
		}

		if(m_ppNewObjs[m_nNewObjs])
		{
      m_ppNewObjs[m_nNewObjs]->Load(f,pChunks[i].FileOffset);

      // find chunk id of needed geom
      if(pChunks[i].ChunkType == ChunkType_Node)
      if(szGeomName && strcmp(szGeomName,((CNodeCGF*)m_ppNewObjs[m_nNewObjs])->m_Chunk.name)==0)
        nGeomToLoadID = ((CNodeCGF*)m_ppNewObjs[m_nNewObjs])->m_Chunk.ObjectID;
  
			m_nNewObjs++;
		}
	}

	//Do pointer and name list bindings
	for(i=0;i<m_nNewObjs;i++)
	{
		if(!m_ppNewObjs[i]) 
      continue;
		m_ppNewObjs[i]->Bind(m_ppNewObjs, m_nNewObjs);
	}

  f->FClose(); delete f; f=0;

	if(pChunks) 
    free(pChunks);
  pChunks=0;

/////////////////////////////////////////////////////////////////////////////
// Make objects
/////////////////////////////////////////////////////////////////////////////

	list2<NAME_ENTITY> lstOtherNames;

  for( i=0; i<m_nNewObjs; i++)
  {
		CNodeCGF * pNode = (CNodeCGF*)m_ppNewObjs[i];

    if(pNode->m_ChunkHeader.ChunkType != ChunkType_Node)
      continue;

    // make list of mesh names
    NAME_ENTITY geomname;
    strcpy(geomname.name, pNode->m_Chunk.name);
    lstOtherNames.Add(geomname);
		
    if(!pNode->m_pObj)
      continue;

    if(pNode->m_pObj->m_nUsers>1)
		{
			Warning( 0,FileName,"Loading of instances from cgf not supported, geom skipped: %s, %s",FileName, pNode->GetName() );
		}

		// Accumulate this and all parent nodes transformations
		// TODO: get rid of the obsolete CryMatrix here

		//CHANGED_BY_IVO
		//CryMatrix matNodeMatrix = pNode->m_Chunk.tm;
		Matrix44 matNodeMatrix = pNode->m_Chunk.tm;

		for(CNodeCGF * pCurNode = pNode->m_pParent; pCurNode; pCurNode = pCurNode->m_pParent)
			//CHANGED_BY_IVO
			//matNodeMatrix = matNodeMatrix * (CryMatrix&)(pCurNode->m_Chunk.tm);
		  matNodeMatrix = matNodeMatrix * (pCurNode->m_Chunk.tm);

		if(pNode->m_pObj->m_ChunkHeader.ChunkType == ChunkType_Mesh) 
    if(pNode->m_pObj->m_nUsers<=1)
    { // geoms
			// make list of mesh names
      NAME_ENTITY geomname;
      strcpy(geomname.name, pNode->m_Chunk.name);
      m_lstGeomNames.Add(geomname);
      lstOtherNames.DeleteLast();

      CGeom * pGeom = (CGeom*)pNode->m_pObj;

      // transform geometry from this node space into CGFs space
			if(!bKeepInLocalSpace)
      for(int v=0; v<pGeom->m_Chunk.nVerts; v++)
      {
        //CHANGED_BY_IVO
				//pGeom->m_pVertices[v].p = matNodeMatrix*pGeom->m_pVertices[v].p;          
        //pGeom->m_pVertices[v].n = matNodeMatrix/pGeom->m_pVertices[v].n;          
				pGeom->m_pVertices[v].p = matNodeMatrix.TransformPointOLD(pGeom->m_pVertices[v].p);          
				pGeom->m_pVertices[v].n = matNodeMatrix.TransformVectorOLD(pGeom->m_pVertices[v].n);          
      }

      if(!szGeomName || strcmp(pNode->GetName(),szGeomName)==0)
        m_lstGeoms.Add(pGeom);
    }

		if(pNode->m_pObj->m_ChunkHeader.ChunkType == ChunkType_Light) 
    { // make light
      CLight * pLight = (CLight*)pNode->m_pObj;

      LightInstance inst;
			memcpy(&inst.Chunk,&pLight->m_Chunk,sizeof(LIGHT_CHUNK_DESC));
      inst.Chunk = pLight->m_Chunk;

			if(bKeepInLocalSpace)

				//inst.vPos.Set(&pNode->m_Chunk.pos.x);            
				inst.vPos.Set(pNode->m_Chunk.pos[0],pNode->m_Chunk.pos[1],pNode->m_Chunk.pos[2]);            

			else
				//CHANGED_BY_IVO
				//inst.vPos = Vec3d(matNodeMatrix.data[3]);
			  inst.vPos = matNodeMatrix.GetTranslationOLD();

			if(!pNode->m_pParent)
				assert( IsEquivalent(pNode->m_Chunk.pos,inst.vPos,VEC_EPSILON) );

      strncpy(inst.szName,pNode->m_Chunk.name, sizeof(inst.szName));

      // load proj texture
  		if(inst.Chunk.szLightImage[0])
      {
			  inst.pLightImage = GetRenderer()->EF_LoadTexture(inst.Chunk.szLightImage, FT_CLAMP, FT2_FORCECUBEMAP, eTT_Cubemap); 
        if (!inst.pLightImage->IsTextureLoaded())
          inst.pLightImage = NULL;
      }
		  else				
			  inst.pLightImage = NULL;

      m_lstLights.Add(inst);
    }

		if(pNode->m_pObj->m_ChunkHeader.ChunkType == ChunkType_Helper) 
    { // make helper
      CHelper * pHelper = (CHelper*)pNode->m_pObj;

      HelperInstance inst;
      inst.Chunk = pHelper->m_Chunk;
      
			if(bKeepInLocalSpace)
				inst.tMat.SetIdentity();
			else
				//CHANGED_BY_IVO
				//inst.tMat = Matrix(matNodeMatrix.matrix);
				inst.tMat = matNodeMatrix;

/*
			if(!pNode->m_pParent)
			{
				assert(inst.vPos == pNode->m_Chunk.pos);
		    float dot = inst.qRot.Dot(pNode->m_Chunk.rot);
				dot=dot;
			}
	*/
      strncpy(inst.szName,pNode->m_Chunk.name, sizeof(inst.szName));
      m_lstHelpers.Add(inst);
    }
  }

  m_lstGeomNames.AddList (lstOtherNames);

  return 1;
}
/*
CHelperInstance * CryStaticModel::GetHelper(const char * name)
{
  for(int i=0; i<m_Helpers.Count(); i++)
  {
    if(!strcmp(m_Helpers[i].name,name))
      return &m_Helpers[i];
  }
  
  return 0;
}
*/

// this timer measures the time spent in the CGF Loader
//double g_dTimeLoadCGF;
