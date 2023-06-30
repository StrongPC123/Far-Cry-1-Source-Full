////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2004.
// -------------------------------------------------------------------------
//  File name:   Linux32Specific.h
//  Version:     v1.00
//  Created:     05/03/2004 by MarcoK.
//  Compilers:   Visual Studio.NET, GCC 3.2
//  Description: Specific to Linux declarations, inline functions etc.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef _CRY_COMMON_LINUX_SPECIFIC_HDR_
#define _CRY_COMMON_LINUX_SPECIFIC_HDR_

#include <pthread.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include </usr/include/ctype.h>

typedef unsigned int				DWORD;
typedef unsigned int*				LPDWORD;
typedef void*								LPVOID;
#define VOID            		void
#define PVOID								void*

#define PHYSICS_EXPORTS

#ifdef __cplusplus
// checks if the heap is valid in debug; in release, this function shouldn't be called
// returns non-0 if it's valid and 0 if not valid
inline int IsHeapValid ()
{
	return true;
}
#endif //__cplusplus

// MSVC compiler-specific keywords
#define __forceinline inline
#define _inline inline
#define __cdecl
#define __stdcall
#define __fastcall
#define IN
#define OUT

// Safe memory freeing
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);		(p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);		(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release();	(p)=NULL; } }
#endif

#ifndef SAFE_RELEASE_FORCE
#define SAFE_RELEASE_FORCE(p)			{ if(p) { (p)->Release(1);	(p)=NULL; } }
#endif

#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))

#define CALLBACK
#define WINAPI

#ifndef __cplusplus
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define TCHAR wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif
typedef wchar_t WCHAR;    // wc,   16-bit UNICODE character
typedef WCHAR *PWCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef const WCHAR *LPCWCH, *PCWCH;
typedef WCHAR *NWPSTR;
typedef WCHAR *LPWSTR, *PWSTR;
typedef WCHAR *LPUWSTR, *PUWSTR;

typedef const WCHAR *LPCWSTR, *PCWSTR;
typedef const WCHAR *LPCUWSTR, *PCUWSTR;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
            ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
            ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#define FILE_ATTRIBUTE_NORMAL               0x00000080

typedef int							BOOL;
typedef int							LONG;
typedef unsigned int 		ULONG;
typedef int 						HRESULT;


#define TRUE 1
#define FALSE 0

#ifndef MAX_PATH
	#define MAX_PATH 256
#endif
#ifndef _MAX_PATH
#define _MAX_PATH MAX_PATH
#endif


//-------------------------------------socket stuff------------------------------------------
#define SOCKET int
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

typedef struct in_addr_windows 
{
	union 
	{
		struct { unsigned char s_b1,s_b2,s_b3,s_b4; } S_un_b;
		struct { unsigned short s_w1,s_w2; } S_un_w;
		unsigned int S_addr;
	} S_un;
}in_addr_windows;

#define WSAEINTR EINTR
#define WSAEBADF EBADF
#define WSAEACCES EACCES
#define WSAEFAULT EFAULT
#define WSAEACCES EACCES
#define WSAEFAULT EFAULT
#define WSAEINVAL EINVAL
#define WSAEMFILE EMFILE
#define WSAEWOULDBLOCK EAGAIN
#define WSAEINPROGRESS EINPROGRESS
#define WSAEALREADY EALREADY
#define WSAENOTSOCK ENOTSOCK 
#define WSAEDESTADDRREQ EDESTADDRREQ
#define WSAEMSGSIZE EMSGSIZE
#define WSAEPROTOTYPE EPROTOTYPE
#define WSAENOPROTOOPT ENOPROTOOPT
#define WSAEPROTONOSUPPORT EPROTONOSUPPORT
#define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT
#define WSAEOPNOTSUPP EOPNOTSUPP
#define WSAEPFNOSUPPORT EPFNOSUPPORT
#define WSAEAFNOSUPPORT EAFNOSUPPORT
#define WSAEADDRINUSE EADDRINUSE
#define WSAEADDRNOTAVAIL EADDRNOTAVAIL
#define WSAENETDOWN ENETDOWN
#define WSAENETUNREACH ENETUNREACH
#define WSAENETRESET ENETRESET
#define WSAECONNABORTED ECONNABORTED
#define WSAECONNRESET ECONNRESET
#define WSAENOBUFS ENOBUFS
#define WSAEISCONN EISCONN
#define WSAENOTCONN ENOTCONN
#define WSAESHUTDOWN ESHUTDOWN
#define WSAETOOMANYREFS ETOOMANYREFS
#define WSAETIMEDOUT ETIMEDOUT
#define WSAECONNREFUSED ECONNREFUSED
#define WSAELOOP ELOOP
#define WSAENAMETOOLONG ENAMETOOLONG
#define WSAEHOSTDOWN EHOSTDOWN
#define WSAEHOSTUNREACH EHOSTUNREACH
#define WSAENOTEMPTY ENOTEMPTY
#define WSAEPROCLIM EPROCLIM
#define WSAEUSERS EUSERS
#define WSAEDQUOT EDQUOT
#define WSAESTALE ESTALE
#define WSAEREMOTE EREMOTE

//-------------------------------------end socket stuff------------------------------------------

#define __TIMESTAMP__ __DATE__" "__TIME__

// function renaming
#define _finite __finite
#define _snprintf snprintf
#define _isnan isnan
#define stricmp strcasecmp
#define _stricmp strcasecmp
#define strnicmp strncasecmp

#define _vsnprintf vsnprintf
#define _wtof( str ) wcstod( str, 0 )

/*static unsigned char toupper(unsigned char c)
{
  return c & ~0x40;
}
*/
typedef union _LARGE_INTEGER
{
  struct
	{
      DWORD LowPart;
      LONG HighPart;
  };
  struct
	{
      DWORD LowPart;
      LONG HighPart;
  } u;
  long long QuadPart;
} LARGE_INTEGER;


// stdlib.h stuff
#define _MAX_DRIVE  3   // max. length of drive component
#define _MAX_DIR    256 // max. length of path component
#define _MAX_FNAME  256 // max. length of file name component
#define _MAX_EXT    256 // max. length of extension component

// fcntl.h
#define _O_RDONLY       0x0000  /* open for reading only */
#define _O_WRONLY       0x0001  /* open for writing only */
#define _O_RDWR         0x0002  /* open for reading and writing */
#define _O_APPEND       0x0008  /* writes done at eof */
#define _O_CREAT        0x0100  /* create and open file */
#define _O_TRUNC        0x0200  /* open and truncate */
#define _O_EXCL         0x0400  /* open only if file doesn't already exist */
#define _O_TEXT         0x4000  /* file mode is text (translated) */
#define _O_BINARY       0x8000  /* file mode is binary (untranslated) */
#define _O_RAW  _O_BINARY
#define _O_NOINHERIT    0x0080  /* child process doesn't inherit file */
#define _O_TEMPORARY    0x0040  /* temporary file bit */
#define _O_SHORT_LIVED  0x1000  /* temporary storage file, try not to flush */
#define _O_SEQUENTIAL   0x0020  /* file access is primarily sequential */
#define _O_RANDOM       0x0010  /* file access is primarily random */


// io.h stuff
typedef unsigned int _fsize_t;

struct _OVERLAPPED;

typedef void (*LPOVERLAPPED_COMPLETION_ROUTINE)(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, struct _OVERLAPPED *lpOverlapped);

typedef struct _OVERLAPPED
{
    void* pCaller;//this is orginally reserved for internal purpose, we store the Caller pointer here
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine; ////this is orginally ULONG_PTR InternalHigh and reserved for internal purpose
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        };
        PVOID Pointer;
    };
		DWORD dwNumberOfBytesTransfered;	//additional member temporary speciying the number of bytes to be read
    /*HANDLE*/void*  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef struct _SECURITY_ATTRIBUTES 
{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

#ifdef __cplusplus
	static pthread_mutex_t mutex_t;
	template<typename T>
	const volatile T InterlockedIncrement(volatile T* pT)
	{
		pthread_mutex_lock(&mutex_t);
		++(*pT);
		pthread_mutex_unlock(&mutex_t);
		return *pT;
	}

	template<typename T>
	const volatile T InterlockedDecrement(volatile T* pT)
	{
		pthread_mutex_lock(&mutex_t);
		--(*pT);
		pthread_mutex_unlock(&mutex_t);
		return *pT;
	}

	template<typename S, typename T>
	inline const S& min(const S& rS, const T& rT)
	{
		return (rS <= rT)? rS : rT;
	}

	template<typename S, typename T>
	inline const S& max(const S& rS, const T& rT)
	{
		return (rS >= rT)? rS : rT;
	}

	template<typename S, typename T>
	inline const S& __min(const S& rS, const T& rT)
	{
		return min(rS, rT);
	}

	template<typename S, typename T>
	inline const S& __max(const S& rS, const T& rT)
	{
		return max(rS, rT);
	}


	typedef enum {INVALID_HANDLE_VALUE = -1l}INVALID_HANDLE_VALUE_ENUM;
	//for compatibility reason we got to create a class which actually contains an int rather than a void* and make sure it does not get mistreated
	template <class T, T U>//U is default type for invalid handle value, T the encapsulated handle type to be used instead of void* (as under windows and never linux)
	class CHandle
	{
	public:
		typedef T			HandleType;
		typedef void* PointerType;	//for compatibility reason to encapsulate a void* as an int

		static const HandleType sciInvalidHandleValue = U;

		CHandle(const CHandle<T,U>& cHandle) : m_Value(cHandle.m_Value){}
		CHandle(const HandleType cHandle = U) : m_Value(cHandle){}
		CHandle(const PointerType cpHandle) : m_Value(reinterpret_cast<HandleType>(cpHandle)){}
		CHandle(INVALID_HANDLE_VALUE_ENUM) : m_Value(U){}//to be able to use a common value for all InvalidHandle - types
#if defined(LINUX64)
		//treat __null tyope also as invalid handle type
		CHandle(typeof(__null)) : m_Value(U){}//to be able to use a common value for all InvalidHandle - types
#endif
		operator HandleType(){return m_Value;}
		bool operator!() const{return m_Value == sciInvalidHandleValue;}
		const CHandle& operator =(const CHandle& crHandle){m_Value = crHandle.m_Value;return *this;}
		const CHandle& operator =(const PointerType cpHandle){m_Value = reinterpret_cast<HandleType>(cpHandle);return *this;}
		const bool operator ==(const CHandle& crHandle)		const{return m_Value == crHandle.m_Value;}
		const bool operator ==(const HandleType cHandle)	const{return m_Value == cHandle;}
		const bool operator ==(const PointerType cpHandle)const{return m_Value == reinterpret_cast<HandleType>(cpHandle);}
		const bool operator !=(const HandleType cHandle)	const{return m_Value != cHandle;}
		const bool operator !=(const CHandle& crHandle)		const{return m_Value != crHandle.m_Value;}
		const bool operator !=(const PointerType cpHandle)const{return m_Value != reinterpret_cast<HandleType>(cpHandle);}
		const bool operator <	(const CHandle& crHandle)		const{return m_Value < crHandle.m_Value;}
		HandleType Handle()const{return m_Value;}

	private:
		HandleType m_Value;	//the actual value, remember that file descriptors are ints under linux

		typedef void	ReferenceType;//for compatibility reason to encapsulate a void* as an int
		//forbid these function which would actually not work on an int
		PointerType operator->();
    PointerType operator->() const;
    ReferenceType operator*();
    ReferenceType operator*() const;
		operator PointerType();
	};

	typedef CHandle<int, (int)-1l> HANDLE;

#endif //__cplusplus

#endif //_CRY_COMMON_LINUX_SPECIFIC_HDR_
