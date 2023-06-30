//////////////////////////////////////////////////////////////////////
//
//  CryFont Source Code
//
//  File: FFont.cpp
//  Description: Font class.
//
//  History:
//  - August 20, 2001: d by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FFont.h"
#include "FBitmap.h"
#include "CryFont.h"
//#include <Image.h>
#ifdef PS2
#include "PS2Font.h"
#endif

#include "FontTexture.h"
#ifdef WIN64
#undef GetCharWidth
#undef GetCharHeight
#endif
///////////////////////////////////////////////
CFFont::CFFont(struct ISystem *pISystem, class CCryFont *pCryFont, const char *pszName)
{
	m_bRealPixels=false;
	m_fWidthScale = 1.0f;
	m_bSameSize = false;
	//	m_pBitmap = NULL;
	m_pISystem = pISystem;
	m_pCryFont = pCryFont;
	m_szName = pszName;
	m_vSize.set(16, 16);
	m_iTextureID = -1;
	m_vCharSize=vector2f(-1,-1);

	m_fClipX = m_fClipY = 0.0f;
	m_fClipR = m_fClipB = 0.0f;

	m_bClipEnabled = 0;
	m_pFontBuffer = 0;

	m_vColorTable[0]=0xff000000;
	m_vColorTable[1]=0xffffffff;
	m_vColorTable[2]=0xffff0000;
	m_vColorTable[3]=0xff00ff00;
	m_vColorTable[4]=0xff0000ff;
	m_vColorTable[5]=0xffffff00;
	m_vColorTable[6]=0xff00ffff;
	m_vColorTable[7]=0xffff00ff;
	m_vColorTable[8]=0xff0080ff;
	m_vColorTable[9]=0xff8f8f8f;

	// create the default effect
	SEffect *pEffect = NewEffect();
	pEffect->strName = "default";
	pEffect->NewPass();
	SetEffect("default");
}

///////////////////////////////////////////////
CFFont::~CFFont()
{
	/*if (m_pCryFont)
	{
	FontMapItor itor;
	itor=m_pCryFont->m_mapFonts.find(m_sName.c_str());
	if (itor!=m_pCryFont->m_mapFonts.end())
	m_pCryFont->m_mapFonts.erase(itor);
	m_pCryFont=NULL;
	}*/
	Free();
}

void CFFont::Reset()
{
	m_fWidthScale = 1.0f;
	m_bClipEnabled = 0;
	m_vSize = vector2f(16.0f, 16.0f);
	
	m_bSameSize = 0;
	m_bRealPixels = 0;
	m_vCharSize = vector2f(-1.0f, -1.0f);
}


///////////////////////////////////////////////
// Release the memory...
void CFFont::Release()
{
	delete this;
}

///////////////////////////////////////////////
// Load a font from a CImage
/*bool CFFont::Load(class CImage *pImg)
{
Free();

m_pBitmap = new CFBitmap(pImg);
if(!m_pBitmap)
return false;

for(int i = 0; i < 256; ++i)
m_TexCooMap[i].z = 1.0f;

return RenderInit();
}*/

///////////////////////////////////////////////
// Load a font from a TTF file
bool CFFont::Load(const char *szFile, unsigned long nWidth, unsigned long nHeight, unsigned long nTTFFlags)
{
	Free();

	int i = 0;
	
#ifdef PS2
	// [marco] for ps2 use the game path to prepend 
	// the file name or load from pack as shown below	
	if((m_pBitmap=LoadXtfFont(szFile, &m_TexCooMap[0])))
	{
		m_bOK = true;
		return RenderInit();
	}
#endif


	int iSmoothMethod = (nTTFFlags & TTFFLAG_SMOOTH_MASK) >> TTFFLAG_SMOOTH_SHIFT;
	int iSmoothAmount = (nTTFFlags & TTFFLAG_SMOOTH_AMOUNT_MASK) >> TTFFLAG_SMOOTH_AMOUNT_SHIFT;

	ICryPak *pPak = m_pISystem->GetIPak();

	FILE *pFile = pPak->FOpen(szFile,"rb");

	if (!pFile)
		return false;

	pPak->FSeek(pFile, 0, SEEK_END); 
	int nSize = pPak->FTell(pFile); 
	pPak->FSeek(pFile, 0, SEEK_SET); 

	if (!nSize)
	{
		pPak->FClose(pFile); 

		return false;
	}	

	unsigned char *pBuffer = new unsigned char[nSize];

	if (!pPak->FRead(pBuffer, nSize, 1, pFile))
	{
		pPak->FClose(pFile);

		delete[] pBuffer;

		return false;
	}

	pPak->FClose(pFile);

	if (!m_pFontTexture.CreateFromMemory(pBuffer, nSize, nWidth, nHeight, iSmoothMethod, iSmoothAmount))
	{
		delete[] pBuffer;

		return false;
	}

	m_pFontBuffer = pBuffer;

	//------------------------------------------------------------------------------------------------- 

	m_bOK = true;
#ifdef PS2
	ConvertXtfFont(m_pBitmap, m_pBitmap->GetWidth(),m_pBitmap->GetHeight(), (uchar *)m_pBitmap->m_pData);
	SaveXtfFont(szFile, m_pBitmap, &m_TexCooMap[0], m_pBitmap->GetWidth(),m_pBitmap->GetHeight());
#endif	
	return RenderInit();
}

///////////////////////////////////////////////
// Free the memory
void CFFont::Free()
{
	RenderCleanup();

	if (m_pFontBuffer)
	{
		delete[] m_pFontBuffer;
	}
	m_pFontBuffer = 0;

	m_bOK = false;
}

///////////////////////////////////////////////
// Set the current effect to use
void CFFont::SetEffect(const char *szEffect)
{
	m_pCurrentEffect = NULL;
	if(!szEffect)
		szEffect = "default";

	for(int i = 0; i < (int)m_vEffects.size(); ++i)
	{
		if(strcmp(m_vEffects[i].strName.c_str(), szEffect) == 0)
		{
			m_pCurrentEffect = &m_vEffects[i];
			return;
		}
	}

	if(!m_pCurrentEffect)
	{
		m_pCurrentEffect = &m_vEffects[0];
	}
}

// Set clipping rectangle
void CFFont::SetClippingRect(float fX, float fY, float fX2, float fY2)
{
	m_fClipX = fX;
	m_fClipY = fY;
	m_fClipR = fX2;
	m_fClipB = fY2;
}

// Enable / Disable clipping (off by default)
void CFFont::EnableClipping(bool bEnable)
{
	m_bClipEnabled = bEnable;
}


///////////////////////////////////////////////
void CFFont::SetColor(const color4f& col, int nPass)
{
	SRenderingPass *pPass;
	if(nPass < 0)
	{
		for(std::vector<SRenderingPass>::iterator i = m_pCurrentEffect->vPass.begin();
			i != m_pCurrentEffect->vPass.end(); ++i)
		{
			pPass = &(*i);
			pPass->SetColor(col);
		}
	}
	else
	{
		if(nPass >= (int)m_pCurrentEffect->vPass.size())
			return;
		pPass = &m_pCurrentEffect->vPass[nPass];
		pPass->SetColor(col);
	}
}

///////////////////////////////////////////////
// Set the characters base size
void CFFont::SetSize(const vector2f &vSize)
{
	m_vCharSize = vector2f(-1.0f, -1.0f);
	m_vSize = vSize;
}


///////////////////////////////////////////////
// Set the same size flag
void CFFont::SetSameSize(bool bSameSize)
{
	m_bSameSize = bSameSize;
	m_vCharSize = vector2f(-1.0f, -1.0f);
}

///////////////////////////////////////////////
// Get the same size flag
bool CFFont::GetSameSize()
{
	return m_bSameSize;
}

///////////////////////////////////////////////
// Return the seted size
vector2f &CFFont::GetSize()
{
	return m_vSize;
}


///////////////////////////////////////////////
// Return the char width
float CFFont::GetCharWidth()
{
	IRenderer *pRenderer = m_pISystem->GetIRenderer();
	assert(pRenderer);

	if (m_vCharSize.x == -1.0f)
	{
		float fMaxW = m_vSize.x;

		if (m_pCurrentEffect)
		{
			
			for(int i = m_pCurrentEffect->vPass.size()-1; i >= 0; --i)
			{
				SRenderingPass *Pass = &m_pCurrentEffect->vPass[i];

				float fScale = Pass->vSizeScale.x;
				float fOffset = Pass->vPosOffset.x;
				float fPassW = fScale * m_vSize.x + fOffset;

				if (!m_bRealPixels)
				{
					fPassW = pRenderer->ScaleCoordX(fScale * m_vSize.x) + fOffset;
				}

				if (m_bSameSize)
				{
					fPassW *= m_fWidthScale;
				}

				if (m_bRealPixels)

				if (fPassW > fMaxW)
				{
					fMaxW = fPassW;
				}
			}
		}

		return fMaxW;
	}

	return m_vCharSize.x;
}

///////////////////////////////////////////////
// Return the char height
float CFFont::GetCharHeight()
{
	IRenderer *pRenderer = m_pISystem->GetIRenderer();
	assert(pRenderer);

	if (m_vCharSize.y == -1.0f)
	{
		float fMaxH = m_vSize.y;

		if (m_pCurrentEffect)
		{

			for(int i = m_pCurrentEffect->vPass.size()-1; i >= 0; --i)
			{
				SRenderingPass *Pass = &m_pCurrentEffect->vPass[i];

				float fScale = Pass->vSizeScale.y;
				float fOffset = Pass->vPosOffset.y;
				float fPassH = fScale * m_vSize.y + fOffset;

				if (!m_bRealPixels)
				{
					fPassH = pRenderer->ScaleCoordY(fScale * m_vSize.y) + fOffset;
				}

				if (fPassH > fMaxH)
				{
					fMaxH = fPassH;
				}
			}
		}

		return fMaxH;
	}

	return m_vCharSize.y;
}

///////////////////////////////////////////////
void CFFont::SetCharWidthScale(float fScale)
{
	m_fWidthScale = fScale;
}

///////////////////////////////////////////////
float CFFont::GetCharWidthScale()
{
	return m_fWidthScale;
}

/*
#define FONT_RGBA(r, g, b, a) \
(   (((long)((a) * 255)) << 24) | (((long)((r) * 255)) << 16) \
|   (((long)((g) * 255)) << 8) | (long)((b) * 255) \
)
*/

#define FONT_RGBA(r, g, b, a) \
	(   (((long)((a) * 255)) << 24) | (((long)((b) * 255)) << 16) \
	|   (((long)((g) * 255)) << 8) | (long)((r) * 255) \
	)

_inline DWORD COLCONV (DWORD clr)
{
	return ((clr & 0xff00ff00) | ((clr & 0xff0000)>>16) | ((clr & 0xff)<<16));
}

///////////////////////////////////////////////
// Draw a formated string

void CFFont::DrawString( float fBaseX, float fBaseY, const char *szMsg, const bool bASCIIMultiLine )
{
	if (!szMsg)
	{
		return;
	}

	int iSize = min(1023, strlen(szMsg));

	static wchar_t szwMsg[1024];

	szwMsg[iSize] = 0;
	while (iSize--)
	{
		szwMsg[iSize] = (unsigned char)szMsg[iSize];
	}

	DrawStringW(fBaseX, fBaseY, szwMsg, bASCIIMultiLine);
}

void CFFont::DrawStringW(float fBaseX, float fBaseY, const wchar_t *szMsg, const bool bASCIIMultiLine)
{
	// please terminate your strings with a '\0'
	// if you want to draw a string with more than 682 char, tell me (marcio)
	// and will allocate two buffers
	//assert(wcslen(szMsg) <= 682);

	IRenderer *pRenderer = m_pISystem->GetIRenderer();
	assert(pRenderer);

	if (!szMsg)
	{
		return;
	}

	Prepare(szMsg);

	float fTexHeight = m_pFontTexture.GetCellHeight() / (float)m_pFontTexture.GetHeight();
	bool	bRGB = (pRenderer->GetFeatures() & RFT_RGBA) != 0;
	float fAlpha = m_pCurrentEffect->vPass[0].cColor.v[3];
	struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *pVertex = 0;
	int		iVertexOffset = 0;
	int		iTextLength = GetTextLengthW(szMsg);

	pRenderer->FontSetTexture(m_iTextureID, FILTER_TRILINEAR);
	pRenderer->FontSetRenderingState(0, 0);

	for(int i = m_pCurrentEffect->vPass.size()-1; i >= 0; --i)
	{
		if (!i)
		{
			fAlpha = 1.0f;
		}		

		SRenderingPass *Pass = &m_pCurrentEffect->vPass[i];
		
		// gather pass data
		vector2f	vBaseXY = vector2f(fBaseX, fBaseY);
		vector2f	vOffset = Pass->vPosOffset;
		vector2f	vSize = vector2f(m_vSize.x * Pass->vSizeScale.x, m_vSize.y * Pass->vSizeScale.y);
		DWORD			dwPassColor = 0;
		DWORD			dwColor = 0;
		int				iVBLen = 0;
		
		if (!m_bRealPixels)
		{
			vSize.x = pRenderer->ScaleCoordX(vSize.x);
			vSize.y = pRenderer->ScaleCoordY(vSize.y);
			vBaseXY.x = pRenderer->ScaleCoordX(vBaseXY.x);
			vBaseXY.y = pRenderer->ScaleCoordY(vBaseXY.y);
		}

		float			fRcpCellWidth = (1.0f / (float)m_pFontTexture.GetCellWidth()) * vSize.x;
		float			fCharX = vBaseXY.x + vOffset.x;
		float			fCharY = vBaseXY.y + vOffset.y;

		if (bRGB)
		{
			dwPassColor = FONT_RGBA(Pass->cColor.r, Pass->cColor.g, Pass->cColor.b, Pass->cColor.a * fAlpha);
		}
		else
		{
			dwPassColor = FONT_RGBA(Pass->cColor.b, Pass->cColor.g, Pass->cColor.r, Pass->cColor.a * fAlpha);
		}

		dwColor = dwPassColor;
		
		pRenderer->FontSetBlending(Pass->blendSrc, Pass->blendDest);
		pVertex = (struct_VERTEX_FORMAT_P3F_COL4UB_TEX2F *)pRenderer->GetDynVBPtr(iTextLength * 6, iVertexOffset, 0);
		assert(pVertex);

		wchar_t *pcChar = (wchar_t *)szMsg;
		wchar_t ch;

		// parse the string, ignoring control characters
		while (ch = *pcChar++)
		{
			switch(ch)
			{
			case L'\\':
				{
					if (*pcChar != L'n' || !bASCIIMultiLine)
					{
						break;
					}
					++pcChar;
				}
			case L'\n':
				{
					fCharX = vBaseXY.x + vOffset.x;
					fCharY += vSize.y;
					continue;
				}
				break;
			case L'\r':
				{
					fCharX = vBaseXY.x + vOffset.x;

					continue;
				}
				break;
			case L'\t':
				{
					if (m_bSameSize)
					{
						fCharX += 4 * vSize.x * m_fWidthScale;
					}
					else
					{
						fCharX += FONT_SPACE_SIZE * 4 * vSize.x;
					}

					continue;
				}
				break;
			case L' ':
				{
					if (m_bSameSize)
					{
						fCharX += vSize.x * m_fWidthScale;
					}
					else
					{
						fCharX += FONT_SPACE_SIZE * vSize.x;
					}
					
					continue;
				}
				break;
			case L'$':
				{
					if (*pcChar == L'$')
					{
						++pcChar;
					}
					else if(isdigit(*pcChar))
					{
						if (!i)
						{
							int iColorIndex = (*pcChar) - L'0';

							dwColor = m_vColorTable[iColorIndex];

							if (bRGB)
							{
								dwColor = m_vColorTable[iColorIndex];
							}
							else
							{
								dwColor = COLCONV(m_vColorTable[iColorIndex]);
							}

							// apply the correct alpha
							dwColor = (dwColor & 0x00ffffff) | ((long)((Pass->cColor.a * fAlpha) * 255.0f)) << 24;
						}

						++pcChar;

						continue;
					}
					else if ((*pcChar == L'O' || *pcChar == L'o') && !i)
					{
						if (!i)
						{
							dwColor = dwPassColor;
						}

						++pcChar;

						continue;
					}
					else if (*pcChar)
					{
						++pcChar;

						continue;
					}
				}
				break;
			default:
				break;
			}

			int iCharWidth = m_pFontTexture.GetCharWidth(ch);
			float fWidth = iCharWidth * fRcpCellWidth;
			float fAdvance = (iCharWidth + 1) * fRcpCellWidth;
			float fTexWidth = (iCharWidth + 1) / (float)m_pFontTexture.GetWidth();

			// get texture coordinates
			float vTexCoord[4];

			m_pFontTexture.GetTextureCoord(ch, &vTexCoord[0], &vTexCoord[1]);

			vTexCoord[2] = vTexCoord[0] + fTexWidth;
			vTexCoord[3] = vTexCoord[1] + fTexHeight;

			float fX = fCharX;
			float fY = fCharY;

			if (m_bSameSize)
			{						
				fX = fCharX + ((vSize.x * m_fWidthScale) - fWidth) * 0.5f;
				fAdvance = vSize.x * m_fWidthScale;
			}

			float fR = fX + fWidth;
			float fB = fY + vSize.y;

			// compute clipping
			float fNewX = fX;
			float fNewY = fY;
			float fNewR = fR;
			float fNewB = fB;
			
			if (m_bClipEnabled)
			{
				// clip non visible
				if ((fX >= m_fClipR) || (fY >= m_fClipB) || (fR < m_fClipX) || (fB < m_fClipY))
				{
					fCharX += fAdvance;

					continue;
				}
				// clip partially visible
				else
				{
					if ((fWidth <= 0.0f) || (vSize.y <= 0.0f))
					{
						fCharX += fAdvance;

						continue;
					}

					// clip the image to the scissor rect
					fNewX = max(m_fClipX, fX);
					fNewY = max(m_fClipY, fY);
					fNewR = min(m_fClipR, fR);
					fNewB = min(m_fClipB, fB);

					float fRcpWidth = 1.0f / fWidth;
					float fRcpHeight = 1.0f / vSize.y;

					float	fTexW = vTexCoord[2] - vTexCoord[0];
					float fTexH = vTexCoord[3] - vTexCoord[1];

					// clip horizontal
					vTexCoord[0] = vTexCoord[0] + (fTexW * ((fNewX - fX) * fRcpWidth));
					vTexCoord[2] = vTexCoord[2] + (fTexW * ((fNewR - (fX + fWidth)) * fRcpWidth));

					// clip vertical
					vTexCoord[1] = vTexCoord[1] + (fTexH * ((fNewY - fY) * fRcpHeight));
					vTexCoord[3] = vTexCoord[3] + (fTexH * ((fNewB - (fY + vSize.y)) * fRcpHeight));
				}
			}

			int iOffset = iVBLen * 6;

			// define char quad
			pVertex[iOffset].xyz.x = fNewX;
			pVertex[iOffset].xyz.y = fNewY;
			pVertex[iOffset].xyz.z = 1.0;
			pVertex[iOffset].color.dcolor = dwColor;
			pVertex[iOffset].st[0] = vTexCoord[0];
			pVertex[iOffset++].st[1] = vTexCoord[1];

			pVertex[iOffset].xyz.x = fNewR;
			pVertex[iOffset].xyz.y = fNewY;
			pVertex[iOffset].xyz.z = 1.0;
			pVertex[iOffset].color.dcolor = dwColor;
			pVertex[iOffset].st[0] = vTexCoord[2];
			pVertex[iOffset++].st[1] = vTexCoord[1];

			pVertex[iOffset].xyz.x = fNewR;
			pVertex[iOffset].xyz.y = fNewB;
			pVertex[iOffset].xyz.z = 1.0;
			pVertex[iOffset].color.dcolor = dwColor;
			pVertex[iOffset].st[0] = vTexCoord[2];
			pVertex[iOffset++].st[1] = vTexCoord[3];

			pVertex[iOffset].xyz.x = fNewR;
			pVertex[iOffset].xyz.y = fNewB;
			pVertex[iOffset].xyz.z = 1.0;
			pVertex[iOffset].color.dcolor = dwColor;
			pVertex[iOffset].st[0] = vTexCoord[2];
			pVertex[iOffset++].st[1] = vTexCoord[3];

			pVertex[iOffset].xyz.x = fNewX;
			pVertex[iOffset].xyz.y = fNewB;
			pVertex[iOffset].xyz.z = 1.0;
			pVertex[iOffset].color.dcolor = dwColor;
			pVertex[iOffset].st[0] = vTexCoord[0];
			pVertex[iOffset++].st[1] = vTexCoord[3];

			pVertex[iOffset].xyz.x = fNewX;
			pVertex[iOffset].xyz.y = fNewY;
			pVertex[iOffset].xyz.z = 1.0;
			pVertex[iOffset].color.dcolor = dwColor;
			pVertex[iOffset].st[0] = vTexCoord[0];
			pVertex[iOffset++].st[1] = vTexCoord[1];

			if (iVBLen >= 682)
			{
				break;
			}

			++iVBLen;

			fCharX += fAdvance;
		}

		// draw this pass
		pRenderer->DrawDynVB(iVertexOffset, 0, iVBLen * 6);
	}

	// restore the old states	
	pRenderer->FontRestoreRenderingState();	
}

void CFFont::DrawWrappedStringW( float fBaseX, float fBaseY, float w, const wchar_t *szMsg, const bool bASCIIMultiLine )
{
	wstring szWrapped;

	WrapText(szWrapped, w, szMsg);
	DrawStringW(fBaseX, fBaseY, szWrapped.c_str(), bASCIIMultiLine);
}

vector2f CFFont::GetWrappedTextSizeW(const wchar_t *swStr, float w, const bool bASCIIMultiLine)
{
	wstring szWrapped;

	WrapText(szWrapped, w, swStr);
	return GetTextSizeW(szWrapped.c_str(), bASCIIMultiLine);
}

vector2f CFFont::GetTextSize(const char *szMsg, const bool bASCIIMultiLine)
{
	if (!szMsg)
	{
		return vector2f(0.0f, 0.0f);
	}

	int iSize = min(1023, strlen(szMsg));

	static wchar_t szwMsg[1024];

	szwMsg[iSize] = 0;
	while (iSize--)
	{
		szwMsg[iSize] = (unsigned char)szMsg[iSize];
	}

	return GetTextSizeW(szwMsg,bASCIIMultiLine);
}

///////////////////////////////////////////////
// Compute the text size
vector2f CFFont::GetTextSizeW(const wchar_t *szMsg, const bool bASCIIMultiLine)
{
	IRenderer *pRenderer = m_pISystem->GetIRenderer();
	assert(pRenderer);

	if (wcslen(szMsg) == 1 && *szMsg == L'$')
	{
		int x = 0;
	}

	if (!szMsg)
	{
		return vector2f(0,0);
	}

	Prepare(szMsg);

	float fMaxW = 0.0f;
	float fMaxH = 0.0f;

	for(int i = m_pCurrentEffect->vPass.size()-1; i >= 0; --i)
	{
		SRenderingPass *Pass = &m_pCurrentEffect->vPass[i];

		// gather pass data
		vector2f	vOffset = Pass->vPosOffset;
		vector2f	vSize = vector2f(m_vSize.x * Pass->vSizeScale.x, m_vSize.y * Pass->vSizeScale.y);

		if (!m_bRealPixels)
		{
			vSize.x = pRenderer->ScaleCoordX(vSize.x);
			vSize.y = pRenderer->ScaleCoordY(vSize.y);
		}

		float			fRcpCellWidth = (1.0f / (float)m_pFontTexture.GetCellWidth()) * vSize.x;
		float			fCharX = vOffset.x;
		float			fCharY = vOffset.y + vSize.y;

		if (fCharY > fMaxH)
		{
			fMaxH = fCharY;
		}

		wchar_t *pcChar = (wchar_t *)szMsg;
		wchar_t ch;

		// parse the string, ignoring control characters
		while (ch = *pcChar++)
		{
			switch(ch)
			{
			case L'\\':
				{
					if (*pcChar != L'n' || !bASCIIMultiLine)
					{
						break;
					}
					++pcChar;
				}
			case L'\n':
				{
					if (fCharX > fMaxW)
					{
						fMaxW = fCharX;
					}

					fCharX = vOffset.x;
					fCharY += vSize.y;

					if (fCharY > fMaxH)
					{
						fMaxH = fCharY;
					}

					continue;
				}
				break;
			case L'\r':
				{
					if (fCharX > fMaxW)
					{
						fMaxW = fCharX;
					}

					fCharX = vOffset.x;

					continue;
				}
				break;
			case L'\t':
				{
					fCharX += FONT_SPACE_SIZE * 4 * vSize.x;

					continue;
				}
				break;
			case L' ':
				{
					fCharX += FONT_SPACE_SIZE * vSize.x;

					continue;
				}
				break;
			case L'$':
				{
					if (*pcChar == L'$')
					{
						++pcChar;
					}
					else if (*pcChar)
					{
						++pcChar;

						continue;
					}
				}
				break;
			default:
				break;
			}

			int iCharWidth = m_pFontTexture.GetCharWidth(ch);
			float fAdvance = (iCharWidth + 1) * fRcpCellWidth;

			fCharX += fAdvance;
		}

		if (fCharX > fMaxW)
		{
			fMaxW = fCharX;
		}
	}

	return vector2f(fMaxW, fMaxH);
}

///////////////////////////////////////////////
int CFFont::GetTextLengthW(const wchar_t *szMsg, const bool bASCIIMultiLine)
{
	int iLength = 0;

	wchar_t *pcChar = (wchar_t *)szMsg;
	wchar_t ch;

	// parse the string, ignoring control characters
	while (ch = *pcChar++)
	{
		switch(ch)
		{
		case L'\\':
			{
				if (*pcChar != L'n' || !bASCIIMultiLine)
				{
					break;
				}
				++pcChar;
			}
		case L'\n':
		case L'\r':
		case L'\t':
			{
				continue;
			}
			break;
		case L'$':
			{
				if (*pcChar == L'$')
				{
					++pcChar;
				}
				else if (*pcChar)
				{
					++pcChar;

					continue;
				}
			}
			break;
		default:
			break;
		}
		++iLength;
	}

	return iLength;
}

///////////////////////////////////////////////
int CFFont::GetTextLength(const char *szMsg, const bool bASCIIMultiLine)
{
	int iLength = 0;

	char *pcChar = (char *)szMsg;
	char ch;

	// parse the string, ignoring control characters
	while (ch = *pcChar++)
	{
		switch(ch)
		{
		case '\\':
			{
				if (*pcChar != L'n' || !bASCIIMultiLine)
				{
					break;
				}
				++pcChar;
			}
		case '\n':
		case '\r':
		case '\t':
			{
				continue;
			}
			break;
		case '$':
			{
				if (*pcChar == L'$')
				{
					++pcChar;
				}
				else if (*pcChar)
				{
					++pcChar;

					continue;
				}
			}
			break;
		default:
			break;
		}
		++iLength;
	}

	return iLength;
}

///////////////////////////////////////////////
// Push a new effect in the vector and return a pointer to it
CFFont::SEffect* CFFont::NewEffect()
{
	SEffect effect;
	m_vEffects.push_back(effect);
	return &m_vEffects[m_vEffects.size()-1];
}

///////////////////////////////////////////////
// Return the current effect
CFFont::SEffect* CFFont::GetCurrentEffect()
{
	return m_pCurrentEffect;
}

///////////////////////////////////////////////
bool CFFont::RenderInit()
{
#ifdef FONT_USE_32BIT_TEXTURE
	m_iTextureID = m_pISystem->GetIRenderer()->FontCreateTexture(m_pFontTexture.GetWidth(), m_pFontTexture.GetHeight(), (unsigned char *)m_pFontTexture.GetBuffer(), eTF_8888);
#else
	m_iTextureID = m_pISystem->GetIRenderer()->FontCreateTexture(m_pFontTexture.GetWidth(), m_pFontTexture.GetHeight(), m_pFontTexture.GetBuffer(), eTF_8000);
#endif

	if (m_iTextureID < 0)
	{
		return false;
	}

	return true;
}

///////////////////////////////////////////////
void CFFont::RenderCleanup()
{
	if (m_iTextureID > -1)
	{
		m_pISystem->GetIRenderer()->RemoveTexture(m_iTextureID);
	}

	m_pFontTexture.Release();
}

void CFFont::GetMemoryUsage (class ICrySizer* pSizer)
{
	if (!pSizer->Add (*this))
		return;

	pSizer->AddObject(&m_pFontTexture, m_pFontTexture.GetWidth() * m_pFontTexture.GetHeight() * 4);

	// <<FIXME>> glyph cache bitmaps should also be added here

	pSizer->AddString (m_szName);
	pSizer->Add (&m_vEffects[0],m_vEffects.size());
	for (VecEffect::iterator it = m_vEffects.begin(); it != m_vEffects.end(); ++it)
	{
		pSizer->Add (it->strName.c_str(), it->strName.capacity()+1);
		pSizer->Add (&it->vPass[0], it->vPass.size());
	}
}

//------------------------------------------------------------------------------------------------- 
void CFFont::Prepare(const wchar_t *szString)
{
	static int n = 0;
	if (m_pFontTexture.PreCacheString(szString) == 1)
	{
		m_pISystem->GetIRenderer()->FontUpdateTexture(m_iTextureID, 0, 0, m_pFontTexture.GetWidth(), m_pFontTexture.GetHeight(), (unsigned char *)m_pFontTexture.GetBuffer());
	}
}

//------------------------------------------------------------------------------------------------- 
void CFFont::WrapText(wstring &szResult, float fMaxWidth, const wchar_t *szString)
{
	szResult = szString;

	if (!m_bRealPixels)
	{
		fMaxWidth = m_pISystem->GetIRenderer()->ScaleCoordX(fMaxWidth);
	}

	vector2f vStringSize = GetTextSizeW(szResult.c_str());

	if (vStringSize.x <= fMaxWidth)
	{
		return;
	}

	int		iLastSpace = -1;
	float	fLastSpaceWidth = 0.0f;

	float fCurrentCharWidth = 0.0f;
	float fCurrentLineWidth = 0.0f;
	float fBiggestLineWidth = 0.0f;
	float fWidthSum = 0.0f;

	int				iCurrentChar = 0;
	wchar_t		*pChar = (wchar_t *)szResult.c_str();
	wchar_t		szChar[2] = {0, 0};

	while(szChar[0] = *pChar++)
	{
		// ignore color codes
		if (szChar[0] == L'$')
		{
			if (*pChar)
			{
				++pChar;
				++iCurrentChar;

				if ((*pChar) != L'$')
				{
					++iCurrentChar;

					continue;
				}
				szChar[0] = *pChar;
			}
		}

		// get char width and sum it to the line width
		fCurrentCharWidth = GetTextSizeW(szChar).x;

		// keep track of spaces
		// they are good for spliting the string :D
		if (szChar[0] == L' ')
		{
			iLastSpace = iCurrentChar;
			fLastSpaceWidth = fCurrentLineWidth + fCurrentCharWidth;
		}

		// if line exceed allowed width, split it
		if ((fCurrentLineWidth + fCurrentCharWidth >= fMaxWidth) && (*pChar))
		{
			if ((iLastSpace > 0) && ((iCurrentChar - iLastSpace) < 16) && (iCurrentChar - iLastSpace > 0)) // 16 is the default treshold
			{
				szResult.insert(iLastSpace + 1, 1, L'\n');

				++iCurrentChar;
				pChar = (wchar_t *)(szResult.c_str() + iCurrentChar + 1);

				if (fLastSpaceWidth > fBiggestLineWidth)
				{
					fBiggestLineWidth = fLastSpaceWidth;
				}

				fCurrentLineWidth = fCurrentLineWidth - fLastSpaceWidth + fCurrentCharWidth;
				fWidthSum += fCurrentLineWidth;
			}
			else
			{
				szResult.insert(iCurrentChar, 1, L'\n');

				++iCurrentChar;
				pChar = (wchar_t *)(szResult.c_str() + iCurrentChar + 1);

				if (fCurrentLineWidth > fBiggestLineWidth)
				{
					fBiggestLineWidth = fCurrentLineWidth;
				}

				fWidthSum += fCurrentLineWidth;
				fCurrentLineWidth = fCurrentCharWidth;
			}

			// if we don't need any more line breaks, then just stop
			if (vStringSize.x - fWidthSum <= fMaxWidth)
			{
				break;
			}

			fLastSpaceWidth = 0;
			iLastSpace = 0;
		}
		else
		{
			fCurrentLineWidth += fCurrentCharWidth;
		}

		++iCurrentChar;
	}
}


