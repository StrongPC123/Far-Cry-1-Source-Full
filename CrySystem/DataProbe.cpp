////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   DataProbe.cpp
//  Version:     v1.00
//  Created:     19/1/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: This is authentication functions, this code is not for public release!!
// -------------------------------------------------------------------------
//  History:
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Timur.
// This is authentication functions, this code is not for public release!!
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#if defined(LINUX)
	#include <CryLibrary.h>
#endif

#include "DataProbe.h"
#include "zlib\zlib.h"
#include "ISystem.h"
#include "CryFile.h"

#include "md5.h"
//#define FARCRY_EXE_CRC_CHECK
#if !defined(LINUX)
	extern HMODULE gDLLHandle;
#endif
// embedd searchable string.
//#pragma comment( exestr, "CScriptObjectEntity:...." )
#define TEST_NAME_END 28
const char test_name[] = "Cannot find attached entity.\0....";

// Check Up to 1 Mb of data.
#define MAX_PROBE_SIZE uint64(1000000)

//#define XOR_VALUE_64 0xB1A72C931C38E5F2

#define ADLER_32_BASE 65521L /* largest prime smaller than 65536 */
#define ADLER_32_NMAX 5552
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */

#define DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);

#define ADLER_32		SpecialFunc1
#define CRC_32			SpecialFunc2
#define CRC_64			SpecialFunc3


/* ========================================================================= */
static unsigned int ADLER_32( unsigned int adler, const char *buf, unsigned int len)
{
	unsigned int s1 = adler & 0xFFFF;
	unsigned int s2 = (adler >> 16) & 0xFFFF;
	int k;

	if (!buf)
		return 1L;

	while (len > 0) {
		k = len < ADLER_32_NMAX ? len : ADLER_32_NMAX;
		len -= k;
		while (k >= 16)
		{
			DO16(buf);
			buf += 16;
			k -= 16;
		}
		if (k != 0) do {
			s1 += *buf++;
			s2 += s1;
		} while (--k);
		s1 %= ADLER_32_BASE;
		s2 %= ADLER_32_BASE;
	}
	return (s2 << 16) | s1;
}

#define AUTODIN_IIREV	0xEDB88320
#define POLY64REV	0xd800000000000000ULL

//////////////////////////////////////////////////////////////////////////
// Calculate CRC32 of buffer.
//////////////////////////////////////////////////////////////////////////
static unsigned int CRC_32( const char *buf,unsigned int len )
{
	static unsigned int Table[256];
	static int init = 0;
	unsigned int code = 0xFFFFFFFF;  

	if (!init) {
		int i;
		init = 1;
		for (i = 0; i <= 255; i++) {
			int j;
			unsigned int part = i;
			for (j = 0; j < 8; j++) {
				if (part & 1)
					part = (part >> 1) ^ AUTODIN_IIREV;
				else
					part >>= 1;
			}
			Table[i] = part;
		}
	}
	while (len--)
	{
		unsigned int temp1 = code >> 8;
		unsigned int temp2 = Table[(code ^ (unsigned int )*buf) & 0xff];
		code = temp1 ^ temp2;
		buf += 1;
	}
	return code;
}

//////////////////////////////////////////////////////////////////////////
// Calculate CRC64 of buffer.
//////////////////////////////////////////////////////////////////////////
static uint64 CRC_64( const char *buf,unsigned int len )
{
	static uint64 Table[256];
	uint64 code = 0;
	static int init = 0;

	if (!init) {
		int i;
		init = 1;
		for (i = 0; i <= 255; i++) {
			int j;
			uint64 part = i;
			for (j = 0; j < 8; j++) {
				if (part & 1)
					part = (part >> 1) ^ POLY64REV;
				else
					part >>= 1;
			}
			Table[i] = part;
		}
	}
	while (len--)
	{
		uint64 temp1 = code >> 8;
		uint64 temp2 = Table[(code ^ (uint64)*buf) & 0xff];
		code = temp1 ^ temp2;
		buf += 1;
	}
	return code;
}

/*
void encipher(unsigned int *src,unsigned int *trg,int len,unsigned int *key )
{
unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; 
register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3];
while (nlen--) {
register unsigned int y=v[0],z=v[1],n=32,sum=0; 
while(n-->0) { sum += delta; y += (z << 4)+a ^ z+sum ^ (z >> 5)+b; z += (y << 4)+c ^ y+sum ^ (y >> 5)+d; } 
w[0]=y; w[1]=z; 
v+=2,w+=2;
}
}

void decipher(unsigned int *src,unsigned int *trg,int len,unsigned int *key )
{
unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; 
register unsigned delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; 
while (nlen--) { 
register unsigned int y=v[0],z=v[1],sum=0xC6EF3720,n=32;
while(n-->0) { z -= (y << 4)+c ^ y+sum ^ (y >> 5)+d; y -= (z << 4)+a ^ z+sum ^ (z >> 5)+b; sum -= delta; } 
w[0]=y; w[1]=z;
v+=2,w+=2;
}
}
*/
// src and trg can be the same pointer (in place encryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit:  int key[4] = {n1,n2,n3,n4};
// void encipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k )
#define TEA_ENCODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) {\
	register unsigned int y=v[0],z=v[1],n=32,sum=0; \
	while(n-->0) { sum += delta; y += (z << 4)+a ^ z+sum ^ (z >> 5)+b; z += (y << 4)+c ^ y+sum ^ (y >> 5)+d; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

// src and trg can be the same pointer (in place decryption)
// len must be in bytes and must be multiple of 8 byts (64bits).
// key is 128bit: int key[4] = {n1,n2,n3,n4};
// void decipher(unsigned int *const v,unsigned int *const w,const unsigned int *const k)
#define TEA_DECODE( src,trg,len,key ) {\
	register unsigned int *v = (src), *w = (trg), *k = (key), nlen = (len) >> 3; \
	register unsigned int delta=0x9E3779B9,a=k[0],b=k[1],c=k[2],d=k[3]; \
	while (nlen--) { \
	register unsigned int y=v[0],z=v[1],sum=0xC6EF3720,n=32; \
	while(n-->0) { z -= (y << 4)+c ^ y+sum ^ (y >> 5)+d; y -= (z << 4)+a ^ z+sum ^ (z >> 5)+b; sum -= delta; } \
	w[0]=y; w[1]=z; v+=2,w+=2; }}

//////////////////////////////////////////////////////////////////////////
// CDataProbe implementation.
//////////////////////////////////////////////////////////////////////////
CDataProbe::CDataProbe()
{
	if (test_name[0]) // Make sure string is linked.
		m_pRandAlloc = 0;

	m_pRandAlloc = 0;

	// Add current module. must be CrySystem.dll
	SModuleInfo module;
	module.filename = string("CrySystem")+string(".dll");
	module.handle = (void*)test_name;
#if !defined(LINUX)
	module.handle = gDLLHandle;
	m_loadedModules.push_back(module);
#endif
#ifdef WIN32
	char sPath[_MAX_PATH];
	char dir[_MAX_DIR];
	char drive[_MAX_DRIVE];
	GetModuleFileName( GetModuleHandle(NULL),sPath,sizeof(sPath) );
	_splitpath( sPath,drive,dir,NULL,NULL);
	_makepath( sPath,drive,dir,NULL,NULL);
	m_sBinPath = sPath;
#endif
}

CDataProbe::~CDataProbe()
{
}

//////////////////////////////////////////////////////////////////////////
bool CDataProbe::GetDataCode( char *pBuf,int nSize,SDataProbeContext &ctx )
{
	uint64 nCode = ((uint64)rand()) | (((uint64)rand())<<16)  | (((uint64)rand())<<32) | (((uint64)rand())<<48);
	nCode = 0; //@TODO: Comment out this later, 0 only for debugging.

	// Hash readed data with choosen algorithm, store result into 64bit nCode.
	switch ((ctx.nCodeInfo) & 7)
	{
	case DATAPROBE_CRC32:
		{
			nCode = CRC_32( pBuf,nSize );
			nCode = nCode | ((~nCode) << 32);
		}
		break;
	case DATAPROBE_CRC64:
		{
			nCode = CRC_64( pBuf,nSize );
		}
		break;
	case DATAPROBE_ADLER32:
		{
			unsigned int code = ADLER_32(0L, 0, 0);
			nCode = ADLER_32( 1,pBuf,ctx.nSize );
			nCode = nCode | ((~nCode) << 32);
		}
		break;
	case DATAPROBE_PURE_CRC32:
		{
			ctx.nCode = CRC_32( pBuf,nSize );
			return true;
		}
		break;
	case DATAPROBE_MD5:
		{
			char md5[16];
			GetMD5( (char*)pBuf,nSize,md5 );
			nCode = *((uint64*)md5);
			return true;
		}
		break;
	default:
		{
			nCode = CRC_32( pBuf,nSize );
			nCode = nCode | ((~nCode) << 32);
		}
	}
	// scramble code not to look like crc or alike.
	int tkey[4] = {2985634234,378634893,387681212,436851212};
	TEA_ENCODE( (unsigned int*)&nCode,(unsigned int*)&nCode,8,(unsigned int*)tkey );
	ctx.nCode = nCode;
	return true;
}

//////////////////////////////////////////////////////////////////////////
uint32 CDataProbe::GetHash( const char *sString )
{
	return CRC_32( sString,strlen(sString) );
}

//////////////////////////////////////////////////////////////////////////
uint32 CDataProbe::GetHash( const void *buffer,int len )
{
	return CRC_32( (const char*)buffer,len );
}

//////////////////////////////////////////////////////////////////////////
bool CDataProbe::GetCode( SDataProbeContext &ctx )
{
	if (ctx.nSize <= 0)
		return false;

	if (ctx.pBuffer)
	{
		return GetDataCode( (char*)ctx.pBuffer,ctx.nSize,ctx );
	}

	// Try to open file and hash it.
	FILE *file = fopen( ctx.sFilename.c_str(),"rb" );
	if (file)
	{
		fseek( file,0,SEEK_END );
		unsigned int nFileSize = ftell(file);
		if (ctx.nOffset >= nFileSize)
		{
			fclose(file);
			return false;
		}
		ctx.nSize = min( ctx.nSize,nFileSize-ctx.nOffset );

		if (fseek( file,ctx.nOffset,SEEK_SET ) != 0)
		{
			fclose( file );
			return false;
		}
		char *pBuf = (char*)malloc( ctx.nSize );
		if (fread( pBuf,ctx.nSize,1,file ) != 1)
		{
			free( pBuf );
			fclose(file);
			return false;
		}
		
		// Hash readed data with choosen algorithm, store result into 64bit nCode.
		bool bOk = GetDataCode( pBuf,ctx.nSize,ctx );

		free( pBuf );
		fclose(file);

		return bOk;
	}
	else
	{
		// Real file on disk not found... so check against one in the pak.
		// Try to open file and hash it.
		CCryFile cryfile;
		if (cryfile.Open( ctx.sFilename.c_str(),"rb" ))
		{
			unsigned int nFileSize = cryfile.GetLength();
			if (ctx.nOffset >= nFileSize)
			{
				return false;
			}
			ctx.nSize = min( ctx.nSize,nFileSize-ctx.nOffset );

			if (cryfile.Seek( ctx.nOffset,SEEK_SET ) != 0)
			{
				return false;
			}
			char *pBuf = (char*)malloc( ctx.nSize );
			if (cryfile.Read( pBuf,ctx.nSize ) != ctx.nSize)
			{
				free( pBuf );
				return false;
			}

			// Hash readed data with choosen algorithm, store result into 64bit nCode.
			bool bOk = GetDataCode( pBuf,ctx.nSize,ctx );

			free( pBuf );

			return bOk;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
inline int64 GetRandom( int64 nRange )
{
	return (int64(nRange) * int64(rand())) / RAND_MAX;
}

//////////////////////////////////////////////////////////////////////////
bool CDataProbe::GetRandomFileProbe( SDataProbeContext &ctx,bool bAtEnd )
{
	// Try to open file and hash it.
	FILE *file = fopen( ctx.sFilename.c_str(),"rb" );
	if (file)
	{
		fseek( file,0,SEEK_END );
		int64 nFileSize = ftell(file);

		// Choose any random size.
		int64 nOffset = 0;
		int64 nSize = GetRandom( MAX_PROBE_SIZE ) + 100;
		if (bAtEnd)
		{
			// Prefare probe near the file end.
			nSize = min( nSize,nFileSize-1 );
			// Choose offset.
			nOffset = nFileSize - nSize - GetRandom( 1000 );
		}
		else
		{
			nOffset = GetRandom( nFileSize - nSize );

			if (nFileSize < MAX_PROBE_SIZE)
			{
				// Choose small random offset from both sides.
				int nOffset1 = 0;
				int nOffset2 = 0;
				do {
					nOffset1 = GetRandom( 1000 );
					nOffset2 = GetRandom( 1000 );
				} while (nOffset1+nOffset2 >= nFileSize+1);
				nOffset = nOffset1;
				nSize = nFileSize - nOffset2 - nOffset1;
			}
		}

		if (nOffset < 0)
			nOffset = 0;
		if (nSize < 0)
			nSize = nFileSize;

		ctx.nSize = nSize;
		ctx.nOffset = nOffset;

		fclose(file);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
void CDataProbe::RandSeed( uint32 seed )
{
	m_rand.Seed( seed );
}

uint32 CDataProbe::GetRand()
{
	return m_rand.Rand();
}

float CDataProbe::GetRand( float fMin,float fMax )
{
	return m_rand.Rand( fMin,fMax );
}

//////////////////////////////////////////////////////////////////////////
// Timur.
// This is FarCry.exe authentication function, this code is not for public release!!
//////////////////////////////////////////////////////////////////////////
bool CDataProbe::CheckLoader( void *pFunc )
{
	const char *sKey = "Timur Davidenko";
	typedef void (*AuthFunction)( void *data );
	AuthFunction pCheckFunc = (AuthFunction)pFunc;

	unsigned int data1[8];
	unsigned int data2[8];
	memcpy( data1,sKey,8 );

#ifdef FARCRY_EXE_CRC_CHECK
	// Comment out after alpha master.
	if (!CheckLoaderFC()) 
	{
		int *p = 0;
		*p = 1;
		return false;
	}
#endif // FARCRY_EXE_CRC_CHECK

	// data1 is filled with 32 bytes of random data.
	// It is then encrypted with key1 and send to supplied check function.
	// Function is supposed to uncrypt the data, and encrypt it again with key2.
	// when function returns we should be able to uncrypt the data with key2 and compare with original.
	// if it fails, we possibly running from the not authorized loader.

	// set of keys (one for farcry.exe second for dedicated server).
	int keys[6][4] = {
		{1873613783,235688123,812763783,1745863682},
		{1897178562,734896899,156436554,902793442},
		{1178362782,223786232,371615531,90884141},
		{89158165, 1389745433,971685123,785741042},
		{389623487,373673863,657846392,378467832},
		{1982697467,3278962783,278963782,287678311},
	};

	m_rand.Seed( GetTickCount() );
	// Data assumed to be 32 bytes (8 ints).
	for (int  i = 0; i < 8; i++)
	{
		data1[i] = m_rand.Rand();
	}
	//////////////////////////////////////////////////////////////////////////
	// First try... key0,key1
	//////////////////////////////////////////////////////////////////////////
	TEA_ENCODE( data1,data2,32,(unsigned int*)keys[0] );
	pCheckFunc( data2 );
	TEA_DECODE( data2,data2,32,(unsigned int*)keys[1] );

	// Now check data1 and data2.
	if (memcmp( data1,data2,32 ) == 0)
	{
		// Authentication passed.
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Second try... key2,key3
	//////////////////////////////////////////////////////////////////////////
	TEA_ENCODE( data1,data2,32,(unsigned int*)keys[2] );
	pCheckFunc( data2 );
	TEA_DECODE( data2,data2,32,(unsigned int*)keys[3] );
	
	// Now check data1 and data2.
	if (memcmp( data1,data2,32 ) == 0)
	{
		// Authentication passed.
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Third try... key4,key5
	//////////////////////////////////////////////////////////////////////////
	TEA_ENCODE( data1,data2,32,(unsigned int*)keys[4] );
	pCheckFunc( data2 );
	TEA_DECODE( data2,data2,32,(unsigned int*)keys[5] );

	// Now check data1 and data2.
	if (memcmp( data1,data2,32 ) == 0)
	{
		// Authentication passed.
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CDataProbe::CheckLoaderFC()
{
#ifdef FARCRY_EXE_CRC_CHECK
	// Open exe file.
	CCryFile file;

	// crysystem.dll
	string fsysname = string("c")+"r"+"y"+"s"+"y"+"s"+"t"+"e"+"m"+"."+"d"+"l"+"l";
	// farcry.exe
	string fcname = string("f")+"a"+"r"+"c"+"r"+"y"+"."+"e"+"x"+"e";

	if (!file.Open( (m_sBinPath + fsysname).c_str(),"rb" ))
		return false;

	int nLen = file.GetLength();
	char *pBuffer = new char[nLen+16];
	file.Read( pBuffer,nLen );
	// find embedded string.
	
	int nStrLen = strlen(test_name)+1;
	CryLogAlways( "Len=%d",nStrLen );
	//nStrLen = TEST_NAME_END;
	unsigned int nStoredCode = 0;
	memcpy( &nStoredCode,test_name+nStrLen,sizeof(nStoredCode) );

	bool bFound = false;
	unsigned int nCode = 0;

	for (int i = 0; i < nLen-nStrLen-32; i++)
	{
		if (memcmp(pBuffer+i,test_name,nStrLen) == 0)
		{
			// Found!
			// Read code.
			memcpy( &nStoredCode,pBuffer+i+nStrLen,sizeof(nCode) );
			bFound = true;
			break;
		}
	}
	if (!bFound)
		return false;

	// Now compare code to the farcry.exe.
	SDataProbeContext ctx;
	ctx.nCodeInfo = DATAPROBE_PURE_CRC32;
	ctx.sFilename = m_sBinPath + fcname;
	ctx.nSize = 10000000;
	if (!GetCode(ctx))
		return false;
	
	unsigned int nCurrentCode = ctx.nCode; // Invert of CRC32

	nCurrentCode = ~nCurrentCode; // Invert of CRC32

	// Codes must match.
	if (nCurrentCode != nStoredCode)
		return false;

	delete []pBuffer;

#endif // FARCRY_EXE_CRC_CHECK
	CryLogComment( "Init Loader" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
int CDataProbe::Compress( void *dest,unsigned int &destLen,const void *source, unsigned int sourceLen,int level )
{
	unsigned long dlen = destLen;
	int res = ::compress2( (Bytef*)dest,&dlen,(const Bytef*)source,sourceLen,level );
	destLen = dlen;
	return res;
}

//////////////////////////////////////////////////////////////////////////
int CDataProbe::Uncompress( void *dest,unsigned int &destLen,const void *source, unsigned int sourceLen )
{
	unsigned long dlen = destLen;
	int res = ::uncompress( (Bytef*)dest,&dlen,(const Bytef*)source,sourceLen );
	destLen = dlen;
	return res;
}

void CDataProbe::RandomAlloc()
{
#ifndef _DEBUG
	RandSeed( GetTickCount() );
	m_pRandAlloc = malloc( (GetRand() & 0xFFF) ); // do a random allocation to make debugging harder.
#endif
}

#if !defined(LINUX)
//////////////////////////////////////////////////////////////////////////
// Get address of executable code RAM for loaded DLL or EXE.
//////////////////////////////////////////////////////////////////////////
static void GetModuleCodeAddress( void *base,void **pCodeStartAddress,int *nCodeSize )
{
	const IMAGE_DOS_HEADER *dos_head = (IMAGE_DOS_HEADER*)base;

	assert(base);
	assert(pCodeStartAddress);
	assert(nCodeSize);

	*pCodeStartAddress = 0;
	*nCodeSize = 0;

#include <pshpack1.h>     // no padding.
	const struct PEHeader
	{
		DWORD signature;
		IMAGE_FILE_HEADER _head;
		IMAGE_OPTIONAL_HEADER opt_head;
		IMAGE_SECTION_HEADER *section_header;  // actual number in NumberOfSections
	}
	*header;
#include <poppack.h>

	if (dos_head->e_magic != IMAGE_DOS_SIGNATURE)
	{
		// Wrong pointer, not to PE header.
		return;
	}                         
	// verify DOS-EXE-Header
	// after end of DOS-EXE-Header: offset to PE-Header
	header = (PEHeader*)(const void *)((char *)dos_head + dos_head->e_lfanew);

	if (IsBadReadPtr(header, sizeof(*header)))  // start of PE-Header
	{
		// (no PE header, probably DOS executable)
		return;
	}

/*
	CryLog( "[CodeModule] base: %x",base );
	CryLog( "[CodeModule] base of code: %x",header->opt_head.BaseOfCode );
	CryLog( "[CodeModule] size of code: %x",header->opt_head.SizeOfCode );
*/

	void *pCodeAddress = (char*)base + header->opt_head.BaseOfCode;
	if (IsBadReadPtr(pCodeAddress, header->opt_head.SizeOfCode ))
	{
		// Something wrong..., cant read code memory, skip it.
		return;
	}
	*pCodeStartAddress = pCodeAddress;
	*nCodeSize = header->opt_head.SizeOfCode;
	
	/*
	{
		int sect;
		const IMAGE_SECTION_HEADER *section_header;
		for (sect = 0, section_header = header->section_header; sect < header->_head.NumberOfSections; sect++, section_header++)
		{

		}
	}
	*/
}
#endif //LINUX

//////////////////////////////////////////////////////////////////////////
bool CDataProbe::GetRandomModuleProbe( SDataProbeContext &ctx )
{
#ifndef WIN32
	return false; // Not WIN32
#else // WIN32
	if (!ctx.pModuleBaseAddress)
	{
		// Try to find module handle from filename.
		HMODULE hModule = GetModuleHandle( ctx.sFilename.c_str() );
		if (!hModule)
			return false;
		ctx.pModuleBaseAddress = hModule;
	}

	void *pCodeAddress = 0;
	int nCodeSize = 0;
	// Get code sections pointers.
	GetModuleCodeAddress( ctx.pModuleBaseAddress,&pCodeAddress,&nCodeSize );
	if (!pCodeAddress || nCodeSize == 0)
		return false;

	// Choose small random offset from both sides.
	int nOffset1 = 0;
	int nOffset2 = 0;
	do {
		nOffset1 = GetRandom( 1000 );
		nOffset2 = GetRandom( 1000 );
	} while (nOffset1+nOffset2 >= nCodeSize+1);

	ctx.pBuffer = pCodeAddress;
	ctx.nOffset = nOffset1;
	ctx.nSize = nCodeSize - nOffset1 - nOffset2;
	if (IsBadReadPtr( (char*)pCodeAddress+nOffset1,ctx.nSize))
	{
		// Something wrong..., cant read code memory, skip it.
		return false;
	}

	/*
	//////////////////////////////////////////////////////////////////////////
	// Write this to file....
	CryLog( "[CodeModule] %s, base: %x",ctx.sFilename.c_str(),ctx.pModuleBaseAddress );
	char *pBuf = (char*)malloc(ctx.nSize + 128);
	memset(pBuf,0,ctx.nSize+64);
	memcpy(pBuf,(char*)ctx.pBuffer+ctx.nOffset,ctx.nSize);
	FILE *file;
	if (!GetISystem()->IsDedicated())
		file = fopen( (ctx.sFilename+".client_code").c_str(),"wb" );
	else
		file = fopen( (ctx.sFilename+".server_code").c_str(),"wb" );
	fwrite( pBuf,ctx.nSize,1,file );
	fclose(file);

	unsigned int nCode = CRC_32( pBuf,ctx.nSize );	
	CryLog( "Code CRC32=%u, offset=%d,size=%d",nCode,ctx.nOffset,ctx.nSize );
	free(pBuf);
	//////////////////////////////////////////////////////////////////////////
	*/

	return true;
#endif //WIN32
}

//////////////////////////////////////////////////////////////////////////
bool CDataProbe::GetModuleProbe( SDataProbeContext &ctx )
{
#ifndef WIN32
	return false; // Not WIN32
#else // WIN32
	if (!ctx.pModuleBaseAddress)
	{
		// Try to find module handle from filename.
		HMODULE hModule = GetModuleHandle( ctx.sFilename.c_str() );
		if (!hModule)
			return false;
		ctx.pModuleBaseAddress = hModule;
	}

	void *pCodeAddress = 0;
	int nCodeSize = 0;
	// Get code sections pointers.
	GetModuleCodeAddress( ctx.pModuleBaseAddress,&pCodeAddress,&nCodeSize );
	if (!pCodeAddress || nCodeSize == 0)
		return false;

	ctx.pBuffer = pCodeAddress;
	if (IsBadReadPtr( (char*)pCodeAddress+ctx.nOffset,ctx.nSize))
	{
		// Something wrong..., cant read code memory, skip it.
		return false;
	}

	/*
	//////////////////////////////////////////////////////////////////////////
	// Write this to file....
	CryLog( "[CodeModule] %s, base: %x",ctx.sFilename.c_str(),ctx.pModuleBaseAddress );
	char *pBuf = (char*)malloc(ctx.nSize + 128);
	memset(pBuf,0,ctx.nSize+64);
	memcpy(pBuf,(char*)ctx.pBuffer+ctx.nOffset,ctx.nSize);
	FILE *file;
	if (!GetISystem()->IsDedicated())
		file = fopen( (ctx.sFilename+".client_code").c_str(),"wb" );
	else
		file = fopen( (ctx.sFilename+".server_code").c_str(),"wb" );
	fwrite( pBuf,ctx.nSize,1,file );
	fclose(file);

	unsigned int nCode = CRC_32( pBuf,ctx.nSize );	
	CryLog( "Code CRC32=%u, offset=%d,size=%d",nCode,ctx.nOffset,ctx.nSize );
	free(pBuf);
	//////////////////////////////////////////////////////////////////////////
	*/


	return true;
#endif //WIN32
}

//////////////////////////////////////////////////////////////////////////
int CDataProbe::GetLoadedModules( SModuleInfo **pModules )
{
	*pModules = 0;
	int numModules = m_loadedModules.size();
	if (numModules > 0)
	{
		*pModules = &m_loadedModules[0];
	}
	return numModules;
}

//////////////////////////////////////////////////////////////////////////
void CDataProbe::AddModule( SModuleInfo &moduleInfo )
{
	m_loadedModules.push_back(moduleInfo);
}

//////////////////////////////////////////////////////////////////////////
void CDataProbe::AESDecryptBuffer( const char *pSrcBuffer,int nSrcSize,char *pDestBuffer,int &nDestSize,const char *sKey )
{
	nDestSize = nSrcSize;
	memcpy( pDestBuffer,pSrcBuffer,nSrcSize );
}

//////////////////////////////////////////////////////////////////////////
void CDataProbe::GetMD5( const char *pSrcBuffer,int nSrcSize,char signatureMD5[16] )
{
//#if !defined(LINUX)
	struct MD5Context md5c; 
	MD5Init(&md5c);
	MD5Update(&md5c, (unsigned char*)pSrcBuffer,nSrcSize ); 
	MD5Final( (unsigned char*)signatureMD5, &md5c); 
//#endif
}

