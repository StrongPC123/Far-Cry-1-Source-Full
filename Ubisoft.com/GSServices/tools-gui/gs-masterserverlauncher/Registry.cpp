// Registry.cpp: implementation of the CRegistry class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Registry.h"

#ifdef _DEBUG
#undef THIS_FILE
static GSchar THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//	Function:	CRegistry()
//
//	Purpose:	Create a CRegistry object. If you use the default constructor,
//				(this one), you should consider calling these three functions 
//				before opening a key or accessing key data:
//					- SetRootKey()
//					- SetPath()
//					- SetSecurity()
//				If you fail to do so, you will use the default values, which
//				is really not a great idea...
//
//	Parameters:	none.
//
//	RetValue:	n/a
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
CRegistry::CRegistry()
{
	m_hRootKey = HKEY_LOCAL_MACHINE;
	m_hKey = NULL;
	m_csPath = "Software\\";
	m_samSecurity = KEY_ALL_ACCESS;
	m_bShouldCreateKeyIfNotExist = GS_TRUE;
	m_bKeyOpen = GS_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	CRegistry()
//
//	Purpose:	Create a CRegistry object. Opens the registry key according to
//				supplied parameters.
//
//	Parameters:	[HKEY] hRootKey:		Root key to open. Choose from these:
//											HKEY_CLASSES_ROOT
//											HKEY_CURRENT_CONFIG
//											HKEY_CURRENT_USER
//											HKEY_LOCAL_MACHINE
//											HKEY_USERS
//											HKEY_PERFORMANCE_DATA (Windows NT)
//											HKEY_DYN_DATA (Windows 95/98)
//
//				[CString] csPath:		Path of Subkey to open.
//
//				[GSbool] bShouldCreateKeyIfNotExist:
//										Indicated wether to create the key
//										in case it doesn't exist.
//
//				[REGSAM] samSecurity:	Security attributes or restrictions.
//										Choose at least one of these attributes:
//											KEY_ALL_ACCESS 
//												Combination of:
//													KEY_QUERY_VALUE, 
//													KEY_ENUMERATE_SUB_KEYS,
//													KEY_NOTIFY,
//													KEY_CREATE_SUB_KEY,
//													KEY_CREATE_LINK, and
//													KEY_SET_VALUE access.
//											KEY_CREATE_LINK
//												Permission to create a symbolic link.
//											KEY_CREATE_SUB_KEY
//												Permission to create subkeys.
//											KEY_ENUMERATE_SUB_KEYS
//												Permission to enumerate subkeys.
//											KEY_EXECUTE
//												Permission for read access.
//											KEY_NOTIFY
//												Permission for change notification.
//											KEY_QUERY_VALUE
//												Permission to query subkey data. 
//											KEY_READ
//												Combination of:
//													KEY_QUERY_VALUE,
//													KEY_ENUMERATE_SUB_KEYS, and
//													KEY_NOTIFY access. 
//											KEY_SET_VALUE
//												Permission to set subkey data. 
//											KEY_WRITE
//												Combination of
//													KEY_SET_VALUE and
//													KEY_CREATE_SUB_KEY access. 
//
//										If no paramater is supplied, the default
//										is KEY_ALL_ACCESS
//
//	RetValue:	n/a
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
CRegistry::CRegistry(HKEY hRootKey, LPCTSTR lpszPath, GSbool bShouldCreateKeyIfNotExist, REGSAM samSecurity)
{
	m_hRootKey = hRootKey;
	m_hKey = NULL;
	m_csPath = lpszPath;
	m_samSecurity = samSecurity;
	m_bShouldCreateKeyIfNotExist = bShouldCreateKeyIfNotExist;
	m_bKeyOpen = GS_FALSE;

	OpenKey();
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	~CRegistry()
//
//	Purpose:	Destructor. If key is still open, the destructor closes it.
//
//	Parameters:	none.
//
//	RetValue:	n/a
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
CRegistry::~CRegistry()
{
	if(m_bKeyOpen)
		CloseKey();
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	CreateKey()
//
//	Purpose:	Creates a registry key and opens it. If the key Already existed,
//				it just opens it.
//
//				See also:
//					OpenKey, CloseKey()
//
//	Parameters:	none.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::CreateKey(DWORD dwOptions)
{
	DWORD dwDisposition;

	if(m_bKeyOpen)
		CloseKey();

	m_bKeyOpen = (RegCreateKeyEx(m_hRootKey,
		(LPCTSTR)m_csPath, 
		0,
		"",
		dwOptions,
		m_samSecurity,
		NULL,
		&m_hKey,
		&dwDisposition) == ERROR_SUCCESS);

	return m_bKeyOpen;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	DeleteKey()
//
//	Purpose:	Deletes the current key, including any values or subkey it 
//				contains. This is of course done recursively and while the keys
//				are destroyed, they should not be accessed  by any other thread
//				or application, or else, the destruction may not be completed
//				succesfully.
//
//				See also:
//					OpenKey(), CloseKey()
//
//	Parameters:	none.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::DeleteKey()
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSchar szSubKeyName[UBI_CREGISTRY_NAMEBUFFERSIZE];
	DWORD dwSubKeyNameSize = UBI_CREGISTRY_NAMEBUFFERSIZE;
	DWORD dwIndex;
	DWORD dwNumberOfSubKeys;
	CStringList cslSubKeys;

	if(!bKeyWasAlreadyOpen)
		if(!OpenKey())
			return GS_FALSE;

	if(RegQueryInfoKey(m_hKey,
		NULL,
		NULL,
		NULL,
		&dwNumberOfSubKeys,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL) != ERROR_SUCCESS)
	{
		if(!bKeyWasAlreadyOpen)
			CloseKey();

		return GS_FALSE;
	}

	for(dwIndex = 0; dwIndex < dwNumberOfSubKeys; dwIndex++)
	{
		if(RegEnumKeyEx(m_hKey, 
			dwIndex, 
			szSubKeyName, 
			&dwSubKeyNameSize, 
			0, 
			NULL, 
			NULL, 
			NULL) != ERROR_SUCCESS)
		{
			if(!bKeyWasAlreadyOpen)
				CloseKey();
			return GS_FALSE;
		}

		cslSubKeys.AddTail(szSubKeyName);

		dwSubKeyNameSize = UBI_CREGISTRY_NAMEBUFFERSIZE;
	}

	for(dwIndex = 0; dwIndex < dwNumberOfSubKeys; dwIndex++)
	{	
		CRegistry regToDelete(m_hRootKey, 
			m_csPath + "\\" + cslSubKeys.RemoveHead(), 
			GS_FALSE, 
			KEY_ALL_ACCESS);
		
		if(!regToDelete.DeleteKey())
		{
			if(!bKeyWasAlreadyOpen)
				CloseKey();
			return GS_FALSE;
		}
	}

	if(!DeleteKeyValues())
	{
		if(!bKeyWasAlreadyOpen)
			CloseKey();
		return GS_FALSE;
	}

	// Delete the current Key
	CloseKey();
	CString path(m_csPath.Left(m_csPath.ReverseFind('\\')));
	HKEY hKey;
	if(RegOpenKeyEx(m_hRootKey, 
			path, 
			0, 
			KEY_ALL_ACCESS, 
			&hKey) != ERROR_SUCCESS)
		return GS_FALSE;
	CString keyname(m_csPath.Right(m_csPath.GetLength() - (m_csPath.ReverseFind('\\') + 1)));
	if(RegDeleteKey(hKey, keyname) != ERROR_SUCCESS)
		return GS_FALSE;

	m_csPath = "";

	return GS_TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	OpenKey()
//
//	Purpose:	Opens the registry key. If the key was already open, it is
//				automatically closed. You should open the key before accessing
//				it only if you intend to read or write many data items. If you
//				just want to write or read once, let the object handle it for
//				you: note that the access functions will open the key and close
//				the key if it was not already opened. Only call OpenKey if you
//				want to accelerate multiple consecutive data accesses.
//
//				If the key does not exist and the bShouldCreateKeyNotExist flag
//				is set to GS_TRUE, then the key is created then opened.
//
//				See also:
//					CreateKey(), CloseKey()
//
//	Parameters:	none.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::OpenKey()
{
	if(m_bKeyOpen)
		CloseKey();

	if(m_bShouldCreateKeyIfNotExist)
	{
		m_bKeyOpen = CreateKey();
	}

	if(!m_bKeyOpen)
	{
		HKEY hKeyParent = m_hRootKey;
		HKEY hKeyChild = NULL;

		GSchar* lpszPath = new GSchar[m_csPath.GetLength() + 1];
		strcpy(lpszPath, (LPCTSTR)m_csPath);

		GSchar* lpszToken;

		lpszToken = strtok(lpszPath, "\\");

		while(lpszToken)
		{
			if(::RegOpenKeyEx(hKeyParent,
				(LPCTSTR)lpszToken,
				0,
				m_samSecurity,
				&hKeyChild) != ERROR_SUCCESS)
			{
				::RegCloseKey(hKeyParent);
				goto hell;
			}

			::RegCloseKey(hKeyParent);

			hKeyParent = hKeyChild;

			lpszToken = strtok( NULL, "\\");
		}

		// Set the Key;
		m_hKey = hKeyParent;
		m_bKeyOpen = GS_TRUE;
hell:
		delete lpszPath;
	}

	return m_bKeyOpen;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	CloseKey()
//
//	Purpose:	Closes an open registry key. Only call when you previously
//				called OpenKey(). If the key is still open when the object is
//				destroyed, the destructor automatically closes it.
//
//				See also:
//					OpenKey(), CreateKey
//
//	Parameters:	none.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::CloseKey()
{
	if(!m_bKeyOpen)
		return GS_FALSE;

	if(RegFlushKey(m_hKey) != ERROR_SUCCESS)
		return GS_FALSE;

	m_bKeyOpen = GS_FALSE;
	return(RegCloseKey(m_hKey) == ERROR_SUCCESS);
}


////////////////////////////////////////////////////////////////////////////////
//	Function:	DoesKeyExist()
//
//	Purpose:	Checks if the current key exists. It leaves it in the state it
//				found it.
//
//	Parameters:	none.
//
//	RetValue:	GS_TRUE
//					"Yes!".
//				GS_FALSE
//					"No!".
///////////////////Created/1999/02/08/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::DoesKeyExist()
{
	GSbool bOldShouldCreate = m_bShouldCreateKeyIfNotExist;

	// If key is open, then it exists...
	if(m_bKeyOpen)
		return GS_TRUE;

	// Remember old setting
	SetShouldCreateKeyIfNotExist(GS_FALSE);
	
	// Try to open the key
	if(OpenKey())
	{
		// If key opened successfully, close it
		CloseKey();
		SetShouldCreateKeyIfNotExist(bOldShouldCreate);
		return GS_TRUE;
	}
	else
	{
		SetShouldCreateKeyIfNotExist(bOldShouldCreate);
		return GS_FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	SetRootKey()
//
//	Purpose:	Sets the root key to use. Call after using default constructor
//				or if you want to reuse a CRegistry object on another registry
//				key. If a key was open, it automatically is closed. You will
//				need to call OpenKey() in order to access the registry. If a NULL
//				Parameter is supplied, SetRootKey() returns the current value
//				without changing it.
//
//				See also:
//					OpenKey(), SetPath(), SetSecurity()
//
//	Parameters:	[HKEY] hRootKey:		Root key to use. Choose from these:
//											HKEY_CLASSES_ROOT
//											HKEY_CURRENT_CONFIG
//											HKEY_CURRENT_USER
//											HKEY_LOCAL_MACHINE
//											HKEY_USERS
//											HKEY_PERFORMANCE_DATA (Windows NT)
//											HKEY_DYN_DATA (Windows 95/98)
//
//										If a NULL value is supplied, the root key
//										value is not modified.
//
//										If no paramater is supplied, the default
//										is HKEY_LOCAL_MACHINE.
//
//	RetValue:	The old root key value.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
HKEY CRegistry::SetRootKey(HKEY hRootKey)
{
	HKEY oldRootKey = m_hRootKey;

	// If parameter is NULL, do nothing and return current value
	if(hRootKey)
	{	if(m_bKeyOpen)
			CloseKey();

		m_hRootKey = hRootKey;
	}
	
	return oldRootKey;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	SetPath()
//
//	Purpose:	Sets the key path to use. Call after using default constructor
//				or if you want to reuse a CRegistry object on another registry
//				key. If a key was open, it automatically is closed. You will
//				need to call OpenKey() in order to access the registry. If a NULL
//				Parameter is supplied, SetPath() returns the current value
//				without changing it.
//
//				See also:
//					OpenKey(), SetRootKey(), SetSecurity()
//
//	Parameters:	[CString] csPath:		Path to use.
//
//										If a NULL value is supplied, the root key
//										value is not modified.
//
//										If no paramater is supplied, the default
//										is "Software\\".
//
//										Always use GSdouble backslashes, but none at
//										the end of the path. "Software\\bla\\bobo"
//
//	RetValue:	The old path value.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
CString CRegistry::SetPath(LPCTSTR lpszPath /* = "" */)
{
	CString oldPath(m_csPath);

	// If parameter is NULL, do nothing and return current value
	if(lpszPath)
	{
		if(m_bKeyOpen)
			CloseKey();

		m_csPath = lpszPath;
	}

	return oldPath;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	SetSecurity()
//
//	Purpose:	Sets the security attributes to use. Call after using default 
//				constructor or if you want to reuse a CRegistry object on 
//				another registry key. If a key was open, it automatically is 
//				closed. You will need to call OpenKey() in order to access the 
//				registry. If a NULL	Parameter is supplied, SetPath() returns
//				the current value without changing it.
//
//				See also:
//					OpenKey(), SetRootKey(), SetPath()
//
// Parameter:	[REGSAM] samSecurity:	Security attributes or restrictions.
//										Choose at least one of these attributes:
//											KEY_ALL_ACCESS 
//												Combination of:
//													KEY_QUERY_VALUE, 
//													KEY_ENUMERATE_SUB_KEYS,
//													KEY_NOTIFY,
//													KEY_CREATE_SUB_KEY,
//													KEY_CREATE_LINK, and
//													KEY_SET_VALUE access.
//											KEY_CREATE_LINK
//												Permission to create a symbolic link.
//											KEY_CREATE_SUB_KEY
//												Permission to create subkeys.
//											KEY_ENUMERATE_SUB_KEYS
//												Permission to enumerate subkeys.
//											KEY_EXECUTE
//												Permission for read access.
//											KEY_NOTIFY
//												Permission for change notification.
//											KEY_QUERY_VALUE
//												Permission to query subkey data. 
//											KEY_READ
//												Combination of:
//													KEY_QUERY_VALUE,
//													KEY_ENUMERATE_SUB_KEYS, and
//													KEY_NOTIFY access. 
//											KEY_SET_VALUE
//												Permission to set subkey data. 
//											KEY_WRITE
//												Combination of
//													KEY_SET_VALUE and
//													KEY_CREATE_SUB_KEY access. 
//
//										If no paramater is supplied, the default
//										is KEY_ALL_ACCESS
//
//										If a NULL value is supplied, the root key
//										value is not modified.
//
//	RetValue:	The old security value.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
REGSAM CRegistry::SetSecurity(REGSAM samSecurity)
{
	REGSAM oldSecurity = m_samSecurity;
	
	if(samSecurity)
	{
		if(m_bKeyOpen)
			CloseKey();
		m_samSecurity = samSecurity;
	}

	return oldSecurity;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	SetShouldCreateKeyIfNotExist()
//
//	Purpose:	Sets if when you want to open a key and it does not exist, if
//				the key should be created.
//
//	Parameters:	[GSbool] bShouldCreateKeyIfNotExist:
//							Indicates if you want to ...
//
//	RetValue:	none.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSvoid CRegistry::SetShouldCreateKeyIfNotExist(GSbool bShouldCreateKeyIfNotExist)
{
	m_bShouldCreateKeyIfNotExist = bShouldCreateKeyIfNotExist;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	ReadBinary()
//
//	Purpose:	Reads binary data from the registry. If a key was open, the
//				function does not close that key after use. If the registry key
//				was not open, the function opens it and closes it after use.
//				All the registry access function like this and the purpose of
//				this is simple: if you want to read or write a single value, 
//				don't open the key before calling the access function (this
//				function), but if you want to read or write more than once, call
//				OpenKey() before the accesses and CloseKey() after them; this
//				will ensure that the keys are not open and closed at every
//				function call.
//
//				See also:
//					OpenKey(), CloseKey()
//
//	Parameters:	[CString] csValueName:			The name of the value to query.
//
//				[LPVOID] lpBuffer:				Void pointer to the data buffer.
//												The buffer will be filled with
//												the binary data.
//
//				[unsigned GSint] lpBufferSize:	Pointer to a GSuint containing the
//												buffer size (in bytes). Note
//												that this value will be replaced
//												with the actual amount of bytes
//												copied into the buffer.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::ReadBinary(LPCTSTR lpszValueName, LPVOID lpBuffer,GSuint uiBufferSize)
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSbool bResult;

	if(!bKeyWasAlreadyOpen)
		OpenKey();

	bResult = (RegQueryValueEx(m_hKey,
		lpszValueName,
		0,
		NULL,
		(LPBYTE)lpBuffer,
		(LPDWORD)&uiBufferSize) == ERROR_SUCCESS);

	if(!bKeyWasAlreadyOpen)
		CloseKey();

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	ReadDWord()
//
//	Purpose:	Reads a GSdouble word from the registry. If a key was open, the
//				function does not close that key after use. If the registry key
//				was not open, the function opens it and closes it after use.
//				All the registry access function like this and the purpose of
//				this is simple: if you want to read or write a single value, 
//				don't open the key before calling the access function (this
//				function), but if you want to read or write more than once, call
//				OpenKey() before the accesses and CloseKey() after them; this
//				will ensure that the keys are not open and closed at every
//				function call.
//
//				See also:
//					OpenKey(), CloseKey()
//
//	Parameters:	[CString] csValueName:			The name of the value to query.
//
//				[LPDWORD] lpBuffer:				Pointer to the 4 byte buffer
//												that will hold the numeric data
//												in GSdouble word format.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::ReadDWord(LPCTSTR lpszValueName, LPDWORD lpBuffer)
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSbool bResult;
	DWORD dwBufferSize = sizeof(DWORD);

	if(!bKeyWasAlreadyOpen)
		OpenKey();

	bResult = (RegQueryValueEx(m_hKey,
		lpszValueName,
		0,
		NULL,
		(LPBYTE)lpBuffer,
		&dwBufferSize) == ERROR_SUCCESS);

	if(!bKeyWasAlreadyOpen)
		CloseKey();

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	ReadString()
//
//	Purpose:	Reads a character string from the registry. If a key was open, 
//				the function does not close that key after use. If the registry
//				key	was not open, the function opens it and closes it after use.
//				All the registry access function like this and the purpose of
//				this is simple: if you want to read or write a single value, 
//				don't open the key before calling the access function (this
//				function), but if you want to read or write more than once, call
//				OpenKey() before the accesses and CloseKey() after them; this
//				will ensure that the keys are not open and closed at every
//				function call.
//
//				See also:
//					OpenKey(), CloseKey()
//
//	Parameters:	[CString] csValueName:			The name of the value to query.
//
//				[CString] plpBuffer:			Pointer to CString object that
//												will recieve text read from the
//												registry.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::ReadString(LPCTSTR lpszValueName, CString* pcsBuffer)
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSbool bResult;
	GSchar swBuffer[UBI_CREGISTRY_NAMEBUFFERSIZE];
	DWORD dwBufferSize = UBI_CREGISTRY_NAMEBUFFERSIZE;

	if(!bKeyWasAlreadyOpen)
		OpenKey();

	if(bResult = (RegQueryValueEx(m_hKey,
		lpszValueName,
		0,
		NULL,
		(PBYTE)swBuffer,
		(LPDWORD)&dwBufferSize) == ERROR_SUCCESS))
	{
		*pcsBuffer = swBuffer;
	}

	if(!bKeyWasAlreadyOpen)
		CloseKey();

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	EnumKeys()
//
//	Purpose:	
//
// Parameters: [CStringList] pcslKeys: Pointer to CStringList object that
//                                        will receive the availlable
//                                        sub-keys of the current key.
//
// RetValue:   GS_TRUE
//                On success.
//             GS_FALSE
//                On failure.
///////////////////Created/1999/09/23/by/Antoine/Boivin/Filion/UbiSoft/Montréal/
GSbool CRegistry::EnumKeys(CStringList* pcslKeys)
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSchar szValueName[UBI_CREGISTRY_NAMEBUFFERSIZE];
	DWORD dwIndex;
	DWORD dwNumberOfValues, dwKeyMaxLength, 
	dwValueNameSize = UBI_CREGISTRY_NAMEBUFFERSIZE;

	if(RegQueryInfoKey(m_hKey,             // handle sur cle
		NULL,               // class
		NULL,
		NULL,               // reserved
		&dwNumberOfValues,  // nb subkeys
		&dwKeyMaxLength,    // longueur max des cles
		NULL,               // values
		NULL,               // values length
		NULL,               // data length
		NULL,
		NULL,
		NULL) != ERROR_SUCCESS)
	goto end;

	
	// read all strings
	for(dwIndex = 0; dwIndex < dwNumberOfValues; dwIndex++)
	{
		if(RegEnumKeyEx(m_hKey, 
			dwIndex, 
			szValueName,
			&dwValueNameSize, 
			NULL,  // reserved
			NULL, 
			NULL, 
			NULL) != ERROR_SUCCESS)
		{
			goto end;
		}
      
		// store string
		pcslKeys->AddTail(szValueName);

		// reset container size
		dwValueNameSize = UBI_CREGISTRY_NAMEBUFFERSIZE;
	}
	return GS_TRUE;

end:
	if(!bKeyWasAlreadyOpen)
		CloseKey();
	return GS_FALSE;
}


////////////////////////////////////////////////////////////////////////////////
//	Function:	WriteBinary()
//
//	Purpose:	Reads binary data from the registry. If a key was open, the
//				function does not close that key after use. If the registry key
//				was not open, the function opens it and closes it after use.
//				All the registry access function like this and the purpose of
//				this is simple: if you want to read or write a single value, 
//				don't open the key before calling the access function (this
//				function), but if you want to read or write more than once, call
//				OpenKey() before the accesses and CloseKey() after them; this
//				will ensure that the keys are not open and closed at every
//				function call.
//
//				See also:
//					OpenKey(), CloseKey()
//
//	Parameters:	[CString] csValueName:			The name of the value to write.
//
//				[LPVOID] lpBuffer:				Void pointer to the data buffer.
//												that contains the binary data.
//
//				[unsigned GSint] uiBufferSize:	Pointer to a GSuint containing the
//												buffer size (in bytes). Note
//												that this value will be replaced
//												with the actual amount of bytes
//												copied into the registry.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::WriteBinary(LPCTSTR lpszValueName, LPVOID lpBuffer,GSuint uiBufferSize)
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSbool bResult;
	DWORD dwReserved = 0;
	DWORD dwType = REG_BINARY;

	if(!bKeyWasAlreadyOpen)
		OpenKey();

	bResult = (RegSetValueEx(m_hKey,
		lpszValueName,
		dwReserved,
		dwType,
		(LPBYTE)lpBuffer,
		(DWORD)uiBufferSize) == ERROR_SUCCESS);

	if(!bKeyWasAlreadyOpen)
		CloseKey();

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	Write DWord()
//
//	Purpose:	Writes a GSdouble word to the registry. If a key was open, the
//				function does not close that key after use. If the registry key
//				was not open, the function opens it and closes it after use.
//				All the registry access function like this and the purpose of
//				this is simple: if you want to read or write a single value, 
//				don't open the key before calling the access function (this
//				function), but if you want to read or write more than once, call
//				OpenKey() before the accesses and CloseKey() after them; this
//				will ensure that the keys are not open and closed at every
//				function call.
//
//				See also:
//					OpenKey(), CloseKey()
//
//	Parameters:	[CString] csValueName:			The name of the value to write.
//
//				[DWORD] dwBuffer:				DWORD to be written to the
//												registry.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::WriteDWord(LPCTSTR lpszValueName, DWORD dwBuffer)
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSbool bResult;
	DWORD dwReserved = 0;
	DWORD dwType = REG_DWORD;

	if(!bKeyWasAlreadyOpen)
		OpenKey();

	bResult = (RegSetValueEx(m_hKey,
		lpszValueName,
		dwReserved,
		dwType,
		(LPBYTE)&dwBuffer,
		(DWORD)sizeof(DWORD)) == ERROR_SUCCESS);

	if(!bKeyWasAlreadyOpen)
		CloseKey();

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	WriteString()
//
//	Purpose:	Writes a character string from the registry. If a key was open, 
//				the function does not close that key after use. If the registry
//				key	was not open, the function opens it and closes it after use.
//				All the registry access function like this and the purpose of
//				this is simple: if you want to read or write a single value, 
//				don't open the key before calling the access function (this
//				function), but if you want to read or write more than once, call
//				OpenKey() before the accesses and CloseKey() after them; this
//				will ensure that the keys are not open and closed at every
//				function call.
//
//				See also:
//					OpenKey(), CloseKey()
//
//	Parameters:	[CString] csValueName:			The name of the value to write.
//
//				[CString] csBuffer:			CString object containing text
//												to be written to the registry.
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::WriteString(LPCTSTR lpszValueName, LPCTSTR lpszBuffer)
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSbool bResult;
	DWORD dwReserved = 0;
	DWORD dwType = REG_SZ;

	if(!bKeyWasAlreadyOpen)
		OpenKey();

	bResult = (RegSetValueEx(m_hKey,
		lpszValueName,
		0,
		dwType,
		(GSuchar*)lpszBuffer,
		strlen(lpszBuffer)+1) == ERROR_SUCCESS);

	if(!bKeyWasAlreadyOpen)
		CloseKey();

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	DeleteValue()
//
//	Purpose:	Deletes A value from the current key. Note that the key must
//				have been previously open with sufficient security attributes.
//
//				See also:
//					OpenKey()
//
//	Parameters:	[CString] csValueName:			The name of the value to remove.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::DeleteValue(LPCTSTR lpszValueName)
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSbool bResult;

	if(!bKeyWasAlreadyOpen)
		OpenKey();

	bResult = (RegDeleteValue(m_hKey, lpszValueName) == ERROR_SUCCESS);

	if(!bKeyWasAlreadyOpen)
		CloseKey();

	return bResult;
}

////////////////////////////////////////////////////////////////////////////////
//	Function:	DeleteKeyValues()
//
//	Purpose:	Removes all the value entries from the current key. Note that
//				no other accesses should be made (by other threads etc.) to the
//				current key while the values are being deleted. The key has to
//				have been opened with the sufficient security attributes.
//
//				See also:
//					OpenKey()
//
//	Parameters:	none.
//
//	RetValue:	GS_TRUE
//					On success.
//				GS_FALSE
//					On failure.
///////////////////Created/1999/01/28/by/Jean-Pierre/Martineau/UbiSoft/Montréal/
GSbool CRegistry::DeleteKeyValues()
{
	GSbool bKeyWasAlreadyOpen = m_bKeyOpen;
	GSchar szValueName[UBI_CREGISTRY_NAMEBUFFERSIZE];
	DWORD dwValueNameSize = UBI_CREGISTRY_NAMEBUFFERSIZE;
	DWORD dwIndex;
	DWORD dwNumberOfValues;
	CStringList cslValues;

	if(!bKeyWasAlreadyOpen)
		if(!OpenKey())
			return GS_FALSE;

	if(RegQueryInfoKey(m_hKey,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&dwNumberOfValues,
		NULL,
		NULL,
		NULL,
		NULL) != ERROR_SUCCESS)
	{
		if(!bKeyWasAlreadyOpen)
			CloseKey();

		return GS_FALSE;
	}

	for(dwIndex = 0; dwIndex < dwNumberOfValues; dwIndex++)
	{
		if(RegEnumValue(m_hKey, 
			dwIndex, 
			szValueName, 
			&dwValueNameSize, 
			0, 
			NULL, 
			NULL, 
			NULL) != ERROR_SUCCESS)
		{
			if(!bKeyWasAlreadyOpen)
				CloseKey();
			return GS_FALSE;
		}

		cslValues.AddTail(szValueName);

		dwValueNameSize = UBI_CREGISTRY_NAMEBUFFERSIZE;
	}

	for(dwIndex = 0; dwIndex < dwNumberOfValues; dwIndex++)
	{
		if(!DeleteValue(cslValues.RemoveHead()))
		{
			if(!bKeyWasAlreadyOpen)
				CloseKey();
			return GS_FALSE;
		}
	}

	if(!bKeyWasAlreadyOpen)
		CloseKey();

	return GS_TRUE;
}