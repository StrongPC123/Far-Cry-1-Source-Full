//////////////////////////////////////////////////////////////////////
//
//  CryFont Source Code
//
//  File: FFont.h
//  Description: Font class.
//
//  History:
//  - August 20, 2001: Created by Alberto Demichelis
//	- January 29, 2004: Refactored by Márcio Martins
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYFONT_FFONT_H
#define CRYFONT_FFONT_H


#include <vector>
#include <string>
#include <Cry_Math.h>
#include "FontTexture.h"

#define FONTCOLORS 10

#define FONT_SMOOTH_NONE					0
#define FONT_SMOOTH_BLUR					1
#define FONT_SMOOTH_SUPERSAMPLE		2

#define FONT_SMOOTH_AMOUNT_NONE		0
#define FONT_SMOOTH_AMOUNT_2X			1
#define FONT_SMOOTH_AMOUNT_4X			2

#ifdef WIN64
#undef GetCharWidth
#undef GetCharHeight
#endif


class CFFont : public IFFont
{
public:
	enum eBlendMode
	{
		BLEND_ZERO              =  0,
		BLEND_ONE               =  1,
		BLEND_SRCCOLOR          =  2,
		BLEND_INVSRCCOLOR       =  3,
		BLEND_SRCALPHA          =  4,
		BLEND_INVSRCALPHA       =  5,
		BLEND_DSTALPHA          =  6,
		BLEND_INVDSTALPHA       =  7,
		BLEND_DSTCOLOR          =  8,
		BLEND_INVDSTCOLOR       =  9,
	};

	struct SRenderingPass
	{
		color4f	cColor;
		vector2f			vSizeScale;
		vector2f			vPosOffset;
		eBlendMode		blendSrc;
		eBlendMode		blendDest;

		inline void SetColor(const color4f &color)
		{
			cColor = color;
		}

		void ResetDefault()
		{
			vPosOffset.set(0,0);
			vSizeScale.set(1.0f,1.0f);
			blendSrc = BLEND_SRCALPHA;
			blendDest = BLEND_INVSRCALPHA;

			SetColor(color4f(1,1,1,1));
		}

		SRenderingPass()
		{
			ResetDefault();
		}

		~SRenderingPass()
		{
		}
	};

	struct SEffect
	{
		string											strName;
		std::vector<SRenderingPass> vPass;

		SEffect()
		{
		}

		SRenderingPass* NewPass()
		{
			SRenderingPass Pass;

			vPass.push_back(Pass);

			return &vPass[vPass.size()-1];
		}

		void Clear()
		{
			vPass.clear();
		}
	};

	typedef std::vector<SEffect>	VecEffect;
	typedef VecEffect::iterator		VecEffectItor;

public:

	void Reset();

	// Load a font from a TTF file
	bool Load(const char *szFile, unsigned long nWidth, unsigned long nHeight, unsigned long nTTFFlags);

	// Load a font from a XML script
	bool Load(const char *szFile);

	// Free the memory
	void Free();

	// Set the current effect to use
	void SetEffect(const char *szEffect);

	// Set the color of current effect
	void SetColor(const color4f& col, int nPass);
	void UseRealPixels(bool bRealPixels=true) { m_bRealPixels=bRealPixels; m_vCharSize = vector2f(-1.0f, -1.0f); }
	bool UsingRealPixels() { return m_bRealPixels; }

	// Set the characters base size
	void SetSize(const vector2f &size);

	// Set clipping rectangle
	void SetClippingRect(float fX, float fY, float fW, float fH);

	// Enable / Disable clipping (off by default)
	void EnableClipping(bool bEnable);

	// Return the seted size
	vector2f &GetSize();

	// Return the char width
	float GetCharWidth();

	// Return the char height
	float GetCharHeight();

	// Set the width scaling
	void SetCharWidthScale(float fScale);

	// Get the width scaling
	float GetCharWidthScale();

	// Set the same size flag
	void SetSameSize(bool bSameSize);

	// Get the same size flag
	bool GetSameSize();

	// Draw a not formated string
	void DrawStringNF( float x, float y, const char *szMsg);

	// Draw a formated string
	void DrawString( float x, float y, const char *szMsg, const bool bASCIIMultiLine=true );

	// Compute the text size
	vector2f GetTextSize(const char *szMsg, const bool bASCIIMultiLine=true );	

	// Draw a formated string
	void DrawStringW( float x, float y, const wchar_t *swStr, const bool bASCIIMultiLine=true );

	// Draw a formated string
	void DrawWrappedStringW( float x, float y, float w, const wchar_t *swStr, const bool bASCIIMultiLine=true );
  
	// Compute the text size
	vector2f GetTextSizeW(const wchar_t *swStr, const bool bASCIIMultiLine=true );

	// Compute the text size
	vector2f GetWrappedTextSizeW(const wchar_t *swStr, float w, const bool bASCIIMultiLine=true );

	// Compute virtual text-length (because of special chars...)
	int GetTextLength(const char *szMsg, const bool bASCIIMultiLine=true);
	int GetTextLengthW(const wchar_t *szMsg, const bool bASCIIMultiLine=true);

	///////////////////////////////////////////////
	// Create a new effect
	SEffect*	NewEffect();
	// Return the current effect
	SEffect*	GetCurrentEffect();

	bool RenderInit();
	void RenderCleanup();

	void GetMemoryUsage (class ICrySizer* pSizer);

private:

	void Prepare(const wchar_t *szString);
	void WrapText(wstring &szResult, float fMaxWidth, const wchar_t *szString);

	friend class CCryFont;
	CFFont(struct ISystem *pISystem, class CCryFont *pCryFont, const char *pszName);
	virtual ~CFFont();

	// Release the memory...
	void Release();

	DWORD								m_vColorTable[FONTCOLORS];

	bool								m_bRealPixels;
	bool								m_bOK;					// false if the font has not been correctly initialised
	bool								m_bSameSize;			// True if all char must have the same size
	float								m_fWidthScale;			// Width scale
	vector2f						m_vCharSize;

	vector2f						m_vSize;					// Base size
	VecEffect						m_vEffects;				// Rendering effects
	SEffect*						m_pCurrentEffect;		// Current effect
	CFontTexture				m_pFontTexture;
	unsigned char				*m_pFontBuffer;						// inmemory image of the font file (needed because of pack files)
	int									m_iTextureID;							// texture id
	struct ISystem			*m_pISystem;							// Renderer interface
	CCryFont*						m_pCryFont;
	string							m_szName;
  
	float m_fClipX;
	float m_fClipY;
	float m_fClipR;
	float m_fClipB;
	bool	m_bClipEnabled;
};


#endif // CRYFONT_FFONT_H