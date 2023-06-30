////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2004.
// -------------------------------------------------------------------------
//  File name:   IDataProbe.h
//  Version:     v1.00
//  Created:     19/1/2004 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: Defines IDataProbe interface.
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////

#ifndef __IDataProbe_h__
#define __IDataProbe_h__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct SDataProbeContext
{
	// Folder name to check.
	string sFolder;
	// Filename to check.
	string sFilename;
	void *pBuffer;
	void *pModuleBaseAddress;
	unsigned int nOffset;
	unsigned int nSize;
	// Algorithm used fo code.
	unsigned int nCodeInfo;
	// Resulting code.
	uint64 nCode;

	SDataProbeContext()
	{
		pBuffer = 0;
		pModuleBaseAddress = 0;
		nOffset = nSize = nCodeInfo = 0;
		nCode = 0;
	}
};

struct IDataProbe
{
	struct SModuleInfo
	{
		string filename;
		void* handle;
	};

	virtual ~IDataProbe() {};
	virtual bool Dummy1( SDataProbeContext &ctx ) = 0;
	virtual int Dummy2( void *pBuf,int aa,SDataProbeContext &ctx ) = 0;
	virtual bool GetCode( SDataProbeContext &ctx ) = 0;
	virtual bool Dummy3( SDataProbeContext &ctx ) = 0;

	virtual bool GetRandomFileProbe( SDataProbeContext &ctx,bool bAtEnd )=0;
	virtual bool GetRandomModuleProbe( SDataProbeContext &ctx ) = 0;
	virtual bool GetModuleProbe( SDataProbeContext &ctx ) = 0;

	// Hash of ASCII string.
	virtual uint32 GetHash( const char *sString ) = 0;
	// Hash of any buffer.
	virtual uint32 GetHash( const void *buffer,int len ) = 0;

	// Return array of loaded modules information, returns number of modules.
	virtual int GetLoadedModules( SModuleInfo **pModules ) = 0;
	virtual void AddModule( SModuleInfo &moduleInfo ) = 0;

	virtual void RandomAlloc() = 0;
	
	// Compress block of data with zlib.
	virtual int Compress( void *dest,unsigned int &destLen,const void *source, unsigned int sourceLen,int level=6 ) = 0;
	// Uncompress block of data with zlib.
	virtual int Uncompress( void *dest,unsigned int &destLen,const void *source, unsigned int sourceLen ) = 0;

	//////////////////////////////////////////////////////////////////////////
	// MD5 Digest.
	//////////////////////////////////////////////////////////////////////////
	virtual void GetMD5( const char *pSrcBuffer,int nSrcSize,char signatureMD5[16] ) = 0;

	// Decrypt buffer.
	// Support in place decryption, (pSrcBuffer == pDestBuffer).
	// Output buffer should be at least the same size as source buffer.
	// Key must be at least 32 bytes long.
	virtual void AESDecryptBuffer( const char *pSrcBuffer,int nSrcSize,char *pDestBuffer,int &nDestSize,const char *sKey ) = 0;

	virtual	void RandSeed( uint32 seed ) = 0;
	virtual	uint32 GetRand() = 0;
	virtual	float GetRand( float fMin,float fMax ) = 0;
};

#endif // __IDataProbe_h__
