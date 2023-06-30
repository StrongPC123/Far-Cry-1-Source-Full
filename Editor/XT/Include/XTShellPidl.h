// XTShellPidl.h : header file
//
// This file is a part of the Xtreme Toolkit for MFC.
// ©1998-2003 Codejock Software, All Rights Reserved.
//
// This source code can only be used under the terms and conditions 
// outlined in the accompanying license agreement.
//
// support@codejock.com
// http://www.codejock.com
//
//////////////////////////////////////////////////////////////////////

#ifndef __XTSHELLPIDL_H__
#define __XTSHELLPIDL_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

LRESULT CALLBACK HookWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);

//////////////////////////////////////////////////////////////////////
// Summary: CXTShellPidl is a stand alone base class.  This class is used by the
//			shell tree and list controls to handle PIDL creation and management.
class _XT_EXT_CLASS CXTShellPidl
{
public:
	
    // Summary: Constructs a CXTShellPidl object.
	CXTShellPidl();

	// Summary: Destroys a CXTShellPidl object, handles cleanup and de-allocation.
    virtual ~CXTShellPidl();

public:

	// Input:	path - File system path string.
	// Returns: A pointer to an item ID list.  Returns NULL if it fails.
	// Summary:	This member function gets the fully qualified PIDL for the path string.
	LPITEMIDLIST IDLFromPath(CString path);

	// Input:	pidlPath - Fully qualified PIDL.
	// Returns: A fully qualified parent PIDL.
	// Summary:	This member function performs the OneUp or back function. 
	LPITEMIDLIST OneUpPIDL(LPITEMIDLIST pidlPath);

	// Input:	path - Path string.
	// Returns: A path string to the parent.
	// Summary:	This member function gets the parent folder using PIDLs. 
	CString	OneUpPATH(CString path);

	// Input:	pidl - PIDL list.
	// Returns: A pointer to the last IDL in the list.
	// Summary:	This member function walks an ITEMIDLIST and points to the last one.
	LPITEMIDLIST GetLastITEM(LPITEMIDLIST pidl);

	// Input:	pidl - Pointer to an ITEMIDLIST.
	// Returns: A new pointer to a copy of the PIDL.
	// Summary:	This member function copies a whole ITEMIDLIST.  Remember to Free()
	//			the old one if it is no longer needed. 
	LPITEMIDLIST CopyIDList(LPITEMIDLIST pidl);

	// Input:	pidl1 - Pointer to an item ID list.
	//			pidl2 - Pointer to an item ID list.
	// Returns: A pointer to an item ID list.
	// Summary:	This member function concatenates two PIDLs. 
    LPITEMIDLIST ConcatPidls(LPCITEMIDLIST pidl1,LPCITEMIDLIST pidl2);

	// Input:	lpsf - Pointer to the parent shell folder.
	//			lpi - Pointer to the item ID that is relative to 'lpsf'.
	// Returns: A pointer to an item ID list.
	// Summary:	This member function gets the fully qualified PIDLs for the specified
	//			folder. 
    LPITEMIDLIST GetFullyQualPidl(LPSHELLFOLDER lpsf,LPITEMIDLIST lpi);

	// Input:	lpMalloc - Points to the shell’s IMalloc interface.
	//			lpi - Pointer to item ID that is to be copied.
	// Returns: A pointer to an item ID list.
	// Summary:	This member function copies the ITEMID. 
    LPITEMIDLIST DuplicateItem(LPMALLOC lpMalloc,LPITEMIDLIST lpi);

	// Input:	lpsf - Pointer to the parent shell folder.
	//			lpi - Pointer to the item ID that is relative to 'lpsf'.
	//			dwFlags - Flags to determine which value to return.  See SHGNO for more details.
	//			lpFriendlyName - Buffer to receive the friendly name of the folder.
	// Returns: TRUE if successful, otherwise returns FALSE.
	// Summary:	This member function gets the friendly name for the folder or file.
    BOOL GetName(LPSHELLFOLDER lpsf,LPITEMIDLIST  lpi,DWORD dwFlags,TCHAR* lpFriendlyName);

	// Input:	cbSize - Initial size of the PIDL.
	// Returns: A pointer to an item ID list.
	// Summary:	This member function allocates a PIDL. 
    LPITEMIDLIST CreatePidl(UINT cbSize);

	// Input:	pidl - Pointer to an item ID list.
	// Returns: The number of item IDs in the list.
	// Summary:	This member function computes the number of item IDs in an item ID
	//			list. 
    UINT GetPidlItemCount(LPCITEMIDLIST pidl);

	// Input:	pidl - Pointer to an item ID list.
	// Returns: A pointer to the next PIDL item in the list.
	// Summary:	This member function gets the next PIDL in the list. 
    LPITEMIDLIST GetNextPidlItem(LPCITEMIDLIST pidl);

	// Input:	hwnd - Context menu owner.
	//			lpsfParent - Pointer to the parent shell folder.
	//			lpi - Pointer to the item ID that is relative to 'lpsfParent'.
	//			nCount - Number of PIDLs.
	//			lppt - Screen location of where to popup the menu.
	// Returns: TRUE on success, or FALSE on failure.
	// Summary:	This member function displays a popup context menu, given a parent
	//			shell folder, relative item ID, and screen location. 
	BOOL ShowContextMenu(HWND hwnd,LPSHELLFOLDER lpsfParent,LPCITEMIDLIST *lpi,int nCount,LPPOINT lppt);

	// Input:	hwnd - Context menu owner.
	//			lpsfParent - Pointer to the parent shell folder.
	//			lpi - Pointer to the item ID that is relative to 'lpsfParent'.
	//			lppt - Screen location of where to popup the menu.
	// Returns: TRUE on success, or FALSE on failure.
	// Summary:	This member function displays a popup context menu, given a parent
	//			shell folder, relative item ID, and screen location. 
	BOOL ShowContextMenu(HWND hwnd,LPSHELLFOLDER lpsfParent,LPITEMIDLIST lpi,LPPOINT lppt);

	// Input:	lpi - Fully qualified item ID list for the current item.
	//			uFlags - Flags for SHGetFileInfo().
	// Returns: An icon index for the current item.
	// Summary:	This member function gets the index for the current icon. Index is
	//			the index into the system image list. 
	int GetItemIcon(LPITEMIDLIST lpi,UINT uFlags);

	// Input:	psfFolder - A pointer to a valid IShellFolder data type.
	//			localPidl - A pointer to a valid _ITEMIDLIST structure.
	//			nCount - Number of items in the context menu.
	//			ppCM - Long pointer to a CONTEXTMENU struct.
	//			pcmType - A pointer to a valid int data type that represents the version number
	//			of the context menu.
	// Returns: An HRESULT value.
    // Summary:	This member function gets the IContextMenu, IContextMenu2 or IContextMenu3
	//			interface. 
	HRESULT GetSHContextMenu(LPSHELLFOLDER psfFolder,LPCITEMIDLIST *localPidl,int nCount,void** ppCM,int* pcmType);

protected:
	virtual void OnShowContextMenu(int idCmd, CMINVOKECOMMANDINFO& cmi);

private:
    
	static WNDPROC			m_pOldWndProc; // regular window proc
	static LPCONTEXTMENU2	m_pIContext2;  // active shell context menu

    static LRESULT CALLBACK HookWndProc(
		HWND hWnd, 
		UINT msg, 
		WPARAM wp, 
		LPARAM lp);

};

//////////////////////////////////////////////////////////////////////

#endif // __XTSHELLPIDL_H__
