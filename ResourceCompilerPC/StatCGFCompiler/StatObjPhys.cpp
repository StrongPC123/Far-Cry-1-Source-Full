////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   statobjphys.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: make physical representation
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatObj.h"
#include "MeshIdx.h"
#include "3dengine.h"

/////////////////////////////////////////////////////////////////////////////////////
// Buffer optimizer
/////////////////////////////////////////////////////////////////////////////////////

int CStatObj::FindInPosBuffer(const Vec3d & opt, Vec3d * _vbuff, int _vcount, list2<int> * pHash)
{ 
  for(int i=0; i<pHash->Count(); i++) 
  {
    if(
			IsEquivalent(*((Vec3d*)(&_vbuff[(*pHash)[i]].x)), *((Vec3d*)(&opt.x)), VEC_EPSILON)
			) 
      return (*pHash)[i];  
  }

  return -1;
}

void CStatObj::CompactPosBuffer(Vec3d * _vbuff, int * _vcount, list2<int> * pindices)
{
  int before = *_vcount; assert(before);
  if(!before)
    GetConsole()->Exit("Error: CStatObj::CompactPosBuffer: Input vertex count is zero");

  Vec3d * tmp_buff = new Vec3d[*_vcount];
  int tmp_count = 0;

  pindices->Clear();
 
  list2<int> pos_hash_table[256];//[256];

  for(uint v=0; v<(uint)(*_vcount); v++)
  {
    list2<int> * pHash = &pos_hash_table[(unsigned char)(_vbuff[v].x*100)];//[(unsigned char)(_vbuff[v].y*100)];
    int find = FindInPosBuffer( _vbuff[v], tmp_buff, tmp_count, pHash);
    if(find<0)
    {
      tmp_buff[tmp_count] = _vbuff[v];
      pindices->Add(tmp_count);

      pos_hash_table[(unsigned char)(_vbuff[v].x*100)]/*[(unsigned char)(_vbuff[v].y*100)]*/.Add(tmp_count);

      tmp_count++;
    }
    else
    {
      int u = (uint)find;
      pindices->Add(u);
    }
  }

  * _vcount = tmp_count;
  memcpy( _vbuff, tmp_buff, tmp_count*sizeof(Vec3d));

  delete [] tmp_buff;
}

// This function prepares 3 additional meshes: 
// usual physical representation(mat_phys or bounce koeff), 
// obstruct physical representation(mat_obstruct)
// and occlusion volume(mat_occl).
// Register physical stuff in physics engine.
void CStatObj::Physicalize()
{
  bool bShowINfo = (m_pTriData->m_nFaceCount>10000);

  if(bShowINfo)
    GetLog()->UpdateLoadingScreen("  Creating buffer for physics ...");

	// get phys material id's from game code
  IPhysMaterialEnumerator * pPhysMaterialEnumerator = ((C3DEngine*)Get3DEngine())->m_pPhysMaterialEnumerator;
  int i=0;
  for(i=0; pPhysMaterialEnumerator && i<m_pTriData->m_lstMatTable.Count(); i++)
    m_pTriData->m_lstMatTable[i].nGamePhysMatId = pPhysMaterialEnumerator->EnumPhysMaterial(m_pTriData->m_lstMatTable[i].sScriptMaterial);

  // find mat id's
  int nPhysMatID  = -1;
  int nObstrMatID = -1;
  int nOcclMatID  = -1;
  
  { 
    // find phys material id
    for(int m=0; m<m_pTriData->m_lstMatTable.Count(); m++)
    if(strstr(m_pTriData->m_lstMatTable[m].sScriptMaterial,"mat_phys"))
    {
      nPhysMatID = m;
      break;
    }

    // find obstruct material id
    for(int m=0; m<m_pTriData->m_lstMatTable.Count(); m++)
    if(strstr(m_pTriData->m_lstMatTable[m].sScriptMaterial,"mat_obstruct"))
    {
      nObstrMatID = m;
      break;
    }

    // find occlusion material id
    for(int m=0; m<m_pTriData->m_lstMatTable.Count(); m++)
    if(strstr(m_pTriData->m_lstMatTable[m].sScriptMaterial,"mat_occl"))
    {
      nOcclMatID = m;
      break;
    }
  }

#define MESH_PHYSIC 0
#define MESH_OBSTRUCT 1
#define MESH_OCCLUSION 2

  for(int nMesh = 0; nMesh<=2; nMesh++)
  { // fill physics indices
    list2<int> lstPhysIndices;
    list2<unsigned char> lstFaceMaterials;

    if(nMesh == MESH_PHYSIC)
    { // find all physicalized faces
      for(i=0; i<m_pTriData->m_nFaceCount; i++)
      {
        if( ((m_pTriData->m_lstMatTable[m_pTriData->m_pFaces[i].shader_id].m_Flags & MIF_PHYSIC) && nPhysMatID<0) || 
              m_pTriData->m_pFaces[i].shader_id == nPhysMatID )
        {
	        for(int v=0; v<3; v++)
		        lstPhysIndices.Add(m_pTriData->m_pFaces[i].v[v]);
          
          lstFaceMaterials.Add(m_pTriData->m_lstMatTable[m_pTriData->m_pFaces[i].shader_id].nGamePhysMatId);

          if(m_pTriData->m_pFaces[i].shader_id == nPhysMatID)
          { // remove face from list (it's not needed for rendering)
            if(m_pTriData->m_nFaceCount>1)
              m_pTriData->m_pFaces[i] = m_pTriData->m_pFaces[m_pTriData->m_nFaceCount-1];
            m_pTriData->m_nFaceCount--;
            i--;
          }
        }
      }
    }
    else if(nMesh == MESH_OBSTRUCT)
    { // find all obstruct faces
      if(nObstrMatID>=0)
      { // find all obstruct faces
        for(i=0; i<m_pTriData->m_nFaceCount; i++)
        {
          if(m_pTriData->m_pFaces[i].shader_id == nObstrMatID)
          {
	          for(int v=0; v<3; v++)
		          lstPhysIndices.Add(m_pTriData->m_pFaces[i].v[v]);
            
            lstFaceMaterials.Add(m_pTriData->m_lstMatTable[m_pTriData->m_pFaces[i].shader_id].nGamePhysMatId);

            // remove face from list (it's not needed for rendering)
            if(m_pTriData->m_nFaceCount>1)
              m_pTriData->m_pFaces[i] = m_pTriData->m_pFaces[m_pTriData->m_nFaceCount-1];
            m_pTriData->m_nFaceCount--;
            i--;
          }
        }
      }
    }
    else if(nMesh == MESH_OCCLUSION)
    {
      if(nOcclMatID>=0)
      { // find all occlusion faces
        for(i=0; i<m_pTriData->m_nFaceCount; i++)
        {
          if(m_pTriData->m_pFaces[i].shader_id == nOcclMatID)
          {
	          for(int v=0; v<3; v++)
		          lstPhysIndices.Add(m_pTriData->m_pFaces[i].v[v]);
            
            lstFaceMaterials.Add(m_pTriData->m_lstMatTable[m_pTriData->m_pFaces[i].shader_id].nGamePhysMatId);

            // remove face from list (it's not needed for rendering)
            if(m_pTriData->m_nFaceCount>1)
              m_pTriData->m_pFaces[i] = m_pTriData->m_pFaces[m_pTriData->m_nFaceCount-1];
            m_pTriData->m_nFaceCount--;
            i--;
          }
        }
      }
    }

	  if(lstPhysIndices.Count())
	  {
      Vec3d * pExVerts = new Vec3d[lstPhysIndices.Count()];
      
      for(i=0; i<lstPhysIndices.Count();i++)
        pExVerts[i] = m_pTriData->m_pVerts[lstPhysIndices[i]];

      if(bShowINfo)
        GetLog()->UpdateLoadingScreen("  Compacting buffer ...");

      int init_count = lstPhysIndices.Count();
      CompactPosBuffer(pExVerts, &init_count, &lstPhysIndices);

      if(bShowINfo)
        GetLog()->UpdateLoadingScreen("  Creating OBB tree ...");

      if(GetPhysicalWorld() && (nMesh==MESH_PHYSIC || nMesh==MESH_OBSTRUCT))
      {
		    IGeomManager *pGeoman = GetPhysicalWorld()->GetGeomManager();
				Vec3d ptmin=pExVerts[0],ptmax=pExVerts[0],sz;
				for(int i=1;i<lstPhysIndices.Count();i++)
				{
					ptmin.x = min(ptmin.x,pExVerts[i].x);
					ptmax.x = max(ptmax.x,pExVerts[i].x);
					ptmin.y = min(ptmin.y,pExVerts[i].y);
					ptmax.y = max(ptmax.y,pExVerts[i].y);
					ptmin.z = min(ptmin.z,pExVerts[i].z);
					ptmax.z = max(ptmax.z,pExVerts[i].z);
				}
				int nMinTrisPerNode=2, nMaxTrisPerNode=4;
				sz = ptmax-ptmin;
				int flags = mesh_multicontact1 | mesh_uchar_ids;
				float tol = 0.05f;
				flags |= lstPhysIndices.Count()<=60 ? mesh_SingleBB : mesh_OBB|mesh_AABB;
				if (strstr(m_szGeomName,"wheel"))
				{
					flags |= mesh_approx_cylinder;
					tol = 1.0f;
				} //else
					//flags |= mesh_approx_box;
				if (lstPhysIndices.Count()<600 && max(max(sz.x,sz.y),sz.z)>6) // make more dense OBBs for large (wrt terrain grid) objects
					nMinTrisPerNode = nMaxTrisPerNode = 1;
			  m_arrPhysGeomInfo[nMesh] = pGeoman->RegisterGeometry(pGeoman->CreateMesh((vectorf*)&pExVerts[0], &lstPhysIndices[0], 
					(short*)&lstFaceMaterials[0], lstPhysIndices.Count()/3, flags, true, true, tol, nMinTrisPerNode,nMaxTrisPerNode, 2.5f));
      }

      if(nOcclMatID>=0 && nMesh==MESH_OCCLUSION)
      {
        m_lstOcclVolVerts.AddList(pExVerts,init_count);
        m_lstOcclVolInds.AddList(lstPhysIndices);
      }

      delete [] pExVerts;
    }
  }

  if(bShowINfo)
    GetLog()->UpdateLoadingScreenPlus("ok");

#undef MESH_PHYSIC
#undef MESH_OBSTRUCT
#undef MESH_OCCLUSION
}
