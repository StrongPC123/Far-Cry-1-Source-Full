#include "stdafx.h"
#ifdef _CRY_ANIMATION_BASE_HEADER_
#include "CVars.h"
#endif
#include "BoneLightBindInfo.h"

// initializes this from the structures found in the cgf file
void CBoneLightBindInfo::load (const SBoneLightBind& bind, const LIGHT_CHUNK_DESC& light, const char* szLightName, float fScale)
{
	// set up the flags that will be set to the DLight

	m_nDLightFlags = 0;

	// the light source must have a name with substrings _ls and/or _hs
	// _ls means a light source, _hs means a heat source
	// if none is present, then the heat source flag is automatically set up
	if (strstr(szLightName,"_ls"))
		m_nDLightFlags |= DLF_LIGHTSOURCE;
	if (strstr(szLightName, "_hs"))
		m_nDLightFlags |= DLF_HEATSOURCE;
	if (!(m_nDLightFlags & (DLF_HEATSOURCE|DLF_LIGHTSOURCE)))
		m_nDLightFlags |= DLF_HEATSOURCE;

	if (strstr(szLightName, "local"))
		m_nDLightFlags |= DLF_LOCAL;

	if (light.szLightImage[0])
		m_nDLightFlags |= DLF_PROJECT;
	else
		m_nDLightFlags |= DLF_POINT;


	m_nBone = bind.nBoneId;
	m_vPos  = bind.vLightOffset;

	m_qRot  = exp( quaternionf(0,bind.vRotLightOrientation) );

	constructMatLight();

	m_nType = light.type;
	
	m_rgbColor.r = light.color.r / 255.0f;
	m_rgbColor.g = light.color.g / 255.0f;
	m_rgbColor.b = light.color.b / 255.0f;
	m_rgbColor.a = 1.0f;

	m_fIntensity      = light.intens;
	m_fHotSpotSize    = light.hotsize;
	m_fFalloffSize    = light.fallsize;
	m_fNearAttenStart = light.nearAttenStart;
	m_fNearAttenEnd   = light.nearAttenEnd;
	m_fFarAttenStart  = light.attenStart;
	m_fFarAttenEnd    = light.attenEnd;
	m_vDirection      = light.vDirection;
	m_strLightImage   = light.szLightImage;

	m_bOn             = light.on;
	m_bUseNearAtten   = light.useNearAtten;
	m_bUseFarAtten    = light.useAtten;
	m_bShadow         = light.shadow;

	if (!m_bUseFarAtten)
	{
#ifdef _CRY_ANIMATION_BASE_HEADER_
		g_GetLog()->LogWarning ("\003no far attenuation in the heat source (bone) light, using default");
#endif
		m_fFarAttenEnd = 40;
	}
	scale (fScale);
}

void CBoneLightBindInfo::scale (float fScale)
{
	m_vPos *= fScale;
	m_matLight.SetTranslationOLD (m_matLight.GetTranslationOLD()*fScale);

	m_fNearAttenStart *= fScale;
	m_fNearAttenEnd   *= fScale;
	m_fFarAttenStart  *= fScale;
	m_fFarAttenEnd    *= fScale;

	if (!m_bUseFarAtten)
		m_fFarAttenEnd *= fScale;
}

void CBoneLightBindInfo::constructMatLight()
{
	m_matLight=Matrix44(m_qRot);
	m_matLight.SetTranslationOLD(m_vPos);
}

//////////////////////////////////////////////////////////////////////////
// initializes the given DLight structure out of the given bone instance
// this is one-time call that is only required after construction of the DLight
// to initialize its constant parameters
void CBoneLightBindInfo::initDLight (CDLight& rDLight)
{
	rDLight.m_Flags &= ~(DLF_LIGHTSOURCE|DLF_HEATSOURCE|DLF_PROJECT|DLF_POINT);
  rDLight.m_Flags |= m_nDLightFlags;
  rDLight.m_Flags |= DLF_LOCAL;
  rDLight.m_fDirectFactor = 0;
  rDLight.m_Color = m_rgbColor;
  rDLight.m_fRadius = m_fFarAttenEnd;
}


//////////////////////////////////////////////////////////////////////////
// per-frame update of the DLight structure. Updates the light position and radius
// PARAMETERS:
//   matParentBone - the partent coordinate frame
//   fRadiusFactor - the multiplier for the original radius of the light
//   rDLight       - the light to update
void CBoneLightBindInfo::updateDLight (const Matrix44& matParentBone, float fRadiusMultiplier, CDLight& DLight)
{
	DLight.m_Origin = matParentBone.TransformPointOLD(m_vPos);
	DLight.m_fRadius = m_fFarAttenEnd * fRadiusMultiplier;
}

// returns true if this light source is local (affects only the character)
bool CBoneLightBindInfo::isLocal()const
{
	return (m_nDLightFlags & DLF_LOCAL) != 0;
}

bool CBoneLightBindInfo::isHeatSource()const
{
	return (m_nDLightFlags & DLF_HEATSOURCE) != 0;
}

bool CBoneLightBindInfo::isLightSource()const
{
	return (m_nDLightFlags & DLF_LIGHTSOURCE) != 0;
}


// returns the priority to sort
int CBoneLightBindInfo::getPriority()const
{
	int nPriority = isLocal()?0:1;
	nPriority <<= 1;
	nPriority |= isHeatSource()?0:1;
	nPriority <<= 1;
	return nPriority;
}


//////////////////////////////////////////////////////////////////////////
// Serialization to/from memory buffer.
// if the buffer is NULL, and bSave is true, returns the required buffer size
// otherwise, tries to save/load from the buffer, returns the number of bytes written/read
// or 0 if the buffer is too small or some error occured
unsigned CBoneLightBindInfo::Serialize (bool bSave, void* pBuffer, unsigned nSize)
{
	const int nVersion = 1;

	typedef CBoneLightBindDesc Desc;
	if (bSave)
	{
		// calculate the required size
		unsigned nRequiredSize = unsigned(sizeof(nVersion) + sizeof(Desc) + m_strLightImage.length()+1);
		if (!pBuffer)																	
			return nRequiredSize;

		if (nSize < nRequiredSize)
			return 0;

		*(int*)pBuffer = 1;
		pBuffer = (int*)pBuffer+1;

		*((Desc*)pBuffer) = *(Desc*)this;
		pBuffer = ((Desc*)pBuffer)+1;

		memcpy (pBuffer, m_strLightImage.c_str(), m_strLightImage.length()+1);
		return nRequiredSize;
	}
	else
	{
		if (nSize < sizeof(nVersion) +sizeof(Desc))
			return 0;

		const char* pBufEnd = (const char*)pBuffer + nSize;

		int nRealVersion = *(int*)pBuffer;
		if (nRealVersion != nVersion)
			return 0;
		void* pRawData = (int*)pBuffer + 1;

		*(Desc*)this = *(const Desc*)pRawData;
		pRawData = ((Desc*)pRawData)+1;

		const char* pName = (const char*)pRawData;

		m_strLightImage = "";
		for(;*pName && pName < pBufEnd;++pName)
			m_strLightImage += *pName;

		constructMatLight();
		return (unsigned)(pName+1 - (const char*)pBuffer);
	}
}
