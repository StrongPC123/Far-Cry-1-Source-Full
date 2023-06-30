//////////////////////////////////////////////////////////////////////
//
//  CryFont Source Code
//
//  File: CryFont.h
//  Description: Main interface to the DLL.
//
//  History:
//  - August 17, 2001: Created by Alberto Demichelis
//
//////////////////////////////////////////////////////////////////////

#ifndef CRYFONT_CRYFONT_H
#define CRYFONT_CRYFONT_H

#include <map>
#include <string>

typedef std::map<string, IFFont*> FontMap;
typedef FontMap::iterator FontMapItor;
//////////////////////////////////////////////////////////////////////////////////////////////
class CCryFont : public ICryFont
{
public:
	CCryFont(ISystem *pSystem);
	virtual ~CCryFont();

	void Release();
	
	// create a font
	struct IFFont *NewFont(const char *pszName);
	struct IFFont *GetFont(const char *pszName);

	//! Puts the objects used in this module into the sizer interface
	void GetMemoryUsage (class ICrySizer* pSizer);
private:
	friend class CFFont;
	FontMap m_mapFonts;
  ISystem*	m_pISystem;		        // system interface

private:
};

#endif // CRYFONT_CRYFONT_H
