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
  IPhysMaterialEnumerator * pPhysMaterialEnumerator = Get3DEngine()->GetPhysMaterialEnumerator();

  // special mat id's
  int nPhysMatID  = -1;
  list2<int> arrObstrMatIDs;
  int nOcclMatID  = -1;
	list2<int> arrLeavesMatIDs;
  
  for(int m=0; m<m_pTriData->m_lstMatTable.Count(); m++)
	{
		// get phys material id's from game code
		if(pPhysMaterialEnumerator)
			m_pTriData->m_lstMatTable[m].nGamePhysMatId = pPhysMaterialEnumerator->EnumPhysMaterial(m_pTriData->m_lstMatTable[m].sScriptMaterial);

		// find phys material id
		if(strstr(m_pTriData->m_lstMatTable[m].sScriptMaterial,"mat_phys"))
		{ assert(nPhysMatID<0); nPhysMatID = m; }

		// find obstruct material id
		else if(strstr(m_pTriData->m_lstMatTable[m].sScriptMaterial,"mat_obstruct"))
		{ arrObstrMatIDs.Add(m); }

		// find obstruct material id
		else if	(pPhysMaterialEnumerator &&
						!pPhysMaterialEnumerator->IsCollidable(m_pTriData->m_lstMatTable[m].nGamePhysMatId) && 
						!(m_pTriData->m_lstMatTable[m].m_Flags & MIF_PHYSIC))
		{ arrLeavesMatIDs.Add(m); }

		// find occlusion material id
		else if(strstr(m_pTriData->m_lstMatTable[m].sScriptMaterial,"mat_occl"))
		{ assert(nOcclMatID<0); nOcclMatID = m; }
	}

#define MESH_PHYSIC 0
#define MESH_OBSTRUCT 1
#define MESH_LEAVES 2
#define MESH_OCCLUSION 3

  for(int nMesh = 0; nMesh<=3; nMesh++)
  { // fill physics indices
    list2<int> lstPhysIndices;
    list2<unsigned char> lstFaceMaterials;

    if(nMesh == MESH_PHYSIC)
    { // find all physicalized faces
      for(int i=0; i<m_pTriData->m_nFaceCount; i++)
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
      if(arrObstrMatIDs.Count())
      { // find all obstruct faces
        for(int i=0; i<m_pTriData->m_nFaceCount; i++)
        {
          if( arrObstrMatIDs.Find(m_pTriData->m_pFaces[i].shader_id)>=0 )
          {
	          for(int v=0; v<3; v++)
		          lstPhysIndices.Add(m_pTriData->m_pFaces[i].v[v]);
            
            lstFaceMaterials.Add(m_pTriData->m_lstMatTable[m_pTriData->m_pFaces[i].shader_id].nGamePhysMatId);

            // remove face from list (it's not needed for rendering)
						if(arrObstrMatIDs.Find(m_pTriData->m_pFaces[i].shader_id)>=0)
						{
							if(m_pTriData->m_nFaceCount>1)
								m_pTriData->m_pFaces[i] = m_pTriData->m_pFaces[m_pTriData->m_nFaceCount-1];
							m_pTriData->m_nFaceCount--;
							i--;
						}
          }
        }
      }
    }
		else if(nMesh == MESH_LEAVES)
		{ // find all obstruct faces
			if(arrLeavesMatIDs.Count())
			{ // find all obstruct faces
				for(int i=0; i<m_pTriData->m_nFaceCount; i++)
				{
					if( arrLeavesMatIDs.Find(m_pTriData->m_pFaces[i].shader_id)>=0 )
					{
						for(int v=0; v<3; v++)
							lstPhysIndices.Add(m_pTriData->m_pFaces[i].v[v]);
						lstFaceMaterials.Add(m_pTriData->m_lstMatTable[m_pTriData->m_pFaces[i].shader_id].nGamePhysMatId);
					}
				}
			}
		}
    else if(nMesh == MESH_OCCLUSION)
    {
      if(nOcclMatID>=0)
      { // find all occlusion faces
        for(int i=0; i<m_pTriData->m_nFaceCount; i++)
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
      Vec3d * pExVerts;
			int nInitVertCount;

			if (m_pTriData->m_lstGeomNames.Count()>0 && strstr(m_pTriData->m_lstGeomNames[0],"cloth")!=0)
			{
				pExVerts = m_pTriData->m_pVerts;
				nInitVertCount = m_pTriData->m_nVertCount;
			}
			else
			{
				pExVerts = new Vec3d[lstPhysIndices.Count()];
      
				for(int i=0; i<lstPhysIndices.Count();i++)
					pExVerts[i] = m_pTriData->m_pVerts[lstPhysIndices[i]];

				if(bShowINfo)
					GetLog()->UpdateLoadingScreen("  Compacting buffer ...");

				nInitVertCount = lstPhysIndices.Count();
				CompactPosBuffer(pExVerts, &nInitVertCount, &lstPhysIndices);
			}

      if(bShowINfo)
        GetLog()->UpdateLoadingScreen("  Creating OBB tree ...");

      if(GetPhysicalWorld() && (nMesh==MESH_PHYSIC || nMesh==MESH_OBSTRUCT || nMesh==MESH_LEAVES) && nInitVertCount>2)
      {
        int nPhysTris = lstPhysIndices.Count()/3;
        if(GetCVars()->e_check_number_of_physicalized_polygons && 
          nPhysTris > 100+m_pTriData->m_nFaceCount/2)
        { 
#if !defined(LINUX) //does not matter in dedicated server mode
					GetLog()->Log("Physicalized geometry contains too many polygons(%d of %d), for CGF: %s",
						nPhysTris, m_pTriData->m_nFaceCount, GetFileName() );
          GetLog()->Log("  Number of physicalized tris is more than 100 + number of all tris divided by 2");
#endif
        }

		    IGeomManager *pGeoman = GetPhysicalWorld()->GetGeomManager();
				Vec3d ptmin=pExVerts[0],ptmax=pExVerts[0],sz;
				for(int i=1;i<nInitVertCount;i++)
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
				} else
					flags |= mesh_approx_box | mesh_approx_sphere | mesh_approx_cylinder;
				if (lstPhysIndices.Count()<600 && max(max(sz.x,sz.y),sz.z)>6) // make more dense OBBs for large (wrt terrain grid) objects
					nMinTrisPerNode = nMaxTrisPerNode = 1;
				assert(nMesh<MAX_PHYS_GEOMS_IN_CGF);
			  m_arrPhysGeomInfo[nMesh] = pGeoman->RegisterGeometry(pGeoman->CreateMesh((vectorf*)&pExVerts[0], &lstPhysIndices[0], 
					(short*)&lstFaceMaterials[0], lstPhysIndices.Count()/3, flags, true, true, tol, nMinTrisPerNode,nMaxTrisPerNode, 2.5f));
				if (lstFaceMaterials.Count()>0)
					m_arrPhysGeomInfo[nMesh]->surface_idx = lstFaceMaterials[0];
      }

      if(nOcclMatID>=0 && nMesh==MESH_OCCLUSION)
      {
        m_lstOcclVolVerts.AddList(pExVerts,nInitVertCount);
        m_lstOcclVolInds.AddList(lstPhysIndices);
      }

			if (pExVerts!=m_pTriData->m_pVerts)
				delete [] pExVerts;
    }
  }

  if(bShowINfo)
    GetLog()->UpdateLoadingScreenPlus("ok");

#undef MESH_PHYSIC
#undef MESH_OBSTRUCT
#undef MESH_OCCLUSION
#undef MESH_LEAVES
}

void CStatObj::PhysicalizeCompiled()
{
//  if(bShowINfo)
  //  GetLog()->UpdateLoadingScreen("  Creating buffer for physics ...");

  // get phys material id's from game code
  IPhysMaterialEnumerator * pPhysMaterialEnumerator = Get3DEngine()->GetPhysMaterialEnumerator();
  {
    for(int i=0; pPhysMaterialEnumerator && i<m_pLeafBuffer->m_pMats->Count(); i++)
      (*m_pLeafBuffer->m_pMats)[i].nGamePhysMatId = pPhysMaterialEnumerator->EnumPhysMaterial((*m_pLeafBuffer->m_pMats)[i].sScriptMaterial);
  }

  // find mat id's
  int nPhysMatID  = -1;
  int nObstrMatID = -1;
  int nOcclMatID  = -1;

#define MESH_PHYSIC 0
#define MESH_OBSTRUCT 1
#define MESH_OCCLUSION 3

  for(int nMesh = 0; nMesh<=3; nMesh++)
  { // fill physics indices
    
  //  list2<int> & lstPhysIndices = 
//    list2<unsigned char> & lstFaceMaterials;

//    for(int i=0; i<3; i++)
//    {
/*      m_lstProxyVerts[nMesh];
      list2<int> & lstPhysIndices = m_lstProxyInds[nMesh].LoadFromBuffer(pSerBuf,nPos);
      LoadBuffer(m_vPhysBoxMin[i], sizeof(m_vPhysBoxMin[i]), pSerBuf, nPos);
      LoadBuffer(m_vPhysBoxMin[i], sizeof(m_vPhysBoxMin[i]), pSerBuf, nPos);*/
  //  }

    /*
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
    */

    list2<int> & lstPhysIndices = m_lstProxyInds[nMesh];//.LoadFromBuffer(pSerBuf,nPos);
//    LoadBuffer(m_vPhysBoxMin[i], sizeof(m_vPhysBoxMin[i]), pSerBuf, nPos);
  //  LoadBuffer(m_vPhysBoxMin[i], sizeof(m_vPhysBoxMin[i]), pSerBuf, nPos);

    if(lstPhysIndices.Count())
    {
      Vec3d * pExVerts = &m_lstProxyVerts[nMesh][0];
//      if(bShowINfo)
  //      GetLog()->UpdateLoadingScreen("  Creating OBB tree ...");

      if(GetPhysicalWorld() && (nMesh==MESH_PHYSIC || nMesh==MESH_OBSTRUCT))
      {
				int nPhysTris = lstPhysIndices.Count()/3;
				int nFaceCount = 0;
				m_pLeafBuffer->GetIndices(&nFaceCount);
				nFaceCount/=3;
				if(GetCVars()->e_check_number_of_physicalized_polygons && 
					nPhysTris > 100+nFaceCount/2)
				{
#if !defined(LINUX) //does not matter in dedicated server mode
					GetLog()->Log("Physicalized geometry contains too many polygons(%d of %d), for CGF: %s",
						nPhysTris, nFaceCount, GetFileName() );
					GetLog()->Log("  Number of physicalized tris is more than 100 + number of all tris divided by 2");
#endif
				}

        list2<unsigned char> & lstFaceMaterials = m_lstProxyFaceMaterials[nMesh];

        // remap shader id to game mat id
        for(int f=0; f<lstFaceMaterials.Count(); f++)
        {
          CMatInfo * pMat = m_pLeafBuffer->m_pMats->Get(lstFaceMaterials[f]);
          lstFaceMaterials[f] = pMat->nGamePhysMatId;
        }

        IGeomManager *pGeoman = GetPhysicalWorld()->GetGeomManager();
        Vec3d & ptmin = m_vPhysBoxMin[nMesh];
        Vec3d & ptmax = m_vPhysBoxMax[nMesh];

        int nMinTrisPerNode=2, nMaxTrisPerNode=4;
        Vec3d sz = ptmax - ptmin;
        int flags = mesh_multicontact1 | mesh_uchar_ids;
        float tol = 0.05f;
        flags |= lstPhysIndices.Count()<=60 ? mesh_SingleBB : mesh_OBB|mesh_AABB;
        if (strstr(m_szGeomName,"wheel"))
        {
          flags |= mesh_approx_cylinder;
          tol = 1.0f;
        } else
          flags |= mesh_approx_box | mesh_approx_sphere | mesh_approx_cylinder;
        if (lstPhysIndices.Count()<600 && max(max(sz.x,sz.y),sz.z)>6) // make more dense OBBs for large (wrt terrain grid) objects
          nMinTrisPerNode = nMaxTrisPerNode = 1;
        assert(!m_arrPhysGeomInfo[nMesh]);
        m_arrPhysGeomInfo[nMesh] = pGeoman->RegisterGeometry(pGeoman->CreateMesh((vectorf*)&pExVerts[0], &lstPhysIndices[0], 
          (short*)&lstFaceMaterials[0], lstPhysIndices.Count()/3, flags, true, true, tol, nMinTrisPerNode,nMaxTrisPerNode, 2.5f));
        assert(m_arrPhysGeomInfo[nMesh]->nRefCount == 1);
				if (lstFaceMaterials.Count()>0)
					m_arrPhysGeomInfo[nMesh]->surface_idx = lstFaceMaterials[0];
      }

      if(nOcclMatID>=0 && nMesh==MESH_OCCLUSION)
      {
        m_lstOcclVolVerts.AddList(m_lstProxyVerts[nMesh]);
        m_lstOcclVolInds.AddList(lstPhysIndices);
      }

//      delete [] pExVerts;
    }
    /*else if(lstPhysIndices.Count())
    {
			Warning(0,GetFileName(),"CStatObj::Physicalize: proxy geometry contains more than 200 polygons - skipped, for CGF: %s",GetFileName() );
    }*/

    m_lstProxyInds[nMesh].Reset();
    m_lstProxyVerts[nMesh].Reset();
    m_lstProxyFaceMaterials[nMesh].Reset();
  }

//  if(bShowINfo)
  //  GetLog()->UpdateLoadingScreenPlus("ok");

#undef MESH_PHYSIC
#undef MESH_OBSTRUCT
#undef MESH_OCCLUSION
}