// TextureCompression.h: interface for the CTextureCompression class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTURECOMPRESSION_H__B2702EC6_F5D8_4BB3_B2EE_A2F66C128380__INCLUDED_)
#define AFX_TEXTURECOMPRESSION_H__B2702EC6_F5D8_4BB3_B2EE_A2F66C128380__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTextureCompression  
{
public:
	CTextureCompression();
	virtual ~CTextureCompression();

	//void CompressCTU( PSTR pszTexturePath,bool bHiQuality );
	void CompressDXT1( CFile &toFile,CImage &image,bool bHiQuality );
	void CompressDDS( CFile &toFile,CImage &image,bool bHiQuality );
	void WriteDDS( CFile &toFile,unsigned char *dat, int w,int h,int Size, EImFormat eF, int NumMips );

private:
	static HRESULT SaveCompessedMipmapLevel(void * data, int miplevel, DWORD size, int width, int height, void * user_data);
	static CMemFile* m_pFile;
	static int m_numMips;
};

#endif // !defined(AFX_TEXTURECOMPRESSION_H__B2702EC6_F5D8_4BB3_B2EE_A2F66C128380__INCLUDED_)
