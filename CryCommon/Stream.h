//////////////////////////////////////////////////////////////////////
//
//	CryCommon source code
//	
//	File: stream.h
//  Description: stream class
//
//	History:
//	-07/25/2001:Created by Alberto Demichelis
//  -07/08/2002:Cleaned up, added compression Martin Mittring
//
//////////////////////////////////////////////////////////////////////


#ifndef _STREAM_H_
#define _STREAM_H_

// -------------------------------------------
// debugging defines:  
//#define STREAM_ACTIVETYPECHECK			// add a lot of bit for typechecking the steam
// -------------------------------------------

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// size in bytes
#define DEFAULT_STREAM_BYTESIZE			1124
#define MAX_STRING_SIZE							256

#ifndef CHAR_BIT
	#define CHAR_BIT 8
#endif //CHAR_BIT

#define DIV8(n)((n) >> 3)
#define MODULO8(n)((n)&0x00000007)
#define BITS2BYTES(n)(MODULO8((n))?(DIV8((n)) + 1):DIV8((n)))
#define BYTES2BITS(n)((n) << 3)
#define GET_BYTE_INDEX(n) DIV8((n))
#include <string>
#include "Cry_Math.h"								// Vec3
#include "ISystem.h"								// ISystem
#include "IStreamEngine.h"					// IStreamEngine
#include "IGame.h"									// IGame
#include "INetwork.h"								// INetwork
#include "ICompressionHelper.h"			// ICompressionHelper
#include "ILog.h"										// ILOg

class CStream;



#ifdef STREAM_ACTIVETYPECHECK
	#define STREAM_VERIFY_TYPE_READ(a)	{																																\
		if((GetISystem()->GetStreamEngine()->GetStreamCompressionMask()&0x80)!=0)													\
		{																																																	\
			unsigned char t1=0,t2=(a); _Read(t1);																														\
			if(t1!=t2)																																											\
			{																																																\
				char str[256];																																								\
				sprintf(str,"Entity read typedcookie error %d!=%d file '%s' @ line %d",												\
					(DWORD)t1,(DWORD)t2,__FILE__,__LINE__);																											\
				Debug(str);																																										\
				assert(0);																																										\
				return false;																																									\
			}																																																\
		}}
	#define STREAM_VERIFY_TYPE_WRITE(a)	{																																\
		if((GetISystem()->GetStreamEngine()->GetStreamCompressionMask()&0x80)!=0)													\
		{																																																	\
			unsigned char t=(a); _Write(t);																																	\
		}}
#else	// !STREAM_ACTIVETYPECHECK
	#define STREAM_VERIFY_TYPE_READ(a) {}
	#define STREAM_VERIFY_TYPE_WRITE(a) {}
#endif // STREAM_ACTIVETYPECHECK



struct IStreamAllocator
{
	virtual void *Alloc(size_t size)=0;
	virtual void *Realloc(void *old,size_t size)=0;
	virtual void Free(void *old)=0;
};


struct IStreamData
{
	virtual bool Read( CStream &inStream ) const=0;
	virtual bool Write( CStream &inStream ) const=0;
};



//#define OLD_STREAM
class CStream  
{
public:

	//! default constructor
	CStream()
	{
		m_sa=NULL;
		m_bStackBuffer=true;
		m_dwAllocatedBitSize = DEFAULT_STREAM_BYTESIZE*8;
		m_pBuffer=&m_vBuffer[0];
		Reset();
		if (m_dwAllocatedBitSize < 1)
			CryError("CStream:CStream()");

		m_iStreamVersion=999999;	// newest
	}

	//! constructor
	//! \param indwByteSize >0
	//! \param sa pointer to the memory allocation helper, must not be 0
	CStream( const size_t indwByteSize, IStreamAllocator *sa	)
	{
		assert(indwByteSize>0);
		assert(sa);

		m_sa=sa;
		m_bStackBuffer=false;
		m_dwAllocatedBitSize = indwByteSize*8;
		m_pBuffer=(BYTE *)m_sa->Alloc(BITS2BYTES(m_dwAllocatedBitSize));//new BYTE[BITS2BYTES(m_dwAllocatedBitSize)];
		
		Reset();
		m_iStreamVersion=999999;	// newest
	}

	//! constructor (used by physics system for physics on demand to store physics state for not demanded phyiscs objects)
	//! \param indwByteSize
	//! \param inpBuffer
	CStream( const size_t indwByteSize, BYTE *inpBuffer )
	{
		assert(indwByteSize);
		assert(inpBuffer);
		m_sa=NULL;
		m_bStackBuffer=true;
		m_dwAllocatedBitSize = m_dwBitSize = indwByteSize*8;
		m_pBuffer=inpBuffer;
		m_dwReadBitPos = 0;
		m_nCheckPoint = 0;
		m_iStreamVersion=999999;	// newest
	}

	//! destructor
	virtual ~CStream()
	{
		if(!m_bStackBuffer && m_pBuffer)
			m_sa->Free(m_pBuffer);

		m_pBuffer=NULL;
	}

	//! copy constructor
	CStream(const CStream& s)
	{
		m_iStreamVersion=s.m_iStreamVersion;

		m_pBuffer=&m_vBuffer[0];
		if(s.m_bStackBuffer)
		{
			m_dwAllocatedBitSize = s.GetAllocatedSize();
			if (m_dwAllocatedBitSize < 1)
				CryError("CStream:GetAllocatedSize");				
		}
		else
		{
			//m_dwAllocatedBitSize = s.GetAllocatedSize();
			//m_pBuffer=new BYTE[BITS2BYTES(m_dwAllocatedBitSize)];
			CryError("CStream:CStream");			
		}

		m_bStackBuffer=s.m_bStackBuffer;

		Reset();
		if(s.GetSize())
		{
			SetSize(s.GetSize());
			memcpy(GetPtr(),s.GetPtr(),BITS2BYTES(s.GetSize()));
		}
	}

	//! assignment operator
	CStream& operator =(const CStream &s)
	{
		m_iStreamVersion=s.m_iStreamVersion;

		if(GetAllocatedSize() < s.GetAllocatedSize())
		{
			CryError("CStream:operator(=)");			
		}
		Reset();
		if(s.GetSize())
		{
			SetSize(s.GetSize());
			memcpy(GetPtr(),s.GetPtr(),BITS2BYTES(s.GetSize()));
		}
		return *this;
	}

	void CStream::Debug( const char *inTxt=0 );

	//! set a bit at the specified position in the bitstream
	bool SetBit(size_t nPos, DWORD_PTR nValue);
	//! get a bit at the specified position in the bitstream
	bool GetBit(size_t nPos,bool &bBit);
	//! append nSize bits at the end
	bool WriteBits(BYTE *pBits, size_t nSize);
	//! read nSize bits from the current read pos in the bitstream
	bool ReadBits(BYTE *pBits, size_t nSize);
	//! write a number into the bitsream using nSize bits
	bool WriteNumberInBits(int n,size_t nSize);
	bool WriteNumberInBits(unsigned int n,size_t nSize);
	//! read a number into the bitsream using nSize bits
	bool ReadNumberInBits(int &n,size_t nSize);
	bool ReadNumberInBits(unsigned int &n,size_t nSize);
	//! read a nSize bits from nPos in the bitstream
	bool GetBits(BYTE *pBits, size_t nPos, size_t nSize);
	//! write a nSize bits from nPos in the bitstream
	bool SetBits(BYTE *pBits, size_t nPos, size_t nSize);
//! read a variable from the bitstream an increament the read position
//@{
	bool Read(unsigned char &uc);
	bool Read(unsigned short &us);
	bool Read(unsigned int &ui);
	bool Read(char &c);
	bool Read(short &s);
	bool Read(int &i);
	bool Read(float &f);
	bool Read(bool &b);
	bool Read(char *psz, const DWORD indwBufferSize);
	bool Read(string &str);
	bool Read(Vec3 &v);
	//!read all remaining data in the stream starting from the read pos
	bool Read(CStream &stm);
	//!read packed
	bool ReadPkd(unsigned char &uc);
	bool ReadPkd(unsigned short &us);
	bool ReadPkd(unsigned int &ui);
	bool ReadPkd(char &c);
	bool ReadPkd(short &c);
	bool ReadPkd(int &i);
	//! packed, inType specifies the compression mode
	bool ReadPkd( IStreamData &outData );
//@}
	bool _Read(unsigned char &uc);			// not typechecked

//! write a variable from the bitstream an increament the read position
//@{
	bool Write(unsigned char uc);
	bool Write(unsigned short us);
	bool Write(unsigned int ui);
	bool Write(char c);
	bool Write(short c);
	bool Write(int i);
	bool Write(float f);
	bool Write(bool b);
	bool Write(const char *psz);
	bool Write(const string &str);
	bool Write(CStream &stm);
	bool Write(const Vec3 &v);
	bool _Write(unsigned char c);			// not typechecked

	bool WritePkd(unsigned char uc);
	bool WritePkd(unsigned short us);
	bool WritePkd(unsigned int ui);
	bool WritePkd(char c);
	bool WritePkd(short c);
	bool WritePkd(int i);
	//! packed, inType specifies the compression mode
	//! /param v is modified to the reconstruction of the compressed value
	bool WritePkd( const IStreamData &inData );

	bool WritePacked(unsigned int ui);
#if defined(WIN64)
	// Win64 defines size_t as 64-bit integer and there's an ambiguity between converting it to long or int automatically
	// if we need to save/restore it really as a 64-bit int, care must be taken to match the write/read functions in 32-bit version
	bool WritePacked(size_t sz) {return WritePacked((unsigned int)sz);}
	bool ReadPacked(size_t &sz) { return ReadPacked((unsigned int&)sz); }
#endif
	bool ReadPacked(unsigned int &ui);
//@}

	//! write data to stream.
	//! @param nSize size of buffer in bytes.
	bool WriteData( const void *pBuffer,int nSize ) { return WriteBits( (BYTE*)pBuffer,nSize*8 ); };
	//! read data from stream.
	//! @param nSize size of buffer in bytes.
	bool ReadData( void *pBuffer,int nSize )  { return ReadBits( (BYTE*)pBuffer,nSize*8 ); };

//! write a variable from the bitstream starting from the dwPos bit
//@{
	bool GetAt(size_t dwPos, BYTE &c);
	bool GetAt(size_t dwPos, WORD &w);
	bool GetAt(size_t dwPos, DWORD &dw);
#if defined(WIN64) || defined(LINUX64)
	bool GetAt(size_t dwPos, ULONG_PTR &dw);
#endif
//@}

	//! move the read pointer to the dwPos bit
	bool Seek(size_t dwPos = 0);
	//! Get the sizeof the stream in bits
	size_t GetSize()const {return m_dwBitSize;}
	//! Set the sizeof the stream in bits
	bool SetSize( const size_t indwBitSize )
	{
		if(m_dwAllocatedBitSize<indwBitSize)
		{
			if(!Resize(indwBitSize))
				return false;
		}
		m_dwBitSize = indwBitSize;
		return true;
	}
	//! Set the number of unreaded bites into the stream
	size_t GetNumUnreadedBytes(){return m_dwBitSize - m_dwReadBitPos;}
	//! Get the number of allocated bits into the stream(max size)
	size_t GetAllocatedSize() const{ return m_dwAllocatedBitSize;}
	//! get the current read position
	size_t GetReadPos(){return m_dwReadBitPos;}
	//! copy the stream into p
	bool GetBuffer(BYTE *p, size_t nMaxBuf)
	{
		if (nMaxBuf < m_dwBitSize)
			return false;
		memcpy(p, GetPtr(), BITS2BYTES(m_dwBitSize));
		return true;
	}
	//! get a pointer to the stream buffer
	BYTE *GetPtr() const 
	{
		return (BYTE *)m_pBuffer;
	}
	//! legacy
	BYTE *GetUnreadedPtr()
	{
		return &m_vBuffer[m_dwReadBitPos];
	}
	//! (END OF STREAM)return true if the read pointer is at the end of the stream
	bool EOS()
	{
		return m_dwBitSize<=m_dwReadBitPos;
	}
	//! cleanup the buffer ans reset size and read pointer to 0
	void Reset();
//only for debug
//	unsigned int TUNING_EvaluateRLE();
	void SetCheckPoint(){m_nCheckPoint=m_dwReadBitPos;}

	void AlignRead()  { /*m_dwReadBitPos = (m_dwReadBitPos+7)&~7;*/ };
	void AlignWrite() { /*m_dwBitSize    = (m_dwBitSize   +7)&~7;*/ };  // cannot overflow m_dwAllocatedBitSize if byte-aligned

	//! resize the stream (is in bits)
	bool Resize( const size_t iniBitSize )
	{
		if(!m_sa)return false;
		size_t oldsize=BITS2BYTES(m_dwAllocatedBitSize);

		m_pBuffer=(BYTE *)m_sa->Realloc(m_pBuffer,BITS2BYTES(iniBitSize));

		if(oldsize<BITS2BYTES(iniBitSize))
			memset(&m_pBuffer[oldsize],0,BITS2BYTES(iniBitSize)-oldsize);

		m_dwAllocatedBitSize=iniBitSize;
		return true;
	}
	
	//! used for to be able to load old savegames
	int GetStreamVersion() const { return m_iStreamVersion; }
	//! used for to be able to load old savegames
	void SetStreamVersion( const int iVersion ) { m_iStreamVersion=iVersion; }

private: // -----------------------------------------------------------------------

	unsigned int __htonl(unsigned int n){
		return (unsigned int)(((n&0xFF000000)>>24)  |
												((n&0x00FF0000)>>8)  |
												((n&0x0000FF00)<<8)  |
												((n&0x000000FF)<<24));
	}
	unsigned int __ntohl(unsigned int n){
		return (unsigned int)(((n&0xFF000000)>>24)  |
												((n&0x00FF0000)>>8)  |
												((n&0x0000FF00)<<8)  |
												((n&0x000000FF)<<24));
	}

	BYTE *								m_pBuffer;													//!<
	BYTE									m_vBuffer[DEFAULT_STREAM_BYTESIZE];	//!<
	size_t								m_dwAllocatedBitSize;								//!< allocated size is in bits
	size_t								m_dwBitSize;												//!< payload size is in bits
	size_t								m_dwReadBitPos;											//!< pos in bits
	size_t								m_nCheckPoint;											//!<
	IStreamAllocator *		m_sa;																//!< might be 0 if there was no allocator specified
	bool									m_bStackBuffer;											//!<
	int										m_iStreamVersion;										//!< 999999=newest, PATCH1_SAVEVERSION, SAVEVERSION, ..	
};



#define _GROW() {if(!Resize(m_dwAllocatedBitSize*2))return false;}
//! BIT are nubered as follow
//! [0 1 2 3 4 5 6 7|8 9 10 11 12 13 14...
//! for instance if you want write 3 bits you should shift your data of 5 bits
inline bool CStream::SetBit(size_t nPos, DWORD_PTR nValue)
{
	if (nPos>=GetAllocatedSize())
	{
		if(!Resize(m_dwAllocatedBitSize*2))
		{
			Debug("SetBit resize failed");
			return false;
		}
	}
	BYTE *pBuf = GetPtr();
	pBuf += GET_BYTE_INDEX(nPos);
	if (nValue)
		*pBuf|=0x80 >> MODULO8(nPos);
	else 
		*pBuf&=~(0x80 >> MODULO8(nPos));
	return true;
}


//////////////////////////////////////////////////////////////////////////
inline void CStream::Debug( const char *inTxt )
{
	assert(0);

#if (defined(WIN32)||defined(WIN64))
	{
		char str[256];

		sprintf(str,"CStream::Debug (%d/%d/%d %c)%s\n",m_dwReadBitPos,m_dwBitSize,m_dwAllocatedBitSize,m_sa?'t':'f',inTxt?inTxt:"");

		OutputDebugString(str);

		GetISystem()->GetILog()->LogError("%s",str);
	}
#endif
}


inline bool CStream::GetBit(size_t nPos,bool &bBit)
{
	if (nPos>=m_dwBitSize)
		return false;
	BYTE *pBuf = GetPtr();
	pBuf += GET_BYTE_INDEX(nPos);
	bBit=(*pBuf&(0x80 >> (MODULO8(nPos)))) != 0;
	return true;
}

inline bool CStream::SetBits(BYTE *pBits, size_t nPos, size_t nSize)
{
#ifdef OLD_STREAM
	BYTE *pBuf, *pSource;
	DWORD_PTR nValue;
#endif
	if ((nPos + nSize)>GetAllocatedSize())
	{
		_GROW();
	}
	BYTE *pBuffer=GetPtr();
	// the following code can be optimized
#ifdef OLD_STREAM
	for (size_t n = 0; n < nSize; n++)
	{
		pBuf = pBuffer;
		pSource = pBits;
		pSource += GET_BYTE_INDEX(n);
		nValue = ((*pSource&(0x80 >> (MODULO8(n)))) != 0);
		pBuf += GET_BYTE_INDEX(nPos + n);
		if (nValue)
			*pBuf|=0x80 >> MODULO8(nPos + n);
		else 
			*pBuf&=~(0x80 >> MODULO8(nPos + n));
	}
#else
	BYTE *pSrc = pBits;
	BYTE *pStart = &pBuffer[GET_BYTE_INDEX(nPos)];// + (nPos >> 3);
	BYTE *pEnd = pBuffer + ((nSize + nPos - 1) >> 3);
	size_t nUpShift  = MODULO8(nPos);
	size_t nDownShift= 8 - nUpShift;
	BYTE cLastMask  = 0xFF << (7-MODULO8((nPos + nSize - 1)));
	BYTE cStartMask = 0xFF << nDownShift;
	BYTE cCurrent = *pSrc++;
	BYTE end=*pEnd;
	*pStart=(cCurrent >> nUpShift) | (*pStart & cStartMask);
	pStart++;
	while(pStart <= pEnd)
	{
		BYTE next = *pSrc++;
		*pStart++ = (cCurrent << nDownShift) | (next >> nUpShift);
		cCurrent = next;
	}
	*pEnd &= cLastMask;
	//m_dwBitSize += nSize;
#endif
	return true;
}

inline bool CStream::GetBits(BYTE *pBits, size_t nPos, size_t nSize)
{
#ifdef OLD_STREAM
	BYTE *pBuf, *pDest;
	DWORD_PTR nValue;
#endif
	BYTE *pBuffer=GetPtr();
	if ((nPos + nSize)>m_dwBitSize)
		return false;
	// the following code can be optimized
#ifdef OLD_STREAM
	for (size_t n = 0; n < nSize; n++)
	{
		pBuf = pBuffer;
		pBuf += GET_BYTE_INDEX(nPos + n);
		pDest = pBits;
		pDest += GET_BYTE_INDEX(n);
		nValue = ((*pBuf&(0x80 >> (MODULO8(nPos + n)))) != 0);
		if (nValue)
			*pDest|=0x80 >> MODULO8(n);
		else 
			*pDest&=~(0x80 >> MODULO8(n));
	}
#else
	BYTE *stPtr = pBuffer + (nPos >> 3);
	size_t byteCount = (nSize + 7) >> 3;

	BYTE *ptr = (BYTE *) pBits;

	size_t downShift = nPos & 0x7;
	size_t upShift = 8 - downShift;


	BYTE curB = *stPtr;
	while(byteCount--)
	{
		BYTE nextB = *++stPtr;
		*ptr++ = (curB << downShift) | (nextB >> upShift);
		curB = nextB;
	};
//	if(nSize&7) *(ptr-1) &= (1<<(nSize&7))-1;
	
#endif
	return true;
}

inline bool CStream::WriteBits(BYTE *pBits, size_t nSize)
{
	if (!SetBits(pBits, m_dwBitSize, nSize))
		return false;
	m_dwBitSize += nSize;
	return true;
}

inline bool CStream::ReadBits(BYTE *pBits, size_t nSize)
{
	if (!GetBits(pBits, m_dwReadBitPos, nSize))
		return false;
	m_dwReadBitPos += nSize;
	return true;
}

inline bool CStream::WriteNumberInBits(unsigned int n,size_t nSize)
{
	STREAM_VERIFY_TYPE_WRITE(50);
	assert(nSize>0 && nSize<=32);
	unsigned int nSwapped;
	if(nSize>32)
	{
		CryError("CStream:WriteNumberinBits");		
		return false;
	}
	n=n<<(32-nSize);
	nSwapped=__htonl(n);
	return WriteBits((BYTE *)&nSwapped,nSize);
}

inline bool CStream::ReadNumberInBits(unsigned int &n,size_t nSize)
{
	STREAM_VERIFY_TYPE_READ(50);
	assert(nSize>0 && nSize<=32);
	unsigned int nSwapped;
	if(nSize>32)
	{
		CryError("CStream:ReadNumberinBits ulong %d>32",(int)nSize);		
		return false;
	}
	if(!ReadBits((BYTE *)&nSwapped,nSize))return false;
	n=__ntohl(nSwapped);
	n=n>>(32-nSize);
	return true;
}

inline bool CStream::WriteNumberInBits(int n,size_t nSize)
{
	STREAM_VERIFY_TYPE_WRITE(51);
	assert(nSize>0 && nSize<=32);
	unsigned int nSwapped;
	if(nSize>32)
	{
		CryError("CStream:WriteNumberinBits");		
		return false;
	}
	n=n<<(32-nSize);
	nSwapped=__htonl((unsigned int)n);
	return WriteBits((BYTE *)&nSwapped,nSize);
}

inline bool CStream::ReadNumberInBits(int &n,size_t nSize)
{
	STREAM_VERIFY_TYPE_READ(51);
	assert(nSize>0 && nSize<=32);
	unsigned int nSwapped=0;
	if(nSize>32)
	{
		CryError("CStream:ReadNumberinBits int %d>32",(int)nSize);		
		return false;
	}
	if(!ReadBits((BYTE *)&nSwapped,(nSize)))return false;
	n=__ntohl((unsigned int)nSwapped);
	n=(n>>(32-nSize))&(0xFFFFFFFF>>(32-nSize));
	
	return true;
}

inline void CStream::Reset()
{
	memset(GetPtr(), 0, BITS2BYTES(GetAllocatedSize()));
	m_dwBitSize = 0;
	m_dwReadBitPos = 0;
	m_nCheckPoint = 0;
}

// *************************************
// ************** READ *****************
// *************************************

// not typechecked
inline bool CStream::_Read(unsigned char &uc)
{
	return ReadBits((BYTE *)&uc, BYTES2BITS(sizeof(unsigned char)));
}



inline bool CStream::Read(bool &b)
{
	STREAM_VERIFY_TYPE_READ(9);
	static bool bRet;
	static unsigned char cTemp;
	cTemp=0;
	bRet=GetBit(m_dwReadBitPos,b);
	if(bRet)
		m_dwReadBitPos++;
	return bRet;
}

inline bool CStream::Read(unsigned char &uc)
{
	STREAM_VERIFY_TYPE_READ(10);
	return ReadBits((BYTE *)&uc, BYTES2BITS(sizeof(unsigned char)));
}


inline bool CStream::Read(unsigned short &us)
{
	STREAM_VERIFY_TYPE_READ(11);
	return ReadBits((BYTE *)&us, BYTES2BITS(sizeof(unsigned short)));
}

inline bool CStream::Read(unsigned int &ul)
{
	STREAM_VERIFY_TYPE_READ(12);
	return ReadBits((BYTE *)&ul, BYTES2BITS(sizeof(unsigned int)));
}

inline bool CStream::Read(char &c)
{
	STREAM_VERIFY_TYPE_READ(13);
	return ReadBits((BYTE *)&c, BYTES2BITS(sizeof(char)));
}

inline bool CStream::Read(short &s)
{
	STREAM_VERIFY_TYPE_READ(14);
	return ReadBits((BYTE *)&s, BYTES2BITS(sizeof(short)));
}

inline bool CStream::Read(int &i)
{
	STREAM_VERIFY_TYPE_READ(15);
	return ReadBits((BYTE *)&i, BYTES2BITS(sizeof(int)));
}

inline bool CStream::Read(float &f)
{
	STREAM_VERIFY_TYPE_READ(16);
	return ReadBits((BYTE *)&f, BYTES2BITS(sizeof(float)));
}



inline bool CStream::Read( char *psz, const DWORD indwBufferSize )
{
	STREAM_VERIFY_TYPE_READ(30);
	ICompressionHelper *pCHelper = GetISystem()->GetINetwork()->GetCompressionHelper();

	return pCHelper->Read(*this,psz,indwBufferSize);
}

inline bool CStream::Read(string &str)
{	
	static char cTemp[MAX_STRING_SIZE];
	STREAM_VERIFY_TYPE_READ(30);
	ICompressionHelper *pCHelper = GetISystem()->GetINetwork()->GetCompressionHelper();
	
	bool bRet = pCHelper->Read(*this,cTemp,MAX_STRING_SIZE);
	str=cTemp;
	return bRet;
}

inline bool CStream::Read(Vec3 &v)
{
	STREAM_VERIFY_TYPE_READ(31);
	Read(v.x);
	Read(v.y);
	return Read(v.z);
}

inline bool CStream::Read(CStream &stm)
{
	size_t nToRead=GetSize()-GetReadPos();
	if(nToRead>0 && ((stm.GetSize()+nToRead)<= stm.GetAllocatedSize()))
	{
		static BYTE cTemp[MAX_STRING_SIZE];
		size_t readed;
		while(nToRead>0)
		{
			readed=min((int)nToRead,(int)BYTES2BITS(100));
			if(!ReadBits(cTemp,readed))return false;
			stm.WriteBits(cTemp,readed);
			nToRead-=readed;
		}
		return true;
	}
	return false;
}

// *************************************
// ************** WRITE ****************
// *************************************

// not typechecked
inline bool CStream::_Write(unsigned char uc)
{
	return WriteBits((BYTE *)&uc, BYTES2BITS(sizeof(unsigned char)));
}


inline bool CStream::Write(bool b)
{
	STREAM_VERIFY_TYPE_WRITE(9);

	bool bRet=SetBit(m_dwBitSize,b);
	if(bRet)
		m_dwBitSize++;
	return bRet;
}

inline bool CStream::Write(unsigned char uc)
{
	STREAM_VERIFY_TYPE_WRITE(10);
	return WriteBits((BYTE *)&uc, BYTES2BITS(sizeof(unsigned char)));
}


inline bool CStream::Write(unsigned short us)
{
	STREAM_VERIFY_TYPE_WRITE(11);
	return WriteBits((BYTE *)&us, BYTES2BITS(sizeof(unsigned short)));
}

inline bool CStream::Write(unsigned int ul)
{
	STREAM_VERIFY_TYPE_WRITE(12);
	return WriteBits((BYTE *)&ul, BYTES2BITS(sizeof(unsigned int)));
}

inline bool CStream::Write(char c)
{
	STREAM_VERIFY_TYPE_WRITE(13);
	return WriteBits((BYTE *)&c, BYTES2BITS(sizeof(char)));
}

inline bool CStream::Write(short s)
{
	STREAM_VERIFY_TYPE_WRITE(14);
	return WriteBits((BYTE *)&s, BYTES2BITS(sizeof(short)));
}

inline bool CStream::Write(int i)
{
	STREAM_VERIFY_TYPE_WRITE(15);
	return WriteBits((BYTE *)&i, BYTES2BITS(sizeof(int)));
}


inline bool CStream::Write(float f)
{
	STREAM_VERIFY_TYPE_WRITE(16);
	return WriteBits((BYTE *)&f, BYTES2BITS(sizeof(float)));
}


inline bool CStream::Write(CStream &stm)
{
	if ((m_dwBitSize + stm.GetSize()) >= GetAllocatedSize())
	{
		_GROW()
	}
		
	WriteBits(stm.GetPtr(), stm.GetSize());
	return true;
}

inline bool CStream::Write(const char *psz)
{
	STREAM_VERIFY_TYPE_WRITE(30);
	ICompressionHelper *pCHelper = GetISystem()->GetINetwork()->GetCompressionHelper();

	return pCHelper->Write(*this,psz);
}


inline bool CStream::Write(const string &str)
{
	return Write(str.c_str());
}

inline bool CStream::Write(const Vec3 &v)
{
	STREAM_VERIFY_TYPE_WRITE(31);
	Write(v.x);
	Write(v.y);
	return Write(v.z);
}




///////////////////////////////////////////////////////
inline bool CStream::WritePkd(unsigned char uc)
{
	STREAM_VERIFY_TYPE_WRITE(40);
	if(uc<=0x0F)
	{
		uc<<=4;
		Write(true);
		return WriteBits((BYTE *)&uc,4);
	}
	else{
		Write(false);
		return WriteBits((BYTE *)&uc, BYTES2BITS(sizeof(unsigned char)));
	}
	
}

inline bool CStream::WritePkd(unsigned short us)
{
	STREAM_VERIFY_TYPE_WRITE(41);
	if(us<=0xFF)
	{
		Write(true);
		return Write((BYTE)us);
	}
	else{
		Write(false);
		return WriteBits((BYTE *)&us, BYTES2BITS(sizeof(unsigned short)));
	}
}

inline bool CStream::WritePkd(unsigned int ul)
{
	STREAM_VERIFY_TYPE_WRITE(42);
	if(ul<=0xFFFF)
	{
		Write(true);
		return Write((unsigned short)ul);
	}
	else{
		Write(false);
		return WriteBits((BYTE *)&ul, BYTES2BITS(sizeof(unsigned int)));
	}
}

inline bool CStream::WritePkd(char c)
{
	STREAM_VERIFY_TYPE_WRITE(43);
	return WriteBits((BYTE *)&c, BYTES2BITS(sizeof(char)));
}

inline bool CStream::WritePkd(short s)
{
	STREAM_VERIFY_TYPE_WRITE(44);
	if(abs(s)<=0xFF)
	{
		Write(true);
		Write(s<0);
		return Write((BYTE)abs(s));
	}
	else{
		Write(false);
		return WriteBits((BYTE *)&s, BYTES2BITS(sizeof(short)));
	}
}

inline bool CStream::WritePkd(int i)
{
	STREAM_VERIFY_TYPE_WRITE(45);
	if(abs(i)<=0xFFFF)
	{
		Write(true);
		Write(i<0);
		return Write((unsigned short)abs(i));
	}
	else{
		Write(false);
		return WriteBits((BYTE *)&i, BYTES2BITS(sizeof(int)));
	}
	
}

///////////////////////////////////////////////////////

inline bool CStream::ReadPkd(unsigned char &uc)
{
	STREAM_VERIFY_TYPE_READ(40);
	bool comp;
	bool ret;
	Read(comp);
	if(comp)
	{
		BYTE b;
		ret=ReadBits((BYTE *)&b, 4);
		b>>=4;
		uc=b;
	}
	else{
		ret=ReadBits((BYTE *)&uc, BYTES2BITS(sizeof(unsigned char)));
	}
	return ret;
}

inline bool CStream::ReadPkd(unsigned short &us)
{
	STREAM_VERIFY_TYPE_READ(41);
	bool comp;
	bool ret;
	Read(comp);
	if(comp)
	{
		BYTE b;
		ret=Read(b);
		us=b;
	}
	else{
		ret=ReadBits((BYTE *)&us, BYTES2BITS(sizeof(unsigned short)));
	}
	return ret;
}

inline bool CStream::ReadPkd(unsigned int &ul)
{
	STREAM_VERIFY_TYPE_READ(42);
	bool ret;
	bool comp;
	ret=Read(comp);
	if(comp){
		unsigned short us;
		ret=Read(us);
		ul=us;
	}
	else{
		ret=ReadBits((BYTE *)&ul, BYTES2BITS(sizeof(unsigned int)));
	}
	return ret;
}

inline bool CStream::ReadPkd(char &c)
{
	STREAM_VERIFY_TYPE_READ(43);
	return ReadBits((BYTE *)&c, BYTES2BITS(sizeof(char)));
}


inline bool CStream::ReadPkd(short &s)
{
	STREAM_VERIFY_TYPE_READ(44);
	bool comp,ret,sign;

	Read(comp);
	if(comp)
	{
		BYTE b;
		Read(sign);
		ret=Read(b);
		s=sign?-b:b;
	}
	else
	{
		ret=ReadBits((BYTE *)&s, BYTES2BITS(sizeof(short)));
	}
	return ret;
}


inline bool CStream::ReadPkd(int &i)
{
	STREAM_VERIFY_TYPE_READ(45);
	bool comp,ret,sign;
	Read(comp);
	if(comp)
	{
		unsigned short b;
		Read(sign);
		ret=Read(b);
		i=sign?-b:b;
	}
	else
		ret=ReadBits((BYTE *)&i, BYTES2BITS(sizeof(int)));

	return ret;
}

inline bool CStream::ReadPkd( IStreamData &outData )
{
	return outData.Read(*this);
}


inline bool CStream::WritePkd( const IStreamData &inData )
{
	return inData.Write(*this);
}




inline bool CStream::WritePacked(unsigned int ul)
{
	STREAM_VERIFY_TYPE_WRITE(91);
	int i; 
	
	for(i=1;i<16 && ul>=1u<<i*2;i++);			// i=1..16

	bool res = WriteNumberInBits(i-1,4);		//
	
	res = WriteNumberInBits(ul,i*2);

	return res;
}

inline bool CStream::ReadPacked(unsigned int &ul)
{
	STREAM_VERIFY_TYPE_READ(91);
	int i;
	ul = 0;
	bool res = ReadNumberInBits(i,4);i++;			// i=1..16

	if(!res)
		return res;

	res = ReadNumberInBits(ul,i*2);

	return res;
}

///////////////////////////////////////////////////////
inline bool CStream::GetAt(size_t dwPos, BYTE &c)
{
	return GetBits((BYTE *)&c, dwPos, BYTES2BITS(sizeof(WORD)));
}

inline bool CStream::GetAt(size_t dwPos, WORD &w)
{
	return GetBits((BYTE *)&w, dwPos, BYTES2BITS(sizeof(WORD)));
}

inline bool CStream::GetAt(size_t dwPos, DWORD &dw)
{
	return GetBits((BYTE *)&dw, dwPos, BYTES2BITS(sizeof(DWORD)));
}

inline bool CStream::Seek(size_t dwPos)
{
	if (dwPos <= m_dwBitSize)
		m_dwReadBitPos = dwPos;
	else 
		return false;

	return true;
}



//#define NOT_COOKIEFIED



#ifndef NOT_COOKIEFIED
	#define WRITE_COOKIE_NO(stm,c) {\
	if((GetISystem()->GetStreamEngine()->GetStreamCompressionMask()&0x8)==0  || !GetISystem()->GetIGame()->GetModuleState(EGameMultiplayer))\
		stm.Write((BYTE)(c));\
	}
	#define WRITE_COOKIE(stm) WRITE_COOKIE_NO(stm,0xAA);
#else
	#define WRITE_COOKIE_NO(stm,c) {}
	#define WRITE_COOKIE(stm) {}
#endif

#ifndef NOT_COOKIEFIED
	#define VERIFY_COOKIE_NO(stm,c) {\
	if((GetISystem()->GetStreamEngine()->GetStreamCompressionMask()&0x8)==0 || !GetISystem()->GetIGame()->GetModuleState(EGameMultiplayer))\
	{\
		BYTE cCookie; stm.Read(cCookie);\
		if(cCookie!=(BYTE)(c))\
		{	\
			stm.Debug();\
			CryError( "COOKIE ERROR %x!=%x AT %i, %s, %s",(DWORD)cCookie,(DWORD)(c), __LINE__, __FUNCTION__,  __FILE__ );\
		} else {stm.SetCheckPoint();} \
		}\
	} 
	#define VERIFY_COOKIE(stm) VERIFY_COOKIE_NO(stm,0xAA)
	#define VERIFY_ENTITY_COOKIE_NO(stm,c) {\
		if((GetISystem()->GetStreamEngine()->GetStreamCompressionMask()&0x8)==0 || !GetISystem()->GetIGame()->GetModuleState(EGameMultiplayer))\
		{\
			BYTE cCookie; stm.Read(cCookie);\
			if (cCookie!=(BYTE)c)\
			{\
				stm.Debug();\
				m_pISystem->GetILog()->LogError("Entity read cookie error %x!=%x file '%s' @ line %d (%s of class %s)",\
					(DWORD)cCookie,(DWORD)c,__FILE__,__LINE__,GetName(),GetEntityClassName());\
				return false;\
			}\
		}\
	}
	#define VERIFY_ENTITY_COOKIE(stm) VERIFY_ENTITY_COOKIE_NO(stm,0xAA)
#else
	#define VERIFY_COOKIE(stm) {}
	#define VERIFY_COOKIE_NO(stm,c) {}
	#define VERIFY_ENTITY_COOKIE_NO(stm,c) {}
	#define VERIFY_ENTITY_COOKIE(stm) {}
#endif

//////////////////////////////////////////////////////////////////////////
class CDefaultStreamAllocator : public IStreamAllocator
{
	void *Alloc(size_t size) { return malloc(size); }
	void *Realloc(void *old,size_t size) { return realloc(old,size); }
	void Free(void *old) { free(old); }
};


#endif //_STREAM_H_
