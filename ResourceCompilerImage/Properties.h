#pragma once

class CProperties
{
public:

	CProperties( void )
	{
		m_bMipmaps=true;
		m_bMipMirror=true;
		m_bUserDialog=true;
		m_iDestPixelFormat=0;
		m_bPreviewAlpha=false;
		m_dwReduceResolution=0;
		m_bPreviewFiltered=false;
		m_bPreviewTiled=true;
		m_dwDitherMode=0;
	}

	// config properties
	bool	m_bMipmaps;						//!< about +1/3 more memoyr
	bool	m_bMipMirror;					//!< for tiled texture (only )
	bool	m_bUserDialog;				//!< show the user dialog for interactive tweaking
	int		m_iDestPixelFormat;		//!< index in the g_pixelformats table [0..GetPixelFormatCount()-1]
	DWORD	m_dwReduceResolution;	//!< [0..[ to remove the top mipmap levels
	DWORD	m_dwDitherMode;				//!< 0:none, 1:simple
	
	CString m_sPreset;

	// preview properties
	bool	m_bPreviewAlpha;			//!< replicate the alpha channel as greyscale value
	bool	m_bPreviewFiltered;		//!< activate the bilinear filter in the preview
	bool	m_bPreviewTiled;			//!< 
};
