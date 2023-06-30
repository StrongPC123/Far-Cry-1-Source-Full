////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushmtl.cpp
//  Version:     v1.00
//  Created:     2/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BrushMtl.h"

#define BASE_SHADER_NAME "$Editor"
#define DEFAULT_SHADER "TemplDecal"
#define DEFAULT_TEXTURE "Checker.tga"

//////////////////////////////////////////////////////////////////////////
CBrushMtl::CBrushMtl()
{
	m_shaderItem.m_pShader = 0;
  m_shaderItem.m_pShaderResources = 0;

	// Default shader.
	m_shaderName = DEFAULT_SHADER;
	m_sr.m_Textures[EFTT_DIFFUSE].m_Name = DEFAULT_TEXTURE;
}

//////////////////////////////////////////////////////////////////////////
void CBrushMtl::ReloadShader()
{
	m_shaderItem = GetIEditor()->GetRenderer()->EF_LoadShaderItem( BASE_SHADER_NAME,eSH_Misc,true,m_shaderName,0,&m_sr );
}