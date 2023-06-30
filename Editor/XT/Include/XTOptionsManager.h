// XTOptionsManager.h: interface for the CXTOptionsManager class.
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

#if !defined(__XTOPTIONSMANAGER_H__)
#define __XTOPTIONSMANAGER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// forwards

class CXTCoolMenu;
class CXTAccelManager;
class CXTToolsManager;

//////////////////////////////////////////////////////////////////////
// Summary: XT_OPTIONS is a stand alone structure class.  It is used by CXTOptionsManager
//			to manage option data settings.
struct XT_OPTIONS
{
	int  nAnimationType;	  // Animation type
	bool bToolBarVisualize;   // true to render the toolbar while dragging or resizing.
	bool bMenuShadows;		  // true to use shadows under the menus.
	bool bMenuRecentCommands; // true to use intelligent menus to hide selected menu commands.
	bool bToolBarScreenTips;  // true to show tooltips on toolbar commands.
	bool bShowFullAfterDelay; // true to display hidden menu commands after a short delay.
	bool bToolBarAccelTips;   // true to add accelerator keys to toolbar tips.
};

//////////////////////////////////////////////////////////////////////
// Summary: CXTOptionsManager is a stand alone class. It is used to create a CXTOptionsManager
//			object that is used to manage toolbar and menu options for the application.
class CXTOptionsManager  
{
private:

	// Summary: Constructs a CXTOptionsManager object.
	CXTOptionsManager();

	
public:

	// Summary: Destroys a CXTOptionsManager object, handles cleanup and de-allocation.
	virtual ~CXTOptionsManager();

protected:

    bool         m_bInitialized; // true if the options data has been initialized for the frame object.
    bool         m_bAutoSave;    // true if the options data is automatically saved when this class is destroyed.
    bool         m_bChanged;     // true if the contents of 'm_options' has been changed.
    bool         m_bRemoveAll;   // true if all options data is to be removed from the system registry.
    CFrameWnd*   m_pFrameWnd;    // Pointer to the frame window for the application.
    XT_OPTIONS   m_options;      // Option value structure.
    CXTCoolMenu* m_pCoolMenu;    // Pointer to the cool menu manager for the frame specified by 'm_pFrameWnd'.

public:

	// Returns: A reference to the one and only CXTOptionsManager object.
	// Example: <pre>CXTOptionsManager::Get().Init( this, GetCoolMenu() );</pre>
	// Summary:	This static member function will retrieve a reference to the one
	//			and only CXTOptionsManager object.  You can use this function to access
	//			data members for the CXTOptionsManager class.
    static CXTOptionsManager& Get();

	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to save the options data to the system registry.
	bool Save();

	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to load the options data from the system
	//			registry. 
	bool Load();

	// Input:	pFrameWnd - A pointer to a valid CFrameWnd object.
	//			pCoolMenu - A pointer to a valid CXTCoolMenu object.
	// Returns: true if successful, otherwise returns false.
	// Summary:	This member function is called to initialize the Options manager. 
	//			This function is called by the framework, usually from an overrode
	//			LoadFrame handler. 
	bool Init(CFrameWnd* pFrameWnd,CXTCoolMenu* pCoolMenu);

	// Returns: true if it has been initialized, otherwise returns false.
	// Summary:	This member function is called to determine if the Options manager
	//			has been initialized. 
	bool IsInitialized();

	// Input:	options - A reference to a valid XT_OPTIONS structure.
	// Summary:	This member function is called to update the option data for the Options
	//			manager.
	void SetOptions(XT_OPTIONS& options);

	// Returns: A reference to an XT_OPTIONS structure.
	// Summary:	This member function is called to return a reference to the options
	//			data for the Options manager.  
	XT_OPTIONS& GetOptions();

	// Summary: This member function is called to reset the the cool menus usage data
	//			for recently used commands. Calling this function will remove the list
	//			of recently used commands for intelligent menus. 
	void Reset();

protected:

	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function is called by the Options manager during load
	//			operations.  
	virtual bool OnLoad();

	// Returns: true if successful, otherwise returns false.
    // Summary:	This member function is called by the Options manager during save
	//			operations.  
	virtual bool OnSave();
};

//////////////////////////////////////////////////////////////////////

AFX_INLINE bool CXTOptionsManager::IsInitialized() {
	return m_bInitialized;
}

//////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(__XTOPTIONSMANAGER_H__)