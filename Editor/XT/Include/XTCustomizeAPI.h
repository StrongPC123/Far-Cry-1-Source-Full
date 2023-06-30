// XTCustomizeAPI.h APIs used in toolbar customization
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

#if !defined(__XTCUSTOMIZEAPI_H__)
#define __XTCUSTOMIZEAPI_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// f.forwards

struct XT_DROPDOWNBUTTON;
class CXTIconMap;

// Summary: Descriptor of a control bar that supports customizations
class CXTCustomControlBarInfo
{
public:

	
    bool         m_bCanShowHide; // true if this control bar can toggle its visibility on/off
    bool         m_bDynamic;     // true if this bar was dynamically created
    CControlBar* m_pBar;         // A control bar that supports customizations

	// Summary: Callback for renaming the control bar or NULL if operation is not supported.
	typedef CXTDelegate1<const CString&> RENAME;
	RENAME Rename;

	// Summary: Callback for deleting the control bar or NULL if operation is not supported.
	typedef CXTDelegate0 REMOVE;
	REMOVE Remove;

	// Summary: Callback for resetting the control bar or NULL if operation is not supported.
	typedef CXTDelegate0 RESET;
	RESET Reset;

	CXTCustomControlBarInfo()
	: m_pBar(0)
	, m_bCanShowHide(false)
	, m_bDynamic(false)
	{}

};

// Summary: A delegate to store CXTCustomControlBarInfo's
typedef CXTDelegate1<CXTCustomControlBarInfo*> STORECUSTOMINFO;

// Summary: A request to create a new customizable control bar
class CXTNewCustomBarRequest
{
public:

    UINT         m_nBarID;   // ID of the control bar to create
    CString      m_strTitle; // The toolbar title
    CFrameWnd*   m_pFrame;   // Frame window on which to create the toolbar
    CControlBar* m_pNewBar;  // Newly created custom bar

	CXTNewCustomBarRequest()
	: m_pFrame(0)
	, m_nBarID(0)
	, m_pNewBar(0)
	{ }
};

// Summary: An item included in the custom group
class CXTCustomGroupItemInfo
{
public:
	
	
	CString m_strTitle; // Item title, will be displayed in commands listbox in customize dialog

	// Summary: Known types of items being transferred
	enum Type 
	{
		Button, // Toolbar button
		Control // A control on the toolbar
	};
	const Type m_type;

    bool                m_bOwnsDropDown; // true if owning a private copy of XT_DROPDOWNBUTTON
    UINT                m_nCmdID;        // Command or control identifier
    UINT                m_nWidth;        // Control width, valid for controls
    CString             m_strLabel;      // Button label
    CXTIconHandle       m_icon;          // Icon assigned to the command
    CXTIconHandle       m_disabledIcon;  // Disabled icon assigned to the command
    CXTIconHandle       m_hotIcon;       // Hot icon assigned to the command
    XT_DROPDOWNBUTTON*  m_pDropDown;     // Descriptor of the dropdown button


// Construction

	CXTCustomGroupItemInfo(Type type);
	virtual ~CXTCustomGroupItemInfo();

// Operations
	
	// Summary: Implement if you derive from this class
	virtual CXTCustomGroupItemInfo* Clone() const;

private: 

	// no implementation

	CXTCustomGroupItemInfo(const CXTCustomGroupItemInfo& other);
	CXTCustomGroupItemInfo& operator=(const CXTCustomGroupItemInfo& other);	
};

// Summary: CTypedPtrArray definition for the custom group item info array.
typedef CTypedPtrArray<CPtrArray, CXTCustomGroupItemInfo*> CXTCustomGroupItemInfoArray;

// Summary: Customizable group descriptor, the group is identified by a group owner (can later
//			be queried for group content) and group ID to uniquely identify the group with its owner	
class CXTCustomGroupInfo
{
public:
	
    int                          m_nSortPriority; // Sort priority customize dialog sorts groups by this number
    CString                      m_strTitle;      // Group title
    CXTCustomGroupItemInfoArray  m_items;         // Items included in this group

// Construction
	
	// Summary: Constructs a CustomGroupInfo object.
	CXTCustomGroupInfo();
	// Summary: Destroys a CustomGroupInfo object, handles cleanup and de-allocation.
	virtual ~CXTCustomGroupInfo();

	void Copy(const CXTCustomGroupInfo* other);
	void DeleteItems();
	void DeleteItem(int nIndex);

private:
	// no implementation
	
	CXTCustomGroupInfo(const CXTCustomGroupInfo& other);
	CXTCustomGroupInfo& operator=(const CXTCustomGroupInfo& other);
};

// Summary: Array of custom groups
class CXTCustomGroups : public CTypedPtrArray<CPtrArray, CXTCustomGroupInfo*> 
{
public:

	// Summary: Constructs a CXTCustomGroups object.
	CXTCustomGroups();
	// Summary: Destroys a CXTCustomGroups object, handles cleanup and de-allocation.
	virtual ~CXTCustomGroups();
	void RemoveAll();
};

// Summary: Definition of the drop target
interface IXTCustDropTarget
{

	// Input:	ptCursor - location in screen coordinates
	// Returns: true to indicate the drop is possible
	// Summary:	Checks to see if cursor is over the target
	virtual bool IsDropPoint(CPoint ptCursor) = 0;

	// Input:	pInfo - item being dragged
	//			ptCursor - location in screen coordinates
	//			bCopyAllowed - OUT: tells if copy operation is acceptable
	// Returns: true if the data has been accepted for the drop
	// Summary:	Drag cursor is over the target notification	
	virtual bool DragEnter(CXTCustomGroupItemInfo* pInfo,CPoint ptCursor,bool& bCopyAllowed) = 0;


	// Input:	ptCursor - location in screen coordinates
	// Summary:	Mouse is still over the target but position has changed
	virtual void DragMove(CPoint ptCursor) = 0;

	// Summary: Mouse has left the building.
	virtual void DragLeave() = 0;

	// Input:	pInfo - item being dropped
	//			ptCursor - location in screen coordinates
	// Summary:	Mouse dropped, must accept the data
	virtual void Drop(CXTCustomGroupItemInfo* pInfo,CPoint ptCursor) = 0;
};


// Summary: A sink to report a drop target
interface IXTCustDropSink
{
	virtual void Add(IXTCustDropTarget* target) = 0;
};

// Summary: A connection point for objects involved in customize toolbar operations
class CXTCustomizeConnection : public CObject
{

    bool     m_bEnableCmdUI;
    bool     m_bCustMode;
    CPoint   m_startPoint;   // Last known point of a newly created toolbar.
    CObArray m_garbage;      // Array of objects scheduled for garbage collection.
	
	
// Construction

private:
	CXTCustomizeConnection();

public:
	static _XT_EXT_CLASS CXTCustomizeConnection* GetInstance();

// Operations

public:

	// Summary: A delegate to check if the command shall be displayed in the customization dialog	.
	typedef CXTDelegate1Ret<bool, UINT> ISCUSTCMD;
	ISCUSTCMD IsCustCmd;

	// Summary: A delegate to create new customizable control bar.
	typedef CXTDelegate1<CXTNewCustomBarRequest*> CREATEBAR;
	CREATEBAR CreateBar;

	// Summary: A delegate to properly place newly created customizable control bar.
	typedef CXTDelegate2<CFrameWnd*, CControlBar*> PLACEBAR;
	PLACEBAR PlaceBar;

	// Summary: A delegate for to get info about custom command groups.
	typedef CXTMultiCastDelegate1<CXTCustomGroups&> GETCUSTOMGROUPS;
	GETCUSTOMGROUPS GetCustomGroups;

	// Summary: A delegate to collect all command-to-icon associations.
	typedef CXTMultiCastDelegate1<CXTIconMap*> GETICONS;
	GETICONS GetIcons;

	// Summary: A delegate for to get info about customizable control bars.
	typedef CXTMultiCastDelegate1<const STORECUSTOMINFO&> GETCUSTOMBARINFO;
	GETCUSTOMBARINFO GetCustomBarInfo;

	// Summary: A delegate for resetting tracking state.
	typedef CXTMultiCastDelegate0 RESETTRACKING;
	RESETTRACKING ResetTracking;

	// Summary: A delegate to receive notifications on customize mode on/off events.
	typedef CXTMultiCastDelegate1<bool> SETCUSTMODE;
	SETCUSTMODE SetCustMode;

	// Summary: A delegate to obtain current drop targets.
	typedef CXTMultiCastDelegate1<IXTCustDropSink*> GETTARGETS;
	GETTARGETS GetTargets;

	// Summary: Notifies that a control bar has been hidden/shown.
	typedef CXTMultiCastDelegate2<CControlBar*, bool> ONSHOWHIDE;
	ONSHOWHIDE OnShowHide;

	// Summary: Notifies a new customizable bar has been created.
	typedef CXTMultiCastDelegate1<CXTCustomControlBarInfo*> ONNEWBAR;
	ONNEWBAR OnNewBar;

	// Summary: Notifies customizable bar has been renamed.
	typedef CXTMultiCastDelegate2<CControlBar*, LPCTSTR> ONBARRENAMED;
	ONBARRENAMED OnBarRenamed;

	// Summary: Notifies customizable bar has been destroyed.
	typedef CXTMultiCastDelegate1<CControlBar*> ONBARDESTROYED;
	ONBARDESTROYED OnBarDestroyed;

	// Summary: Returns a menu handle given its ID (a DWORD).
	typedef CXTDelegate1Ret<HMENU, DWORD> GETMENUPOPUP;
	GETMENUPOPUP GetMenuPopup;

	// Summary: Application command removed event, gets ID of the command removed.
	typedef CXTMultiCastDelegate1<UINT> ONCMDCHANGE;
	ONCMDCHANGE OnCmdRemoved;

	// Returns: false if no data is in the profile.
	// Summary:	A delegate to define operation for persisting toolbars to/from the registry.
	//			Takes a parent frame pointer and a profile name to load from.
	//			Default refers to LoadDynamicBarsImp().
	typedef CXTDelegate2Ret<bool, CFrameWnd*, LPCTSTR> PERSISTER;
	
	
	PERSISTER LoadDynamicBars; // A delegate to load dynamic custom toolbars from the registry (defaults to LoadDynamicBarsImp())
	PERSISTER SaveDynamicBars; // A delegate to store dynamically created toolbars to the registry (defaults to SaveBarsImp())

	// Summary: A delegate to find out if a command is defined in the app.
	//			Takes a command ID, returns true if the command is defined in the app.
	//			Default refers to IsAppCmdImp().
	typedef CXTDelegate1Ret<bool, UINT> ISAPPCMD;
	ISAPPCMD IsAppCmd;

	// Returns:	True if successful, otherwise returns false.
	// Summary:	Tells if customization is currently active.
	bool IsCustMode() const;

	// Returns:	True if successful, otherwise returns false.
	// Summary:	Tells if UI shall be enabled during customization (valid if IsCustMode() 
	//			returns true).
	bool IsEnableCmdUI() const;

	// Input:	bEnableCmdUI - True to enable command.
	// Summary:	Sets the flag telling if UI shall be enabled during toolbar customization
	//			default (as set when toolbar customization is turned on) is true.
	void SetEnableCmdUI(bool bEnableCmdUI);

	// Input:	pTarget - Points to a CObject object.
	// Summary:	Removes a target. 
	void Remove(CObject* pTarget);

	// Input:	pWnd - A window to capture the mouse
	//			canMove - True if the data can be moved to other locations
	//			pInfo - Item to move/copy
	// Returns: true to indicate a move operation has been requested.
	// Summary:	Processes drag-n-drop during toolbar customization.
	bool DragNDrop(CWnd* pWnd,bool canMove,CXTCustomGroupItemInfo* pInfo);

	// Input:	pRequest - Points to a CXTNewCustomBarRequest object.
	// Example: To customize, replace CreateBar delegate with you own version such as
	//			shown below:
    //          <pre>
    //          BOOL CMyApp::InitInstance()
    //          {
    //                  CXTCustomizeConnection::GetInstance()->CreateBar =
    //                      CXTCustomizeConnection::CREATEBAR(this,
    //                      CXTCustomizeConnection::CREATEBAR::CB(OnCreateCustomBar));
    //                  ..... other stuff ...
    //          }
    //          void CMyApp::OnCreateCustomBar(CXTNewCustomBarRequest* pRequest)
    //          {
    //                  // create default toolbar
    //                  CXTCustomizeConnection::GetInstance()->CreateBarImp(pRequest);
    //                  CXTCustomToolBar* pBar = (CXTCustomToolBar*)pRequest->m_pNewBar;
    //               .... customize pBar ....
    //          }</pre>
	// Summary:	Default implementation of the CreateBar delegate.
	//			It creates CXTCustomToolBar objects.
	void CreateBarImp(CXTNewCustomBarRequest* pRequest);

	// Input:	obj - Points to a CObject object.
	// Summary:	VC7 has ushered a bug that they call DestroyWindow() twice on the same dangling pointer
	//			hence effectively precluding you from using 'delete this'
	//			in PostNcDestroy(). All control bars created in your implementation:
	//			[ol]
	//			[li]shall not self-delete in PostNcDestroy()[/li]
	//			[li]shall use garbage collection facility below where obj is the object
	//			to be deleted later[/li]
	//			[/ol]
	void ScheduleGC(CObject* obj);

	// Summary: Companion to ScheduleGC() that actually deletes cached objects, called
	//			when it is safe to do so.
	void RunGC();

	// Input:	nCmdID - Comamnd identifier.
	// Returns:	True if successful, otherwise returns false.
	// Summary:	Default implementation of the IsCustCmd delegate.
	//			This function filters out IDs for MRU file list (ID_FILE_MRU1 thru 16)
	// Example:	To customize, replace IsCustomizableCommand delegate with you own 
	//			version such as shown below.
	//			<pre>
    //          BOOL CMyApp::InitInstance()
    //          {
    //                  CXTCustomizeConnection::GetInstance()->IsCustCmd =
    //                      CXTCustomizeConnection::ISCUSTCMD(this,
    //                      CXTCustomizeConnection::ISCUSTCMD::CB(IsCustCmd));
    //                  ..... other stuff ...
    //          }
    //          bool CMyApp::IsCustCmd(UINT nCmdID)
    //          {
    //                  return CXTCustomizeConnection::GetInstance()->IsCustCmdImp(nCmdID) &&
    //                          nCmdID != ID_A_COMMAND;
    //          }</pre> 
	bool IsCustCmdImp(UINT nCmdID);

	// Input:	pFrame - Points to a CFrameWnd object.
	//			lpszProfileName - Profile name to store in registry.
	// Returns:	True if successful, otherwise returns false.
	// Summary:	Default implementation of restoring dynamic custom toolbars
	//			from the data persistently stored in the registry.
	bool LoadDynamicBarsImp(CFrameWnd* pFrame, LPCTSTR lpszProfileName);

	// Input:	pFrame - Points to a CFrameWnd object.
	//			lpszProfileName - Profile name to store in registry.
	// Returns:	True if successful, otherwise returns false.
	// Summary:	Default implementation of saving dynamic custom toolbars
	//			to the data persistently stored in the registry.
	bool SaveDynamicBarsImp(CFrameWnd* pFrame, LPCTSTR lpszProfileName);

	// Input:	pFrame - Points to a CFrameWnd object.
	//			pBar - Points to a CControlBar object.
	// Summary:	Default implementation of newly created toolbar placement
	//			Floats the control bar.
	void PlaceBarImp(CFrameWnd* pFrame, CControlBar* pBar);

private:
	void OnSetCustMode(bool bMode);
};


//////////////////////////////////////////////////////////////////////////////

AFX_INLINE bool CXTCustomizeConnection::IsCustMode() const {
	return m_bCustMode;
}
AFX_INLINE bool CXTCustomizeConnection::IsEnableCmdUI() const {
	return m_bEnableCmdUI;
}
AFX_INLINE void CXTCustomizeConnection::SetEnableCmdUI(bool bEnableCmdUI) {
	m_bEnableCmdUI = bEnableCmdUI;
}
//////////////////////////////////////////////////////////////////////////////


#endif //__XTCUSTOMIZEAPI_H__
