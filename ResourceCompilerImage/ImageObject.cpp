//

#include "stdafx.h"
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

HRESULT P8Image::Convert(LPDIRECT3DSURFACE9 psurfSrc, int mip, int filter, D3DCUBEMAP_FACES facetype, LPDIRECT3DDEVICE9 pd3ddev)
{
	assert(psurfSrc);

	D3DSURFACE_DESC sd;
	psurfSrc->GetDesc(&sd);

	LPDIRECT3DSURFACE9 psurfTarget;
	HRESULT hr = pd3ddev->CreateOffscreenPlainSurface(sd.Width, sd.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &psurfTarget, NULL);
	if(FAILED(hr)) return hr;
	hr = D3DXLoadSurfaceFromSurface(psurfTarget, NULL, NULL, psurfSrc, NULL, NULL, D3DX_FILTER_TRIANGLE, 0);
	if(FAILED(hr)) return hr;

	unsigned char *pic = (unsigned char*)malloc(3*sd.Width*sd.Height+1);
	assert(pic);

	{
		D3DLOCKED_RECT lrTarg;
		hr = psurfTarget->LockRect(&lrTarg, NULL, 0);
		if(FAILED(hr)) return hr;
		
		DWORD* pdwRowTarg = (DWORD*)lrTarg.pBits;
		unsigned char *p = pic;

		for (DWORD yp = 0; yp < sd.Height; yp++)
		{
			DWORD* pdwPtrTarg = pdwRowTarg;
			for (DWORD xp = 0; xp < sd.Width; xp++)
			{
				
				*((DWORD *)p) = *pdwPtrTarg++;	// both use BGR order
				p += 3;							// quick and dirty: only works correctly on x86 cpus
			}
			pdwRowTarg += lrTarg.Pitch / 4;
		};
		
		psurfTarget->UnlockRect();
	};

	ReleasePpo(&psurfTarget);

	if(!m_pPalette)	// FIXME: for now, generate palette based solely on main image, may give better result if mipmaps are taken into account
	{
		initnet(pic, 3*sd.Width*sd.Height, 1);		// can use 1 instead of 10 for gold master :)

		learn();
		unbiasnet();
		
		m_pPalette = (unsigned char *)malloc(256*3);
		writecolourmap(m_pPalette);

		inxbuild();
	};
	
	{		
		unsigned char *d = (unsigned char*)malloc(sd.Width*sd.Height);
		assert(d);
		m_ppIndices[mip] = d;

		unsigned char *p = pic;

		for (unsigned int yp = 0; yp < sd.Height; yp++)
		{
			for (unsigned int xp = 0; xp < sd.Width; xp++)
			{
				*d++ = inxsearch(p[0], p[1], p[2]);
				p += 3;	
			}
		};
	};

	free(pic);

	return S_OK;
};

bool P8Image::Save(const char *name)
{
	// FIXME: do this extension hacking somewhere else, it is not appropriate here
	CString filename = name;
	filename.Truncate(filename.GetLength()-4);
	filename += ".tga";

	FILE *fh = fopen(filename.GetString(), "wb");
	if(!fh) return false;
	
	SavePaletizedTextureHeader(fh);

/*
	for(unsigned int mip = 0; mip<tex->GetLevelCount(); mip++)
	{
		LPDIRECT3DSURFACE9 psurf = NULL;
		DXERRORCHECK(tex->GetSurfaceLevel(mip, &psurf));
		D3DSURFACE_DESC sd;
		psurf->GetDesc(&sd);

		D3DLOCKED_RECT lr;
		DXERRORCHECK(psurf->LockRect(&lr, NULL, 0));
		
		SavePaletizedTextureMip(fh, (unsigned char *)lr.pBits, sd.Width, sd.Height, lr.Pitch, mip);

		psurf->UnlockRect();

		ReleasePpo(&psurf);
	};
	*/
	fclose(fh);
	
	return true;
};

void P8Image::SavePaletizedTextureHeader(FILE *fh)
{
	/*
	// DDS P8 appearently not supported
	fwrite("DDS ", 4, 1, fh);
	DDSURFACEDESC2 sd2 =
	{
		124,
		DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PITCH,
		sd.Height,
		sd.Width,
		(sd.Width+3)&3,	
		0,
		tex->GetLevelCount(),
		0,0,0,0,0,0,0,0,0,0,0,
		{ 32, DDPF_PALETTEINDEXED8, 0, 8, 0, 0, 0, 0 }, 
		{ DDSCAPS_TEXTURE | DDSCAPS_MIPMAP | DDSCAPS_COMPLEX, 0, 0, 0 },
		0
	};
	*/

	unsigned char tgaheader[] =
	{
		0, 1, 1,
		0, 0, 0, 1,
		24,
		0, 0, 0, 0,
		(m_nWidth  & 0x00FF), (m_nWidth  & 0xFF00) / 256,
		(m_nHeight & 0x00FF), (m_nHeight & 0xFF00) / 256,
		8, 0,
	};
	
	fwrite(&tgaheader, 18, 1, fh);
	fwrite(m_pPalette, 3*256, 1, fh);
	
};

void P8Image::SavePaletizedTextureMip(FILE *fh, unsigned char *buf, int mip)
{
	// if this line is commented, it will save an "extended TGA" with mipmaps following the main picture
	//if(mip) return;

	for(int y = m_nHeight-1; y>=0; y--)
	{
		fwrite(buf+y*m_nWidth, 1, m_nWidth, fh);
		//if(xs&3) fwrite("    ", 4-(xs&3), 1, fh);	// dword align for dds
	};
};

IDirect3DTexture9 *P8Image::CopyToXRGB(LPDIRECT3DDEVICE9 pd3ddev)
{
	assert(m_pPalette);

	IDirect3DTexture9 *ptexNew = NULL;

	if(FAILED(pd3ddev->CreateTexture(m_nWidth, m_nHeight, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &ptexNew, NULL))) return NULL;

	LPDIRECT3DSURFACE9 psurfd = NULL;
	if(FAILED(ptexNew->GetSurfaceLevel(0, &psurfd))) return NULL;

	D3DLOCKED_RECT lrd;
	if(FAILED(psurfd->LockRect(&lrd, NULL, 0))) return NULL;
	
	for(int y = m_nHeight-1; y>=0; y--)
	{
		for(int x = 0; x<m_nHeight; x++)
		{
			int index = *(m_ppIndices[0]+y*m_nWidth+x);
			unsigned char *rgb = m_pPalette+index*3;
			unsigned char *dest = ((unsigned char *)lrd.pBits)+y*lrd.Pitch+x*4;
			dest[0] = rgb[0];						
			dest[1] = rgb[1];						
			dest[2] = rgb[2];						
			dest[3] = 0xFF;						
		};
	};

	psurfd->UnlockRect();
	ReleasePpo(&psurfd);	
	return ptexNew;
};
