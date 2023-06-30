#include "stdafx.h"
#include "compressionhelper.h"
#include "stream.h"								// CStream


CCompressionHelper::CCompressionHelper()
{
#ifdef GATHER_CHARSTATISTICS
	memset(m_dwCharStats,0,sizeof(DWORD)*256);
	m_dwWriteBitsUncompressed=0;
	m_dwWriteBitsCompressed=0;
#endif

	{
		DWORD dwCharStats[]=														// created with GATHER_CHARSTATISTICS 
		{
			47,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			131,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,
			61,51,19,2,18,15,3,0,0,0,0,0,0,0,0,0,
			0,7,0,14,2,0,7,9,0,1,0,0,4,0,0,2,
			28,0,26,6,14,4,0,0,7,0,0,0,0,0,0,0,
			0,11,0,5,5,6,0,0,0,4,0,0,0,0,6,3,
			5,0,10,10,22,6,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		};
		// m_dwWriteBitsUncompressed=4584 m_dwWriteBitsCompressed=2499

		char c; 

		for(c='0';c<='9';c++)	dwCharStats[(DWORD)c]++;
		for(c='a';c<='z';c++)	dwCharStats[(DWORD)c]++;
		for(c='A';c<='Z';c++)	dwCharStats[(DWORD)c]++;

		m_CharCompressor.InitFromStatistics(dwCharStats);
	}


/*	// testing code

#ifdef _DEBUG
	DWORD t[3];

	for(t[0]=0;t[0]<256;t[0]++)
	for(t[1]=0;t[1]<256;t[1]++)
	for(t[2]=0;t[2]<256;t[2]++)
	{
		char s[4];
		char q[4];

		s[0]=(char)t[0];
		s[1]=(char)t[1];
		s[2]=(char)t[2];
		s[3]=0;

		CStream str;

		Write(str,s);
		Read(str,q,4);

		assert(strcmp((const char *)s,(const char *)q)==0);
	}
#endif
*/
}

CCompressionHelper::~CCompressionHelper()
{
#ifdef GATHER_CHARSTATISTICS
	FILE *out=fopen("c:\\temp\\CharStats.cpp","wb");

//	fprintf(out,"");

	for(DWORD a=0;a<16;a++)
	{
		fprintf(out,"   ");
		for(DWORD b=0;b<16;b++)
		{
			DWORD i=a*16+b;

			fprintf(out,"%d,",m_dwCharStats[i]);
		}
		fprintf(out,"\r\n");
	}
	
	fprintf(out,"\r\nm_dwWriteBitsUncompressed=%d m_dwWriteBitsCompressed=%d\r\n",m_dwWriteBitsUncompressed,m_dwWriteBitsCompressed);
	

	fclose(out);
#endif
}


bool CCompressionHelper::Write( CStream &outStream, const unsigned char inChar )
{
	return m_CharCompressor.Write(outStream,inChar);
}

bool CCompressionHelper::Read( CStream &inStream, unsigned char &outChar )
{
	return m_CharCompressor.Read(inStream,outChar);
}

bool CCompressionHelper::Write( CStream &outStream, const char *inszString )
{
	assert(inszString);

	const unsigned char *p=(unsigned char *)inszString;

#ifdef GATHER_CHARSTATISTICS
	DWORD dwOldSize=outStream.GetSize();
#endif

	while(*p)
	{
		unsigned char c=*p++;
#ifdef GATHER_CHARSTATISTICS
		m_dwCharStats[c]++;
		m_dwWriteBitsUncompressed+=8;
#endif
		if(!Write(outStream,c))
			return false;
	}

	bool bRet=Write(outStream,(unsigned char)0);

#ifdef GATHER_CHARSTATISTICS
	m_dwCharStats[0]++;							// zero termination
	m_dwWriteBitsUncompressed+=8;
	m_dwWriteBitsCompressed+=outStream.GetSize()-dwOldSize;
#endif

	return bRet;
}

bool CCompressionHelper::Read( CStream &inStream, char *outszString, const DWORD indwStringSize )
{
	assert(outszString);
	assert(indwStringSize);

	for(DWORD i=0;i<indwStringSize;i++)
	{
		unsigned char c;
		
		if(!Read(inStream,c))
			return false;

		outszString[i]=(char)c;

		if(c==0)
			return true;
	}

	outszString[indwStringSize-1]=0;
	return false;
}




