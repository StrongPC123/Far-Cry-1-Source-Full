// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E24__INCLUDED_)
#define AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E24__INCLUDED_


//#define _CRTDBG_MAP_ALLOC

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////////
// THIS MUST BE AT THE VERY BEGINING OF STDAFX.H FILE.
// Disable STL threading support, (makes STL faster)
//////////////////////////////////////////////////////////////////////////
#define _NOTHREADS
#define _STLP_NO_THREADS
//////////////////////////////////////////////////////////////////////////


#ifdef _XBOX
// '<name>' name was marked as #pragma deprecated
//#pragma warning (disable : 4995)
#endif

// Insert your headers here
#include "platform.h"


#define REDUCED_FOR_PUBLIC_RELEASE				// remark this if you want to get more network stats (but keep it out of every public build)


//#define NET_PACKET_LOGGING				// pure netcode debug purpose - very slow networking - writes files to c:/temp
 

#ifdef GetClassName
#undef GetClassName
#endif

#include <stdlib.h>
#include <stdio.h>

#if defined(LINUX)
#	include <stdarg.h>
#	include "platform.h"
#	include "IGame.h"
#	include "string.h"
#endif

#if defined(_AMD64_) && !defined(LINUX)
#include <io.h>
#endif

#define USE_NEWPOOL
#include <CryMemoryManager.h>


/////////////////////////////////////////////////////////////////////////////
// VARIOUS MACROS ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

#ifndef PS2
#if defined(WIN64) || defined(LINUX)
#define FIXME_ASSERT(cond) { if(!(cond)) abort(); }
#else
#define FIXME_ASSERT(cond) { if(!(cond)) { DEBUG_BREAK; } }
#endif
#else //PS2
#define FIXME_ASSERT(cond) { if(!(cond)) { FORCE_EXIT();} }
#endif

template <class T> inline void ZeroStruct( T &t ) { memset( &t,0,sizeof(t) ); }

#ifdef PS2
inline void __CRYTEKDLL_TRACE(const char *sFormat, ... )
#else
_inline void __cdecl __CRYTEKDLL_TRACE(const char *sFormat, ... )
#endif
{
	va_list vl;
	static char sTraceString[1024];
	
	va_start(vl, sFormat);
	vsprintf(sTraceString, sFormat, vl);
	va_end(vl);

	strcat(sTraceString, "\n");

	::OutputDebugString(sTraceString);	
}

#if 1

/*
#ifndef _XBOX
#ifdef WIN32
#include <windows.h>
#include <stdarg.h>
#endif

#else
#include <xtl.h>
#endif
*/
//Timur[9/3/2001] #include <windows.h>

/*
#define ASSERT(x)																									\
{																													\
    if (!(x)) {																										\
	static char sAssertionMessage[1024];																				\
	\
	sprintf(sAssertionMessage, "Assertion Failed!!\n\nFile: \"%s\"\n\nLine: %d\n", __FILE__, __LINE__);	\
	::MessageBox(NULL, sAssertionMessage, "Assertion Failed", MB_OK | MB_ICONERROR);						\
	DEBUG_BREAK;																								\
    }																												\
}   */

//#define ENABLE_NET_TRACE					// enable for network debugging

//@FIXME this function should not be inline.

#define TRACE __CRYTEKDLL_TRACE

#ifdef ENABLE_NET_TRACE
#define NET_TRACE __CRYTEKDLL_TRACE
#else
#	if defined(LINUX)
#		define NET_TRACE //
#	else
#		define NET_TRACE __noop
#	endif
#endif

#ifdef ASSERT
#undef ASSERT
#endif

#if defined(WIN64) || defined(LINUX64)
#define ASSERT(x) {assert(x);}
#else
#define ASSERT(x)	{ if (!(x)) { TRACE("Assertion Failed (%s) File: \"%s\" Line: %d\n", #x, __FILE__, __LINE__); DEBUG_BREAK; } }
#endif


#else

#define ASSERT(x)

#if defined(LINUX)
#undef assert
#define assert(exp) (void)( (exp) || (printf("Assert: ' %s ' has failed\n", #exp), 0) )
#undef ASSERT
#define ASSERT(exp) (void)( (exp) || (printf("Assert: ' %s ' has failed\n", #exp), 0) )
#endif

#ifndef PS2
#define TRACE 1?(void)0 : __CRYTEKDLL_TRACE;
#else
#define TRACE __CRYTEKDLL_TRACE;
#endif

#endif


#ifdef _DEBUG
#define _VERIFY(x) ASSERT(x)
#else
#define _VERIFY(x) x
#endif


/////////////////////////////////////////////////////////////////////////////
// STL //////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#pragma warning (disable : 4786)
#include <vector>
#include <list>
#include <map>	
#include <set>
#include <string>
#include <algorithm>
#include <memory>


#if defined(_DEBUG) && !defined(LINUX)
#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, __FILE__, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);		(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release();	(p)=NULL; } }
#endif



/////////////////////////////////////////////////////////////////////////////
// CRY Stuff ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <Cry_Math.h>
#include <Cry_Camera.h>


/////////////////////////////////////////////////////////////////////////////
// Interfaces ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include <ISystem.h>
#include <INetwork.h>
#include <IConsole.h>
#include <ILog.h>
#include <IInput.h>
#include <ITimer.h>
#include <IProcess.h>
//#include <I3DEngine.h>
#include <IEntitySystem.h>
#include <IPhysics.h>
#include <IRenderer.h>
#include "EntityDesc.h"
/////////////////////////////////////////////////////////////////////////////
// Classe used often/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
#include "IXSystem.h"
#include "GameShared.h"
/////////////////////////////////////////////////////////////////////////////
#include "Game.h"
#include "XNetwork.h"
#include "XServer.h"

#include "XServerSlot.h"
#include "XClient.h"

//////////////////////////////////////////////////////////////////////////
//! Reports a Game Warning to validator with WARNING severity.
inline void GameWarning( const char *format,... )
{
	if (!format)
		return;

	char buffer[MAX_WARNING_LENGTH];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);
	CryWarning( VALIDATOR_MODULE_GAME,VALIDATOR_WARNING,buffer );
}
//////////////////////////////////////////////////////////////////////////

inline void __DumpEntity(ILog *pLog,IEntity *pEntity)
{
	const char *sTemp=NULL;
	Vec3 v;
	pLog->Log("************************************");
	if(!pEntity)
	{
		pLog->Log("DUMPING ENTITY .... the pEntity IS NULL");
		return;
	}
	pLog->Log("DUMPING ENTITY %d",pEntity->GetId());
	pLog->Log("CLASSID [%03d]",(int)pEntity->GetClassId());
	sTemp=pEntity->GetEntityClassName();
  pLog->Log("GetClassName return [%p]",sTemp);
	pLog->Log("CLASSNAME %s",sTemp);
	sTemp=pEntity->GetName();
	pLog->Log("GetName return [%p]",sTemp);
	pLog->Log("NAME %s",sTemp);
	v=pEntity->GetPos();
	pLog->Log("POS %f,%f,%f",v.x,v.y,v.z);
	pLog->Log("CONTAINER (ptr)[%p]",pEntity->GetContainer());
	pLog->Log("************************************");
}

inline void __DumpEntity(ILog *pLog,const CEntityDesc &desc)
{
	const char *sTemp=NULL;
	Vec3 v;
	pLog->Log("*************ENTITYDESCDUMP****************");
	pLog->Log("CLASSID [%03d]",(int)desc.ClassId);
	pLog->Log("CLASSNAME %s",desc.className.c_str());
	v=desc.pos;
	pLog->Log("POS %f,%f,%f",v.x,v.y,v.z);
	pLog->Log("************************************");
}


//
//claps angl to be within min-max range. Check fro special case when min>max -- for example min=350 max=40
inline float	ClampAngle360( float min, float max, float angl )
{
	if(min>max)
	{
		if( angl>min || angl<max )
			return angl;
		if( fabs( angl-min )<fabs( angl-max ) )
			angl = min;
		else
			angl = max;
		return angl;
	}

	if( angl<min ) 
		angl = min;
	else if( angl>max )
		angl = max;
	return angl;
}


//
//gets angles difference, checks for case when a1 and a2 on different sides of 0. for example a1=350 a2=10
inline float	GetAngleDifference360( float a1, float a2 )
{
float	res = a1-a2;

		if(res>180)
			res = res-360;
		else if(res<-180)
			res = 360 + res;
		return res;
}

//
// claps angl to be within min-max range. Check fro special case when min>max -- for example min=350 max=40
// all angles have to be in range (0, 360)
inline float	ClampAngle( float minA, float maxA, float angl, bool &bClamped )
{
	bClamped = false;
	if(minA>maxA)
	{
		if( angl>minA || angl<maxA )
			return angl;
	}
	else
	 if( angl>minA && angl<maxA )
		return angl;

	bClamped = true;

	if( fabs(GetAngleDifference360(minA, angl)) < fabs(GetAngleDifference360(maxA, angl)) )
		return minA;
	return maxA;
}

//
// claps angl to be within min-max range. Check fro special case when min>max -- for example min=350 max=40
// all angles have to be in range (0, 360)
inline float	ClampAngle( float minA, float maxA, float angl)
{
	if(minA>maxA)
	{
		if( angl>minA || angl<maxA )
			return angl;
	}
	else
	 if( angl>minA && angl<maxA )
		return angl;

	if( fabs(GetAngleDifference360(minA, angl)) < fabs(GetAngleDifference360(maxA, angl)) )
		return minA;
	return maxA;
}

#if !defined(min) && !defined(LINUX)
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif






//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__B36C365D_F0EA_4545_B3BC_1E0EAB3B5E24__INCLUDED_)
