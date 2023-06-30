////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   brushmtl.h
//  Version:     v1.00
//  Created:     2/12/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __brushmtl_h__
#define __brushmtl_h__
#pragma once

/** Material used by brush.
*/
class CBrushMtl : public CRefCountBase
{
public:
	CBrushMtl();
	IShader* GetShader() const { return m_shaderItem.m_pShader; };
	SRenderShaderResources* GetShaderResources() const { return m_shaderItem.m_pShaderResources; };
	ITexPic* GetEditorTexture() const
	{
    if (m_shaderItem.m_pShaderResources->m_Textures[EFTT_DIFFUSE])
		  return m_shaderItem.m_pShaderResources->m_Textures[EFTT_DIFFUSE]->m_TU.m_ITexPic;
    return NULL;
	}

	//! Reload shader description inside renderer.
	void ReloadShader();

private:
	SShaderItem m_shaderItem;
	SInputShaderResources m_sr;

	CString m_shaderName;
};

// Typedef smart pointer to brush material.
typedef TSmartPtr<CBrushMtl> CBrushMtlPtr;

#endif // __brushmtl_h__
