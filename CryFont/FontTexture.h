//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - Create and update a texture with the most recently used glyphs
//
// History:
//  - [6/6/2003] created the file
//
//-------------------------------------------------------------------------------------------------
#pragma once


#ifdef WIN64
// Workaround for Amd64 compiler
#include <map>
#define hash_map map
#else
#if defined(LINUX)
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#endif

#include <string>

#include "GlyphCache.h"
#include "GlyphBitmap.h"


// comment this to use 8bit (alpha only texture) wich is both faster, and uses much less memory
//#undef 	FONT_USE_32BIT_TEXTURE
#define	FONT_USE_32BIT_TEXTURE

// the number of slots in the glyph cache
// each slot ocupies ((glyph_bitmap_width * glyph_bitmap_height) + 24) bytes
#define	FONT_GLYPH_CACHE_SIZE			(1)

// the glyph spacing offset from the left margin, in pixels
#define FONT_GLYPH_OFFSET					(2)

// the size of a rendered space, this value gets multiplied by the default characted width
#define FONT_SPACE_SIZE						(0.5f)

// don't draw this char (used to avoid drawing color codes)
#define FONT_NOT_DRAWABLE_CHAR		(0xffff)

// smoothing methods
#define FONT_SMOOTH_NONE			0
#define FONT_SMOOTH_BLUR			1
#define FONT_SMOOTH_SUPERSAMPLE		2

// smoothing amounts
#define FONT_SMOOTH_AMOUNT_NONE		0
#define FONT_SMOOTH_AMOUNT_2X		1
#define FONT_SMOOTH_AMOUNT_4X		2


typedef struct CTextureSlot
{
	unsigned int	dwUsage;
	wchar_t				cCurrentChar;

	int						iTextureSlot;
	float					vTexCoord[2];
	int						iCharWidth;
	int						iCharHeight;

	void			Reset()
	{
		dwUsage = 0;
		cCurrentChar = -1;
		iCharWidth = 0;
		iCharHeight = 0;

	};

} CTextureSlot;


typedef std::vector<CTextureSlot *>							CTextureSlotList;
typedef std::vector<CTextureSlot *>::iterator				CTextureSlotListItor;

typedef std::hash_map<wchar_t, CTextureSlot *>				CTextureSlotTable;
typedef std::hash_map<wchar_t, CTextureSlot *>::iterator	CTextureSlotTableItor;

#ifdef WIN64
#undef GetCharWidth
#undef GetCharHeight
#endif

class CFontTexture
{
public:
	CFontTexture();
	~CFontTexture();

	int CreateFromFile(const string &szFileName, int iWidth, int iHeight, int iSmoothMethod, int iSmoothAmount, float fSizeRatio = 0.8f, int iWidthCharCount = 16, int iHeightCharCount = 16);
	int CreateFromMemory(unsigned char *pFileData, int iDataSize, int iWidth, int iHeight, int iSmoothMethod, int iSmoothAmount, float fSizeRatio = 0.875f, int iWidthCharCount = 16, int iHeightCharCount = 16);
	int Create(int iWidth, int iHeight, int iSmoothMethod, int iSmoothAmount, float fSizeRatio = 0.8f, int iWidthCharCount = 16, int iHeightCharCount = 16);
	int Release();

	int SetEncoding(FT_Encoding pEncoding) { return m_pGlyphCache.SetEncoding(pEncoding); };
	FT_Encoding GetEncoding() { return m_pGlyphCache.GetEncoding(); };

	int GetCellWidth() { return m_iCellWidth; };
	int GetCellHeight() { return m_iCellHeight; };

	int GetWidth() { return m_iWidth; };
	int GetHeight() { return m_iHeight; };

	int GetWidthCellCount() { return m_iWidthCellCount; };
	int GetHeightCellCount() { return m_iHeightCellCount; };

	float GetTextureCellWidth() { return m_fTextureCellWidth; };
	float GetTextureCellHeight() { return m_fTextureCellHeight; };

#ifdef FONT_USE_32BIT_TEXTURE
	 unsigned int *GetBuffer() { return m_pBuffer; };
#else
	unsigned char *GetBuffer() { return m_pBuffer; };
#endif

	wchar_t			GetSlotChar(int iSlot);
	CTextureSlot	*GetCharSlot(wchar_t cChar);

	CTextureSlot	*GetLRUSlot();
	CTextureSlot	*GetMRUSlot();

	// returns 1 if texture updated, returns 2 if texture not updated, returns 0 on error
	// pUpdated is the number of slots updated
	int PreCacheString(const wchar_t *szString, int *pUpdated = 0);
	int GetTextureCoord(wchar_t cChar, float *fU, float *fV);
	int GetCharWidth(wchar_t cChar);
	int GetCharHeight(wchar_t cChar);

	int WriteToFile(const string &szFileName);

private:

	int				CreateSlotList(int iListSize);
	int				ReleaseSlotList();

	int				UpdateSlot(int iSlot, unsigned int dwUsage, wchar_t cChar);

	int				m_iWidth;
	int				m_iHeight;

	int				m_iCellWidth;
	int				m_iCellHeight;

	float			m_fTextureCellWidth;
	float			m_fTextureCellHeight;

	int				m_iWidthCellCount;
	int				m_iHeightCellCount;

	int				m_nTextureSlotCount;

	int				m_iSmoothMethod;
	int				m_iSmoothAmount;

	CGlyphCache		m_pGlyphCache;
	CTextureSlotList m_pSlotList;
	CTextureSlotTable m_pSlotTable;


#ifdef FONT_USE_32BIT_TEXTURE
	unsigned int	*m_pBuffer;
#else
	unsigned char	*m_pBuffer;
#endif

	unsigned int	m_dwUsage;
};

