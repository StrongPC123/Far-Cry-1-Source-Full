///////////////////////////////////////////////////////////////////////
//
//  CryFont Source Code
//
//  File: FFontXML.cpp
//  Description: XML parsing to load a font.
//
//  History:
//  - August 21, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FFont.h"
#include <XmlStream.h>
#include <Cry_Math.h>
#include <string>



//////////////////////////////////////////////////////////////////////////////////////////////
// Xml parser class.
// Not very nice implementation but... cool enought ;)

#define ELEMENT_UNKNOWN					0
#define ELEMENT_FONT					1
#define ELEMENT_EFFECT					2
#define ELEMENT_PASS					4
#define ELEMENT_PASS_COLOR				5
#define ELEMENT_PASS_SIZESCALE			11
#define ELEMENT_PASS_POSOFFSET			12
#define ELEMENT_PASS_BLEND				14

///////////////////////////////////////////////
// utility function to get the blending type from a string
static inline CFFont::eBlendMode GetBlendModeFromString(const string& str)
{
	CFFont::eBlendMode blend = CFFont::BLEND_SRCCOLOR;

	if(str == "zero")
	{
		blend = CFFont::BLEND_ZERO;
	}
	else if(str == "one")
	{
		blend = CFFont::BLEND_ONE;
	}
	else if(str == "srccolor" ||
		    str == "src_color")
	{
		blend = CFFont::BLEND_SRCCOLOR;
	}
	else if(str == "invsrccolor" ||
			str == "inv_src_color")
	{
		blend = CFFont::BLEND_INVSRCCOLOR;
	}
	else if(str == "srcalpha" ||
			str == "src_alpha")
	{
		blend = CFFont::BLEND_SRCALPHA;
	}
	else if(str == "invsrcalpha" ||
			str == "inv_src_alpha")
	{
		blend = CFFont::BLEND_INVSRCALPHA;
	}
	else if(str == "dstalpha" ||
			str == "dst_alpha")
	{
		blend = CFFont::BLEND_DSTALPHA;
	}
	else if(str == "invdstalpha" ||
			str == "inv_dst_alpha")
	{
		blend = CFFont::BLEND_INVDSTALPHA;
	}
	else if(str == "dstcolor" ||
			str == "dst_color")
	{
		blend = CFFont::BLEND_DSTCOLOR;
	}
	else if(str == "invdstcolor" ||
			str == "inv_dst_color")
	{
		blend = CFFont::BLEND_INVDSTCOLOR;
	}

	return blend;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Xml parser implementation
class CXmlFontShader : public IXmlNotify
{
public:
	CXmlFontShader(CFFont *pFont)
	{
		m_pFont = pFont;
		m_nElement = ELEMENT_UNKNOWN;
		m_pEffect = NULL;
		m_pPass = NULL;
		m_FontTexSize.set(0,0);
		m_bNoRescale = 0,
		m_FontSmoothAmount = 0;
		m_FontSmoothMethod = FONT_SMOOTH_NONE;
	}

	~CXmlFontShader()
	{
	}

	// notify methods
	void FoundElement(const string& name,const string& value)
	{
		//MessageBox(NULL, string("[" + name + "]").c_str(), "FoundElement", MB_OK);		
		// process the previous element
		switch(m_nElement)
		{
			case ELEMENT_FONT:
				{
					int iWidth = GetISystem()->GetIRenderer()->GetWidth();

					if(!m_FontTexSize.x || !m_FontTexSize.y)
					{
						m_FontTexSize.set(512, 512);
					}

					m_pFont->Load(m_strFontPath.c_str(), m_FontTexSize.x,m_FontTexSize.y, TTFFLAG_CREATE(m_FontSmoothMethod, m_FontSmoothAmount));
				}
				break;

			default:
				break;
		}

		// Translate the m_nElement name to a define
		if(name == "font")
		{
			m_nElement = ELEMENT_FONT;
		}
		else if(name == "effect")
		{
			m_nElement = ELEMENT_EFFECT;
		}
		else if(name == "pass")
		{
			m_pPass = NULL;
			m_nElement = ELEMENT_PASS;
			if(m_pEffect)
				m_pPass = m_pEffect->NewPass();
		}
		else if(name == "color")
		{
			m_nElement = ELEMENT_PASS_COLOR;
		}
		else if(name == "size")
		{	
			m_nElement = ELEMENT_PASS_SIZESCALE;
		}
		else if(name == "pos" ||
				name == "offset")
		{
			m_nElement = ELEMENT_PASS_POSOFFSET;
		}
		else if(name == "blend" ||
				name == "blending")
		{
			m_nElement = ELEMENT_PASS_BLEND;
		}
		else
		{
			m_nElement = ELEMENT_UNKNOWN;
		}
	}

	void FoundAttribute	(const  string & name,const  string &value )
	{
		//MessageBox(NULL, string(name + "\n" + value).c_str(), "FoundAttribute", MB_OK);
		switch(m_nElement)
		{
			case ELEMENT_FONT:
				if(name == "path")
				{
					m_strFontPath = value;
				}
				else if(name == "w")
				{
					m_FontTexSize.x = (long)atof(value.c_str());
				}
				else if(name == "h")
				{
					m_FontTexSize.y = (long)atof(value.c_str());
				}
				else if(name == "norescale")
				{
					if (value.empty() || (atoi(value.c_str()) != 0))
					{
						m_bNoRescale = 1;
					}
				}
				else if (name == "smooth")
				{
					if (value == "blur")
					{
						m_FontSmoothMethod = FONT_SMOOTH_BLUR;
					}
					else if (value == "supersample")
					{
						m_FontSmoothMethod = FONT_SMOOTH_SUPERSAMPLE;
					}
					else if (value == "none")
					{
						m_FontSmoothMethod = FONT_SMOOTH_NONE;
					}
				}
				else if (name == "smooth_amount")
				{
					m_FontSmoothAmount = (long)atof(value.c_str());
				}
				break;

			case ELEMENT_EFFECT:
				if(name == "name")
				{					
					if(value == "default")
					{
						m_pFont->SetEffect(NULL);
						m_pEffect = m_pFont->GetCurrentEffect();
						m_pEffect->strName = "default";

						m_pEffect->Clear();
					}
					else
					{
						m_pEffect = m_pFont->NewEffect();
						m_pEffect->strName = value;
					}
				}
				break;

			case ELEMENT_PASS_COLOR:
				if(!m_pPass) break;
				if(name == "r")			{	m_pPass->cColor.r = (float)atof(value.c_str()); }
				else if(name == "g")	{	m_pPass->cColor.g = (float)atof(value.c_str()); }
				else if(name == "b")	{	m_pPass->cColor.b = (float)atof(value.c_str()); }
				else if(name == "a")	{	m_pPass->cColor.a = (float)atof(value.c_str()); }
				break;

			case ELEMENT_PASS_SIZESCALE:
				if(!m_pPass) break;
				if(name == "x")
				{
					m_pPass->vSizeScale.x = (float)atof(value.c_str());
				}
				else if(name == "y")
				{
					m_pPass->vSizeScale.y = (float)atof(value.c_str());
				}
				else if(name == "scale")
				{
					m_pPass->vSizeScale.x =
					m_pPass->vSizeScale.y = (float)atof(value.c_str());
				}
				break;

			case ELEMENT_PASS_POSOFFSET:
				if(!m_pPass) break;
				if(name == "x")
				{
					m_pPass->vPosOffset.x = (float)atoi(value.c_str());
				}
				else if(name == "y")
				{
					m_pPass->vPosOffset.y = (float)atoi(value.c_str());
				}
				break;

			case ELEMENT_PASS_BLEND:
				if(!m_pPass) break;
				if(name == "src")
				{
					m_pPass->blendSrc = GetBlendModeFromString(value);
				}
				else if(name == "dst")
				{
					m_pPass->blendDest = GetBlendModeFromString(value);
				}
				else if(name == "type")
				{
					if(value == "modulate")
					{
						m_pPass->blendSrc = CFFont::BLEND_SRCALPHA;
						m_pPass->blendDest = CFFont::BLEND_INVSRCALPHA;
					}
					else if(value == "additive")
					{
						m_pPass->blendSrc = CFFont::BLEND_ZERO;
						m_pPass->blendDest = CFFont::BLEND_INVSRCALPHA;
					}				
				}
				break;

			default:
			case ELEMENT_UNKNOWN:
				break;
		}
	}

public:
	CFFont *m_pFont;

	unsigned long			m_nElement;

	CFFont::SEffect*		m_pEffect;
	CFFont::SRenderingPass*	m_pPass;	

	string		m_strFontPath;
	vector2l	m_FontTexSize;
	bool			m_bNoRescale;

	int				m_FontSmoothMethod;
	int				m_FontSmoothAmount;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Main loading function

///////////////////////////////////////////////
bool CFFont::Load(const char *szFile)
{
	ICryPak *pPak = GetISystem()->GetIPak();

	if (!pPak)
	{
		return false;
	}

	FILE *fp = pPak->FOpen(szFile, "rb");
	if(!fp)
		return false;

	pPak->FSeek(fp,0,SEEK_END);
	int size = pPak->FTell(fp);

	char *buffer = new char[size+1];
	if(!buffer)
	{
		pPak->FClose(fp);
		return false;
	}
	buffer[size] = 0;

	pPak->FSeek(fp,0,SEEK_SET);
	pPak->FRead(buffer,size,1,fp);
	pPak->FClose(fp);

	CXmlFontShader xmlfs(this);

	XmlStream xml(&xmlfs);
	xml.parse(buffer,size);

	SetEffect("default");
	delete [] buffer;
	return m_bOK;
}
