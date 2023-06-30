/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Crytek Character Animation source code
//	
//	History:
//	10 Feb. 2003 :- Created by Sergiy Migdalskiy
//
//  Contains:
//    Generic inline implementations of miscellaneous string manipulation utilities,
//    including path manipulation
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CRY_ENGINE_STRING_UTILS_HDR_
#define _CRY_ENGINE_STRING_UTILS_HDR_

#if defined(LINUX)
	#include <ctype.h>
#endif

namespace CryStringUtils
{

// removes the extension from the file path
inline void StripFileExtension (string&strFileName)
{
	for (const char* p = strFileName.c_str() + strFileName.length() - 1;
		p >= strFileName.c_str();
		--p)
	{
		switch (*p)
		{
		case ':':
		case '/':
		case '\\':
			// we've reached a path separator - it means there's no extension in this name
			return;

		case '.':
			// there's an extension in this file name
			strFileName.resize (p - strFileName.c_str());
			return;
		}
	};
	// it seems the file name is a pure name, without path or extension
}

// replaces the given file extension with the new one
// NOTE: the new extension must NOTE be with the dot
inline void ReplaceExtension (string& strFileName, const char* szNewExtension)
{
	StripFileExtension(strFileName);
	strFileName += '.';
	strFileName += szNewExtension;
}


// removes the extension from the file path
inline char* StripFileExtension (char * szFilePath)
{
	for (char* p = szFilePath + (int)strlen(szFilePath)-1; p >= szFilePath; --p)
	{
		switch(*p)
		{
		case ':':
		case '/':
		case '\\':
			// we've reached a path separator - it means there's no extension in this name
			return NULL;
		case '.':
			// there's an extension in this file name
			*p = '\0';
			return p+1;
		}
	}
	// it seems the file name is a pure name, without path or extension
	return NULL;
}

 
// returns the parent directory of the given file or directory.
// the returned path is WITHOUT the trailing slash
// if the input path has a trailing slash, it's ignored
// nGeneration - is the number of parents to scan up
template <class String>
String GetParentDirectory (const String& strFilePath, int nGeneration = 1)
{
	for (const char* p = strFilePath.c_str() + strFilePath.length() - 2; // -2 is for the possible trailing slash: there always must be some trailing symbol which is the file/directory name for which we should get the parent
		p >= strFilePath.c_str();
		--p)
	{
		switch (*p)
		{
		case ':':
			return String (strFilePath.c_str(), p);
			break;
		case '/':
		case '\\':
			// we've reached a path separator - return everything before it.
			if (!--nGeneration)
				return String(strFilePath.c_str(), p);
			break;
		}
	};
	// it seems the file name is a pure name, without path or extension
	return String();
}

// converts all chars to lower case
inline string toLower (const string& str)
{
	string strResult = str;
	for (string::iterator it = strResult.begin(); it != strResult.end(); ++it)
		*it = tolower (*it);
	return strResult;
}

// searches and returns the pointer to the extension of the given file
inline const char* FindExtension (const char* szFileName)
{
	const char* szEnd = szFileName + (int)strlen(szFileName);
	for (const char* p = szEnd-1; p >= szFileName; --p)
		if (*p == '.')
			return p+1;

	return szEnd;
}

// searches and returns the pointer to the file name in the given file path
inline const char* FindFileNameInPath (const char* szFilePath)
{
	for (const char* p = szFilePath + (int)strlen(szFilePath)-1; p >= szFilePath; --p)
		if (*p == '\\' || *p == '/')
			return p+1;
	return szFilePath;
}

// works like strstr, but is case-insensitive
inline const char* stristr(const char* szString, const char* szSubstring)
{
	int nSuperstringLength = (int)strlen(szString);
	int nSubstringLength = (int)strlen(szSubstring);

	for (int nSubstringPos = 0; nSubstringPos <= nSuperstringLength - nSubstringLength; ++nSubstringPos)
	{
		if (strnicmp(szString+nSubstringPos, szSubstring, nSubstringLength) == 0)
			return szString+nSubstringPos;
	}
	return NULL;
}

// replaces slashes
inline void UnifyFilePath (string& strPath)
{
	// replace all forward slashes with backslashes
	for (string::iterator itPath = strPath.begin(); itPath != strPath.end(); ++itPath)
		if (*itPath == '/')
			*itPath = '\\';
		else
			*itPath = tolower (*itPath);
}

// converts the number to a string
inline string toString(unsigned nNumber)
{
	char szNumber[16];
	sprintf (szNumber, "%u", nNumber);
	return szNumber;
}

inline string toString(signed int nNumber)
{
	char szNumber[16];
	sprintf (szNumber, "%d", nNumber);
	return szNumber;
}

#ifdef MATRIX_H
inline string toString(const Matrix44& m)
{
	char szBuf[0x200];
	sprintf (szBuf, "{%g,%g,%g,%g}{%g,%g,%g,%g}{%g,%g,%g,%g}{%g,%g,%g,%g}",
		m(0,0),m(0,1),m(0,2),m(0,3),
		m(1,0),m(1,1),m(1,2),m(1,3),
		m(2,0),m(2,1),m(2,2),m(2,3),
		m(3,0),m(3,1),m(3,2),m(3,3));
	return szBuf;
}
#endif

#ifdef _CRYQUAT_H
inline string toString (const CryQuat& q)
{
	char szBuf[0x100];
	sprintf (szBuf, "{%g,{%g,%g,%g}}", q.w, q.v.x, q.v.y, q.v.z);
	return szBuf;
}
#endif

#ifdef VECTOR_H
inline string toString (const Vec3& v)
{
	char szBuf[0x80];
	sprintf (szBuf, "{%g,%g,%g}", v.x, v.y, v.z);
	return szBuf;
}
#endif

// does the same as strstr, but the szString is allowed to be no more than the specified size
inline const char* strnstr (const char* szString, const char* szSubstring, int nSuperstringLength)
{
	int nSubstringLength = (int)strlen(szSubstring);
	if (!nSubstringLength)
		return szString;

	for (int nSubstringPos = 0; szString[nSubstringPos] && nSubstringPos < nSuperstringLength - nSubstringLength; ++nSubstringPos)
	{
		if (strncmp(szString+nSubstringPos, szSubstring, nSubstringLength) == 0)
			return szString+nSubstringPos;
	}
	return NULL;
}


// calculates the number of characters in the given string, limited by the end pointer
inline int strnlen (const char* szString, const char* szStringEnd)
{
	const char* p;
	for (p = szString; p < szStringEnd && *p; ++p)
		continue;

	return p - szString;
}


// finds the string in the array of strings
// returns its 0-based index or -1 if not found
// comparison is case-sensitive
// The string array end is demarked by the NULL value
inline int findString(const char* szString, const char* arrStringList[])
{
	for (const char** p = arrStringList; *p; ++p)
	{
		if (0 == strcmp(*p, szString))
			return p - arrStringList;
	}
	return -1; // string was not found
}


// This function is used for printing out sets of objects of string type.
// just forms the comma-delimited string where each string in the set is presented as a formatted substring
inline string toString (const std::set<string>& setStrings)
{
	string strResult;
	if (!setStrings.empty ())
	{
		strResult += "{";
		for (std::set<string>::const_iterator it = setStrings.begin(); it != setStrings.end(); ++it)
		{
			if (it != setStrings.begin())
				strResult += ", ";
			strResult += "\"";
			strResult += *it;
			strResult += "\"";
		}
		strResult += "}";
	}
	return strResult ;
}

// cuts the string and adds leading ... if it's longer than specified maximum length
inline string cutString (const string& strPath, unsigned nMaxLength)
{
	if (strPath.length() > nMaxLength && nMaxLength > 3)
		return string("...") + string (strPath.c_str() + strPath.length() - (nMaxLength - 3));
	else
		return strPath;
}


/*
//////////////////////////////////////////////////////////////////////////
// this class is used for Boyer/Moore/Gosper-assisted 'egrep' search
// First, upon construction, the delta table is compiled (you don't need to know what it is,
// just believe me - it's needed for fast substring search). Then you can pass the actual text
// buffer(s) to search the substring in.
class CBMGSubstring
{
public:
	enum {g_nMaxPatternLength = 64};
	// initializes the pattern substring; this must not be bigger than g_nMaxPatternLength
	CBMGSubstring(const char* szPattern);

	// searchees for the substring in the given text buffer
	const char* findSubstringIn (const char* szBuffer, int nBufLength);
protected:
	string m_strPattern;
	unsigned char m_arrDelta[0x100];
};

CBMGSubstring::CBMGSubstring (const char* szPattern):
	m_strPattern (szPattern)
{
	int	nPatternLength = m_strPattern.length(); // pattern length
	if (nPatternLength > g_nMaxPatternLength)
		m_strPattern.resize (nPatternLength = g_nMaxPatternLength);

	memset(m_arrDelta, nPatternLength, sizeof(m_arrDelta));

	for (int i = 0; i < nPatternLength; ++i)
		m_arrDelta[((const unsigned char *)szPattern)[i]] = nPatternLength - i - 1;
}

// searchees for the substring in the given text buffer
const char* CBMGSubstring::findSubstringIn (const char* buffer, int buflen)
{
	char	*s; // temp ptr for comparisons
	int	inc,  // position increment
		k,      // current buffer index
		nhits,  // match ctr
		patlen; // pattern length

	nhits = 0;
	k = (patlen = m_strPattern.length()) - 1;

	for (;;)
	{
		// the following (unsigned char *) type casts save a
		// few clocks by freeing us from some XCHGs
		while ((inc = m_arrDelta[((unsigned char *)buffer)[k]]) &&
			((k += inc) < buflen))
			;
		if (k >= buflen)
			return NULL; // we didn't find
    
		s = buffer + (k++ - (patlen - 1));
		if (!strncmp(s, m_strPattern.c_str(), patlen))
			return s;
	}

	return NULL;
}
*/

//////////////////////////////////////////////////////////////////////////
// Converts the given set of NUMBERS into the string
template <typename T>
string toString (const std::set<T>& setMtls, const char* szFormat, const char* szPostfix = "")
{
	string strResult;
	char szBuffer[64];
	if (!setMtls.empty ())
	{
		strResult += strResult.empty()?"(":" (";
		for (typename std::set<T>::const_iterator it = setMtls.begin(); it != setMtls.end(); )
		{
			if (it != setMtls.begin())
				strResult += ", ";
			sprintf (szBuffer, szFormat, *it);
			strResult += szBuffer;
			T nStart = *it;

			++it;

			if (it != setMtls.end() && *it == nStart + 1)
			{
				T nPrev = *it;
				// we've got a region
				while (++it !=setMtls.end() && *it == nPrev+1)
					nPrev = *it;
				if (nPrev == nStart + 1)
					// special case - range of length 1
					strResult += ",";
				else
					strResult += "..";
				sprintf (szBuffer, szFormat, nPrev);
				strResult += szBuffer;
			}
		}
		strResult += ")";
	}
	return szPostfix[0]?strResult+szPostfix:strResult;
}

// returns true if the string matches the wildcard
inline bool MatchWildcard (const char* szString, const char* szWildcard)
{
	const char* pString = szString, *pWildcard = szWildcard;
	// skip the obviously the same starting substring
	while (*pWildcard && *pWildcard != '*' && *pWildcard != '?')
		if (*pString != *pWildcard)
			return false; // must be exact match unless there's a wildcard character in the wildcard string
		else
			++pString, ++pWildcard;

	if (!*pString)
	{
		// this will only match if there are no non-wild characters in the wildcard
		for(; *pWildcard; ++pWildcard)
			if (*pWildcard != '*' && *pWildcard != '?')
				return false;
		return true;
	}

	switch(*pWildcard)
	{
	case '\0':
		return false; // the only way to match them after the leading non-wildcard characters is !*pString, which was already checked

		// we have a wildcard with wild character at the start.
	case '*':
		{
			// merge consecutive ? and *, since they are equivalent to a single *
			while (*pWildcard == '*' || *pWildcard == '?')
				++pWildcard;

			if (!*pWildcard)
				return true; // the rest of the string doesn't matter: the wildcard ends with *

			for (; *pString; ++pString)
				if (MatchWildcard(pString, pWildcard))
					return true;

			return false;
		}
	case '?':
		return MatchWildcard(pString+1, pWildcard + 1) || MatchWildcard(pString, pWildcard+1);
	default:
		assert (0);
		return false;
	}
}

// the type used to parse a yes/no string in the CAL file
enum YesNoType
{
	nYNT_Yes,
	nYNT_No,
	nYNT_Invalid
};

// parse the yes/no string in the cal file
inline YesNoType toYesNoType(const char* szString)
{
	if (!stricmp(szString, "yes")
		|| !stricmp(szString, "enable")
		|| !stricmp(szString, "1"))
		return nYNT_Yes;
	if (!stricmp(szString, "no")
		|| !stricmp(szString, "disable")
		|| !stricmp(szString, "0"))
		return nYNT_No;
	return nYNT_Invalid;
}

}

#endif