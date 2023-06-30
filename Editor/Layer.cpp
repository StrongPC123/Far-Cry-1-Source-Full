// Layer.cpp: implementation of the CLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Layer.h"

#include "Heightmap.h"
#include "TerrainGrid.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//! Size of the texture preview
#define LAYER_TEX_PREVIEW_CX 128
//! Size of the texture preview
#define LAYER_TEX_PREVIEW_CY 128

#define MAX_TEXTURE_SIZE (1024*1024*2)

#define DEFAULT_MASK_RESOLUTION 4096

// Static member variables
UINT CLayer::m_iInstanceCount = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLayer::CLayer()
{
	////////////////////////////////////////////////////////////////////////
	// Initialize member variables
	////////////////////////////////////////////////////////////////////////
	// One more instance
	m_iInstanceCount++;

	m_bAutoGen = true;
	m_bNoise = true;
	m_noiseSize = 0;

	m_bSmooth = true;

	// Create a layer name based on the instance count
	m_strLayerName.Format("Layer %i", m_iInstanceCount);

	// Init member variables
	m_iLayerStart = 0;
	m_iLayerEnd = 255;
	m_strLayerTexPath = "";
	m_cTextureDimensions.cx = 0;
	m_cTextureDimensions.cy = 0;

	m_minSlope = 0;
	m_maxSlope = 255;
	m_bNeedUpdate = true;
	m_bCompressedMaskValid = false;

	// Create the DCs
	m_dcLayerTexPrev.CreateCompatibleDC(NULL);

	// Create the bitmap
	VERIFY(m_bmpLayerTexPrev.CreateBitmap(LAYER_TEX_PREVIEW_CX, LAYER_TEX_PREVIEW_CX, 1, 32, NULL));
	m_dcLayerTexPrev.SelectObject(&m_bmpLayerTexPrev);

	// Make sure the layer mask is updated the first time
	m_currWidth = m_currHeight = 0;

	// Layer is used
	m_bLayerInUse = true;
	m_bSelected = false;

	m_numSectors = 0;

	AllocateMaskGrid();
}

CLayer::~CLayer()
{
	////////////////////////////////////////////////////////////////////////
	// Clean up on exit
	////////////////////////////////////////////////////////////////////////

	m_iInstanceCount--;

	// Make sure the DCs are freed correctly
	m_dcLayerTexPrev.SelectObject((CBitmap *) NULL);

	// Free layer mask data
	m_layerMask.Release();
}

CString CLayer::GetTextureFilename() 
{
	return CString(PathFindFileName(LPCTSTR(m_strLayerTexPath)));
};

//////////////////////////////////////////////////////////////////////////
void CLayer::DrawLayerTexturePreview(LPRECT rcPos, CDC *pDC)
{
	////////////////////////////////////////////////////////////////////////
	// Draw the preview of the layer texture
	////////////////////////////////////////////////////////////////////////

	ASSERT(rcPos);
	ASSERT(pDC);
	CBrush brshGray;

	if (m_bmpLayerTexPrev.m_hObject)
	{
		pDC->SetStretchBltMode(HALFTONE);
		pDC->StretchBlt(rcPos->left, rcPos->top, rcPos->right - rcPos->left, 
			rcPos->bottom - rcPos->top, &m_dcLayerTexPrev, 0, 0,
			LAYER_TEX_PREVIEW_CX, LAYER_TEX_PREVIEW_CY, SRCCOPY);
	}
	else
	{
		brshGray.CreateSysColorBrush(COLOR_BTNFACE);
		pDC->FillRect(rcPos, &brshGray);
	}
}

void CLayer::Serialize( CXmlArchive &xmlAr )
{
	////////////////////////////////////////////////////////////////////////
	// Save or restore the class
	////////////////////////////////////////////////////////////////////////
  if (xmlAr.bLoading)
	{
		// We need an update
		InvalidateMask();

		////////////////////////////////////////////////////////////////////////
		// Loading
		////////////////////////////////////////////////////////////////////////

		CLogFile::WriteLine("Loading layer...");

		XmlNodeRef layer = xmlAr.root;

		layer->getAttr( "Name",m_strLayerName );
		
		// Texture
		layer->getAttr( "Texture",m_strLayerTexPath );
		layer->getAttr( "TextureWidth",m_cTextureDimensions.cx );
		layer->getAttr( "TextureHeight",m_cTextureDimensions.cy );

		// Parameters (Altitude, Slope...)
		layer->getAttr( "AltStart",m_iLayerStart );
		layer->getAttr( "AltEnd",m_iLayerEnd );
		layer->getAttr( "MinSlope",m_minSlope );
		layer->getAttr( "MaxSlope",m_maxSlope );

		// In use flag
		layer->getAttr( "InUse",m_bLayerInUse );

		layer->getAttr( "AutoGenMask",m_bAutoGen );

		layer->getAttr( "Noise",m_bNoise );
		layer->getAttr( "NoiseSize",m_noiseSize );

		layer->getAttr( "Smooth",m_bSmooth );

		layer->getAttr( "SurfaceType",m_surfaceType );

	/*
		if (!m_strLayerTexPath.IsEmpty())
		{
			if (!LoadTexture( m_strLayerTexPath ))
			{
				CLogFile::FormatLine("Can't load layer texture data %s!",(const char*)m_strLayerTexPath );
			}
		}
		*/

		void *pData;
		int nSize;
		if (xmlAr.pNamedData->GetDataBlock( CString("Layer_")+m_strLayerName,pData,nSize ))
		{
			// Load it
			if (!LoadTexture( (DWORD*)pData,m_cTextureDimensions.cx, m_cTextureDimensions.cy))
			{
				CLogFile::WriteLine("Can't load layer texture data !");
				AfxMessageBox("Critical error while loading texture from layer !");
			}
		}
		else
		{
			// Try loading texture from external file,
			if (!LoadTexture( m_strLayerTexPath))
			{
				CString str;
				str.Format( "Can't find or load layer texture: %s",(const char*)m_strLayerTexPath );
				CLogFile::WriteLine( str );
				AfxMessageBox( str );
			}
		}

		if (!m_bAutoGen)
		{
			int maskWidth=0,maskHeight=0;
			layer->getAttr( "MaskWidth",maskWidth );
			layer->getAttr( "MaskHeight",maskHeight );
			
			m_maskResolution = maskWidth;
			if (m_maskResolution == 0)
				m_maskResolution = GetNativeMaskResolution();

			bool bCompressed = true;
			CMemoryBlock *memBlock = xmlAr.pNamedData->GetDataBlock( CString("LayerMask_")+m_strLayerName,bCompressed );
			if (memBlock)
			{
				m_compressedMask = *memBlock;
				m_bCompressedMaskValid = true;
			}
			else
			{
				// No compressed block, fallback to back-compatability mode.
				if (xmlAr.pNamedData->GetDataBlock( CString("LayerMask_")+m_strLayerName,pData,nSize ))
				{
					CByteImage mask;
					mask.Attach( (unsigned char*)pData,maskWidth,maskHeight );
					if (maskWidth == DEFAULT_MASK_RESOLUTION)
					{
						m_layerMask.Allocate(m_maskResolution,m_maskResolution);
						m_layerMask.Copy(mask);
					}
					else
					{
						GenerateLayerMask( mask,m_maskResolution,m_maskResolution );
					}
				}
			}
		}
	}
	else
	{
		////////////////////////////////////////////////////////////////////////
		// Storing
		////////////////////////////////////////////////////////////////////////

		CLogFile::WriteLine("Storing layer...");

		XmlNodeRef layer = xmlAr.root;

		// Name
		layer->setAttr( "Name",m_strLayerName );
		
		// Texture
		layer->setAttr( "Texture",m_strLayerTexPath );
		layer->setAttr( "TextureWidth",m_cTextureDimensions.cx );
		layer->setAttr( "TextureHeight",m_cTextureDimensions.cy );

		// Parameters (Altitude, Slope...)
		layer->setAttr( "AltStart",m_iLayerStart );
		layer->setAttr( "AltEnd",m_iLayerEnd );
		layer->setAttr( "MinSlope",m_minSlope );
		layer->setAttr( "MaxSlope",m_maxSlope );

		// In use flag
		layer->setAttr( "InUse",m_bLayerInUse );

		// Auto mask or explicit mask.
		layer->setAttr( "AutoGenMask",m_bAutoGen );

		layer->setAttr( "Noise",m_bNoise );
		layer->setAttr( "NoiseSize",m_noiseSize );

		layer->setAttr( "Smooth",m_bSmooth );

		layer->setAttr( "SurfaceType",m_surfaceType );

		int layerTexureSize = m_cTextureDimensions.cx*m_cTextureDimensions.cy*sizeof(DWORD);
		if (layerTexureSize <= MAX_TEXTURE_SIZE)
		{
			PrecacheTexture();
      xmlAr.pNamedData->AddDataBlock( CString("Layer_")+m_strLayerName,m_texture.GetData(),m_texture.GetSize() );
		}

		if (!m_bAutoGen)
		{
			//xmlAr.pNamedData->AddDataBlock( CString("LayerMask_")+m_strLayerName, m_maskImage.GetData(),m_maskImage.GetSize() );

			//CMemoryBlock mem;
			//SaveMaskGrid( mem );
			//xmlAr.pNamedData->AddDataBlock( CString("LayerMaskGrid_")+m_strLayerName,mem.GetBuffer(),mem.GetSize() );

			//////////////////////////////////////////////////////////////////////////
			// Save old stuff...
			//////////////////////////////////////////////////////////////////////////
			layer->setAttr( "MaskWidth",m_maskResolution );
			layer->setAttr( "MaskHeight",m_maskResolution );
			if (m_maskFile.IsEmpty())
				layer->setAttr( "MaskFileName",m_maskFile );

			/*
			if (m_maskImage.GetSize() > 1024*1024)
			{
				// If mask is too big only store mask file name.
				layer->setAttr( "MaskFileName",m_maskFile );
			}
			else
			{
				xmlAr.pNamedData->AddDataBlock( CString("LayerMask_")+m_strLayerName, m_maskImage.GetData(),m_maskImage.GetSize() );
			}
			*/
			if (!m_bCompressedMaskValid && m_layerMask.IsValid())
			{
				CompressMask();
			}
			if (m_bCompressedMaskValid)
			{
				// Store uncompressed block of data.
				xmlAr.pNamedData->AddDataBlock( CString("LayerMask_")+m_strLayerName, m_compressedMask );
			}
			else
			{
				// no mask.
			}

			/*
			if (m_layerMask.IsValid())
			{
				xmlAr.pNamedData->AddDataBlock( CString("LayerMask_")+m_strLayerName, m_layerMask.GetData(),m_layerMask.GetSize() );
				*/
			//////////////////////////////////////////////////////////////////////////

		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CLayer::SetAutoGen( bool bAutoGen )
{
	bool prev = m_bAutoGen;
	m_bAutoGen = bAutoGen;
	
	if (prev != m_bAutoGen)
	{
		InvalidateMask();
		if (!m_bAutoGen)
		{
			m_maskResolution = GetNativeMaskResolution();
			// Not autogenerated layer must keep its mask.
			m_layerMask.Allocate( m_maskResolution,m_maskResolution );
			m_layerMask.Clear();
			SetAllSectorsValid();
		}
		else
		{
			// Release layer mask.
			m_layerMask.Release();
			InvalidateAllSectors();
		}
	}
};

void CLayer::FillWithColor( COLORREF col,int width,int height )
{
	m_cTextureDimensions = CSize(width, height);

	// Allocate new memory to hold the bitmap data
	m_cTextureDimensions = CSize(width,height);
	m_texture.Allocate( width,height );
	uint *pTex = m_texture.GetData();
	for (int i = 0; i < width * height; i++)
	{
		*pTex++ = col;
	}
}

bool CLayer::LoadTexture(LPCTSTR lpBitmapName, UINT iWidth, UINT iHeight)
{
	////////////////////////////////////////////////////////////////////////
	// Load a layer texture out of a ressource
	////////////////////////////////////////////////////////////////////////

	CBitmap bmpLoad;
	BOOL bReturn;
	
	ASSERT(lpBitmapName);
	ASSERT(iWidth);
	ASSERT(iHeight);

	// Load the bitmap
	bReturn = bmpLoad.Attach(::LoadBitmap( AfxGetInstanceHandle(),lpBitmapName));

	if (!bReturn)
	{
		ASSERT(bReturn);
		return false;
	}

	// Save the bitmap's dimensions
	m_cTextureDimensions = CSize(iWidth, iHeight);

	// Free old tetxure data

	// Allocate new memory to hold the bitmap data
	m_cTextureDimensions = CSize(iWidth,iHeight);
	m_texture.Allocate(iWidth,iHeight);

	// Retrieve the bits from the bitmap
	VERIFY(bmpLoad.GetBitmapBits(m_texture.GetSize(),m_texture.GetData() ));	

	// Convert from BGR tp RGB
	BGRToRGB();

	return true;
}

inline bool IsPower2( int n )
{
	for (int i = 0; i < 30; i++)
	{
		if (n == (1<<i))
			return true;
	}
	return false;
}

bool CLayer::LoadTexture(CString strFileName)
{
	////////////////////////////////////////////////////////////////////////
	// Load a BMP texture into the layer
	////////////////////////////////////////////////////////////////////////
	CLogFile::FormatLine("Loading layer texture (%s)...", (const char*)strFileName );

	// Save the filename
	m_strLayerTexPath = GetIEditor()->GetRelativePath(strFileName);
	if (m_strLayerTexPath.IsEmpty())
		m_strLayerTexPath = strFileName;

	if (!CImageUtil::LoadImage( m_strLayerTexPath,m_texture ))
	{
		CLogFile::FormatLine("Error loading layer texture (%s)...", (const char*)m_strLayerTexPath );
		m_cTextureDimensions.cx = 0;
		m_cTextureDimensions.cy = 0;
		return false;
	}

	if (!IsPower2(m_texture.GetWidth()) && !IsPower2(m_texture.GetHeight()))
	{
		Warning( "Selected Layer Texture must have power of 2 size." );
		m_strLayerTexPath = "";
		m_texture.Allocate( 4,4 );
		m_cTextureDimensions = CSize(m_texture.GetWidth(),m_texture.GetHeight());
		return false;
	}

	// Store the size
	m_cTextureDimensions = CSize(m_texture.GetWidth(),m_texture.GetHeight());

	CBitmap bmpLoad;
	CDC dcLoad;
	// Create the DC for blitting from the loaded bitmap
	VERIFY(dcLoad.CreateCompatibleDC(NULL));

	CImage inverted;
	inverted.Allocate( m_texture.GetWidth(),m_texture.GetHeight() );
	for (int y = 0; y < m_texture.GetHeight(); y++)
	{
		for (int x = 0; x < m_texture.GetWidth(); x++)
		{
			uint val = m_texture.ValueAt(x,y);
			inverted.ValueAt(x,y) = 0xFF000000 | RGB( GetBValue(val),GetGValue(val),GetRValue(val) );
		}
	}

	bmpLoad.CreateBitmap(m_texture.GetWidth(),m_texture.GetHeight(), 1, 32, inverted.GetData() );
	//bmpLoad.SetBitmapBits( m_texture.GetSize(),m_texture.GetData() );

	// Select it into the DC
	dcLoad.SelectObject(&bmpLoad);

	// Copy it to the preview bitmap
	m_dcLayerTexPrev.SetStretchBltMode(COLORONCOLOR);
	m_dcLayerTexPrev.StretchBlt(0, 0, LAYER_TEX_PREVIEW_CX, LAYER_TEX_PREVIEW_CY, &dcLoad, 
		0, 0, m_texture.GetWidth(),m_texture.GetHeight(), SRCCOPY);
	dcLoad.DeleteDC();

	// Convert from BGR tp RGB
	//BGRToRGB();

	return true;
}
 
bool CLayer::LoadTexture(DWORD *pBitmapData, UINT iWidth, UINT iHeight)
{
	////////////////////////////////////////////////////////////////////////
	// Load a texture from an array into the layer
	////////////////////////////////////////////////////////////////////////

	CDC dcLoad;
	CBitmap bmpLoad;
	DWORD *pPixData = NULL, *pPixDataEnd = NULL;

	if (iWidth == 0 || iHeight == 0)
	{
		ASSERT(0);
		return false;
	}

	// Allocate new memory to hold the bitmap data
	m_cTextureDimensions = CSize(iWidth,iHeight);
	m_texture.Allocate(iWidth,iHeight);

	// Copy the image data into the layer
	memcpy( m_texture.GetData(), pBitmapData, m_texture.GetSize() );

	////////////////////////////////////////////////////////////////////////
	// Generate the texture preview
	////////////////////////////////////////////////////////////////////////

	// Set the loop pointers
	pPixData = pBitmapData;
	pPixDataEnd = &pBitmapData[GetTextureWidth() * GetTextureHeight()];

	// Switch R and B
	while (pPixData != pPixDataEnd)
	{
		// Extract the bits, shift them, put them back and advance to the next pixel
		*pPixData++ = ((* pPixData & 0x00FF0000) >> 16) | 
			(* pPixData & 0x0000FF00) | ((* pPixData & 0x000000FF) << 16);
	}

	// Create the DC for blitting from the loaded bitmap
	VERIFY(dcLoad.CreateCompatibleDC(NULL));

	// Create the matching bitmap
	if (!bmpLoad.CreateBitmap(iWidth, iHeight, 1, 32, pBitmapData))
	{
		ASSERT(FALSE);
		return false;
	}

	// Select it into the DC
	dcLoad.SelectObject(&bmpLoad);

	// Copy it to the preview bitmap
	m_dcLayerTexPrev.SetStretchBltMode(COLORONCOLOR);
	m_dcLayerTexPrev.StretchBlt(0, 0, LAYER_TEX_PREVIEW_CX, LAYER_TEX_PREVIEW_CY, &dcLoad, 
		0, 0, iWidth, iHeight, SRCCOPY);

	return true;
}

bool CLayer::LoadMask( const CString &strFileName )
{
	assert( m_layerMask.IsValid() );
	if (!m_layerMask.IsValid())
		return false;
	////////////////////////////////////////////////////////////////////////
	// Load a BMP texture into the layer mask.
	////////////////////////////////////////////////////////////////////////
	CLogFile::FormatLine("Loading mask texture (%s)...", (const char*)strFileName );

	m_maskFile = GetIEditor()->GetRelativePath(strFileName);
	if (m_maskFile.IsEmpty())
		m_maskFile = strFileName;

	// Load the bitmap
	CImage maskRGBA;
	if (!CImageUtil::LoadImage( m_maskFile,maskRGBA ))
	{
		CLogFile::FormatLine("Error loading layer mask (%s)...", (const char*)m_maskFile );
		return false;
	}

	CByteImage mask;
	mask.Allocate( maskRGBA.GetWidth(),maskRGBA.GetHeight() );

	unsigned char *dest = mask.GetData();
	unsigned int *src = maskRGBA.GetData();
	int size = maskRGBA.GetWidth()*maskRGBA.GetHeight();
	for (int i = 0; i < size; i++)
	{
		*dest = *src & 0xFF;
		src++;
		dest++;
	}
	GenerateLayerMask( mask,m_layerMask.GetWidth(),m_layerMask.GetHeight() );
	return true;
}

//////////////////////////////////////////////////////////////////////////
void CLayer::GenerateLayerMask( CByteImage &mask,int width,int height )
{
	// Mask is not valid anymore.
	InvalidateMask();

	// Check if mask image is same size.
	if (width == mask.GetWidth() && height == mask.GetHeight())
	{
		m_layerMask.Copy( mask );
		return;
	}

	if (m_layerMask.GetWidth() != width || m_layerMask.GetHeight() != height)
	{
		// Allocate new memory for the layer mask
		m_layerMask.Allocate(width,height);
	}

	// Scale mask image to the size of destination layer mask.
	CImageUtil::ScaleToFit( mask,m_layerMask );

	m_currWidth = width;
	m_currHeight = height;

	if (m_layerMask.GetWidth() != mask.GetWidth() || m_layerMask.GetHeight() != mask.GetHeight())
	{
		if (m_bSmooth)
		{
			CImageUtil::SmoothImage( m_layerMask,2 );
		}
	}

	// All sectors of mask are valid.
	SetAllSectorsValid();
	
	// Free compressed mask.
	m_compressedMask.Free();
	m_bCompressedMaskValid = false;
}

void CLayer::UpdateLayerMask16(float *pHeightmapPixels, 
							   UINT iHeightmapWidth, UINT iHeightmapHeight, 
							   bool bStreamFromDisk)
{
	////////////////////////////////////////////////////////////////////////
	// Update the layer mask. The heightmap bits are supplied for speed
	// reasons, repeated memory allocations during batch generations of
	// layers are to slow
	////////////////////////////////////////////////////////////////////////
	PrecacheTexture();

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	return;

	uchar *pData = NULL;
	uchar *pArrayEnd = NULL;
	DWORD *pPreviewData = NULL;
	DWORD *pPreviewDataStart = NULL;
	unsigned int i, j;
	long iCurPos;
	float hVal = 0.0f;
	FILE *hLayerFile = NULL;

	CLogFile::WriteLine("Updating layer mask for this layer...");
	
	// Only proceed when the mask needs an update
	if (m_bNeedUpdate == false && m_currWidth == iHeightmapWidth && m_currHeight == iHeightmapHeight)
		return;

	if (!IsAutoGen())
	{
		/*
		if (m_maskImage.IsValid())
		{
			GenerateLayerMask( m_maskImage,iHeightmapWidth,iHeightmapHeight );
			m_currWidth = iHeightmapWidth;
			m_currHeight = iHeightmapHeight;
			m_bNeedUpdate = false;
			return;
		}
		*/
	}

	// Delete the old layer mask
	if (!pHeightmapPixels)
		return;

	// Allocate new memory for the layer mask
	if (m_layerMask.GetWidth() != iHeightmapWidth)
	{
		m_layerMask.Allocate( iHeightmapWidth,iHeightmapHeight );
		m_layerMask.Clear();
		InvalidateAllSectors();
	}
	
	// Set the loop pointers
	uchar *pLayerMask = m_layerMask.GetData();
	pData = &pLayerMask[iHeightmapWidth + 1];
	pArrayEnd = &pData[(iHeightmapWidth - 1) * (iHeightmapHeight - 1)];
	pHeightmapPixels = &pHeightmapPixels[iHeightmapWidth + 1];

	// We need constant random numbers
	srand(0);

	float heightmapCompensate = (iHeightmapWidth / 128.0f) * 255.0f;

	float MinAltitude = m_iLayerStart;
	float MaxAltitude = m_iLayerEnd;

	int MinSlope = ftoi(m_minSlope*255.0f);
	int MaxSlope = ftoi(m_maxSlope*255.0f);
 
	// Scan the heightmap for pixels that belong to this layer
	while (pData != pArrayEnd)
	{
		// Get the height value from the heightmap
		hVal = *pHeightmapPixels;

		if (hVal < MinAltitude || hVal > MaxAltitude)
		{
			// Advance to the next heightmap value
			pHeightmapPixels++;
			*pData++ = 0;
			continue;
		}

		// Calculate the slope for this point
		float fs = (fabs((* (pHeightmapPixels + 1)) - hVal) +
			          fabs((* (pHeightmapPixels - 1)) - hVal) +
			          fabs((* (pHeightmapPixels + iHeightmapWidth)) - hVal) +
			          fabs((* (pHeightmapPixels - iHeightmapWidth)) - hVal) +
			          fabs((* (pHeightmapPixels + iHeightmapWidth + 1)) - hVal) +
			          fabs((* (pHeightmapPixels - iHeightmapWidth - 1)) - hVal) +
        			  fabs((* (pHeightmapPixels + iHeightmapWidth - 1)) - hVal) +
		        	  fabs((* (pHeightmapPixels - iHeightmapWidth + 1)) - hVal));

		// Compensate the smaller slope for bigger heightfields
		int slope = ftoi(fs*heightmapCompensate);

		// Normalize the range to 0 - 255*255
		if (slope > 255*255)
			slope = 255*255;

		// Check if the current point belongs to the layer
		if (slope >= MinSlope && slope <= MaxSlope)
		{
			*pData = 0xFF;
		}
		else
		{
			*pData = 0;
		}
		
		/*
		// Modify it by a random value
		if (*pData && m_bNoise)
		{
			*pData -= (unsigned char) (FloatToIntRet((float) rand() / (float) RAND_MAX * iBlendFactor));
		}
		*/

		// Advance to the next heightmap value
		pHeightmapPixels++;
		*pData++;
	}
	
	CLogFile::WriteLine("Smoothing layer mask...");

	if (m_bSmooth)
	{
		// Smooth the layer
		for (j=1; j<iHeightmapHeight - 1; j++)
		{
			// Precalculate for better speed
			iCurPos = j * iHeightmapWidth + 1;
			
			for (i=1; i<iHeightmapWidth - 1; i++)
			{
				// Smooth it out
				pLayerMask[iCurPos] =(
					(uint)pLayerMask[iCurPos] +
								pLayerMask[iCurPos + 1] +
								pLayerMask[iCurPos + iHeightmapWidth] +
								pLayerMask[iCurPos + iHeightmapWidth + 1] +
								pLayerMask[iCurPos - 1] +
								pLayerMask[iCurPos - iHeightmapWidth] + 
								pLayerMask[iCurPos - iHeightmapWidth - 1] +
								pLayerMask[iCurPos - iHeightmapWidth + 1] +
								pLayerMask[iCurPos + iHeightmapWidth - 1]) / 9;
				
				// Next pixel
				iCurPos++;
			}
		}
	}

	m_currWidth = iHeightmapWidth;
	m_currHeight = iHeightmapHeight;

	// No update neccessary
	m_bNeedUpdate = false;
}

void CLayer::GenerateWaterLayer16(float *pHeightmapPixels, UINT iHeightmapWidth,UINT iHeightmapHeight,float waterLevel )
{
	////////////////////////////////////////////////////////////////////////
	// Generate a layer that is used to render the water in the preview
	////////////////////////////////////////////////////////////////////////

	uchar *pLayerEnd = NULL;
	uchar *pLayer = NULL;
	long iCurPos;
	unsigned int i, j;

	m_bAutoGen = false;

	ASSERT(!IsBadWritePtr(pHeightmapPixels, iHeightmapWidth * 
		iHeightmapHeight * sizeof(float)));

	CLogFile::WriteLine("Generating the water layer...");

	// Allocate new memory for the layer mask
	m_layerMask.Allocate( iHeightmapWidth,iHeightmapHeight );

	uchar *pLayerMask = m_layerMask.GetData();

	// Set the loop pointers
	pLayerEnd = &pLayerMask[iHeightmapWidth * iHeightmapHeight];
	pLayer = pLayerMask;

	// Generate the layer
	while (pLayer != pLayerEnd)
		*pLayer++ = ((* pHeightmapPixels++) <= waterLevel) ? 127 : 0;

	if (m_bSmooth)
	{
		// Smooth the layer
		for (j=1; j<iHeightmapHeight - 1; j++)
		{
			// Precalculate for better speed
			iCurPos = j * iHeightmapWidth + 1;
			
			for (i=1; i<iHeightmapWidth - 1; i++)
			{
				// Next pixel
				iCurPos++;
				
				// Smooth it out
				pLayerMask[iCurPos] = ftoi(
					(pLayerMask[iCurPos] +
					pLayerMask[iCurPos + 1]+
					pLayerMask[iCurPos + iHeightmapWidth] +
					pLayerMask[iCurPos + iHeightmapWidth + 1] +
					pLayerMask[iCurPos - 1]+
					pLayerMask[iCurPos - iHeightmapWidth] +
					pLayerMask[iCurPos - iHeightmapWidth - 1] +
					pLayerMask[iCurPos - iHeightmapWidth + 1] +
					pLayerMask[iCurPos + iHeightmapWidth - 1])
					* 0.11111111111f);
			}
		}
	}
	m_currWidth = iHeightmapWidth;
	m_currHeight = iHeightmapHeight;

	m_bNeedUpdate = false;
}

void CLayer::BGRToRGB()
{
	////////////////////////////////////////////////////////////////////////
	// Convert the layer data from BGR to RGB
	////////////////////////////////////////////////////////////////////////
	PrecacheTexture();

	DWORD *pPixData = NULL, *pPixDataEnd = NULL;

	// Set the loop pointers
	pPixData = (DWORD*)m_texture.GetData();
	pPixDataEnd = pPixData + (m_texture.GetWidth()*m_texture.GetHeight());

	// Switch R and B
	while (pPixData != pPixDataEnd)
	{
		// Extract the bits, shift them, put them back and advance to the next pixel
		*pPixData++ = ((* pPixData & 0x00FF0000) >> 16) | 
			(* pPixData & 0x0000FF00) | ((* pPixData & 0x000000FF) << 16);
	}
}

void CLayer::ExportTexture(CString strFileName)
{
	////////////////////////////////////////////////////////////////////////
	// Save the texture data of this layer into a BMP file
	////////////////////////////////////////////////////////////////////////

	DWORD *pTempBGR = NULL;
	DWORD *pPixData = NULL, *pPixDataEnd = NULL;

	CLogFile::WriteLine("Exporting texture from layer data to BMP...");

	PrecacheTexture();

	if (!m_texture.IsValid())
		return;

	// Make a copy of the layer data
	CImage image;
	image.Allocate(GetTextureWidth(),GetTextureHeight());
	pTempBGR = (DWORD*)image.GetData();
	memcpy(pTempBGR, m_texture.GetData(),m_texture.GetSize());

	// Set the loop pointers
	pPixData = pTempBGR;
	pPixDataEnd = &pTempBGR[GetTextureWidth() * GetTextureHeight()];

	/*
	// Switch R and B
	while (pPixData != pPixDataEnd)
	{
		// Extract the bits, shift them, put them back and advance to the next pixel
		*pPixData++ = ((* pPixData & 0x00FF0000) >> 16) | 
			(* pPixData & 0x0000FF00) | ((* pPixData & 0x000000FF) << 16);
	}
	*/
	
	// Write the bitmap
	CImageUtil::SaveImage( strFileName,image );
}

void CLayer::ExportMask( const CString &strFileName )
{
	////////////////////////////////////////////////////////////////////////
	// Save the texture data of this layer into a BMP file
	////////////////////////////////////////////////////////////////////////
	DWORD *pTempBGR = NULL;

	CLogFile::WriteLine("Exporting layer mask to BMP...");

	CByteImage layerMask;
	uchar *pLayerMask = NULL;

	int w,h;

	if (m_bAutoGen)
	{
		int nativeResolution = GetNativeMaskResolution();
		w = h = nativeResolution;
		// Create mask at native texture resolution.
		CRect rect( 0,0,nativeResolution,nativeResolution );
		CFloatImage hmap;
		hmap.Allocate( nativeResolution,nativeResolution );
		layerMask.Allocate( nativeResolution,nativeResolution );
		// Get hmap at native resolution.
		GetIEditor()->GetHeightmap()->GetData( rect,hmap,true,true );
		AutogenLayerMask( rect,hmap,layerMask );

		pLayerMask = layerMask.GetData();
	}
	else
	{
		PrecacheMask();
		pLayerMask = m_layerMask.GetData();
		w = m_layerMask.GetWidth();
		h = m_layerMask.GetHeight();
	}

	if (w == 0 || h == 0 || pLayerMask == 0)
	{
		AfxMessageBox( "Cannot export empty mask" );
		return;
	}

	// Make a copy of the layer data
	CImage image;
	image.Allocate(w,h);
	pTempBGR = (DWORD*)image.GetData();
	for (int i = 0; i < w*h; i++)
	{
		uint col = pLayerMask[i];
		pTempBGR[i] = col | col << 8 | col << 16;
	}

	// Write the mask
	CImageUtil::SaveImage( strFileName,image );
}

//! Release allocated mask.
void CLayer::ReleaseMask()
{
	// Free layer mask data
	if (IsAutoGen())
		m_layerMask.Release();
	else
	{
		//CompressMask();
	}
	m_currWidth = 0;
	m_currHeight = 0;

	// If texture image is too big, release it.
	if (m_texture.GetSize() > MAX_TEXTURE_SIZE)
		m_texture.Release();
}

void CLayer::PrecacheTexture()
{
	if (m_texture.IsValid())
		return;

	if (m_strLayerTexPath.IsEmpty())
	{
		m_cTextureDimensions = CSize(4,4);
		m_texture.Allocate(m_cTextureDimensions.cx,m_cTextureDimensions.cy);
	}
	else if (!CImageUtil::LoadImage( m_strLayerTexPath,m_texture ))
	{
		Error( "Error loading layer texture (%s)...", (const char*)m_strLayerTexPath );
		m_cTextureDimensions = CSize(4,4);
		m_texture.Allocate(m_cTextureDimensions.cx,m_cTextureDimensions.cy);
		return;
	}
	m_cTextureDimensions.cx = m_texture.GetWidth();
	m_cTextureDimensions.cx = m_texture.GetHeight();
	// Convert from BGR tp RGB
	//BGRToRGB();
}

//////////////////////////////////////////////////////////////////////////
void CLayer::InvalidateAllSectors()
{
	// Fill all sectors with 0, (clears all flags).
	memset( &m_maskGrid[0],0,m_maskGrid.size()*sizeof(m_maskGrid[0]) );
}

//////////////////////////////////////////////////////////////////////////
void CLayer::SetAllSectorsValid()
{
	// Fill all sectors with 0xFF, (Set all flags).
	memset( &m_maskGrid[0],0xFF,m_maskGrid.size()*sizeof(m_maskGrid[0]) );
}

//////////////////////////////////////////////////////////////////////////
void CLayer::InvalidateMaskSector( CPoint sector )
{
	GetSector(sector) = 0;
	m_bCompressedMaskValid = false;
}

//////////////////////////////////////////////////////////////////////////
void CLayer::InvalidateMask()
{
	m_bNeedUpdate = true;

	InvalidateAllSectors();
	if (m_scaledMask.IsValid())
		m_scaledMask.Release();
	/*
	if (m_layerMask.IsValid
	if (m_bCompressedMaskValid)
		m_compressedMask.Free();
	m_bCompressedMaskValid = false;
	m_compressedMask.Free();
	*/
};

//////////////////////////////////////////////////////////////////////////
void CLayer::PrecacheMask()
{
	if (IsAutoGen())
		return;
	if (!m_layerMask.IsValid())
	{
		m_layerMask.Allocate( m_maskResolution,m_maskResolution );
		if (!m_layerMask.IsValid())
		{
			Warning( "Layer %s compressed mask have invalid resolution %d.",(const char*)m_strLayerName,m_maskResolution );
			m_layerMask.Allocate( GetNativeMaskResolution(),GetNativeMaskResolution() );
			return;
		}
		if (m_bCompressedMaskValid)
		{
			bool bUncompressOk = m_layerMask.Uncompress( m_compressedMask );
			m_compressedMask.Free();
			m_bCompressedMaskValid = false;
			if (!bUncompressOk)
			{
				Warning( "Layer %s compressed mask have invalid resolution.",(const char*)m_strLayerName );
			}
		}
		else
		{
			// No compressed layer data.
			m_layerMask.Clear();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
CByteImage& CLayer::GetMask()
{
	PrecacheMask();
	return m_layerMask;
}

//////////////////////////////////////////////////////////////////////////
bool CLayer::UpdateMaskForSector( CPoint sector,const CRect &sectorRect,const CFloatImage &hmap,CByteImage& mask )
{
	////////////////////////////////////////////////////////////////////////
	// Update the layer mask. The heightmap bits are supplied for speed
	// reasons, repeated memory allocations during batch generations of
	// layers are to slow
	////////////////////////////////////////////////////////////////////////
	PrecacheTexture();
	PrecacheMask();

	int resolution = hmap.GetWidth();
	
	uchar &sectorFlags = GetSector(sector);
	if (sectorFlags & SECTOR_MASK_VALID && !m_bNeedUpdate)
	{
		if (m_layerMask.GetWidth() == resolution)
		{
			mask.Attach(m_layerMask);
			return true;
		}
	}
	m_bNeedUpdate = false;
	
	if (!IsAutoGen())
	{
		if (!m_layerMask.IsValid())
			return false;

		if (resolution == m_layerMask.GetWidth())
		{
			mask.Attach(m_layerMask);
		}
		else if (resolution == m_scaledMask.GetWidth())
		{
			mask.Attach(m_scaledMask);
		}
		else
		{
			m_scaledMask.Allocate( resolution,resolution );
			CImageUtil::ScaleToFit( m_layerMask,m_scaledMask );
			if (m_bSmooth)
			{
				CImageUtil::SmoothImage( m_scaledMask,2 );
			}
			mask.Attach(m_scaledMask);
		}

		// Mark this sector as valid.
		sectorFlags |= SECTOR_MASK_VALID;
		// All valid.
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Auto generate mask.
	//////////////////////////////////////////////////////////////////////////

	// If layer mask differ in size, invalidate all sectors.
	if (resolution != m_layerMask.GetWidth())
	{
		m_layerMask.Allocate( resolution,resolution );
		m_layerMask.Clear();
		InvalidateAllSectors();
	}

	// Mark this sector as valid.
	sectorFlags |= SECTOR_MASK_VALID;

	mask.Attach(m_layerMask);

	float hVal = 0.0f;

	AutogenLayerMask( sectorRect,hmap,m_layerMask );
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool CLayer::UpdateMask( const CFloatImage &hmap,CByteImage& mask )
{
	PrecacheTexture();
	PrecacheMask();

	int resolution = hmap.GetWidth();

	if (!m_bNeedUpdate && m_layerMask.GetWidth() == resolution)
	{
		mask.Attach( m_layerMask );
		return true;
	}
	m_bNeedUpdate = false;
	
	if (!IsAutoGen())
	{
		if (!m_layerMask.IsValid())
			return false;

		if (resolution == m_layerMask.GetWidth())
		{
			mask.Attach(m_layerMask);
		}
		else if (resolution == m_scaledMask.GetWidth())
		{
			mask.Attach(m_scaledMask);
		}
		else
		{
			m_scaledMask.Allocate( resolution,resolution );
			CImageUtil::ScaleToFit( m_layerMask,m_scaledMask );
			if (m_bSmooth)
			{
				CImageUtil::SmoothImage( m_scaledMask,2 );
			}
			mask.Attach(m_scaledMask);
		}
		// All valid.
		SetAllSectorsValid();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Auto generate mask.
	//////////////////////////////////////////////////////////////////////////

	// If layer mask differ in size, invalidate all sectors.
	if (resolution != m_layerMask.GetWidth())
	{
		m_layerMask.Allocate( resolution,resolution );
		m_layerMask.Clear();
	}

	mask.Attach(m_layerMask);

	CRect rect(0,0,resolution,resolution);
	AutogenLayerMask( rect,hmap,m_layerMask );
	SetAllSectorsValid();

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CLayer::AutogenLayerMask( const CRect &rc,const CFloatImage &hmap,CByteImage& mask )
{
	CRect rect = rc;

	assert( hmap.IsValid() );
	assert( mask.IsValid() );
	assert( hmap.GetWidth() == mask.GetWidth() );

	int resolution = hmap.GetWidth();

	// Inflate rectangle.
	rect.InflateRect(1,1,1,1);

	rect.left = max(rect.left,1);
	rect.top = max(rect.top,1);
	rect.right = min(rect.right,resolution-1);
	rect.bottom = min(rect.bottom,resolution-1);

	// Set the loop pointers
	uchar *pLayerMask = mask.GetData();
	float *pHmap = hmap.GetData();

	// We need constant random numbers
	srand(0);

	float heightmapCompensate = (resolution / 128.0f) * 256;

	float MinAltitude = m_iLayerStart;
	float MaxAltitude = m_iLayerEnd;

	int MinSlope = RoundFloatToInt(m_minSlope*256);
	int MaxSlope = RoundFloatToInt(m_maxSlope*256);

	// Scan the heightmap for pixels that belong to this layer
	unsigned int x,y,pos;
	unsigned int hw = resolution;
	float hVal = 0.0f;

	for (y = rect.top; y < rect.bottom; y++)
	{
		for (x = rect.left; x < rect.right; x++,pos++)
		{
			pos = x + y*resolution;
			// Get the height value from the heightmap
			float *h = &pHmap[pos];
			hVal = *h;

			if (hVal < MinAltitude || hVal > MaxAltitude)
			{
				pLayerMask[pos] = 0;
				continue;
			}

			// Calculate the slope for this point
			float fs = (
				fabs((*(h + 1)) - hVal) +
				fabs((*(h - 1)) - hVal) +
				fabs((*(h + hw)) - hVal) +
				fabs((*(h - hw)) - hVal) +
				fabs((*(h + hw + 1)) - hVal) +
				fabs((*(h - hw - 1)) - hVal) +
				fabs((*(h + hw - 1)) - hVal) +
				fabs((*(h - hw + 1)) - hVal));

			// Compensate the smaller slope for bigger heightfields
			int slope = RoundFloatToInt(fs*heightmapCompensate);

			// Normalize the range to 0 - 255*256
			if (slope > 255*256)
				slope = 255*256;

			// Check if the current point belongs to the layer
			if (slope >= MinSlope && slope <= MaxSlope)
			{
				pLayerMask[pos] = 0xFF;
			}
			else
			{
				pLayerMask[pos] = 0;
			}

			/*
			// Modify it by a random value
			if (*pData && m_bNoise)
			{
			*pData -= (unsigned char) (FloatToIntRet((float) rand() / (float) RAND_MAX * iBlendFactor));
			}
			*/
		}
	}
	rect.DeflateRect(1,1,1,1);
	
	if (m_bSmooth)
	{
		// Smooth the layer
		for (y = rect.top; y < rect.bottom; y++)
		{
			pos = y*resolution + rect.left;
			for (x = rect.left; x < rect.right; x++,pos++)
			{
				// Smooth it out
				pLayerMask[pos] =(
					(uint)pLayerMask[pos] +
					pLayerMask[pos + 1] +
					pLayerMask[pos + hw] +
					pLayerMask[pos + hw + 1] +
					pLayerMask[pos - 1] +
					pLayerMask[pos - hw] + 
					pLayerMask[pos - hw - 1] +
					pLayerMask[pos - hw + 1] +
					pLayerMask[pos + hw - 1]) / 9;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CLayer::AllocateMaskGrid()
{
	CHeightmap *pHeightmap = GetIEditor()->GetHeightmap();
	SSectorInfo si;
	pHeightmap->GetSectorsInfo( si );
	m_numSectors = si.numSectors;
	m_maskGrid.resize( si.numSectors*si.numSectors );
	memset( &m_maskGrid[0],0,m_maskGrid.size()*sizeof(m_maskGrid[0]) );

	m_maskResolution = si.surfaceTextureSize;
}

//////////////////////////////////////////////////////////////////////////
uchar& CLayer::GetSector( CPoint sector )
{
	int p = sector.x + sector.y*m_numSectors;
	assert( p >= 0 && p < m_maskGrid.size() );
	return m_maskGrid[p];
}

//////////////////////////////////////////////////////////////////////////
void CLayer::CompressMask()
{
	if (m_bCompressedMaskValid)
		return;

	m_bCompressedMaskValid = false;
	m_compressedMask.Free();

	// Compress mask.
	if (m_layerMask.IsValid())
	{
		m_layerMask.Compress( m_compressedMask );
		m_bCompressedMaskValid = true;
	}
	m_layerMask.Release();
	m_scaledMask.Release();
}

//////////////////////////////////////////////////////////////////////////
int CLayer::GetSize() const
{
	int size = sizeof(*this);
	size += m_texture.GetSize();
	size += m_layerMask.GetSize();
	size += m_scaledMask.GetSize();
	size += m_compressedMask.GetSize();
	size += m_maskGrid.size()*sizeof(unsigned char);
	return size;
}

//////////////////////////////////////////////////////////////////////////
//! Export layer block.
void CLayer::ExportBlock( const CRect &rect,CXmlArchive &xmlAr )
{
	// ignore autogenerated layers.
	if (m_bAutoGen)
		return;

	XmlNodeRef node = xmlAr.root;

	PrecacheMask();
	if (!m_layerMask.IsValid())
		return;

	node->setAttr( "Name",m_strLayerName );
	node->setAttr( "MaskWidth",m_layerMask.GetWidth() );
	node->setAttr( "MaskHeight",m_layerMask.GetHeight() );

	CRect subRc( 0,0,m_layerMask.GetWidth(),m_layerMask.GetHeight() );
	subRc &= rect;

	node->setAttr( "X1",subRc.left );
	node->setAttr( "Y1",subRc.top );
	node->setAttr( "X2",subRc.right );
	node->setAttr( "Y2",subRc.bottom );

	if (!subRc.IsRectEmpty())
	{
		CByteImage subImage;
		subImage.Allocate( subRc.Width(),subRc.Height() );
		m_layerMask.GetSubImage( subRc.left,subRc.top,subRc.Width(),subRc.Height(),subImage );

		xmlAr.pNamedData->AddDataBlock( CString("LayerMask_")+m_strLayerName,subImage.GetData(),subImage.GetSize() );
	}
}

//! Import layer block.
void CLayer::ImportBlock( CXmlArchive &xmlAr,CPoint offset )
{
	// ignore autogenerated layers.
	if (m_bAutoGen)
		return;

	XmlNodeRef node = xmlAr.root;

	PrecacheMask();
	if (!m_layerMask.IsValid())
		return;

	CRect subRc;

	node->getAttr( "X1",subRc.left );
	node->getAttr( "Y1",subRc.top );
	node->getAttr( "X2",subRc.right );
	node->getAttr( "Y2",subRc.bottom );

	void *pData;
	int nSize;
	if (xmlAr.pNamedData->GetDataBlock( CString("LayerMask_")+m_strLayerName, pData,nSize ))
	{
		CByteImage subImage;
		subImage.Attach( (unsigned char*)pData,subRc.Width(),subRc.Height() );
		m_layerMask.SetSubImage( subRc.left+offset.x,subRc.top+offset.y,subImage );
	}
}

//////////////////////////////////////////////////////////////////////////
int CLayer::GetNativeMaskResolution() const
{
	CHeightmap *pHeightmap = GetIEditor()->GetHeightmap();
	SSectorInfo si;
	pHeightmap->GetSectorsInfo( si );
	return si.surfaceTextureSize;
}