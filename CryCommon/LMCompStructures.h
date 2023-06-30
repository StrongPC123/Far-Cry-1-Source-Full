#ifndef __LM_COMP_STRUCTURES_H__
#define __LM_COMP_STRUCTURES_H__

#pragma once 

#include "IRenderer.h"
#include <IEntitySystem.h>
#include <vector>
#include <string>

#define USE_DOT3_ALPHA 

#ifdef USE_DOT3_ALPHA 
	#define APPLY_COLOUR_FIX
#endif

#ifdef USE_DOT3_ALPHA
//	#define USE_ALPHA_FOR_LOWSPEC
#endif

//lightmap generation modes
typedef enum ELMMode
{
	ELMMode_ALL = 0,
	ELMMode_CHANGES,
	ELMMode_SEL
}ELMMode;

namespace NSAVE_RESULT
{
	static const unsigned int ESUCCESS = 0;
	static const unsigned int EPAK_FILE_UPDATE_FAIL = 1;
	static const unsigned int EDXT_COMPRESS_FAIL = 2;
	static const unsigned int EPAK_FILE_OPEN_FAIL = 3;
};

//! \brief Contains a Color + lerp factor / Dominant direction texture pair
struct RenderLMData : public _reference_target_t
{
	//! \brief Requires renderer for texture resource release
	RenderLMData(struct IRenderer *pIRenderer, int iColorLerpTex, int iHDRColorLerpTex, int iDomDirectionTex, int iOcclTex = 0)
	{
		m_pIRenderer		= pIRenderer;
		m_iColorLerpTex		= iColorLerpTex;
		m_iHDRColorLerpTex = iHDRColorLerpTex;
		m_iDomDirectionTex	= iDomDirectionTex;
		m_iOcclTex			= iOcclTex;
	};

	//! \brief Obtain textures
	int GetColorLerpTex()		{ return m_iColorLerpTex; };
	int GetHDRColorLerpTex()		{ return m_iHDRColorLerpTex; };
	int GetDomDirectionTex()	{ return m_iDomDirectionTex; };
	int GetOcclTex()			{ return m_iOcclTex; };

protected:
	//! \brief Destrucktor protected, call Release()
	~RenderLMData() 
	{ 
		m_pIRenderer->RemoveTexture(m_iColorLerpTex);
		m_pIRenderer->RemoveTexture(m_iDomDirectionTex);
		if(m_iOcclTex != 0)
			m_pIRenderer->RemoveTexture(m_iOcclTex);
		if(m_iHDRColorLerpTex != 0)
			m_pIRenderer->RemoveTexture(m_iHDRColorLerpTex);
	};

	struct IRenderer *m_pIRenderer; //!< Needed to correctly release texture resources
	int m_iColorLerpTex;            //!< Color + lerp factor texture
	int m_iHDRColorLerpTex;         //!< HDR Color in RGBE representation
	int m_iDomDirectionTex;         //!< Dominant direction texture
	int m_iOcclTex;					//!< occlusion map texture
};

TYPEDEF_AUTOPTR(RenderLMData);

struct LMGenParam
{
	//! \brief Initializes fields with useable defaults
	LMGenParam()
	{
		m_iTextureResolution = 256;
		m_fTexelSize = 0.25f;
		m_bDebugBorders = false;
		m_iSubSampling = 9;
		m_bGenOcclMaps = false;
		m_iMinBlockSize = 4;
		m_uiSmoothingAngle = 45;
		m_bComputeShadows = true;
		m_bUseSunLight = false;
		m_bDontMergePolys = false;
		m_bSpotAsPointlight = true;
		m_bOnlyExportLights = false;
    m_bHDR = false;
	};
 
	UINT m_iTextureResolution;		//!< Resolution of the produced textures
	float m_fTexelSize;				//!< World space size of one texel
	UINT m_iSubSampling;			//!< Amount of sub-sampling used for shadows etc.
	UINT m_iMinBlockSize;			//!< Smallest possible block assigned on a lightmap
	UINT m_uiSmoothingAngle;		//!< smoothing angle where edge sharing triangles be treated as smooth even when the normal is not smoothed and normal generation gets smoothed to
	bool m_bDebugBorders;			//!< Add colored borders to the generated lightmaps
	bool m_bGenOcclMaps;			//!< indicates whether to generate occlusion maps or not
	bool m_bComputeShadows;			//!< Set to true to enable shadow casting
	bool m_bUseSunLight;			//!< Specifies if lighting coming from the sun should be taken into account
	bool m_bDontMergePolys;			//!<
	bool m_bSpotAsPointlight;		//!< overrides spotlight to be used as omnilight source
	bool m_bOnlyExportLights;		//!< Export only static lights sources, does not recompile lightmaps.
  bool m_bHDR;                //!< Generate HDR light map data in RGBE representation
};

struct TexCoord2Comp
{
	TexCoord2Comp(float _s, float _t) { s = _s; t = _t; };
	TexCoord2Comp() : s(0.f), t(0.f){};
	float s;
	float t;
};

//! \brief Callback interface for compiler output
struct ICompilerProgress
{
	virtual void Output(const char *pszText) = 0;
};

struct LMStatLightFileHeader
{
	LMStatLightFileHeader()
	{
		iVersion = 0;
		iSizeOfDLight = sizeof(CDLight);
		iNumDLights = 0;  
	};

	uint8 iVersion;
	UINT iSizeOfDLight;
	UINT iNumDLights;
};

typedef struct SSharedLMEditorData
{
	HWND			hwnd;				//!< window handle to set the progress 	
	bool			bCancelled;			//!< if cancel has been pressed, change that to true
	unsigned char	ucProgress;			//!< progress in percent (corresponds to processed GLMs) 
	unsigned int	uiProgressMessage;	//!< message to send to progress bar
	unsigned int	uiMemUsageMessage;	//!< message to send to memory usage bar
	unsigned int	uiMemUsageStatic;	//!< message to send to text for memory usage bar static element
	unsigned int	uiGLMNameEdit;		//!< message to send to display the current glm name

	SSharedLMEditorData() : ucProgress(0), uiMemUsageMessage(0), bCancelled(false), hwnd(0), uiMemUsageStatic(0), uiGLMNameEdit(0)
	{}
}SSharedLMEditorData;

//!< colour corresponding to an stored occlusion index
typedef enum EOCCLCOLOURASSIGNMENT
{
	EOCCLCOLOURASSIGNMENT_NONE = -1,
	EOCCLCOLOURASSIGNMENT_RED = 0,
	EOCCLCOLOURASSIGNMENT_GREEN,
	EOCCLCOLOURASSIGNMENT_BLUE,
	EOCCLCOLOURASSIGNMENT_ALPHA,
}EOCCLCOLOURASSIGNMENT;

typedef struct SOcclusionMapTexel
{
	uint16 colour;
	SOcclusionMapTexel() : colour(0){}
	const SOcclusionMapTexel& operator =(const SOcclusionMapTexel& rTexel){colour = rTexel.colour; return *this;}
	const bool operator ==(const SOcclusionMapTexel& rTexel)const{return rTexel.colour == colour;}
	const bool operator !=(const SOcclusionMapTexel& rTexel)const{return rTexel.colour != colour;}
	const uint8 operator [](const EOCCLCOLOURASSIGNMENT eChannel) const
	{
		switch(eChannel)
		{
		case EOCCLCOLOURASSIGNMENT_NONE:
			assert(true);
			return 0;
			break;
		case EOCCLCOLOURASSIGNMENT_BLUE:
			return (uint8)(colour & 0x000F);
			break;
		case EOCCLCOLOURASSIGNMENT_GREEN:
			return (uint8)((colour >> 4)& 0x000F);
			break;
		case EOCCLCOLOURASSIGNMENT_RED:
			return (uint8)((colour >> 8)& 0x000F);
			break;
		case EOCCLCOLOURASSIGNMENT_ALPHA:
			return (uint8)((colour >> 12)& 0x000F);
			break;
		}
		return 0;
	}
	const SOcclusionMapTexel& SetValue(const EOCCLCOLOURASSIGNMENT eChannel, const uint8 cValue) 
	{
		assert(cValue < 0x10);
		switch(eChannel)
		{
		case EOCCLCOLOURASSIGNMENT_NONE:
			assert(true);
			break;
		case EOCCLCOLOURASSIGNMENT_BLUE:
			colour &= 0xFFF0;
			colour |= cValue;
			break;
		case EOCCLCOLOURASSIGNMENT_GREEN:
			colour &= 0xFF0F;
			colour |= (((uint32)cValue) << 4);
			break;
		case EOCCLCOLOURASSIGNMENT_RED:
			colour &= 0xF0FF;
			colour |= (((uint32)cValue) << 8);
			break;
		case EOCCLCOLOURASSIGNMENT_ALPHA:
			colour &= 0x0FFF;
			colour |= (((uint32)cValue) << 12);
			break;
		}
		return *this;
	}
}SOcclusionMapTexel;

//!< colour channel info for GLM
typedef struct GLMOcclLightInfo
{
	EOCCLCOLOURASSIGNMENT iChannelAssignment[4];					//!< channels assigned, -1 if not used, usually assigned in ascending order 0..3 
	unsigned int uiLightCount;										//!< number of active lights
	std::pair<EntityId, EntityId>  iLightIDs[4];
	GLMOcclLightInfo() : uiLightCount(0)
	{
		iChannelAssignment[0] = iChannelAssignment[1] = iChannelAssignment[2] = iChannelAssignment[3] = EOCCLCOLOURASSIGNMENT_NONE;
		iLightIDs[0] = iLightIDs[1] = iLightIDs[2] = iLightIDs[3] = std::pair<EntityId, EntityId>(0,0);
	}
	const unsigned int UsedChannelCount()const{return uiLightCount;}
	const EOCCLCOLOURASSIGNMENT FindLightSource(const std::pair<EntityId, EntityId>& ciID)
	{
		EOCCLCOLOURASSIGNMENT ret = EOCCLCOLOURASSIGNMENT_NONE;
		int i = 0;
		while(iChannelAssignment[i] != EOCCLCOLOURASSIGNMENT_NONE && i<4)
		{
			if(iLightIDs[i] == ciID)
				return iChannelAssignment[i];
			i++;
		}
		return ret;
	}
	const EOCCLCOLOURASSIGNMENT AddLightsource(const std::pair<EntityId, EntityId>& cID)
	{
		if(uiLightCount >= 4)
			return EOCCLCOLOURASSIGNMENT_NONE;//already 4 light sources used
		EOCCLCOLOURASSIGNMENT eChannel = FindLightSource(cID);
		if(eChannel != EOCCLCOLOURASSIGNMENT_NONE)
			return eChannel;//already contained
		//else insert
		switch(uiLightCount)
		{
		case 0:
			iChannelAssignment[0] = EOCCLCOLOURASSIGNMENT_RED;
			break;
		case 1:
			iChannelAssignment[1] = EOCCLCOLOURASSIGNMENT_GREEN;
			break;
		case 2:
			iChannelAssignment[2] = EOCCLCOLOURASSIGNMENT_BLUE;
			break;
		case 3:
			iChannelAssignment[3] = EOCCLCOLOURASSIGNMENT_ALPHA;
			break;
		}
		iLightIDs[uiLightCount] = cID;
		uiLightCount++;
		return iChannelAssignment[uiLightCount-1];
	}
}GLMOcclLightInfo;

//! \brief Name of the lightmap data file in the level's directory
#define LM_EXPORT_FILE_NAME "Dot3LM.dat"

//! \brief
#define LM_STAT_LIGHTS_EXPORT_FILE_NAME "StatLights.dat"

#endif // __LM_COMP_STRUCTURES_H__