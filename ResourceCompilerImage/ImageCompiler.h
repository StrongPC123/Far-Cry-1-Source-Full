#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include "dds.h"								//

#include "IConvertor.h"					// IConvertor

#include "Properties.h"					// CProperties

struct ImageObject;							// forward decl

class CImageCompiler : public IConvertor
{
public:

	//! constructor
	CImageCompiler();
	//! destructor
	~CImageCompiler();

	//!
	bool Init( HWND inhWnd );
	//!
	void DeInit( void );
	//!
	bool Load( const char *lpszPathName );

	//!
	bool Save( const char *lpszPathName );

	//! /param indwFilter D3DX_FILTER bitmask (see DirectX documentation): e.g. D3DX_FILTER_POINT|D3DX_FILTER_MIRROR, D3DX_FILTER_TRIANGLE
	//! /return 
	LPDIRECT3DBASETEXTURE9 CreateUncompressedMipMaps( const DWORD indwFilter, const DWORD indwReduceResolution,
		const bool inbRemoveMips );

	//!
	bool Compress( LPDIRECT3DBASETEXTURE9 inSrc, D3DFORMAT fmtTo, const DWORD indwFilter );

	//! /param infOffsetX
	//! /param infOffsetY
	//! /param iniScale16 16=1:1, bigger values magnify more
	//! /return true=blit was successful, false otherwise
	bool BlitTo( HWND inHWND, RECT &inRect, const float infOffsetX, const float infOffsetY, const int iniScale16, const bool inbOrig );

	//! textre for the info below the preview
	CString GetInfoString( const bool inbOrig );

	//!
	//! /return true=x and y is bound to 0.5 because the texture is smaller than the preview area, false otherwise
	bool ClampBlitOffset( const int iniWidth, const int iniHeight, float &inoutfX, float &inoutfY, const int iniScale16 ) const;

	//! 
	DWORD GetWidth( void ) const;

	//!
	DWORD GetHeight( void ) const;

	//! run with the user specified user properties in m_Prop
	//! /param inbSave true=save the result, false=update only the internal structures for previewing
	//! /return true=success, false otherwise(e.g. compression failed because of non power of two)
	bool RunWithProperties( const bool inbSave );

	//!
	static char *GetPixelFormatName( const int iniNo );

	//!
	static char *GetPixelFormatDesc( const int iniNo );

	//!
	static int GetPixelFormatCount( void );

	//! set the stored properties to the current file and save it
	//! @return true=success, false otherwise
	bool UpdateAndSaveConfigFile( void );

	CString GetSourceFilename( void );

	// interface IConvertor ----------------------------------------------------

	virtual void Release( void );
	virtual bool Process( ConvertContext &cc );
	virtual bool GetOutputFile( ConvertContext& cc);
	virtual int GetNumPlatforms() const;
	virtual Platform GetPlatform( int index ) const;
	virtual int GetNumExt() const;
	virtual const char* GetExt( int index ) const;
	virtual DWORD GetTimestamp() const;

	// -------------------------------------------------------------------------
	
	void SetPresetSettings();

	CProperties								m_Props;							//!< user settings

	ConvertContext *					m_pCC;								//!< pointer to the object give to Process (is 0 outside of the call) 

private:

	D3DPRESENT_PARAMETERS			m_presentParams;			//!< presentation parameters used on device creation and reset
	LPDIRECT3DDEVICE9					m_pd3ddev;						//!<
	LPDIRECT3D9								m_pd3d;								//!<
	LPDIRECT3DBASETEXTURE9		m_ptexOrig;						//!<
	//LPDIRECT3DBASETEXTURE9		m_ptexNew;						//!<
	//unsigned char *						m_pPalette;
	ImageObject *							m_pNewImage;
	int												m_iOrigPixelFormatNo;	//!<
	DWORD											m_dwOrigWidth;				//!<
	DWORD											m_dwOrigHeight;				//!<
	DWORD											m_dwDepth;						//!< For volume textures
	DWORD											m_dwCubeMapFlags;			//!<
	HWND											m_hWnd;								//!< DirectX needs a window to attach to

	//!
	bool IsCubeMap( void ) { return (m_dwCubeMapFlags > 0); }

	//!
	bool IsVolumeMap( void ) { return (m_dwDepth > 0); }

	//!
	//! /param indwFilter D3DX_FILTER bitmask (see DirectX documentation): e.g. D3DX_FILTER_POINT|D3DX_FILTER_MIRROR, D3DX_FILTER_TRIANGLE
	HRESULT ChangeFormat( LPDIRECT3DBASETEXTURE9 ptexCur, D3DFORMAT fmtTo, ImageObject *&pNewImage, 
		const DWORD indwFilter, const DWORD indwReduceResolution );

	//!
	//! /param indwFilter D3DX_FILTER bitmask (see DirectX documentation): e.g. D3DX_FILTER_POINT|D3DX_FILTER_MIRROR, D3DX_FILTER_TRIANGLE
	HRESULT BltAllLevels( D3DCUBEMAP_FACES FaceType, LPDIRECT3DBASETEXTURE9 ptexSrc, ImageObject *pNewImage,
		const DWORD indwFilter, const DWORD indwReduceResolution, D3DFORMAT fmtTo );

	//! convert to 8bit paletized texture
	HRESULT PaletteQuantize(LPDIRECT3DSURFACE9 psurfSrc, LPDIRECT3DSURFACE9 psurfDest);

	//! helper function
	bool GetBoolParam( CString insName, const bool inbDefault ) const;

	//! helper function
	int GetIntParam( CString insName, const int iniDefault ) const;

	//!
	DWORD CalcTextureMemory( const bool inbOrig ) const;

	//! \return pixelformat no which has same properties but is uncompressed
	static int GetPixelFormatUncompressed( const int iniNo );

	//! not case sensitive
	//! /return -1 if the name was not recognized, otherwise no
	static int GetNoFromName( const char *inszName );

	//! /return -1 if the format was not recognized, otherwise no
	static int GetNoFromD3DFormat( D3DFORMAT inFormat );

	//! /param psurf has to be in the format A8R8G8B8
	bool CopyAlphaToRGB( LPDIRECT3DSURFACE9 psurf ) const;
	IDirect3DTexture9 *CopyP8ToXRGB(LPDIRECT3DBASETEXTURE9 texp8);

	//!
	bool LoadConfigFile( void );
	

	//! /return 
	static DWORD _CalcMipCount( const DWORD indwWidth, const DWORD indwHeight );

	bool IsPowerOfTwo( const DWORD indwValue );
};


#define DXERRORCHECK(cmt,exp) { HRESULT _hr = (exp); /*assert(!_hr);*/ if(_hr) CCLOG->LogError("'%s' DX ERROR: %s", cmt,DXGetErrorString9A(_hr)); }
