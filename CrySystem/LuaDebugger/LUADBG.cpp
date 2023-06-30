// LUADBG.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "LUADBG.h"
#include "_TinyRegistry.h"
#include "_TinyFileEnum.h"
#include "_TinyBrowseFolder.h"
#include "AboutWnd.h"
#include <ICryPak.h>

#include <direct.h>
#ifndef WIN64
#include "Shlwapi.h"
#pragma comment (lib, "Shlwapi.lib")
#endif

_TINY_DECLARE_APP();

CLUADbg::CLUADbg()
{
	m_pIScriptSystem = NULL;
	m_pIPak = NULL;
	m_pIVariable = NULL;
	m_hRoot = NULL;
	m_bDisabled = false;
	m_pTreeToAdd = NULL;
}

CLUADbg::~CLUADbg()
{
	_TinyRegistry cTReg;
	_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "MainHorzSplitter", m_wndMainHorzSplitter.GetSplitterPos()));
 	_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "SrcEditSplitter", m_wndSrcEditSplitter.GetSplitterPos()));
	_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "WatchSplitter", m_wndWatchSplitter.GetSplitterPos()));
	_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "WatchCallstackSplitter", m_wndWatchCallstackSplitter.GetSplitterPos()));
	_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "Fullscreen", ::IsZoomed(m_hWnd) ? 1 : 0));
}

TBBUTTON tbButtonsAdd [] = 
{
	{ 0, ID_DEBUG_RUN, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0L, 0 },
	{ 1, ID_DEBUG_BREAK, 0, BTNS_BUTTON, { 0 }, 0L, 0 },
	{ 2, ID_DEBUG_STOP, 0, BTNS_BUTTON, { 0 }, 0L, 0 },

	{ 0, 0, TBSTATE_ENABLED, BTNS_SEP, { 0 }, 0L, 0 },
	
	{ 3, ID_DEBUG_STEPINTO, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0L, 0 },
	{ 4, ID_DEBUG_STEPOVER, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0L, 0 },
	{ 5, ID_DEBUG_TOGGLEBREAKPOINT, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0L, 0 },
	{ 6, ID_DEBUG_DISABLE, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0L, 0 },

	{ 0, 0, TBSTATE_ENABLED, BTNS_SEP, { 0 }, 0L, 0 },

	{ 7, IDM_ABOUT, TBSTATE_ENABLED, BTNS_BUTTON, { 0 }, 0L, 0 },
};


//! add a backslash if needed
inline void MyPathAddBackslash( char* szPath )
{
	if(szPath[0]==0) 
		return;

	size_t nLen = strlen(szPath);

	if (szPath[nLen-1] == '\\')
		return;

	if (szPath[nLen-1] == '/')
	{
		szPath[nLen-1] = '\\';
		return;
	}

	szPath[nLen] = '\\';
	szPath[nLen+1] = '\0';
}


LRESULT CLUADbg::OnCreate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	_TinyRect erect;
	_TinyRect lrect;
	_TinyRect wrect;
	_TinyRegistry cTReg;

	DWORD dwXOrigin=0,dwYOrigin=0,dwXSize=800,dwYSize=600;
	if(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "XOrigin",dwXOrigin))
	{
		cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "YOrigin",dwYOrigin);
		cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "XSize",dwXSize);
		cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "YSize",dwYSize);
	}
	
	SetWindowPos(dwXOrigin, dwYOrigin, dwXSize, dwYSize, SWP_NOZORDER | SWP_NOMOVE);
	// TODO: Make sure fullscreen flag is loaded and used
	// WS_MAXIMIZE
	// DWORD dwFullscreen = 0;
	// cTReg.ReadNumber("Software\\Tiny\\LuaDebugger\\", "Fullscreen", dwFullscreen);
	// if (dwFullscreen == 1)
    // ShowWindow(SW_MAXIMIZE);
	CenterOnScreen();
	GetClientRect(&wrect);

	erect=wrect;
	erect.top=0;
	erect.bottom=32;
	m_wToolbar.Create((ULONG_PTR) GetMenu(m_hWnd), WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS, 0, &erect, this);	//AMD Port
	m_wToolbar.AddButtons(IDC_LUADBG, tbButtonsAdd, 10);
	
	m_wndStatus.Create(0, NULL, this);

	// Client area window
	_TinyVerify(m_wndClient.Create(NULL, _T(""), WS_CHILD | WS_VISIBLE, 0, &wrect, this));
	m_wndClient.NotifyReflection(TRUE);

	// Splitter dividing source/file and watch windows
	m_wndMainHorzSplitter.Create(&m_wndClient, NULL, NULL, true);

	// Divides source and file view
	m_wndSrcEditSplitter.Create(&m_wndMainHorzSplitter);

	// Divides two watch windows
	m_wndWatchSplitter.Create(&m_wndMainHorzSplitter);

	// Divides the watch window and the cllstack
	m_wndWatchCallstackSplitter.Create(&m_wndWatchSplitter);
	
	// Add all scripts to the file tree
	m_wFilesTree.Create(erect, &m_wndSrcEditSplitter, IDC_FILES);
	char szRootPath[_MAX_PATH];
	_getcwd(szRootPath, _MAX_PATH);
	MyPathAddBackslash(szRootPath);
	strcat(szRootPath, "SCRIPTS\\");
	m_wFilesTree.ScanFiles(szRootPath);

	_TinyVerify(m_wScriptWindow.Create(WS_VISIBLE | WS_CHILD | ES_WANTRETURN | WS_HSCROLL | WS_VSCROLL | 
		ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE, 0, &erect, &m_wndSrcEditSplitter));
	m_wScriptWindow.SendMessage(EM_SETEVENTMASK,0,ENM_SCROLL);

	m_wndFileViewCaption.Create("Scripts", &m_wFilesTree, &m_wndSrcEditSplitter);
	m_wndSourceCaption.Create("Source", &m_wScriptWindow, &m_wndSrcEditSplitter);
	m_wndSrcEditSplitter.SetFirstPan(&m_wndFileViewCaption);
	m_wndSrcEditSplitter.SetSecondPan(&m_wndSourceCaption);

	m_wLocals.Create(IDC_LOCALS,WS_CHILD|TVS_HASLINES|TVS_HASBUTTONS|TVS_LINESATROOT|WS_VISIBLE,WS_EX_CLIENTEDGE,&erect, &m_wndWatchSplitter);
	m_wWatch.Create(IDC_WATCH,WS_CHILD|WS_VISIBLE|TVS_HASLINES|TVS_HASBUTTONS|TVS_LINESATROOT,WS_EX_CLIENTEDGE,&erect, &m_wndWatchSplitter);
	m_wCallstack.Create(0,WS_CHILD|WS_VISIBLE|LVS_REPORT,WS_EX_CLIENTEDGE,&erect, &m_wndWatchSplitter);

	m_wndLocalsCaption.Create("Locals", &m_wLocals, &m_wndWatchSplitter);
	m_wndWatchCaption.Create("Watch", &m_wWatch, &m_wndWatchCallstackSplitter);
	m_wndCallstackCaption.Create("Callstack", &m_wCallstack, &m_wndWatchCallstackSplitter);

	m_wndWatchSplitter.SetFirstPan(&m_wndWatchCallstackSplitter);

	m_wndWatchCallstackSplitter.SetFirstPan(&m_wndWatchCaption);
	m_wndWatchCallstackSplitter.SetSecondPan(&m_wndCallstackCaption);

	m_wndWatchSplitter.SetSecondPan(&m_wndLocalsCaption);

	m_wndMainHorzSplitter.SetFirstPan(&m_wndSrcEditSplitter);
	m_wndMainHorzSplitter.SetSecondPan(&m_wndWatchSplitter);

	Reshape(wrect.right-wrect.left,wrect.bottom-wrect.top);

	// Read splitter window locations from registry
	DWORD dwVal;
	cTReg.ReadNumber("Software\\Tiny\\LuaDebugger\\", "MainHorzSplitter", dwVal, 150);
	m_wndMainHorzSplitter.SetSplitterPos(dwVal);
	m_wndMainHorzSplitter.Reshape();
	cTReg.ReadNumber("Software\\Tiny\\LuaDebugger\\", "SrcEditSplitter", dwVal, 190);
	m_wndSrcEditSplitter.SetSplitterPos(dwVal);
	m_wndSrcEditSplitter.Reshape();
	cTReg.ReadNumber("Software\\Tiny\\LuaDebugger\\", "WatchSplitter", dwVal, wrect.right / 3 * 2);
	m_wndWatchSplitter.SetSplitterPos(dwVal);
	m_wndWatchSplitter.Reshape();
	cTReg.ReadNumber("Software\\Tiny\\LuaDebugger\\", "WatchCallstackSplitter", dwVal, wrect.right / 3);
	m_wndWatchCallstackSplitter.SetSplitterPos(dwVal);
	m_wndWatchCallstackSplitter.Reshape();

	m_wCallstack.InsertColumn(0, "Function", 108, 0);
	m_wCallstack.InsertColumn(1, "Line", 40, 1);
	m_wCallstack.InsertColumn(2, "File", 108, 2);

	//_TinyVerify(LoadFile("C:\\MASTERCD\\SCRIPTS\\Default\\Entities\\PLAYER\\BasicPlayer.lua"));
	//_TinyVerify(LoadFile("C:\\MASTERCD\\SCRIPTS\\Default\\Entities\\PLAYER\\player.lua"));
	// _TINY_CHECK_LAST_ERROR
	
	return 0;
}

bool CLUADbg::LoadFile(const char *pszFile, bool bForceReload)
{
	FILE *hFile = NULL;
	char *pszScript = NULL, *pszFormattedScript = NULL;
	UINT iLength, iCmpBufPos, iSrcBufPos, iDestBufPos, iNumChars, iStrStartPos, iCurKWrd, i;
	char szCmpBuf[2048];
	bool bIsKeyWord;
	string strLuaFormatFileName = "@";
	char szMasterCD[_MAX_PATH];

	// Create lua specific filename and check if we already have that file loaded
	_getcwd(szMasterCD, _MAX_PATH);
	if (strncmp(szMasterCD, pszFile, strlen(szMasterCD) - 1) == 0)
	{
		strLuaFormatFileName += &pszFile[strlen(szMasterCD) + 1];
		for (i=0; i<strLuaFormatFileName.length(); i++)
			if (strLuaFormatFileName[i] == '\\')
				strLuaFormatFileName[i] = '/';
		std::transform(strLuaFormatFileName.begin(), strLuaFormatFileName.end(), 
			strLuaFormatFileName.begin(), (int (*) (int)) tolower);
	}
	else
		strLuaFormatFileName += pszFile;

	if (bForceReload == false && m_wScriptWindow.GetSourceFile() == strLuaFormatFileName)
		return true;

	m_wScriptWindow.SetSourceFile(strLuaFormatFileName.c_str());

	
	// hFile = fopen(pszFile, "rb");
	hFile = m_pIPak->FOpen(pszFile, "rb");

	if (!hFile)
		return false;

	// set filename in window title
	{
		char str[_MAX_PATH];

		sprintf(str,"Lua Debugger [%s]",pszFile);
		SetWindowText(str);
	}

	// Get file size
	// fseek(hFile, 0, SEEK_END);
	m_pIPak->FSeek(hFile, 0, SEEK_END);
	// iLength = ftell(hFile);
	iLength = m_pIPak->FTell(hFile);
	// fseek(hFile, 0, SEEK_SET);
	m_pIPak->FSeek(hFile, 0, SEEK_SET);

    pszScript = new char [iLength + 1];
	pszFormattedScript = new char [512 + iLength * 3];

	// _TinyVerify(fread(pszScript, iLength, 1, hFile) == 1);
	_TinyVerify(m_pIPak->FRead(pszScript, iLength, 1, hFile) == 1);
	pszScript[iLength] = '\0';
	_TinyAssert(strlen(pszScript) == iLength);

	// RTF text, font Courier, green comments and blue keyword
	strcpy(pszFormattedScript, "{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033" \
		"{\\fonttbl{\\f0\\fmodern\\fprq1\\fcharset0 Courier(PS);}{\\f1\\fswiss\\fcharset0 Arial;}}" \
		"{\\colortbl ;\\red0\\green0\\blue255;\\red0\\green128\\blue0;\\red160\\green160\\blue160;}\\f0\\fs20");

	const char szKeywords[][32] =
	{
		"function",
		"do",
		"for",
		"end",
		"and",
		"or",
		"not",
		"while",
		"return",
		"if",
		"then",
		"else",
		"elseif",
		"self",
		"local",
		"in",
		"nil",
		"repeat",
		"until",
		"break",
	};

	// Format text with syntax coloring
	iDestBufPos = strlen(pszFormattedScript);

	iSrcBufPos = 0;
	while (pszScript[iSrcBufPos] != '\0')
	{
		// Scan next token
		iNumChars = 1;
		iStrStartPos = iSrcBufPos;
		while (pszScript[iSrcBufPos] != ' '  &&
			   pszScript[iSrcBufPos] != '\n' &&
			   pszScript[iSrcBufPos] != '\r' &&
			   pszScript[iSrcBufPos] != '\t' &&
			   pszScript[iSrcBufPos] != '\0' &&
			   pszScript[iSrcBufPos] != '('  &&
			   pszScript[iSrcBufPos] != ')'  &&
			   pszScript[iSrcBufPos] != '['  &&
			   pszScript[iSrcBufPos] != ']'  &&
			   pszScript[iSrcBufPos] != '{'  &&
			   pszScript[iSrcBufPos] != '}'  &&
			   pszScript[iSrcBufPos] != ','  &&
			   pszScript[iSrcBufPos] != '.'  &&
			   pszScript[iSrcBufPos] != ';'  &&
			   pszScript[iSrcBufPos] != ':'  &&
			   pszScript[iSrcBufPos] != '='  &&
			   pszScript[iSrcBufPos] != '==' &&
			   pszScript[iSrcBufPos] != '*'  &&
			   pszScript[iSrcBufPos] != '+'  &&
			   pszScript[iSrcBufPos] != '/'  &&
			   pszScript[iSrcBufPos] != '~'  &&
			   pszScript[iSrcBufPos] != '"')
		{
			iSrcBufPos++;	
			iNumChars++;

			// Special treatment of '-' to allow parsing of '--'
			if (pszScript[iSrcBufPos - 1] == '-' && pszScript[iSrcBufPos] != '-')
				break;
		}
		if (iNumChars == 1)
			iSrcBufPos++;
		else
			iNumChars--;

		// Copy token and add escapes
		iCmpBufPos = 0;
		for (i=iStrStartPos; i<iStrStartPos + iNumChars; i++)
		{
			_TinyAssert(i - iStrStartPos < sizeof(szCmpBuf));

			if (pszScript[i] == '{' || pszScript[i] == '}' || pszScript[i] == '\\')
			{
				// Add \ to mark it as non-escape character
				szCmpBuf[iCmpBufPos++] = '\\';
				szCmpBuf[iCmpBufPos++] = pszScript[i];
				szCmpBuf[iCmpBufPos] = '\0';
			}
			else
			{
				szCmpBuf[iCmpBufPos++] = pszScript[i];
				szCmpBuf[iCmpBufPos] = '\0';
			}
		}

		// Comment
		if (strncmp(szCmpBuf, "--", 2) == 0)
		{
			// Green
			strcat(pszFormattedScript, "\\cf2 ");
			
			iDestBufPos += 5;
			
			strcpy(&pszFormattedScript[iDestBufPos], szCmpBuf);
			iDestBufPos += strlen(szCmpBuf);

			// Parse until newline
			while (pszScript[iSrcBufPos] != '\n' && pszScript[iSrcBufPos] != '\0')
			{
				pszFormattedScript[iDestBufPos++] = pszScript[iSrcBufPos++];
			}
			iSrcBufPos++;
			pszFormattedScript[iDestBufPos] = '\0';

			// Add newline and restore color
			strcat(pszFormattedScript, "\\par\n");
			iDestBufPos += 5;
			strcat(pszFormattedScript, "\\cf0 ");
			iDestBufPos += 5;

			continue;
		}
		
		// String
		if (strncmp(szCmpBuf, "\"", 2) == 0)
		{
			// Gray
			strcat(pszFormattedScript, "\\cf3 ");
			iDestBufPos += 5;
			
			strcpy(&pszFormattedScript[iDestBufPos], szCmpBuf);
			iDestBufPos += strlen(szCmpBuf);

			// Parse until end string / newline
			while (pszScript[iSrcBufPos] != '\n' && pszScript[iSrcBufPos] != '\0' && pszScript[iSrcBufPos] != '"')
			{
				pszFormattedScript[iDestBufPos++] = pszScript[iSrcBufPos++];
			}
			iSrcBufPos++;
			pszFormattedScript[iDestBufPos] = '\0';

			// Add literal
			strcat(pszFormattedScript, "\"");
			iDestBufPos += 1;

			// Restore color
			strcat(pszFormattedScript, "\\cf0 ");
			iDestBufPos += 5;

			continue;
		}

		// Have we parsed a keyword ?
		bIsKeyWord = false;
		for (iCurKWrd=0; iCurKWrd<sizeof(szKeywords) / sizeof(szKeywords[0]); iCurKWrd++)
		{
			if (strcmp(szKeywords[iCurKWrd], szCmpBuf) == 0)
			{
				strcat(pszFormattedScript, "\\cf1 ");
				strcat(pszFormattedScript, szKeywords[iCurKWrd]);
				strcat(pszFormattedScript, "\\cf0 ");
				iDestBufPos += 5 + 5 + strlen(szKeywords[iCurKWrd]);
				bIsKeyWord = true;
			}
		}
		if (bIsKeyWord)
			continue;

		if (strcmp(szCmpBuf, "\n") == 0)
		{
			// Newline
			strcat(pszFormattedScript, "\\par ");
			iDestBufPos += 5;
		}
		else
		{
			// Anything else, just append
			iDestBufPos += strlen(szCmpBuf);
			strcat(pszFormattedScript, szCmpBuf);
		}
	}

	if (hFile)
		// fclose(hFile);
		m_pIPak->FClose(hFile);

	if (pszScript)
	{
		delete [] pszScript;
		pszScript = NULL;
	}

	/*	
	hFile = fopen("C:\\Debug.txt", "w");
	fwrite(pszFormattedScript, 1, strlen(pszFormattedScript), hFile);
	fclose(hFile);
	*/

	m_wScriptWindow.SetText(pszFormattedScript);
	
	if (pszFormattedScript)
	{
		delete [] pszFormattedScript;
		pszFormattedScript = NULL;
	}

	return true;
}

LRESULT CLUADbg::OnEraseBkGnd(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	return 1;
}

extern bool g_bDone; // From LuaDbgInterface.cpp
LRESULT CLUADbg::OnClose(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	// Quit();
	WINDOWINFO wi;
	if(::GetWindowInfo(m_hWnd,&wi))
	{
		_TinyRegistry cTReg;
		_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "XOrigin", wi.rcWindow.left));
		_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "YOrigin", wi.rcWindow.top));
		_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "XSize", wi.rcWindow.right-wi.rcWindow.left));
		_TinyVerify(cTReg.WriteNumber("Software\\Tiny\\LuaDebugger\\", "YSize", wi.rcWindow.bottom-wi.rcWindow.top));
	}
	g_bDone=true;
	return 0;
}


LRESULT CLUADbg::OnSize(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
{

	int w=LOWORD(lParam);
	int h=HIWORD(lParam);
	Reshape(w,h);

	return 0;
}

LRESULT CLUADbg::OnAbout(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	CAboutWnd wndAbout;
	wndAbout.DoModal(this);
	return 0;
}

LRESULT CLUADbg::OnToggleBreakpoint(HWND hWnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	m_wScriptWindow.AddBreakpoint();
	return 0;
}

#define TOOLBAR_HEIGHT 28
#define STATUS_BAR_HEIGHT 20

bool CLUADbg::Reshape(int w,int h)
{
	/*
	int nWatchHeight=(((float)h)*WATCH_HEIGHT_MULTIPLIER);
	int nFilesWidth=(((float)h)*FILES_WIDTH_MULTIPLIER);
	m_wScriptWindow.SetWindowPos(nFilesWidth,TOOLBAR_HEIGHT,w-nFilesWidth,h-TOOLBAR_HEIGHT-nWatchHeight,SWP_DRAWFRAME);
	m_wLocals.SetWindowPos(0,h-nWatchHeight,w/2,nWatchHeight,SWP_DRAWFRAME);
	m_wFilesTree.SetWindowPos(nFilesWidth,TOOLBAR_HEIGHT,nFilesWidth,h-TOOLBAR_HEIGHT-nWatchHeight,SWP_DRAWFRAME);
	m_wWatch.SetWindowPos(w/2,h-nWatchHeight,w/2,nWatchHeight,SWP_DRAWFRAME);
	*/

	m_wndClient.Reshape(w, h - TOOLBAR_HEIGHT - STATUS_BAR_HEIGHT);
	m_wndClient.SetWindowPos(0, TOOLBAR_HEIGHT, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	m_wndMainHorzSplitter.Reshape(w, h - TOOLBAR_HEIGHT - STATUS_BAR_HEIGHT);
	m_wToolbar.SetWindowPos(0,0,w,TOOLBAR_HEIGHT,0);
	m_wndStatus.SetWindowPos(0, h -  STATUS_BAR_HEIGHT, w, STATUS_BAR_HEIGHT, NULL);

	return true;
}
 
void CLUADbg::PlaceLineMarker(UINT iLine)
{
	m_wScriptWindow.SetLineMarker(iLine);
	m_wScriptWindow.ScrollToLine(iLine);
}

void CLUADbg::SetStatusBarText(const char *pszText)
{
	// TODO: Find out why setting the panel text using the
	//       dedicated message doesn't work. For some reason the
	//       text gets drawn once but never again.
	// m_wndStatus.SetPanelText(pszText);

	m_wndStatus.SetWindowText(pszText);
}

void CLUADbg::GetStackAndLocals()
{
	///////////
	// Stack //
	///////////

	{
		IScriptObject *pICallstack = m_pIScriptSystem->GetCallsStack();
		_SmartScriptObject pIEntry(m_pIScriptSystem, true);
		const char *pszText = NULL;
		int iItem=0;
		m_wCallstack.Clear();
		int i;
		for (i=pICallstack->Count() - 1; i>=1; i--)
		{
			pICallstack->GetAt(i, pIEntry);

			pIEntry->GetValue("description", pszText);
			iItem = m_wCallstack.InsertItem(0, pszText, 0);

			pIEntry->GetValue("line", pszText);
			m_wCallstack.SetItemText(iItem, 1, pszText);

			if (pIEntry->GetValue("sourcefile", pszText))
			{
				if (_stricmp(pszText, "=C") == 0)
					m_wCallstack.SetItemText(iItem, 2, "Native C Function");
				else
					m_wCallstack.SetItemText(iItem, 2, &pszText[1]);
			}
			else
				m_wCallstack.SetItemText(iItem, 2, "No Source");

		}
		if (pICallstack->Count() == 0)
			iItem = m_wCallstack.InsertItem(0, "No Callstack Available", 0);
		pICallstack->Release();
		pICallstack = NULL;
	}

	////////////
	// Locals //
	////////////

	m_wLocals.Clear();
	m_pIVariable = m_pIScriptSystem->GetLocalVariables(1);
	m_pTreeToAdd = &m_wLocals;
	if (m_pIVariable)
	{
		m_hRoot = NULL;
		m_iRecursionLevel = 0;
		m_pIVariable->Dump((IScriptObjectDumpSink *) this);
		m_pIVariable->Release();
		m_pIVariable = NULL;
	}
	else
		m_wLocals.AddItemToTree("No Locals Available");
	m_pTreeToAdd = NULL;

	
	///////////
	// Watch //
	///////////

	const char *pszText = NULL;

	m_wWatch.Clear();
	IScriptObject *pIScriptObj = m_pIScriptSystem->GetLocalVariables(1);

	string strWatchVar = "self";

	bool bVarFound = false;
	pIScriptObj->BeginIteration();
	while (pIScriptObj->MoveNext())
	{
		if (pIScriptObj->GetCurrentKey(pszText))
		{
			if (strWatchVar == pszText)
			{
				_SmartScriptObject pIEntry(m_pIScriptSystem, true);
				pIScriptObj->GetCurrent(pIEntry);

				m_pIVariable = pIEntry;
				m_pTreeToAdd = &m_wWatch;

				if (m_pIVariable)
				{
					bVarFound = true;

					m_hRoot = NULL;
					m_iRecursionLevel = 0;

					// Dump only works for tables, in case of values call the sink directly
					if (m_pIVariable->GetCurrentType() == svtObject)
						m_pIVariable->Dump((IScriptObjectDumpSink *) this);
					else
					{
						m_pIVariable = pIScriptObj; // Value needs to be retrieved from parent table
						OnElementFound(pszText, m_pIVariable->GetCurrentType());
					}

					// No AddRef() !
					// m_pIVariable->Release();

					m_pIVariable = NULL;
				}
				
				m_pTreeToAdd = NULL;
			}
		}
	}
	pIScriptObj->EndIteration();

	if (!bVarFound)
		m_wWatch.AddItemToTree(const_cast<LPSTR>((strWatchVar + " = ?").c_str())); // TODO: Cast...
	
}

HTREEITEM CLUADbg::AddVariableToTree(const char *sName, ScriptVarType type, HTREEITEM hParent)
{
	char szBuf[2048];
	char szType[32];
	const char *pszContent = NULL;
	bool bRetrieved = false;
	int iIdx;

	if (m_pTreeToAdd == NULL)
	{
		_TinyAssert(m_pTreeToAdd !=	NULL);
		return 0;
	}

	switch (type)
	{
	case svtNull:
		strcpy(szType, "[nil]");
		break;
	case svtString:
		strcpy(szType, "[string]");
		break;
	case svtNumber:
		strcpy(szType, "[numeric]");
		break;
	case svtFunction:
		strcpy(szType, "[function]");
		break;
	case svtObject:
		strcpy(szType, "[table]");
		break;
	case svtUserData:
		strcpy(szType, "[user data]");
		break;
	default:
		strcpy(szType, "[unknown]");
		break;
	}

	if (type == svtString || type == svtNumber)
	{
		if (sName[0] == '[')
		{
			strcpy(szBuf, &sName[1]);
			szBuf[strlen(szBuf) - 1] = '\0';
			iIdx = atoi(szBuf);
			bRetrieved = m_pIVariable->GetAt(iIdx, pszContent);
		}
		else
			bRetrieved = m_pIVariable->GetValue(sName, pszContent);

		if (bRetrieved)
			sprintf(szBuf, "%s %s = %s", szType, sName, pszContent);
		else
			sprintf(szBuf, "%s %s = (Unknown)", szType, sName);
	}
	else
		sprintf(szBuf, "%s %s", szType, sName);

	HTREEITEM hNewItem = m_pTreeToAdd->AddItemToTree(szBuf, NULL, hParent);
	TreeView_SortChildren(m_pTreeToAdd->m_hWnd, hNewItem, FALSE);

	return hNewItem;
}

void CLUADbg::OnElementFound(const char *sName, ScriptVarType type)
{
	HTREEITEM hRoot = NULL;
	UINT iRecursionLevel = 0;
	_SmartScriptObject pTable(m_pIScriptSystem, true);
	IScriptObject *pIOldTbl = NULL;
	HTREEITEM hOldRoot = NULL;

	if(!sName)sName="[table idx]";
	hRoot = AddVariableToTree(sName, type, m_hRoot);

	if (type == svtObject && m_iRecursionLevel < 5)
	{
		if (m_pIVariable->GetValue(sName, pTable))
		{
			pIOldTbl = m_pIVariable;
			hOldRoot = m_hRoot;
			m_pIVariable = pTable;
			m_hRoot = hRoot;
			m_iRecursionLevel++;
			pTable->Dump((IScriptObjectDumpSink *) this);
			m_iRecursionLevel--;
			m_pIVariable = pIOldTbl;
			m_hRoot = hOldRoot;
		}
	}
}

void CLUADbg::OnElementFound(int nIdx,ScriptVarType type)
{
	char szBuf[32];
	sprintf(szBuf, "[%i]", nIdx);
	OnElementFound(szBuf, type);
}