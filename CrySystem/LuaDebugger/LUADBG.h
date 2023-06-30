#pragma once

#include "_TinyMain.h"
#include "_TinySplitter.h"
#include "_TinyStatusBar.h"
#include "_TinyCaptionWindow.h"

#include "FileTree.h"

#include <IScriptSystem.h>

#include "..\resource.h"
#include <stdio.h>
#include <list>

#define IDC_SOURCE 666
#define IDC_LOCALS 667
#define IDC_WATCH 668
#define IDC_FILES 669

#define BORDER_SIZE 50

class CSourceEdit : public _TinyWindow
{
	public:
	CSourceEdit() { m_iLineMarkerPos = 1; };
	~CSourceEdit() { };

protected:
	_BEGIN_MSG_MAP(CSourceEdit)
		_MESSAGE_HANDLER(WM_SIZE,OnSize)
		_MESSAGE_HANDLER(WM_PAINT,OnPaint)
		_BEGIN_COMMAND_HANDLER()
			_BEGIN_CMD_EVENT_FILTER(IDC_SOURCE)
					_EVENT_FILTER(EN_VSCROLL,OnScroll)
					_EVENT_FILTER(EN_HSCROLL,OnScroll)
					_EVENT_FILTER(EN_UPDATE ,OnScroll)
			_END_CMD_EVENT_FILTER()
		_END_COMMAND_HANDLER()
	_END_MSG_MAP()

public:
	
	void SetLineMarker(UINT iLine) {  m_iLineMarkerPos = iLine; };

	void SetScriptSystem(IScriptSystem *pIScriptSystem) { m_pIScriptSystem = pIScriptSystem; };
	
	BOOL Create(DWORD dwStyle=WS_VISIBLE,DWORD dwExStyle=0,const RECT* pRect=NULL,_TinyWindow *pParentWnd=NULL,ULONG nID=0)
	{
		_TinyRect erect;
		erect.left=BORDER_SIZE;
		erect.right=pRect->right;
		erect.top=0;
		erect.bottom=pRect->bottom;
		if(!_TinyWindow::Create(NULL,_T("asd"),WS_CHILD|WS_VISIBLE,0,pRect,pParentWnd,nID))
			return FALSE;
		if(!m_wEditWindow.Create(IDC_SOURCE,WS_VISIBLE|WS_CHILD|ES_WANTRETURN|WS_HSCROLL|WS_VSCROLL|ES_AUTOHSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|ES_NOHIDESEL,WS_EX_CLIENTEDGE,&erect,this))
			return FALSE;
		m_wEditWindow.SetFont((HFONT) GetStockObject(ANSI_FIXED_FONT));
		return TRUE;
	}

	BOOL SetText(char *pszText)
	{
		EDITSTREAM strm;
		char ***pppText = new char **;
		*pppText = &pszText;
		strm.dwCookie = (DWORD_PTR) pppText;
		strm.dwError = 0;
		strm.pfnCallback = EditStreamCallback;
		m_wEditWindow.SendMessage(EM_LIMITTEXT, 0x7FFFFFF, 0);
		return m_wEditWindow.SendMessage(EM_STREAMIN, SF_RTF, (LPARAM) &strm);
	};

	friend DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
	{
		char ***pppText = reinterpret_cast<char ***> (dwCookie);
		char **ppText = *pppText;
		LONG iLen = (LONG)strlen(* ppText) /*- 1*/;
		*pcb = __min(cb, iLen);
		if (*pcb == 0)
		{
			delete pppText;
			return 0;
		}
		memcpy(pbBuff, (* ppText), *pcb);
		*ppText += *pcb;
		return 0;
	};

	void SetSourceFile(const char *pszFileName)
	{
		m_strSourceFile = pszFileName;
	};

	const char * GetSourceFile() { return m_strSourceFile.c_str(); };

	void AddBreakpoint()
	{
		// Adds a breakpoint for the current file in the current line
		_TinyAssert(m_pIScriptSystem);
		HBREAKPOINT hBreakPt =  m_pIScriptSystem->AddBreakPoint(m_strSourceFile.c_str(), 
			m_wEditWindow.LineFromChar(m_wEditWindow.GetSel()));
		_TinyVerify(::InvalidateRect(m_wEditWindow.m_hWnd, NULL, FALSE));
	}

	void ScrollToLine(UINT iLine)
	{
		m_wEditWindow.ScrollToLine(iLine);
	};

private:
	LRESULT OnEraseBkGnd(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam){return 1;};
	LRESULT OnPaint(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		_TinyRect rect;
		PAINTSTRUCT ps;
		HDC hDC;
		BeginPaint(m_hWnd,&ps);
		hDC=::GetDC(m_hWnd);
		int w,h;
		UINT i;
		int iLine;
		const char *pszFile = NULL;
		/////////////////////////////
		GetClientRect(&rect);
		w=rect.right-rect.left;
		h=rect.bottom-rect.top;
		_TinyRect rc(0,0,BORDER_SIZE,h);
		::DrawFrameControl(hDC, &rc, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED);
		::SetBkMode(hDC,TRANSPARENT);
		POINT pt;
		m_wEditWindow.GetScrollPos(&pt);
		int iFirstLine = m_wEditWindow.GetFirstVisibleLine() + 1;
		TEXTMETRIC tm;
		m_wEditWindow.GetTextMetrics(&tm);
		
		
		int iFontY=tm.tmHeight-tm.tmDescent;
		int y=-pt.y%iFontY+3;
		char sTemp[10];
		HRGN hOldRgn=::CreateRectRgn(0,0,0,0);
		HRGN hClippedRgn=::CreateRectRgn(0,0,BORDER_SIZE,h - 1);
		::GetWindowRgn(m_hWnd,hOldRgn);
		::SelectClipRgn(hDC,hClippedRgn);
		::SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
		while(y<h)
		{
			sprintf(sTemp,"%d",iFirstLine);
			::TextOut(hDC,2,y,sTemp,(int)strlen(sTemp));
			y+=iFontY;
			iFirstLine++;
		}
 
		// Draw current line marker
		HBRUSH brsh = ::CreateSolidBrush(0x00FFFF00);
		::SelectObject(hDC, brsh);
		POINT sTriangle[4];
		sTriangle[0].x = 36;
		sTriangle[0].y = 0;
		sTriangle[1].x = 46;
		sTriangle[1].y = 5;
		sTriangle[2].x = 36;
		sTriangle[2].y = 10;
		sTriangle[3].x = 36;
		sTriangle[3].y = 0;
		iFirstLine = m_wEditWindow.GetFirstVisibleLine();
		for (i=0; i<4; i++)
			sTriangle[i].y += (-pt.y % iFontY + 3) + (m_iLineMarkerPos - iFirstLine - 1) * iFontY + 2;
		Polygon(hDC, sTriangle, 4);

		
		// Breakpoints
		::DeleteObject(brsh);
		brsh = ::CreateSolidBrush(0x0000007F);
		::SelectObject(hDC, brsh); 
		/*
		std::list<UINT>::iterator it;
		for (it=m_lBreakPoints.begin(); it!=m_lBreakPoints.end(); it++)
		{
			INT iY = (-pt.y % iFontY + 3) + ((* it) - iFirstLine - 1) * iFontY + 7;
			Ellipse(hDC, 41 - 5, iY - 5, 41 + 5, iY + 5);
		}
		*/
		IScriptObject *pIBreakPoints = m_pIScriptSystem->GetBreakPoints();
		if (pIBreakPoints)
		{
			for (i=0; i<(UINT) pIBreakPoints->Count(); i++)
			{
				_SmartScriptObject pIBreakPt(m_pIScriptSystem, true);
				pIBreakPoints->GetAt(i + 1, pIBreakPt);
				if (pIBreakPt->GetValue("line", iLine))
				{
					if (pIBreakPt->GetValue("sourcefile", pszFile))
					{
						if (pszFile == m_strSourceFile)
						{
							INT iY = (-pt.y % iFontY + 3) + (iLine - iFirstLine - 1) * iFontY + 7;
							Ellipse(hDC, 41 - 5, iY - 5, 41 + 5, iY + 5);
						}
					}
				}
			}
			pIBreakPoints->Release();
			pIBreakPoints = NULL;
		}
		::DeleteObject(brsh);
		

		::SelectClipRgn(hDC,hOldRgn);
		::DeleteObject(hOldRgn);
		::DeleteObject(hClippedRgn);
		//::DeleteObject(hBrush);
		/////////////////////////////
		EndPaint(m_hWnd,&ps);
		return 0;
	}
	LRESULT OnSize(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		int w=LOWORD(lParam);
		int h=HIWORD(lParam);
		_TinyRect erect;
		erect.left=BORDER_SIZE;
		erect.right=w-BORDER_SIZE;
		erect.top=0;
		erect.bottom=h;
		m_wEditWindow.SetWindowPos(BORDER_SIZE,0,w-BORDER_SIZE,h,0);
		return 0;
	}
	LRESULT OnScroll(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		_TinyRect rect;
		GetClientRect(&rect);
		
		rect.top=0;
		rect.left=0;
		rect.right=BORDER_SIZE;
		return InvalidateRect(&rect);
	}

protected:
	_TinyRichEdit m_wEditWindow;
	UINT m_iLineMarkerPos;
	IScriptSystem *m_pIScriptSystem;
	string m_strSourceFile;

};

class CLUADbg : public _TinyFrameWindow, IScriptObjectDumpSink
{
public:
	CLUADbg();
	virtual ~CLUADbg();

	bool LoadFile(const char *pszFile, bool bForceReload = false);
	void PlaceLineMarker(UINT iLine);
	void SetStatusBarText(const char *pszText);
	void GetStackAndLocals();

	void SetSystem(ISystem *pISystem)
	{
		SetScriptSystem(pISystem->GetIScriptSystem());
		m_pIPak = pISystem->GetIPak();
	};
	IScriptSystem * GetScriptSystem() { return m_pIScriptSystem; };

	bool IsUserDisabled() const { return m_bDisabled; };

	// For callback use byIScriptObjectDumpSink only, don't call directly
	void OnElementFound(const char *sName,ScriptVarType type);
	void OnElementFound(int nIdx,ScriptVarType type);

protected:
	_BEGIN_MSG_MAP(CMainWindow)
		_MESSAGE_HANDLER(WM_CLOSE,OnClose)
		_MESSAGE_HANDLER(WM_SIZE,OnSize)
		_MESSAGE_HANDLER(WM_CREATE,OnCreate)
		_MESSAGE_HANDLER(WM_ERASEBKGND,OnEraseBkGnd)
			_BEGIN_COMMAND_HANDLER()
			_COMMAND_HANDLER(IDM_EXIT, OnClose)
			_COMMAND_HANDLER(IDM_ABOUT, OnAbout)
			_COMMAND_HANDLER(ID_DEBUG_RUN, OnDebugRun)
			_COMMAND_HANDLER(ID_DEBUG_TOGGLEBREAKPOINT, OnToggleBreakpoint)
			_COMMAND_HANDLER(ID_DEBUG_STEPINTO, OnDebugStepInto)
			_COMMAND_HANDLER(ID_DEBUG_STEPOVER, OnDebugStepOver)
			_COMMAND_HANDLER(ID_DEBUG_DISABLE, OnDebugDisable)
			_COMMAND_HANDLER(ID_FILE_RELOAD, OnFileReload)
		_END_COMMAND_HANDLER()
		_MESSAGE_HANDLER(WM_NOTIFY,OnNotify)
		
	_END_MSG_MAP()

	//
	LRESULT OnClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnEraseBkGnd(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnAbout(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnToggleBreakpoint(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam);
	LRESULT OnDebugStepInto(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		m_pIScriptSystem->DebugStepInto();
		OnClose(hWnd,message,wParam,lParam);
		return 1;
	};
	LRESULT OnDebugStepOver(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		m_pIScriptSystem->DebugStepNext();
		OnClose(hWnd,message,wParam,lParam);
		return 1;
	};
	LRESULT OnDebugRun(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_pIScriptSystem->GetBreakState() == bsStepNext ||
			m_pIScriptSystem->GetBreakState() == bsStepInto)
		{
			// Leave step-by-step debugging
			m_pIScriptSystem->DebugContinue();
		}
		OnClose(hWnd,message,wParam,lParam);
		return 1;
	};
	LRESULT OnDebugDisable(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		// This doesn't completely disable debugging, errors will still cause the debugger to pop up
		m_pIScriptSystem->DebugDisable();

		m_bDisabled = true;
		OnClose(hWnd,message,wParam,lParam);
		return 1;
	}
	LRESULT OnNotify(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		NMHDR *n = (NMHDR *) lParam;
		if (n->code == NM_DBLCLK) {
			if(n->hwndFrom == m_wFilesTree.m_hWnd)
			{
				const char *pszFileName = m_wFilesTree.GetCurItemFileName();
				LoadFile(pszFileName);
			}
			if(n->hwndFrom == m_wCallstack.m_hWnd)
			{
				char temp[512];
				int sel=m_wCallstack.GetSelection();
				if(sel!=-1)
				{
					m_wCallstack.GetItemText(sel,1,temp,sizeof(temp));
					int linenum=atoi(temp);
					if(linenum!=-1){
						m_wCallstack.GetItemText(sel,2,temp,sizeof(temp));
						LoadFile(temp);
						PlaceLineMarker(linenum);
						
					}
				}

				//jumping in the carrect func
			}
		}
		return 1;
	}
		
	LRESULT OnFileReload(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
	{
		string strFileName = m_wScriptWindow.GetSourceFile();

		if ((* strFileName.begin()) == '@')
			strFileName.erase(strFileName.begin());

		LoadFile(strFileName.c_str(), true);
		return 1;
	}

	bool Reshape(int w, int h);
	HTREEITEM AddVariableToTree(const char *sName, ScriptVarType type, HTREEITEM hParent = NULL);
	void DumpTable(HTREEITEM hParent, IScriptObject *pITable, UINT& iRecursionLevel);

	void SetScriptSystem(IScriptSystem *pIScriptSystem)
	{ 
		m_pIScriptSystem = pIScriptSystem; 
		m_wScriptWindow.SetScriptSystem(pIScriptSystem); 
	};

	CSourceEdit m_wScriptWindow;
	_TinyToolbar m_wToolbar;
	_TinyTreeView m_wLocals;
	_TinyTreeView m_wWatch;
	_TinyListView m_wCallstack;

	CFileTree m_wFilesTree;
	
	_TinyWindow m_wndClient;

	_TinyStatusBar m_wndStatus;

	_TinyCaptionWindow m_wndLocalsCaption;
	_TinyCaptionWindow m_wndWatchCaption;
	_TinyCaptionWindow m_wndCallstackCaption;
	_TinyCaptionWindow m_wndFileViewCaption;
	_TinyCaptionWindow m_wndSourceCaption;

	_TinySplitter m_wndMainHorzSplitter;
	_TinySplitter m_wndWatchSplitter;
	_TinySplitter m_wndWatchCallstackSplitter;
	_TinySplitter m_wndSrcEditSplitter;

	IScriptSystem *m_pIScriptSystem;
	ICryPak *m_pIPak;

	bool m_bDisabled;

	// Used to let the enumeration function access it
	IScriptObject *m_pIVariable;
	HTREEITEM m_hRoot;
	UINT m_iRecursionLevel;
	_TinyTreeView *m_pTreeToAdd;

};