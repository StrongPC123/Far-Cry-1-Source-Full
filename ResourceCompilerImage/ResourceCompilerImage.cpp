// ResourceCompilerImage.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <assert.h>
#include "ResourceCompilerImage.h"
#include "IResCompiler.h"
#include "ImageCompiler.h"							// CImageCompiler

HMODULE g_hInst;

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
void __stdcall RegisterConvertors( IResourceCompiler *pRC )
{
	CImageCompiler *pImageComp=new CImageCompiler();

	pImageComp->Init(pRC->GetEmptyWindow());

	pRC->RegisterConvertor(pImageComp);
}

