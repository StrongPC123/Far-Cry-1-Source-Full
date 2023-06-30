#ifndef _MAKEMATINFOFROMMAT_ENTITY_H_
#define _MAKEMATINFOFROMMAT_ENTITY_H_

// material loading code is shared between resource compiler and 3dengine

inline int SetTexType(TextureMap3 *tm)
{
  if (tm->type == TEXMAP_CUBIC)
    return eTT_Cubemap;
  else
    if (tm->type == TEXMAP_AUTOCUBIC)
      return eTT_AutoCubemap;
  return eTT_Base;
}

// szFileName - [May be NULL] the file name (without the path) of the file containing the MAT_ENTITY chunk
// szFolderName - the folder path where the file containing the MAT_ENTITY chunk resides; with the trailing shash.
//                This is used for searching textures.
inline bool CIndexedMesh__LoadMaterial(const char *szFileName, const char *szFolderName, 
                                CMatInfo & newMat, IRenderer * pRenderer, MAT_ENTITY * me)
{
  SInputShaderResources Res;
  memset(&Res, 0, sizeof(Res));
  SLightMaterial LMs;

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
  //  CMatInfo newMat & = MatInfo;
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

  Res.m_Textures[EFTT_DIFFUSE].m_TexModificator.m_Tiling[0] = me->map_d.uscl_val;
  Res.m_Textures[EFTT_DIFFUSE].m_TexModificator.m_Tiling[1] = me->map_d.vscl_val;
  Res.m_Textures[EFTT_DIFFUSE].m_TexModificator.m_Offs[0] = me->map_d.uoff_val;
  Res.m_Textures[EFTT_DIFFUSE].m_TexModificator.m_Offs[1] = me->map_d.voff_val;
  Res.m_Textures[EFTT_DIFFUSE].m_TexModificator.m_Rot[0] = Degr2Word(me->map_d.urot_val);
  Res.m_Textures[EFTT_DIFFUSE].m_TexModificator.m_Rot[1] = Degr2Word(me->map_d.vrot_val);
  Res.m_Textures[EFTT_DIFFUSE].m_bUTile = me->map_d.utile;
  Res.m_Textures[EFTT_DIFFUSE].m_bVTile = me->map_d.vtile;

  Res.m_Textures[EFTT_BUMP].m_TexModificator.m_Tiling[0] = me->map_b.uscl_val;
  Res.m_Textures[EFTT_BUMP].m_TexModificator.m_Tiling[1] = me->map_b.vscl_val;
  Res.m_Textures[EFTT_BUMP].m_TexModificator.m_Offs[0] = me->map_b.uoff_val;
  Res.m_Textures[EFTT_BUMP].m_TexModificator.m_Offs[1] = me->map_b.voff_val;
  Res.m_Textures[EFTT_BUMP].m_TexModificator.m_Rot[0] = Degr2Word(me->map_b.urot_val);
  Res.m_Textures[EFTT_BUMP].m_TexModificator.m_Rot[1] = Degr2Word(me->map_b.vrot_val);
  Res.m_Textures[EFTT_BUMP].m_bUTile = me->map_b.utile;
  Res.m_Textures[EFTT_BUMP].m_bVTile = me->map_b.vtile;

  Res.m_Textures[EFTT_SPECULAR].m_TexModificator.m_Tiling[0] = me->map_s.uscl_val;
  Res.m_Textures[EFTT_SPECULAR].m_TexModificator.m_Tiling[1] = me->map_s.vscl_val;
  Res.m_Textures[EFTT_SPECULAR].m_TexModificator.m_Offs[0] = me->map_s.uoff_val;
  Res.m_Textures[EFTT_SPECULAR].m_TexModificator.m_Offs[1] = me->map_s.voff_val;
  Res.m_Textures[EFTT_SPECULAR].m_TexModificator.m_Rot[0] = Degr2Word(me->map_s.urot_val);
  Res.m_Textures[EFTT_SPECULAR].m_TexModificator.m_Rot[1] = Degr2Word(me->map_s.vrot_val);
  Res.m_Textures[EFTT_SPECULAR].m_bUTile = me->map_b.utile;
  Res.m_Textures[EFTT_SPECULAR].m_bVTile = me->map_b.vtile;

  Res.m_Textures[EFTT_GLOSS].m_TexModificator.m_Tiling[0] = me->map_g.uscl_val;
  Res.m_Textures[EFTT_GLOSS].m_TexModificator.m_Tiling[1] = me->map_g.vscl_val;
  Res.m_Textures[EFTT_GLOSS].m_TexModificator.m_Offs[0] = me->map_g.uoff_val;
  Res.m_Textures[EFTT_GLOSS].m_TexModificator.m_Offs[1] = me->map_g.voff_val;
  Res.m_Textures[EFTT_GLOSS].m_TexModificator.m_Rot[0] = Degr2Word(me->map_g.urot_val);
  Res.m_Textures[EFTT_GLOSS].m_TexModificator.m_Rot[1] = Degr2Word(me->map_g.vrot_val);
  Res.m_Textures[EFTT_GLOSS].m_bUTile = me->map_g.utile;
  Res.m_Textures[EFTT_GLOSS].m_bVTile = me->map_g.vtile;

  if (!Res.m_Textures[EFTT_DETAIL_OVERLAY].m_Name.empty())
  {
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexModificator.m_Tiling[0] = me->map_detail.uscl_val;
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexModificator.m_Tiling[1] = me->map_detail.vscl_val;
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexModificator.m_Offs[0] = me->map_detail.uoff_val;
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexModificator.m_Offs[1] = me->map_detail.voff_val;
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexModificator.m_Rot[0] = Degr2Word(me->map_detail.urot_val);
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_TexModificator.m_Rot[1] = Degr2Word(me->map_detail.vrot_val);
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_bUTile = me->map_detail.utile;
    Res.m_Textures[EFTT_DETAIL_OVERLAY].m_bVTile = me->map_detail.vtile;
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
  else if(str=strchr(mName, '('))
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

// newMat.SetName(mName);		// set material name
	strncpy( newMat.sMaterialName, mName, sizeof(newMat.sMaterialName) );
	newMat.sMaterialName[sizeof(newMat.sMaterialName)-1] = 0;

  // load shader
  if(mName[0]==0)
    strcpy(mName,"nodraw");

	if(!templName || !templName[0])
		templName = "nodraw";

  if(pRenderer) // in compiler there is no renderer
    newMat.shaderItem = pRenderer->EF_LoadShaderItem(mName, eSH_World, true, templName, 0, &Res);

  //  newMat.shaderItem->m_pShader->AdjustResources(newMat.shaderItem->m_pShaderResources);
  //  IShader * ef = newMat.shaderItem.m_pShader->GetTemplate(-1);
  //  if(ef->GetPhysMaterialFlags() & MATF_NOCLIP)
  //  newMat.m_Flags &= ~MIF_PHYSIC;
  //	strcpy(newMat.szFolderName, szFolderName);

  // remember polybump state
  newMat.m_Flags &= ~MIF_POLYBUMP;

  newMat.fAlpha = me->opacity;

  if(!pRenderer)
  { // in compiler remember also source MAT_ENTITY
    assert(newMat.pMatEnt != me);
    newMat.pMatEnt = new MAT_ENTITY;
    *newMat.pMatEnt = *me;
  }

  return true;
}

#endif // _MAKEMATINFOFROMMAT_ENTITY_H_
