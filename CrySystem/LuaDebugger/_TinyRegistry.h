#ifndef __TINY_REGISTRY_H__
#define __TINY_REGISTRY_H__

#pragma once

class _TinyRegistry {
public:
	_TinyRegistry() {};
	virtual ~_TinyRegistry() {};

	bool WriteNumber(const char *pszKey, const char *pszValueName, 
		             DWORD dwValue, HKEY hRoot = HKEY_CURRENT_USER) {
		HKEY hKey = _RegCreateKeyEx(pszKey, hRoot);
		if (hKey == NULL)
			return false;
		if (RegSetValueEx(hKey, pszValueName, 0, REG_DWORD, 
			(CONST BYTE *) &dwValue, sizeof(DWORD)) != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			_TINY_CHECK_LAST_ERROR
			return false;
		}
		RegCloseKey(hKey);
		return true;
	};
	bool WriteString(const char *pszKey, const char *pszValueName, 
		             const char *pszString, HKEY hRoot = HKEY_CURRENT_USER) {
		HKEY hKey = _RegCreateKeyEx(pszKey, hRoot);
		if (hKey == NULL)
			return false;
		if (RegSetValueEx(hKey, pszValueName, 0, REG_SZ, 
			(CONST BYTE *) pszString, strlen(pszString) + 1) != ERROR_SUCCESS) {
			RegCloseKey(hKey);
			_TINY_CHECK_LAST_ERROR
			return false;
		}
		RegCloseKey(hKey);
		return true;
	};

	bool ReadNumber(const char *pszKey, const char *pszValueName, DWORD& dwValOut,
		            DWORD dwValDefault = 0, HKEY hRoot = HKEY_CURRENT_USER) {
		HKEY hKey = _RegCreateKeyEx(pszKey, hRoot);
		DWORD dwType, dwSize = sizeof(DWORD);
		LONG lRet;
		if (hKey == NULL)
			return false;
		dwValOut = dwValDefault;
		lRet = RegQueryValueEx(hKey, pszValueName, NULL, &dwType, 
			(LPBYTE) &dwValOut, &dwSize);
		if (lRet != ERROR_SUCCESS || dwType != REG_DWORD) {
			RegCloseKey(hKey);
			return false;
		}
		RegCloseKey(hKey);
		return true;
	}

protected:
	HKEY _RegCreateKeyEx(const char *pszKey, HKEY hRoot = HKEY_CURRENT_USER) {
		LONG lRes;
		HKEY hKey;
		DWORD dwDisp;
		lRes = RegCreateKeyEx(hRoot, pszKey, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);
		if (lRes != ERROR_SUCCESS) {
			_TINY_CHECK_LAST_ERROR
			return NULL;
		}
		return hKey;
	}

};

#endif