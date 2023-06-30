// ResourceCompilerPC.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CgfConvertor.h"
#include "GC_CgfConverter.h"
#include "StatCGFCompiler\StatCGFCompiler.h"
#include "ResourceCompilerPC.h"

IRCLog* g_pLog = NULL;

void LogWarning (const char* szFormat, ...)
{
	va_list args;
	va_start (args, szFormat);
	if (g_pLog)
		g_pLog->LogV (IMiniLog::eWarning, szFormat, args);
	else
		vprintf (szFormat, args);
	va_end(args);
}
void Log (const char* szFormat, ...)
{
	va_list args;
	va_start (args, szFormat);
	if (g_pLog)
		g_pLog->LogV (IMiniLog::eMessage, szFormat, args);
	else
		vprintf (szFormat, args);
	va_end(args);
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInst = (HMODULE)hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

// This is an example of an exported function.
void __stdcall RegisterConvertors(IResourceCompiler*pRC)
{
	pRC->RegisterConvertor(new CGFConvertor());
	//pRC->RegisterConvertor(new GC_CGFConvertor());
	//pRC->RegisterConvertor(new CALConvertor());
}

HMODULE g_hInst;