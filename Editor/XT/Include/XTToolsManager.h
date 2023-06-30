// XTToolsManager.h: interface for the CXTToolsManager class.
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

#if !defined(__XTTOOLSMANAGER_H__)
#define __XTTOOLSMANAGER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forwards

class CXTMenuBar;
class CXTCoolMenu;
class CXTEditListBox;

//////////////////////////////////////////////////////////////////////
// Summary: XT_TOOL is a stand alone structure class.  It is used by CXTToolsManager
//			to manage tool data settings.
struct XT_TOOL
{
	UINT uCmdID;			// Command ID to be associated with this tool.
	TCHAR szTitle[128];		// A NULL terminated string that represents the command title.
	TCHAR szArg[128];		// A NULL terminated string that represents the argument passed to the executable.
	TCHAR szCmd[_MAX_PATH]; // A NULL terminated string that represents the executable associated with this command.
	TCHAR szDir[_MAX_PATH]; // A NULL terminated string that represents the default directory. 
};

typedef CMap<UINT,UINT,XT_TOOL*,XT_TOOL*> CXTToolsMap;
typedef	CMap<HMENU,HMENU,HMENU,HMENU>     CXTToolsMenuMap;

//////////////////////////////////////////////////////////////////////
// Summary: CXTColorRef is a stand alone class.  It is used to manage the tools
//			configuration page in the Customize dialog.
class _XT_EXT_CLASS CXTToolsManager  
{
private:

	// Summary: Constructs a CXTToolsManager object.
	CXTToolsManager();
	
public:

    // Summary: Destroys a CXTToolsManager object, handles cleanup and de-allocation.
    virtual ~CXTToolsManager();

protected:

	//////////////////////////////////////////////////////////////////////
	// Frame window binding information
	//////////////////////////////////////////////////////////////////////

	CFrameWnd* m_pFrameWnd; // Frame window the accelerators are connected to.

	//////////////////////////////////////////////////////////////////////
	// Our tools buffer
	//////////////////////////////////////////////////////////////////////

    int				m_iHTMLIcon;		// Index of the HTML icon in the shell image list.
    int				m_iArgPopup;		// Resource ID of the popup menu to be displayed for the arguments browse edit box.
    int				m_iDirPopup;		// Resource ID of the popup menu to be displayed for the directory browse edit box.
    int				m_iNormalIndex;		// Index where the 'Tools' menu should be inserted into the standard menu.
    int				m_iWindowIndex;		// Index where the 'Tools' menu should be inserted into the MDI window menu.
	int				m_nToolsCount;		// The count of tools pointed to by 'm_pToolsArray'.
	bool			m_bChanged;			// true if the contents of 'm_pToolsArray' has been changed.
	bool			m_bAutoSave;		// true if the tool's data is automatically saved when this class is destroyed.
	bool			m_bInitialized;		// true if the tool's data has been initialized.
    bool			m_bRemoveAll;		// true if all tool's have been removed.
	HMENU			m_hToolsMenu;		// Handle to the tools menu inserted into the applications menu.
	CString			m_strToolsTitle;	// User defined title of the 'Tools' pulldown menu.
	XT_TOOL*		m_pToolsArray;		// Array of tool commands.
	CXTIconMap		m_mapIconCmd;		// Hash table to map tool commands with corresponding menu icons.
	HIMAGELIST		m_hImageList;		// Handle to the system image list.
	CXTMenuBar*		m_pMenuBar;			// Points to the menubar for the application.
	CXTToolsMap		m_mapTools;			// Hash table of XT_TOOL structures mapped to user defined commands.
	CXTCoolMenu*	m_pCoolMenu;		// Points to the application's cool menu hook.
	CXTToolsMenuMap m_mapToolsMenu;		// Hash table of application menus mapped to existing tools menus.

public:

	// Example: <pre>CXTOptionsManager::Get().Init( this, GetMenuBar(), GetCoolMenu() );</pre>
	// Returns: A reference to the one and only CXTToolsManager object.
	// Summary:	This static member function will retrieve a reference to the one
	//			and only CXTToolsManager object.  You can use this function to access
	//			data members for the CXTToolsManager class.
    static CXTToolsManager& Get();

	// Input:	iArgPopup - Resource ID of the popup menu to be displayed.
    // Summary:	This member function is called to set the popup menu used by the argument browse
	//			edit control in the toolbar customize dialogs tools page.
	void SetArgPopupMenu(int iArgPopup);

	// Returns: An integer value that represents the resource ID.
    // Summary:	This member function retrieves the resource ID of the argument menu
	//			associated with the argument browse edit control in the toolbar customize
	//			dialog tools page. 
	int GetArgPopupMenu() const;

	// Input:	iDirPopup - Resource ID of the popup menu to be displayed.
    // Summary:	This member function is called to set the popup menu used by the directory browse
	//			edit control in the toolbar customize dialogs tools page.
	void SetDirPopupMenu(int iDirPopup);

	// Returns: An integer value that represents the resource ID.
    // Summary:	This member function retrieves the resource ID of the directory
	//			menu associated with the argument browse edit control in the toolbar
	//			customize dialog tools page. 
	int GetDirPopupMenu() const;

	// Input:	bNormal - true for normal index, false to return the MDI window index.
	// Returns: An integer value that represents the zero-based index where the tools
	//			menu is inserted.
    // Summary:	This member function is called to retrieve the location where the
	//			tools menu is currently inserted into the framework's main menu.  
    int GetMenuIndex(bool bNormal);

	// Returns: A handle to the tools menu.
    // Summary:	This member function is called to retrieve a handle to the tools
	//			menu that is inserted into the framework's main menu. 
	HMENU GetToolsMenu() const;

	// Returns: A reference to the CXTToolMap object.
    // Summary:	This member function is called to retrieve a reference to the CXTToolMap
	//			object that represents the array of user commands defined for the tools
	//			menu. 
	CXTToolsMap& GetToolsMap();

	// Returns: true if the accelerator data has been initialized, otherwise it returns false.
	// Summary:	This member function will check to see if the accelerator data has
	//			been initialized. 
	bool IsInitialized();

    // Summary: This member function is called to remove all of the custom user
	//			defined tools.
	void RemoveAllTools();

	// Input:	pTool - A pointer to a valid XT_TOOL structure.
	// Returns: true if successful, otherwise returns false. 
	// Summary:	This member function is called to remove the user tool specified by
	//			'pTool'.  
	bool RemoveTool(XT_TOOL* pTool);

	// Input:	pTool - A pointer to a valid XT_TOOL structure.
	// Summary:	This member function is called to add a user defined tool to the Tools
	//			manager command array.
	void AddTool(XT_TOOL* pTool);

	// Input:	bUnInitialize - true if the object is to be un-initialized.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function saves the current tool information.
	bool Save(bool bUnInitialize=false);

	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function loads saved tool information.
	bool Load();

	// Input:	pFrameWnd - A pointer to a valid CFrameWnd object.
	//			pMenuBar - A pointer to a valid CXTMenuBar object.
	//			m_pCoolMenu  - A pointer to a valid CXTCoolMenu object.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to initialize the Tools manager.  This
	//			function is called by the framework, usually from an overridden LoadFrame
	//			handler. 
	bool Init(CFrameWnd* pFrameWnd,CXTMenuBar* pMenuBar,CXTCoolMenu* m_pCoolMenu);

	// Input:	editListBox - A reference to a valid CXTEditListBox object.
    // Summary:	This member function is called to initialize the tools list that
	//			is displayed in the customize toolbar tools page.
	void GetToolsList(CXTEditListBox& editListBox);

	// Input:	bEnable - true to enable autosave.
	// Summary:	Call this member function to enable or disable the autosave feature.
	//			Autosave will automatically save the tool's information when this object
	//			is destroyed.
	void EnableAutoSave(bool bEnable);

	// Input:	bChanged - true if the tools data has changed.
    // Summary:	This member function is called to set the changed flag for the tools manager.
	void HasChanged(bool bChanged);

	// Input:	bDeleteAll - TRUE to remove all menus including items not belonging to us.
    // Summary:	This member function is called to rebuild the tools menu that is
	//			displayed in the framework's main menu.
    void BuildToolsMenu(BOOL bDeleteAll);

	// Returns: A zero-based index value.
    // Summary:	This member function is called to retrieve the index of the HTML
	//			icon stored in the system's image list.  
    int GetHTMLIcon();

	// Input:	iNormalIndex - Index where the 'Tools' menu should be inserted into the standard menu.
	//			iWindowIndex - Index where the 'Tools' menu should be inserted into the MDI window menu.
    // Summary:	This member function is called to set the index location of where
	//			the Tools manager will insert the tools menu in the framework's main menu.
    void SetMenuIndex(int iNormalIndex,int iWindowIndex);

	// Returns: A UINT value that represents the next available command id.
    // Summary:	This member function is called to retrieve the next available command
	//			ID to be used when creating a user defined tool.  
	UINT GetNextCmdID();

	// Input:	strToolsText - NULL terminated string that represents the new tools title.
	// Summary:	This member function will set the title for the 'Tools' pulldown menu.  You can
	//			use this function to set the name to something other than the default 'Tools'.  If
	//			you set the title to an existing pulldown menu then the existing menu will be
	//			appended to include the user defined items normally seen in the 'Tools' menu.
	void SetMenuTitle(CString strToolsText);

	// Returns: A CString object that represents the current text for the 'Tools' pulldown
	//			menu item.
	// Summary:	This member function returns the current title for the 'Tools' pulldown menu.  
	CString GetMenuTitle() const;

	// Input:	hMenu - Handle to the menu to search.
	//			iMenuItem - Index of the sub menu found, or -1 if no menu was found.
	// Returns: The handle of the existing tools menu if found, otherwise returns NULL.
	// Summary:	This member function searches the menu specified by 'hMenu' for a submenu that
	//			is titled 'Tools'.  
    HMENU ToolsMenuExists(HMENU hMenu,int& iMenuItem);

	// Input:	hNewMenu - Handle of the menu to modify.
	//			iIndex - Zero based index where to insert the tools submenu.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function will insert the tools menu into the menu specified by 'hNewMenu'
	//			at the index specified by iIndex.  
	bool InsertToolsMenu(HMENU hNewMenu,int iIndex);

protected:

	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function is called by the Tools manager during load
	//			operations.  
	virtual bool OnLoad();

	// Input:	pToolsArray - A pointer to a valid XT_TOOL structure.
	//			nCount - Size of the 'pToolsArray'.
	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function is called by the Tools manager during save
	//			operations.  
	virtual bool OnSave(XT_TOOL* pToolsArray,int nCount);

	friend class CXTToolBar;
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE bool CXTToolsManager::IsInitialized() {
	return m_bInitialized;
}
AFX_INLINE void CXTToolsManager::EnableAutoSave(bool bEnable) {
	m_bAutoSave = bEnable;
}
AFX_INLINE void CXTToolsManager::HasChanged(bool bChanged) {
	m_bChanged = bChanged;
}
AFX_INLINE CXTToolsMap& CXTToolsManager::GetToolsMap() {
	return m_mapTools;
}
AFX_INLINE HMENU CXTToolsManager::GetToolsMenu() const {
	return m_hToolsMenu;
}
AFX_INLINE void CXTToolsManager::SetArgPopupMenu(int iArgPopup) {
	m_iArgPopup = iArgPopup;
}
AFX_INLINE int CXTToolsManager::GetArgPopupMenu() const {
	return m_iArgPopup;
}
AFX_INLINE void CXTToolsManager::SetDirPopupMenu(int iDirPopup) {
	m_iDirPopup = iDirPopup;
}
AFX_INLINE int CXTToolsManager::GetDirPopupMenu() const {
	return m_iDirPopup;
}
AFX_INLINE int CXTToolsManager::GetMenuIndex(bool bNormal) {
    return bNormal ? m_iNormalIndex : m_iWindowIndex;
}
AFX_INLINE void CXTToolsManager::SetMenuIndex(int iNormalIndex, int iWindowIndex) {
    m_iNormalIndex = iNormalIndex; m_iWindowIndex = iWindowIndex;
}
AFX_INLINE void CXTToolsManager::SetMenuTitle(CString strToolsText) {
	m_strToolsTitle = strToolsText;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // #if !defined(__XTTOOLSMANAGER_H__)