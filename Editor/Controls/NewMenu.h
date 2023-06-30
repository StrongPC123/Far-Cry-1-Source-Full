// File    : NewMenu.cpp 
// Version : 1.10
// Date    : 30. July 2002
// Author  : Bruno Podetti
// Email   : Podetti@gmx.net
// Systems : VC6.0 and VC7.0 (Run under (Window 98/ME), Windows Nt 2000/XP)
//
// Special
// Compiled version with visual studio V7.0 doesn't run correctly on WinNt 4.0
// nor Windows 95. For those platforms, you have to compile it with 
// visual studio V6.0
// 
// ToDo's
// checking GDI-Leaks, are there some more?
// Shade-Drawing, when underground is changing
// XP-changing settings Border Style => to fix border painting without restarting
// Border repainting after menu closing!!!
//
// (30. July 2002) Bug Fixes or nice hints from 1.03
// Color of checked icons / selection marks more XP-look
// Updating images 16 color by changing systems colors
// Icon drawing with more highlighting and half transparency 
// Scroller for large menu correcting (but the overlapping border is not corrected)
// Resource-problem on windows 95/98/ME?, when using too much bitmaps in menus
//  changing the kind of storing bitmaps (images list)
// Calculating the size for a menuitem (specially submenuitem) corrected.
// Recentfilelist corrected (Seperator gots a wrong ID number)
//
// (25. June 2002) Bug Fixes or nice hints from 1.02
// Adding removing menu corrected for drawing the shade in menubar
// Draw the menuborder corrected under win 98 at the bottom side
// Highlighting under 98 for menu corrected (Assert), Neville Franks
//  Works also on Windows 95
// Changing styles on XP menucolor of the bar now corrected.
//
// (18. June 2002) Bug Fixes or nice hints from 1.01
// Popup menu which has more items than will fit, better 
// LoadMenu changed for better languagessuport, SnowCat
// Bordercolor & Drawing of XP-Style-Menu changed to more XP-Look
// Added some functions for setting/reading MenuItemData
// Menubar highlighting now supported under 98/ME (NT 4.0 and 95?)
// Font for menutitle can be changed
// Bugs for popupmenu by adding submenu corrected, no autodestroy
//
// (6. June 2002) Bug Fixes or nice hints from 1.00
// Loading Icons from a resource dll expanded (LoadToolBar) Jonathan de Halleux, Belgium.
// Minimized resource leak of User and Gdi objects
// Problem of disabled windows button on 98/Me solved
// Gradient-drawing without Msimg32.dll now supported especially for win 95
// Using solid colors for drawing menu items
// GetSubMenu changed to const
// Added helper function for setting popup-flag for popup menu (centered text)
// Redraw menu bar corrected after canceling popup menu in old style menu
//
// (23. Mai 2002) Bug Fixes and portions of code from previous version supplied by:
// Brent Corkum, Ben Ashley, Girish Bharadwaj, Jean-Edouard Lachand-Robert,
// Robert Edward Caldecott, Kenny Goers, Leonardo Zide, Stefan Kuhr, 
// Reiner Jung, Martin Vladic, Kim Yoo Chul, Oz Solomonovich, Tongzhe Cui, 
// Stephane Clog, Warren Stevens, Damir Valiulin
// 
// You are free to use/modify this code but leave this header intact.
// This class is public domain so you are free to use it any of your 
// applications (Freeware, Shareware, Commercial). 
// All I ask is that you let me know so that if you have a real winner I can
// brag to my buddies that some of my code is in your app. I also wouldn't 
// mind if you sent me a copy of your application since I like to play with
// new stuff.
//------------------------------------------------------------------------------

#ifndef __CNewMenu_H_
#define __CNewMenu_H_

#pragma warning(push)
#pragma warning(disable : 4211)

#include <afxtempl.h>
#include <afxpriv.h>

// Flagdefinitions 
#define MFT_TITLE       0x0001
#define MFT_TOP_TITLE   0x0000
#define MFT_SIDE_TITLE  0x0002
#define MFT_GRADIENT    0x0004
#define MFT_SUNKEN      0x0008
#define MFT_LINE        0x0010
#define MFT_ROUND       0x0020
#define MFT_CENTER      0x0040

// Typedefinition for compatibility to MFC 7.0
#ifndef DWORD_PTR
typedef DWORD DWORD_PTR, *PDWORD_PTR;
#endif

#ifndef ULONG_PTR
typedef unsigned long ULONG_PTR, *PULONG_PTR;
#endif

#ifndef LONG_PTR
typedef long LONG_PTR, *PLONG_PTR;
#endif

// Additional flagdefinition for highlighting
#ifndef ODS_HOTLIGHT
#define ODS_HOTLIGHT        0x0040
#endif

#ifndef ODS_INACTIVE
#define ODS_INACTIVE        0x0080
#endif


/////////////////////////////////////////////////////////////////////////////
// Constans for detecting OS-Type
enum Win32Type
{
  Win32s,
  WinNT3,
  Win95,
  Win98,
  WinME,
  WinNT4,
  Win2000,
  WinXP
};

extern const Win32Type g_Shell;
extern BOOL bRemoteSession;

/////////////////////////////////////////////////////////////////////////////
// Global helperfunctions
UINT GetSafeTimerID(HWND hWnd, const UINT uiElapse);
BOOL DrawMenubarItem(CWnd* pWnd,CMenu* pMenu, UINT nItemIndex,UINT nState);

WORD NumBitmapColors(LPBITMAPINFOHEADER lpBitmap);
HBITMAP LoadColorBitmap(LPCTSTR lpszResourceName, HMODULE hInst, int* pNumcol=NULL);

/////////////////////////////////////////////////////////////////////////////
// Forwarddeclaration for drawing purpose
class CMenuTheme;

/////////////////////////////////////////////////////////////////////////////
// CNewMenuIcons menu icons for drawing

class CNewMenuIcons : public CObject
{
  DECLARE_DYNCREATE(CNewMenuIcons)

public:
  CNewMenuIcons();
  virtual ~CNewMenuIcons();

public:
  BOOL GetIconSize(int* cx, int* cy);
  CSize GetIconSize();

  virtual int FindIndex(UINT nID);
  virtual void OnSysColorChange();

  virtual BOOL LoadToolBar(LPCTSTR lpszResourceName, HMODULE hInst);
  virtual BOOL LoadToolBar(WORD* pToolInfo, COLORREF crTransparent=CLR_NONE);

  virtual BOOL DoMatch(LPCTSTR lpszResourceName, HMODULE hInst);
  virtual BOOL DoMatch(WORD* pToolInfo, COLORREF crTransparent=CLR_NONE);

  virtual BOOL LoadBitmap(int nWidth, int nHeight, LPCTSTR lpszResourceName, HMODULE hInst=NULL);

//  virtual BOOL SetBlendImage();
  virtual int AddGloomIcon(HICON hIcon, int nIndex=-1);
  virtual int AddGrayIcon(HICON hIcon, int nIndex=-1);
  virtual BOOL MakeImages();

  void SetResourceName(LPCTSTR lpszResourceName);

  int AddRef();
  int Release();

#if defined(_DEBUG) || defined(_AFXDLL)
  // Diagnostic Support
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

public:
  LPCTSTR m_lpszResourceName;
  HMODULE m_hInst;
  int m_nColors;
  COLORREF m_crTransparent;

  CImageList m_IconsList;

  CArray<UINT,UINT&> m_IDs;
  DWORD m_dwRefCount;
};

/////////////////////////////////////////////////////////////////////////////
// CNewMenuBitmaps menu icons for drawing
class CNewMenuBitmaps : public CNewMenuIcons
{
  DECLARE_DYNCREATE(CNewMenuBitmaps)

public:
  CNewMenuBitmaps();
  virtual ~CNewMenuBitmaps();

public:
  int Add(UINT nID, COLORREF crTransparent=CLR_NONE);
  int Add(HICON hIcon, UINT nID=0);
  int Add(CBitmap* pBitmap, COLORREF crTransparent=CLR_NONE);

  virtual void OnSysColorChange();

  CArray<COLORREF,COLORREF&> m_TranspColors;
};

/////////////////////////////////////////////////////////////////////////////
// CNewMenuItemData menu item data for drawing

class CNewMenuItemData : public CObject
{
  DECLARE_DYNCREATE(CNewMenuItemData)

public:
  CNewMenuItemData();
  virtual ~CNewMenuItemData();

public:
  LPCTSTR GetString();
  void SetString(LPCTSTR szMenuText);

#if defined(_DEBUG) || defined(_AFXDLL)
  // Diagnostic Support
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

public:
  CString m_szMenuText;

  UINT m_nTitleFlags;
  UINT m_nFlags;
  UINT m_nID;
  UINT m_nSyncFlag;

  CNewMenuIcons* m_pMenuIcon;
  int m_nMenuIconOffset;

  void* m_pData;
};

/////////////////////////////////////////////////////////////////////////////
// CNewMenu the new menu class

class CNewMenu : public CMenu
{
  friend class CNewMenuHook;
  friend class CNewMenuIcons;

  DECLARE_DYNCREATE(CNewMenu)

public:
  // how the menu's are drawn, either original or XP style
  typedef enum 
  { 
    STYLE_ORIGINAL,
    STYLE_ORIGINAL_NOBORDER,

    STYLE_XP,
    STYLE_XP_NOBORDER,

    STYLE_SPECIAL,
    STYLE_SPECIAL_NOBORDER
  } EDrawStyle;

  // how seperators are handled when removing a menu (Tongzhe Cui)
  typedef enum {NONE, HEAD, TAIL, BOTH} ESeperator;

public:
  CNewMenu(HMENU hParent=0); 
  virtual ~CNewMenu();

  // Functions for loading and applying bitmaps to menus (see example application)
  virtual BOOL LoadMenu(HMENU hMenu);
  virtual BOOL LoadMenu(LPCTSTR lpszResourceName);
  virtual BOOL LoadMenu(int nResource);

  BOOL LoadToolBar(WORD* pIconInfo, COLORREF crTransparent=CLR_NONE);
  BOOL LoadToolBar(LPCTSTR lpszResourceName, HMODULE hInst = NULL);
  BOOL LoadToolBar(UINT nToolBar, HMODULE hInst = NULL);
  BOOL LoadToolBars(const UINT *arID,int n, HMODULE hInst = NULL);

  BOOL LoadFromToolBar(UINT nID,UINT nToolBar,int& xoffset);
  BOOL AddBitmapToImageList(CImageList *list,UINT nResourceID);

  static HBITMAP LoadSysColorBitmap(int nResourceId);
  // custom check mark bitmaps
  void LoadCheckmarkBitmap(int unselect, int select); 

  // functions for appending a menu option, use the AppendMenu call
  BOOL AppendMenu(UINT nFlags,UINT nIDNewItem=0,LPCTSTR lpszNewItem=NULL,int nIconNormal=-1);
  BOOL AppendMenu(UINT nFlags,UINT nIDNewItem,LPCTSTR lpszNewItem,CImageList *il,int xoffset);
  BOOL AppendMenu(UINT nFlags,UINT nIDNewItem,LPCTSTR lpszNewItem,CBitmap *bmp);

  BOOL AppendODMenu(LPCTSTR lpstrText, UINT nFlags = MF_OWNERDRAW, UINT nID = 0, int nIconNormal = -1);  
  BOOL AppendODMenu(LPCTSTR lpstrText, UINT nFlags, UINT nID, CBitmap* pbmp);
  BOOL AppendODMenu(LPCTSTR lpstrText, UINT nFlags, UINT nID, CImageList *il, int xoffset);
  BOOL AppendODMenu(LPCTSTR lpstrText, UINT nFlags, UINT nID, CNewMenuIcons* pIcons, int nIndex);

  // for appending a popup menu (see example application)
  CNewMenu* AppendODPopupMenu(LPCTSTR lpstrText);

  // functions for inserting a menu option, use the InsertMenu call (see above define)
  BOOL InsertMenu(UINT nPosition,UINT nFlags,UINT nIDNewItem=0,LPCTSTR lpszNewItem=NULL,int nIconNormal=-1);
  BOOL InsertMenu(UINT nPosition,UINT nFlags,UINT nIDNewItem,LPCTSTR lpszNewItem,CImageList *il,int xoffset);
  BOOL InsertMenu(UINT nPosition,UINT nFlags,UINT nIDNewItem,LPCTSTR lpszNewItem,CBitmap *bmp);

  BOOL InsertODMenu(UINT nPosition,LPCTSTR lpstrText, UINT nFlags = MF_OWNERDRAW,UINT nID = 0,int nIconNormal = -1);  
  BOOL InsertODMenu(UINT nPosition,LPCTSTR lpstrText, UINT nFlags, UINT nID, CBitmap* pBmp);
  BOOL InsertODMenu(UINT nPosition,LPCTSTR lpstrText, UINT nFlags, UINT nID, CImageList *il,int xoffset);
  BOOL InsertODMenu(UINT nPosition,LPCTSTR lpstrText, UINT nFlags, UINT nID, CNewMenuIcons* pIcons, int nIndex);

  // functions for modifying a menu option, use the ModifyODMenu call (see above define)
  BOOL ModifyODMenu(LPCTSTR lpstrText, UINT nID=0, int nIconNormal=-1);
  BOOL ModifyODMenu(LPCTSTR lpstrText, UINT nID, CImageList *il, int xoffset);
  BOOL ModifyODMenu(LPCTSTR lpstrText, UINT nID, CNewMenuIcons* pIcons, int nIndex);

  BOOL ModifyODMenu(LPCTSTR lpstrText,UINT nID,CBitmap *bmp);
  BOOL ModifyODMenu(LPCTSTR lpstrText,LPCTSTR OptionText,int nIconNormal);
  // use this method for adding a solid/hatched colored square beside a menu option
  // courtesy of Warren Stevens
  BOOL ModifyODMenu(LPCTSTR lpstrText,UINT nID,COLORREF fill,COLORREF border,int hatchstyle=-1);

  // for deleting and removing menu options
  BOOL  DeleteMenu(UINT uiId,UINT nFlags);
  BOOL  RemoveMenu(UINT uiId,UINT nFlags);
  int RemoveMenu(LPCTSTR pText, ESeperator sPos=CNewMenu::NONE);

  // function for retrieving and setting a menu options text (use this function
  // because it is ownerdrawn)
  BOOL GetMenuText(UINT id,CString &string,UINT nFlags = MF_BYPOSITION);
  BOOL SetMenuText(UINT id,CString string, UINT nFlags = MF_BYPOSITION);

  // Getting a submenu from it's name or position
  CMenu* GetSubMenu (LPCTSTR lpszSubMenuName) const;
  CMenu* GetSubMenu (int nPos) const;
  int GetMenuPosition(LPCTSTR pText);

  // Destoying
  virtual BOOL DestroyMenu();

  // Drawing: 
  // Draw an item
  virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
  // Measure an item  
  virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
  // Draw title of the menu
  virtual void DrawTitle(LPDRAWITEMSTRUCT lpDIS,BOOL bIsMenuBar);
  // Erase the Background of the menu
  virtual BOOL EraseBkgnd(HWND hWnd,HDC hDC);

  static COLORREF GetMenuBarColor();

  static void SetMenuTitleFont(CFont* pFont);
  static void SetMenuTitleFont(LOGFONT* pLogFont);
  static LOGFONT GetMenuTitleFont();

  // Menutitle function
  BOOL SetMenuTitle(LPCTSTR pTitle,UINT nTitleFlags=MFT_TOP_TITLE);
  BOOL RemoveMenuTitle();

  // Function to set how menu is drawn, either original or XP style
  static int  GetMenuDrawMode();
  static void SetMenuDrawMode(UINT mode);

  // Function to set how disabled items are drawn 
  //(mode=FALSE means they are not drawn selected)
  static BOOL SetSelectDisableMode(BOOL mode);
  static BOOL GetSelectDisableMode();

  // Function to set how icons were drawn in normal mode 
  //(enable=TRUE means they are drawn blended)
  static BOOL SetXpBlendig(BOOL bEnable=TRUE);
  static BOOL GetXpBlendig();

  static void OnSysColorChange();

  // Static functions used for handling menu's in the mainframe
  static LRESULT FindKeyboardShortcut(UINT nChar,UINT nFlags,CMenu *pMenu);
  static BOOL OnMeasureItem(const MSG* pMsg);
  static void OnInitMenuPopup(HWND hWnd, CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

  // Helperfunction to find the menu to the item
  static CMenu* FindPopupMenuFromID(CMenu* pMenu, UINT nID);
  static CMenu* FindPopupMenuFromID(HMENU hMenu, UINT nID);

  static CMenu* FindPopupMenuFromIDData(CMenu* pMenu, UINT nID, ULONG_PTR pData);
  static CMenu* FindPopupMenuFromIDData(HMENU hMenu, UINT nID, ULONG_PTR pData);

  virtual void OnInitMenuPopup();
  virtual BOOL OnUnInitPopupMenu(HWND hWnd);

  // Customizing:
  // Set icon size
  void SetIconSize(int width, int height);

  // set the color in the bitmaps that is the background transparent color
  COLORREF SetBitmapBackground(COLORREF newColor);
  COLORREF GetBitmapBackground(); 

  // Return the last itemrect from a menubaritem.
  CRect GetLastActiveMenuRect();
  HMENU GetParent();
  BOOL IsPopup();
  BOOL SetPopup(BOOL bIsPopup=TRUE);

  BOOL SetItemData(UINT uiId, DWORD_PTR dwItemData, BOOL fByPos = FALSE);
  BOOL SetItemDataPtr(UINT uiId, void* pItemData, BOOL fByPos = FALSE);

  DWORD_PTR GetItemData(UINT uiId, BOOL fByPos = FALSE) const;
  void* GetItemDataPtr(UINT uiId, BOOL fByPos = FALSE) const;

  BOOL SetMenuData(DWORD_PTR dwMenuData);
  BOOL SetMenuDataPtr(void* pMenuData);

  DWORD_PTR GetMenuData() const;
  void* GetMenuDataPtr() const;

// Miscellaneous Protected Member functions
protected:
  CNewMenuIcons* GetMenuIcon(int &nIndex, UINT nID, CImageList *pil, int xoffset);
  CNewMenuIcons* GetMenuIcon(int &nIndex, int nID);
  CNewMenuIcons* GetMenuIcon(int &nIndex, CBitmap* pBmp);

  CNewMenuIcons* GetToolbarIcons(UINT nToolBar, HMODULE hInst=NULL);

  DWORD SetMenuIcons(CNewMenuIcons* pMenuIcons);
  BOOL Replace(UINT nID, UINT nNewID);

  static BOOL IsNewShell();
  BOOL IsMenuBar(HMENU hMenu=NULL);

  void SetLastMenuRect(HDC hDC, LPRECT pRect);

  CNewMenuItemData* FindMenuItem(UINT nID);
  CNewMenu* FindMenuOption(int nId, int& nLoc);
  CNewMenu* FindMenuOption(LPCTSTR lpstrText, int& nLoc);

  CNewMenu* FindAnotherMenuOption(int nId, int& nLoc, CArray<CNewMenu*,CNewMenu*>&newSubs, CArray<int,int&>&newLocs);

  CNewMenuItemData* NewODMenu(UINT pos, UINT nFlags, UINT nID, LPCTSTR string);

  void SynchronizeMenu();
  void InitializeMenuList(int value);
  void DeleteMenuList();
  
  CNewMenuItemData* FindMenuList(UINT nID);
  CNewMenuItemData* CheckMenuItemData(ULONG_PTR nItemData) const;

  void DrawSpecial_OldStyle(CDC* pDC, LPCRECT pRect, UINT nID, DWORD dwStyle);
  void DrawSpecial_WinXP(CDC* pDC, LPCRECT pRect, UINT nID, DWORD dwStyle);

  void DrawSpecialCharStyle(CDC* pDC, LPCRECT pRect, TCHAR Sign, DWORD dwStyle);
  void DrawSpecialChar(CDC* pDC, LPCRECT pRect, TCHAR Sign, BOOL bBold);

  void DrawMenuTitle(LPDRAWITEMSTRUCT lpDIS, BOOL bIsMenuBar);

  // Measure an item
  void MeasureItem_OldStyle( LPMEASUREITEMSTRUCT lpMIS, BOOL bIsMenuBar); 
  void DrawItem_OldStyle (LPDRAWITEMSTRUCT lpDIS, BOOL bIsMenubar);
  BOOL Draw3DCheckmark(CDC* dc, const CRect& rc, HBITMAP hbmCheck, DWORD dwStyle);

  void MeasureItem_WinXP( LPMEASUREITEMSTRUCT lpMIS, BOOL bIsMenuBar); 
  void DrawItem_WinXP (LPDRAWITEMSTRUCT lpDIS, BOOL bIsMenuBar);

  void DrawItem_SpecialStyle (LPDRAWITEMSTRUCT lpDIS, BOOL bIsMenubar);

//  BOOL ImageListDuplicate(CImageList* il,int xoffset,CImageList* newlist);
  void ColorBitmap(CDC* pDC, CBitmap& bmp, CSize size, COLORREF fill, COLORREF border, int hatchstyle=-1);
  
// Member Variables
public:
  static DWORD m_dwLastActiveItem;

protected: 
  // Stores list of menu items
  CTypedPtrArray<CPtrArray, CNewMenuItemData*> m_MenuItemList;   
  // When loading an owner-drawn menu using a Resource, CNewMenu must keep track of
  // the popup menu's that it creates. Warning, this list *MUST* be destroyed
  // last item first :)
  // Stores list of sub-menus
  CTypedPtrArray<CPtrArray, HMENU>  m_SubMenus;

  static BOOL m_bEnableXpBlendig;
  static BOOL m_bSelectDisable;
  static CMenuTheme* m_pActMenuDrawing;
  static LOGFONT m_MenuTitleFont;
  static CTypedPtrList<CPtrList, CNewMenuIcons*>* m_pSharedMenuIcons;

  int m_iconX;
  int m_iconY;

  COLORREF m_bitmapBackground;

  CImageList* m_checkmaps;
  BOOL m_checkmapsshare;

  int m_selectcheck;
  int m_unselectcheck;

  BOOL m_bDynIcons;

  HMENU m_hParentMenu;

  BOOL m_bIsPopupMenu;

  CRect m_LastActiveMenuRect;

  DWORD m_dwOpenMenu;

  void* m_pData;
};

/////////////////////////////////////////////////////////////////////////////
// CNewFrame<> template for easy using of the new menu

template<class baseClass>
class CNewFrame : public baseClass
{
  typedef CNewFrame<baseClass> MyNewFrame;
public:
  CNewMenu m_DefaultNewMenu;
  CNewMenu m_SystemNewMenu;

public:

  CNewFrame()
  {
    m_bInMenuLoop = FALSE;
    m_TimerID = NULL;
    m_menubarItemIndex = UINT(-1);
  }

  // control bar docking
  void EnableDocking(DWORD dwDockStyle);

private:
  static const AFX_MSGMAP_ENTRY _messageEntries[];

protected: 
  static const AFX_MSGMAP messageMap;

  BOOL m_bInMenuLoop;
  UINT m_TimerID;
  UINT m_menubarItemIndex;


#if _MFC_VER < 0x0700 
  static const AFX_MSGMAP* PASCAL _GetBaseMessageMap()
  { 
    return &baseClass::messageMap; 
  };
#else
  static const AFX_MSGMAP* PASCAL GetThisMessageMap()
  {
    return &CNewFrame<baseClass>::messageMap; 
  }
#endif

  virtual const AFX_MSGMAP* GetMessageMap() const 
  {
    return &CNewFrame<baseClass>::messageMap; 
  }

  static const AFX_MSGMAP_ENTRY* GetMessageEntries()
  {
    static const AFX_MSGMAP_ENTRY Entries[] =
    {
      ON_WM_MEASUREITEM()
      ON_WM_MENUCHAR()
      ON_WM_INITMENUPOPUP()
      ON_WM_ENTERMENULOOP()
      ON_WM_EXITMENULOOP() 
      ON_WM_TIMER()
      ON_WM_CREATE()
      ON_WM_NCHITTEST()
      ON_WM_DESTROY()
      ON_WM_SYSCOLORCHANGE()
      {0, 0, 0, 0, AfxSig_end, (AFX_PMSG)0 }
    }; 
    return Entries;
  }

  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct)
  {
    if (baseClass::OnCreate(lpCreateStruct) == -1)
      return -1;

    m_SystemNewMenu.Attach(::GetSystemMenu(m_hWnd,FALSE));
    return 0;
  }

  afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
  {
    baseClass::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
    CNewMenu::OnInitMenuPopup(m_hWnd,pPopupMenu, nIndex, bSysMenu);
  }

  afx_msg void OnSysColorChange() 
  {
    baseClass::OnSysColorChange();
    CNewMenu::OnSysColorChange();
  }

  afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu)
  {
    m_bInMenuLoop = TRUE;
    if(m_TimerID!=NULL)
    {
      KillTimer(m_TimerID);
      m_TimerID = NULL;
    }
    if (m_menubarItemIndex!=UINT(-1))
    {
      DrawMenubarItem(this,GetMenu(),m_menubarItemIndex,NULL);
      m_menubarItemIndex=UINT(-1);
    }
    baseClass::OnEnterMenuLoop(bIsTrackPopupMenu);
  }

  afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu)
  {
    m_bInMenuLoop = FALSE;
    baseClass::OnExitMenuLoop(bIsTrackPopupMenu);
  }

  afx_msg void OnTimer(UINT_PTR nIDEvent)
  {
    baseClass::OnTimer(nIDEvent);
    if(m_TimerID==nIDEvent)
    {   
      CPoint pt;
      GetCursorPos(&pt);
      SendMessage(WM_NCHITTEST, 0, MAKELONG(pt.x, pt.y));
    }
  }

  afx_msg void OnDestroy()
  {
    if(m_TimerID!=NULL)
    {
      KillTimer(m_TimerID);
      m_TimerID = NULL;
    }
    baseClass::OnDestroy();
  }

  afx_msg UINT OnNcHitTest(CPoint point)
  {
    UINT nHitCode = baseClass::OnNcHitTest(point);
    // Test Win85/98/me and Win NT 4.0
    if(g_Shell<Win2000 || bRemoteSession)
    {
      UINT nStatus = IsChild(GetFocus()) ? 0: ODS_INACTIVE;
      CNewMenu* pNewMenu = DYNAMIC_DOWNCAST(CNewMenu,GetMenu());
      if (!m_bInMenuLoop && nHitCode == HTMENU)
      {
        // I support only CNewMenu ownerdrawings menu!!
        if(pNewMenu)
        {
          UINT nItemIndex = MenuItemFromPoint(m_hWnd, pNewMenu->m_hMenu, point);

          if ( nItemIndex!=(UINT)-1 )
          {
            if(m_menubarItemIndex!=nItemIndex)
            { 
              // Clear the old Item
              DrawMenubarItem(this,pNewMenu,m_menubarItemIndex,nStatus);

              // Draw the hotlight item.
              if(DrawMenubarItem(this,pNewMenu,nItemIndex,ODS_HOTLIGHT|nStatus))
              {
                // Set a new Timer
                if(m_TimerID==NULL)
                {
                  m_TimerID=GetSafeTimerID(m_hWnd,100);
                }
                m_menubarItemIndex = nItemIndex;
              }
              else
              {
                m_menubarItemIndex = UINT(-1);
              }
            }
            else
            {
              // Draw the hotlight item again.
              if(CNewMenu::m_dwLastActiveItem==NULL && 
                 DrawMenubarItem(this,pNewMenu,nItemIndex,ODS_HOTLIGHT|nStatus))
              {
                // Set a new Timer
                if(m_TimerID==NULL)
                {
                  m_TimerID=GetSafeTimerID(m_hWnd,100);
                }
                m_menubarItemIndex = nItemIndex;
              }
            }
            return nHitCode;
          }
        }
      }

      if (m_menubarItemIndex!=UINT(-1))
      {
        DrawMenubarItem(this,pNewMenu,m_menubarItemIndex,nStatus);
        m_menubarItemIndex=UINT(-1);
      }
      if(m_TimerID!=NULL)
      {
        KillTimer(m_TimerID);
        m_TimerID=NULL;
      }
    }
    return nHitCode;
  }

  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS) 
  {
    if(!CNewMenu::OnMeasureItem(GetCurrentMessage()))
    {
      baseClass::OnMeasureItem(nIDCtl, lpMIS);
    }
  } 

  afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu) 
  {
    LRESULT lresult;
    if( DYNAMIC_DOWNCAST(CNewMenu,pMenu) )
      lresult=CNewMenu::FindKeyboardShortcut(nChar, nFlags, pMenu);
    else
      lresult=baseClass::OnMenuChar(nChar, nFlags, pMenu);
    
    return lresult;
  }
};

#ifdef _AFXDLL
  #if _MFC_VER < 0x0700 
    template<class baseClass>
    const AFX_MSGMAP CNewFrame<baseClass>::messageMap = { &CNewFrame<baseClass>::_GetBaseMessageMap, GetMessageEntries()};
  #else
    template<class baseClass>
    const AFX_MSGMAP CNewFrame<baseClass>::messageMap = { &baseClass::GetThisMessageMap, GetMessageEntries()};
  #endif
#else
  template<class baseClass>
  const AFX_MSGMAP CNewFrame<baseClass>::messageMap = { &baseClass::messageMap, GetMessageEntries()};
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewMiniDockFrameWnd for docking toolbars with new menu

class CNewMiniDockFrameWnd: public CNewFrame<CMiniDockFrameWnd> 
{
  DECLARE_DYNCREATE(CNewMiniDockFrameWnd) 
};

// control bar docking
template<class baseClass>
void CNewFrame<baseClass>::EnableDocking(DWORD dwDockStyle)
{
  baseClass::EnableDocking(dwDockStyle);
  // Owerite registering for floating frame
  m_pFloatingFrameClass = RUNTIME_CLASS(CNewMiniDockFrameWnd);
}

/////////////////////////////////////////////////////////////////////////////
// CNewDialog for dialog implementation

class CNewDialog : public CNewFrame<CDialog>
{
  DECLARE_DYNAMIC(CNewDialog);

public:
  CNewDialog();
  CNewDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
  CNewDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);

  // Overridables (special message map entries)
  virtual BOOL OnInitDialog();

protected:
  // Generated message map functions
  //{{AFX_MSG(CNewDialog)
  afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CNewMiniFrameWnd for menu to documents

class CNewMiniFrameWnd : public CNewFrame<CMiniFrameWnd>
{
  DECLARE_DYNCREATE(CNewMiniFrameWnd)
};

/////////////////////////////////////////////////////////////////////////////
// CNewMDIChildWnd for menu to documents

class CNewMDIChildWnd : public CNewFrame<CMDIChildWnd>
{
  DECLARE_DYNCREATE(CNewMDIChildWnd)
};

/////////////////////////////////////////////////////////////////////////////
// CNewFrameWnd for menu to documents

class CNewFrameWnd : public CNewFrame<CFrameWnd>
{
  DECLARE_DYNCREATE(CNewFrameWnd)

public:
#if _MFC_VER < 0x0700 
    // dynamic creation - load frame and associated resources
  virtual BOOL LoadFrame(UINT nIDResource,
        DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
        CWnd* pParentWnd = NULL,
        CCreateContext* pContext = NULL);
#endif

  // under MFC 7.0 the next function is virtual so we don't neet to owerwrite
  // loadframe
  BOOL Create(LPCTSTR lpszClassName,
        LPCTSTR lpszWindowName,
        DWORD dwStyle = WS_OVERLAPPEDWINDOW,
        const RECT& rect = rectDefault,
        CWnd* pParentWnd = NULL,        // != NULL for popups
        LPCTSTR lpszMenuName = NULL,
        DWORD dwExStyle = 0,
        CCreateContext* pContext = NULL);
};

/////////////////////////////////////////////////////////////////////////////
// CNewMDIFrameWnd for menu to documents

class CNewMDIFrameWnd : public CNewFrame<CMDIFrameWnd>
{
  DECLARE_DYNCREATE(CNewMDIFrameWnd);
public: 

#if _MFC_VER < 0x0700 
    // dynamic creation - load frame and associated resources
  virtual BOOL LoadFrame(UINT nIDResource,
        DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
        CWnd* pParentWnd = NULL,
        CCreateContext* pContext = NULL);
#endif
    // under MFC 7.0 the next function is virtual so we don't neet to owerwrite
    // loadframe
    BOOL Create(LPCTSTR lpszClassName,
        LPCTSTR lpszWindowName,
        DWORD dwStyle = WS_OVERLAPPEDWINDOW,
        const RECT& rect = rectDefault,
        CWnd* pParentWnd = NULL,        // != NULL for popups
        LPCTSTR lpszMenuName = NULL,
        DWORD dwExStyle = 0,
        CCreateContext* pContext = NULL);
};


/////////////////////////////////////////////////////////////////////////////
// CNewMultiDocTemplate for menu to documents

class CNewMultiDocTemplate: public CMultiDocTemplate
{
  DECLARE_DYNAMIC(CNewMultiDocTemplate)

public:
  CNewMenu m_NewMenuShared;

// Constructors
public:
  CNewMultiDocTemplate(UINT nIDResource, CRuntimeClass* pDocClass,
                       CRuntimeClass* pFrameClass, CRuntimeClass* pViewClass);

  ~CNewMultiDocTemplate();
};

#pragma warning(pop)

#endif // __CNewMenu_H_
