/******************************************************************************************
TINY WINDOWS LIBRARY
copyright by Alberto Demichelis 2001 
email: albertodemichelis@hotmail.com
right now this code is not GPL or LGPL in any way
******************************************************************************************/
#ifndef _Tiny_WINDOW_H_
#define _Tiny_WINDOW_H_

#include <commctrl.h>
#pragma comment (lib , "comctl32.lib")
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyRect: public RECT{
public:
	_TinyRect(){
		left=0;
		right=0;
		top=0;
		bottom=0;
	}
	_TinyRect(RECT &rect){
		left=rect.left;
		right=rect.right;
		top=rect.top;
		bottom=rect.bottom;
	}
	_TinyRect(int w,int h){
		left=0;
		right=w;
		top=0;
		bottom=h;
	}
	_TinyRect(int x,int y,int w,int h){
		left=x;
		right=x+w;
		top=y;
		bottom=y+h;
	}
	
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK _TinyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK _TinyDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#define _TINY_DECLARE_APP()	\
	HINSTANCE g_hInstance;	\
	HINSTANCE g_hPrevInstance;	\
	LPTSTR g_lpCmdLine;	

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hPrevInstance;
extern LPTSTR g_lpCmdLine;	

inline ATOM __RegisterSmartClass(HINSTANCE hInstance,DWORD nIcon)
{
	WNDCLASS	wc;

    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) _TinyWndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(nIcon));
    wc.hCursor			= 0;
    wc.hbrBackground	= (HBRUSH) GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= _T("_default_TinyWindowClass");

	return RegisterClass(&wc);
}

inline BOOL _Tiny_InitApp(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPTSTR lpCmdLine,DWORD nIcon=0)
{
	g_hInstance=hInstance;
	g_hPrevInstance=hPrevInstance;
	g_lpCmdLine=lpCmdLine;
	INITCOMMONCONTROLSEX icc;
	icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC=ICC_TREEVIEW_CLASSES |ICC_BAR_CLASSES |ICC_LISTVIEW_CLASSES|ICC_COOL_CLASSES;
	::InitCommonControlsEx(&icc);
	return __RegisterSmartClass(hInstance,nIcon)?TRUE:FALSE;
}

inline int _Tiny_MainLoop()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

inline HINSTANCE _Tiny_GetInstance()
{
	return g_hInstance;
}

inline HINSTANCE _Tiny_GetPrevInstance()
{
	return g_hPrevInstance;
}

inline LPCTSTR _Tiny_GetCommandLine()
{
	return g_lpCmdLine;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
#define _BEGIN_MSG_MAP(__class)	\
	virtual LRESULT __Tiny_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)	\
	{	int wmId, wmEvent;\
	if(hWnd!=m_hWnd)DebugBreak();	\
	switch(message){

#define _BEGIN_DLG_MSG_MAP(__class)	\
	virtual LRESULT __Tiny_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)	\
	{	int wmId, wmEvent;\
	if(hWnd!=m_hWnd)DebugBreak();	\
	switch(message){


#define _MESSAGE_HANDLER(__message,__handler)	\
	case __message:		\
		return __handler(hWnd,message,wParam,lParam);	\
	break;	

#define _BEGIN_COMMAND_HANDLER()	\
	case WM_COMMAND:	\
		wmId    = LOWORD(wParam);	\
		wmEvent = HIWORD(wParam);	\
		switch(wmId){		\

#define _COMMAND_HANDLER(__wmId,__command_handler)	\
		case __wmId:	\
			return __command_handler(hWnd,message, wParam, lParam);	\
		break;	
#define _DEFAULT_DLG_COMMAND_HANDLERS()	\
		case IDOK:	\
			m_nModalRet=IDOK;\
			::OutputDebugString("IDOK\n"); \
			DestroyWindow(m_hWnd); \
		break;	\
		case IDCANCEL:	\
			m_nModalRet=IDCANCEL;\
			::OutputDebugString("IDCANCEL\n"); \
			DestroyWindow(m_hWnd); \
		break;	

#define _BEGIN_CMD_EVENT_FILTER(__wmId)	\
		case __wmId:	\
			switch(wmEvent){

#define _EVENT_FILTER(__wmEvent,__command_handler) \
		case __wmEvent:	\
			return __command_handler(hWnd,message, wParam, lParam);	\
		break;	

#define _END_CMD_EVENT_FILTER()	\
		default:	\
			break;	\
			}	\
		break;	

#define _END_COMMAND_HANDLER()	\
		default:	\
		  return DefWindowProc(hWnd, message, wParam, lParam);	\
		break;	\
		}	\
	break;

#define  _END_MSG_MAP()	\
	default:	\
		return DefWindowProc(hWnd, message, wParam, lParam);	\
	break;	\
	}	\
	return 0;	\
	}	

#define  _END_DLG_MSG_MAP()	\
	default:	\
		return FALSE;	\
	break;	\
	}	\
	return 0;	\
	}	

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyWindow{
	
public:
	_TinyWindow(){
		m_hWnd=NULL;
	}
	virtual ~_TinyWindow(){
		Close();
	}
	virtual void Close(){
		if(m_hWnd){
			SetWindowLong(m_hWnd,GWLP_USERDATA,NULL);
			DestroyWindow(m_hWnd);
			m_hWnd=NULL;
		}
	}
	virtual void Quit(){
		Close();
		PostQuitMessage(0);
	}

	virtual LRESULT __Tiny_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){return ::DefWindowProc(hWnd,message,wParam,lParam);}
	virtual BOOL Create(LPCTSTR lpszClassName,LPCTSTR lpszWindowName,DWORD dwStyle=WS_VISIBLE,DWORD dwExStyle=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL,ULONG nID=0){
		HWND hParent=NULL;
		BOOL bSmart=FALSE;
		int x=CW_USEDEFAULT,y=CW_USEDEFAULT,width=CW_USEDEFAULT,height=CW_USEDEFAULT;
		//class name
		if(!lpszClassName){
			lpszClassName=_T("_default_TinyWindowClass");
			bSmart=TRUE;
		}
		//parent
		if(pParentWnd!=NULL)hParent=pParentWnd->m_hWnd;
		//rect
		if(pRect){
			x=pRect->left;
			y=pRect->top;
			width=(pRect->right-pRect->left);
			height=(pRect->bottom-pRect->top);
		}
		//create
		m_hWnd=::CreateWindowEx(dwExStyle,lpszClassName,
			lpszWindowName,
			dwStyle,
			x,
			y,
			width,
			height,
			hParent,
			(HMENU)nID,
			_Tiny_GetInstance(),
			NULL);
		if(!m_hWnd)
		{
			DWORD n=::GetLastError();
			return FALSE;
		}
		__Tiny_WindowProc(m_hWnd,WM_CREATE,0,0);
		::SetWindowLongPtr(m_hWnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(this));
		return TRUE;
		
	}
	
public:
	HWND m_hWnd;
public:
	//wrappers
	virtual BOOL SetTimer(UINT nIDEvent,UINT uElapse){
		return (BOOL)::SetTimer(m_hWnd,nIDEvent,uElapse,NULL);
	}
	virtual BOOL KillTimer(UINT nIDEvent){
		return (BOOL)::KillTimer(m_hWnd,nIDEvent);
	}
	virtual BOOL ShowWindow(int nCmdShow=SW_SHOW){
		return (BOOL)::ShowWindow(m_hWnd,nCmdShow);
	}
	virtual BOOL SendMessage(UINT Msg,WPARAM wParam=0,LPARAM lParam=0){
		return (BOOL)::SendMessage(m_hWnd,Msg,wParam,lParam);
	}
	virtual BOOL PostMessage(UINT Msg,WPARAM wParam=0,LPARAM lParam=0){
		return (BOOL)::PostMessage(m_hWnd,Msg,wParam,lParam);
	}
	virtual BOOL SetWindowText(LPCTSTR lpString){
		return (BOOL)::SetWindowText(m_hWnd,lpString);
	}
	virtual BOOL GetWindowText(LPTSTR lpString,int nMaxCount){
		return (BOOL)::GetWindowText(m_hWnd,lpString,nMaxCount);
	}
	virtual BOOL GetClientRect(_TinyRect *pRect){
		return (BOOL)::GetClientRect(m_hWnd,pRect);
	}
	virtual BOOL SetWindowPos(int x,int y,int cx,int cy,UINT flags){
		return (BOOL)::SetWindowPos(m_hWnd,0,x,y,cx,cy,flags);
	}
		
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyDialog : public _TinyWindow{
public:
	BOOL Create(LPCTSTR lpTemplate,_TinyWindow *pParent=NULL){
		m_nModalRet=0;
		HWND hParent=NULL;
		if(pParent)hParent=pParent->m_hWnd;
		m_hWnd=CreateDialog(_Tiny_GetInstance(),lpTemplate,hParent,(DLGPROC)_TinyDlgProc);
		if(!m_hWnd)return FALSE;
		//fake the WM_CREATE message
		//__Tiny_WindowProc(m_hWnd,WM_CREATE,0,0);
		return ::SetWindowLongPtr(m_hWnd,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(this) );
	}
	int DoModal(){
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return m_nModalRet;
	}
public:
	int m_nModalRet;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyFrameWindow: public _TinyWindow{
public:
	_TinyFrameWindow()
	{
		m_hMenu=NULL;
	}
#ifdef WIN_CE
	BOOL AddBarMenu(WORD idMenu){
		m_hCommandBar = ::CommandBar_Create(_Tiny_GetInstance(), m_hWnd, 1);			
		::CommandBar_InsertMenubar(m_hCommandBar , _Tiny_GetInstance(), idMenu, 0);
		return ::CommandBar_AddAdornments(m_hCommandBar , 0, 0);
	}
	HWND m_hCommandBar;
#else
	bool AddMenu(WORD idMenu)
	{
		_asm int 3;
		m_hMenu=LoadMenu(_Tiny_GetInstance(),MAKEINTRESOURCE(idMenu));
		//<<FIXME>>
	}
	HMENU m_hMenu;
#endif
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyEdit: public _TinyWindow{
public:
	virtual BOOL Create(ULONG nID=0,DWORD dwStyle=WS_VISIBLE|WS_CHILD,DWORD dwExStyle=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL){
		BOOL bRes=_TinyWindow::Create(_T("EDIT"),_T(""),dwStyle,dwExStyle,pRect,pParentWnd,nID);
		if(!bRes)return FALSE;
		return TRUE;		
	}
	virtual void AppendText(const TCHAR *sText)
	{
		SendMessage(EM_SETSEL,-1,-1);
		SendMessage(EM_REPLACESEL,0,(LPARAM)sText);
		SendMessage(EM_SCROLLCARET,0,0);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyStatic: public _TinyWindow{
public:
	virtual BOOL Create(ULONG nID=0,DWORD dwStyle=WS_VISIBLE,DWORD dwStyleEx=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL){
		BOOL bRes=_TinyWindow::Create(_T("STATIC"),_T(""),dwStyle,dwStyleEx,pRect,pParentWnd,nID);
		if(!bRes)return FALSE;
		return TRUE;		
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyToolbar: public _TinyWindow{
public:
	virtual BOOL Create(ULONG nMenuID=0,DWORD dwStyle=WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS,DWORD dwStyleEx=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL)
	{
		BOOL bRes=_TinyWindow::Create(TOOLBARCLASSNAME,_T(""),dwStyle,dwStyleEx,pRect,pParentWnd,nMenuID);
		if(!bRes)
			return FALSE;
		SendMessage(TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
		return TRUE;
	}

	virtual BOOL AddButtons(DWORD nBitmapID,TBBUTTON *pButtons,DWORD nNumOfButtons)
	{
		TBADDBITMAP tb;
		tb.hInst = _Tiny_GetInstance();
		tb.nID = nBitmapID;
		DWORD stdidx= SendMessage (TB_ADDBITMAP, nNumOfButtons, (LPARAM)&tb);
		for (DWORD index = 0; index < nNumOfButtons; index++)
			pButtons[index].iBitmap += stdidx;
		SendMessage (TB_ADDBUTTONS, nNumOfButtons, (LPARAM) pButtons);
		return TRUE;
	}
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyListBox: public _TinyWindow{
public:
	virtual BOOL Create(ULONG nID=0,DWORD dwStyle=WS_CHILD,DWORD dwStyleEx=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL)
	{
		BOOL bRes=_TinyWindow::Create(_T("LISTBOX"),_T(""),dwStyle,dwStyleEx,pRect,pParentWnd,nID);
		if(!bRes)return FALSE;
		return TRUE;		
	}
	virtual int AddString(const TCHAR *sText)
	{
		return SendMessage(LB_ADDSTRING,0,(LPARAM)sText);
	}
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyButton: public _TinyWindow{
public:
	virtual BOOL Create(ULONG nID=0,DWORD dwStyle=WS_CHILD,DWORD dwStyleEx=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL)
	{
		BOOL bRes=_TinyWindow::Create(_T("BUTTON"),_T(""),dwStyle,dwStyleEx,pRect,pParentWnd,nID);
		if(!bRes)return FALSE;
		return TRUE;		
	}
	virtual int AddString(const TCHAR *sText)
	{
		return SendMessage(LB_ADDSTRING,0,(LPARAM)sText);
	}
	
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyListView: public _TinyWindow{
public:
	virtual BOOL Create(ULONG nID=0,DWORD dwStyle=WS_CHILD,DWORD dwStyleEx=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL)
	{
		BOOL bRes=_TinyWindow::Create(WC_LISTVIEW,_T(""),dwStyle,dwStyleEx,pRect,pParentWnd,nID);
		if(!bRes)return FALSE;
		return TRUE;		
	}
	virtual void SetViewStyle(DWORD dwView) 
	{ 
		// Retrieve the current window style. 
		DWORD dwStyle = GetWindowLong(m_hWnd, GWL_STYLE); 

		// Only set the window style if the view bits have changed. 
		if ((dwStyle & LVS_TYPEMASK) != dwView) 
			SetWindowLong(m_hWnd, GWL_STYLE, 
			(dwStyle & ~LVS_TYPEMASK) | dwView); 
	} 
	virtual int InsertColumn(int nCol,LPCTSTR lpszColumnHeading,int nWidth,int nFormat = LVCFMT_LEFT)
	{
		LVCOLUMN lvc;
		lvc.mask=LVCF_WIDTH|LVCF_FMT|LVCF_TEXT;
		lvc.cx=nWidth;
		lvc.pszText=(LPSTR)lpszColumnHeading;
		ListView_InsertColumn(m_hWnd,nCol,&lvc);
	}
	
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyTreeView: public _TinyWindow{
public:
	virtual BOOL Create(ULONG nID=0,DWORD dwStyle=WS_CHILD|TVS_HASLINES|TVS_HASBUTTONS,DWORD dwStyleEx=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL)
	{
		BOOL bRes=_TinyWindow::Create(WC_TREEVIEW,_T(""),dwStyle,dwStyleEx,pRect,pParentWnd,nID);
		if(!bRes)
			return FALSE;
		return TRUE;

	}
	virtual HTREEITEM AddItemToTree(LPTSTR lpszItem,LPARAM ud=NULL,HTREEITEM hParent=NULL)
	{ 
		TVITEM tv;
		TVINSERTSTRUCT tvins; 
		static HTREEITEM hPrev = (HTREEITEM) TVI_FIRST; 
		memset(&tv,0,sizeof(TVITEM));
		tv.mask=TVIF_TEXT|TVIF_PARAM;
		tv.pszText=lpszItem;
		tv.lParam=ud;
		tvins.item = tv; 
    tvins.hInsertAfter = hPrev; 
		if(hParent==NULL)
			tvins.hParent=TVI_ROOT;
		else
			tvins.hParent=hParent;
		hPrev = (HTREEITEM) SendMessage(TVM_INSERTITEM, 0, 
         (LPARAM) (LPTVINSERTSTRUCT) &tvins); 

		return hPrev; 
	} 
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
class _TinyScrollBar: public _TinyWindow{
public:
	virtual BOOL Create(ULONG nID=0,DWORD dwStyle=WS_VISIBLE,DWORD dwStyleEx=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL){
		BOOL bRes=_TinyWindow::Create(_T("SCROLLBAR"),_T(""),dwStyle,dwStyleEx,pRect,pParentWnd,nID);
		int n=GetLastError();
		if(SBS_VERT&dwStyle)m_nBar=SB_VERT;
		if(SBS_HORZ&dwStyle)m_nBar=SB_HORZ;
		if(!bRes)return FALSE;
		return TRUE;		
	}
	virtual BOOL SetScrollPos(int nPos,BOOL bRedraw=TRUE){
		return ::SetScrollPos(m_hWnd,SB_CTL,nPos,bRedraw);
	}
	virtual BOOL SetScrollInfo(LPSCROLLINFO lpsi,BOOL bRedraw=TRUE){
		return ::SetScrollInfo(m_hWnd,SB_CTL,lpsi,bRedraw);
	}
	virtual int GetScrollPos(){
		SCROLLINFO si;
		si.cbSize=sizeof(SCROLLINFO);
		si.fMask=SIF_POS;
		::GetScrollInfo(m_hWnd,SB_CTL,&si);
		return si.nPos;
	}
private:
	int m_nBar;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK _TinyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	_TinyWindow *pWnd;
	pWnd=(_TinyWindow *)::GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if(pWnd) return pWnd->__Tiny_WindowProc(hWnd,message,wParam,lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}

static BOOL CALLBACK _TinyDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	_TinyWindow *pWnd;
	pWnd=(_TinyWindow *)::GetWindowLongPtr(hWnd,GWLP_USERDATA);
	if(pWnd) return (BOOL)pWnd->__Tiny_WindowProc(hWnd,message,wParam,lParam);
	else return FALSE;
}

#endif //_Tiny_WINDOW_H_