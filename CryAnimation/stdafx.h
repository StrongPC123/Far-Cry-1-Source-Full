//! Include standard headers.
#if !defined(LINUX)
#include <assert.h>
#endif


//////////////////////////////////////////////////////////////////////////
// THIS MUST BE AT THE VERY BEGINING OF STDAFX.H FILE.
// Disable STL threading support, (makes STL faster)
//////////////////////////////////////////////////////////////////////////
#define _NOTHREADS
#define _STLP_NO_THREADS
//////////////////////////////////////////////////////////////////////////

// if UNIQUE_VERT_BUFF_PER_INSTANCE defined - every instance will have unique vertex buffer (default state for now)
// it allows to do not skin for reflections and shadow maps(or stencil)
// allows to do not skin if no animation played (death body, not active weapon)
// allows to spread heavy calculations by several frames (tangent spaces calculations)
// disadvantage: system memory duplication (video memory duplication is solved by leaf buffer manager)
// can be optimized by activation/deactivation of vert buffers if character becomes visible
//
// if UNIQUE_VERT_BUFF_PER_INSTANCE not defined - only every model will use unique vertex buffer
// works much slower now, especially with reflections. shadow maps not supported now.
// buffer waiting can be optimized: cycle thru 4-8 vertex buffers (for all models)
#define UNIQUE_VERT_BUFF_PER_INSTANCE


#ifdef _XBOX

//! Include standard headers.
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <memory.h>
#include <time.h>
#include <direct.h>
#include <search.h>
#include <stdarg.h>
#if !defined(DWORD)
#if defined(LINUX64)
typedef unsigned int DWORD;
#else
typedef unsigned long DWORD;
#endif
#endif
typedef unsigned short WORD;
typedef unsigned char BYTE;

#endif

#ifdef GAMECUBE
//#include <assert.h>
//#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
//#include <string.h>
//#include <assert.h>
//#include <time.h>
#include <dolphin.h>
#include <stdarg.h>
#endif

#include <platform.h>

#include <stdlib.h>

#ifndef GAMECUBE
#	include <stdio.h>
#	if defined(LINUX)
#		include <sys/io.h>
#	else
#		include <io.h>
#	endif
#endif


// enable memory pool usage
#define USE_NEWPOOL
#include <CryMemoryManager.h>


/////////////////////////////////////////////////////////////////////////////
// STL //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <map>	
#include <set>
#include <stack>
#include <string>
#include <algorithm>




#ifdef _DEBUG
#ifdef WIN32
#include <crtdbg.h>
#define DEBUG_NEW_NORMAL_CLIENTBLOCK(file, line) new(_NORMAL_BLOCK, file, line)
#define new DEBUG_NEW_NORMAL_CLIENTBLOCK( __FILE__, __LINE__)
#define   calloc(s,t)       _calloc_dbg(s, t, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define   realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif //WIN32
#endif //_DEBUG

//#include <StlDbgAlloc.h>

#include "TArrays.h"

#ifdef WIN32
#if defined(_DEBUG) && !defined(new)
#error Incompatible TArray or cry_dbg_allocator header
#endif
#endif








typedef const char*			cstr;


#include "TArray.h"
//! Include main interfaces.
#include <Cry_Math.h>
#include <CryEngineDecalInfo.h>
#include <ISystem.h>
#include <ICryAnimation.h>
#include <IProcess.h>
#include <ITimer.h>
#include <ILog.h>
#include <IPhysics.h>
#include <IConsole.h>
#include <IRenderer.h>
#include <ICryPak.h>
#include <CrySizer.h>

#include "math.h"
#include "colordefs.h"

#if !defined(LINUX)
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
// <<TO-DO>> Get rid of this
#include <list2.h>

#include <TArray.h>
#include <smartptr.h>
#include <CryHeaders.h>
#include "CrySizer.h"

#define SIZEOF_ARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))
// sets the memory for the given vector, compatible with the STL vector by value_type, size() and &[0]
#define MEMSET_VECTOR(arr,value) memset (&((arr)[0]),value,sizeof(arr[0])*arr.size())
#define MEMZERO_VECTOR(arr) MEMSET_ARRAY(arr,0)

const double gPi = 3.1415926535897932384626433832795;

#include "CryAnimationBase.h"
//#include "CryAnimation.h"

inline double DLength( Vec3 &v ) { 
	double dx=v.x*v.x;	
	double dy=v.y*v.y;	
	double dz=v.z*v.z;	
	return sqrt( dx + dy + dz );
}

// maximum number of LODs per one geometric model (CryGeometry)
enum {g_nMaxGeomLodLevels = 3};

#define DECLARE_VECTOR_GETTER_METHODS(Type, Singular, Plural, member) \
	Type* get##Plural() {return member.empty()?NULL:&member[0];}												\
	const Type* get##Plural()const {return member.empty()?NULL:&member[0];}							\
	Type& get##Singular(unsigned i) {assert (i < num##Plural()); return member[i];}             \
	const Type& get##Singular(unsigned i)const {assert (i < num##Plural()); return member[i];}	\
	void set##Singular (unsigned i, const Type& newValue) {assert (i < num##Plural()); member[i] = newValue;} \
	unsigned num##Plural() const{return (unsigned)member.size();}

#ifdef _DEBUG

//@FIXME this function should not be inline.
_inline void __cdecl __CRYTEKDLL_TRACE(const char *sFormat, ... )
{
  va_list vl;
  static char sTraceString[1024];

  va_start(vl, sFormat);
  vsprintf(sTraceString, sFormat, vl);
  va_end(vl);

  strcat(sTraceString, "\n");

#ifdef WIN32
  ::OutputDebugString(sTraceString);	
#endif

#ifdef GAMECUBE
  OSReport(sTraceString);
#endif

}

#define TRACE __CRYTEKDLL_TRACE

#else
#define TRACE(str) ;
#endif

