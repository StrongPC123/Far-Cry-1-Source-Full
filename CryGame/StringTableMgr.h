
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// StringTableMgr.h: interface for the CStringTableMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRINGTABLEMGR_H__892DBBEA_EC07_456B_8259_4A09006D8523__INCLUDED_)
#define AFX_STRINGTABLEMGR_H__892DBBEA_EC07_456B_8259_4A09006D8523__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////
class CScriptObjectLanguage;
typedef std::map<string,void*>	FileNamesMap;
typedef FileNamesMap::iterator			FileNamesMapItor;

//////////////////////////////////////////////////////////////////////
/*!maps all language dependent string to a numeric id and
vice-versa it also expose the string id over the script
system
*/
class CStringTableMgr  
{
public:
	const wstring & EnumString(int nID) const;
	///const string & EnumString(const char *szId);
	CStringTableMgr();
	virtual ~CStringTableMgr();
	bool Load(ISystem *pSystem,CScriptObjectLanguage &oLang,string sLanguage);
	bool LoadStringTable(string sFileName);
	bool LoadExcelXmlSpreadsheet( const string &sFileName );
	
	bool LookupString( const char *sKey, wstring &sLocalizedString );
	bool LookupEnglishString( const char *sKey, string &sLocalizedString );

	void Localize( const string &sString, wstring &sLocalizedString, bool bEnglish = false);

	bool GetSubtitleLabel(const char *szFilename,char *szLabel);

private:
	//! append the sSource string to the sDest string
	void AppendToUnicodeString(const string& sSource, wstring &sDest);
	void AppendToAsciiString(const string& sSource, string &sDest);
	void AddControl(int nKey);

	typedef std::map<string,int>	StringsKeyMap;
	std::vector<wstring> m_vStrings;
	std::vector<string> m_vEnglishStrings;
	StringsKeyMap m_keysMap;
	
	string m_sLanguage;
	//to keep track of the tables already loaded
	FileNamesMap m_mapLoadedTables;

	ISystem *m_pSystem;
	CScriptObjectLanguage *m_pLanguageStriptObject;
};

#endif // !defined(AFX_STRINGTABLEMGR_H__892DBBEA_EC07_456B_8259_4A09006D8523__INCLUDED_)
