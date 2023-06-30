////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   TerrainTexGen.cpp
//  Version:     v1.00
//  Created:     8/10/2002 by Timur.
//  Compilers:   Visual Studio.NET
//  Description: 
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "TerrainTexGen.h"

#include "CryEditDoc.h"
#include "TerrainLighting.h"
#include "Heightmap.h"
#include "TerrainGrid.h"
#include "Layer.h"
#include "VegetationMap.h"
#include "HeightmapAccessibility.h"					// CHeightmapAccessibility

#include "Util\Thread.h"

#include <I3Dengine.h>

#include <afxmt.h>

// Sector flags.
enum
{
	eSectorHeightmapValid  = 0x01,
	eSectorLightmapValid  = 0x02,
	eSectorLayersValid  = 0x04
};

#define MAX_BRIGHTNESS 100
//#define LIGHTBIT_INSHADOW_LEVEL 196
#define LIGHTBIT_INSHADOW_LEVEL (min( MAX_BRIGHTNESS,ftoi( MAX_BRIGHTNESS*fAmbient*1.25f*pSettings->sunMultiplier) ))

//////////////////////////////////////////////////////////////////////////
CTerrainTexGen::CTerrainTexGen( int resolution )
{
	m_pLightingBits = NULL;
	m_bLog = true;
	m_waterLayer = NULL;
	m_vegetationMap = NULL;
	m_iCachedSkyAccessiblityQuality=0;
	m_bNotValid = false;

	Init(resolution);
}

CTerrainTexGen::CTerrainTexGen()
{
	m_pLightingBits = NULL;
	m_bLog = true;
	m_waterLayer = NULL;
	m_vegetationMap = NULL;
	m_iCachedSkyAccessiblityQuality=0;
	m_bNotValid = false;

	m_heightmap = GetIEditor()->GetHeightmap();
	assert( m_heightmap );

	SSectorInfo si;
	m_heightmap->GetSectorsInfo( si );
	Init(si.surfaceTextureSize);
}

//////////////////////////////////////////////////////////////////////////
CTerrainTexGen::~CTerrainTexGen()
{
	// Release masks for all layers to save memory.
	int numLayers = GetLayerCount();
	for (int i = 0; i < numLayers; i++)
	{
		if (m_layers[i].layerMask)
			delete m_layers[i].layerMask;
		CLayer *pLayer = GetLayer(i);
		pLayer->ReleaseMask();
	}

	for (int i = 0; i < m_sectorGrid.size(); i++)
	{
		if (m_sectorGrid[i].lightmap)
			delete m_sectorGrid[i].lightmap;
	}

	if (m_waterLayer)
	{
		delete m_waterLayer;
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::Init( int resolution )
{
	int i;
	m_heightmap = GetIEditor()->GetHeightmap();
	assert( m_heightmap );

	m_terrainMaxZ = 255.0f;

	m_vegetationMap = GetIEditor()->GetVegetationMap();

	// Fill layers array.
	ClearLayers();
	
	CCryEditDoc *pDocument = GetIEditor()->GetDocument();
	int numLayers = pDocument->GetLayerCount();
	m_layers.reserve( numLayers + 2 ); // Leave some space for water layers.
	for (i = 0; i < numLayers; i++)
	{
		SLayerInfo li;
		li.pLayer = pDocument->GetLayer(i);
		m_layers.push_back(li);
	}

	SSectorInfo si;
	m_heightmap->GetSectorsInfo( si );

	m_resolution = resolution;
	m_numSectors = si.numSectors;
	m_sectorResolution = m_resolution / m_numSectors;

	m_pixelSizeInMeters = float(si.numSectors*si.sectorSize) / (float)m_resolution;

	//! Allocate heightmap big enough.
	if (m_hmap.GetWidth() != resolution)
	{
		if (!m_hmap.Allocate( resolution,resolution ))
			m_bNotValid = true;
		
		// Invalidate all sectors.
		m_sectorGrid.resize( m_numSectors*m_numSectors );
		memset( &m_sectorGrid[0],0,m_sectorGrid.size()*sizeof(m_sectorGrid[0]) );
	}

	// Resolution is always power of 2.
	m_resolutionShift = 1;
	for (i = 0; i<32 ;i++)
	{
		if ((1<<i)==m_resolution)
		{
			m_resolutionShift = i;
			break;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::SetLightingBits( CBitArray *array )
{
	m_pLightingBits = array;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::UpdateSectorLayers( CPoint sector )
{
	bool bFirstUsedLayer = true;

	CRect sectorRect;
	GetSectorRect( sector,sectorRect );

	int numLayers = GetLayerCount();
	for (int i = 0; i < numLayers; i++)
	{
		CLayer *pLayer = GetLayer(i);

		// Skip the layer if it is not in use
		if (!pLayer->IsInUse() || !pLayer->HasTexture())
			continue;

		// For first used layer mask is not needed.
		if (!bFirstUsedLayer)
		{
			if (!m_layers[i].layerMask)
			{
				m_layers[i].layerMask = new CByteImage;
			}

			if (pLayer->UpdateMaskForSector(sector,sectorRect,m_hmap,*m_layers[i].layerMask ))
			{
			}
		}
		bFirstUsedLayer = false;
	}
	// For this sector all layers are valid.
	SetSectorFlags( sector,eSectorLayersValid );
}

// written by M.M.
float CTerrainTexGen::GetSunAmount( const float *inpHeightmapData,int iniX,int iniY,
																		float infInvHeightScale, const Vec3 &vSunShadowVector, const float infShadowBlur ) const
{
	assert(inpHeightmapData);
	assert(iniX<m_resolution);
	assert(iniY<m_resolution);

	float fForwardStepSize=1.0f;											// [0.1f .. [ the less size, the better quality but it will be slower

//	fForwardStepSize *= (float)iniWidth/1024.0f;

	const int iFixPointBits=8;																// wide range .. more precision
	const int iFixPointBase=1<<iFixPointBits;									//

	float fZInit=inpHeightmapData[iniX+(iniY<<m_resolutionShift)];

	float fSlope=vSunShadowVector.z*infInvHeightScale*fForwardStepSize;

	float fSlopeTop = fSlope + infShadowBlur;
	float fSlopeBottom = fSlope - infShadowBlur;

	assert(fSlopeTop>=0.0f);

	fZInit+=0.1f;			// Bias to avoid little bumps in the result

	int iDirX=(int)(vSunShadowVector.x*fForwardStepSize*iFixPointBase);
	int iDirY=(int)(vSunShadowVector.y*fForwardStepSize*iFixPointBase);

//	float fLen=(float)(rand()) * (fForwardStepSize/RAND_MAX);
	float fLen=0.0f;

	int iX=iniX*iFixPointBase+iFixPointBase/2,iY=iniY*iFixPointBase+iFixPointBase/2;

	float fZBottom=fZInit+0.1f,fZTop=fZInit+1.4f;
	float fArea=1.0f;

	// inner loop
	for(;fZBottom<m_terrainMaxZ;)
	{
		assert(fZBottom<=fZTop);

		iX+=iDirX;iY+=iDirY;
		fZBottom+=fSlopeBottom;fZTop+=fSlopeTop;

		int iXBound=(iX>>iFixPointBits);											// shift right iFixPointBits bits = /iFixPointBase
		int iYBound=(iY>>iFixPointBits);											// shift right iFixPointBits bits = /iFixPointBase

		if(iXBound>=m_resolution)break;
		if(iYBound>=m_resolution)break;

		float fGround=inpHeightmapData[iXBound + (iYBound<<m_resolutionShift)];

		if(fZTop<fGround)					// ground hit
			return(0.0f);						// full shadow						

		if(fZBottom<fGround)			// ground hit
		{
			float fNewArea=(fZTop-fGround)/(fZTop-fZBottom);							// this is slow in the penumbra of the shadow (but this is a rare case)

			if(fNewArea<fArea)fArea=fNewArea;
			assert(fArea>=0.0f);
			assert(fArea<=1.0f);
		}
	}

	return(fArea);
}



float CTerrainTexGen::GetSkyAccessibilityFast( const int iniX, const int iniY ) const
{
	if(!m_SkyAccessiblity.GetData())
		return 1.0f;															// RefreshAccessibility was not sucessful

	assert(m_SkyAccessiblity.GetWidth()!=0);
	assert(m_SkyAccessiblity.GetHeight()!=0);

	int iW=m_hmap.GetWidth();
	int iH=m_hmap.GetHeight();

	int invScaleX=iW / m_SkyAccessiblity.GetWidth();
	int invScaleY=iH / m_SkyAccessiblity.GetHeight();

	int iXmul256=256/invScaleX;
	int iYmul256=256/invScaleY;

	return CImageUtil::GetBilinearFilteredAt(iniX*iXmul256,iniY*iYmul256,m_SkyAccessiblity)*(1.0f/255.0f);
}

float CTerrainTexGen::GetSunAccessibilityFast( const int iniX, const int iniY ) const
{
	if(!m_SunAccessiblity.GetData())
		return 1.0f;															// RefreshAccessibility was not sucessful
	
	assert(m_SunAccessiblity.GetWidth()!=0);
	assert(m_SunAccessiblity.GetHeight()!=0);

	int iW=m_hmap.GetWidth();
	int iH=m_hmap.GetHeight();

	int invScaleX=iW / m_SunAccessiblity.GetWidth();
	int invScaleY=iH / m_SunAccessiblity.GetHeight();

	int iXmul256=256/invScaleX;
	int iYmul256=256/invScaleY;

	return CImageUtil::GetBilinearFilteredAt(iniX*iXmul256,iniY*iYmul256,m_SunAccessiblity)*(1.0f/255.0f);
}

/*
// written by M.M.
float CTerrainTexGen::GetSkyAccessibilityNoise( const float *inpHeightmapData, 
	const int iniWidth, const int iniHeight, const int iniX, const int iniY, const int iniQuality, const float infHeightScale ) const
{
	assert(inpHeightmapData);
	assert(iniX<iniWidth);
	assert(iniY<iniHeight);

	int iAngleSteps;																// [4..[ the more steps, the less noise but it will be slower
	float fForwardStepSize;														// [0.1f .. [ the less size, the better quality but it will be slower

	switch(iniQuality)
	{
		case 1:  iAngleSteps=3; fForwardStepSize=8.0f;break;
		case 2:  iAngleSteps=4; fForwardStepSize=6.0f;break;
		case 3:  iAngleSteps=5; fForwardStepSize=5.0f;break;
		case 4:  iAngleSteps=6; fForwardStepSize=4.0f;break;
		case 5:  iAngleSteps=7; fForwardStepSize=3.5f;break;
		case 6:  iAngleSteps=8; fForwardStepSize=3.0f;break;
		case 7:  iAngleSteps=10; fForwardStepSize=2.0f;break;
		case 8:  iAngleSteps=12; fForwardStepSize=1.5f;break;
		case 9:  iAngleSteps=14; fForwardStepSize=1.0f;break;
		case 10: iAngleSteps=16; fForwardStepSize=0.7f;break;
		default: return(1.0f);			// no sampling at all
	}

//	fForwardStepSize *= (float)iniWidth/1024.0f;

	const int iFixPointBits=8;																// wide range .. more precision
	const int iFixPointBase=1<<iFixPointBits;									//

	float const fAngleStepsSize=gf_PI_MUL_2/(float)iAngleSteps;

	float fJitterAngle=(float)(rand()) * (fAngleStepsSize/RAND_MAX);


	float fZInit=inpHeightmapData[iniX+(iniY<<m_resolutionShift)];
	float fSlopeInit=0.0f;					// 

	assert(fSlopeInit>=0.0f);

	fZInit+=0.1f;			// Bias to avoid little bumps in the accessibility result

	float fArea=0.0f;


	//  To sample the whole hemisphere we divide it into wedges
	for(float iWedgeNo=fJitterAngle;iWedgeNo<iAngleSteps;iWedgeNo++)
	{
		float fAngle = iWedgeNo*fAngleStepsSize + fJitterAngle;

		int iDirX=(int)(sin(fAngle)*fForwardStepSize*iFixPointBase);
		int iDirY=(int)(cos(fAngle)*fForwardStepSize*iFixPointBase);

		float fSlope=fSlopeInit;								
		float fLen=(float)(rand()) * (fForwardStepSize/RAND_MAX);

		int iX=iniX*iFixPointBase+iFixPointBase/2,iY=iniY*iFixPointBase+iFixPointBase/2;

		// for every wedge a shadow horizon is searched

		// inner loop
		for(float fZ=fZInit;fZ<m_terrainMaxZ;fZ+=fSlope)
		{
			int iXBound=(iX>>iFixPointBits);											// shift right iFixPointBits bits = /iFixPointBase
			int iYBound=(iY>>iFixPointBits);											// shift right iFixPointBits bits = /iFixPointBase

			if(((DWORD)iXBound)>=iniWidth)break;
			if(((DWORD)iYBound)>=iniHeight)break;

			float fGround=inpHeightmapData[iXBound + (iYBound<<m_resolutionShift)];

			if(fZ<fGround)		// ground hit
			{
				assert(fLen!=0.0f);

				fSlope=(fGround-fZInit)/fLen;				assert(fSlope>=0.0f);
				fZ=fGround+0.1f;					// Bias to avoid little bumps in the accessibility result
			}

			fLen+=fForwardStepSize;
			iX+=iDirX;iY+=iDirY;
		}

		float fWedgeHorizonPart=1.0f-atanf(fSlope*infHeightScale)/gf_PI_DIV_2;
		float fWedgeArea = fWedgeHorizonPart*fWedgeHorizonPart/(float)iAngleSteps;

		fArea+=fWedgeArea;
	}

	return(fArea);
}
*/


////////////////////////////////////////////////////////////////////////
// Generate the surface texture with the current layer and lighting
// configuration and write the result to surfaceTexture.
// Also give out the results of the terrain lighting if pLightingBit is not NULL.
////////////////////////////////////////////////////////////////////////
bool CTerrainTexGen::GenerateSectorTexture( CPoint sector,const CRect &rect,int flags,CImage &surfaceTexture )
{
	if (m_bNotValid)
		return false;

	// set flags.
	bool bUseLighting = flags & ETTG_LIGHTING;
	bool bShowWater = flags & ETTG_SHOW_WATER;
  bool bConvertToABGR = flags & ETTG_ABGR;
	bool bNoTexture = flags & ETTG_NOTEXTURE;
	bool bUseLightmap = flags & ETTG_USE_LIGHTMAPS;
	//bool bKeepsLayers = flags & ETTG_KEEP_LAYERMASKS;
	m_bLog = !(flags & ETTG_QUIET);

	if (flags & ETTG_INVALIDATE_LAYERS)
	{
		InvalidateLayers();
	}

	uint i;

	CCryEditDoc *pDocument = GetIEditor()->GetDocument();
	CHeightmap *pHeightmap = GetIEditor()->GetHeightmap();
	SectorInfo& sectorInfo = GetSectorInfo(sector);
	int sectorFlags = sectorInfo.flags;

	assert( pDocument );
	assert( pHeightmap );

	float waterLevel = pHeightmap->GetWaterLevel();

	uint *pSurface = surfaceTexture.GetData();

	// Update heightmap for that sector.
	UpdateSectorHeightmap(sector);

	if (bNoTexture)
	{
		// fill texture with white color if there is no texture present
		surfaceTexture.Fill( 255 );
	}
	else
	{
		// Enable texturing.
		//////////////////////////////////////////////////////////////////////////
		// Setup water layer.
		if (bShowWater && m_waterLayer == NULL)
		{
			// Apply water level.
			// Add a temporary water layer to the list
			SLayerInfo li;
			m_waterLayer = new CLayer;
			m_waterLayer->LoadTexture(MAKEINTRESOURCE(IDB_WATER), 128, 128);
			m_waterLayer->SetAutoGen(true);
			m_waterLayer->SetLayerStart(0);
			m_waterLayer->SetLayerEnd(waterLevel);

			li.pLayer = m_waterLayer;
			m_layers.push_back( li );
			/*
			//m_waterLayer->FillWithColor( pDocument->GetWaterColor(),8,8 );
			m_waterLayer->GenerateWaterLayer16(m_hmap.GetData(),m_resolution, m_resolution, waterLevel );
			li.pLayer = m_waterLayer;
			li.layerMask.Attach( m_waterLayer->GetMask() );
			m_layers.push_back( li );
			*/
		}

		////////////////////////////////////////////////////////////////////////
		// Generate the layer masks
		////////////////////////////////////////////////////////////////////////
  
		// if lLayers not valid for this sector, update them.
		// Update layers for this sector.
		if (!(sectorFlags & eSectorLayersValid))
			UpdateSectorLayers( sector );

		bool bFirstLayer = true;

		// Calculate sector rectangle.
		CRect sectorRect;
		GetSectorRect( sector,sectorRect );

		// Calculate surface texture rectangle.
		CRect layerRc( sectorRect.left+rect.left,sectorRect.top+rect.top, sectorRect.left+rect.right, sectorRect.top+rect.bottom );

		CByteImage layerMask;

		////////////////////////////////////////////////////////////////////////
		// Generate the masks and the texture.
		////////////////////////////////////////////////////////////////////////
		int numLayers = GetLayerCount();
		for (i = 0; i < (int)numLayers; i++)
		{
			CLayer *pLayer = GetLayer(i);

			// Skip the layer if it is not in use
			if (!pLayer->IsInUse() || !pLayer->HasTexture())
				continue;

			// Set the write pointer (will be incremented) for the surface data
			unsigned int *pTex = pSurface;

			uint layerWidth = pLayer->GetTextureWidth();
			uint layerHeight = pLayer->GetTextureHeight();

			if (bFirstLayer)
			{
				bFirstLayer = false;
				// Draw the first layer, without layer mask.
				for (int y = layerRc.top; y < layerRc.bottom; y++)
				{
					uint layerY = y & (layerHeight-1);
					for (int x = layerRc.left; x < layerRc.right; x++)
					{
						uint layerX = x & (layerWidth-1);
						// Get the color of the tiling texture at this position
            // WAT_EDIT
						*pTex++ = pLayer->GetTexturePixel( layerX,layerY );
            //unsigned int tmp = pLayer->GetTexturePixel( layerX,layerY );
            //*pTex = (tmp & 0xff00ff00) | ((tmp & 0x000000ff) << 16) | ((tmp & 0x00ff0000) >> 16);
            //pTex++;
					}
				}
			}
			else
			{
				if (!m_layers[i].layerMask || !m_layers[i].layerMask->IsValid())
					continue;

				layerMask.Attach( *m_layers[i].layerMask );

				uint iBlend;
				COLORREF clr;

				// Draw the current layer with layer mask.
				for (int y = layerRc.top; y < layerRc.bottom; y++)
				{
					uint layerY = y & (layerHeight-1);
					for (int x = layerRc.left; x < layerRc.right; x++)
					{
						uint layerX = x & (layerWidth-1);

						// Scale the current preview coordinate to the layer mask and get the value.
						iBlend = layerMask.ValueAt(x,y);
						// Check if this pixel should be drawn.
						if (iBlend == 0)
						{
							pTex++;
							continue;
						}

						// Get the color of the tiling texture at this position
						clr = pLayer->GetTexturePixel( layerX,layerY );

						// Just overdraw when the opaqueness of the new layer is maximum
						if (iBlend == 255)
						{
							*pTex = clr;
						}
						else
						{
							// Blend the layer into the existing color, iBlend is the blend factor taken from the layer
							int iBlendSrc = 255 - iBlend;
              // WAT_EDIT
							*pTex = ((iBlendSrc * (*pTex & 0x000000FF)	+  (clr & 0x000000FF)        * iBlend) >> 8)      |
								(((iBlendSrc * (*pTex & 0x0000FF00) >>  8) + ((clr & 0x0000FF00) >>  8) * iBlend) >> 8) << 8 |
								(((iBlendSrc * (*pTex & 0x00FF0000) >> 16) + ((clr & 0x00FF0000) >> 16) * iBlend) >> 8) << 16;
							//*pTex = ((iBlendSrc * (*pTex & 0x00ff0000)	+  (clr & 0x00ff0000)        * iBlend) >> 8)      |
							//	(((iBlendSrc * (*pTex & 0x0000FF00) >>  8) + ((clr & 0x0000FF00) >>  8) * iBlend) >> 8) << 8 |
							//	(((iBlendSrc * (*pTex & 0x000000ff) >> 16) + ((clr & 0x000000ff) >> 16) * iBlend) >> 8) << 16;

            }
						pTex++;
					}
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////
	// Light the texture 
	////////////////////////////////////////////////////////////////////////
	if (bUseLighting)
	{
		LightingSettings *ls = pDocument->GetLighting();

		if (bUseLightmap)
		{
			CImage *pSectorLightmap = sectorInfo.lightmap;
			if (!(sectorFlags & eSectorLightmapValid) || !pSectorLightmap)
			{
				if (sectorInfo.lightmap)
				{
					delete sectorInfo.lightmap;
					sectorInfo.lightmap = 0;
				}

				pSectorLightmap = new CImage;
				if (!pSectorLightmap->Allocate( m_sectorResolution,m_sectorResolution ))
				{
					m_bNotValid = true;
					return false;
				}
				pSectorLightmap->Fill( 255 );
				sectorInfo.lightmap = pSectorLightmap;
				// Generate Lightmap for this sector.

				if (!GenerateLightmap( sector,ls,*pSectorLightmap,flags,m_pLightingBits ))
					return false;
			}
			Vec3 blendColor;
			blendColor.x = ls->sunMultiplier * 255; // Red										// *2 for overbrighting two times
			blendColor.y = ls->sunMultiplier * 255; // Green
			blendColor.z = ls->sunMultiplier * 255; // Blue
			// Blend lightmap with base texture.
			BlendLightmap( sector,surfaceTexture,rect,*pSectorLightmap,blendColor );
		}
		else
		{
			// If not lightmaps.
			// Pass base texture instead of lightmap (Higher precision).
			if (!GenerateLightmap( sector,ls,surfaceTexture,flags,m_pLightingBits ))
				return false;
		}
	}

	if (bConvertToABGR)
	{
		ConvertToABGR( surfaceTexture,rect );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::GetSectorRect( CPoint sector,CRect &rect )
{
	rect.left = sector.x * m_sectorResolution;
	rect.top = sector.y * m_sectorResolution;
	rect.right = rect.left + m_sectorResolution;
	rect.bottom = rect.top + m_sectorResolution;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::BlendLightmap( CPoint sector,CImage &surfaceTexture,const CRect &rect,const CImage &lightmap,const Vec3& blendColor )
{
	uint *pTexStart = surfaceTexture.GetData();
	uint *pTex = pTexStart;
	
	uint *pLightmapStart = lightmap.GetData();
	uint *pLightmap = pLightmapStart;

	int lx = rect.left;
	int ly = rect.top;
	
	uint blend_r = blendColor.x;
	uint blend_g = blendColor.y;
	uint blend_b = blendColor.z;

	uint maxBlendValue = 32768;
	
	blend_r = min(blend_r,maxBlendValue);
	blend_g = min(blend_g,maxBlendValue);
	blend_b = min(blend_b,maxBlendValue);

	uint r,g,b;
	
	int texWidth = surfaceTexture.GetWidth();
	int lightmapWidth = lightmap.GetWidth();
	int w = rect.Width();
	int h = rect.Height();
	for (int y = 0; y < h; y++)
	{
		pTex = &pTexStart[y*texWidth];
		pLightmap = &pLightmapStart[(y+ly)*lightmapWidth + lx];
		for (int x = 0; x < w; x++)
		{
			//*pTex++ = ((* pTex & 0x00FF0000) >> 16) | (* pTex & 0x0000FF00) | ((* pTex & 0x000000FF) << 16);
			uint l = *pLightmap++;

			r = (blend_r * GetRValue(l) * GetRValue(*pTex)) >> 16;
			g = (blend_g * GetGValue(l) * GetGValue(*pTex)) >> 16;
			b = (blend_b * GetBValue(l) * GetBValue(*pTex)) >> 16;
			if (r > 255) r = 255;
			if (g > 255) g = 255;
			if (b > 255) b = 255;

			*pTex++ = RGB(r,g,b);
		}
	}
}




/*
//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::GetLightmapTexture( CPoint sector,CImage &surfaceTexture,const CRect &rect,const CImage &lightmap )
{
	uint *pTexStart = surfaceTexture.GetData();
	uint *pTex = pTexStart;
	
	uint *pLightmapStart = lightmap.GetData();
	uint *pLightmap = pLightmapStart;

	int lx = rect.left;
	int ly = rect.top;
	
	int texWidth = surfaceTexture.GetWidth();
	int lightmapWidth = lightmap.GetWidth();
	int w = rect.Width();
	int h = rect.Height();

	for (int y = 0; y < h; y++)
	{
		pTex = &pTexStart[y*texWidth];
		pLightmap = &pLightmapStart[(y+ly)*lightmapWidth + lx];

		for (int x = 0; x < w; x++)
		{
			*pTex++ = RGB(GetRValue(*pLightmap),GetGValue(*pLightmap),GetBValue(*pLightmap));
			pLightmap++;
		}
	}
}
*/

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::ConvertToABGR( CImage &surfaceTexture,const CRect &rect )
{
	int texWidth = surfaceTexture.GetWidth();
	// Set the write pointer (will be incremented) for the surface data
	uint *pTex = surfaceTexture.GetData();
	uint *pTexEnd = pTex + surfaceTexture.GetWidth()*surfaceTexture.GetHeight();

	while (pTex != pTexEnd)
	{
		*pTex = ((*pTex & 0x00FF0000) >> 16) | (* pTex & 0x0000FF00) | ((* pTex & 0x000000FF) << 16);
		pTex++;
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::Log( const char *format,... )
{
	if (!m_bLog)
		return;

	va_list	ArgList;
	char		szBuffer[1024];

	va_start(ArgList, format);
	vsprintf(szBuffer, format, ArgList);
	va_end(ArgList);

	GetIEditor()->SetStatusText(szBuffer);
	CLogFile::WriteLine(szBuffer);
}

//////////////////////////////////////////////////////////////////////////
// Lighting.
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
inline Vec3 CalcVertexNormal( int x,int y,float *pHeightmapData,int resolution,float fHeightScale )
{
	if (x < resolution-1 && y < resolution-1 && x>0 && y>0)
	{
		Vec3 v1,v2,vNormal;
/*
		// First triangle
		Vec3 Tri[3];

		Tri[0].x = x;
		Tri[0].y = y;
		Tri[0].z = pHeightmapData[x+y*resolution] * fHeightScale;

		Tri[1].x = x+1;
		Tri[1].y = y;
		Tri[1].z = pHeightmapData[(x+1)+y*resolution] * fHeightScale;

		Tri[2].x = x+1;
		Tri[2].y = y+1;
		Tri[2].z = pHeightmapData[(x+1)+(y+1)*resolution] * fHeightScale;

		// Calculate the normal
		v1 = Tri[1] - Tri[0];
		v2 = Tri[2] - Tri[0];
*/

		// faster and better quality
		v1 = Vec3( 2,0, (pHeightmapData[x+1+y*resolution]-pHeightmapData[x-1+y*resolution])*fHeightScale );
		v2 = Vec3( 0,2, (pHeightmapData[x+(y+1)*resolution]-pHeightmapData[x+(y-1)*resolution])*fHeightScale );

		vNormal = v1.Cross(v2);
		vNormal.Normalize();
		return vNormal;
	}
	 else
		return(Vec3(0,0,1));
}

// r,g,b in range 0-1.
inline float ColorLuminosity( float r,float g,float b )
{
	float mx,mn;
	mx = MAX( r,g );
	mx = MAX( mx,b );
	mn = MIN( r,g );
	mn = MIN( mn,b );
	return (mx + mn) / 2.0f;
}

// r,g,b in range 0-255.
inline uint ColorLuminosityInt( uint r,uint g,uint b )
{
	uint mx,mn;
	mx = MAX( r,g );
	mx = MAX( mx,b );
	mn = MIN( r,g );
	mn = MIN( mn,b );
	return (mx + mn) >> 1;
}

//!
float CalcHeightScale( const DWORD indwTargetResolution, const uint inHeightmapsize, const int iniUnitSize )
{
	return (float)(indwTargetResolution) / (float)(inHeightmapsize) / (float)(iniUnitSize);
}


//////////////////////////////////////////////////////////////////////////
bool CTerrainTexGen::GenerateLightmap( CPoint sector,LightingSettings *pSettings,CImage &lightmap,int genFlags,CBitArray *pLightingBits )
{
	////////////////////////////////////////////////////////////////////////
	// Light the color values in a DWORD array with the generated lightmap.
	// Parameters are queried from the document. In case of the lighting
	// bit you are supposed to pass a NULL pointer if your don't want it.
	////////////////////////////////////////////////////////////////////////
	int i, j;
	float fDP3;

	CRect rc;
	GetSectorRect(sector,rc);

	float fSunCol[3];
	fSunCol[0] = GetRValue(pSettings->dwSunColor)/255.0f * pSettings->sunMultiplier; // Red
	fSunCol[1] = GetGValue(pSettings->dwSunColor)/255.0f * pSettings->sunMultiplier; // Green
	fSunCol[2] = GetBValue(pSettings->dwSunColor)/255.0f * pSettings->sunMultiplier; // Blue

	float fSkyCol[3];
	fSkyCol[0] = GetRValue( pSettings->dwSkyColor )/255.0f * pSettings->sunMultiplier; // Red
	fSkyCol[1] = GetGValue( pSettings->dwSkyColor )/255.0f * pSettings->sunMultiplier; // Green
	fSkyCol[2] = GetBValue( pSettings->dwSkyColor )/255.0f * pSettings->sunMultiplier; // Blue
	
	assert(pSettings->sunMultiplier>=0);
	assert(fSunCol[0]>=0 && fSunCol[1]>=0 && fSunCol[2]>=0);
	assert(fSkyCol[0]>=0 && fSkyCol[1]>=0 && fSkyCol[2]>=0);

	float fHeighmapSizeInMeters = m_heightmap->GetWidth()*m_heightmap->GetUnitSize();

	// Calculate a height scalation factor. This is needed to convert the
	// relative nature of the height values. The contrast value is used to
	// raise the slope of the triangles, whoch results in higher contrast
	// lighting

	// commented out  because this is doing more harm than good
	//	float fContrast = 2.0f*((float)pSettings->iContrast / 1000.0f);
	//	float fHeightScale = fContrast*m_resolution/fHeighmapSizeInMeters;

	float fHeightScale=CalcHeightScale(m_resolution,m_heightmap->GetWidth(),m_heightmap->GetUnitSize());
	float fInvHeightScale = 1.0f/fHeightScale;

	uint iWidth = m_resolution;
	uint iHeight = m_resolution;

	//////////////////////////////////////////////////////////////////////////
	// Prepare constants.
	//////////////////////////////////////////////////////////////////////////
	// Calculate the light vector
	Vec3 sunVector = pSettings->GetSunVector();

	//////////////////////////////////////////////////////////////////////////
	// Get heightmap data for this sector.
	//////////////////////////////////////////////////////////////////////////
	UpdateSectorHeightmap(sector);
	float *pHeightmapData = m_hmap.GetData();

	//////////////////////////////////////////////////////////////////////////
	// generate accessiblity for this sector.
	//////////////////////////////////////////////////////////////////////////

	if (!RefreshAccessibility(pSettings,genFlags))
	{
		CLogFile::FormatLine( "RefreshAccessibility Failed." );
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// Generate shadowmap.
	//////////////////////////////////////////////////////////////////////////
	CByteImage shadowmap;
	float shadowAmmount = 255.0f*pSettings->iShadowIntensity/100.0f;
	float fShadowIntensity = (float)pSettings->iShadowIntensity/100.0f;
	float fShadowBlur=pSettings->iShadowBlur*0.04f;				// defines the slope blurring, 0=no blurring, .. (angle would be better but is slower)

	bool bStatObjShadows = genFlags & ETTG_STATOBJ_SHADOWS;
	bool bPaintBrightness = (genFlags & ETTG_STATOBJ_PAINTBRIGHTNESS) && (m_vegetationMap != 0);
	bool bUseFastLighting = genFlags & ETTG_FAST_LLIGHTING;
	bool bTerrainShadows = pSettings->bTerrainShadows && (!(genFlags & ETTG_NO_TERRAIN_SHADOWS));


	if(!pSettings->bObjectShadows)		// no shadows for objects
		bStatObjShadows=false;

	if(bStatObjShadows)
	{
		if (!shadowmap.Allocate(m_sectorResolution,m_sectorResolution))
		{
			m_bNotValid = true;
			return false;
		}

		if(pSettings->eAlgo==ePrecise)
			GenerateShadowmap( sector,shadowmap,255,sunVector );										// shadow is full shadow (realistic)
		 else
			GenerateShadowmap( sector,shadowmap,shadowAmmount,sunVector );					// shadow percentage (more flexible?)
	}

	////////////////////////////////////////////////////////////////////////
	// Perform lighting
	////////////////////////////////////////////////////////////////////////

	// Calculate the climb factor for the shadow calculations. The expressions
	// in the braces are used to express the (anit-)proportional relationship
	// between the climb factor and various input parameters. This is f.e.
	// necessary to guarantee a consistent look of the shadows for various
	// output image sizes
	/*
	float dxStepAdjust = fHeighmapSizeInMeters/m_resolution;
	Vec3 sunXYVector = Vec3(sunVector.x,sunVector.y,0);
	float sunXYLen = GetLength(sunXYVector);
	float sunLen = GetLength(sunVector);
	// dyStep^2 = vSun^2 - dxStep^2
	float dyStep = sqrtf( sunLen*sunLen - sunXYLen*sunXYLen );
	float fClimbPerStep = dyStep*dxStepAdjust;

	int lightDirX = ftoi( (-sunVector.x) * m_resolution );
	int lightDirY = ftoi( (-sunVector.y) * m_resolution );
	*/


	uint ambient = pSettings->iAmbient;
	float fAmbient255 = ambient;
	float fAmbient = fAmbient255 / 255.0f;

	int brightness,brightness_shadowmap;

	uint *pLightmap = lightmap.GetData();

	Vec3 lightVector = -sunVector;


	Vec3 vSunShadowVector;
	{
		float invR=1.0f/(sqrtf(lightVector.x*lightVector.x + lightVector.y*lightVector.y)+0.001f);
		vSunShadowVector = lightVector*invR;
	}

	float fSkyLuminosity = ColorLuminosity( fSkyCol[0],fSkyCol[1],fSkyCol[2] );
	float fBrightness,fBrightnessShadowmap;

	eLightAlgorithm lightAlgo = pSettings->eAlgo;
	if (bUseFastLighting)
		lightAlgo = eDP3;

	// Execute actual lighting algorithm
	if (lightAlgo == eDP3)
	{
		////////////////////////////////////////////////////////////////////////
		// DP3 lighting
		////////////////////////////////////////////////////////////////////////
		for (j = rc.top; j < rc.bottom; j++)
		{
			for (i = rc.left; i < rc.right; i++)
			{
				// Calc vertex normal and Make the dot product
				Vec3 vNormal = CalcVertexNormal( i,j,pHeightmapData,m_resolution,fHeightScale );

				fDP3 = lightVector.Dot(vNormal);

				assert(fDP3>=-1.01f && fDP3<=1.01f);

				// Calculate the intensity
				float fLight = ((fDP3 + 1.0f)*0.5f + fAmbient) * 255.0f;
				fBrightness = min(1,(fDP3+1.0f) + fAmbient );

				assert(fLight>=0.0f);

				float fSunVisibility = 1;
				if (bTerrainShadows)
				{
					fSunVisibility = GetSunAmount(pHeightmapData,i,j,fInvHeightScale,vSunShadowVector,fShadowBlur);
					if (fSunVisibility < 1)
					{
						fSunVisibility = (1.0f-fShadowIntensity) + (fShadowIntensity)*fSunVisibility;
						fLight = fAmbient255*(1.0f-fSunVisibility) + fSunVisibility*fLight;
						fBrightness = fAmbient*(1.0f-fSunVisibility) + fSunVisibility*fBrightness;
					}
				}
				fBrightnessShadowmap = fBrightness;

				assert(fLight>=0.0f);

				// Apply shadow map.
				if (bStatObjShadows && fSunVisibility > 0)
				{
					float fShadow = shadowmap.ValueAt(i-rc.left,j-rc.top)*(1.0f/255.0f);
					fLight = fAmbient255*fShadow + (1.0f-fShadow)*fLight;
					
					// Calculate brightness after applying shadow map.
					fBrightnessShadowmap = fAmbient*fShadow + (1.0f-fShadow)*fBrightness;
				}

				assert(fLight>=0.0f);

				// Get Current LM pixel.
				uint *pLMPixel = &pLightmap[(i-rc.left) + (j-rc.top)*m_sectorResolution];

				//! Multiply light color with lightmap and sun color.
				uint lr = ftoi(fLight*fSunCol[0]*GetRValue(*pLMPixel)) >> 8;
				uint lg = ftoi(fLight*fSunCol[1]*GetGValue(*pLMPixel)) >> 8;
				uint lb = ftoi(fLight*fSunCol[2]*GetBValue(*pLMPixel)) >> 8;
		
				//! Clamp color to 255.
				if (lr > 255) lr = 255;
				if (lg > 255) lg = 255;
				if (lb > 255) lb = 255;
				*pLMPixel = RGB(lr,lg,lb);

				if (bPaintBrightness)
				{
					brightness = min( MAX_BRIGHTNESS,ftoi( MAX_BRIGHTNESS*fBrightness*pSettings->sunMultiplier ) );
					brightness_shadowmap = min( MAX_BRIGHTNESS,ftoi( MAX_BRIGHTNESS*fBrightnessShadowmap*pSettings->sunMultiplier) );
					// swap X/Y
					float worldPixelX = i*m_pixelSizeInMeters;
					float worldPixelY = j*m_pixelSizeInMeters;
					m_vegetationMap->PaintBrightness( worldPixelY,worldPixelX,m_pixelSizeInMeters,m_pixelSizeInMeters,brightness,brightness_shadowmap );

					// Write the lighting bits
					if (pLightingBits)
					{
						if (brightness_shadowmap > LIGHTBIT_INSHADOW_LEVEL)
						{
							uint pos = i + j*iWidth;
							pLightingBits->set(pos);
						}
					}
				}
			}
		}
	}
	else if (lightAlgo == eHemisphere)
	{
		////////////////////////////////////////////////////////////////////////
		// Hemisphere lighting
		////////////////////////////////////////////////////////////////////////
		float fSunWeight = 1;
		float fSkyWeight = 0;
		float fMaxWeight = 0.5f;

		for (j = rc.top; j < rc.bottom; j++)
		{
			for (i = rc.left; i < rc.right; i++)
			{
				// Calc vertex normal and Make the dot product
				Vec3 vNormal = CalcVertexNormal( i,j,pHeightmapData,m_resolution,fHeightScale );
				fDP3 = lightVector.Dot(vNormal);

				//assert(fDP3 >= -1.0f);
				//assert(fDP3 <= 1.0f);
				if (fDP3 <= -1.0f) fDP3 = -0.9999f;
				if (fDP3 >= 1.0f) fDP3 = 0.9999f;

				fBrightness = min(1,(fDP3+1.0f) + fAmbient );

				// Calculate the angle
				float fW = acos(fDP3);
				//assert(!_isnan(fW));
				//assert(fW >= 0.0f);
				//assert(fW <= PI);

				// Calculate weights for sun / sky color
				if (fW > PI_HALF)
				{
					fSunWeight = sinf(fW) * 0.5f;
					fSkyWeight = 1.0f - fSunWeight;
				}
				else
				{
					fSkyWeight = sinf(fW) * 0.5f;
					fSunWeight = 1.0f - fSkyWeight;
				}

				float fSunVisibility = 1;
				if (bTerrainShadows)
				{
					fSunVisibility = GetSunAmount(pHeightmapData,i,j,fInvHeightScale,vSunShadowVector,fShadowBlur);
					if (fSunVisibility < 1)
					{
						fSunVisibility = (1.0f-fShadowIntensity) + (fShadowIntensity)*fSunVisibility;
						fSunWeight *= fSunVisibility;
						fSkyWeight = (1.0f-fSunWeight);
						fBrightness = fAmbient*(1.0f-fSunVisibility) + fSunVisibility*fBrightness;
					}
				}
				fBrightnessShadowmap = fBrightness;

				/*
				if (bPaintBrightness)
				{
					// Calculate brightness before applying shadowmaps.
					if (fDP3 < 0)
					{
						// Dark side.
						float l = fSunWeight * pSettings->sunMultiplier;
						float lum = l + fSkyWeight*fSkyLuminosity;
						brightness = min( MAX_BRIGHTNESS,ftoi(lum*MAX_BRIGHTNESS) );
					}
					else
					{
						// Sunny side.
						float sun = fSunVisibility;
						float sky = 1.0f - sun;
						float lum = sun*pSettings->sunMultiplier + sky*fSkyLuminosity;
						brightness = min( MAX_BRIGHTNESS,ftoi(lum*MAX_BRIGHTNESS) );
					}
					brightness_shadowmap = brightness;
				}
				*/

				// Apply shadow map.
				if (bStatObjShadows && fSunVisibility > 0)
				{
					float fShadow = shadowmap.ValueAt(i-rc.left,j-rc.top)*(1.0f/255.0f);
					fSunWeight = fSunWeight * (1.0f-fShadow);
					fSkyWeight = (1.0f - fSunWeight);

					// Calculate brightness after applying shadow map.
					fBrightnessShadowmap = fAmbient*fShadow + (1.0f-fShadow)*fBrightness;
					/*
					if (bPaintBrightness)
					{
						if (fDP3 < 0)
						{
							// Dark side.
							// Calculate brightness before after applying shadowmaps.
							float l = fSunWeight * pSettings->sunMultiplier;
							float lum = l + fSkyWeight*fSkyLuminosity;
							brightness_shadowmap = min( MAX_BRIGHTNESS,ftoi(lum*MAX_BRIGHTNESS) );
						}
						else
						{
							// Sunny side.
							// Calculate brightness before after applying shadowmaps.
							float sun = (1.0f - fShadow)*fSunVisibility;
							float sky = 1.0f - sun;
							float lum = sun*pSettings->sunMultiplier + sky*fSkyLuminosity;
							brightness_shadowmap = min( MAX_BRIGHTNESS,ftoi(lum*MAX_BRIGHTNESS) );
						}
					}
					*/
				}
				if (bPaintBrightness)
				{
					brightness = min( MAX_BRIGHTNESS,ftoi( MAX_BRIGHTNESS*fBrightness*pSettings->sunMultiplier ) );
					brightness_shadowmap = min( MAX_BRIGHTNESS,ftoi( MAX_BRIGHTNESS*fBrightnessShadowmap*pSettings->sunMultiplier) );
					// swap X/Y
					float worldPixelX = i*m_pixelSizeInMeters;
					float worldPixelY = j*m_pixelSizeInMeters;
					m_vegetationMap->PaintBrightness( worldPixelY,worldPixelX,m_pixelSizeInMeters,m_pixelSizeInMeters,brightness,brightness_shadowmap );

					// Write the lighting bits
					if (pLightingBits)
					{
						if (brightness_shadowmap > LIGHTBIT_INSHADOW_LEVEL)
						{
							uint pos = i + j*iWidth;
							pLightingBits->set(pos);
						}
					}
				}


				// ScaleSun^2
				//fScaleSun *= fScaleSun;
				// ScaleSky^2
				//fScaleSky *= fScaleSky;

				// Get Current LM pixel.
				uint *pLMPixel = &pLightmap[(i-rc.left) + (j-rc.top)*m_sectorResolution];

				//assert(fScaleSun>=0);
				//assert(fScaleSky>=0);

				// Apply weights to colors, and multiply with lightmap.
				uint lr = ftoi((fSunWeight*fSunCol[0] + fSkyWeight*fSkyCol[0]) * GetRValue(*pLMPixel));
				uint lg = ftoi((fSunWeight*fSunCol[1] + fSkyWeight*fSkyCol[1]) * GetGValue(*pLMPixel));
				uint lb = ftoi((fSunWeight*fSunCol[2] + fSkyWeight*fSkyCol[2]) * GetBValue(*pLMPixel));


				//! Clamp color to 255.
				if (lr > 255) lr = 255;
				if (lg > 255) lg = 255;
				if (lb > 255) lb = 255;
				*pLMPixel = RGB(lr,lg,lb);
			}
		}
	}
	else if (lightAlgo == ePrecise)
	{
		assert(m_heightmap->GetWidth()==m_heightmap->GetHeight());
		bool bHaveSkyColor = (fSkyCol[0]!=0) || (fSkyCol[1]!=0) || (fSkyCol[2]!=0);
		float fSunBrightness=0;

		////////////////////////////////////////////////////////////////////////
		// Precise lighting
		////////////////////////////////////////////////////////////////////////
		for (j = rc.top; j < rc.bottom; j++)
		for (i = rc.left; i < rc.right; i++)
		{
			float fr=0.0f,fg=0.0f,fb=0.0f;	// 0..1..

			// Calc vertex normal and Make the dot product
			Vec3 vNormal = CalcVertexNormal( i,j,pHeightmapData,m_resolution,fHeightScale);

			//assert(vNormal.z>=0.0f);
			fDP3 = lightVector.Dot(vNormal);

			// Calculate the intensity (basic lambert, this is incorrect for fuzzy materials like grass, more smooth around 0 would be better)
			float fLight = max(fDP3,0);				// 0..1

			fBrightness = min(1,(fDP3+1.0f) + fAmbient );

			float fSunVisibility = 1;
			// in shadow
			if (bTerrainShadows)
			{
				// label_dddd remove soon - and integrate in other calculation modes
				fSunVisibility = GetSunAccessibilityFast(i,j);

				fBrightness = fAmbient*(1.0f-fSunVisibility) + fSunVisibility*fBrightness;
			}
			fBrightnessShadowmap = fBrightness;

			fLight *= fSunVisibility;

			float fSkyWeight = 0;
			if(bHaveSkyColor)
			{
				if(pSettings->iHemiSamplQuality)
				{
					fSkyWeight=GetSkyAccessibilityFast(i,j);		// 0..1
				}
				else fSkyWeight=vNormal.z*0.6f+0.4f;	// approximate the sky accissibility without hills, only based only on surface slope

				assert(fSkyWeight>=0.0f && fSkyWeight<=1.0f);

				//! Multiply light color with lightmap and sun color.
				fr += fSkyWeight*fSkyCol[0];	// 0..1..
				fg += fSkyWeight*fSkyCol[1];	// 0..1..
				fb += fSkyWeight*fSkyCol[2];	// 0..1..
			}

			// Apply shadow map (shadows from objects)
			if(bStatObjShadows)
			{
				float fShadow = shadowmap.ValueAt(i-rc.left,j-rc.top)*(1.0f/255.0f);
				float fShadowMapVisibility = 1.0f - fShadow;
				fLight *= fShadowMapVisibility;

				// Calculate brightness after applying shadow map.
				fBrightnessShadowmap = fAmbient*fShadow + (1.0f-fShadow)*fBrightness;
			}


			fr += fLight*fSunCol[0];	// 0..1..
			fg += fLight*fSunCol[1];	// 0..1..
			fb += fLight*fSunCol[2];	// 0..1..


			// Get Current LM pixel.
			uint *pLMPixel = &pLightmap[(i-rc.left) + (j-rc.top)*m_sectorResolution];			// uint RGB

			// Clamp color to 255.
			uint lr = min(ftoi(fr*GetRValue(*pLMPixel)),255);	// 0..255
			uint lg = min(ftoi(fg*GetGValue(*pLMPixel)),255);	// 0..255
			uint lb = min(ftoi(fb*GetBValue(*pLMPixel)),255);	// 0..255

			// store it
			*pLMPixel = RGB(lr,lg,lb);

			// brightness for vegetation
			if (bPaintBrightness)
			{
				brightness = min( MAX_BRIGHTNESS,ftoi( MAX_BRIGHTNESS*fBrightness*pSettings->sunMultiplier ) );
				brightness_shadowmap = min( MAX_BRIGHTNESS,ftoi( MAX_BRIGHTNESS*fBrightnessShadowmap*pSettings->sunMultiplier) );

				// swap X/Y
				float worldPixelX = i*m_pixelSizeInMeters;
				float worldPixelY = j*m_pixelSizeInMeters;
				m_vegetationMap->PaintBrightness( worldPixelY,worldPixelX,m_pixelSizeInMeters,m_pixelSizeInMeters,brightness,brightness_shadowmap );

				// Write the lighting bits
				if (pLightingBits)
				{
					if (brightness_shadowmap > LIGHTBIT_INSHADOW_LEVEL)
					{
						uint pos = i + j*iWidth;
						pLightingBits->set(pos);
					}
				}
			}
		}
	}

	SetSectorFlags( sector,eSectorLightmapValid );
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainTexGen::IsOccluded( float *pHeightmapData, int iWidth, int iHeight,int iStartX, int iStartY, int lightDirX,int lightDirY,float fClimbPerStep)
{
	////////////////////////////////////////////////////////////////////////
	// Checks if a terrain point can be illuminated by a given directional
	// light
	////////////////////////////////////////////////////////////////////////

	int iDeltaX, iDeltaY;
	int iUDeltaX, iUDeltaY;
	int iXAdd, iYAdd;
	int iError, iLoop;
	int X1, Y1, X2, Y2;

	if (iStartX < 0 || iStartY < 0 || iStartX >= iWidth || iStartY >= iHeight)
		return false;

	// Initial terrain height

	// Set the startpoint of the ray
	X1 = iStartX;
	Y1 = iStartY;

	// Calculate the endpoint
	X2 = X1 + lightDirX;
	Y2 = Y1 + lightDirY;

	iDeltaX = X2 - X1; // Work out X delta
	iDeltaY = Y2 - Y1; // Work out Y delta

	iUDeltaX = abs(iDeltaX); // iUDeltaX is the unsigned X delta
	iUDeltaY = abs(iDeltaY); // iUDeltaY is the unsigned Y delta

	// Work out direction to step in the Y direction
	if (iDeltaX < 0)
		iXAdd = -1;
	else
		iXAdd = 1;

	// Work out direction to step in the Y direction
	if (iDeltaY < 0)
		iYAdd = -1;
	else
		iYAdd = 1;

	iError = 0;
	iLoop = 0;

	/*
	int iTerrainPointHeight = ftoi(pHeightmapData[X1 + Y1*iWidth]*255.0f);

	if (iUDeltaX > iUDeltaY)
	{
		// Delta X > Delta Y
		do
		{
			iError += iUDeltaY;

			// Time to move up / down ?
			if (iError >= iUDeltaX)	
			{
				iError -= iUDeltaX;
				Y1 += iYAdd;
			}

			iLoop++;

			// Stop when we reach the border
			if (X1 < 0 || Y1 < 0 || X1 >= iWidth || Y1 >= iHeight)
				return false;

			// Verify heightpoint at (X1, Y1)
			if (iTerrainPointHeight < ftoi(pHeightmapData[X1 + Y1 * iWidth]*255.0f))
				return true;

			// Increase the height to similuate the elevated sun
			iTerrainPointHeight += iClimbPerStep;

			// Abort when it is impossible for the terrian point to be occluded
			if (iTerrainPointHeight > 65536)
				return false;

			// Move horizontally
			X1 += iXAdd;			
		}
		while (iLoop < iUDeltaX); // Repeat for x length of line
	}
	else
	{
		// Delta Y > Delta X
		do
		{
			iError += iUDeltaX;

			// Time to move left / right?			
			if (iError >= iUDeltaY)	
			{
				iError -= iUDeltaY;
				// Move across
				X1 += iXAdd;		
			}

			iLoop++;

			// Stop when we reach the border
			if (X1 < 0 || Y1 < 0 || X1 >= iWidth || Y1 >= iHeight)
				return false;

			// Verify heightpoint at (X1, Y1)
			if (iTerrainPointHeight < ftoi(pHeightmapData[X1 + Y1 * iWidth]*255.0f))
				return true;

			// Increase the height to similuate the elevated sun
			iTerrainPointHeight += iClimbPerStep;

			// Abort when it is impossible for the terrian point to be occluded
			if (iTerrainPointHeight > 65536)
				return false;

			// Move up / down a row
			Y1 += iYAdd;	
		}
		while (iLoop < iUDeltaY); // Repeat for y length of line
	}
	*/
	
	// maxSteps - Abort when it is impossible for the terrian point to be occluded, ray goes out of height borders.
	float fTerrainPointHeight = pHeightmapData[X1 + Y1*iWidth];
	int maxSteps = ftoi((m_terrainMaxZ - fTerrainPointHeight) / fClimbPerStep);
	int maxStepsX;
	int maxStepsY;
	if (iXAdd > 0)
		maxStepsX = (iWidth-X1);
	else
		maxStepsX = (X1-0);

	if (iYAdd > 0)
		maxStepsY = (iHeight - Y1);
	else
		maxStepsY = (Y1 - 0);
	
	maxSteps = MIN( maxSteps,maxStepsX );
	maxSteps = MIN( maxSteps,maxStepsY );

	if (iUDeltaX > iUDeltaY)
	{
		int NumSteps = iUDeltaX;
		NumSteps = MIN( NumSteps,maxSteps );

		// Delta X > Delta Y
		while (iLoop < NumSteps) // Repeat for x length of line
		{
			iError += iUDeltaY;

			// Time to move up / down ?
			if (iError >= iUDeltaX)	
			{
				iError -= iUDeltaX;
				Y1 += iYAdd;
			}

			// Increase the height to similuate the elevated sun
			fTerrainPointHeight += fClimbPerStep;

			/*
			// Stop when we reach the border
			if (X1 < 0 || Y1 < 0 || X1 >= iWidth || Y1 >= iHeight)
				return false;
			*/

			// Verify heightpoint at (X1, Y1)
			if (fTerrainPointHeight <= pHeightmapData[X1 + Y1 * iWidth])
				return true;

			// Move horizontally
			X1 += iXAdd;
			iLoop++;
		}
	}
	else
	{
		int NumSteps = iUDeltaY;
		NumSteps = MIN( NumSteps,maxSteps );

		// Delta Y > Delta X
		while (iLoop < NumSteps) // Repeat for y length of line
		{
			iError += iUDeltaX;

			// Time to move left / right?			
			if (iError >= iUDeltaY)	
			{
				iError -= iUDeltaY;
				// Move across
				X1 += iXAdd;		
			}

			// Increase the height to similuate the elevated sun
			fTerrainPointHeight += fClimbPerStep;

			/*
			// Stop when we reach the border
			if (X1 < 0 || Y1 < 0 || X1 >= iWidth || Y1 >= iHeight)
				return false;
			*/

			// Verify heightpoint at (X1, Y1)
			if (fTerrainPointHeight <= pHeightmapData[X1 + Y1 * iWidth])
				return true;

			// Move up / down a row
			Y1 += iYAdd;
			iLoop++;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//! Generate shadows from static objects and place them in shadow map bitarray.
void CTerrainTexGen::GenerateShadowmap( CPoint sector,CByteImage &shadowmap,float shadowAmmount,const Vec3 &sunVector )
{
	//	if(!m_pTerrain)
	//	return;
	SSectorInfo si;
	GetIEditor()->GetHeightmap()->GetSectorsInfo( si );

	int width = shadowmap.GetWidth();
	int height = shadowmap.GetHeight();
	int i = 0;

	int numSectors = si.numSectors;

	int sectorTexSize = shadowmap.GetWidth();
	int sectorTexSize2 = sectorTexSize*2;
	assert( sectorTexSize > 0 );

	uint shadowValue = shadowAmmount;
	
	CMemoryBlock mem;
	if (!mem.Allocate( sectorTexSize2*sectorTexSize2*3 ))
	{
		m_bNotValid = true;
		return;
	}
	unsigned char *sectorImage2 = (unsigned char*)mem.GetBuffer();

	Vec3 wp = GetIEditor()->GetHeightmap()->GetTerrainGrid()->SectorToWorld( sector );
	GetIEditor()->Get3DEngine()->MakeSectorLightMap( wp.x+0.1f,wp.y+0.1f,sectorImage2,sectorTexSize2 );

	// Scale sector image down and store into the shadow map.
	int pos;
	uint color;
	for (int j = 0; j < sectorTexSize; j++)
	{
		int sx1 = sectorTexSize - j - 1;
		for (int i = 0; i < sectorTexSize; i++)
		{
			pos = (i + j*sectorTexSize2)*2*3;
			color = (shadowValue*
				((uint)
				(255-sectorImage2[pos]) + 
				(255-sectorImage2[pos+3]) +
				(255-sectorImage2[pos+sectorTexSize2*3]) +
				(255-sectorImage2[pos+sectorTexSize2*3+3])
				)) >> 10;
			//						color = color*shadowValue >> 8;
			// swap x/y
			//color = (255-sectorImage2[(i+j*sectorTexSize)*3]);
			shadowmap.ValueAt(sx1,i) = color;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::CalcTerrainMaxZ()
{
	// Caluclate max terrain height.
	m_terrainMaxZ = 0;
	int hmapSize = m_hmap.GetSize() / sizeof(float);
	float *pData = m_hmap.GetData();
	for (int i = 0; i < hmapSize; i++)
	{
		if (pData[i] > m_terrainMaxZ)
			m_terrainMaxZ = pData[i];
	}
}

//////////////////////////////////////////////////////////////////////////
class CGenSectorThread : public CThread
{
public:
	CImage sectorImage;
	CImage *pSurfaceTexture;
	CTerrainTexGen *pTexGen;
	int sectorResolution;
	int flags;
	volatile bool bKillThread;

	MTDeque<CPoint> *pJobs;

	// Event 0 == start job.
	CEvent eventStart;
	CEvent *pEventDone;

	CGenSectorThread( int resolution )
	{
		sectorResolution = resolution;
		// start this thread.
		sectorImage.Allocate( sectorResolution,sectorResolution );
		bKillThread = false;
	}

	bool DoJob()
	{
		CPoint sector;
		if (!pJobs->pop_front(sector))
		{
			// Nothing to do.
			return false;
		}

		CRect rect(0,0,sectorResolution,sectorResolution);
		if (!pTexGen->GenerateSectorTexture( sector,rect,flags,sectorImage ))
			return false;

		CRect sectorRect;
		pTexGen->GetSectorRect(sector,sectorRect);
		pSurfaceTexture->SetSubImage( sectorRect.left,sectorRect.top,sectorImage );
		return true;
	}

protected:
	virtual void Run()
	{
		// start this thread.
		sectorImage.Allocate( sectorResolution,sectorResolution );
		//CSingleLock lock( &eventStart );

		//eventStart.Lock();
		//eventStart.ResetEvent();
		
		while (!bKillThread)
		{
			if (bKillThread)
				break;

			if (!DoJob())
			{
				bKillThread = true;
				break;
			}
		}
		// Signal done event.
		pEventDone->SetEvent();
	}
};

//////////////////////////////////////////////////////////////////////////
bool CTerrainTexGen::GenerateSurfaceTexture( int flags,CImage &surfaceTexture )
{
	if (m_bNotValid)
		return false;
	int num = 0;

	Init( surfaceTexture.GetWidth() );
	// Generate texture for all sectors.
	bool bProgress = surfaceTexture.GetWidth() >= 1024;

	if (!UpdateWholeHeightmap())
		return false;

	//////////////////////////////////////////////////////////////////////////
	LightingSettings *ls = GetIEditor()->GetDocument()->GetLighting();

	// Needed for faster shadow calculations and hemisphere sampling
	CalcTerrainMaxZ();

	//////////////////////////////////////////////////////////////////////////

	CWaitProgress wait( _T("Generating Surface Texture") );


	if (bProgress)
		wait.Start();

	if (flags & ETTG_INVALIDATE_LAYERS)
	{
		// Only invalidate layres ones at start.
		InvalidateLayers();
		flags &= ~ETTG_INVALIDATE_LAYERS;
	}

	// Multi-Threaded surface generation.
	/*
	flags &= ~ETTG_STATOBJ_SHADOWS;

	MTDeque<CPoint> jobs;
	CRect sectorRect;
	CImage sectorImage;
	sectorImage.Allocate( m_sectorResolution,m_sectorResolution );
	CRect rect(0,0,m_sectorResolution,m_sectorResolution);
	int freeThread = 0;

	// Push all jobs in.
	for (int y = 0; y < m_numSectors; y++)
	{
		for (int x = 0; x < m_numSectors; x++)
		{
			jobs.push_back( CPoint(x,y) );
		}
	}

	int totalJobs = jobs.size();

	// Start threads.
	#define NUM_THREADS 2

	CGenSectorThread *pThread[NUM_THREADS];
	CEvent threadEventDone[NUM_THREADS];
	CSyncObject* pEvents[NUM_THREADS];

	// Start threads.
	int i;
	for (i = 0; i < NUM_THREADS; i++)
	{
		pEvents[i] = &threadEventDone[i];

		pThread[i] = new CGenSectorThread( m_sectorResolution );
		pThread[i]->pJobs = &jobs;
		pThread[i]->pEventDone = &threadEventDone[i];
		pThread[i]->pSurfaceTexture = &surfaceTexture;
		pThread[i]->pTexGen = this;
		pThread[i]->flags = flags;
	}
	CMultiLock multiLockEventDone( pEvents,NUM_THREADS );

	// Do first sector, preload all.
	// Do it in main thread.
	pThread[0]->DoJob();

	for (i = 0; i < NUM_THREADS; i++)
	{
		pThread[i]->Start();
	}

	// Track progress.
	if (bProgress)
	{
		while (!jobs.empty())
		{
			int jobsLeft = jobs.size();
			int numDone = totalJobs - jobsLeft;
			wait.Step( numDone*100/totalJobs );
			Sleep(100);
	//		num++;
		}
	}


	// Wait untill all threads die.
	multiLockEventDone.Lock( INFINITE );
	*/


	CRect sectorRect;
	CRect rect(0,0,m_sectorResolution,m_sectorResolution);
	CImage sectorImage;
	if (!sectorImage.Allocate( m_sectorResolution,m_sectorResolution ))
	{
		m_bNotValid = true;
		return false;
	}
	// Normal, not multithreaded surface generation code.
	for (int y = 0; y < m_numSectors; y++)
	{
		for (int x = 0; x < m_numSectors; x++)
		{
			if (bProgress)
			{
				if (!wait.Step( num*100/(m_numSectors*m_numSectors) ))
					return false;
				num++;
			}

			CPoint sector(x,y);
			if (!GenerateSectorTexture( sector,rect,flags,sectorImage ))
				return false;

			GetSectorRect(sector,sectorRect);
			surfaceTexture.SetSubImage( sectorRect.left,sectorRect.top,sectorImage );
		}
	}
	
// M.M. to take a look at the result of the lighting calculation
//	CImageUtil::SaveImage( "c:\\temp\\out.bmp", surfaceTexture );




	if (bProgress)
		wait.Stop();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::UpdateSectorHeightmap( CPoint sector )
{
	int sectorFlags = GetSectorFlags(sector);
	if (!(sectorFlags & eSectorHeightmapValid))
	{
		CRect rect;
		GetSectorRect(sector,rect);

		// Increase rectangle by one pixel..
		rect.InflateRect( 2,2,2,2 );
	
		//m_heightmap->GetData( rect,m_hmap,true,true );
		m_heightmap->GetData( rect,m_hmap,true,true );
		SetSectorFlags( sector,eSectorHeightmapValid );
	}
}

//////////////////////////////////////////////////////////////////////////
bool CTerrainTexGen::UpdateWholeHeightmap()
{
	bool bAllValid = true;
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		if (!(m_sectorGrid[i].flags & eSectorHeightmapValid))
		{
			// Mark all heighmap sector flags as valid.
			bAllValid = false;
			m_sectorGrid[i].flags |= eSectorHeightmapValid;
		}
	}

	if (!bAllValid)
	{
		CRect rect( 0,0,m_hmap.GetWidth(),m_hmap.GetHeight() );
		if (!m_heightmap->GetData( rect,m_hmap,true,true ))
			return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
int CTerrainTexGen::GetSectorFlags( CPoint sector )
{
	assert( sector.x >= 0 && sector.x < m_numSectors && sector.y >= 0 && sector.y < m_numSectors );
	int index = sector.x + sector.y*m_numSectors;
	return m_sectorGrid[index].flags;
}

//////////////////////////////////////////////////////////////////////////
CTerrainTexGen::SectorInfo& CTerrainTexGen::GetSectorInfo( CPoint sector )
{
	assert( sector.x >= 0 && sector.x < m_numSectors && sector.y >= 0 && sector.y < m_numSectors );
	int index = sector.x + sector.y*m_numSectors;
	return m_sectorGrid[index];
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::SetSectorFlags( CPoint sector,int flags )
{
	assert( sector.x >= 0 && sector.x < m_numSectors && sector.y >= 0 && sector.y < m_numSectors );
	m_sectorGrid[sector.x + sector.y*m_numSectors].flags |= flags;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::InvalidateSector( CPoint sector )
{
	assert( sector.x >= 0 && sector.x < m_numSectors && sector.y >= 0 && sector.y < m_numSectors );
	int pos = sector.x + sector.y*m_numSectors;
	m_sectorGrid[pos].flags = 0;
	if (m_sectorGrid[pos].lightmap)
	{
		delete m_sectorGrid[pos].lightmap;
		m_sectorGrid[pos].lightmap = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::InvalidateLayers()
{
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		m_sectorGrid[i].flags &= ~eSectorLayersValid;
	}
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::InvalidateLighting()
{
	for (int i = 0; i < m_numSectors*m_numSectors; i++)
	{
		m_sectorGrid[i].flags &= ~eSectorLightmapValid;
		if (m_sectorGrid[i].lightmap)
		{
			delete m_sectorGrid[i].lightmap;
			m_sectorGrid[i].lightmap = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
int CTerrainTexGen::GetLayerCount() const
{
	return m_layers.size();
}

//////////////////////////////////////////////////////////////////////////
CLayer* CTerrainTexGen::GetLayer( int index ) const
{
	assert( index >= 0 && index < m_layers.size() );
	return m_layers[index].pLayer;
}

//////////////////////////////////////////////////////////////////////////
void CTerrainTexGen::ClearLayers()
{
	for (int i = 0; i < m_layers.size(); i++)
	{
	}
	m_layers.clear();
}

//////////////////////////////////////////////////////////////////////////
CByteImage* CTerrainTexGen::GetLayerMask( CLayer* layer )
{
	for (int i = 0; i < m_layers.size(); i++)
	{
		if (m_layers[i].pLayer == layer)
			return m_layers[i].layerMask;
	}
	return 0;
}



//////////////////////////////////////////////////////////////////////////
bool CTerrainTexGen::RefreshAccessibility( const LightingSettings *inpLSettings,int genFlags )
{
	// refresh sky?
	bool bUpdateSkyAccessiblity=false;
	if(!m_SkyAccessiblity.GetData() || m_iCachedSkyAccessiblityQuality!=inpLSettings->iHemiSamplQuality)
	{
		if(inpLSettings->eAlgo == ePrecise && !(genFlags&ETTG_FAST_LLIGHTING))
		{
			m_iCachedSkyAccessiblityQuality=inpLSettings->iHemiSamplQuality;

			if(inpLSettings->iHemiSamplQuality)
				bUpdateSkyAccessiblity=true;
		}
	}

	bool bTerrainShadows = inpLSettings->bTerrainShadows && (!(genFlags & ETTG_NO_TERRAIN_SHADOWS));

	// refresh sun?
	bool bUpdateSunAccessiblity=false;
	if(bTerrainShadows)
		if(!m_SunAccessiblity.GetData() 
			|| m_iCachedSunBlurLevel!=inpLSettings->iShadowBlur 
			|| m_vCachedSunDirection!=inpLSettings->GetSunVector())
		{
			m_iCachedSunBlurLevel=inpLSettings->iShadowBlur;
			m_vCachedSunDirection=inpLSettings->GetSunVector();

			bUpdateSunAccessiblity=true;
		}

	if(!bUpdateSkyAccessiblity && !bUpdateSunAccessiblity)
		return true;																								// no update neccessary because 

	DWORD w=m_heightmap->GetWidth(),h=m_heightmap->GetHeight();		// // unscaled

	if(m_resolution<w)w=m_resolution;			// use minimum needed size (fast in preview))
	if(m_resolution<h)h=m_resolution;			// use minimum needed size (fast in preview)


	CFloatImage	FloatHeightMap;		// unscaled

	if (!FloatHeightMap.Allocate(w,h))
	{
		m_bNotValid = true;
		return false;
	}

	CRect full(0,0,m_heightmap->GetWidth(),m_heightmap->GetHeight());

	m_heightmap->GetData( full,FloatHeightMap,false,false );		// no smooth, no noise

	// scale height to fit with the scaling in x and y direction
	{
		assert(w==h);
		float fHeightScale=CalcHeightScale(w,m_heightmap->GetWidth(),m_heightmap->GetUnitSize());
		float *p=FloatHeightMap.GetData();

		// copy intermediate to result
		for(DWORD i=0;i<w*h;i++,p++)
			(*p) *= fHeightScale;		// from 8.8 fixpoint to 8bit
	}

	// sky
	if(bUpdateSkyAccessiblity)
	{
		DWORD dwSkyAngleSteps=inpLSettings->iHemiSamplQuality*3+1;						// 1 .. 31

		CHeightmapAccessibility<CHemisphereSink_Solid> calc(w,h,dwSkyAngleSteps);

		if(!calc.Calc(FloatHeightMap.GetData()))
			return(false);
		
		// store result more compressed 
		if (!m_SkyAccessiblity.Allocate(w,h))
		{
			m_bNotValid = true;
			return false;
		}

		const unsigned short *src=calc.GetSamplePtr();
		unsigned char *dst=m_SkyAccessiblity.GetData();

		// copy intermediate to result
		for(DWORD i=0;i<w*h;i++)
		{
			unsigned short in = *src++;
			unsigned char out = (unsigned char) min( ((in+0x88)>>8), 255 );

			*dst++ = out;		// from 8.8 fixpoint to 8bit
		}
	}


	// sun
	if(bUpdateSunAccessiblity)
	{
		// quality depends on blur level
		DWORD dwSunAngleSteps=(inpLSettings->iShadowBlur+5*2-1)/5;	// 1..
		float fBlurAngle=inpLSettings->iShadowBlur*0.5f*gf_DEGTORAD;		// 0.5 means 1 unit is 0.5 degrees 
		float fHAngle=-(inpLSettings->iSunRotation-90.0f) * gf_DEGTORAD;
		float fMinHAngle=fHAngle-fBlurAngle*0.5f, fMaxHAngle=fHAngle+fBlurAngle*0.5f;
		float fVAngle=gf_PI_DIV_2-asin(inpLSettings->iSunHeight*0.01f);
		float fMinVAngle=max(0,fVAngle-fBlurAngle*0.5f), fMaxVAngle=min(gf_PI_DIV_2,fVAngle+fBlurAngle*0.5f);
		CHeightmapAccessibility<CHemisphereSink_Slice> calc(w,h,dwSunAngleSteps,fMinHAngle,fMaxHAngle);

		calc.m_Sink.SetMinAndMax(fMinVAngle,fMaxVAngle);

		if(!calc.Calc(FloatHeightMap.GetData()))
			return(false);
		
		// store result more compressed 
		if (!m_SunAccessiblity.Allocate(w,h))
		{
			m_bNotValid = true;
			return false;
		}

		const unsigned short *src=calc.GetSamplePtr();
		unsigned char *dst=m_SunAccessiblity.GetData();

		// copy intermediate to result
		for(DWORD i=0;i<w*h;i++)
		{
			unsigned short in = *src++;
			unsigned char out = (unsigned char) min( ((in+0x88)>>8), 255 );

			*dst++ = out;		// from 8.8 fixpoint to 8bit
		}
	}

	return true;
}

