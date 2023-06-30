////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   MaterialNode.cpp
//  Version:     v1.00
//  Created:     11/5/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MaterialNode.h"
#include "AnimTrack.h"

#include <ISystem.h>
#include <I3DEngine.h>
#include <IRenderer.h>
#include <IShader.h>

namespace
{
	bool s_nodeParamsInitialized = false;
	std::vector<IAnimNode::SParamInfo> s_nodeParams;

	void AddSupportedParam( const char *sName,int paramId,EAnimValue valueType )
	{
		IAnimNode::SParamInfo param;
		param.name = sName;
		param.paramId = paramId;
		param.valueType = valueType;
		s_nodeParams.push_back( param );
	}
}

enum EMaterialNodeParam
{
	MTL_PARAM_OPACITY    = APARAM_USER + 1,
	MTL_PARAM_AMBIENT    = APARAM_USER + 2,
	MTL_PARAM_DIFFUSE    = APARAM_USER + 3,
	MTL_PARAM_SPECULAR   = APARAM_USER + 4,
	MTL_PARAM_EMMISION   = APARAM_USER + 5,
	MTL_PARAM_SHININESS  = APARAM_USER + 6,
	
	MTL_PARAM_SHADER_PARAM1  = APARAM_USER + 100,
};

//////////////////////////////////////////////////////////////////////////
CAnimMaterialNode::CAnimMaterialNode( IMovieSystem *sys )
: CAnimNode(sys)
{
	SetFlags( GetFlags()|ANODE_FLAG_CAN_CHANGE_NAME );
	m_dwSupportedTracks = PARAM_BIT(APARAM_FLOAT_1);

	//////////////////////////////////////////////////////////////////////////
	// One time initialization of material node supported params.
	if (!s_nodeParamsInitialized)
	{
		s_nodeParamsInitialized = true;
		
		AddSupportedParam( "Opacity",MTL_PARAM_OPACITY,AVALUE_FLOAT );
		AddSupportedParam( "Ambient",MTL_PARAM_AMBIENT,AVALUE_VECTOR );
		AddSupportedParam( "Diffuse",MTL_PARAM_DIFFUSE,AVALUE_VECTOR );
		AddSupportedParam( "Specular",MTL_PARAM_SPECULAR,AVALUE_VECTOR );
		AddSupportedParam( "Emission",MTL_PARAM_EMMISION,AVALUE_VECTOR );
		AddSupportedParam( "Shininess",MTL_PARAM_SHININESS,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 1",MTL_PARAM_SHADER_PARAM1,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 2",MTL_PARAM_SHADER_PARAM1+1,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 3",MTL_PARAM_SHADER_PARAM1+2,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 4",MTL_PARAM_SHADER_PARAM1+3,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 5",MTL_PARAM_SHADER_PARAM1+4,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 6",MTL_PARAM_SHADER_PARAM1+5,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 7",MTL_PARAM_SHADER_PARAM1+6,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 8",MTL_PARAM_SHADER_PARAM1+7,AVALUE_FLOAT );
		AddSupportedParam( "Shader Param 9",MTL_PARAM_SHADER_PARAM1+8,AVALUE_FLOAT );
	}
}

//////////////////////////////////////////////////////////////////////////
int CAnimMaterialNode::GetParamCount() const
{
	return s_nodeParams.size();
}

//////////////////////////////////////////////////////////////////////////
bool CAnimMaterialNode::GetParamInfo( int nIndex, SParamInfo &info ) const
{
	if (nIndex >= 0 && nIndex < s_nodeParams.size())
	{
		info = s_nodeParams[nIndex];
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimMaterialNode::GetParamInfoFromId( int paramId, SParamInfo &info ) const	
{
	for (int i = 0; i < s_nodeParams.size(); i++)
	{
		if (s_nodeParams[i].paramId == paramId)
		{
			info = s_nodeParams[i];
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CAnimMaterialNode::Animate( SAnimContext &ec )
{
	IAnimBlock *anim = GetAnimBlock();
	if (!anim)
		return;

	int paramCount = anim->GetTrackCount();
	if (paramCount <= 0)
		return;

	// Find material.
	IMatInfo *pMtl = m_pMovieSystem->GetSystem()->GetI3DEngine()->FindMaterial( GetName() );
	if (!pMtl)
		return;

	SRenderShaderResources *pShaderResources = pMtl->GetShaderItem().m_pShaderResources;
	if (!pShaderResources)
		return;

	float fValue;
	Vec3 vValue;
	for (int paramIndex = 0; paramIndex < paramCount; paramIndex++)
	{
		int paramId;
		IAnimTrack *pTrack;
		if (!anim->GetTrackInfo( paramIndex,paramId,&pTrack ))
			continue;
		switch (paramId)
		{
		case MTL_PARAM_OPACITY:
			pTrack->GetValue( ec.time,fValue );
			pShaderResources->m_Opacity = fValue;
			break;
		case MTL_PARAM_AMBIENT:
			pTrack->GetValue( ec.time,vValue );
			if (pShaderResources->m_LMaterial)
				pShaderResources->m_LMaterial->Front.m_Ambient = CFColor(vValue.x,vValue.y,vValue.z);
			break;
		case MTL_PARAM_DIFFUSE:
			pTrack->GetValue( ec.time,vValue );
			if (pShaderResources->m_LMaterial)
				pShaderResources->m_LMaterial->Front.m_Diffuse = CFColor(vValue.x,vValue.y,vValue.z);
			break;
		case MTL_PARAM_SPECULAR:
			pTrack->GetValue( ec.time,vValue );
			if (pShaderResources->m_LMaterial)
				pShaderResources->m_LMaterial->Front.m_Specular = CFColor(vValue.x,vValue.y,vValue.z);
			break;
		case MTL_PARAM_EMMISION:
			pTrack->GetValue( ec.time,vValue );
			if (pShaderResources->m_LMaterial)
				pShaderResources->m_LMaterial->Front.m_Emission = CFColor(vValue.x,vValue.y,vValue.z);
			break;
		case MTL_PARAM_SHININESS:
			pTrack->GetValue( ec.time,fValue );
			if (pShaderResources->m_LMaterial)
				pShaderResources->m_LMaterial->Front.m_SpecShininess = fValue;
			break;
		default:
			if (paramId >= MTL_PARAM_SHADER_PARAM1)
			{
				int id = paramId - MTL_PARAM_SHADER_PARAM1;
				if (id < pShaderResources->m_ShaderParams.size())
				{
					pTrack->GetValue( ec.time,fValue );
					pShaderResources->m_ShaderParams[id].m_Value.m_Float = fValue;
				}
			}
		}
	}
}
