#include "StdAfx.h"
#include <assert.h>							// assert()
#include <Dbghelp.h>						// GetTimestampForLoadedLibrary

#include "IRCLog.h"								// IRCLog
#include "IConfig.h"						// IConfig
#include "PathUtil.h"						// ReplaceExtension

#include "ImageCompiler.h"			// CImageCompiler
#include "UserDialog.h"					// CUserDialog
#include "ICfgFile.h"						// ICfgFile

#include <ddraw.h>

#include "neuquant.h"

#include "ImageObject.h"
#include <commctrl.h>								// TCITEM

#define CCERROR		m_pCC->pError
#define CCLOG			m_pCC->pLog


struct SPixelFormats
{
	int iBitsPerPixel;
	char *szAlpha;
	D3DFORMAT DxtNo;
	char *szName;
	char *szUncompressedAltName;
	char *szDescription;
	bool bCompressed;
};



// *.dds /targetroot:c:\mastercd2 /reduce:1 /userdialog=0

static SPixelFormats g_pixelformats[]=
{
// Unsigned Formats (Data in an unsigned format must be positive. Unsigned formats use combinations of
// (R)ed, (G)reen, (B)lue, (A)lpha, (L)uminance, and (P)alette data.
// Palette data is also referred to as color indexed data because the data is used to index a color palette.)

		{ 24,"0",			D3DFMT_R8G8B8,			"R8G8B8",			"X8R8G8B8",		"24-bit RGB pixel format with 8 bits per channel",false },
		{ 32,"8",			D3DFMT_A8R8G8B8,		"A8R8G8B8",		"A8R8G8B8",		"32-bit ARGB pixel format with alpha, using 8 bits per channel",false },
		{ 32,"0",			D3DFMT_X8R8G8B8,		"X8R8G8B8",		"X8R8G8B8",		"32-bit RGB pixel format, where 8 bits are reserved for each color",false },
		{ 16,"0",			D3DFMT_R5G6B5,			"R5G6B5",			"X8R8G8B8",		"16-bit RGB pixel format with 5 bits red, 6 bits green, and 5 bits blue",false },
		{ 16,"1",			D3DFMT_A1R5G5B5,		"A1R5G5B5",		"A8R8G8B8",		"16-bit ARGB using 5 bits for each color and 1 bit for alpha",false },
		{ 16,"0",			D3DFMT_X1R5G5B5,		"X1R5G5B5",		"X8R8G8B8",		"16-bit RGB using 5 bits for each color",false },
		{ 16,"4",			D3DFMT_A4R4G4B4,		"A4R4G4B4",		"A8R8G8B8",		"16-bit ARGB with 4 bits for each channel",false },
		{ 16,"0",			D3DFMT_X4R4G4B4,		"X4R4G4B4",		"X8R8G8B8",		"16-bit RGB using 4 bits for each color",false },
		{ 8, "8",			D3DFMT_A8,					"A8",					"A8",					"8-bit alpha only",false },
		{ 8, "0",			D3DFMT_R3G3B2,			"R3G3B2",			"X8R8G8B8",		"8-bit RGB using 3 bits red, 3 bits green, and 2 bits blue",false },
//	{ 16,"8",			D3DFMT_A8R3G3B2,		"A8R3G3B2",		"A8R8G8B8",		"16-bit ARGB texture format using 8 bits for alpha, 3 bits each for red and green, and 2 bits for blue",false },
//	{ 32,"2",			D3DFMT_A2B10G10R10,	"A2B10G10R10","A2B10G10R10","32-bit pixel format using 10 bits for each color and 2 bits for alpha",false },
//	{ 32,"0",			D3DFMT_G16R16,			"G16R16",			"G16R16",			"32-bit pixel format using 16 bits each for green and red",false },
//	{ 8, "8",			D3DFMT_A8P8,				"A8P8",				"A8P8",				"8-bit color indexed with 8 bits of alpha",false },
		{ 8, "0",			D3DFMT_P8,					"P8",					"X8R8G8B8",		"8-bit color indexed",false },
		{ 8, "0",			D3DFMT_L8,					"L8",					"L8",					"8-bit luminance only",false },
//	{ 16,"8",			D3DFMT_A8L8,				"A8L8",				"A8L8",				"16-bit using 8 bits each for alpha and luminance",false },
//	{ 8, "4",			D3DFMT_A4L4,				"A4L4",				"A8L8",				"8-bit using 4 bits each for alpha and luminance",false },

// Signed Formats (Data in a signed format can be both positive and negative. Signed formats use combinations of (U), (V), (W), and (Q) data.) -----------------------------------------

		{ 16,"0",			D3DFMT_V8U8,				"V8U8",				"V8U8",				"16-bit signed bump-map format using 8 bits each for u and v data",false },
//	{ 32,"0",			D3DFMT_Q8W8V8U8,		"Q8W8V8U8",		"Q8W8V8U8",		"32-bit signed bump-map format using 8 bits for each channel",false },
//		{ 32,"0",			D3DFMT_V16U16,			"V16U16",			"V16U16",			"32-bit signed bump-map format using 16 bits for each channel",false },
//	{ 32,"0",			D3DFMT_W11V11U10,		"W11V11U10",	"W11V11U10",	"32-bit bump-map format using 11 bits each for w and v, and 10 bits for u",false },

// Mixed Formats (Data in mixed formats can contain a combination of unsigned data and signed data.) -----------------------------------------

//	{ 16,"0",			D3DFMT_L6V5U5,			"L6V5U5",			"X8L8V8U8",		"16-bit signed/unsigned bump-map format with luminance using 6 bits for luminance, and 5 bits each for u and v",false },
//	{ 32,"0",			D3DFMT_X8L8V8U8,		"X8L8V8U8",		"X8L8V8U8",		"32-bit signed/unsigned bump-map format with luminance using 8 bits for each channel",false },
//	{ 32,"2",			D3DFMT_A2W10V10U10,	"A2W10V10U10","A2W10V10U10","32-bit signed/unsigned bump-map format using 2 bits for alpha and 10 bits each for w, v, and u",false },

// FourCC Formats (Data in a FourCC format is compressed data.) -----------------------------------------

//	{ 16,"0",			D3DFMT_UYVY,				"UYVY",				"A8R8G8B8",		"UYVY compressed format (PC98 compliance)",true },
//	{ 16,"0",			D3DFMT_YUY2,				"YUY2",				"A8R8G8B8",		"YUY2 compressed format (PC98 compliance)",true },
		{ 8, "0/1",		D3DFMT_DXT1,				"DXT1",				"A8R8G8B8",		"DXT1 compression texture format",true },
//	{ 16,"4p",		D3DFMT_DXT2,				"DXT2",				"A8R8G8B8",		"DXT2 compression texture format",true },
		{ 16,"4",			D3DFMT_DXT3,				"DXT3",				"A8R8G8B8",		"DXT3 compression texture format",true },
//	{ 16,"3of8p",	D3DFMT_DXT4,				"DXT4",				"A8R8G8B8",		"DXT4 compression texture format",true },
		{ 16,"3of8",	D3DFMT_DXT5,				"DXT5",				"A8R8G8B8",		"DXT5 compression texture format",true }
};




// constructor
CImageCompiler::CImageCompiler()
{
	m_pd3ddev=0;
	m_pd3d=0;
	m_ptexOrig=0;
	m_pNewImage=NULL;
	m_dwOrigWidth=0;
	m_dwOrigHeight=0;
	m_dwDepth=0;
	m_dwCubeMapFlags=0;
	m_pCC=0;
	m_iOrigPixelFormatNo=0;
	m_hWnd=0;
}


// destructor
CImageCompiler::~CImageCompiler()
{
	DeInit();

	assert(!m_pd3ddev);
	assert(!m_pd3d);
	assert(!m_ptexOrig);
	assert(!m_pNewImage);
}



void CImageCompiler::DeInit( void )
{
	ReleasePpo(&m_ptexOrig);
	if(m_pNewImage) delete m_pNewImage;

	ReleasePpo(&m_pd3ddev);
	ReleasePpo(&m_pd3d);

	UnregisterClass("skeleton", g_hInst);
}


bool CImageCompiler::Save( const char *lpszPathName )
{
	if(!m_pNewImage)
	{
		CCLOG->LogError("CImageCompiler::Save failed (conversion failed?)");
		return(false);
	}
	
	if(!m_pNewImage->Save(lpszPathName))
	{
		CCLOG->LogError("CImageCompiler::Save failed (file IO?)");
		return false;
	};
	
	/*
	//LPDIRECT3DTEXTURE9 pmiptexSrc = (LPDIRECT3DTEXTURE9)ptexSrc;
	LPDIRECT3DSURFACE9 psurf = NULL;
	DXERRORCHECK(((LPDIRECT3DTEXTURE9)m_ptexNew)->GetSurfaceLevel(0, &psurf));

	D3DSURFACE_DESC sd;
	psurf->GetDesc(&sd);
	
	switch(sd.Format)
	{
		case D3DFMT_P8:
			if(!m_pPalette)
			{
				CCLOG->LogError("CImageCompiler::Save missing palette??");
				return(false);
			};
			if(!SavePaletizedTexture(lpszPathName, (LPDIRECT3DTEXTURE9)m_ptexNew, sd))
			{
				CCLOG->LogError("CImageCompiler::Save palettized texture failed");
				return(false);
			};	
			free(m_pPalette);
			m_pPalette = NULL;
			break;		
		
		default:
			if( FAILED( D3DXSaveTextureToFile( lpszPathName, D3DXIFF_DDS, m_ptexNew, NULL ) ) )
			{
				CCLOG->LogError("CImageCompiler::Save D3DXSaveTextureToFile failed (file IO?)");
				return(false);
			};
			break;
	};
	*/

	return true;
}


LRESULT CALLBACK MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd,msg,wParam,lParam);
}


bool CImageCompiler::Init( HWND inhWnd )
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= 0;
	wcex.hCursor		= 0;
	wcex.hbrBackground	= 0;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= "skeleton";
	wcex.hIconSm		= 0;

	RegisterClassEx(&wcex);

  m_hWnd = CreateWindow(
      "skeleton", "the title",
      WS_OVERLAPPEDWINDOW,
      0, 0, 400, 400,
      NULL, NULL, g_hInst, NULL);

   // return if error on CreateWindow
  if(m_hWnd == NULL)
		return false;

	 // show the window
//  ShowWindow(m_hWnd, SW_SHOWNORMAL);
//  UpdateWindow(m_hWnd);




	InitCommonControls();
	// Initialize DirectDraw
	m_pd3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_pd3d == NULL)
	{
		CCLOG->LogError("Direct3DCreate9 failed");
		return false;
	}

	HRESULT hr;
	D3DDEVTYPE devType;

	ZeroMemory(&m_presentParams, sizeof(m_presentParams));
	m_presentParams.Windowed = TRUE;
	m_presentParams.SwapEffect = D3DSWAPEFFECT_COPY;
	m_presentParams.BackBufferWidth = 256;
	m_presentParams.BackBufferHeight = 256;
	m_presentParams.BackBufferFormat = D3DFMT_UNKNOWN;

	//devType = D3DDEVTYPE_REF; 
	devType = D3DDEVTYPE_HAL; 

	hr = m_pd3d->CreateDevice(D3DADAPTER_DEFAULT, devType,m_hWnd, 
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, &m_presentParams, &m_pd3ddev);
	if (FAILED(hr))
		return false;

	D3DCAPS9 Caps;
	m_pd3ddev->GetDeviceCaps(&Caps);
	if (Caps.PrimitiveMiscCaps & D3DPMISCCAPS_NULLREFERENCE)
		return false;

	return(true);
}





bool CImageCompiler::Load( const char * lpszPathName )
{
	D3DXIMAGE_INFO imageinfo;
	D3DXIMAGE_INFO imageinfo2;

	if( FAILED( D3DXGetImageInfoFromFile( lpszPathName, &imageinfo ) ) )
	{
		CCLOG->LogError("D3DXGetImageInfoFromFile failed");
		return FALSE;
	}

	HRESULT hRes;

	switch( imageinfo.ResourceType )
	{
	case D3DRTYPE_TEXTURE:
		hRes=D3DXCreateTextureFromFileEx( m_pd3ddev, lpszPathName, 
			imageinfo.Width, imageinfo.Height, imageinfo.MipLevels, 0,
			imageinfo.Format, D3DPOOL_DEFAULT, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, 
			&imageinfo2, NULL, (LPDIRECT3DTEXTURE9*)&m_ptexOrig );

		if(FAILED(hRes))
		{
			CCLOG->LogError("D3DXCreateTextureFromFileEx failed (power of two?) '%s'",DXGetErrorString9(hRes));
			return false;
		}
		m_dwOrigWidth = imageinfo2.Width;
		m_dwOrigHeight = imageinfo2.Height;
		m_dwDepth = 0;

		if( imageinfo.ImageFileFormat == D3DXIFF_BMP )
		{
			/* Baustelle			// Look for "foo_a.bmp" for alpha channel
			CString strPath = lpszPathName;
			int i = strPath.ReverseFind('.');
			HRESULT hr;
			strPath = strPath.Left(i) + "_a.bmp";
			CFileStatus status;
			if (CFile::GetStatus(strPath, status))
			{
			// Make sure there's an alpha channel to load alpha image into
			if (FAILED(EnsureAlpha(&m_ptexOrig)))
			return false;

			LPDIRECT3DSURFACE9 psurf;

			hr = ((LPDIRECT3DTEXTURE9)m_ptexOrig)->GetSurfaceLevel(0, &psurf);
			if (FAILED(hr))
			return false;

			hr = LoadAlphaIntoSurface(strPath, psurf);
			ReleasePpo(&psurf);
			if (FAILED(hr))
			return false;
			}
			*/
		}
		break;

	case D3DRTYPE_VOLUMETEXTURE:
		if( FAILED( D3DXCreateVolumeTextureFromFileEx( m_pd3ddev, lpszPathName, 
			imageinfo.Width, imageinfo.Height, imageinfo.Depth, imageinfo.MipLevels,
			0, imageinfo.Format, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE,
			0, &imageinfo2, NULL, (LPDIRECT3DVOLUMETEXTURE9*)&m_ptexOrig ) ) )
		{
			CCLOG->LogError("D3DXCreateVolumeTextureFromFileEx failed");
			return false;
		}
		m_dwOrigWidth = imageinfo2.Width;
		m_dwOrigHeight = imageinfo2.Height;
		m_dwDepth = imageinfo2.Depth;
		break;

	case D3DRTYPE_CUBETEXTURE:
		if( FAILED( D3DXCreateCubeTextureFromFileEx( m_pd3ddev, lpszPathName, 
			imageinfo.Width, imageinfo.MipLevels, 0, imageinfo.Format, 
			D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 
			0, &imageinfo2, NULL, (LPDIRECT3DCUBETEXTURE9*)&m_ptexOrig ) ) )
		{
			CCLOG->LogError("D3DXCreateCubeTextureFromFileEx failed");
			return false;
		}
		m_dwOrigWidth = imageinfo2.Width;
		m_dwOrigHeight = imageinfo2.Height;
		m_dwDepth = 0;
		m_dwCubeMapFlags = DDS_CUBEMAP_ALLFACES;
		break;

	default:
		CCLOG->LogError("ERROR_COULDNTLOADFILE");
		return false;
	}

	return true;
}



LPDIRECT3DBASETEXTURE9 CImageCompiler::CreateUncompressedMipMaps( const DWORD indwFilter, const DWORD indwReduceResolution,
		const bool inbRemoveMips ) 
{
	D3DFORMAT fmt;
	HRESULT hr;
	LPDIRECT3DTEXTURE9 pmiptex = NULL;
	LPDIRECT3DCUBETEXTURE9 pcubetex = NULL;
	LPDIRECT3DVOLUMETEXTURE9 pvoltex = NULL;
	LPDIRECT3DTEXTURE9 pmiptexNew = NULL;
	LPDIRECT3DCUBETEXTURE9 pcubetexNew = NULL;
	LPDIRECT3DVOLUMETEXTURE9 pvoltexNew = NULL;
	LPDIRECT3DSURFACE9 psurfSrc;
	LPDIRECT3DSURFACE9 psurfDest;
	LPDIRECT3DVOLUME9 pvolSrc;
	LPDIRECT3DVOLUME9 pvolDest;

	if (IsVolumeMap())
		pvoltex = (LPDIRECT3DVOLUMETEXTURE9)m_ptexOrig;
	else if (IsCubeMap())
		pcubetex = (LPDIRECT3DCUBETEXTURE9)m_ptexOrig;
	else
		pmiptex = (LPDIRECT3DTEXTURE9)m_ptexOrig;

	if (pvoltex != NULL)
	{
		D3DVOLUME_DESC vd;
		pvoltex->GetLevelDesc(0, &vd);
		fmt = vd.Format;
	}
	else if (pcubetex != NULL)
	{
		D3DSURFACE_DESC sd;
		pcubetex->GetLevelDesc(0, &sd);
		fmt = sd.Format;
	}
	else
	{
		D3DSURFACE_DESC sd;
		pmiptex->GetLevelDesc(0, &sd);
		fmt = sd.Format;
	}

	// if possible do the mipmaps without compression
	{
		int iInputNo=GetNoFromD3DFormat(fmt);

		if(iInputNo!=-1)																										// format is recognized
			fmt=g_pixelformats[GetPixelFormatUncompressed(iInputNo)].DxtNo;		// use an uncompressed format
	}

	DWORD dwW,dwH;

	DWORD dwNumNewMips = max(_CalcMipCount(m_dwOrigWidth,m_dwOrigHeight)-indwReduceResolution,1);
	if(inbRemoveMips)dwNumNewMips=1;

	dwW=max(m_dwOrigWidth>>indwReduceResolution,1);
	dwH=max(m_dwOrigHeight>>indwReduceResolution,1);


	// Create destination mipmap surface - same format as source
	if (pvoltex != NULL)
	{
		if (FAILED(hr = m_pd3ddev->CreateVolumeTexture(dwW, dwH, m_dwDepth, dwNumNewMips, 0, fmt, D3DPOOL_SYSTEMMEM, &pvoltexNew, NULL)))
			{	assert(0);return(0); }

		hr = pvoltex->GetVolumeLevel(0, &pvolSrc);
		hr = pvoltexNew->GetVolumeLevel(0, &pvolDest);
		hr = D3DXLoadVolumeFromVolume(pvolDest, NULL, NULL, pvolSrc, NULL, NULL, 
			indwFilter, 0);
		ReleasePpo(&pvolSrc);
		ReleasePpo(&pvolDest);
		hr = D3DXFilterVolumeTexture(pvoltexNew, NULL, 0, indwFilter);
		assert(pvoltexNew);
		return((LPDIRECT3DBASETEXTURE9)pvoltexNew);
	}
	else if (pmiptex != NULL)
	{
		HRESULT hr = m_pd3ddev->CreateTexture(dwW, dwH, dwNumNewMips, 0, fmt, D3DPOOL_MANAGED, &pmiptexNew, NULL);
		DXERRORCHECK("CreateTexture",hr);
		if (FAILED(hr))
		{	
			CCLOG->LogError("CreateTexture failed (size is not power of two?)");
			assert(0);return(0); 
		}

		hr = pmiptex->GetSurfaceLevel(0, &psurfSrc);
		hr = pmiptexNew->GetSurfaceLevel(0, &psurfDest);
		hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL, psurfSrc, NULL, NULL, 
			indwFilter, 0);
		ReleasePpo(&psurfSrc);
		ReleasePpo(&psurfDest);
		hr = D3DXFilterTexture(pmiptexNew, NULL, 0, indwFilter);
		assert(pmiptexNew);
		return((LPDIRECT3DBASETEXTURE9)pmiptexNew);
	}
	else
	{
		if (FAILED(hr = m_pd3ddev->CreateCubeTexture(dwW, dwNumNewMips, 0, fmt, D3DPOOL_MANAGED, &pcubetexNew, NULL)))
			{	assert(0);return(0); }

		hr = pcubetex->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X, 0, &psurfSrc);
		hr = pcubetexNew->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X, 0, &psurfDest);
		hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL, psurfSrc, NULL, NULL, 
			indwFilter, 0);
		ReleasePpo(&psurfSrc);
		ReleasePpo(&psurfDest);
		hr = pcubetex->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_X, 0, &psurfSrc);
		hr = pcubetexNew->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_X, 0, &psurfDest);
		hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL, psurfSrc, NULL, NULL, 
			indwFilter, 0);
		ReleasePpo(&psurfSrc);
		ReleasePpo(&psurfDest);
		hr = pcubetex->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_Y, 0, &psurfSrc);
		hr = pcubetexNew->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_Y, 0, &psurfDest);
		hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL, psurfSrc, NULL, NULL, 
			indwFilter, 0);
		ReleasePpo(&psurfSrc);
		ReleasePpo(&psurfDest);
		hr = pcubetex->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_Y, 0, &psurfSrc);
		hr = pcubetexNew->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_Y, 0, &psurfDest);
		hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL, psurfSrc, NULL, NULL, 
			indwFilter, 0);
		ReleasePpo(&psurfSrc);
		ReleasePpo(&psurfDest);
		hr = pcubetex->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_Z, 0, &psurfSrc);
		hr = pcubetexNew->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_Z, 0, &psurfDest);
		hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL, psurfSrc, NULL, NULL, 
			indwFilter, 0);
		ReleasePpo(&psurfSrc);
		ReleasePpo(&psurfDest);
		hr = pcubetex->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_Z, 0, &psurfSrc);
		hr = pcubetexNew->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_Z, 0, &psurfDest);
		hr = D3DXLoadSurfaceFromSurface(psurfDest, NULL, NULL, psurfSrc, NULL, NULL, 
			indwFilter, 0);
		ReleasePpo(&psurfSrc);
		ReleasePpo(&psurfDest);
		hr = D3DXFilterCubeTexture(pcubetexNew, NULL, 0, indwFilter);
		assert(pcubetexNew);
		return((LPDIRECT3DBASETEXTURE9)pcubetexNew);
	}
}

CString CImageCompiler::GetSourceFilename( void )
{
	assert(m_pCC);	

	return(CString(m_pCC->sourceFile));
}

DWORD CImageCompiler::_CalcMipCount( const DWORD indwWidth, const DWORD indwHeight )
{
	LONG lwTempH,lwTempW,lwPowsW,lwPowsH;

	lwTempW = indwWidth;
	lwTempH = indwHeight;
	lwPowsW = 0;
	lwPowsH = 0;
	while (lwTempW > 0)
	{
		lwPowsW++;
		lwTempW = lwTempW / 2;
	}
	while (lwTempH > 0)
	{
		lwPowsH++;
		lwTempH = lwTempH / 2;
	}
	return(lwPowsW > lwPowsH ? lwPowsW : lwPowsH);
}



bool CImageCompiler::Compress( LPDIRECT3DBASETEXTURE9 inSrc, D3DFORMAT fmtTo, const DWORD indwFilter )
{
	assert(inSrc);
	if(m_pNewImage) delete m_pNewImage;
	m_pNewImage = NULL;
	return ChangeFormat(inSrc,fmtTo,m_pNewImage,indwFilter,0)==S_OK;
}



HRESULT CImageCompiler::ChangeFormat( LPDIRECT3DBASETEXTURE9 ptexCur, D3DFORMAT fmtTo, ImageObject *&pNewImage,
	const DWORD indwFilter, const DWORD indwReduceResolution )
{
	assert(ptexCur);

	HRESULT hr;
	LPDIRECT3DTEXTURE9 pmiptex;
	LPDIRECT3DCUBETEXTURE9 pcubetex;
	LPDIRECT3DVOLUMETEXTURE9 pvoltex;
	D3DFORMAT fmtFrom;
	LPDIRECT3DTEXTURE9 pmiptexNew;
	LPDIRECT3DCUBETEXTURE9 pcubetexNew;
	LPDIRECT3DVOLUMETEXTURE9 pvoltexNew;

	DWORD dwWidth,dwHeight,dwMips=ptexCur->GetLevelCount();

	if (IsVolumeMap())
	{
		pvoltex = (LPDIRECT3DVOLUMETEXTURE9)ptexCur;
		D3DVOLUME_DESC vd;
		pvoltex->GetLevelDesc(0, &vd);
		fmtFrom = vd.Format;
		dwWidth=vd.Width;dwHeight=vd.Height;
	}
	else if (IsCubeMap())
	{
		pcubetex = (LPDIRECT3DCUBETEXTURE9)ptexCur;
		D3DSURFACE_DESC sd;
		pcubetex->GetLevelDesc(0, &sd);
		fmtFrom = sd.Format;
		dwWidth=sd.Width;dwHeight=sd.Height;
	}
	else
	{
		pmiptex = (LPDIRECT3DTEXTURE9)ptexCur;
		D3DSURFACE_DESC sd;
		pmiptex->GetLevelDesc(0, &sd);
		fmtFrom = sd.Format;
		dwWidth=sd.Width;dwHeight=sd.Height;
	}

	if (fmtFrom == D3DFMT_DXT2 || fmtFrom == D3DFMT_DXT4)
	{
		if (fmtTo == D3DFMT_DXT1)
		{
			CCLOG->LogError("ERROR_PREMULTTODXT1");
		}
		else if (fmtTo != D3DFMT_DXT2 && fmtTo != D3DFMT_DXT4)
		{
			CCLOG->LogError("ERROR_PREMULTALPHA");
			return E_FAIL;
		}
	}

	if (IsVolumeMap())
	{
		hr = m_pd3ddev->CreateVolumeTexture(dwWidth,dwHeight, m_dwDepth, dwMips,	0, fmtTo, D3DPOOL_SYSTEMMEM, &pvoltexNew, NULL);
		if (FAILED(hr))
			return hr;
		pNewImage = new DXImageVol(pvoltexNew);
		if (FAILED(BltAllLevels(D3DCUBEMAP_FACE_FORCE_DWORD, ptexCur, pNewImage,indwFilter,indwReduceResolution, fmtTo)))
			return hr;
	}
	else if (IsCubeMap())
	{
		hr = m_pd3ddev->CreateCubeTexture(dwWidth, dwMips, 0, fmtTo, D3DPOOL_MANAGED, &pcubetexNew, NULL);
		if (FAILED(hr))
			return hr;
		pNewImage = new DXImage(pcubetexNew);
		if (FAILED(hr = BltAllLevels(D3DCUBEMAP_FACE_NEGATIVE_X, ptexCur, pNewImage,indwFilter,indwReduceResolution, fmtTo)))
			return hr;
		if (FAILED(hr = BltAllLevels(D3DCUBEMAP_FACE_POSITIVE_X, ptexCur, pNewImage,indwFilter,indwReduceResolution, fmtTo)))
			return hr;
		if (FAILED(hr = BltAllLevels(D3DCUBEMAP_FACE_NEGATIVE_Y, ptexCur, pNewImage,indwFilter,indwReduceResolution, fmtTo)))
			return hr;
		if (FAILED(hr = BltAllLevels(D3DCUBEMAP_FACE_POSITIVE_Y, ptexCur, pNewImage,indwFilter,indwReduceResolution, fmtTo)))
			return hr;
		if (FAILED(hr = BltAllLevels(D3DCUBEMAP_FACE_NEGATIVE_Z, ptexCur, pNewImage,indwFilter,indwReduceResolution, fmtTo)))
			return hr;
		if (FAILED(hr = BltAllLevels(D3DCUBEMAP_FACE_POSITIVE_Z, ptexCur, pNewImage,indwFilter,indwReduceResolution, fmtTo)))
			return hr;
	}
	else
	{
		if(fmtTo==D3DFMT_P8)
		{
			pNewImage = new P8Image(dwWidth, dwHeight, dwMips);
		}
		else
		{
			if ((fmtTo == D3DFMT_DXT1 || fmtTo == D3DFMT_DXT2 ||
				fmtTo == D3DFMT_DXT3 || fmtTo == D3DFMT_DXT4 ||
				fmtTo == D3DFMT_DXT5) && (m_dwOrigWidth % 4 != 0 || m_dwOrigHeight % 4 != 0))
			{
				CCLOG->LogError("ERROR_NEEDMULTOF4 = for DXT compression we need width and height to be multiple of 4");
				return E_FAIL;
			}

			hr = m_pd3ddev->CreateTexture(dwWidth,dwHeight, dwMips, 0, fmtTo, D3DPOOL_MANAGED, &pmiptexNew, NULL);
			if (FAILED(hr))
			{
				CCLOG->LogError("CreateTexture failed (texture format not supported by driver)");
				return hr;
			}
			pNewImage = new DXImage(pmiptexNew);
		};
		if (FAILED(BltAllLevels(D3DCUBEMAP_FACE_FORCE_DWORD, ptexCur, pNewImage,indwFilter,indwReduceResolution, fmtTo)))
			return hr;
	}
	return S_OK;
}

HRESULT CImageCompiler::BltAllLevels( D3DCUBEMAP_FACES FaceType, LPDIRECT3DBASETEXTURE9 ptexSrc,
	ImageObject *pNewImage, const DWORD indwFilter, const DWORD indwReduceResolution, D3DFORMAT fmtTo )
{
	DWORD dwReduceRes=indwReduceResolution;
//	DWORD dwMips=_CalcMipCount(m_dwOrigWidth,m_dwOrigHeight)-dwReduceRes;
	DWORD dwMips=ptexSrc->GetLevelCount();
	
	for (DWORD iLevel = 0; iLevel < dwMips; iLevel++)
	{
		if (IsVolumeMap())
		{
			LPDIRECT3DVOLUMETEXTURE9 pvoltexSrc = (LPDIRECT3DVOLUMETEXTURE9)ptexSrc;
			LPDIRECT3DVOLUME9 pvolSrc = NULL;
			DXERRORCHECK("GetVolumeLevel",pvoltexSrc->GetVolumeLevel(iLevel+indwReduceResolution, &pvolSrc));
			DXERRORCHECK("Convert1",pNewImage->Convert((LPDIRECT3DSURFACE9)pvolSrc, iLevel, indwFilter, FaceType, m_pd3ddev));
			ReleasePpo(&pvolSrc);
		}
		else if (IsCubeMap())
		{
			LPDIRECT3DCUBETEXTURE9 pcubetexSrc = (LPDIRECT3DCUBETEXTURE9)ptexSrc;
			LPDIRECT3DSURFACE9 psurfSrc = NULL;
			DXERRORCHECK("GetCubeMapSurface",pcubetexSrc->GetCubeMapSurface(FaceType, iLevel+indwReduceResolution, &psurfSrc));
			DXERRORCHECK("Convert2",pNewImage->Convert(psurfSrc, iLevel, indwFilter, FaceType, m_pd3ddev));
			ReleasePpo(&psurfSrc);
		}
		else
		{
			LPDIRECT3DTEXTURE9 pmiptexSrc = (LPDIRECT3DTEXTURE9)ptexSrc;
			LPDIRECT3DSURFACE9 psurfSrc = NULL;
			DXERRORCHECK("GetSurfaceLevel",pmiptexSrc->GetSurfaceLevel(iLevel+indwReduceResolution, &psurfSrc));

			{ 
				HRESULT _hr = pNewImage->Convert(psurfSrc, iLevel, indwFilter, FaceType, m_pd3ddev); 
				if(FAILED(_hr))
					CCLOG->LogError("'%s' DX ERROR: %s", "Convert3",DXGetErrorString9A(_hr));
			}

			ReleasePpo(&psurfSrc);
		}
	};
	
	return S_OK;
}

void CImageCompiler::Release( void )
{
	delete this;
}


int CImageCompiler::GetIntParam( CString insName, const int iniDefault ) const
{
	int iRet=iniDefault;		
	CString sValue;

	if(m_pCC->config->Get(insName,sValue))
		sscanf(sValue.GetBuffer(),"%d",&iRet);

	CCLOG->Log("CImageCompiler::%s=%d",insName.GetBuffer(),iRet);

	return(iRet);
}

bool CImageCompiler::GetBoolParam( CString insName, const bool inbDefault ) const
{
	return( GetIntParam(insName,inbDefault?1:0)!=0 );
}


// Process file. Return error code or 0 if successful
bool CImageCompiler::Process( ConvertContext &cc )
{
	m_pCC=&cc;

	// this would fit much better in the Init() method - but unfortunatly we don't have access to CCLOG there
	static bool bFirstTime=true;
	if(bFirstTime)											// print the supported formats
	{
		CCLOG->Log("");
		CCLOG->Log("ResourceCompilerImage supported output pixel formats:");

		int iCount=GetPixelFormatCount();

		for(int i=0;i<iCount;i++)
		{
			CCLOG->Log(" * %s",g_pixelformats[i].szName);
			CCLOG->Log("   %s",g_pixelformats[i].szDescription);
		}
		CCLOG->Log("");

		bFirstTime=false;
	}


	// highest to lowest priority:
	//
	// ranger_statue._rc  rc.ini  commandline
	CString sourceFile = cc.getSourcePath();

	if(Load(sourceFile))
	{
		if(!LoadConfigFile())
			return false;			// error
			
		SetPresetSettings();

		if(m_Props.m_bUserDialog)
		{
			CUserDialog dialog;

			// user dialog should show only the first time - if not specified differently
			{
				int sValue;

				if(!m_pCC->config->Get("userdialog",sValue))
					m_Props.m_bUserDialog=false;
			}
			
			dialog.DoModal(this);
		}
		else RunWithProperties(true);			// run and save

	}
	else CCLOG->LogError("CImageCompiler::Process load '%s' failed",(const char*)sourceFile );

	m_pCC=0;

	return true;		// success
}


bool CImageCompiler::RunWithProperties( const bool inbSave )
{
	if(m_pNewImage) delete m_pNewImage;
	m_pNewImage = NULL;

	DWORD dwFilter=D3DX_FILTER_TRIANGLE;

	if(m_Props.m_bMipMirror) dwFilter|=D3DX_FILTER_MIRROR;

	if(m_Props.m_bMipmaps || m_Props.m_dwReduceResolution)					// generate mipmaps
	{
		// with mipmaps
		LPDIRECT3DBASETEXTURE9 pSrc=CreateUncompressedMipMaps(dwFilter,m_Props.m_dwReduceResolution,!m_Props.m_bMipmaps);

		assert(pSrc);

		if(pSrc)
		{
			if(!Compress(pSrc,g_pixelformats[m_Props.m_iDestPixelFormat].DxtNo,dwFilter))
				{	ReleasePpo(&pSrc);return(false); }
		}

		ReleasePpo(&pSrc);
	}
	else
	{
		// without downsampling
		if(!Compress(m_ptexOrig,g_pixelformats[m_Props.m_iDestPixelFormat].DxtNo,dwFilter))
			return(false);
	}


	if(inbSave)
	{
		CString outputFile = m_pCC->getOutputPath();
		if(Save(outputFile))
		{
			CCLOG->Log("CImageCompiler::Process save '%s'",(const char*)outputFile );
		}
		else
		{
			CCLOG->LogError("CImageCompiler::Process load '%s' failed",(const char*)outputFile );
			return(false);
		}
	}

	return(true);
}



int CImageCompiler::GetNoFromName( const char *inszName )
{
	int iCount=GetPixelFormatCount();

	for(int i=0;i<iCount;i++)
		if(stricmp(g_pixelformats[i].szName,inszName)==0)
			return(i);

	return(-1);
}



int CImageCompiler::GetNoFromD3DFormat( D3DFORMAT inFormat )
{
	int iCount=GetPixelFormatCount();

	for(int i=0;i<iCount;i++)
		if(g_pixelformats[i].DxtNo==inFormat)
			return(i);

	return(-1);
}


char *CImageCompiler::GetPixelFormatName( const int iniNo )
{
	assert(iniNo>=0);
	assert(iniNo<GetPixelFormatCount());

	return(g_pixelformats[iniNo].szName);
}


int CImageCompiler::GetPixelFormatUncompressed( const int iniNo )
{
	assert(iniNo>=0);
	assert(iniNo<GetPixelFormatCount());

	int iRet=GetNoFromName(g_pixelformats[iniNo].szUncompressedAltName);

	assert(iRet!=-1);			// szUncompressedAltName must be in szName

	return(iRet);
}


int CImageCompiler::GetPixelFormatCount( void )
{
	return(sizeof(g_pixelformats)/sizeof(SPixelFormats));
}


char *CImageCompiler::GetPixelFormatDesc( const int iniNo )
{
	assert(iniNo>=0);
	assert(iniNo<GetPixelFormatCount());

	return(g_pixelformats[iniNo].szDescription);
}


CString CImageCompiler::GetInfoString( const bool inbOrig )
{
	int iNo=inbOrig?m_iOrigPixelFormatNo:m_Props.m_iDestPixelFormat;

	char str[256];

	int iMem=CalcTextureMemory(inbOrig);

	if(inbOrig)
		sprintf(str,"W:%d  H:%d  Alpha:%s\nMem:%.1fkB",m_dwOrigWidth,m_dwOrigHeight,g_pixelformats[iNo].szAlpha,iMem/1024.0f);
	else 
	{
		DWORD dwW=max(m_dwOrigWidth>>m_Props.m_dwReduceResolution,1);
		DWORD dwH=max(m_dwOrigHeight>>m_Props.m_dwReduceResolution,1);
		DWORD dwNumMips=max((int)_CalcMipCount(m_dwOrigWidth,m_dwOrigHeight)-(int)m_Props.m_dwReduceResolution,1);
		if(!m_Props.m_bMipmaps)dwNumMips=1;

		sprintf(str,"W:%d  H:%d  Alpha:%s  Mips:%d\nMem:%.1fkB",dwW,dwH,g_pixelformats[iNo].szAlpha,dwNumMips,iMem/1024.0f);
	}

	return(str);
}


DWORD CImageCompiler::CalcTextureMemory( const bool inbOrig ) const
{
	int iSum=0;
	DWORD iW=m_dwOrigWidth;
	DWORD iH=m_dwOrigHeight;
	int iNo=inbOrig?m_iOrigPixelFormatNo:m_Props.m_iDestPixelFormat;
	int iBpp=g_pixelformats[iNo].iBitsPerPixel;
	int iMipmaps=1;
	
	if(!inbOrig && m_Props.m_bMipmaps)iMipmaps=_CalcMipCount(m_dwOrigWidth,m_dwOrigHeight);

	int iIgnoreTopMips=0;
	
	if(!inbOrig)iIgnoreTopMips=(int)m_Props.m_dwReduceResolution;

	for(int i=0;i<iMipmaps;i++)
	{
		if(iIgnoreTopMips)iIgnoreTopMips--;
		else
		{
			if(g_pixelformats[iNo].bCompressed)
			{
				iSum+=max(1,iW/4) * max(1,iH/4) * iBpp;			// from DX SDK:  max(1,width ÷ 4)x max(1,height ÷ 4)x 8 (DXT1) or 16 (DXT2-5)
			}
			else
			{
				iSum+=(iW*iH*iBpp)/8;
			}
		}

		iW=(iW+1)/2; iH=(iH+1)/2;
	}

	return(iSum);
}






bool CImageCompiler::GetOutputFile( ConvertContext& cc )
{
	//specify output path
	cc.outputFile = Path::ReplaceExtension( cc.sourceFile,"dds" );
	return true;
}


int CImageCompiler::GetNumPlatforms() const
{
	return(4);
}


Platform CImageCompiler::GetPlatform( int index ) const
{
	switch (index)
	{
		case 0:	return PLATFORM_PC;
		case 1:	return PLATFORM_XBOX;
			//case 2:	return PLATFORM_PS2;
			//case 3:	return PLATFORM_GAMECUBE;
	};
	//assert(0);
	return PLATFORM_UNKNOWN;
}


int CImageCompiler::GetNumExt() const
{
	return(6);
}


const char* CImageCompiler::GetExt( int index ) const
{
	switch (index)
	{
		// DirectX supports bmp dds dib jpg png tga
		case 0:	return("bmp");
		case 1:	return("dds");
		case 2:	return("dib");
		case 3:	return("jpg");
		case 4:	return("png");
		case 5:	return("tga");
	};

	//assert(0);
	return(0);
}


// this should retrieve the timestamp of the convertor executable:
// when it was created by the linker, normally. This date/time is used to
// compare with the compiled file date/time and even if the compiled file
// is not older than the source file, it will be recompiled if it's older than the
// convertor
DWORD CImageCompiler::GetTimestamp() const
{
	return GetTimestampForLoadedLibrary(g_hInst);
}



struct my_vertex
{
    FLOAT x, y, z,rhw;
    FLOAT u1, v1;
};

// Define corresponding FVF macro.
#define D3D8T_CUSTOMVERTEX ( D3DFVF_XYZRHW|D3DFVF_TEX1|D3DFVF_TEXCOORDSIZE2(0))


static void PrintInPreview( HWND inHWND, RECT &inRect, const char *inszTxt1, const char *inszTxt2=0 )
{
	HDC hdc=GetDC(inHWND);
	FillRect(hdc,&inRect,GetSysColorBrush(COLOR_3DFACE));
	SetBkMode(hdc,TRANSPARENT);
	SetTextAlign(hdc,TA_CENTER);
	ExtTextOut(hdc,(inRect.left+inRect.right)/2,(inRect.top+inRect.bottom)/2-8,ETO_CLIPPED,&inRect,inszTxt1,(int)strlen(inszTxt1),0);
	if(inszTxt2)ExtTextOut(hdc,(inRect.left+inRect.right)/2,(inRect.top+inRect.bottom)/2+8,ETO_CLIPPED,&inRect,inszTxt2,(int)strlen(inszTxt2),0);
	ReleaseDC(inHWND,hdc);
}



bool CImageCompiler::BlitTo( HWND inHWND, RECT &inRect, const float infOffsetX, const float infOffsetY, const int iniScale16, const bool inbOrig )
{
	HRESULT hr;
	IDirect3DTexture9 *pTexture=0;

	if(!IsPowerOfTwo(m_dwOrigWidth) || !IsPowerOfTwo(m_dwOrigHeight))
	{
		char str[256];

		sprintf(str,"(%dx%d)",m_dwOrigWidth,m_dwOrigHeight);
		PrintInPreview(inHWND,inRect,"Image size is not power of two",str);
		return(true);
	}

	// check if P8 format which DX can't render, and convert to XRGB
	if(g_pixelformats[m_Props.m_iDestPixelFormat].DxtNo==D3DFMT_P8 && !inbOrig)
	{	
		assert(m_pNewImage);

		if(!m_pNewImage)
		{
			PrintInPreview(inHWND,inRect,"Conversion failed","(format not supported by driver?)");
			return(true);
		}

		pTexture = m_pNewImage->CopyToXRGB(m_pd3ddev);	
	}
	else
	{
		// error message is neccessary
		if(inbOrig)
		{
			if(!m_ptexOrig)
			{
				PrintInPreview(inHWND,inRect,"Load failed","(format not supported by driver?)");
				return(true);
			}
		}
		else
		{
			if(m_pNewImage==0 || m_pNewImage->GetDXTex()==0)
			{
				PrintInPreview(inHWND,inRect,"Conversion failed","(format not supported by driver?)");
				return(true);
			}
		}
		// convert to blitable format
		IDirect3DBaseTexture9 *pBaseTexture = inbOrig ? m_ptexOrig : m_pNewImage->GetDXTex();
		
		assert(pBaseTexture);			// this was checked earlier

		ImageObject *pimgNew = NULL;

		if(m_Props.m_bPreviewAlpha)		// preview alpha
		{
			// convert to format with alpha
			if(FAILED(hr = ChangeFormat(pBaseTexture,D3DFMT_A8R8G8B8,pimgNew,D3DX_FILTER_NONE,0)))
				return(false);

			pimgNew->GetDXTexI(pTexture);

			assert(pTexture);

			// for all mipmaps
			for(unsigned int mip = 0; mip<pTexture->GetLevelCount(); mip++)
			{
				IDirect3DSurface9 *pSurfaceLevel=0;

				// convert alpha to gray
				if(pTexture->GetSurfaceLevel(mip,&pSurfaceLevel)==D3D_OK)
				{	
					CopyAlphaToRGB(pSurfaceLevel);

					pSurfaceLevel->Release();pSurfaceLevel=0;
				}
			};

			ReleasePpo(&pTexture);

			// convert to blitable format
			if(FAILED(hr = ChangeFormat(pimgNew->GetDXTex(),D3DFMT_X8R8G8B8,pimgNew,D3DX_FILTER_NONE,0)))
				return(false);

			pimgNew->GetDXTexI(pTexture);
		}
		else
		{
			if(FAILED(hr = ChangeFormat(pBaseTexture,D3DFMT_X8R8G8B8,pimgNew,D3DX_FILTER_NONE,0)))
				return(false);

			if(pimgNew) pimgNew->GetDXTexI(pTexture);
		}

		if(pimgNew) delete pimgNew;
	}

	if(!pTexture)
		return(false);

	int iWidth=inRect.right-inRect.left,iHeight=inRect.bottom-inRect.top;

	int iW=((iWidth)*16)/iniScale16;	
	int iH=((iHeight)*16)/iniScale16;

	int iX=(int)(infOffsetX*GetWidth()*1.0f);
	int iY=(int)(infOffsetY*GetHeight()*1.0f);

	m_pd3ddev->BeginScene();

//	m_pd3ddev->Clear(0,0,D3DCLEAR_TARGET,0xff0000,0.0f,0);

	hr=m_pd3ddev->SetTexture(0,pTexture);

	if (FAILED(hr))
	{
		CCLOG->LogError("SetTexture failed");
		return false;
	}


	if(m_Props.m_bPreviewFiltered)
	{
		m_pd3ddev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_LINEAR);
		m_pd3ddev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_LINEAR);
	}
	else
	{
		m_pd3ddev->SetSamplerState(0,D3DSAMP_MAGFILTER,D3DTEXF_POINT);
		m_pd3ddev->SetSamplerState(0,D3DSAMP_MINFILTER,D3DTEXF_POINT);
	}



	m_pd3ddev->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTEXF_POINT);

	m_pd3ddev->SetSamplerState(0,D3DSAMP_MIPFILTER,D3DTADDRESS_BORDER);

	if(m_Props.m_bPreviewTiled)
	{
		m_pd3ddev->SetSamplerState(0, D3DSAMP_ADDRESSU,	D3DTADDRESS_WRAP);
		m_pd3ddev->SetSamplerState(0, D3DSAMP_ADDRESSV,	D3DTADDRESS_WRAP);
	}
	else
	{
		m_pd3ddev->SetSamplerState(0, D3DSAMP_ADDRESSU,	D3DTADDRESS_BORDER);
		m_pd3ddev->SetSamplerState(0, D3DSAMP_ADDRESSV,	D3DTADDRESS_BORDER);
	}



	// Rendering of scene objects happens here.

	m_pd3ddev->SetVertexShader(NULL);
	m_pd3ddev->SetFVF( D3D8T_CUSTOMVERTEX );

	
	int iScaleX=(int)(iWidth*16.0f/iniScale16*0.5f);
	int iScaleY=(int)(iHeight*16.0f/iniScale16*0.5f);

	// Create vertex data with position and texture coordinates.
	my_vertex g_triangle_vertices[]=
	{
			//  x      y       z     rhw  u1  v1
			{ -0.5f,   -0.5f,   0.5f,  1,   (float)(-iScaleX+iX)/GetWidth(),  (float)(-iScaleY+iY)/GetHeight(), }, 
			{ 255.5f, -0.5f,   0.5f,  1,   (float)(iScaleX+iX)/GetWidth(),   (float)(-iScaleY+iY)/GetHeight(), }, 
			{ 255.5f, 255.5f, 0.5f,  1,   (float)(iScaleX+iX)/GetWidth(),   (float)(iScaleY+iY)/GetHeight(), }, 
			{ -0.5f,   255.5f, 0.5f,  1,   (float)(-iScaleX+iX)/GetWidth(),  (float)(iScaleY+iY)/GetHeight(), }, 
	};



	m_pd3ddev->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, g_triangle_vertices, sizeof(my_vertex));
	m_pd3ddev->SetTexture(0,0);

// End the scene.
	m_pd3ddev->EndScene();

	hr = m_pd3ddev->Present( NULL, &inRect, inHWND, 0 );

	if (FAILED(hr))
	{
		//check if device was lost
		if (hr == D3DERR_DEVICELOST)
		{
			CCLOG->LogError("Device Lost");

			m_pd3ddev->Reset(&m_presentParams);
		}
	}


/*
		for(int y=0;y<iHeight;y++)
		for(int x=0;x<iWidth;x++)
		{
			SetPixel(inHdc,inRect.left+x,inRect.top+y,  GetPixel(dc,x*(iW)/iWidth+iX-iW/2,y*(iH)/iHeight+iY-iH/2)  );
		}
*/
//		BOOL ok=StretchBlt(inHdc,	inRect.left,inRect.top,iWidth,iHeight,
//											 dc,		iX-iW,iY-iH,						 iW*2,iH*2,  SRCCOPY);
//		assert(ok);

//		pSurfaceLevel->ReleaseDC(dc);

//	}

	ReleasePpo(&pTexture);
	return(true);
}


bool CImageCompiler::ClampBlitOffset( const int iniWidth, const int iniHeight, float &inoutfX, float &inoutfY, const int iniScale16 ) const
{
	bool bRet=true;

	if( iniWidth >= (int)(m_dwOrigWidth*iniScale16)/16 ) inoutfX=0.5f;
	else
	{
		float fXMin=(float)((iniWidth*16)/iniScale16)/(float)m_dwOrigWidth*0.5f;
		float fXMax=1.0f-fXMin;

		if(inoutfX<fXMin)inoutfX=fXMin;
		if(inoutfX>fXMax)inoutfX=fXMax;

		bRet=false;
	}

	if( iniHeight >= (int)(m_dwOrigHeight*iniScale16)/16 ) inoutfY=0.5f;
	else
	{
		float fYMin=(float)((iniHeight*16)/iniScale16)/(float)m_dwOrigHeight*0.5f;
		float fYMax=1.0f-fYMin;

		if(inoutfY<fYMin)inoutfY=fYMin;
		if(inoutfY>fYMax)inoutfY=fYMax;

		bRet=false;
	}

	return(bRet);
}




DWORD CImageCompiler::GetWidth( void ) const
{
	return(m_dwOrigWidth);
}


DWORD CImageCompiler::GetHeight( void ) const
{
	return(m_dwOrigHeight);
}







bool CImageCompiler::CopyAlphaToRGB( LPDIRECT3DSURFACE9 psurf ) const
{
  HRESULT hr;
  D3DSURFACE_DESC sd;
  LPDIRECT3DSURFACE9 psurfTarget;

  hr=psurf->GetDesc(&sd);
	assert(hr==D3D_OK);

  // Copy the target surface into an A8R8G8B8 surface
  hr = m_pd3ddev->CreateOffscreenPlainSurface(sd.Width, sd.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &psurfTarget, NULL);
	assert(hr==D3D_OK);
  hr = D3DXLoadSurfaceFromSurface(psurfTarget, NULL, NULL, psurf, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
	assert(hr==D3D_OK);

  D3DLOCKED_RECT lrDest;

  hr = psurfTarget->LockRect(&lrDest, NULL, 0);
	assert(hr==D3D_OK);

  DWORD xp;
  DWORD yp;
	DWORD* pdwRow = (DWORD*)lrDest.pBits;
  DWORD* pdwPtr;
  LONG dataBytesPerRow = 4 * sd.Width;

  for (yp = 0; yp < sd.Height; yp++)
  {
      pdwPtr = pdwRow;
      for (xp = 0; xp < sd.Width; xp++)
      {
				  DWORD dwAlpha= (*pdwPtr) >> 24;
			
					*pdwPtr = dwAlpha | (dwAlpha<<8) | (dwAlpha<<16);

          pdwPtr++;
      }
      pdwRow += lrDest.Pitch / 4;
  }

  psurfTarget->UnlockRect();

  // Copy psurfTarget back into real surface
  hr = D3DXLoadSurfaceFromSurface(psurf, NULL, NULL, psurfTarget, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
  assert(hr==D3D_OK);

  // Release allocated interfaces
  ReleasePpo(&psurfTarget);

  return S_OK;
}

bool CImageCompiler::IsPowerOfTwo( const DWORD indwValue )
{
	return((indwValue&(indwValue-1)) == 0);
}

// set the stored properties to the current file and save it
bool CImageCompiler::LoadConfigFile( void )
{
	assert(m_pCC);

	CString sPixelformat="UNKNOWN";						

	// pixelformat
	if(m_pCC->config->Get("pixelformat",sPixelformat))
	{
		int iNo=GetNoFromName(sPixelformat.GetBuffer());	

		if(iNo==-1)
		{ 
			CCLOG->LogError("CImageCompiler::Process pixelformat '%s' not recognized",sPixelformat.GetBuffer());
			return(false); 
		}

		m_Props.m_iDestPixelFormat=iNo;

		CCLOG->Log("CImageCompiler::pixelformat='%s'",sPixelformat.GetBuffer());
	}

	// ---------------------------

	m_Props.m_bMipmaps=GetBoolParam("mipmaps",true);
	m_Props.m_bMipMirror=GetBoolParam("mipmirror",true);
	m_Props.m_bUserDialog=GetBoolParam("userdialog",true);
	m_Props.m_dwReduceResolution=max(GetIntParam("reduce",0),0);
	m_Props.m_dwDitherMode=max(GetIntParam("dither",0),0);
	
	if(!m_pCC->config->Get("preset",m_Props.m_sPreset)) m_Props.m_sPreset = "";

	return(true);
} 


void CImageCompiler::SetPresetSettings()
{
	if(m_Props.m_sPreset=="") return;
	m_pCC->config->Set("preset", m_Props.m_sPreset);
	m_pCC->presets->SetConfig(m_Props.m_sPreset, m_pCC->config);
	LoadConfigFile();
};


// set the stored properties to the current file and save it
bool CImageCompiler::UpdateAndSaveConfigFile( void )
{
	m_pCC->pFileSpecificConfig->UpdateOrCreateEntry("","preset",m_Props.m_sPreset);
	// bools

	m_pCC->pFileSpecificConfig->UpdateOrCreateEntry("","pixelformat",g_pixelformats[m_Props.m_iDestPixelFormat].szName);
	m_pCC->pFileSpecificConfig->UpdateOrCreateEntry("","mipmaps",m_Props.m_bMipmaps?"1":"0");
	m_pCC->pFileSpecificConfig->UpdateOrCreateEntry("","mipmirror",m_Props.m_bMipMirror?"1":"0");
	m_pCC->pFileSpecificConfig->UpdateOrCreateEntry("","userdialog",m_Props.m_bUserDialog?"1":"0");

	// DWORDs
	char str[256];

	sprintf(str,"%d",m_Props.m_dwReduceResolution);
	m_pCC->pFileSpecificConfig->UpdateOrCreateEntry("","reduce",str);

	sprintf(str,"%d",m_Props.m_dwDitherMode);
	m_pCC->pFileSpecificConfig->UpdateOrCreateEntry("","dither",str);

	return(m_pCC->pFileSpecificConfig->Save());
} 






