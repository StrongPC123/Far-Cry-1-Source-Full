////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   savecfg.cpp
//  Version:     v1.00
//  Created:     28/5/2002 by Andrey
//  Compilers:   Visual Studio.NET
//  Description: cgf file writer
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "3dEngine.h"
#include "CryStaticModel.h" 
#include "statobj.h"
#include "MeshIdx.h"

bool C3DEngine::WriteMaterials(TArray<CHUNK_HEADER>& Chunks, TArray<IShader *>& Shaders, FILE *out, int &MatChunk)
{
  CHUNK_HEADER ch;
	
	// the type and version of the material chunk
	typedef MTL_CHUNK_DESC_0745 MTL_CHUNK_TYPE;

  MTL_CHUNK_TYPE chunk;
  memset (&chunk, 0, sizeof(chunk));
  int Offs = ftell(out);
  
  ch.ChunkType = ChunkType_Mtl;
  ch.ChunkID = Chunks.Num();
  MatChunk = ch.ChunkID;
	ch.ChunkVersion = MTL_CHUNK_TYPE::VERSION;
  ch.FileOffset = Offs;
  Offs += sizeof(chunk) + sizeof(int) * Shaders.Num();
  Chunks.AddElem(ch);
  int i=0;
  for (i=0; i<Shaders.Num(); i++)
  {
    ch.ChunkType = ChunkType_Mtl;
    ch.ChunkID = Chunks.Num();
    ch.ChunkVersion = MTL_CHUNK_TYPE::VERSION;
    ch.FileOffset = Offs;
    Offs += sizeof(chunk);
    Chunks.AddElem(ch);
  }
  
  chunk.chdr = Chunks[MatChunk];
  chunk.MtlType = MTL_MULTI;
  strcpy(chunk.name, "Multi");
  chunk.nChildren = Shaders.Num();
  fwrite(&chunk, 1, sizeof(chunk), out);
  for (i=0; i<Shaders.Num(); i++)
  {
    int nn = MatChunk+i;
    fwrite(&nn, 1, sizeof(int), out);
  }
  /*for (i=0; i<Shaders.Num(); i++)
  {
    memset(&chunk, 0, sizeof(chunk));
    chunk.chdr = Chunks[MatChunk+i+1];
    chunk.MtlType = MTL_STANDARD;
    IShader *sh = Shaders[i];
    strcpy(chunk.name, sh->GetName());
    if (sh->GetTemplates())
    {
      SEfTemplates *eft = sh->GetTemplates();
      for (int j=0; j<16; j++)
      {
        if (!eft->m_TexInfo[j].m_Name.empty())
        {
          switch (j)
          {
            case EFTT_DECAL:
              strcpy(chunk.tex_d.name, eft->m_TexInfo[j].m_Name);
              break;
            case EFTT_BUMP:
            case EFTT_DSDTBUMP:
              strcpy(chunk.tex_b.name, eft->m_TexInfo[j].m_Name);
              break;
            case EFTT_GLOSS:
              strcpy(chunk.tex_g.name, eft->m_TexInfo[j].m_Name);
              break;
            case EFTT_OPACITY:
              strcpy(chunk.tex_o.name, eft->m_TexInfo[j].m_Name);
              break;
            case EFTT_ENVIRONMENT:
            case EFTT_REFLECTION:
              strcpy(chunk.tex_rl.name, eft->m_TexInfo[j].m_Name);
              break;
            case EFTT_REFRACTION:
              strcpy(chunk.tex_rr.name, eft->m_TexInfo[j].m_Name);
              break;
            case EFTT_CUBEMAP:
              strcpy(chunk.tex_c.name, eft->m_TexInfo[j].m_Name);
              break;
            case EFTT_SPECULAR:
              strcpy(chunk.tex_s.name, eft->m_TexInfo[j].m_Name);
              break;
            case EFTT_DETAIL:
              strcpy(chunk.tex_det.name, eft->m_TexInfo[j].m_Name);
              break;
          }
        }
      }
    }
    else
      strcpy(chunk.tex_d.name, chunk.name);
    chunk.Dyn_Bounce = 1;
    chunk.Dyn_StaticFriction = 1;
    chunk.Dyn_SlidingFriction = 1;
    chunk.nChildren = -1;
    fwrite(&chunk, 1, sizeof(chunk), out);
  }*/

  return true;
}

struct SNodeInfo
{
  int bGlobals;
  int Sector;
  CStatObj *pObj;
  char name[64];
};

bool C3DEngine::WriteNodes(TArray<CHUNK_HEADER>& Chunks, TArray<NODE_CHUNK_DESC>& Nodes, FILE *out, TArray<SNodeInfo>& NI, int& MatChunk, int& ExpFrame, std::vector<IStatObj *>& pObjs)
{
  CHUNK_HEADER ch;
  int numMeshs = 0;
  NODE_CHUNK_DESC nd;
  SNodeInfo ni;
  int i;
  
  NI.Free();
  ExpFrame++;
  
  int nChildren;

  std::vector<IStatObj*>::iterator n;
  for (n=pObjs.begin();n!=pObjs.end();n++)
  {
    IStatObj *io = (*n);
    CStatObj *co = (CStatObj *)io;

    memset(&nd, 0, sizeof(NODE_CHUNK_DESC));
    strcpy(nd.name, co->m_szGeomName);
    nd.tm.SetIdentity();
    nd.scl = Vec3(1.0,1.0,1.0);
    nd.MatID = MatChunk;
    nd.ObjectID = numMeshs++;
    int nnd = Nodes.Num();
    Nodes.AddElem(nd);
    ni.bGlobals = 1;
    ni.pObj = co;
    strcpy(ni.name, nd.name);
    NI.AddElem(ni);
    nChildren = 0;
    Nodes[nnd].nChildren = nChildren;
  }
  
  memset(&ch, 0, sizeof(CHUNK_HEADER));
  int no = Chunks.Num() + Nodes.Num();
  for (i=0; i<Nodes.Num(); i++)
  {
    Nodes[i].ObjectID += no;
    ch.ChunkType = ChunkType_Node;
    ch.ChunkID = Chunks.Num();
    ch.ChunkVersion = NODE_CHUNK_DESC_VERSION;
    ch.FileOffset = ftell(out);
    Nodes[i].chdr = ch;
    Chunks.AddElem(ch);
    fwrite(&Nodes[i], sizeof(NODE_CHUNK_DESC), 1, out);
    for (int j=0; j<Nodes[i].nChildren; j++)
    {
      int nn = Nodes[i].chdr.ChunkID+1+j;
      fwrite(&nn, sizeof(int), 1, out);
    }
  }
  return true;
}

bool C3DEngine::WriteNodeMesh(int nNode, MESH_CHUNK_DESC *chunk, FILE *out, TArray<IShader *>& Shaders, TArray<SNodeInfo>& NI, CStatObj *pObj)
{
  TArray<CryVertex> Verts;
  TArray<CryFace> Faces;
  TArray<CryUV> UVs;
  TArray<CryTexFace> TFaces;
  TArray<CryIRGB> Colors;
  int i, j;
  
  SNodeInfo *ni = &NI[nNode];

  CIndexedMesh *pMesh = pObj->GetTriData();
  
  for (i=0; i<pMesh->m_nFaceCount; i++)
  {
    CryFace fc;
    CryTexFace tf;

    fc.v0 = pMesh->m_pFaces[i].v[0];
    fc.v1 = pMesh->m_pFaces[i].v[1];
    fc.v2 = pMesh->m_pFaces[i].v[2];

    tf.t0 = pMesh->m_pFaces[i].t[0];
    tf.t1 = pMesh->m_pFaces[i].t[1];
    tf.t2 = pMesh->m_pFaces[i].t[2];

    int ns = pMesh->m_pFaces[i].shader_id;
    IShader *sh = pMesh->m_lstMatTable[ns].shaderItem.m_pShader;
    for (j=0; j<Shaders.Num(); j++)
    {
      if (!stricmp(sh->GetName(), Shaders[j]->GetName()))
        break;
    }
    assert (j != Shaders.Num());
    fc.MatID = j;
    fc.SmGroup = 0;
    Faces.AddElem(fc);
    TFaces.AddElem(tf);
  }

  for (i=0; i<pMesh->m_nVertCount; i++)
  {
    CryVertex vr;
    CryIRGB rgb;

    vr.p[0] = pMesh->m_pVerts[i].x * 100.0f;
    vr.p[1] = pMesh->m_pVerts[i].y * 100.0f;
    vr.p[2] = pMesh->m_pVerts[i].z * 100.0f;

    vr.n[0] = pMesh->m_pNorms[i].x;
    vr.n[1] = pMesh->m_pNorms[i].y;
    vr.n[2] = pMesh->m_pNorms[i].z;

    rgb.r = pMesh->m_pColor[i].r;
    rgb.g = pMesh->m_pColor[i].g;
    rgb.b = pMesh->m_pColor[i].b;

    Verts.AddElem(vr);
    Colors.AddElem(rgb);
  }

  for (i=0; i<pMesh->m_nCoorCount; i++)
  {
    CryUV uv;

    uv.u = pMesh->m_pCoors[i].s;
    uv.v = pMesh->m_pCoors[i].t;

    UVs.AddElem(uv);
  }
  
  chunk->nFaces = Faces.Num();
  chunk->nTVerts = UVs.Num();
  chunk->nVerts = Verts.Num();
  chunk->VertAnimID = 0;

  if (Colors.Num())
    chunk->HasVertexCol = 1;
  else
    chunk->HasVertexCol = 0;
  
  fwrite(chunk, 1, sizeof(MESH_CHUNK_DESC), out);
  
  fwrite(&Verts[0], sizeof(CryVertex), chunk->nVerts, out);
  fwrite(&Faces[0], sizeof(CryFace), chunk->nFaces, out);
  fwrite(&UVs[0], sizeof(CryUV), chunk->nTVerts, out);
  fwrite(&TFaces[0], sizeof(CryTexFace), chunk->nFaces, out);
  if (chunk->HasVertexCol)
    fwrite(&Colors[0], sizeof(CryIRGB), chunk->nVerts, out);
    
  return true;
}

bool C3DEngine::WriteMesh(TArray<CHUNK_HEADER>& Chunks, TArray<NODE_CHUNK_DESC>& Nodes, TArray<IShader *>& Shaders, FILE *out, TArray<SNodeInfo>& NI, int& MatChunk, int& ExpFrame)
{
  MESH_CHUNK_DESC chunk;
  CHUNK_HEADER ch;
  
  ExpFrame++;
  for (int i=0; i<Nodes.Num(); i++)
  {
    NODE_CHUNK_DESC *nd = &Nodes[i];
    SNodeInfo *ni = &NI[i];
    ch.ChunkType = ChunkType_Mesh;
    ch.ChunkID = Chunks.Num();
    ch.ChunkVersion = GeomFileVersion;
    ch.FileOffset = ftell(out);
    Chunks.AddElem(ch);
    
    chunk.chdr = ch;
    chunk.HasBoneInfo = 0;
    chunk.HasVertexCol = 0;
    
    WriteNodeMesh(i, &chunk, out, Shaders, NI, ni->pObj);
  }
  return true;
}

bool C3DEngine::WriteLights(TArray<CHUNK_HEADER>& Chunks, TArray<NODE_CHUNK_DESC>& Nodes, FILE *out, std::vector<IStatObj *>& pObjs)
{
  LIGHT_CHUNK_DESC lcd;
  CHUNK_HEADER ch;
  NODE_CHUNK_DESC nd;
  
  std::vector<IStatObj*>::iterator n;
  for (n=pObjs.begin();n!=pObjs.end();n++)
  {
    IStatObj *io = (*n);
    CStatObj *co = (CStatObj *)io;
//    CIndexedMesh *im = co->GetTriData();
    for (int j=0; j<co->m_lstLSources.Count(); j++)
    {
      CDLight *dl = &co->m_lstLSources[j];
        
      memset(&lcd, 0, sizeof(LIGHT_CHUNK_DESC));
      memset(&ch, 0, sizeof(CHUNK_HEADER));
      memset(&nd, 0, sizeof(NODE_CHUNK_DESC));
      int no = Chunks.Num();
      
      ch.ChunkType = ChunkType_Node;
      ch.ChunkID = Chunks.Num(); 
      ch.ChunkVersion = NODE_CHUNK_DESC_VERSION;
      ch.FileOffset = ftell(out);
      Chunks.AddElem(ch);
      
      nd.chdr = ch;
      nd.ObjectID = Chunks.Num();
      if (dl->m_Name)
        strcpy(nd.name, dl->m_Name);
      else
        nd.name[0] = 0;
      Vec3d Org;
/*      if (dl->m_OrigLight)
        Org = dl->m_OrigLight->m_Origin;
      else
        Org = dl->m_Origin;
*/      nd.pos.x = Org[0] * 100.0f;
      nd.pos.y = Org[1] * 100.0f;
      nd.pos.z = Org[2] * 100.0f;
      
      nd.tm.SetIdentity();
      nd.tm[3][0] = nd.pos.x;
      nd.tm[3][1] = nd.pos.y;
      nd.tm[3][2] = nd.pos.z;
      
      nd.scl = Vec3(1.0f, 1.0f, 1.0f);
      
      fwrite(&nd, sizeof(NODE_CHUNK_DESC), 1, out);
      
      
      // Light
      ch.ChunkType = ChunkType_Light;
      ch.ChunkID = Chunks.Num();
      ch.ChunkVersion = LIGHT_CHUNK_DESC_VERSION;
      ch.FileOffset = ftell(out);
      Chunks.AddElem(ch);
      
      lcd.chdr = ch;
      lcd.intens = dl->m_fRadius * 100.0f;
      if (lcd.intens <= 0)
        lcd.intens = 500;
      lcd.attenEnd = lcd.intens;
      UCol col;
      col.dcolor = dl->m_Color.GetTrue();
      lcd.color.r = col.bcolor[0];
      lcd.color.g = col.bcolor[1];
      lcd.color.b = col.bcolor[2];
      lcd.vDirection = dl->m_ProjAngles;
      lcd.fallsize = dl->m_fLightFrustumAngle;
      if (dl->m_pLightImage)
      {
        strcpy(lcd.szLightImage, dl->m_pLightImage->GetName());
        lcd.type = LT_OMNI;
      }
      else
      if (dl->m_Flags & DLF_DIRECTIONAL)
        lcd.type = LT_DIRECT;
      else
      if (dl->m_Flags & DLF_POINT)
      {
        lcd.useAtten = true;
        lcd.type = LT_OMNI;
      }
      fwrite(&lcd, sizeof(LIGHT_CHUNK_DESC), 1, out);
    }
  }

  return true;
}

bool C3DEngine::SaveCGF(std::vector<IStatObj *>& pObjs)
{
  TArray<CHUNK_HEADER> Chunks;
  TArray<NODE_CHUNK_DESC> Nodes;
  TArray<IShader *> Shaders;
  TArray<SNodeInfo> NI;
  int ExpFrame = 0;
  char fName[1024];
  int res;
  int i, j;
  int MatChunk;

  if (pObjs.empty())
    return false;
  IStatObj *io = pObjs.front();
  CStatObj *so = (CStatObj *)io;

  // Remove extension
  char *in = so->m_szFileName;
  char *o = fName;
  char c;
  
  while (*in)
  {
    if (*in=='.')
    {
      c = in[1];
      if (c!='.' && c!='/' && c!='\\')
        break;
    }
    *o++ = *in++;
  }
  *o = 0;

  // Add default extension
  strcat(fName, ".cgf");
  FILE *out = fxopen(fName, "wb");
  if (!out)
    return false;

  FILE_HEADER hd;
  strcpy(hd.Signature, FILE_SIGNATURE);
  hd.Version = GeomFileVersion;
  hd.FileType = FileType_Geom;
  
  //write the header
  res = fwrite(&hd, sizeof(hd), 1, out);
  if(res != 1)
    return false;

  std::vector<IStatObj*>::iterator n;
  for (n=pObjs.begin();n!=pObjs.end();n++)
  {
    IStatObj *io = (*n);
    CStatObj *co = (CStatObj *)io;
    CLeafBuffer *lb = co->GetLeafBuffer();
  
    // Prepare shaders for write
    for (i=0; i<lb->m_pMats->Count(); i++)
    {
      IShader *e = (*lb->m_pMats)[i].shaderItem.m_pShader;
      if (!e)
        continue;
      for (j=0; j<Shaders.Num(); j++)
      {
        if (!stricmp(e->GetName(), Shaders[j]->GetName()))
          break;
      }
      if (j == Shaders.Num())
        Shaders.AddElem(e);
    }
  }
  
  WriteMaterials(Chunks, Shaders, out, MatChunk);
  WriteNodes(Chunks, Nodes, out, NI, MatChunk, ExpFrame, pObjs);
  WriteMesh(Chunks, Nodes, Shaders, out, NI, MatChunk, ExpFrame);
  Nodes.Free();
  WriteLights(Chunks, Nodes, out, pObjs);

  hd.ChunkTableOffset = ftell(out);
  int nn = Chunks.Num();
  fwrite(&nn, sizeof(int), 1, out);
  for (i=0; i<Chunks.Num(); i++)
  {
    fwrite(&Chunks[i], sizeof(CHUNK_HEADER), 1, out);
  }
  fseek(out, 0, SEEK_SET);
  fwrite(&hd, sizeof(hd), 1, out);
  
  fclose (out);
  
  return true;
}

//============================================================================================
