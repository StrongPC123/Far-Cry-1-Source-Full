// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_)
#define AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_

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

#pragma warning (disable : 4554 4305 4244)
#include "platform.h"

#include "Cry_Math.h"
#include "Cry_XOptimise.h"


#ifdef GAMECUBE

//#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
//#include <string.h>

//#include <time.h>
#include <dolphin.h>
#include <stdarg.h>
#include "GCDefines.h"
#endif


#ifndef _XBOX
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#endif
#else
#include <xtl.h>
#endif
 
#include <stdlib.h>
#include <float.h>

#ifndef NO_CRY_STREAM
#include "Stream.h"
#else
class CStream {
public:
	bool WriteBits(BYTE *pBits, DWORD nSize) { return true; }
	bool ReadBits(BYTE *pBits, DWORD nSize) { return true; }
	bool Write(bool b) { return true; }
	bool Write(char c) { return true; }
	bool Write(unsigned char uc) { return true; }
	bool Write(float f) { return true; }
	bool Write(unsigned short us) { return true; }
	bool Write(short s) { return true; }
	bool Write(int i) { return true; }
	bool Write(unsigned int ui) { return true; }
	bool Write(const vectorf &v) { return true; }
	bool Read(bool &b) { return true; }
	bool Read(char &c) { return true; }
	bool Read(unsigned char &uc) { return true; }
	bool Read(unsigned short &us) { return true; }
	bool Read(short &s) { return true; }
	bool Read(int &i) { return true; }
	bool Read(unsigned int &ui) { return true; }
	bool Read(float &f) { return true; }
	bool Read(vectorf &v) { return true; }
	bool WriteNumberInBits(int n,size_t nSize) { return true; }
	bool WriteNumberInBits(unsigned int n,size_t nSize) { return true; }
	bool ReadNumberInBits(int &n,size_t nSize) { return true; }
	bool ReadNumberInBits(unsigned int &n,size_t nSize) { return true; }
	bool Seek(size_t dwPos = 0) { return true; }
	size_t GetReadPos() { return 0; }
	unsigned char *GetPtr() const { return 0; };
	size_t GetSize() const { return 0; }
	bool SetSize(size_t indwBitSize) { return true; }
};
#endif

#ifdef WIN64
#undef min
#undef max
#endif

#include <string>
#include <map>
#include "ILog.h"
#include "CrySizer.h"

// TODO: reference additional headers your program requires here
#include "primitives.h"
#include "utils.h"
#include "physinterface.h"

#ifdef _DEBUG
#ifdef WIN32
#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK new( _NORMAL_BLOCK, __FILE__, __LINE__) 
#define new DEBUG_CLIENTBLOCK
#endif //WIN32
#endif


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4AA14050_1B79_4A11_9D24_4E209BF87E2C__INCLUDED_)
