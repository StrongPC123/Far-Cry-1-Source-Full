// SyntaxColorizer.h: interface for the CSyntaxColorizer class.
//
// Version:	1.0.0
// Author:	Jeff Schering jeffschering@hotmail.com
// Date:	Jan 2001
// Copyright 2001 by Jeff Schering
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SYNTAXCOLORIZER_H__5C50E421_E4AC_11D4_A61E_60A459C10100__INCLUDED_)
#define AFX_SYNTAXCOLORIZER_H__5C50E421_E4AC_11D4_A61E_60A459C10100__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define CLR_STRING	RGB(55,0,200)
#define CLR_PLAIN	RGB(0,0,0)
#define CLR_COMMENT RGB(0,170,0)
#define CLR_KEYWORD RGB(0,0,255)
#define GRP_KEYWORD		0

#ifdef WIN64
#define hash_map map
#else
#if defined(LINUX)
#include <ext/hash_map>
#else
#include <hash_map>
#endif
#endif

class CSyntaxColorizer  
{
public:
	CSyntaxColorizer();
	virtual ~CSyntaxColorizer();

//protected vars
protected:
	unsigned char *m_pTableZero, *m_pTableOne;
	unsigned char *m_pTableTwo,  *m_pTableThree;
	unsigned char *m_pTableFour, *m_pAllowable;

	enum Types
	{
		SKIP,
		DQSTART,	//Double Quotes start
		DQEND,		//Double Quotes end
		SQSTART,	//Single Quotes start
		SQEND,		//Single Quotes end
		CMSTART,	//Comment start (both single and multi line)
		SLEND,		//Single line comment end
		MLEND,		//Multi line comment end
		KEYWORD		//Keyword start
	} m_type;

	struct SKeyword 
	{
		char* keyword;
		int  keylen;
		CHARFORMAT cf;
		int group;
		SKeyword* pNext;
		SKeyword() { pNext = NULL; }
	};

	SKeyword*	m_pskKeyword;
	CHARFORMAT	m_cfComment;
	CHARFORMAT	m_cfString;
	CHARFORMAT	m_cfDefault;

	//typedef std::map<LPCSTR,SKeyword*,> Keywords;
	//typedef std::hash_map<const char*,SKeyword*,stl::hash_strcmp<const char*> > Keywords;
	typedef std::hash_map<const char*,SKeyword*,stl::hash_strcmp<const char*> > Keywords;
	Keywords m_keywords;

//protected member functions
protected:
	void addKey(LPCTSTR Keyword, CHARFORMAT cf, int grp);
	void createTables();
	void deleteTables();
	void createDefaultKeywordList();
	void createDefaultCharFormat();
	void colorize(LPTSTR lpszBuf, CRichEditCtrl *pCtrl, long iOffset = 0);

//public member functions
public:
	void Colorize(long StartChar, long nEndChar, CRichEditCtrl *pCtrl);
	void Colorize(CHARRANGE cr, CRichEditCtrl *pCtrl);

	void GetCommentStyle(CHARFORMAT &cf) { cf = m_cfComment; };
	void GetStringStyle(CHARFORMAT &cf) { cf = m_cfString; };
	void GetGroupStyle(int grp, CHARFORMAT &cf);
	void GetDefaultStyle(CHARFORMAT &cf) { cf = m_cfDefault; };

	void SetCommentStyle(CHARFORMAT cf) { m_cfComment = cf; };
	void SetCommentColor(COLORREF cr);
	void SetStringStyle(CHARFORMAT cf) { m_cfString = cf; };
	void SetStringColor(COLORREF cr);
	void SetGroupStyle(int grp, CHARFORMAT cf);
	void SetGroupColor(int grp, COLORREF cr);
	void SetDefaultStyle(CHARFORMAT cf) { m_cfDefault = cf; };

	void AddKeyword(LPCTSTR Keyword, CHARFORMAT cf, int grp = 0); 
	void AddKeyword(LPCTSTR Keyword, COLORREF cr, int grp = 0);
	void ClearKeywordList();
	CString GetKeywordList();
	CString GetKeywordList(int grp);
};

#endif // !defined(AFX_SYNTAXCOLORIZER_H__5C50E421_E4AC_11D4_A61E_60A459C10100__INCLUDED_)

