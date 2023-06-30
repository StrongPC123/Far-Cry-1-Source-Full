//////////////////////////////////////////////////////////////////////
//
//	Crytek CryENGINE Source code
//	
//	File:Font.h
//
//	History:
//	-Feb 2,2001:Created by Marco Corbetta
//
//////////////////////////////////////////////////////////////////////

#ifndef FONT_H
#define FONT_H

#if _MSC_VER > 1000
# pragma once
#endif

#include <IRenderer.h>

#define FONTCOLORS 9

//////////////////////////////////////////////////////////////////////////
//! Depricated file, must be removed.
//////////////////////////////////////////////////////////////////////////
class CXFont
{
public:	
	CXFont( IRenderer *pRenderer )
	{
		m_pRenderer=pRenderer;
		m_invfontsize=0;
		m_charsize=0;
		m_char_inc=0;
		m_font_id=-1;
		m_image=NULL;
	}
	virtual ~CXFont()
	{
		m_image = NULL;
	}
  virtual void Release()
  {
    delete this;
  }
	
	void	SetImage(ITexPic *image)
	{
		m_image=image;	
		CalcCharSize();
	}
	
	void	CalcCharSize()
	{
		if (m_image)
		{
			m_invfontsize=1.0f/(float)m_image->GetWidth();
			m_charsize=(float)m_image->GetWidth()/16.0f;
			m_char_inc=m_charsize*m_invfontsize;	
		}
	}

public:
	float	m_invfontsize;
	float	m_charsize;
	float	m_char_inc;
	int		m_font_id;

	ITexPic * m_image;
	IRenderer *m_pRenderer;
};

#endif