/*=============================================================================
	RenderPC.cpp: Cry Render support precompiled header generator.
	Copyright 2001 Crytek Studios. All Rights Reserved.

	Revision history:
		* Created by Honitch Andrey

=============================================================================*/

#define CRY_API

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#endif //_DEBUG

//! Include standart headers.
#include <assert.h>

//#define PS2
//#define OPENGL


#ifdef _XBOX

//! Include standart headers.
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <io.h>
#include <memory.h>
#include <time.h>
#include <direct.h>
#include <search.h>
#include <stdarg.h>

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

#include <xtl.h>

#else

#include <windows.h>

#endif

// enable memory pool usage
#define USE_NEWPOOL
#include <CryMemoryManager.h>

#include "CrtOverrides.h"

#if defined _DEBUG && defined OPENGL
#define DEBUGALLOC
#endif

/////////////////////////////////////////////////////////////////////////////
// STL //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <list>
#include <map>	
#include <hash_map>	
#include <set>
#include <string>
#include <algorithm>

typedef const char*			cstr;

#define SIZEOF_ARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))

// Include common headers.
//#include "Common\CryHelpers.h"

typedef string String;

#ifdef DEBUGALLOC

#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, __FILE__, __LINE__) 
#define new DEBUG_CLIENTBLOCK

// memman
#define   calloc(s,t)       _calloc_dbg(s, t, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)

#endif 


#include <list2.h>
#include <Names.h>

#define	MAX_TMU 8

//! Include main interfaces.
#include <ICryPak.h>
#include <IEntitySystem.h>
#include <IProcess.h>
#include <ITimer.h>
#include <ISystem.h>
#include <ILog.h>
#include <IPhysics.h>
#include <IConsole.h>
#include <IRenderer.h>
#include <IStreamEngine.h>
#include <CrySizer.h>

#include "Font.h"
#include "Except.h"

#include <Cry_Math.h>
#include "Cry_Camera.h"
//#include "_Malloc.h"
#include "math.h"

#include <VertexFormats.h>
#include <CREPolyMesh.h>



#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))


#define MAX_PATH_LENGTH	512

