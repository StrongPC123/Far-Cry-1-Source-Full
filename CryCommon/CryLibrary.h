#ifndef CRYLIBRARY_H__
#define CRYLIBRARY_H__

/*!
	CryLibrary
	
	Convenience-Macros which abstract the use of DLLs/shared libraries in a platform independent way.
	A short explanation of the different macros follows:
	
	CrySharedLibraySupported:
		This macro can be used to test if the current active platform support shared library calls. The default
		value is false. This gets redefined if a certain platform (WIN32 or LINUX) is desired.

	CrySharedLibrayExtension:
		The default extension which will get appended to library names in calls to CryLoadLibraryDefExt
		(see below).

	CryLoadLibrary(libName):
		Loads a shared library.

	CryLoadLibraryDefExt(libName):
		Loads a shared library. The platform-specific default extension is appended to the libName. This allows
		writing of somewhat platform-independent library loading code and is therefore the function which should
		be used most of the time, unless some special extensions are used (e.g. for plugins).
	
	CryGetProcAddress(libHandle, procName):
		Import function from the library presented by libHandle.
		
	CryFreeLibrary(libHandle):
		Unload the library presented by libHandle.
	
	HISTORY:
		03.03.2004 MarcoK
			- initial version
			- added to CryPlatform
*/

#include <stdio.h>

#if defined(WIN32)
	#include <windows.h>

	#define CrySharedLibraySupported true
	#define CrySharedLibrayExtension ".dll"
	#define CryLoadLibrary(libName) ::LoadLibrary(libName)
	#define CryGetProcAddress(libHandle, procName) ::GetProcAddress((HMODULE)libHandle, procName)
	#define CryFreeLibrary(libHandle) ::FreeLibrary(libHandle)
#elif defined(LINUX)
	#include <dlfcn.h>
	#include <stdlib.h>
	#include "platform.h"

	// for compatibility with code written for windows
	#define CrySharedLibraySupported true
	#define CrySharedLibrayExtension ".so"
	#define CryGetProcAddress(libHandle, procName) ::dlsym(libHandle, procName)
	#define CryFreeLibrary(libHandle) ::dlclose(libHandle)

	#define HMODULE void*
	static const char* gEnvName("MODULE_PATH");

	static const char* GetModulePath()
	{
		return getenv(gEnvName);
	}

	static void SetModulePath(const char* pModulePath)
	{
		setenv(gEnvName, pModulePath?pModulePath:"",true);
	}

	static HMODULE CryLoadLibrary(const char* libName, const bool cAppend = true, const bool cLoadLazy = false)
	{
		string newLibName(GetModulePath());
#if !defined(NDEBUG)
		string t(libName);
		string c("_debug.so");
		if(cAppend)
			t.replace(t.size()-3, c.size(), c.c_str());
		newLibName += t;
		printf("loading library  %s...\n",newLibName.c_str());
#else
		newLibName += libName;
#endif
		return ::dlopen(newLibName.c_str(), cLoadLazy?(RTLD_LAZY | RTLD_GLOBAL):(RTLD_NOW | RTLD_GLOBAL));
	}


#else
#define CrySharedLibraySupported false
#define CrySharedLibrayExtension ""
#define CryLoadLibrary(libName) NULL
#define CryLoadLibraryDefExt(libName) CryLoadLibrary(libName CrySharedLibrayExtension)
#define CryGetProcAddress(libHandle, procName) NULL
#define CryFreeLibrary(libHandle)
#endif 

#endif //CRYLIBRARY_H__
