/////////////////////////////////////////////////////////////////////////////////////
// Buffer optimizer
/////////////////////////////////////////////////////////////////////////////////////

#include "RenderPCH.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// PipVertex 
////////////////////////////////////////////////////////////////////////////////////////////////////

#define PIP_TEX_EPS 0.001f
#define PIP_VER_EPS 0.001f

bool struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F::operator == (struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F & other)
{
  assert(this != &other);

	// do y check first since x was used for hash
  return fabs(xyz.y-other.xyz.y)<PIP_VER_EPS && fabs(xyz.x-other.xyz.x)<PIP_VER_EPS && fabs(xyz.z-other.xyz.z)<PIP_VER_EPS &&
         fabs(normal.x-other.normal.x)<PIP_VER_EPS && fabs(normal.y-other.normal.y)<PIP_VER_EPS && fabs(normal.z-other.normal.z)<PIP_VER_EPS &&
         fabs(st[0]-other.st[0])<PIP_TEX_EPS && fabs(st[1]-other.st[1])<PIP_TEX_EPS &&
         (color.dcolor&0xffffff) == (other.color.dcolor&0xffffff);
}

int CLeafBuffer::FindInBuffer(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F &opt, SPipTangents &origBasis, uint nMatInfo, uint *uiInfo, struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F* _vbuff, SPipTangents *_vbasis, int _vcount, list2<unsigned short> * pHash, TArray<uint>& ShareNewInfo)
{
  for(int i=0; i<pHash->Count(); i++) 
  {
    int id = (*pHash)[i];
    if(_vbuff[id] == opt) 
    {
      if (ShareNewInfo[id] != nMatInfo)
        continue;
      if (CRenderer::CV_r_indexingWithTangents)
      {
        if (origBasis.m_Binormal.Dot(_vbasis[id].m_Binormal) > 0.005f && origBasis.m_Tangent.Dot(_vbasis[id].m_Tangent) > 0.005f)
          return (*pHash)[i];  
      }
      else
        return (*pHash)[i];  
    }
  }

  return -1;
}

void CLeafBuffer::CompactBuffer(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * _vbuff, SPipTangents *_tbuff, int * _vcount, TArray<unsigned short> * pindices, bool bShareVerts[128], uint *uiInfo)
{
  //assert(*_vcount);
  if(!*_vcount)
  {
//    iLog->Log("CLeafBuffer::CompactBuffer: Mesh has no geometry for rendering");
    return;
  }
  
  int vert_num_before = *_vcount;

  struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F * tmp_vbuff = new struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F[*_vcount];
  SPipTangents *tmp_tbuff = new SPipTangents[*_vcount];
  unsigned int tmp_count = 0;
  pindices->Free();
  TArray<uint> ShareNewInfo;

  list2<unsigned short> hash_table[256];//[256];

  for(unsigned int v=0; v<(unsigned int)(*_vcount); v++)
  {
    int nHashInd = (unsigned char)(_vbuff[v].xyz.x*100);
    uint nMInfo = uiInfo[v];
    uint nMatId = nMInfo & 255;
		int find = bShareVerts[nMatId] ? FindInBuffer( _vbuff[v], _tbuff[v], nMInfo, uiInfo, tmp_vbuff, tmp_tbuff, tmp_count, &hash_table[nHashInd], ShareNewInfo/*[(unsigned char)(_vbuff[v].pos.y*100)]*/) : -1;
    if(find < 0)
    { // not found
      tmp_vbuff[tmp_count] = _vbuff[v];
      tmp_tbuff[tmp_count] = _tbuff[v];
      pindices->AddElem(tmp_count);
      ShareNewInfo.AddElem(uiInfo[v]);

      hash_table[(unsigned char)(_vbuff[v].xyz.x*100)]/*[(unsigned char)(_vbuff[v].pos.y*100)]*/.Add(tmp_count);

      tmp_count++;
    }
    else
    { // found
      pindices->AddElem(find);
    }
  }

  *_vcount = tmp_count;
  cryMemcpy(_vbuff, tmp_vbuff, tmp_count*sizeof(struct_VERTEX_FORMAT_P3F_N_COL4UB_TEX2F));
  cryMemcpy(_tbuff, tmp_tbuff, tmp_count*sizeof(SPipTangents));

  SAFE_DELETE_ARRAY (tmp_vbuff);
  SAFE_DELETE_ARRAY (tmp_tbuff);

  int ratio = 100*(*_vcount)/vert_num_before;
  CryLogComment("  Size after compression = %d %s ( %d -> %d )", ratio, "%", vert_num_before, *_vcount); 
}
