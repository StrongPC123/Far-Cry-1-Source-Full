//////////////////////////////////////////////////////////////////////
//
//  CryFont Source Code
//
//  File: IFont.h
//  Description: CryFont interface.
//
//  History:
//  - August 17, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYFONT_ICRYFONT_H
#define CRYFONT_ICRYFONT_H

#include <Cry_Color4.h>
#include <Cry_Math.h>


struct ISystem;

//////////////////////////////////////////////////////////////////////////////////////////////
// THE Only exported function of the DLL

// export for the dll, very clear ;=)
extern "C"
#if !defined(_XBOX) && !defined(LINUX)
#ifdef CRYFONT_EXPORTS
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
#endif
struct ICryFont* CreateCryFontInterface(ISystem *pSystem);

typedef ICryFont *(*PFNCREATECRYFONTINTERFACE)(ISystem *pSystem);

//////////////////////////////////////////////////////////////////////////////////////////////
// Rendering interfaces
enum CRYFONT_RENDERINGINTERFACE
{
	CRYFONT_RI_OPENGL = 0,		// pRIData is ignored
	CRYFONT_RI_LAST
};

//////////////////////////////////////////////////////////////////////////////////////////////
struct ICryFont
{

	virtual void Release() = 0;
	
	//! create a named font
	virtual struct IFFont *NewFont(const char *pszName) = 0;
	//! get a named font
	virtual struct IFFont *GetFont(const char *pszName) = 0;

	//! Puts the objects used in this module into the sizer interface
	virtual void GetMemoryUsage (class ICrySizer* pSizer) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////
#define TTFFLAG_SMOOTH_NONE			0x00000000		// no smooth
#define TTFFLAG_SMOOTH_BLUR			0x00000001		// smooth by bluring it
#define TTFFLAG_SMOOTH_SUPERSAMPLE	0x00000002		// smooth by rendering the characters into a bigger texture, 
													// and then resize it to the normal size using bilinear filtering

#define TTFFLAG_SMOOTH_MASK			0x0000000f		// mask for retrieving
#define TTFFLAG_SMOOTH_SHIFT				0		// shift amount for retrieving

#define TTFLAG_SMOOTH_AMOUNT_2X		0x00010000		// blur / supersample [2x]
#define TTFLAG_SMOOTH_AMOUNT_4X		0x00020000		// blur / supersample [4x]

#define TTFFLAG_SMOOTH_AMOUNT_MASK	0x000f0000		// mask for retrieving
#define TTFFLAG_SMOOTH_AMOUNT_SHIFT			16		// shift amount for retrieving


// create a ttflag
#define TTFFLAG_CREATE(smooth, amount)		((((smooth) << TTFFLAG_SMOOTH_SHIFT) & TTFFLAG_SMOOTH_MASK) | (((amount) << TTFFLAG_SMOOTH_AMOUNT_SHIFT) & TTFFLAG_SMOOTH_AMOUNT_MASK))
#define TTFFLAG_GET_SMOOTH(flag)			(((flag) & TTFLAG_SMOOTH_MASK) >> TTFFLAG_SMOOTH_SHIFT)
#define TTFFLAG_GET_SMOOTH_AMOUNT(flag)		(((flag) & TTFLAG_SMOOTH_SMOUNT_MASK) >> TTFFLAG_SMOOTH_AMOUNT_SHIFT)


#define FONTRF_HCENTERED		0x80000000		// The font will be centered horizontaly around the x coo
#define FONTRF_VCENTERED		0x40000000		// The font will be centered verticaly around the y coo
#define FONTRF_FILTERED			0x20000000		// The font will be drawn with bilinear filtering

//////////////////////////////////////////////////////////////////////////////////////////////
struct IFFont
{
	//! Reset the font to the default state
	virtual void Reset() = 0;

	virtual void Release() = 0;

	//! Load a font from a TTF file
	virtual bool Load(const char *szFile, unsigned long nWidth, unsigned long nHeight, unsigned long nTTFFlags) = 0;

	//! Load a font from a XML file
	virtual bool Load(const char *szFile) = 0;

	//! Free the memory
	virtual void Free() = 0;

	//! Set the current effect to use
	virtual void SetEffect(const char *szEffect) = 0;

	// Set clipping rectangle
	virtual void SetClippingRect(float fX, float fY, float fW, float fH) = 0;

	// Enable / Disable clipping (off by default)
	virtual void EnableClipping(bool bEnable) = 0;

	//! Set the color of the current effect
	virtual void SetColor(const color4f& col, int nPass = 0) = 0;
	virtual void UseRealPixels(bool bRealPixels=true)=0;
	virtual bool UsingRealPixels()=0;

	//! Set the characters base size
	virtual void SetSize(const vector2f &size) = 0;

	//! Return the seted size
	virtual vector2f &GetSize() = 0;

	//! Return the char width
	virtual float GetCharWidth() = 0;

	//! Return the char height
	virtual float GetCharHeight() = 0;

	//! Set the same size flag
	virtual void SetSameSize(bool bSameSize) = 0;

	//! Get the same size flag
	virtual bool GetSameSize() = 0;

	//! Set the width scaling
	virtual void SetCharWidthScale(float fScale = 1.0f) = 0;

	//! Get the width scaling
	virtual float GetCharWidthScale() = 0;

	//! Draw a formated string
	//! \param bASCIIMultiLine true='\','n' is a valid return, false=it's not
	virtual void DrawString( float x, float y, const char *szMsg, const bool bASCIIMultiLine=true ) = 0;

	//! Compute the text size
	//! \param bASCIIMultiLine true='\','n' is a valid return, false=it's not
	virtual vector2f GetTextSize(const char *szMsg, const bool bASCIIMultiLine=true ) = 0;

	//! Draw a formated string
	//! \param bASCIIMultiLine true='\','n' is a valid return, false=it's not
	virtual void DrawStringW( float x, float y, const wchar_t *swStr, const bool bASCIIMultiLine=true ) = 0;

	// Draw a formated string
	virtual void DrawWrappedStringW( float x, float y, float w, const wchar_t *swStr, const bool bASCIIMultiLine=true ) = 0;

	//! Compute the text size
	//! \param bASCIIMultiLine true='\','n' is a valid return, false=it's not
	virtual vector2f GetTextSizeW(const wchar_t *swStr, const bool bASCIIMultiLine=true ) = 0;	

	// Compute the text size
	virtual vector2f GetWrappedTextSizeW(const wchar_t *swStr, float w, const bool bASCIIMultiLine=true ) = 0;

	//! Compute virtual text-length (because of special chars...)
	virtual int GetTextLength(const char *szMsg, const bool bASCIIMultiLine=true) = 0;

	//! Compute virtual text-length (because of special chars...)
	virtual int GetTextLengthW(const wchar_t *szwMsg, const bool bASCIIMultiLine=true) = 0;

	//! Puts the memory used by this font into the given sizer
	virtual void GetMemoryUsage (class ICrySizer* pSizer) = 0;
};


#endif // CRYFONT_ICRYFONT_H
