// XTAccelManager.h : interface for the CXTAccelManager class.
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

#if !defined(__XTACCELMANAGER_H_)
#define __XTACCELMANAGER_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Summary: CMap definition for mapping accelerator categories.
typedef CMap<UINT, UINT, CString, LPCTSTR> CXTMapAccelCategory;

//////////////////////////////////////////////////////////////////////
// Summary: CXTAccelSwapOutItemList is a CMap derived class.  This class is used
//			to hold a list of swap-out commands.
class _XT_EXT_CLASS CXTAccelSwapOutItemList : public CXTMapAccelCategory {};

// Summary: XT_EXTRA_ACCELITEM is a stand alone structure class.  It is used to
//			create a structure that represents an accelerator item.
struct XT_EXTRA_ACCELITEM
{
	UINT	nCommandID;	// The command ID that this item represents.
	CString szCategory;	// The category, or top level menu name, that this command belongs to.
	CString szName;		// The name, or menu item text, that this item represents.
};

// Summary: XT_VKEYMAP is a stand alone structure class.  It is used to define a
//			structure used to map virtual key codes to their human readable names.
struct XT_VKEYMAP
{
	WORD	wKey;		// Virtual Key Code.
	LPCTSTR szKeyName;	// Display Name (i.e "CTRL").
};

// Summary: XT_CATEGORY is a stand alone structure class.  It is used by the
//			accelerator manager to map a category title with an associated HMENU
//			handle.
struct XT_CATEGORY
{    
    HMENU	hSubMenu;		// Handle to the menu associated with the category.
	CString strCategory;	// Title of the menu category.
};

// Summary: CList definition for the category structure array.
typedef CList<XT_CATEGORY,XT_CATEGORY&> CXTCategoryList;

//////////////////////////////////////////////////////////////////////
// Summary: CXTMapString is a CObject derived helper class.  It is used to map IDs
//			to command strings.
class CXTMapString : public CObject
{
public:
	
	UINT	m_nID;		// ID of the command to map.
	CString	m_szName;	// A NULL terminated string.

	// Input:	item - A reference to a valid CXTMapString object.
	// Returns: A reference to a valid CXTMapString object.
	// Summary: Use this assignment operator to initialize a CXTMapString object.
    const CXTMapString& operator=(const CXTMapString& item);
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE const CXTMapString& CXTMapString::operator=(const CXTMapString& item) {
	m_nID = item.m_nID; m_szName = item.m_szName; return *this;
}

// Summary: CList definition for the map string object array.
typedef CList<CXTMapString, CXTMapString&> CXTMapStringList;

// Summary: CList definition for the extra accelitem struct array.
typedef CList<XT_EXTRA_ACCELITEM*, XT_EXTRA_ACCELITEM*> CXTExtraItems;

// Summary: CMap definition for mapping command exclude lists.
typedef CMap<UINT, UINT, UINT, UINT> CXTMapCommandExcludeList;

// Summary: CMap definition for mapping accelerator accelerator swapo ut item lists.
typedef CMap<UINT, UINT, CXTAccelSwapOutItemList*, CXTAccelSwapOutItemList*&> CXTSwapOutList;

////////////////////////////////////////////////////////////////////// 
// Summary: CXTAccelManager is a stand alone class.  It encapsulates the configurable
//			keyboard accelerator functions.  
class _XT_EXT_CLASS CXTAccelManager
{
private:

    // Summary: Constructs a CXTAccelManager object.
	CXTAccelManager();

public:

	// Summary: Destroys a CXTAccelManager object, handles cleanup and de-allocation.
	virtual ~CXTAccelManager();

protected:
	
	//////////////////////////////////////////////////////////////////////
	// Frame window binding information
	//////////////////////////////////////////////////////////////////////
	
	CFrameWnd* m_pFrameWnd; // A CFrameWnd pointer to the frame window that the accelerators are connected to.

	//////////////////////////////////////////////////////////////////////
	// String list of command categories (top level menu items).
	//////////////////////////////////////////////////////////////////////
	
    CXTCategoryList          m_comboCategoryList;     // A list used to map command IDs to display strings.
    CXTMapStringList         m_mapAccelString;        // A list of command IDs to display categories.
    CXTMapAccelCategory      m_mapAccelCategory;      // A map of command IDs to exclude from the string list.
    CXTMapCommandExcludeList m_MapCommandExcludeList; // A map of swap-out command IDs.
    CXTSwapOutList           m_SwapOutList;           // A map of extra commands for each category.
    CXTExtraItems            m_ExtraItems;            // A list of additional commands that have accelerator definitions.

	//////////////////////////////////////////////////////////////////////
	// Our accelerator buffer
	//////////////////////////////////////////////////////////////////////

    int               m_nAccelBufferCount;  // The count of accelerators pointed to by 'm_pAccelBuffer'.
    int               m_nAccelDefaultCount; // The count of accelerators pointed to by 'm_pAccelDefault'.
    bool              m_bDefaultInUse;      // true if the 'm_pAccelBuffer' currently contains a copy of the default accelerators.
    bool              m_bChanged;           // true if the contents of 'm_pAccelBuffer' has been changed.
    bool              m_bAutoSave;          // true if the accelerator data is automatically saved when this class is destroyed.
    bool              m_bInitialized;       // true if the accelerator data has been initialized for the frame object.
    LPACCEL           m_pAccelBuffer;       // The array of accelerators.
    LPACCEL           m_pAccelDefault;      // The array of default accelerators.
    static XT_VKEYMAP m_VirtSysKeys[];      // Virtual System Key ID to Name mappings.
    static XT_VKEYMAP m_VirtKeys[];         // Virtual Key ID to Name mappings.

public:

	// Example:	<pre>CXTAccelManager::Get().Init( this );</pre>
	// Returns: A reference to the one and only CXTAccelManager object.
	// Summary: This static member function will retrieve a reference to the one
	//			and only CXTAccelManager object.  You can use this function to access
	//			data members for the CXTAccelManager class.
    static CXTAccelManager& Get();

	// Returns: true if the accelerator data has been initialized, otherwise it returns false.
	// Summary: This member function will check to see if the accelerator data has been
	//			initialized. 
	bool IsInitialized();

	// Input:	pWnd - A CFrameWnd pointer to the frame window whose keyboard shortcuts
	//			should be managed.
	// Returns: true if successful, otherwise returns false.
	// Summary: This member function initializes the accelerator manager and loads default
	//			and user defined accelerator tables.  
	bool Init(CFrameWnd* pWnd);

	// Input:	bUnInitialize - true if the object is to be un-initialized.
	// Returns: true if successful, otherwise returns false.
	// Summary: This member function saves the current keyboard accelerator.
	bool Save(bool bUnInitialize=false);

	// Input:	bUpdateWindowAccel - true to, when loaded, apply the configuration to
	//			the current frame window.
	// Returns: true if successful, otherwise returns false.
	// Summary: This member function loads a new keyboard accelerator.
	bool Load(bool bUpdateWindowAccel = true);

	// Returns: A valid CMenu pointer if successful, otherwise returns NULL.
	// Summary: This member function is called to get a pointer to the frame's
	//			active menu.  
	CMenu* GetFrameMenu();

	// Input:	bEnable - true to enable autosave.		
	// Summary: Call this member function to enable or disable the autosave feature.
	//			Autosave will automatically save the accelerator information when 
	//			this object is destroyed.
	void EnableAutoSave(bool bEnable);

	// Input:	hMenu - NEEDS DESCRIPTION
	// Summary: This member function is called to initialize the default and user
	//			defined accelerator tables for the manager, and must be called after the
	//			Init(...) method has been called.
	void InitAccelerators(HMENU hMenu=NULL);
	
	// Input:	nCommand - The command ID to exclude.
	// Summary: This member function is used to exclude a specific command ID. To exclude
	//			a specific command ID, call this method before calling Init(...).
	void ExcludeCommandID(UINT nCommand);

	// Input:	nSwapOutID - The command ID of the menu item, defined in the menu resource,
	//			that should be replaced.		
	//			nCommandID - The command ID to add to the menu.
	//			szName - The menu string to add.
	// Summary: Call this method to set one or more swapout IDs, in the case where
	//			there is a dynamic menu item. An example of where this can be used would
	//			be the MRU in a standard doc/view application. Add a single command
	//			to the menu resource that can be swapped out with the commands you
	//			set here. Call this method once for each command you want to replace.
	//			For example, if you had a command ID called IDC_MRUPLACEHOLDER, and
	//			you wanted to add four new items in place of this which are IDC_MRUFILE1,
	//			IDC_MRUFILE2, IDC_MRUFILE3, IDC_MRUFILE4 you should do the following: -
    //          <pre>
	//			kbsm.AddSwapoutCommand(IDC_MRUPLACEHOLDER, IDC_MRUFILE1, "File 1");
    //          kbsm.AddSwapoutCommand(IDC_MRUPLACEHOLDER, IDC_MRUFILE2, "File 2");
    //          kbsm.AddSwapoutCommand(IDC_MRUPLACEHOLDER, IDC_MRUFILE3, "File 3");
    //          kbsm.AddSwapoutCommand(IDC_MRUPLACEHOLDER, IDC_MRUFILE4, "File 4");</pre>
	void AddSwapoutCommand(UINT nSwapOutID, UINT nCommandID, LPCTSTR szName);

	// Input:	szCategory - The category that this command should be added under.
	//			nCommandID - The command ID for this extra command.
	//			szName - The menu string for this command.
	// Summary: This member function adds command IDs that are not found in the menu,
	//			but you want to have appear in the keyboard shortcut configuration and
	//			assignment property pages.
	void AddExtraCommand(LPCTSTR szCategory, UINT nCommandID, LPCTSTR szName);
	
	// Returns: true if successful, otherwise returns false.
	// Summary: This member function applies the current accelerators to the window.
	bool UpdateWindowAccelerator();
	
	// Returns: true if successful, otherwise returns false.
	// Summary: This member function is called to restore the default accelerators for
	//			the frame window. 
	bool LoadDefaultAccelerator();

	// Input:	nCommand - The command ID.
	//			szKeys - Retrieves the displayable string(s) for the assigned shortcuts
	//			(i.e. "Ctrl+C").
	// Summary: This member function will, given a command ID, return a string describing
	//			the currently assigned keys, if any.
	void GetKeyAssignmentStrings(UINT nCommand,	CString& szKeys);

	// Returns: true if successful, otherwise returns false.
	// Summary: Override this member function to handle custom loading of the accelerator
	//			data. 
	virtual bool OnLoad();

	// Input:	pTable - Pointer to the accelerator table (array) to save.
	//			nCount - The number of elements in the above array.
	// Returns: true if successful, otherwise returns false.
	// Summary: Override this member function to handle custom saving of the accelerator
	//			data. 
	virtual bool OnSave(LPACCEL pTable, int nCount);

	// Input:	pMsg - A pointer to a valid MSG structure.
	// Returns: true if the message was handled, otherwise returns false.
    // Summary: This member function is called by the framework to allow the accelerator
	//			manager to process any messages before they are forwarded.  
	bool TranslateAccelerator(MSG* pMsg);

private:

	// Helper methods used by the configuration pages

	POSITION GetCommandStringStartPosition();
	void GetNextCommandStringItem(POSITION& rPos, UINT& nCommandID, CString& szString);
	bool LookupCommandStringItem(UINT nCommandID, CString& szString);
	void SetCommandStringItem(UINT nCommandID, CString szName);
	void GetCategoryList(CComboBox& cb);
	bool IsCommandInCategory(UINT nCommandID, LPCTSTR szCategory);
	bool LookupCommandCategory(UINT nCommandID, CString& szCategory);
	bool GetKeyAssignmentInfo(int& nPos, UINT nCommand, CString& szName, BYTE& cVirt, WORD& wKey);
	bool AddKeyAssignment(UINT nCommand, BYTE cVirt, WORD wKey);
	bool GetKeyAssignment(BYTE cVirt, WORD wKey, UINT& nCommand);
	bool DeleteKeyAssignment(BYTE cVirt, WORD wKey);
	void MakeDisplayName(CXTString& str);
	void MakeKeyString(ACCEL Accel, CString& szName);
	void SaveDefaultAcceleratorTable(CFrameWnd* pFrameWnd);
	bool AddMenuItemsToNameList(HMENU hMenu, CString szParentName, LPCTSTR szCategory);
	static LPCTSTR GetVirtualKeyString(WORD wKey);
    bool MenuHasCommands(HMENU hMenu);
    void BuildCategoryList(HMENU hMenu, CString strParentItem);
    CXTMapStringList& GetCommandStringList();

	// These classes need access to edit the keyboard accelerators.

	friend class CXTCustAccelerators;
	friend class CXTAccelKeyEdit;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE void CXTAccelManager::ExcludeCommandID(UINT nCommand) {
	m_MapCommandExcludeList.SetAt(nCommand, nCommand);
}
AFX_INLINE POSITION CXTAccelManager::GetCommandStringStartPosition() {
	return m_mapAccelString.GetHeadPosition();
}
AFX_INLINE bool CXTAccelManager::IsInitialized() {
	return m_bInitialized;
}
AFX_INLINE void CXTAccelManager::EnableAutoSave(bool bEnable) {
	m_bAutoSave = bEnable;
}
AFX_INLINE CXTMapStringList& CXTAccelManager::GetCommandStringList() {
    return m_mapAccelString;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(__XTACCELMANAGER_H_)
