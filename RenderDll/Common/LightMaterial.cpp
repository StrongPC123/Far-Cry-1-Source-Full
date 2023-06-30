/*=============================================================================
	LightMaterial.cpp : implementation of LightMaterial interface.
	Copyright (c) 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#define LIGHTMATERIAL_CPP

#include "RenderPCH.h"
#include "CommonRender.h"


		   
SLightMaterial* SLightMaterial::current_material = 0;
int SLightMaterial::m_ObjFrame = 0;
TArray<SLightMaterial *> SLightMaterial::known_materials;

void SLightMaterial::Release()
{
  m_nRefCounter--;
  if (!m_nRefCounter)
  {
    SLightMaterial::known_materials[Id] = NULL;
    delete this;
  }
}

void SLightMaterial::mfApply(int Flags)
{
  if (current_material == this && gRenDev->m_RP.m_FrameObject == m_ObjFrame && gRenDev->m_RP.m_CurrentVLightFlags == Flags)
    return;

  gRenDev->m_RP.m_CurrentVLights = 0;
  gRenDev->m_RP.m_CurrentVLightFlags = Flags;

  //if (!CRenderer::CV_r_hwlights)
  //  return;

  if (Flags & LMF_DISABLE)
    return;

  m_ObjFrame = gRenDev->m_RP.m_FrameObject;
  current_material = this;

  if (Flags & LMF_BUMPMATERIAL)
    gRenDev->m_RP.m_pCurLightMaterial = this;
  else
    gRenDev->EF_LightMaterial(this, Flags);
}

SLightMaterial *SLightMaterial::mfAdd(char *name, SLightMaterial *Compare)
{
  int i;
  SLightMaterial* mat;

  if (Compare)
  {
    for (i=0; i<known_materials.Num(); i++)
    {
      mat = known_materials[i];
      if (!mat)
        continue;
      if (mat->Front == Compare->Front)
      {
        mat->m_nRefCounter++;
        return mat;
      }
    }
    char name[128];
    int n = 0;
    int nFirstUse = -1;
    while (true)
    {
      sprintf(name, "$Auto_%d", n);
      for (i=0; i<known_materials.Num(); i++)
      {
        mat = known_materials[i];
        if (!mat)
        {
          if (nFirstUse < 0)
            nFirstUse = i;
          continue;
        }
        if (!strcmp(mat->name, name))
          break;
      }
      if (i == known_materials.Num())
      {
        SLightMaterial *mt = new SLightMaterial;
        mt->m_nRefCounter = 1;
        if (nFirstUse >= 0)
          i = nFirstUse;
        else
          known_materials.AddIndex(1);
        mt->Id = i;
        known_materials[i] = mt;
        strcpy (mt->name, name);
        mt->Front = Compare->Front;
        mt->side = FRONT;
        return known_materials[i];
      }
      n++;
    }
  }
  
  if (!name || !name[0])
    iConsole->Exit("SLightMaterial::mfAdd: NULL name\n");

//
// search the currently loaded materials
//
  for (i=0; i<known_materials.Num(); i++)
  {
    mat = known_materials[i];

    if (!stricmp (mat->name, name) )
    {
      //ShPrintf(MSG_WARNING, "LightMaterial '%s' is duplicated\n", name);
      mat->m_nRefCounter++;
      return mat;
    }
  }
  SLightMaterial *mt = new SLightMaterial;
  mt->m_nRefCounter = 1;
  known_materials.AddIndex(1);
  strcpy (mt->name, name);
  mt->Id = i;
  known_materials[i] = mt;

  return known_materials[i];
}

SLightMaterial *SLightMaterial::mfGet(char *name)
{
  int i;
  SLightMaterial* mat;

  if (!name || !name[0])
    iConsole->Exit ("SLightMaterial::mfGet: NULL name\n");

//
// search the currently loaded materials
//
  for (i=0; i<known_materials.Num(); i++)
  {
    mat = known_materials[i];

    if (!stricmp (mat->name, name) )
    {
      return mat;
    }
  }

  Warning( 0,0,"Couldn't find LightMaterial '%s' (use <Default>\n", name);
  if (known_materials.Num())
    return known_materials[0];

  return NULL;
}


