// FileEnum.h: Interface of the class CFileEnum.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEENUM_H__9A4EB661_8991_11D4_8100_D1626940A160__INCLUDED_)
#define AFX_FILEENUM_H__9A4EB661_8991_11D4_8100_D1626940A160__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CFileEnum  
{
public:
	static bool ScanDirectory( const CString &path,const CString &file,std::vector<CString> &files, bool recursive=true );

	bool GetNextFile(struct __finddata64_t *pFile);
	bool StartEnumeration( const char *szEnumPathAndPattern, __finddata64_t *pFile);
	bool StartEnumeration( const char *szEnumPath, char szEnumPattern[], __finddata64_t *pFile);
	CFileEnum();
	virtual ~CFileEnum();
protected:
	intptr_t m_hEnumFile;
};

#endif // !defined(AFX_FILEENUM_H__9A4EB661_8991_11D4_8100_D1626940A160__INCLUDED_)
