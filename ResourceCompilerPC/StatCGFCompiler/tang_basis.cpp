////////////////////////////////////////////////////////////////////////////////////////////////
//
//  bm
//
////////////////////////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "StatCGFCompiler.h"

#define EPS  0.00001f

// from nvidia kitchen 
bool CSimpleLeafBuffer::compute_tangent( const float * v0, const float * v1, const float * v2, 
                      const float * t0, const float * t1, const float * t2, 
                      Vec3d & tangent, Vec3d & binormal, Vec3d & tnormal, Vec3d & face_normal)
{
  Vec3d cp;
  Vec3d e0;
  Vec3d e1;

  tangent  = Vec3d(0,0,1);
  binormal = Vec3d(0,1,0);
  tnormal  = Vec3d(1,0,0);

  // x
  e0[0] = v1[0] - v0[0];
  e0[1] = t1[0] - t0[0];
  e0[2] = t1[1] - t0[1];

  e1[0] = v2[0] - v0[0];
  e1[1] = t2[0] - t0[0];
  e1[2] = t2[1] - t0[1];


  cp = e0.Cross(e1);

  if ( fabs(cp[0]) > EPS )
  {
    tangent[0] = -cp[1] / cp[0];
    binormal[0] = -cp[2] / cp[0];
  }

  // y
  e0[0] = v1[1] - v0[1];
  e0[1] = t1[0] - t0[0];
  e0[2] = t1[1] - t0[1];

  e1[0] = v2[1] - v0[1];
  e1[1] = t2[0] - t0[0];
  e1[2] = t2[1] - t0[1];

  cp = e0.Cross(e1);

  if ( fabs(cp[0]) > EPS )
  {
    tangent[1] = -cp[1] / cp[0];
    binormal[1] = -cp[2] / cp[0];
  }

  // z
  e0[0] = v1[2] - v0[2];
  e0[1] = t1[0] - t0[0];
  e0[2] = t1[1] - t0[1];

  e1[0] = v2[2] - v0[2];
  e1[1] = t2[0] - t0[0];
  e1[2] = t2[1] - t0[1];

  cp = e0.Cross(e1);

  if ( fabs(cp[0]) > EPS )
  {
    tangent[2] = -cp[1] / cp[0];
    binormal[2] = -cp[2] / cp[0];
  }

  tangent.Normalize();
  binormal.Normalize();

  tnormal = tangent.Cross(binormal);
  tnormal.Normalize();

  // Gram-Schmidt orthogonalization process for B
  // compute the cross product B=NxT to obtain 
  // an orthogonal basis
  //binormal = tnormal.Cross(tangent);

  if (tnormal.Dot(face_normal) < 0)
    tnormal = -tnormal;

  return true;
}


#include "NvTriStrip/NVTriStrip.h"

static _inline int sGetHash(float *v, float *n)
{
  return (int)(v[0]*64.0f+v[1]*64.0f+v[2]*64.0f+n[0]*2.0f+n[1]*2.0f+n[2]*2.0f) & 511;
}

static _inline void sAddToHash(TArray<unsigned short>& hash, unsigned short ind)
{
  int i;

  for (i=0; i<hash.Num(); i++)
  {
    if (hash[i] == ind)
      return;
  }
  hash.AddElem(ind);
}

static void sGetObjectSpaceVectors( Vec3d &outvA, Vec3d &outvB, Vec3d &outvC )
{
	outvA =	Vec3d(1,0,0);
	outvB =	Vec3d(0,1,0);
	outvC =	Vec3d(0,0,1);				
}
/*
float sCalcAngle( Vec3d &invA, Vec3d &invB )
{
	float LengthQ = invA.Length() * invB.Length();

	if(LengthQ < 0.01f)
    LengthQ = 0.01f;

	return acosf(invA.Dot(invB) / LengthQ);
}
*/

// orthogonalize the base vectors
//! /param v0 input [0..2] position vertex 1
//! /param v1 input [0..2] position vertex 2
//! /param v2 input [0..2] position vertex 3
//! /param t0 input [0..1] texture coordinate vertex 1
//! /param t1 input [0..1] texture coordinate vertex 2
//! /param t2 input [0..1] texture coordinate vertex 3
//! /param tangent output vector 1
//! /param binormal output vector 2
//! /param tnormal output vector 3
/*void CSimpleLeafBuffer::compute_tangent_base_CS( const float *v0, const float *v1, const float *v2, 
                      const float *t0, const float *t1, const float *t2, 
                      Vec3d &tangent, Vec3d &binormal, Vec3d &tnormal )
{
	float fA[2]={ t1[0]-t0[0], t1[1]-t0[1] }, fB[2]={ t2[0]-t0[0], t2[1]-t0[1] };

	float fOrientation = fA[0]*fB[1]-fA[1]*fB[0];

  Vec3d vfNormal = Vec3d(0,0,1);
	compute_tangent_CS(v0,v1,v2,t0,t1,t2,tangent,binormal,tnormal,vfNormal);

	// make sure they are orthogonal
	tnormal = tangent.Cross(binormal);
  tnormal.Normalize();
	binormal = tnormal.Cross(tangent);
  binormal.Normalize();
	if(fOrientation < 0)
    tnormal = -tnormal;
}	*/

#define TV_EPS 0.001f
#define TN_EPS 0.0001f

bool CSimpleLeafBuffer::PrepareTexSpaceBasis()
{
  int hash;
  int i, j;

  // allocate if not ready
  // process faces
  byte *pD = (byte *)m_pSecVertBuffer->m_VS[VSF_GENERAL].m_VData;
  SBufInfoTable *pOffs = &gBufInfoTable[m_pSecVertBuffer->m_vertexformat];
  int Stride = m_VertexSize[m_pSecVertBuffer->m_vertexformat]; // position stride

  // Get pointers to positions, TexCoords and Normals
  byte *verts = pD;
  int StrN; // Normals Stride
  byte *norms;
  int StrTC;  // Tex coords stride
  byte *tc0;
  if (pOffs->OffsNormal)
  {
    norms = &pD[pOffs->OffsNormal];
    StrN = Stride;
  }
  else
  {
    norms = (byte *)this->m_TempNormals;
    StrN = sizeof(Vec3d);
  }

  if (pOffs->OffsTC)
  {
    tc0 = &pD[pOffs->OffsTC];
    StrTC = Stride;
  }
  else
  {
    tc0 = (byte *)this->m_TempTexCoords;
    StrTC = sizeof(SMRendTexVert);
  }
  // If there is no tex coords or normals ignore this mesh
  if (!tc0 || !norms)
    return false;

  if(!m_pBasises)
    m_pBasises = new CBasis[m_SecVertCount];
  memset(m_pBasises, 0, sizeof(CBasis)*m_SecVertCount);

  // Indices hash table for smoothing of vectors between different materials
  TArray<unsigned short> hash_table[2][512];  // 0. for TS; 1. for CS

  // array for avoid smooothing multiple times
  TArray<bool> bUsed;
  bUsed.Create(m_SecVertCount);

  // Clone space map
/*  CPBCloneMapDest *pCM = NULL;
  bool bCM = false;*/
  bool bSpace[2];

  // Init hash tables
  for (i=0; i<2; i++)
  {
    bSpace[i] = false;
    for (j=0; j<512; j++)
    {
      hash_table[i][j].Free();
    }
  }

  // Non-optimized geometry case (usually in Debug mode)
  if (m_nPrimetiveType != R_PRIMV_MULTI_GROUPS)
  {
    // All materials
    for (int nm=0; nm<m_pMats->Count(); nm++)
    {
      CMatInfo *mi = m_pMats->Get(nm);
/*      if (pCM)
      {
        delete pCM;
        pCM = NULL;
      }
	*/	/*
      if (mi->shaderItem.m_pShaderResources)
      {
        STexPic *pBumpTex = mi->shaderItem.m_pShaderResources->m_Textures[EFTT_BUMP].m_TU.m_TexPic;

        // If clone space texture is present
        if (pBumpTex && (pBumpTex->m_Flags2 & FT2_CLONESPACE))
        {
          char name[128];
          strcpy(name, pBumpTex->m_SourceName.c_str());
          StripExtension(name, name);
          AddExtension(name, ".cln");
          FILE *fp = iSystem->GetIPak()->FOpen (name, "rb");
          if (fp)
          {
            iSystem->GetIPak()->FClose(fp);
            pCM = new CPBCloneMapDest;
            pCM->LoadCloneMap(name);
            bCM = true;
          }
        }
        else // otherwise if it's object-space polybump ignore this material
        if (pBumpTex && (pBumpTex->m_Flags2 & FT2_POLYBUMP))
          continue;
      }*/
      // For clone-space - another hash index for smoothing
      int nIndHash = 0;//(pCM != 0) ? 1 : 0;
      if (mi->nNumIndices)
        bSpace[nIndHash] = true;

      for(i=0; i<mi->nNumIndices-2; i+=3)
	    {
		    unsigned short * face = &GetIndices()[i+mi->nFirstIndexId];

		    float *n[3] = 
        {
			    (float *)&norms[face[0]*StrN],
			    (float *)&norms[face[1]*StrN],
			    (float *)&norms[face[2]*StrN],
		    };
    	
		    float *v[3] = 
        {
			    (float *)&verts[face[0]*Stride],
			    (float *)&verts[face[1]*Stride],
			    (float *)&verts[face[2]*Stride],
		    };

		    float *tc[3] =
		    {
			    (float *)&tc0[face[0]*StrTC],
			    (float *)&tc0[face[1]*StrTC],
			    (float *)&tc0[face[2]*StrTC],
		    };
        // Place indices to hash (for future smoothing)
        hash = sGetHash(v[0], n[0]);
        sAddToHash(hash_table[nIndHash][hash], face[0]);
        hash = sGetHash(v[1], n[1]);
        sAddToHash(hash_table[nIndHash][hash], face[1]);
        hash = sGetHash(v[2], n[2]);
        sAddToHash(hash_table[nIndHash][hash], face[2]);
        
        // Compute the face normal based on vertex normals
        Vec3d face_normal;
        face_normal.x = n[0][0] + n[1][0] + n[2][0];
        face_normal.y = n[0][1] + n[1][1] + n[2][1];
        face_normal.z = n[0][2] + n[1][2] + n[2][2];

		    face_normal.Normalize();

        // If we have clone-space
        {
          Vec3d tangents[3];
          Vec3d binormals[3];
          Vec3d tnormals[3];

          // compute tangent vectors
          compute_tangent(v[0], v[1], v[2], tc[0], tc[1], tc[2], tangents[0], binormals[0], tnormals[0], face_normal);
          compute_tangent(v[1], v[2], v[0], tc[1], tc[2], tc[0], tangents[1], binormals[1], tnormals[1], face_normal);
          compute_tangent(v[2], v[0], v[1], tc[2], tc[0], tc[1], tangents[2], binormals[2], tnormals[2], face_normal);

          // accumulate
          for(j=0; j<3; j++)
	        {
            m_pBasises[face[j]].tangent  += tangents [j];
		        m_pBasises[face[j]].binormal += binormals[j];
            m_pBasises[face[j]].tnormal += tnormals[j];
	        }
        }
	    }
    }
    // smooth tangent vectors between different materials with the same positions and normals
    if (1)//CRenderer::CV_r_smoothtangents)
    {
      // Shared indices array for smoothing
      TArray<int> Inds;
      Inds.Create(32);
      // Smooth separatelly for tangent-space and clone-space
      for (int nn=0; nn<2; nn++)
      {
        // If this space wasn't used ignore it
        if (!bSpace[nn])
          continue;
        for (i=0; i<m_SecVertCount; i++)
        {
          // if this vertex was already used ignore it
          if (bUsed[i])
            continue;
          bUsed[i] = true;
          Inds.SetUse(0);
          Inds.AddElem(i);
          // Get position and normal for the current index i
  	      float *v = (float *)&verts[i*Stride];
  	      float *n = (float *)&norms[i*StrN];
          hash = sGetHash(v, n);
          for (j=0; j<hash_table[nn][hash].Num(); j++)
          {
            int m = hash_table[nn][hash][j];
            if (m == i)
              continue;
            // Get position and normal for the new index m
    	      float *v1 = (float *)&verts[m*Stride];
    	      float *n1 = (float *)&norms[m*StrN];
            // If position and normal are the same take this index int account
            if (fabs(v1[0]-v[0])<TV_EPS && fabs(v1[1]-v[1])<TV_EPS && fabs(v1[2]-v[2])<TV_EPS && fabs(n1[0]-n[0])<TN_EPS && fabs(n1[1]-n[1])<TN_EPS && fabs(n1[2]-n[2])<TN_EPS)
            {
              // Check angle between tangent vectors to avoid degenerated tangent vectors
              Vec3d tang, tang1;
              tang = m_pBasises[m].binormal;
              tang.NormalizeFast();
              tang1 = m_pBasises[i].binormal;
              tang1.NormalizeFast();
              float dot = tang.Dot(tang1);
              if (dot > 0.5f)
              {
                tang = m_pBasises[m].tangent;
                tang.NormalizeFast();
                tang1 = m_pBasises[i].tangent;
                tang1.NormalizeFast();
                dot = tang.Dot(tang1);
                if (dot > 0.5f)
                {
                  tang = m_pBasises[m].tnormal;
                  tang.NormalizeFast();
                  tang1 = m_pBasises[i].tnormal;
                  tang1.NormalizeFast();
                  float dot = tang.Dot(tang1);
                  if (dot > 0.5f)
                    Inds.AddElem(m); // Add the new index to the shared indices list
                }
              }
            }
          }
          // If we have more then one shared index smooth vectors between them
          if (Inds.Num() > 1)
          {
            Vec3d tang   = m_pBasises[Inds[0]].tangent;
            Vec3d binorm = m_pBasises[Inds[0]].binormal;
            Vec3d tnorm  = m_pBasises[Inds[0]].tnormal;
            for (j=1; j<Inds.Num(); j++)
            {
              int m = Inds[j];
              bUsed[m] = true;
              tang   += m_pBasises[m].tangent;
              binorm += m_pBasises[m].binormal;
              tnorm  += m_pBasises[m].tnormal;
            }
            for (j=0; j<Inds.Num(); j++)
            {
              int m = Inds[j];
              m_pBasises[m].tangent = tang;
              m_pBasises[m].binormal = binorm;
              m_pBasises[m].tnormal = tnorm;
            }
          }
        }
      }
    }
  }
  else // Optimized geometry (Stripified)
  {
    unsigned int n;
    for (int nm=0; nm<m_pMats->Count(); nm++)
    {
      CMatInfo *mi = m_pMats->Get(nm);
/*      if (pCM)
      {
        delete pCM;
        pCM = NULL;
      }*//*
      if (mi->shaderItem.m_pShaderResources)
      {
        STexPic *pBumpTex = mi->shaderItem.m_pShaderResources->m_Textures[EFTT_BUMP].m_TU.m_TexPic;

        // If we have clone-space texture
        if (pBumpTex && (pBumpTex->m_Flags2 & FT2_CLONESPACE))
        {
          char name[128];
          strcpy(name, pBumpTex->m_SourceName.c_str());
          StripExtension(name, name);
          AddExtension(name, ".cln");
          FILE *fp = iSystem->GetIPak()->FOpen (name, "rb");
          if (fp)
          {
            iSystem->GetIPak()->FClose(fp);
            pCM = new CPBCloneMapDest;
            pCM->LoadCloneMap(name);
            bCM = true;
          }
        }
        else // if it's object-space bump ignore this material
        if (pBumpTex && (pBumpTex->m_Flags2 & FT2_POLYBUMP))
          continue;
      }		 */
      // For clone-space - another hash index for smoothing
      int nIndHash = 0;//(pCM != 0) ? 1 : 0;
      if (mi->nNumIndices)
        bSpace[nIndHash] = true;

      int nOffs = mi->nFirstIndexId;
      for (j=0; j<mi->m_dwNumSections; j++)
      {
        SPrimitiveGroup *g = &mi->m_pPrimitiveGroups[j];
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

          float *n[3] = 
          {
            (float *)&norms[i0*StrN],
            (float *)&norms[i1*StrN],
            (float *)&norms[i2*StrN],
          };

          float *v[3] = 
          {
            (float *)&verts[i0*Stride],
            (float *)&verts[i1*Stride],
            (float *)&verts[i2*Stride],
          };

          float *tc[3] =
          {
            (float *)&tc0[i0*StrTC],
            (float *)&tc0[i1*StrTC],
            (float *)&tc0[i2*StrTC],
          };

          // Place indices to hash (for future smoothing)
          hash = sGetHash(v[0], n[0]);
          sAddToHash(hash_table[nIndHash][hash], i0);
          hash = sGetHash(v[1], n[1]);
          sAddToHash(hash_table[nIndHash][hash], i1);
          hash = sGetHash(v[2], n[2]);
          sAddToHash(hash_table[nIndHash][hash], i2);

          // Compute the face normal based on vertex normals
          Vec3d face_normal;
          face_normal.x = n[0][0] + n[1][0] + n[2][0];
          face_normal.y = n[0][1] + n[1][1] + n[2][1];
          face_normal.z = n[0][2] + n[1][2] + n[2][2];

          //Vec3d::Normalize(face_normal);
          face_normal.Normalize();

          {
            Vec3d tangents[3];
            Vec3d binormals[3];
            Vec3d tnormals[3];

            compute_tangent(v[0], v[1], v[2], tc[0], tc[1], tc[2], tangents[0], binormals[0], tnormals[0], face_normal);
            compute_tangent(v[1], v[2], v[0], tc[1], tc[2], tc[0], tangents[1], binormals[1], tnormals[1], face_normal);
            compute_tangent(v[2], v[0], v[1], tc[2], tc[0], tc[1], tangents[2], binormals[2], tnormals[2], face_normal);

            // accumulate
            m_pBasises[i0].tangent  += tangents [0];
            m_pBasises[i0].binormal += binormals[0];
            m_pBasises[i0].tnormal += tnormals[0];
            m_pBasises[i1].tangent  += tangents [1];
            m_pBasises[i1].binormal += binormals[1];
            m_pBasises[i1].tnormal += tnormals[1];
            m_pBasises[i2].tangent  += tangents [2];
            m_pBasises[i2].binormal += binormals[2];
            m_pBasises[i2].tnormal += tnormals[2];
          }
        }
      }
    }
    // smooth tangent vectors between different materials with the same positions and normals
    if (1)//CRenderer::CV_r_smoothtangents)
    {
      // Shared indices array for smoothing
      TArray<int> Inds;
      Inds.Create(32);
      // Smooth separatelly for tangent-space and clone-space
      for (int nn=0; nn<2; nn++)
      {
        // If this space wasn't used ignore it
        if (!bSpace[nn])
          continue;
        for (i=0; i<m_SecVertCount; i++)
        {
          // if this vertex was already used ignore it
          if (bUsed[i])
            continue;
          bUsed[i] = true;
          Inds.SetUse(0);
          Inds.AddElem(i);
          // Get position and normal for the current index i
  	      float *v = (float *)&verts[i*Stride];
  	      float *n = (float *)&norms[i*StrN];
          hash = sGetHash(v, n);
          for (j=0; j<hash_table[nn][hash].Num(); j++)
          {
            int m = hash_table[nn][hash][j];
            // If it's the same index ignore it
            if (m == i)
              continue;
            // Get position and normal for the tested index m
    	      float *v1 = (float *)&verts[m*Stride];
    	      float *n1 = (float *)&norms[m*StrN];
            // If position and normal are the same take this index int account
            if (fabs(v1[0]-v[0])<TV_EPS && fabs(v1[1]-v[1])<TV_EPS && fabs(v1[2]-v[2])<TV_EPS && fabs(n1[0]-n[0])<TN_EPS && fabs(n1[1]-n[1])<TN_EPS && fabs(n1[2]-n[2])<TN_EPS)
            {
              // Check angle between tangent vectors to avoid degenerated tangent vectors
              Vec3d tang, tang1;
              tang = m_pBasises[m].binormal;
              tang.NormalizeFast();
              tang1 = m_pBasises[i].binormal;
              tang1.NormalizeFast();
              if (tang.Dot(tang1) > 0.5f)
              {
                tang = m_pBasises[m].tangent;
                tang.NormalizeFast();
                tang1 = m_pBasises[i].tangent;
                tang1.NormalizeFast();
                if (tang.Dot(tang1) > 0.5f)
                {
                  tang = m_pBasises[m].tnormal;
                  tang.NormalizeFast();
                  tang1 = m_pBasises[i].tnormal;
                  tang1.NormalizeFast();
                  if (tang.Dot(tang1) > 0.5f)
                    Inds.AddElem(m);  // Add the new index to the shared indices list
                }
              }
            }
          }
          // If we have more then one shared index smooth vectors between them
          if (Inds.Num() > 1)
          {
            Vec3d tang   = m_pBasises[Inds[0]].tangent;
            Vec3d binorm = m_pBasises[Inds[0]].binormal;
            Vec3d tnorm  = m_pBasises[Inds[0]].tnormal;
            for (j=1; j<Inds.Num(); j++)
            {
              int m = Inds[j];
              bUsed[m] = true;
              tang   += m_pBasises[m].tangent;
              binorm += m_pBasises[m].binormal;
              tnorm  += m_pBasises[m].tnormal;
            }
            for (j=0; j<Inds.Num(); j++)
            {
              int m = Inds[j];
              m_pBasises[m].tangent = tang;
              m_pBasises[m].binormal = binorm;
              m_pBasises[m].tnormal = tnorm;
            }
          }
        }
      }
    }
  }

  // normalize
	for(int v=0; v<m_SecVertCount; v++)
  {
    m_pBasises[v].tangent.Normalize();
    m_pBasises[v].binormal.Normalize();
    m_pBasises[v].tnormal.Normalize();
    // if we have CV_r_unifytangentnormals (set by default) use Normals from vertex normal as Tangent normal
    // and orthonormalize tangent vectors
    if (1)//CRenderer::CV_r_unifytangentnormals && !bCM)
    {
      Vec3d *n = (Vec3d *)&norms[v*StrN];
      m_pBasises[v].tnormal = *n;
      Vec3d bin = m_pBasises[v].binormal;
      Vec3d tan = m_pBasises[v].tangent;
      m_pBasises[v].tangent = m_pBasises[v].tnormal.Cross(m_pBasises[v].binormal);
      if (m_pBasises[v].tangent.Dot(tan) < 0.0f)
        m_pBasises[v].tangent = -m_pBasises[v].tangent;
      m_pBasises[v].binormal = m_pBasises[v].tnormal.Cross(m_pBasises[v].tangent);
      if (m_pBasises[v].binormal.Dot(bin) < 0.0f)
        m_pBasises[v].binormal = -m_pBasises[v].binormal;
      m_pBasises[v].tangent.Normalize();
      m_pBasises[v].binormal.Normalize();
    }
  }

  return true;
}
