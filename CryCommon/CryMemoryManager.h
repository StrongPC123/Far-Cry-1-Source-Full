#ifndef _CRY_MEMORY_MANAGER_H_
#define _CRY_MEMORY_MANAGER_H_

#include <malloc.h>
#include <platform.h>
#include <stdlib.h>

#ifdef WIN32
	#ifdef CRYSYSTEM_EXPORTS
		#define CRYMEMORYMANAGER_API __declspec(dllexport)
	#else
		#define CRYMEMORYMANAGER_API __declspec(dllimport)
	#endif
#endif //WIN32 
#if defined(LINUX)
	#define CRYMEMORYMANAGER_API
#endif //LINUX 

#if defined(LINUX)
	#define HMODULE void*
	#include <dlfcn.h>
#endif
//! Structure filled by call to CryModuleGetMemoryInfo()
struct CryModuleMemoryInfo
{
	//! Total Ammount of memory allocated.
	uint64 allocated;
	//! Total Ammount of memory freed.
	uint64 freed;
	//! Total number of memory allocations.
	int num_allocations;
};

#if defined(LINUX)
#undef _ACCESS_POOL
#undef _SYSTEM_POOL
#define _SYSTEM_POOL(a)
#define _ACCESS_POOL
#define CryModuleMalloc malloc
#define CryModuleRealloc realloc
#define CryModuleFree free
#ifdef __cplusplus
	inline void* CryModuleReallocSize(void *ptr,size_t oldsize,size_t size)  { return CryModuleRealloc(ptr,size);}
	inline void  CryModuleFreeSize(void *ptr,size_t size) { CryModuleFree(ptr);}
	#include <new>
		inline void * __cdecl operator new   (size_t  size) throw(std::bad_alloc) { return CryModuleMalloc(size); } 
		inline void * __cdecl operator new[](size_t size) throw(std::bad_alloc) { return CryModuleMalloc(size); }; 
		inline void __cdecl operator delete  (void *p) { CryModuleFree(p); };
		inline void __cdecl operator delete[](void *p) { CryModuleFree(p); };
#else
	static void* CryModuleReallocSize(void *ptr,size_t oldsize,size_t size)  { return CryModuleRealloc(ptr,size);}
	static void  CryModuleFreeSize(void *ptr,size_t size) { CryModuleFree(ptr);}
#endif
#else

//////////////////////////////////////////////////////////////////////////
// Used by overrided new and delete operators.
//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
	// C++ methods.
	extern "C"
	{
		void* CryModuleMalloc(size_t size) throw();
		void* CryModuleRealloc(void *memblock,size_t size) throw();
		void  CryModuleFree(void *ptr) throw();
		void* CryModuleReallocSize(void *memblock,size_t oldsize,size_t size);
		void  CryModuleFreeSize(void *ptr,size_t size);
	}
	#else
		// C methods.
		extern void* CryModuleMalloc(size_t size);
		extern void* CryModuleRealloc(void *memblock,size_t size);
		extern void  CryModuleFree(void *ptr);
		extern void* CryModuleReallocSize(void *memblock,size_t oldsize,size_t size);
		extern void  CryModuleFreeSize(void *ptr,size_t size);
#endif //__cplusplus
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
	extern "C" {
#endif

#if defined(CRYSYSTEM_EXPORTS) || (!defined(WIN32) && !defined(LINUX))
	CRYMEMORYMANAGER_API void *CryMalloc(size_t size);
	CRYMEMORYMANAGER_API void *CryRealloc(void *memblock,size_t size);
	CRYMEMORYMANAGER_API void *CryReallocSize(void *memblock,size_t oldsize,size_t size);
	CRYMEMORYMANAGER_API void CryFree(void *p);
	CRYMEMORYMANAGER_API void CryFreeSize(void *p,size_t size);
	CRYMEMORYMANAGER_API int CryStats(char *buf);
	CRYMEMORYMANAGER_API void CryFlushAll();
#endif

#ifdef __cplusplus
	}
#endif 



//#ifndef CRYSYSTEM_EXPORTS

#define _ACCESS_POOL


#if defined(_DEBUG) || defined(DONT_USE_CRY_MEMORY_MANAGER)
	#define CryModuleMalloc malloc
	#define CryModuleRealloc realloc
	#define CryModuleFree free
#else
	#ifdef USE_NEWPOOL
		#define USING_CRY_MEMORY_MANAGER
	// - check this covers all prototypes
	// - way to check memory in use by old malloc?
	// issues
	// only release
	// - globals with allocs -> can make it possible but rather not
	// - calloc? also malloc -> new
//////////////////////////////////////////////////////////////////////////
// _PoolHelper definition.
//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
struct _CryMemoryManagerPoolHelper
{
	uint64 allocatedMemory;
	uint64 freedMemory;
	int numAllocations;
#if defined(WIN32) || defined(LINUX)
	HMODULE hSystem;
#endif
	typedef void *(*FNC_CryMalloc)(size_t size);
	typedef void *(*FNC_CryRealloc)(void *memblock,size_t size);
	typedef void *(*FNC_CryReallocSize)(void *memblock,size_t oldsize,size_t size);
	typedef void (*FNC_CryFree)(void *p);
	typedef void (*FNC_CryFreeSize)(void *p,size_t size);
	typedef int (*FNC_CryStats)(char *buf);
	FNC_CryMalloc _CryMalloc;
	FNC_CryRealloc _CryRealloc;
	FNC_CryReallocSize _CryReallocSize;
	FNC_CryFree _CryFree;
	FNC_CryFreeSize _CryFreeSize;

	explicit _CryMemoryManagerPoolHelper( void *pHandle = NULL )
	{
		allocatedMemory = 0;
		freedMemory = 0;
		numAllocations = 0;
#if defined(WIN32)
		if (pHandle)
			hSystem = (HMODULE)pHandle;
		else
			hSystem = LoadLibrary("CrySystem.dll"); 
#endif
#if defined(LINUX)
		if (pHandle)
			hSystem = (HMODULE)pHandle;
		else
			hSystem = ::dlopen("crysystem.so", (RTLD_NOW/*RTLD_LAZY*/ | RTLD_GLOBAL));
		if(!hSystem)
		{
			hSystem = ::dlopen("./crysystem.so", (RTLD_NOW/*RTLD_LAZY*/ | RTLD_GLOBAL));
			if(!hSystem)
			{
				//try some dlopen locations
				const char* envName = getenv("MODULE_PATH");
				if(envName)
				{
					char tmp[MAX_PATH+12];					//allocate statically
					strcpy(tmp, envName);
					strcpy(&tmp[strlen(envName)], "crysystem.so");
					tmp[strlen(envName) + strlen("crysystem.so")] = '\0';
					hSystem = ::dlopen(tmp, (RTLD_NOW/*RTLD_LAZY*/ | RTLD_GLOBAL));
				}
				if(!hSystem)
				{
					printf("Could not access crysystem.so (either working directory must the executable directory or LD_LIBRARY_PATH must contain the executable directory)\n");
					exit(1);
				}
			}
		}
#endif
		if(hSystem)
		{
#if defined(LINUX)
			_CryMalloc=(FNC_CryMalloc)::dlsym(hSystem,"CryMalloc"); 
			_CryRealloc=(FNC_CryRealloc)::dlsym(hSystem,"CryRealloc"); 
			_CryReallocSize=(FNC_CryReallocSize)::dlsym(hSystem,"CryReallocSize"); 
			_CryFree=(FNC_CryFree)::dlsym(hSystem,"CryFree"); 
			_CryFreeSize=(FNC_CryFreeSize)::dlsym(hSystem,"CryFreeSize"); 
#else
			_CryMalloc=(FNC_CryMalloc)GetProcAddress((HINSTANCE)hSystem,"CryMalloc"); 
			_CryRealloc=(FNC_CryRealloc)GetProcAddress((HINSTANCE)hSystem,"CryRealloc"); 
			_CryReallocSize=(FNC_CryReallocSize)GetProcAddress((HINSTANCE)hSystem,"CryReallocSize"); 
			_CryFree=(FNC_CryFree)GetProcAddress((HINSTANCE)hSystem,"CryFree"); 
			_CryFreeSize=(FNC_CryFreeSize)GetProcAddress((HINSTANCE)hSystem,"CryFreeSize"); 
#endif
		};
		// Not need system anymore.
#if defined(LINUX)
			if(!_CryMalloc)
				printf("Could not read symbol: CryMalloc from crysystem.so\n");
			if(!_CryRealloc)
				printf("Could not read symbol: CryRealloc from crysystem.so\n");
			if(!_CryReallocSize)
				printf("Could not read symbol: CryReallocSize from crysystem.so\n");
			if(!_CryFree)
				printf("Could not read symbol: CryFree from crysystem.so\n");
			if(!_CryMalloc)
				printf("Could not read symbol: CryFreeSize from crysystem.so\n");
			if(!_CryMalloc || !_CryRealloc || !_CryReallocSize || !_CryFree || !_CryFreeSize)
				exit(1);
#else
		if(!hSystem || !_CryMalloc || !_CryRealloc || !_CryReallocSize || !_CryFree || !_CryFreeSize)
		{
			MessageBox(NULL, "Could not access CrySystem.dll (check working directory)", "Memory Manager", MB_OK);
			if (hSystem)
				::FreeLibrary( hSystem );
			exit(1);
		};
		if (hSystem)
			::FreeLibrary( hSystem );
#endif
	}
	~_CryMemoryManagerPoolHelper()
	{
#if defined(LINUX)
		if (hSystem)
			::dlclose( hSystem );
#endif
	}
	void GetMemoryInfo( CryModuleMemoryInfo *pMmemInfo )
	{
		pMmemInfo->allocated = allocatedMemory;
		pMmemInfo->freed = freedMemory;
		pMmemInfo->num_allocations = numAllocations;
	}
	//////////////////////////////////////////////////////////////////////////
	// Local version of allocations, does memory counting per module.
	//////////////////////////////////////////////////////////////////////////
	__forceinline void*	Malloc(size_t size)
	{
		allocatedMemory += size;
		numAllocations++;
		return _CryMalloc( size );
	}
	//////////////////////////////////////////////////////////////////////////
	__forceinline void*	Realloc(void *memblock,size_t size)
	{
		if (memblock == NULL)
		{
			allocatedMemory += size;
			numAllocations++;
		}
		else
		{
			numAllocations++;
			size_t oldsize = ((int*)memblock)[-1];
			allocatedMemory += size;
			freedMemory += oldsize;
		}
		return _CryRealloc( memblock,size );
	}
	//////////////////////////////////////////////////////////////////////////
	__forceinline void Free( void *memblock )
	{
		if (memblock != 0)
		{
			size_t size = ((int*)memblock)[-1];
			freedMemory += size;
		}
		_CryFree( memblock );
	}
	//////////////////////////////////////////////////////////////////////////
	__forceinline void* ReallocSize(void *memblock,size_t oldsize,size_t size)
	{
		allocatedMemory += size;
		freedMemory += oldsize;
		numAllocations++;
		return _CryReallocSize( memblock,oldsize,size );
	}
	//////////////////////////////////////////////////////////////////////////
	__forceinline void FreeSize( void *memblock,size_t size )
	{
		freedMemory += size;
		_CryFreeSize( memblock,size );
	}
};
#endif //__cplusplus

#ifdef _USRDLL
#define CRY_MEM_USAGE_API extern "C" __declspec(dllexport)
#else
#define CRY_MEM_USAGE_API
#endif


#ifndef _XBOX
#undef _ACCESS_POOL
#define _ACCESS_POOL \
  _CryMemoryManagerPoolHelper g_CryMemoryManagerHelper;\
	void* CryModuleMalloc( size_t size ) throw(){ return g_CryMemoryManagerHelper.Malloc(size); };\
	void* CryModuleRealloc( void *ptr,size_t size )  throw(){ return g_CryMemoryManagerHelper.Realloc(ptr,size); };\
	void  CryModuleFree( void *ptr ) throw() { g_CryMemoryManagerHelper.Free(ptr); };\
	void* CryModuleReallocSize(void *ptr,size_t oldsize,size_t size)  { return g_CryMemoryManagerHelper.ReallocSize(ptr,oldsize,size); };\
	void  CryModuleFreeSize(void *ptr,size_t size) { g_CryMemoryManagerHelper.FreeSize(ptr,size); };\
	CRY_MEM_USAGE_API void CryModuleGetMemoryInfo( CryModuleMemoryInfo *pMemInfo ) { g_CryMemoryManagerHelper.GetMemoryInfo(pMemInfo); };

// To be created inside CrySystem.
#define _SYSTEM_POOL( hSystemHandle ) \
	_CryMemoryManagerPoolHelper g_CryMemoryManagerHelper( hSystemHandle );\
	void* CryModuleMalloc( size_t size ) throw() {return g_CryMemoryManagerHelper.Malloc(size); };\
	void* CryModuleRealloc( void *ptr,size_t size ) throw(){ return g_CryMemoryManagerHelper.Realloc(ptr,size); };\
	void  CryModuleFree( void *ptr ) { g_CryMemoryManagerHelper.Free(ptr); };\
	void* CryModuleReallocSize(void *ptr,size_t oldsize,size_t size)  { return g_CryMemoryManagerHelper.ReallocSize(ptr,oldsize,size); };\
	void  CryModuleFreeSize(void *ptr,size_t size) { g_CryMemoryManagerHelper.FreeSize(ptr,size); };\
	CRY_MEM_USAGE_API void CryModuleGetMemoryInfo( CryModuleMemoryInfo *pMemInfo ) { g_CryMemoryManagerHelper.GetMemoryInfo(pMemInfo); };
#endif

#ifdef __cplusplus
#include <new.h>
#endif


#undef malloc
#undef realloc
#undef free


/*
inline void *malloc(unsigned int size) { return _CryMalloc(size); };
inline void *realloc(void *block, unsigned int size) { return _CryRealloc(block, size); };
inline void free(void *p) { _CryFree(p); };
*/

#define malloc				CryModuleMalloc
#define realloc				CryModuleRealloc
#define free				CryModuleFree
#define realloc_size		CryModuleReallocSize
#define free_size			CryModuleFreeSize

#ifdef __cplusplus
	#ifndef GAMECUBE //I don't know how to compile this on GC
		inline void * __cdecl operator new   (size_t  size) { return CryModuleMalloc(size); } 
		inline void * __cdecl operator new[](size_t size) { return CryModuleMalloc(size); }; 
		inline void __cdecl operator delete  (void *p) { CryModuleFree(p); };
		inline void __cdecl operator delete[](void *p) { CryModuleFree(p); };
	#endif //GAMECUBE
#endif //__cplusplus

#endif // USE_NEWPOOL

#endif // _DEBUG

//#endif // CRYSYSTEM_EXPORTS
#endif //LINUX
#endif //_CRY_MEMORY_MANAGER_H_
