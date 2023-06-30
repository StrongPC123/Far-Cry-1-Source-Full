// TextureCompression.cpp: implementation of the CTextureCompression class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TextureCompression.h"
#include <ddraw.h>
#include "Util\dds.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMemFile* CTextureCompression::m_pFile = NULL;
int CTextureCompression::m_numMips = 0;

CTextureCompression::CTextureCompression()
{
}

CTextureCompression::~CTextureCompression()
{
}

/*
// convert ctu -> ctc
void CTextureCompression::CompressCTU( PSTR pszTexturePath,bool bHiQuality )
{
	char szPath[_MAX_PATH];

	CString trgPath = Path::AddBackslash( pszTexturePath );

	CString ctuFile = Path::AddBackslash( pszTexturePath ) + "cover.ctu";
	CString ctcFile = Path::AddBackslash( pszTexturePath ) + "cover.ctc";

	sprintf(szPath, "%scover.ctu", pszTexturePath);

	if (!CFileUtil::OverwriteFile( ctuFile ))
		return;
	if (!CFileUtil::OverwriteFile( ctcFile ))
		return;

	FILE* f = fopen(ctuFile,"rb");
	if (!f) 
		return;

  FILE *c = fopen(ctcFile,"wb");
	if(!c) 
    return;

	sOutFile = c;

  int tex_res = 0;
  fread(&tex_res, 4, 1, f);
  fwrite(&tex_res, 4, 1, c);

//  int inern_format = 21872;
  
	CWaitProgress progress( "Compressing Surface Texture" );
  int counter = 0;

	char ddsfilename[128];

  while(1)
  {
    int buffer_size = tex_res*tex_res*3;
    uchar *raw_data = new uchar[buffer_size];
    fread( raw_data, buffer_size,1,f );

		if (bHiQuality)
		{
      GetIEditor()->GetRenderer()->DXTCompress( raw_data,tex_res,tex_res,eTF_DXT1,false,true,3,SaveCompessedMipmapLevel );
		}
		else
		{
      GetIEditor()->GetRenderer()->DXTCompress( raw_data,tex_res,tex_res,eTF_DXT1,true,true,3,SaveCompessedMipmapLevel );
		}

		sprintf( ddsfilename,"%sTerrainTex_%d",(const char*)trgPath,counter );
		GetIEditor()->GetRenderer()->WriteDDS( raw_data,tex_res,tex_res,3,ddsfilename,eIF_DXT1,50 );

    delete []raw_data;

    fread(&tex_res, 4, 1, f);

    if(feof(f))
      break;

    if(tex_res != 64 && tex_res != 128 && tex_res != 256)
    {
//      exit(0);
    }

    counter++;
		progress.Step( (counter*100)/1024 );
  }

  fclose(f);
  fclose(c);

	DeleteFile( ctuFile );
}
*/

//////////////////////////////////////////////////////////////////////////
HRESULT CTextureCompression::SaveCompessedMipmapLevel(void * data, int miplevel, DWORD size, int width, int height, void * user_data)
{
	assert( m_pFile );
	m_pFile->Write( data,size );
	m_numMips++;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CTextureCompression::CompressDXT1( CFile &toFile,CImage &image,bool bHiQuality )
{
	m_pFile = new CMemFile;
	m_numMips = 0;

	bool bUseHW = !bHiQuality;
	GetIEditor()->GetRenderer()->DXTCompress( (unsigned char*)image.GetData(),image.GetWidth(),image.GetHeight(),eTF_DXT1,bUseHW,true,4,SaveCompessedMipmapLevel );

	int len = m_pFile->GetLength();
	toFile.Write( m_pFile->Detach(),len );

	delete m_pFile;
}

//////////////////////////////////////////////////////////////////////////
void CTextureCompression::CompressDDS( CFile &toFile,CImage &image,bool bHiQuality )
{
	//if (!CFileUtil::OverwriteFile( filename ))
		//return;
	m_pFile = new CMemFile;
	m_numMips = 0;

	bool bUseHW = !bHiQuality;
	GetIEditor()->GetRenderer()->DXTCompress( (unsigned char*)image.GetData(),image.GetWidth(),image.GetHeight(),eTF_DXT1,bUseHW,true,4,SaveCompessedMipmapLevel );

	int size = m_pFile->GetLength();
	WriteDDS( toFile,m_pFile->Detach(),image.GetWidth(),image.GetHeight(),size,eIF_DXT1,m_numMips );

	delete m_pFile;
}

//////////////////////////////////////////////////////////////////////////
void CTextureCompression::WriteDDS( CFile &file,unsigned char *dat, int w,int h,int Size, EImFormat eF, int NumMips )
{
	DWORD dwMagic;
	DDS_HEADER ddsh;
	ZeroStruct(ddsh);

	dwMagic = MAKEFOURCC('D','D','S',' ');
	file.Write( &dwMagic,sizeof(DWORD) );

	ddsh.dwSize = sizeof(DDS_HEADER);
	ddsh.dwWidth = w;
	ddsh.dwHeight = h;
	ddsh.dwMipMapCount = NumMips;
	if (!NumMips)
		ddsh.dwMipMapCount = 1;
	ddsh.dwHeaderFlags = DDS_HEADER_FLAGS_TEXTURE | DDS_HEADER_FLAGS_MIPMAP;
	ddsh.dwSurfaceFlags = DDS_SURFACE_FLAGS_TEXTURE | DDS_SURFACE_FLAGS_MIPMAP;

	switch (eF)  
	{
	case eIF_DXT1:
		ddsh.ddspf = DDSPF_DXT1;
		break;
	case eIF_DXT3:
		ddsh.ddspf = DDSPF_DXT3;
		break;
	case eIF_DXT5:
		ddsh.ddspf = DDSPF_DXT5;
		break;
	case eIF_DDS_RGB8:
	case eIF_DDS_SIGNED_RGB8:
	case eIF_DDS_DSDT:
		ddsh.ddspf = DDSPF_R8G8B8;
		break;
	case eIF_DDS_RGBA8:
		ddsh.ddspf = DDSPF_A8R8G8B8;
		break;
	default:
		assert(0);
		return;
	}
	file.Write( &ddsh, sizeof(ddsh) );

	byte *data = NULL;

	if (eF == eIF_DDS_RGB8 || eF == eIF_DDS_SIGNED_RGB8 || eF == eIF_DDS_DSDT)
	{
		data = new byte[Size];
		int n = Size / 3;
		for (int i=0; i<n; i++)
		{
			data[i*3+0] = dat[i*3+2];
			data[i*3+1] = dat[i*3+1];
			data[i*3+2] = dat[i*3+0];
		}
		file.Write( data,Size );
	}
	else
		if (eF == eIF_DDS_RGBA8)
		{
			data = new byte[Size];
			int n = Size / 4;
			for (int i=0; i<n; i++)
			{
				data[i*4+0] = dat[i*4+2];
				data[i*4+1] = dat[i*4+1];
				data[i*4+2] = dat[i*4+0];
				data[i*4+3] = dat[i*4+3];
			}
		}
		else
			file.Write( dat,Size );

	SAFE_DELETE_ARRAY(data);
}