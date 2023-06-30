// Registry.h: interface for the CRegistry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGISTRY_H__E7B8045A_B6E3_11D2_AF1A_00105A9F8688__INCLUDED_)
#define AFX_REGISTRY_H__E7B8045A_B6E3_11D2_AF1A_00105A9F8688__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef UBI_CREGISTRY_NAMEBUFFERSIZE
#define UBI_CREGISTRY_NAMEBUFFERSIZE _MAX_PATH+1
#endif // UBI_CREGISTRY_NAMEBUFFERSIZE

#include "StdAfx.h"
#include "GSTypes.h"


/*#ifdef WITHIN_THE_GLOBJECTLIB_PROJECT
class __declspec(dllexport) CRegistry
#else // WITHIN_THE_GLOBJECTLIB_PROJECT
class __declspec(dllimport) CRegistry : public CObject
#endif // WITHIN_THE_GLOBJECTLIB_PROJECT
*/
class CRegistry : public CObject
{
public:
	CRegistry();
	CRegistry(HKEY hKey, LPCTSTR lpszPath, GSbool bShouldCreateKeyIfNotExist = GS_TRUE, REGSAM samSecurity = KEY_ALL_ACCESS);
	virtual ~CRegistry();

	GSbool CreateKey(DWORD dwOptions = REG_OPTION_NON_VOLATILE);
	GSbool DeleteKey();
	GSbool OpenKey();
	GSbool CloseKey();
	GSbool DoesKeyExist();

	HKEY    SetRootKey(HKEY hRootKey = HKEY_LOCAL_MACHINE);
	CString SetPath(LPCTSTR lpszPath = "");
	REGSAM  SetSecurity(REGSAM samSecurity = KEY_ALL_ACCESS);
	GSvoid	SetShouldCreateKeyIfNotExist(GSbool bShouldCreateKeyIfNotExist = GS_TRUE);

	GSbool ReadBinary(LPCTSTR lpszValueName, LPVOID lpBuffer, GSuint uiBufferSize);
	GSbool ReadDWord (LPCTSTR lpszValueName, LPDWORD lpBuffer);
	GSbool ReadString(LPCTSTR lpszValueName, CString* pcsBuffer);
	GSbool EnumKeys(CStringList* pcslKeys);

	GSbool WriteBinary(LPCTSTR lpszValueName, LPVOID lpBuffer, GSuint uiBufferSize);
	GSbool WriteDWord (LPCTSTR lpszValueName, DWORD dwBuffer);
	GSbool WriteString(LPCTSTR lpszValueName, LPCTSTR lpszBuffer);

	GSbool DeleteValue(LPCTSTR lpszValueName);
	GSbool DeleteKeyValues();

protected:
	CString m_csPath;					// Subpath within the root key
	HKEY m_hRootKey;					// Root registry key
	HKEY m_hKey;						// Handle to current key
	REGSAM m_samSecurity;				// Security descriptor (access-rights specifier)

	GSbool m_bShouldCreateKeyIfNotExist;	// Flag indicating if key should be created if don't exist when it is to be opened
	GSbool m_bKeyOpen;					// Flag indicating wether or not the current key is open
};

#endif // !defined(AFX_REGISTRY_H__E7B8045A_B6E3_11D2_AF1A_00105A9F8688__INCLUDED_)
