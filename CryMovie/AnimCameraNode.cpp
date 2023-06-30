////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   animcameranode.cpp
//  Version:     v1.00
//  Created:     16/8/2002 by Lennert.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "animcameranode.h"

//////////////////////////////////////////////////////////////////////////
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
};

CAnimCameraNode::CAnimCameraNode( IMovieSystem *sys )
: CAnimEntityNode(sys)
{
	m_dwSupportedTracks = PARAM_BIT(APARAM_POS)|PARAM_BIT(APARAM_ROT)|
												PARAM_BIT(APARAM_EVENT)|PARAM_BIT(APARAM_FOV);
	m_pMovie=sys;
	m_fFOV = 60.0f;

	if (!s_nodeParamsInitialized)
	{
		s_nodeParamsInitialized = true;
		AddSupportedParam( "Position",APARAM_POS,AVALUE_VECTOR );
		AddSupportedParam( "Rotation",APARAM_ROT,AVALUE_QUAT );
		AddSupportedParam( "Fov",APARAM_FOV,AVALUE_FLOAT );
		AddSupportedParam( "Event",APARAM_EVENT,AVALUE_EVENT );
	}
}

//////////////////////////////////////////////////////////////////////////
CAnimCameraNode::~CAnimCameraNode()
{
}

//////////////////////////////////////////////////////////////////////////
int CAnimCameraNode::GetParamCount() const
{
	return s_nodeParams.size();
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCameraNode::GetParamInfo( int nIndex, SParamInfo &info ) const
{
	if (nIndex >= 0 && nIndex < s_nodeParams.size())
	{
		info = s_nodeParams[nIndex];
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCameraNode::GetParamInfoFromId( int paramId, SParamInfo &info ) const
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
void CAnimCameraNode::CreateDefaultTracks()
{
	CreateTrack(APARAM_POS);
	CreateTrack(APARAM_ROT);
	CreateTrack(APARAM_FOV);
};

//////////////////////////////////////////////////////////////////////////
void CAnimCameraNode::Animate( SAnimContext &ec )
{
	CAnimEntityNode::Animate(ec);
	IAnimBlock *anim = GetAnimBlock();
	if (!anim)
		return;
	IAnimTrack *pFOVTrack = anim->GetTrack(APARAM_FOV);
	
	float fov = m_fFOV;
	
	// is this camera active ? if so, set the current fov
	if (m_pMovie->GetCameraParams().cameraNode == this)
	{
		if (pFOVTrack)
			pFOVTrack->GetValue(ec.time, fov);

		SCameraParams CamParams = m_pMovie->GetCameraParams();
		CamParams.fFOV = DEG2RAD(fov);
		m_pMovie->SetCameraParams(CamParams);
	}

	if (fov != m_fFOV)
	{
		m_fFOV = fov;
		if (m_callback)
		{
			m_callback->OnNodeAnimated();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CAnimCameraNode::Reset()
{
	CAnimEntityNode::Reset();
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCameraNode::SetParamValue( float time,AnimParamType param,float value )
{
	if (param == APARAM_FOV)
	{
		// Set default value.
		m_fFOV = value;
	}
	return CAnimEntityNode::SetParamValue( time,param,value );
}

//////////////////////////////////////////////////////////////////////////
bool CAnimCameraNode::GetParamValue( float time,AnimParamType param,float &value )
{
	if (CAnimEntityNode::GetParamValue(time,param,value))
		return true;
	value = m_fFOV;
	return true;
}

//////////////////////////////////////////////////////////////////////////
IAnimTrack* CAnimCameraNode::CreateTrack(AnimParamType nParamType)
{
	IAnimTrack *pTrack = CAnimEntityNode::CreateTrack(nParamType);
	if (nParamType == APARAM_FOV)
	{
		if (pTrack)
			pTrack->SetValue(0,m_fFOV,true);
	}
	return pTrack;
}

//////////////////////////////////////////////////////////////////////////
void CAnimCameraNode::Serialize( XmlNodeRef &xmlNode,bool bLoading )
{
	CAnimEntityNode::Serialize( xmlNode,bLoading );
	if (bLoading)
	{
		xmlNode->getAttr( "FOV",m_fFOV );
	}
	else
	{
		xmlNode->setAttr( "FOV",m_fFOV );
	}
}