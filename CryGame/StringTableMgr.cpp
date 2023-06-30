
//////////////////////////////////////////////////////////////////////
//
//	Crytek Source code 
//	Copyright (c) Crytek 2001-2004
//
// StringTableMgr.cpp: implementation of the CStringTableMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringTableMgr.h"
#include "ScriptObjectLanguage.h"
#include "IXMLDOM.h"
#include <StlUtils.h>
#include <IInput.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
 
//////////////////////////////////////////////////////////////////////
CStringTableMgr::CStringTableMgr()
{
	m_pSystem=NULL;
	m_pLanguageStriptObject=NULL;
}

//////////////////////////////////////////////////////////////////////
CStringTableMgr::~CStringTableMgr()
{
}

//////////////////////////////////////////////////////////////////////
bool CStringTableMgr::Load(ISystem *pSystem,CScriptObjectLanguage &oLang, string sLanguage)
{
	m_pSystem=pSystem;
	m_sLanguage=sLanguage;
	m_pLanguageStriptObject=&oLang;
	m_vStrings.clear();

	//------------------------------------------------------------------------------------------------- 
	// input localization
	//------------------------------------------------------------------------------------------------- 
	// keyboard
	for (int i = 0; i <= 0x80; i++)
	{
		AddControl(i);
	}

	// mouse
	for (int i = 1; i <= 0x0f; i++)
	{
		AddControl(i*0x10000);
	}

	return (true);
}

void CStringTableMgr::AddControl(int nKey)
{
	IInput *pInput = m_pSystem->GetIInput();

	if (!pInput)
	{
		return;
	}

	wchar_t szwKeyName[256] = {0};
	char		szKey[256] = {0};

	if (!IS_MOUSE_KEY(nKey))
	{
		if (pInput->GetOSKeyName(nKey, szwKeyName, 255))
		{
			sprintf(szKey, "control%d", nKey);

			int nID = (int)m_vStrings.size();

			m_keysMap[szKey] = nID;
			m_vStrings.push_back(szwKeyName);
			m_pLanguageStriptObject->AddString(szKey,nID);

			sprintf(szKey, "%S", szwKeyName);
			m_vEnglishStrings.push_back(szKey);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Retrieve a string by ID from the string-table.
const wstring & CStringTableMgr::EnumString(int nID) const
{
	static wstring unknown (L"???");

	if( nID<0 || nID>=int(m_vStrings.size()) )
		return (unknown);// string doesnt exist, return "???"

	return (m_vStrings[nID]);
}

//////////////////////////////////////////////////////////////////////
// Loads a string-table from a Excel XML Spreadsheet file.
bool CStringTableMgr::LoadExcelXmlSpreadsheet( const string &sFileName )
{ 
	//string sPath="LANGUAGES/"+m_sLanguage+string("/")+sFileName;

	string sPath="LANGUAGES/"+sFileName;

	//check if this table has already been loaded
	FileNamesMapItor nit = m_mapLoadedTables.find(sPath);		
	if (nit!=m_mapLoadedTables.end())
		return (true);

	XDOM::IXMLDOMDocumentPtr pDoc=m_pSystem->CreateXMLDocument();
	
	// load xml-file	
	if (!pDoc->load(sPath.c_str()))
		return (false);

	m_mapLoadedTables[sFileName] = NULL;

	XDOM::IXMLDOMNodeListPtr pNodeList;

	XDOM::IXMLDOMNodePtr pString;
	XDOM::IXMLDOMNodePtr pEnum;
	XDOM::IXMLDOMNodePtr pValue;

	XDOM::IXMLDOMNodeListPtr pWorksheetList = pDoc->getElementsByTagName("Worksheet");
	pWorksheetList->reset();
	XDOM::IXMLDOMNodePtr pWorksheetNode = pWorksheetList->nextNode();
	if (!pWorksheetNode)
		return false;

	XDOM::IXMLDOMNodeListPtr pTableList = pWorksheetNode->getElementsByTagName("Table");
	pTableList->reset();
	XDOM::IXMLDOMNodePtr pTableNode = pTableList->nextNode();
	if (!pTableNode)
		return false;

	XDOM::IXMLDOMNodeListPtr pRowsList = pTableNode->getElementsByTagName("Row");

	XDOM::IXMLDOMNodePtr pRowNode;
	XDOM::IXMLDOMNodePtr pCellNode;
	
	int nRow = 0;
	pRowsList->reset();
	// get all strings in table
	while (pRowNode = pRowsList->nextNode())
	{
		XDOM::IXMLDOMNodeListPtr pCellList = pRowNode->getElementsByTagName("Cell");
		
		nRow++;

		if (nRow == 1) // Skip first row, it only have description.
			continue;
		
		const char* sKeyString = "";
		const char* sEnglishString = "";
		const char* sLanguageString = "";

		int nCell = 0;
		pCellList->reset();
		while (pCellNode = pCellList->nextNode())
		{
			XDOM::IXMLDOMNodeListPtr pDataList = pCellNode->getElementsByTagName("Data");
#if !defined(LINUX64)
			if (pDataList==NULL)
#else
			if (pDataList==0)
#endif
				continue;

			pDataList->reset();
			XDOM::IXMLDOMNodePtr pDataNode = pDataList->nextNode();
			if (!pDataNode)
				continue;

			//CellNode->getTex
			switch (nCell)
			{
			// First cell is message key.
			case 0:
				sKeyString = pDataNode->getText();
				break;
			// Second cell is english message.
			case 1:
				sEnglishString = pDataNode->getText();
				break;
			// Third cell is local language.
			case 2:
				sLanguageString = pDataNode->getText();
				break;
			};
			nCell++;
		}

		const char *sUTF_8_Str = sLanguageString;
		if (strlen(sUTF_8_Str) == 0)
		{
			sUTF_8_Str = sEnglishString;
		}

		int nUTF_8_Len = strlen(sUTF_8_Str);
		int nUnicodeLen = nUTF_8_Len + 16; // + 16 just for safety.
		wchar_t *sUnicodeStr = new wchar_t[ nUnicodeLen*sizeof(wchar_t) + 16 ];
		// Use UTF-8 multibyte unicode decoding to convert to wide string.
		// This is potentially not porrtable, for different platforms, alternative function must be used.
#if defined(LINUX)
		mbstowcs( sUnicodeStr, sUTF_8_Str, nUTF_8_Len );
		sUnicodeStr[ nUTF_8_Len ] = 0;
#else
		MultiByteToWideChar( CP_UTF8,0,sUTF_8_Str,-1,sUnicodeStr,nUnicodeLen );
#endif
		wstring unicodeStr = sUnicodeStr;
		wstring::size_type nPos;
		while ((nPos = unicodeStr.find(L"\\n",0,2))!=wstring::npos)
		{
			unicodeStr.replace(nPos,2,L"\n");
		}

		SAFE_DELETE_ARRAY(sUnicodeStr);

		if (m_keysMap.find(sKeyString) != m_keysMap.end())
		{
			//m_pSystem->GetILog()->Log("\003$6Localized String '%s' Already Loaded!", sKeyString);
		}
		else
		{
			int nID = (int)m_vStrings.size();

			char szLowerCaseKey[1024];
			strcpy(szLowerCaseKey,sKeyString);
			strlwr(szLowerCaseKey);
			m_vStrings.push_back( unicodeStr );
			m_vEnglishStrings.push_back( sEnglishString );
			m_keysMap[szLowerCaseKey] = nID;
			m_pLanguageStriptObject->AddString( sKeyString,nID );
		}
	}
	return (true);
}

//////////////////////////////////////////////////////////////////////////
// Loads a string-table from a XML-file.
bool CStringTableMgr::LoadStringTable(string sFileName)
{
	return LoadExcelXmlSpreadsheet( sFileName );
/*
	string sPath = "LANGUAGES/" + m_sLanguage + "/" + sFileName;

	//check if this table has already been loaded
	FileNamesMapItor nit = m_mapLoadedTables.find(sPath);		
	if (nit!=m_mapLoadedTables.end())
		return (true);

	XDOM::IXMLDOMDocumentPtr pDoc=m_pSystem->CreateXMLDocument();
	// load xml-file	
	if (!pDoc->load(sPath.c_str()))
		return (false);

	m_mapLoadedTables[sFileName] = NULL;

	XDOM::IXMLDOMNodeListPtr pNodeList;
	XDOM::IXMLDOMNodePtr pString;
	XDOM::IXMLDOMNodePtr pEnum;
	XDOM::IXMLDOMNodePtr pValue;
	pNodeList=pDoc->getElementsByTagName("string");
	if(!pNodeList)
		return true;

	pNodeList->reset();
	// get all strings in table
	while (pString=pNodeList->nextNode())
	{
		pEnum=pString->getAttribute("enum");
		pValue=pString->getAttribute("value");
		if((pEnum!=NULL) && (pValue!=NULL))
		{
			wstring::size_type nPos;
			int nID=m_vStrings.size();
			//wstring str=pValue->getText();
			wstring str=pValue->getTextW();
			while ((nPos=str.find(L"\\n",0,2))!=wstring::npos)
			{
				str.replace(nPos,2,L"\n");
			}
			m_vStrings.push_back(str);			

			string keyString=pEnum->getText();						

			m_pLanguageStriptObject->AddString(pEnum->getText(),nID);
		}
	}
	return (true);
	*/
}

//////////////////////////////////////////////////////////////////////////
bool CStringTableMgr::LookupString( const char *sKey,wstring &sLocalizedString )
{
	assert(sKey);

	// Label sign.
	if (sKey[0] == '@')
	{
		char szLowKey[512];
		strcpy(szLowKey,sKey);
		strlwr(szLowKey);

		int nId = stl::find_in_map( m_keysMap,szLowKey,-1 );
		if (nId >= 0)
		{
			sLocalizedString = m_vStrings[nId];
			return true;
		}
		else 
		{
			nId = stl::find_in_map( m_keysMap,szLowKey+1,-1 );
			if (nId >= 0)
			{
				sLocalizedString = m_vStrings[nId];
				return true;
			}
			else
			{
				//GameWarning( "Localized string for Label <%s> not found",sKey );
				AppendToUnicodeString(sKey, sLocalizedString);

				return false;
			}
		}
	}
	else
	{
		//GameWarning( "Not a valid localized string Label <%s>, must start with @ symbol",sKey );
	}

	AppendToUnicodeString(sKey, sLocalizedString);

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CStringTableMgr::LookupEnglishString( const char *sKey, string &sLocalizedString )
{
	assert(sKey);

	// Label sign.
	if (sKey[0] == '@') 
	{
		char szLowKey[512];
		strcpy(szLowKey,sKey);
		strlwr(szLowKey);

		int nId = stl::find_in_map( m_keysMap,szLowKey,-1 );
		if (nId >= 0)
		{
			sLocalizedString = m_vEnglishStrings[nId];
			return true;
		}
		else 
		{
			nId = stl::find_in_map( m_keysMap,szLowKey+1,-1 );
			if (nId >= 0)
			{
				sLocalizedString = m_vEnglishStrings[nId];
				return true;
			}
			else
			{
				//GameWarning( "Localized string for Label <%s> not found",sKey );
				AppendToAsciiString(sKey, sLocalizedString);

				return false;
			}
		}
	}
	else
	{
		//GameWarning( "Not a valid localized string Label <%s>, must start with @ symbol",sKey );
	}

	AppendToAsciiString(sKey, sLocalizedString);

	return false;
}

//////////////////////////////////////////////////////////////////////////
void CStringTableMgr::Localize(const string &sString, wstring &sLocalizedString, bool bEnglish )
{ 	
	// scan the string
	bool done = false;

	int len = sString.length();
	int curpos = 0;
	while (!done)
	{
		int pos = sString.find_first_of("@", curpos);
		if (pos == string::npos)
		{
			// did not find anymore, so we are done
			done = true;
			pos = len;
		}
    // found an occurence

		// we have skipped a few characters, so copy them over
		if (pos > curpos)
		{
			// skip
			AppendToUnicodeString(sString.substr(curpos, pos - curpos), sLocalizedString);
			curpos = pos;
		}

		if (curpos == pos)
		{
			// find the end of the label
			int endpos = sString.find_first_of(" ", curpos);
			if (endpos == string::npos)
			{
				// have reached the end
				done = true;
				endpos = len;
			}

			if (endpos > curpos)
			{
				// localize token		
				if (bEnglish)
				{
					string sLocalizedToken;

					LookupEnglishString(sString.substr(curpos, endpos - curpos).c_str(), sLocalizedToken);

					AppendToUnicodeString(sLocalizedToken, sLocalizedString);
				}
				else
				{
					wstring sLocalizedToken;

					LookupString(sString.substr(curpos, endpos - curpos).c_str(), sLocalizedToken);

					// append
					sLocalizedString+=sLocalizedToken;
				}			

				curpos = endpos;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool CStringTableMgr::GetSubtitleLabel(const char *_szFilename,char *szLabel)
{		
	if (!_szFilename || !szLabel)
		return (false);
	
	char szFilename[512];
	strcpy(szFilename,_szFilename);
	strlwr(szFilename);

  ptrdiff_t len = strlen(szFilename)-1;
  if (len<=1)    
    return (false);  

	char szFilenameCopy[512];
	strcpy(szFilenameCopy,szFilename);
	char *szIn=&szFilenameCopy[len];

	// remove path
	while ((len>0) && (*szIn) && (*szIn!='/') && (*szIn!='\\'))
	{
		szIn--;
    len--;
	}

	szIn++;
  len = strlen(szIn)-1;
  if (len<=1)      
    return (false);  

	// strip extension
  while (szIn[len])
  {
    if (szIn[len]=='.')
    {
      break;
    }
    len--;
    if (!len)
    {      
      return (false);
    }
  }
  
  char szTemp[256];
  strncpy(szTemp, szIn, len);
  szTemp[len] = 0;		

	int nId = stl::find_in_map( m_keysMap,szTemp,-1 );

	if (nId>=0)		
	{        

    // changed this, was copying szIn (original filename) into szLabel
		sprintf(szLabel,"@%s",szTemp);
		return (true);	
	}

	return (false);
} 

//////////////////////////////////////////////////////////////////////////
void CStringTableMgr::AppendToUnicodeString(const string& sSource, wstring &sDest)
{
	std::vector<wchar_t> swTemp;
	swTemp.resize(sSource.size()+1);

#if defined(LINUX)
	swprintf (&swTemp[0], swTemp.size(), L"%S", sSource.c_str());
#else
	swprintf (&swTemp[0], L"%S", sSource.c_str());
#endif
	wstring tmp(&swTemp[0]);
	sDest += tmp;
}

//////////////////////////////////////////////////////////////////////////
void CStringTableMgr::AppendToAsciiString(const string& sSource, string &sDest)
{
	std::vector<char> sTemp;
	sTemp.resize(sSource.size()+1);

	sprintf (&sTemp[0], "%S", sSource.c_str());
	string tmp(&sTemp[0]);
	sDest += tmp;
}