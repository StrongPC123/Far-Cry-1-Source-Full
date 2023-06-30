//-------------------------------------------------------------------------------------------------
// Author: Márcio Martins
//
// Purpose:
//  - Manage and cache glyphs, retrieving them from the renderer as needed
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

#include <vector>
#include "GlyphBitmap.h"
#include "FontRenderer.h"



typedef struct CCacheSlot
{
	unsigned int	dwUsage;
	int				iCacheSlot;
	wchar_t			cCurrentChar;

	int				iCharWidth;
	int				iCharHeight;
    
	CGlyphBitmap	pGlyphBitmap;

	void			Reset()
	{
		dwUsage = 0;
		cCurrentChar = -1;

		iCharWidth = 0;
		iCharHeight = 0;

		pGlyphBitmap.Clear();
	}

} CCacheSlot;


typedef std::hash_map<wchar_t, CCacheSlot *>			CCacheTable;
typedef std::hash_map<wchar_t, CCacheSlot *>::iterator	CCacheTableItor;

typedef std::vector<CCacheSlot *>						CCacheSlotList;
typedef std::vector<CCacheSlot *>::iterator				CCacheSlotListItor;


#ifdef WIN64
#undef GetCharWidth
#undef GetCharHeight
#endif

class CGlyphCache
{
public:
	CGlyphCache();
	~CGlyphCache();

	int Create(int iChacheSize, int iGlyphBitmapWidth, int iGlyphBitmapHeight, int iSmoothMethod, int iSmoothAmount, float fSizeRatio = 0.8f);
	int Release();

	int LoadFontFromFile(const string &szFileName);
	int LoadFontFromMemory(unsigned char *pFileBuffer, int iDataSize);
	int ReleaseFont();

	int SetEncoding(FT_Encoding pEncoding) { return m_pFontRenderer.SetEncoding(pEncoding); };
	FT_Encoding GetEncoding() { return m_pFontRenderer.GetEncoding(); };

	int	GetGlyphBitmapSize(int *pWidth, int *pHeight);

	int PreCacheGlyph(wchar_t cChar);
	int UnCacheGlyph(wchar_t cChar);
	int GlyphCached(wchar_t cChar);

	CCacheSlot *GetLRUSlot();
	CCacheSlot *GetMRUSlot();

	int GetGlyph(CGlyphBitmap **pGlyph, int *piWidth, int *piHeight, wchar_t cChar);

private:

	int				CreateSlotList(int iListSize);
	int				ReleaseSlotList();

	CCacheSlotList	m_pSlotList;
	CCacheTable		m_pCacheTable;

	int				m_iGlyphBitmapWidth;
	int				m_iGlyphBitmapHeight;
	float			m_fSizeRatio;

	int				m_iSmoothMethod;
	int				m_iSmoothAmount;

	CGlyphBitmap	*m_pScaleBitmap;

	CFontRenderer	m_pFontRenderer;

	unsigned int	m_dwUsage;
};