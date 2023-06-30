#include "stdafx.h"
#include "LuaDbgInterface.h"
#include "LUADBG.h"

bool g_bDone = false;

bool InvokeDebugger(CLUADbg *pDebugger, const char *pszSourceFile, int iLine, const char *pszReason)
{
	HACCEL hAccelerators = NULL;
	MSG msg;
	IScriptSystem *pIScriptSystem = NULL;

	if (pDebugger == NULL)
		return false;

	// TODO: Would be better to handle this with a console variable since this
	//       would give the user a chance to reactivate it. Or maybe using the 
	//       m_bsBreakState of the scripting sytem, but I'm not sure how to add it there
	if (pDebugger->IsUserDisabled())
		return true;

	pIScriptSystem = pDebugger->GetScriptSystem();

	// Debugger not inititalized
	if (pDebugger == NULL)
		return false;
	if (!::IsWindow(pDebugger->m_hWnd))
		return false;

	if ((hAccelerators = LoadAccelerators(_Tiny_GetResourceInstance(), MAKEINTRESOURCE(IDR_LUADGB_ACCEL))) == NULL)
	{
		// No accelerators
	}

	// Make sure the debugger is displayed maximized when it was left like that last time
	// TODO: Maybe serialize this with the other window settings
	if (::IsZoomed(pDebugger->m_hWnd))
		::ShowWindow(pDebugger->m_hWnd, SW_MAXIMIZE);
	else
		::ShowWindow(pDebugger->m_hWnd, SW_NORMAL);
	::SetForegroundWindow(pDebugger->m_hWnd);

	if (pszSourceFile && pszSourceFile[0] == '@')
	{
		pDebugger->LoadFile(&pszSourceFile[1]);
		iLine = __max(0, iLine);
		pDebugger->PlaceLineMarker(iLine);
	}

	if (pszReason)
		pDebugger->SetStatusBarText(pszReason);

	pDebugger->GetStackAndLocals();

	g_bDone = false;
 	while (GetMessage(&msg, NULL, 0, 0) && !g_bDone) 
	{
		if (hAccelerators == NULL || TranslateAccelerator(pDebugger->m_hWnd, hAccelerators, &msg) == 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (!IsWindow(pDebugger->m_hWnd))
		DebugBreak();

	// Don't hide the window when the debugger will be triggered next frame anyway
	if (pIScriptSystem->GetBreakState() != bsStepNext &&
		pIScriptSystem->GetBreakState() != bsStepInto)
	{
		::ShowWindow(pDebugger->m_hWnd, SW_HIDE);
	}

	DestroyAcceleratorTable(hAccelerators);

	return (int) msg.wParam == 0;

	return true;
}